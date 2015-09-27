//
// Created by 杨至轩 on 9/5/15.
//

#include <map>
#include <queue>
#include <common/token/word.h>
#include <common/token/pos.h>
#include <assert.h>
#include "titovdp_depparser.h"
#include "titovdp_weight.h"

namespace titovdp{

    template<typename T1, typename T2>
    static void cast_to(T1 &a, T2 b) {
        a = (T1) b;
    };

    DepParser::DepParser(const std::string &sFeatureInput, const std::string &sFeatureOutput, const ParserState &state)
            : DepParserBase(sFeatureInput, state) {
        m_Weight = new Weight(sFeatureInput, sFeatureOutput);
        nRound = 0;
        sentenceLength = 0;
        numGoldDecutions = 0;
        numCorrectDeductions = 0;
        numDecodedDeductions = 0;
        numDecodedArcs = 0;
        numGoldArcs = 0;
        numCorrectArcs = 0;
        cast_to(chart, &chart_mem[1][1]);
        cast_to(chart_sortbyx, &chart_sortbyx_mem[1][1]);
        cast_to(chart_sortbyz, &chart_sortbyz_mem[1][1]);
        for(int i = 0; i < MAX_SENTENCE_SIZE; i++){
            for(int j = 0; j < MAX_SENTENCE_SIZE; j++){
                chart[i][j].clear();
            }
        }
        DepParser::empty_word.refer(TWord::code(EMPTY_WORD), TPOSTag::code(EMPTY_POSTAG));
        DepParser::start_word.refer(TWord::code(START_WORD), TPOSTag::code(START_POSTAG));
        DepParser::end_word.refer(TWord::code(END_WORD), TPOSTag::code(END_POSTAG));
    }

    void DepParser::train(const DependencyGraph &ref_sent) {
        nRound++;

        sentenceLength = ref_sent.num_words;
        for(int i = 0; i <= ref_sent.num_words; i++) {
            sent[i].refer(TWord::code(std::get<0>(ref_sent.pos_words[i])),
                          TPOSTag::code(std::get<1>(ref_sent.pos_words[i])));
        }
        bool parsable;
        decode();
        decodeTransitions();
        parsable = getGoldTransitions(ref_sent);
        if(!parsable) {
            std::cerr<<"This sentence is not parsable in Titov's system"<<std::endl;
            return;
        }

        if (nRound > 2000) {
            std::cout<<"round "<<nRound<<" gold start:\n";
            std::cout<<YELLOW;
            for(auto d : goldDeductions) {
                std::cout<<d;
                if(decodedDeductions.find(d) == decodedDeductions.end()) {
                    std::cout<<", not decoded";
                    PushComputation p;
                    if(d.type == REDUCE) {
                        p = reduce_pc(d.pc1, d.pc2);
                    } else if (d.type == SHIFT) {
                        p = d.pc1;
                    } else {
                        p = singleton_pc(d.pc1, d.type);
                    }
                    int found = 0;
                    for(int k = 0; k < chart[p.i][p.j].size(); k++) {
                        if(chart[p.i][p.j][k]->pc == p) {
                            found = 1;
                            break;
                        }
                    }
                    if(!found) {
                        std::cout<<", not in the beam";
                    }
                }
                std::cout<<std::endl;
            }
            std::cout<<RESET;
            std::cout<<"end;\n";

            std::cout<<BLUE;
            for(auto d : decodedDeductions) {
                std::cout<<d;
                if(goldDeductions.find(d) == goldDeductions.end()) {
                    std::cout<<", not gold";
                }
                std::cout<<std::endl;
            }
            std::cout<<"end;\n";
            std::cout<<RESET;
        }

        int error = 0;
        for(auto d : goldDeductions) {
            numGoldDecutions += 1;
            if(decodedDeductions.find(d) != decodedDeductions.end()) {
                continue;
            }
            if(d.type == REDUCE) {
                getOrUpdateScore(d.pc1, d.pc2, 1);
            } else {
                getOrUpdateScore(d.pc1, d.type, 1);
            }
            error = 1;
        }

        for(auto d : decodedDeductions) {
            numDecodedDeductions += 1;
            if(goldDeductions.find(d) != goldDeductions.end()) {
                numCorrectDeductions += 1;
                continue;
            }
            if(d.type == REDUCE) {
                getOrUpdateScore(d.pc1, d.pc2, -1);
            } else {
                getOrUpdateScore(d.pc1, d.type, -1);
            }
            error = 1;
        }


        m_nTotalErrors += error;

        decodeArcs();
        numDecodedArcs += decodedArcs.size();
        numGoldArcs += ref_sent.num_arcs;
        for(auto arc : decodedArcs) {
            if(ref_sent.graph[arc.gram1][arc.gram2]) {
                numCorrectArcs += 1;
            }
        }

        if(nRound % OUTPUT_STEP == 0) {
            std::cout<<"Training round: " << nRound<<", number of errors: "<< m_nTotalErrors<<std::endl;
            double precision = (double)numCorrectDeductions / numDecodedDeductions;
            double recall = (double)numCorrectDeductions / numGoldDecutions;
            std::cout<<"Deduction precision: "<< precision << ", recall: "<<recall<<std::endl;

            precision = (double) numCorrectArcs / numDecodedArcs;
            recall = (double) numCorrectArcs / numGoldArcs;
            std::cout<<"Arc precision: "<< precision << ", recall: "<<recall<<std::endl;

            numCorrectArcs = numGoldArcs = numDecodedArcs = 0;
            numCorrectDeductions = numGoldDecutions = numDecodedDeductions = 0;
        }
        // std::cerr<<"trained one. "<<"decoded: "<< decodedDeductions.size()<< ", gold: " << goldDeductions.size() <<
        //         "(round: "<<nRound<<")"<<std::endl;
    }



    void DepParser::decode() {
        int i, j, k, x, len;
        PushComputation pc;
        ScoreInformation score;
        PCHashT candidates;

        // Axioms: SHIFT computations
        for(i = -1; i <= sentenceLength; i++) {
            candidates.clear();
            for(x = -1; x < i || /*virtual shift*/ (x == -1 && i == -1); x++) {
                pc = shift_pc(x, i);
                score.score = getOrUpdateScore(pc, SHIFT, 0);
                score.prev1 = score.prev2 = nullptr;
                addItem(candidates, pc, score);
            }
            gatherBeam(candidates, i, i+1);
        }

        for(len = 2; len <= sentenceLength + 2; len++) {
            for(i = -1; i <= sentenceLength; i++) {
                j = i + len;
                if(j > sentenceLength + 1) {
                    break;
                }
                candidates.clear();
                for(k = i + 1; k <= j - 1; k++) {
                    int *p1, *p2, *p1_end, *p2_end;
                    p1 = &chart_sortbyz[i][k][0];
                    p1_end = p1 + chart[i][k].size();
                    p2 = &chart_sortbyx[k][j][0];
                    p2_end = p2 + chart[k][j].size();
                    while(p1 != p1_end) {
                        while(p2 != p2_end && chart[k][j][*p2]->pc.x < chart[i][k][*p1]->pc.z) {
                            p2++;
                        }
                        if(p2 == p2_end){
                            break;
                        }
                        if(chart[k][j][*p2]->pc.x == chart[i][k][*p1]->pc.z) {
                            int *p3 = p2;
                            while(p3 != p2_end && chart[k][j][*p3]->pc.x == chart[i][k][*p1]->pc.z) {
                                pc = reduce_pc(chart[i][k][*p1]->pc, chart[k][j][*p3]->pc);

                                score.score = chart[i][k][*p1]->attr.score + chart[k][j][*p3]->attr.score +
                                        getOrUpdateScore(chart[i][k][*p1]->pc, chart[k][j][*p3]->pc, 0);
                                score.prev1 = chart[i][k][*p1];
                                score.prev2 = chart[k][j][*p3];

                                addItem(candidates, pc, score);
                                p3++;
                            }
                        }
                        p1++;
                    }
                }
                gatherBeam(candidates, i, j);
            }
        }
    }

    void DepParser::traversePC(cSPCPtr ppc) {
        Deduction d;
        cSPCPtr pprev1 = ppc->attr.prev1;
        cSPCPtr pprev2 = ppc->attr.prev2;
        if(pprev1 == nullptr) {
            d.pc1 = ppc->pc;
            d.type = SHIFT;
            decodedDeductions.insert(d);
            return;
        }
        if (pprev1 != nullptr && pprev2 != nullptr) {
            d.pc1 = pprev1->pc;
            d.pc2 = pprev2->pc;
            d.type = REDUCE;
            decodedDeductions.insert(d);
            traversePC(pprev1);
            traversePC(pprev2);
            return;
        }
        if(pprev1->pc.yz_swapped != ppc->pc.yz_swapped) {
            d.type = SWAP;
        } else if(pprev1->pc.zj_connected != ppc->pc.zj_connected) {
            d.type = RIGHT_ARC;
        } else if(pprev1->pc.jz_connected != ppc->pc.jz_connected) {
            d.type = LEFT_ARC;
        }
        d.pc1 = pprev1->pc;
        decodedDeductions.insert(d);
        traversePC(pprev1);
    }

    void DepParser::decodeTransitions() {
        // Goal: [-1 -1 -1 -1 n+1] with highest score
        SPCPtr pc = nullptr;
        decodedDeductions.clear();
        for(int i = 0; i < chart[-1][sentenceLength+1].size(); i++) {
            if(chart[-1][sentenceLength+1][i]->pc.z == -1
                    && chart[-1][sentenceLength+1][i]->pc.y == -1) {
                pc = chart[-1][sentenceLength+1][i];
            }
        }
        if(pc == nullptr) {
            //maybe we should find the pc with highest score?
            std::cerr<<"decode failed: goal not found"<<std::endl;
            return;
        }
        traversePC(pc);
    }

    void DepParser::decodeArcs() {
        decodedArcs.clear();
        for(auto d : decodedDeductions) {
            if(d.type == LEFT_ARC) {
                decodedArcs.insert(Arc(d.pc1.j, d.pc1.z));
            } else if(d.type == RIGHT_ARC) {
                decodedArcs.insert(Arc(d.pc1.z, d.pc1.j));
            }
        }

    }

    void DepParser::addItem(PCHashT & candidates, const PushComputation & pc, const ScoreInformation & score_info) {
        if(candidates.find(pc) == candidates.end()) {
            std::shared_ptr<ScoredPushComputation> spc(new ScoredPushComputation(pc, score_info));
            candidates[pc] = spc;
        } else {
            if(score_info.score > candidates[pc]->attr.score) {
                candidates[pc]->attr = score_info;
            } else {
                return;
            }
        }

        ScoreInformation newinfo;
        newinfo.prev1 = candidates[pc];
        newinfo.prev2 = nullptr;
        if(!pc.zj_connected && pc.z >= 0 && pc.j <= sentenceLength) {
            newinfo.score = score_info.score + getOrUpdateScore(pc, RIGHT_ARC, 0);
            PushComputation newpc = singleton_pc(pc, RIGHT_ARC);
            addItem(candidates, newpc, newinfo);
        }
        if(!pc.jz_connected && pc.z >= 0 && pc.j <= sentenceLength) {
            newinfo.score = score_info.score + getOrUpdateScore(pc, LEFT_ARC, 0);
            PushComputation newpc = singleton_pc(pc, LEFT_ARC);
            addItem(candidates, newpc, newinfo);
        }
        if(!pc.yz_swapped && pc.y >= 0 && pc.z >= 0) {
            newinfo.score = score_info.score + getOrUpdateScore(pc, SWAP, 0);
            PushComputation newpc = singleton_pc(pc, SWAP);
            addItem(candidates, newpc, newinfo);
        }
    }

    void DepParser::gatherBeam(const PCHashT & candidates, int i, int j) {
        chart[i][j].clear();
        for(auto iter : candidates) {
            chart[i][j].insertItem(iter.second);
        }

        for(int k = 0; k < chart[i][j].size(); k++) {
            chart_sortbyx[i][j][k] = k;
            chart_sortbyz[i][j][k] = k;
        }
        std::sort(&chart_sortbyz[i][j][0], (&chart_sortbyz[i][j][0]) + chart[i][j].size(),
                  [&](const int& p1, const int& p2){
                      return chart[i][j][p1]->pc.z < chart[i][j][p2]->pc.z;
                  });
        std::sort(&chart_sortbyx[i][j][0], (&chart_sortbyx[i][j][0]) + chart[i][j].size(),
                  [&](const int& p1, const int& p2){
                      return chart[i][j][p1]->pc.x < chart[i][j][p2]->pc.x;
                  });
    }
}
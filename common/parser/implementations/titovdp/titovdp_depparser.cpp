//
// Created by 杨至轩 on 9/5/15.
//

#include <map>
#include <algorithm>
#include <queue>
#include <common/token/word.h>
#include <common/token/pos.h>
#include <assert.h>
#include "titovdp_depparser.h"
#include "titovdp_weight.h"

namespace titovdp {

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
        for (int i = 0; i < MAX_SENTENCE_SIZE; i++) {
            for (int j = 0; j < MAX_SENTENCE_SIZE; j++) {
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
        for (int i = 0; i <= ref_sent.num_words; i++) {
            sent[i].refer(TWord::code(std::get<0>(ref_sent.pos_words[i])),
                          TPOSTag::code(std::get<1>(ref_sent.pos_words[i])));
        }
        bool parsable;
        decode();
        decodeTransitions();
        parsable = getGoldTransitions(ref_sent);
        if (!parsable) {
            std::cerr << "This sentence is not parsable in Titov's system" << std::endl;
            return;
        }

        if (0 && nRound == 6) {
            std::cout << "round " << nRound << " gold start:\n";
            std::cout << YELLOW;
            tscore goldScore = 0;
            for (auto d : goldDeductions) {
                std::cout << d;
                if(is_reduce(d.type)) {
                    goldScore += getOrUpdateScore(d.pc1, d.pc2, d.type, 0);
                } else {
                    goldScore += getOrUpdateScore(d.pc1, d.type, 0);
                }
                if (decodedDeductions.find(d) == decodedDeductions.end()) {
                    std::cout << ", not decoded";

                    PushComputation p;
                    if (is_reduce(d.type)) {
                        p = reduce_pc(d.pc1, d.pc2, d.type);
                    } else if(is_shift(d.type)) {
                        p = shift_pc(d.pc1, d.type);
                    } else {
                        p = swap_pc(d.pc1, d.type);
                    }
                    int found = 0;
                    for (int k = 0; k < chart[p.i][p.j].size(); k++) {
                        if (chart[p.i][p.j][k]->pc == p) {
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        std::cout << ", not in the beam";
                    }
                }
                std::cout << std::endl;
            }
            std::cout << RESET;
            std::cout << "end;\n";
            std::cerr<<"gold score: "<<goldScore<<std::endl;

            std::cout << BLUE;
            for (auto d : decodedDeductions) {
                std::cout << d;
                if (goldDeductions.find(d) == goldDeductions.end()) {
                    std::cout << ", not gold";
                }
                std::cout << std::endl;
            }
            std::cout << "end;\n";
            std::cout << RESET;
        }

        int error = 0;
        for (auto d : goldDeductions) {
            numGoldDecutions += 1;
            if (decodedDeductions.find(d) != decodedDeductions.end()) {
                continue;
            }
            if (is_reduce(d.type)) {
                getOrUpdateScore(d.pc1, d.pc2, d.type, 1);
            } else {
                getOrUpdateScore(d.pc1, d.type, 1);
            }
            error = 1;
        }

        for (auto d : decodedDeductions) {
            numDecodedDeductions += 1;
            if (goldDeductions.find(d) != goldDeductions.end()) {
                numCorrectDeductions += 1;
                continue;
            }
            if (is_reduce(d.type)) {
                getOrUpdateScore(d.pc1, d.pc2, d.type, -1);
            } else {
                getOrUpdateScore(d.pc1, d.type, -1);
            }
            error = 1;
        }


        m_nTotalErrors += error;

        decodeArcs();
        numDecodedArcs += decodedArcs.size();
        numGoldArcs += ref_sent.num_arcs;
        for (auto arc : decodedArcs) {
            if (ref_sent.graph[arc.gram1][arc.gram2]) {
                numCorrectArcs += 1;
            }
        }

        if (nRound % OUTPUT_STEP == 0) {
            std::cout << "Training round: " << nRound << ", number of errors: " << m_nTotalErrors << std::endl;
            double precision = (double) numCorrectDeductions / numDecodedDeductions;
            double recall = (double) numCorrectDeductions / numGoldDecutions;
            std::cout << "Deduction precision: " << precision << ", recall: " << recall << std::endl;

            precision = (double) numCorrectArcs / numDecodedArcs;
            recall = (double) numCorrectArcs / numGoldArcs;
            std::cout << "Arc precision: " << precision << ", recall: " << recall << std::endl;

            numCorrectArcs = numGoldArcs = numDecodedArcs = 0;
            numCorrectDeductions = numGoldDecutions = numDecodedDeductions = 0;
        }
        // std::cerr<<"trained one. "<<"decoded: "<< decodedDeductions.size()<< ", gold: " << goldDeductions.size() <<
        //         "(round: "<<nRound<<")"<<std::endl;

        if(error) {
            std::cerr<<"retry " << nRound << "\n";
            nRound--;
            train(ref_sent);
        }
    }


    void DepParser::decode() {
        int i, j, k, idx, action;
        PushComputation pc;
        ScoreInformation score;

        // Axioms: Virtual Shift
        pc.i = -1, pc.j = 0, pc.y = -1, pc.z = -1;
        pc.jls = pc.yls = pc.yrs = pc.zls = pc.zrs = -1;
        score.score = 0, score.type = SHIFT;
        score.prefix_score = 0;
        score.prev1 = score.prev2 = nullptr;
        score.predictor = shared_ptr<PredictorSet>(new PredictorSet);
        candidates[1].clear();
        addItem(candidates[1], pc, score);
        gatherBeam(candidates[1], -1, 0);

        for(j = 1; j <= sentenceLength + 1; j++) {
            for(i = -1; i < j; i++) {
                candidates[j - i].clear();
            }
            // SHIFT
            i = j-1;
            for(k = -1; k < i; k++) {
                // [k, *, *, j-1]  --> [j-1, *, *, j]
                for(idx = 0; idx < chart[k][i].size(); idx++) {
                    for(action = 0; action < NUM_SHIFT_ACTIONS; action++) {
                        Transition act = shift_actions[action];
                        if(!actionApplicable(chart[k][i][idx]->pc, act)) {
                            continue;
                        }
                        pc = shift_pc(chart[k][i][idx]->pc, act);
                        score.type = act;
                        score.prev1 = chart[k][i][idx];
                        score.prev2 = nullptr;
                        score.predictor = shared_ptr<PredictorSet>(new PredictorSet);
                        score.predictor->push_back(chart[k][i][idx]);
                        score.score = getOrUpdateScore(chart[k][i][idx]->pc, act, 0);
                        score.prefix_score = chart[k][i][idx]->attr.prefix_score + score.score;
                        addItem(candidates[1], pc, score);
                    }
                }
            }

            // REDUCE
            for(i = j - 1; i >= -1; i--) {
                gatherBeam(candidates[j - i], i, j);
                for(idx = 0; idx < chart[i][j].size(); idx++) {
                    auto pc2 = chart[i][j][idx];
                    for(auto pc1 : *(pc2->attr.predictor)) {
                        for(action = 0; action < NUM_REDUCE_ACTIONS; action++) {
                            Transition act = reduce_actions[action];
                            if(!actionApplicable(pc2->pc, act)) {
                                continue;
                            }
                            pc = reduce_pc(pc1->pc, pc2->pc, act);
                            score.type = act;
                            score.prev1 = pc1;
                            score.prev2 = pc2;
                            tscore delta = getOrUpdateScore(pc1->pc, pc2->pc, act, 0);
                            score.score = pc1->attr.score + pc2->attr.score + delta;
                            score.prefix_score = pc1->attr.prefix_score + pc2->attr.score + delta;
                            score.predictor = pc1->attr.predictor;
                            addItem(candidates[pc.j - pc.i], pc, score);
                        }
                    }
                }
            }
        }
    }

    bool DepParser::actionApplicable(const PushComputation & pc, Transition t) {
        if(is_shift(t)) {
            if( (t == LA_SHIFT || t == RA_SHIFT) && pc.z == -1 ) {
                return false;
            }
            return pc.j <= sentenceLength;
        } else if(is_reduce(t)) {
            if( (t == LA_REDUCE || t == RA_REDUCE) && pc.j > sentenceLength) {
                return false;
            }
            return pc.z != -1;
        } else {
            if( (t == LA_SWAP || t == RA_SWAP) && pc.j > sentenceLength) {
                return false;
            }
            return pc.z != -1 && pc.y != -1;
        }
    }

    void DepParser::traversePC(cSPCPtr pc) {
        Deduction d;
        if(is_reduce(pc->attr.type)) {
            d.pc1 = pc->attr.prev1->pc;
            d.pc2 = pc->attr.prev2->pc;
            d.type = pc->attr.type;
            decodedDeductions.insert(d);
            traversePC(pc->attr.prev1);
            traversePC(pc->attr.prev2);
            return;
        } else if(is_shift(pc->attr.type)) {
            if(pc->attr.prev1 == nullptr) {
                // virtual shift. no predecessor.
                assert(pc->pc.j == 0);
                return;
            }
            d.pc1 = pc->attr.prev1->pc;
            d.type = pc->attr.type;
            decodedDeductions.insert(d);
            return;
        } else {
            assert(pc != pc->attr.prev1);
            d.pc1 = pc->attr.prev1->pc;
            d.type = pc->attr.type;
            //std::cerr<<"swap "<<d.pc1<<std::endl;
            decodedDeductions.insert(d);
            traversePC(pc->attr.prev1);
            return;
        }
    }

    void DepParser::decodeTransitions() {
        // Goal: [-1 -1 -1 -1 n+1] with highest score
        SPCPtr pc = nullptr;
        decodedDeductions.clear();
        for (int i = 0; i < chart[-1][sentenceLength + 1].size(); i++) {
            if (chart[-1][sentenceLength + 1][i]->pc.z == -1
                && chart[-1][sentenceLength + 1][i]->pc.y == -1) {
                pc = chart[-1][sentenceLength + 1][i];
            }
        }
        if (pc == nullptr) {
            //maybe we should find the pc with highest score?
            std::cerr << "decode failed: goal not found" << std::endl;
            return;
        }
        // std::cerr<<"decoded score: "<<pc->attr.score<<std::endl;
        traversePC(pc);
    }

    void DepParser::decodeArcs() {
        decodedArcs.clear();
        for (auto d : decodedDeductions) {
            if(d.type == LA_SHIFT || d.type == LA_SWAP) {
                decodedArcs.insert(Arc(d.pc1.j, d.pc1.z));
            }else if (d.type == LA_REDUCE){
                decodedArcs.insert(Arc(d.pc2.j, d.pc2.z));
            }else if(d.type == RA_SHIFT || d.type == RA_SWAP) {
                decodedArcs.insert(Arc(d.pc1.z, d.pc1.j));
            }else if (d.type == RA_REDUCE) {
                decodedArcs.insert(Arc(d.pc2.z, d.pc2.j));
            }
        }

    }

    void DepParser::updateItem(PCHashT &candidates, const PushComputation &pc, const ScoreInformation &score_info) {
        if(candidates.find(pc) == candidates.end()) {
            shared_ptr<ScoredPushComputation> spc(new ScoredPushComputation(pc, score_info));
            candidates[pc] = spc;
        } else {
            if(is_shift(score_info.type)) {
                // shift. Merge predictor states
                candidates[pc]->attr.predictor->insert(candidates[pc]->attr.predictor->begin(),
                                                       score_info.predictor->begin(),
                                                       score_info.predictor->end());
                if(score_info.prefix_score > candidates[pc]->attr.prefix_score) {
                    candidates[pc]->attr.prefix_score = score_info.prefix_score;
                    candidates[pc]->attr.score = score_info.score;
                    candidates[pc]->attr.prev1 = score_info.prev1;
                    candidates[pc]->attr.prev2 = score_info.prev2;
                    candidates[pc]->attr.type = score_info.type;
                }
            } else {
                // non-shift. Maintain highest score
                if(score_info.score > candidates[pc]->attr.score){
                    candidates[pc]->attr = score_info;
                }
            }
        }
    }

    void DepParser::addItem(PCHashT &candidates, const PushComputation &pc, const ScoreInformation &score_info) {
        updateItem(candidates, pc, score_info);
        // try SWAP actions
        for(int action = 0; action < NUM_SWAP_ACTIONS; action++) {
            if(!actionApplicable(pc, swap_actions[action])) {
                continue;
            }
            PushComputation newpc = swap_pc(pc, swap_actions[action]);
            ScoreInformation newscore;
            tscore delta = getOrUpdateScore(pc, swap_actions[action], 0);
            newscore.score = score_info.score + delta;
            newscore.prefix_score = score_info.prefix_score + delta;
            newscore.type = swap_actions[action];
            newscore.prev1 = candidates[pc];
            newscore.prev2 = nullptr;
            newscore.predictor = score_info.predictor;
            updateItem(candidates, newpc, newscore);
        }
    }


    void DepParser::gatherBeam(const PCHashT &candidates, int i, int j) {
        chart[i][j].clear();
        for (auto iter : candidates) {
            chart[i][j].insertItem(iter.second);
        }
    }
}

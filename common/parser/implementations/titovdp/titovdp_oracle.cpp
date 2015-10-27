//
// Created by 杨至轩 on 9/11/15.
//

#include <deque>
#include <cstring>
#include <assert.h>
#include "titovdp_depparser.h"

namespace titovdp {
    static bool precedes(std::deque<int> &d1, std::deque<int> &d2) {
        auto i1 = d1.begin();
        auto i2 = d2.begin();
        while (i1 != d1.end() && i2 != d2.end()) {
            if (*i1 != *i2) {
                return *i1 < *i2;
            }
            i1++;
            i2++;
        }
        return d1.size() < d2.size();
    }

    static PushComputation traverse(DeductionSet &retval, const std::vector<Transition> &seq,
                                    const std::vector<int> &depth, int start, int end,
                                    const PushComputation* predecessor) {
        Deduction d;
        Transition act = seq[end];
        d.type = act;
        if(is_shift(act)) {
            if(predecessor == nullptr) {
                PushComputation pc;
                pc.i = -1, pc.j = 0, pc.y = -1, pc.z = -1;
                pc.jls = pc.yls = pc.yrs = pc.zls = pc.zrs = -1;
                return pc;
            }
            d.pc1 = *predecessor;
            retval.insert(d);
            return d.pc1 = shift_pc(*predecessor, act);
        } else if(is_reduce(act)) {
            int k;
            for (k = end - 1; k >= start; k--) {
                if (depth[k] == depth[end]) {
                    break;
                }
            }
            assert(k >= start);
            d.pc1 = traverse(retval, seq, depth, start, k, predecessor);
            d.pc2 = traverse(retval, seq, depth, k + 1, end - 1, &d.pc1);
            retval.insert(d);
            return reduce_pc(d.pc1, d.pc2, act);
        } else {
            d.pc1 = traverse(retval, seq, depth, start, end - 1, predecessor);
            retval.insert(d);
            return swap_pc(d.pc1, act);
        }
    }

    bool DepParser::getGoldTransitions(const DependencyGraph &gold) {
        goldDeductions.clear();
        std::vector<int> stack;
        std::vector<Transition> seq;
        std::vector<int> depth;
        std::vector<std::deque<int> > arcs;
        int graph[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];
        memcpy(graph, gold.graph, sizeof(gold.graph));

        for (int i = 0; i <= gold.num_words; i++) {
            std::deque<int> row;
            for (int j = i; j <= gold.num_words; j++) {
                if (gold.graph[j][i]) {
                    row.push_back(j);
                }
                if (gold.graph[i][j]) {
                    row.push_back(j);
                }
            }
            arcs.push_back(row);
        }

        seq.push_back(SHIFT);      // virtual shift
        depth.push_back(1);

        int arc_mark = 0;           // -1 : right_arc, 1 : left_arc, 0 : nothing
        // std::cerr<<"start"<<std::endl;
        int ptr = 0, success = 1;
        while (ptr <= gold.num_words || !stack.empty()) {
            if (stack.empty()) {
                seq.push_back(SHIFT);
                stack.push_back(ptr);
                depth.push_back(stack.size() + 1);
                ptr++;
                continue;
            }

            if (!stack.empty()) {
                int &z = stack[stack.size() - 1];
                if (ptr <= gold.num_words && graph[z][ptr]) {
                    arc_mark = -1;
                    // std::cerr<<"want: "<<z<<" "<<ptr<<", got: "<<arcs[z].front()<<"\n";
                    // assert(arcs[z].front() == ptr);
                    if (arcs[z].front() != ptr) {
                        success = 0;
                        break;
                    }
                    arcs[z].pop_front();
                    graph[z][ptr] = 0;
                }
                if (ptr <= gold.num_words && graph[ptr][z]) {
                    arc_mark = 1;
                    // std::cerr<<"want: "<<z<<" "<<ptr<<", got: "<<arcs[z].front()<<"\n";
                    // assert(arcs[z].front() == ptr);
                    if (arcs[z].front() != ptr) {
                        success = 0;
                        break;
                    }
                    arcs[z].pop_front();
                    graph[ptr][z] = 0;
                }
                if (arcs[z].empty()) {
                    if(arc_mark == 1) {
                        seq.push_back(LA_REDUCE);
                    } else if(arc_mark == -1) {
                        seq.push_back(RA_REDUCE);
                    } else {
                        seq.push_back(REDUCE);
                    }
                    arc_mark = 0;
                    stack.pop_back();
                    depth.push_back(stack.size() + 1);
                    continue;
                }

                if (stack.size() >= 2) {
                    int &y = stack[stack.size() - 2];
                    if (precedes(arcs[y], arcs[z])) {
                        if(arc_mark == 1) {
                            seq.push_back(LA_SWAP);
                        } else if(arc_mark == -1) {
                            seq.push_back(RA_SWAP);
                        } else {
                            seq.push_back(SWAP);
                        }
                        arc_mark = 0;
                        depth.push_back(stack.size() + 1);
                        std::swap(z, y);
                        continue;
                    }
                }

                // no choice, use shift
                if (ptr > gold.num_words) {
                    success = 0;
                    break;
                }
                if(arc_mark == 1) {
                    seq.push_back(LA_SHIFT);
                } else if(arc_mark == -1) {
                    seq.push_back(RA_SHIFT);
                } else {
                    seq.push_back(SHIFT);
                }
                arc_mark = 0;
                stack.push_back(ptr);
                depth.push_back(stack.size() + 1);
                ptr++;
            }
        }

        // std::cerr<<"end"<<std::endl;
        if (!success) {
            return false;
        }

        goldDeductions.clear();
        traverse(goldDeductions, seq, depth, 0, seq.size() - 1, nullptr);

#if 0
        std::cout<< sentenceLength<<std::endl;
            for(auto act : seq) {
                std::cout << act << " ";
            }
            std::cout<<std::endl;
#endif
        return success;
    }

}

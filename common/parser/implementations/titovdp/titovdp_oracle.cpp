//
// Created by 杨至轩 on 9/11/15.
//

#include <deque>
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
                                        const std::vector<int> &depth, int start, int end, int x, int i) {
        if (start == end) {
            Deduction d;
            d.pc1 = shift_pc(x, i);
            d.type = SHIFT;
            retval.insert(d);
            return d.pc1;
        }
        Deduction d;
        d.type = seq[end];
        switch (seq[end]) {
            case LEFT_ARC:
            case RIGHT_ARC:
            case SWAP:
                d.pc1 = traverse(retval, seq, depth, start, end - 1, x, i);
                retval.insert(d);
                return singleton_pc(d.pc1, seq[end]);
                break;
            case REDUCE:
                int k;
                for (k = end - 1; k >= start; k--) {
                    if (depth[k] == depth[end]) {
                        break;
                    }
                }
                assert(k >= start);
                d.pc1 = traverse(retval, seq, depth, start, k, x, i);
                d.pc2 = traverse(retval, seq, depth, k + 1, end - 1, d.pc1.z, d.pc1.j);
                d.type = REDUCE;
                retval.insert(d);
                return reduce_pc(d.pc1, d.pc2);
                break;
            default:
                std::cerr << "You are doomed in LINE" << __LINE__ << std::endl;
                return PushComputation();
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
                    seq.push_back(RIGHT_ARC);
                    depth.push_back(stack.size() + 1);
                    // std::cerr<<"want: "<<z<<" "<<ptr<<", got: "<<arcs[z].front()<<"\n";
                    // assert(arcs[z].front() == ptr);
                    if(arcs[z].front() != ptr) {
                        success = 0;
                        break;
                    }
                    arcs[z].pop_front();
                    graph[z][ptr] = 0;
                }
                if (ptr <= gold.num_words && graph[ptr][z]) {
                    seq.push_back(LEFT_ARC);
                    depth.push_back(stack.size() + 1);
                    // std::cerr<<"want: "<<z<<" "<<ptr<<", got: "<<arcs[z].front()<<"\n";
                    // assert(arcs[z].front() == ptr);
                    if(arcs[z].front() != ptr) {
                        success = 0;
                        break;
                    }
                    arcs[z].pop_front();
                    graph[ptr][z] = 0;
                }
                if (arcs[z].empty()) {
                    seq.push_back(REDUCE);
                    stack.pop_back();
                    depth.push_back(stack.size() + 1);
                    continue;
                }

                if (stack.size() >= 2) {
                    int &y = stack[stack.size() - 2];
                    if (precedes(arcs[y], arcs[z])) {
                        seq.push_back(SWAP);
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
                seq.push_back(SHIFT);
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
        traverse(goldDeductions, seq, depth, 0, seq.size() - 1, -1, -1);

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

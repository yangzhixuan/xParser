//
// Created by 杨至轩 on 9/6/15.
//

#ifndef XPARSER_TITOVDP_STATE_H
#define XPARSER_TITOVDP_STATE_H

#include <include/learning/perceptron/score.h>
#include <unordered_set>
#include <iostream>
#include <memory>

namespace titovdp{

    using std::shared_ptr;

    #define RESET   "\033[0m"
    #define BLACK   "\033[30m"      /* Black */
    #define RED     "\033[31m"      /* Red */
    #define GREEN   "\033[32m"      /* Green */
    #define YELLOW  "\033[33m"      /* Yellow */
    #define BLUE    "\033[34m"      /* Blue */
    #define MAGENTA "\033[35m"      /* Magenta */
    #define CYAN    "\033[36m"      /* Cyan */
    #define WHITE   "\033[37m"      /* White */
    #define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
    #define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
    #define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
    #define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
    #define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
    #define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
    #define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
    #define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

    enum Transition {
        SHIFT,
        LEFT_ARC,
        RIGHT_ARC,
        SWAP,
        REDUCE,
        NUM_TRANSITION_TYPES
    };

    struct PushComputation {
        int i, j, x, y, z;
        int jls, jrs;           // j's left/right-most child
        int yz_swapped;
        int jz_connected;
        int zj_connected;
        int jy_connected;
        int yj_connected;
    };

    struct Deduction {
        PushComputation pc1;
        PushComputation pc2;
        Transition type;
    };

    struct ScoredPushComputation;

    struct ScoreInformation {
        tscore score;
        shared_ptr<const ScoredPushComputation> prev1;
        shared_ptr<const ScoredPushComputation> prev2;
    };

    struct ScoredPushComputation{
        PushComputation pc;
        ScoreInformation attr;
        ScoredPushComputation(const PushComputation & pc, const ScoreInformation & info);
    };

    struct PtrSPCCompare {
        bool operator()(const shared_ptr<const ScoredPushComputation> &p1,
                        const shared_ptr<const ScoredPushComputation> &p2);
    };

    std::ostream& operator<<(std::ostream& out, const PushComputation & pc);
    std::ostream& operator<<(std::ostream& out, const Deduction& pc);
    bool operator==(const PushComputation & pc1, const PushComputation & pc2);
    bool operator==(const Deduction & d1, const Deduction & d2);
    bool operator<(const ScoreInformation & pc1, const ScoreInformation & pc2);
    bool operator<(const ScoredPushComputation & pc1, const ScoredPushComputation & pc2);
    bool operator>(const ScoredPushComputation & pc1, const ScoredPushComputation & pc2);
    typedef BiGram<int> Arc;
    typedef std::unordered_set<Arc> ArcSet;
    typedef std::unordered_map<PushComputation, ScoreInformation> PCHashTable;
    typedef shared_ptr<ScoredPushComputation> SPCPtr;
    typedef shared_ptr<const ScoredPushComputation> cSPCPtr;
    typedef std::unordered_map<PushComputation, SPCPtr> PCHashT;
    typedef std::unordered_set<Deduction> DeductionSet;

    PushComputation reduce_pc(const PushComputation & pc1, const PushComputation & pc2);

    PushComputation singleton_pc(const PushComputation &pc, Transition action);
    PushComputation shift_pc(int x, int i);
}

namespace std {
    template<>
    struct hash<titovdp::Deduction> {
        size_t operator()(const titovdp::Deduction & d) const {
            size_t seed = 0;
            hash_combine(seed, d.pc1);
            hash_combine(seed, (int)d.type);
            if(d.type == titovdp::REDUCE) {
                hash_combine(seed, d.pc2);
            }
            return seed;
        }
    };

    template<>
    struct hash<titovdp::PushComputation> {
        size_t operator()(const titovdp::PushComputation & pc) const {
            size_t seed = 0;
            hash_combine(seed, pc.i);
            hash_combine(seed, pc.j);
            hash_combine(seed, pc.x);
            hash_combine(seed, pc.y);
            hash_combine(seed, pc.z);
          //  hash_combine(seed, pc.jls);
          //  hash_combine(seed, pc.jrs);
            hash_combine(seed, pc.jz_connected);
            hash_combine(seed, pc.zj_connected);
            hash_combine(seed, pc.jy_connected);
            hash_combine(seed, pc.yj_connected);
            hash_combine(seed, pc.yz_swapped);
            return seed;
        }
    };
}

#endif //XPARSER_TITOVDP_STATE_H

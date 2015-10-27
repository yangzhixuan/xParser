//
// Created by 杨至轩 on 9/6/15.
//

#ifndef XPARSER_TITOVDP_STATE_H
#define XPARSER_TITOVDP_STATE_H

#include <include/learning/perceptron/score.h>
#include <unordered_set>
#include <iostream>
#include <memory>

namespace titovdp {

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
        SWAP,
        REDUCE,
        LA_SHIFT,
        LA_SWAP,
        LA_REDUCE,
        RA_SHIFT,
        RA_SWAP,
        RA_REDUCE,
        NUM_TRANSITION_TYPES
    };


    extern Transition shift_actions[];
    extern Transition swap_actions[];
    extern Transition reduce_actions[];

    const int NUM_SHIFT_ACTIONS = 3;
    const int NUM_SWAP_ACTIONS = 3;
    const int NUM_REDUCE_ACTIONS = 3;

    struct PushComputation {
        int i, j, y, z;
        int jls;
        int yls, yrs;
        int zls, zrs;
    };

    struct Deduction {
        PushComputation pc1;
        PushComputation pc2;
        Transition type;
    };

    struct ScoredPushComputation;

    struct ScoreInformation {
        tscore score;
        tscore prefix_score;
        Transition type;
        shared_ptr<const ScoredPushComputation> prev1;
        shared_ptr<const ScoredPushComputation> prev2;
        shared_ptr<std::vector<shared_ptr<const ScoredPushComputation>>> predictor;
    };

    struct ScoredPushComputation {
        PushComputation pc;
        ScoreInformation attr;

        ScoredPushComputation(const PushComputation &pc, const ScoreInformation &info);
    };
    typedef std::vector<shared_ptr<const ScoredPushComputation>> PredictorSet;

    struct PtrSPCCompare {
        bool operator()(const shared_ptr<const ScoredPushComputation> &p1,
                        const shared_ptr<const ScoredPushComputation> &p2);
    };

    struct PCMergeHash {
        size_t operator()(const PushComputation & pc) const;
    };

    struct PCMergeEqual {
        bool operator()(const PushComputation & pc, const PushComputation & pc2) const;
    };

    std::ostream &operator<<(std::ostream &out, const PushComputation &pc);

    std::ostream &operator<<(std::ostream &out, const Deduction &pc);

    bool operator==(const PushComputation &pc1, const PushComputation &pc2);

    bool operator==(const Deduction &d1, const Deduction &d2);

    bool operator<(const ScoreInformation &pc1, const ScoreInformation &pc2);

    bool operator<(const ScoredPushComputation &pc1, const ScoredPushComputation &pc2);

    bool operator>(const ScoredPushComputation &pc1, const ScoredPushComputation &pc2);

    bool is_reduce(Transition t);
    bool is_shift(Transition t);


    typedef BiGram<int> Arc;
    typedef std::unordered_set<Arc> ArcSet;
    typedef std::unordered_map<PushComputation, ScoreInformation> PCHashTable;
    typedef shared_ptr<ScoredPushComputation> SPCPtr;
    typedef shared_ptr<const ScoredPushComputation> cSPCPtr;
    typedef std::unordered_map<PushComputation, SPCPtr, PCMergeHash, PCMergeEqual> PCHashT;
    typedef std::unordered_set<Deduction> DeductionSet;

    PushComputation swap_pc(const PushComputation &pc, Transition action);
    PushComputation shift_pc(const PushComputation &pc, Transition action);
    PushComputation reduce_pc(const PushComputation &pc1, const PushComputation &pc2, Transition action);

    std::string action_str(Transition t);
}

namespace std {
    template<>
    struct hash<titovdp::Deduction> {
        size_t operator()(const titovdp::Deduction &d) const {
            size_t seed = 0;
            hash_combine(seed, d.pc1);
            hash_combine(seed, (int) d.type);
            if (d.type == titovdp::REDUCE
                || d.type == titovdp::LA_REDUCE
                || d.type == titovdp::RA_REDUCE) {
                hash_combine(seed, d.pc2);
            }
            return seed;
        }
    };

    template<>
    struct hash<titovdp::PushComputation> {
        size_t operator()(const titovdp::PushComputation &pc) const {
            size_t seed = 0;
            hash_combine(seed, pc.i);
            hash_combine(seed, pc.j);
            hash_combine(seed, pc.y);
            hash_combine(seed, pc.z);
            hash_combine(seed, pc.jls);
            hash_combine(seed, pc.yls);
            hash_combine(seed, pc.yrs);
            hash_combine(seed, pc.zls);
            hash_combine(seed, pc.zrs);
            return seed;
        }
    };
}

#endif //XPARSER_TITOVDP_STATE_H

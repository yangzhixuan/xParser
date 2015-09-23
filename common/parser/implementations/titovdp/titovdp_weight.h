//
// Created by 杨至轩 on 9/5/15.
//

#ifndef XPARSER_TITOVDP_WEIGHT_H
#define XPARSER_TITOVDP_WEIGHT_H

#include <common/parser/weight_base.h>
#include <common/parser/macros_base.h>
#include <include/learning/perceptron/packed_score.h>
#include "titovdp_state.h"

typedef PackedScoreMap<WordInt> WordIntMap;
typedef PackedScoreMap<POSTagInt> POSTagIntMap;
typedef PackedScoreMap<TwoWordsInt> TwoWordsIntMap;
typedef PackedScoreMap<ThreeWordsInt> ThreeWordsIntMap;
typedef PackedScoreMap<POSTagSet2Int> POSTagSet2IntMap;
typedef PackedScoreMap<WordPOSTagInt> WordPOSTagIntMap;
typedef PackedScoreMap<POSTagSet3Int> POSTagSet3IntMap;
typedef PackedScoreMap<WordWordPOSTagInt> WordWordPOSTagIntMap;
typedef PackedScoreMap<WordPOSTagPOSTagInt> WordPOSTagPOSTagIntMap;
typedef PackedScoreMap<POSTagSet4Int> POSTagSet4IntMap;
typedef PackedScoreMap<WordWordPOSTagPOSTagInt> WordWordPOSTagPOSTagIntMap;

namespace titovdp{
    class Weight : public WeightBase {
    public:
        // Naming convention:
        //   S0: the top word of the stack
        //   S1: the second word of the stack
        //   B0: the next word in the buffer
        //   HS0: the top word of the stack before this computation
        //    w: word
        //    p: POS tag
        //    d: distance between z and j
        //
        WordIntMap S0w[NUM_TRANSITION_TYPES];
        WordIntMap S0wd[NUM_TRANSITION_TYPES];
        POSTagIntMap S0p[NUM_TRANSITION_TYPES];
        POSTagIntMap S0pd[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap S0wp[NUM_TRANSITION_TYPES];

        WordIntMap S1w[NUM_TRANSITION_TYPES];
        POSTagIntMap S1p[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap S1wp[NUM_TRANSITION_TYPES];

        WordIntMap B0w[NUM_TRANSITION_TYPES];
        WordIntMap B0wd[NUM_TRANSITION_TYPES];
        POSTagIntMap B0p[NUM_TRANSITION_TYPES];
        POSTagIntMap B0pd[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap B0wp[NUM_TRANSITION_TYPES];

        WordIntMap HS0w[NUM_TRANSITION_TYPES];
        POSTagIntMap HS0p[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap HS0wp[NUM_TRANSITION_TYPES];

        TwoWordsIntMap S0wB0w[NUM_TRANSITION_TYPES];
        TwoWordsIntMap S0wB0wd[NUM_TRANSITION_TYPES];
        TwoWordsIntMap S1wB0w[NUM_TRANSITION_TYPES];

        WordPOSTagIntMap S0wB0p[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap B0wS0p[NUM_TRANSITION_TYPES];

        WordPOSTagIntMap S1wB0p[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap B0wS1p[NUM_TRANSITION_TYPES];

        POSTagSet2IntMap S0pB0p[NUM_TRANSITION_TYPES];
        POSTagSet2IntMap S0pB0pd[NUM_TRANSITION_TYPES];
        POSTagSet2IntMap S1pB0p[NUM_TRANSITION_TYPES];

        ThreeWordsIntMap S0wS1wB0w[NUM_TRANSITION_TYPES];
    public:
        void loadScores();
        void saveScores() const;
        void computeAverageFeatureWeights(const int & round);

        Weight(const std::string &sInput, const std::string &sRecord);
        ~Weight();
    };
}

#endif //XPARSER_TITOVDP_WEIGHT_H

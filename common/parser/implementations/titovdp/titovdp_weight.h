//
// Created by 杨至轩 on 9/5/15.
//

#ifndef XPARSER_TITOVDP_WEIGHT_H
#define XPARSER_TITOVDP_WEIGHT_H

#include <common/parser/weight_base.h>
#include <common/parser/macros_base.h>
#include <include/learning/perceptron/packed_score.h>
#include "titovdp_state.h"

typedef PackedScoreMap<Word> WordMap;
typedef PackedScoreMap<POSTag> POSTagMap;
typedef PackedScoreMap<TwoWords> TwoWordsMap;
typedef PackedScoreMap<ThreeWords> ThreeWordsMap;
typedef PackedScoreMap<POSTagSet2> POSTagSet2Map;
typedef PackedScoreMap<WordPOSTag> WordPOSTagMap;
typedef PackedScoreMap<POSTagSet3> POSTagSet3Map;
typedef PackedScoreMap<WordWordPOSTag> WordWordPOSTagMap;
typedef PackedScoreMap<WordPOSTagPOSTag> WordPOSTagPOSTagMap;
typedef PackedScoreMap<POSTagSet4> POSTagSet4Map;
typedef PackedScoreMap<WordWordPOSTagPOSTag> WordWordPOSTagPOSTagMap;

typedef PackedScoreMap<WordInt> WordIntMap;
typedef PackedScoreMap<POSTagInt> POSTagIntMap;
typedef PackedScoreMap<TwoWordsInt> TwoWordsIntMap;
typedef PackedScoreMap<POSTagSet2Int> POSTagSet2IntMap;
typedef PackedScoreMap<WordPOSTagInt> WordPOSTagIntMap;

namespace titovdp {
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
        WordMap S0w[NUM_TRANSITION_TYPES];
        WordMap S0wd[NUM_TRANSITION_TYPES];
        POSTagMap S0p[NUM_TRANSITION_TYPES];
        POSTagMap S0pd[NUM_TRANSITION_TYPES];
        WordPOSTagMap S0wp[NUM_TRANSITION_TYPES];

        WordMap S1w[NUM_TRANSITION_TYPES];
        POSTagMap S1p[NUM_TRANSITION_TYPES];
        WordPOSTagMap S1wp[NUM_TRANSITION_TYPES];

        WordMap B0w[NUM_TRANSITION_TYPES];
        WordMap B0wd[NUM_TRANSITION_TYPES];
        POSTagMap B0p[NUM_TRANSITION_TYPES];
        POSTagMap B0pd[NUM_TRANSITION_TYPES];
        WordPOSTagMap B0wp[NUM_TRANSITION_TYPES];

        WordMap HS0w[NUM_TRANSITION_TYPES];
        POSTagMap HS0p[NUM_TRANSITION_TYPES];
        WordPOSTagMap HS0wp[NUM_TRANSITION_TYPES];

        TwoWordsMap S0wB0w[NUM_TRANSITION_TYPES];
        TwoWordsMap S0wB0wd[NUM_TRANSITION_TYPES];
        TwoWordsMap S1wB0w[NUM_TRANSITION_TYPES];

        WordPOSTagMap S0wB0p[NUM_TRANSITION_TYPES];
        WordPOSTagMap B0wS0p[NUM_TRANSITION_TYPES];

        WordPOSTagMap S1wB0p[NUM_TRANSITION_TYPES];
        WordPOSTagMap B0wS1p[NUM_TRANSITION_TYPES];

        POSTagSet2Map S0pB0p[NUM_TRANSITION_TYPES];
        POSTagSet2Map S0pB0pd[NUM_TRANSITION_TYPES];
        POSTagSet2Map S1pB0p[NUM_TRANSITION_TYPES];

        ThreeWordsMap S0wS1wB0w[NUM_TRANSITION_TYPES];

        WordMap B0Lw[NUM_TRANSITION_TYPES];
        POSTagMap B0Lp[NUM_TRANSITION_TYPES];
        WordMap B0Rw[NUM_TRANSITION_TYPES];
        POSTagMap B0Rp[NUM_TRANSITION_TYPES];

        WordMap B1w[NUM_TRANSITION_TYPES];
        POSTagMap B1p[NUM_TRANSITION_TYPES];
        WordPOSTagMap B1wp[NUM_TRANSITION_TYPES];

        WordMap B2w[NUM_TRANSITION_TYPES];
        POSTagMap B2p[NUM_TRANSITION_TYPES];
        WordPOSTagMap B2wp[NUM_TRANSITION_TYPES];

        WordIntMap S0iw[NUM_TRANSITION_TYPES];
        TwoWordsIntMap S0iwS0jw[NUM_TRANSITION_TYPES];
        WordIntMap B0iw[NUM_TRANSITION_TYPES];
        TwoWordsIntMap B0iwB0jw[NUM_TRANSITION_TYPES];

        POSTagIntMap S0ip[NUM_TRANSITION_TYPES];
        POSTagSet2IntMap S0ipS0jp[NUM_TRANSITION_TYPES];
        POSTagIntMap B0ip[NUM_TRANSITION_TYPES];
        POSTagSet2IntMap B0ipB0jp[NUM_TRANSITION_TYPES];

        WordWordPOSTagMap S0wB0wS0p[NUM_TRANSITION_TYPES];
        WordWordPOSTagMap S0wB0wB0p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap S0wS0pB0p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap B0wS0pB0p[NUM_TRANSITION_TYPES];
        WordWordPOSTagPOSTagMap S0wB0wS0pB0p[NUM_TRANSITION_TYPES];

        TwoWordsIntMap S0wB0wD[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap S0wB0pD[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap B0wS0pD[NUM_TRANSITION_TYPES];
        POSTagSet2IntMap S0pB0pD[NUM_TRANSITION_TYPES];

        TwoWordsIntMap S1wB0wD[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap S1wB0pD[NUM_TRANSITION_TYPES];
        WordPOSTagIntMap B0wS1pD[NUM_TRANSITION_TYPES];
        POSTagSet2IntMap S1pB0pD[NUM_TRANSITION_TYPES];

        WordWordPOSTagMap S1wB0wS1p[NUM_TRANSITION_TYPES];
        WordWordPOSTagMap S1wB0wB0p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap S1wS1pB0p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap B0wS1pB0p[NUM_TRANSITION_TYPES];
        WordWordPOSTagPOSTagMap S1wB0wS1pB0p[NUM_TRANSITION_TYPES];


        TwoWordsMap S0wS1w[NUM_TRANSITION_TYPES];
        WordPOSTagMap S0wS1p[NUM_TRANSITION_TYPES];
        WordPOSTagMap S1wS0p[NUM_TRANSITION_TYPES];
        POSTagSet2Map S0pS1p[NUM_TRANSITION_TYPES];
        WordWordPOSTagMap S0wS1wS0p[NUM_TRANSITION_TYPES];
        WordWordPOSTagMap S0wS1wS1p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap S0wS0pS1p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap S1wS0pS1p[NUM_TRANSITION_TYPES];
        WordWordPOSTagPOSTagMap S0wS1wS0pS1p[NUM_TRANSITION_TYPES];

        WordPOSTagPOSTagMap S0wS1pB0p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap S1wS0pB0p[NUM_TRANSITION_TYPES];
        WordPOSTagPOSTagMap B0wS0pS1p[NUM_TRANSITION_TYPES];
        POSTagSet3Map S0pS1pB0p[NUM_TRANSITION_TYPES];
    public:
        void loadScores();

        void saveScores() const;

        void computeAverageFeatureWeights(const int &round);

        Weight(const std::string &sInput, const std::string &sRecord);

        ~Weight();
    };
}

#endif //XPARSER_TITOVDP_WEIGHT_H

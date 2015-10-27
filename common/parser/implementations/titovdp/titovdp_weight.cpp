//
// Created by 杨至轩 on 9/5/15.
//

#include <fstream>
#include <common/token/word.h>
#include <common/token/pos.h>
#include "titovdp_weight.h"

namespace titovdp {
    Weight::Weight(const std::string &sInput, const std::string &sRecord) : WeightBase(sInput, sRecord) {
        loadScores();
        std::cout << "load finished" << std::endl;
    }

    Weight::~Weight() = default;

    void Weight::loadScores() {
        if (m_sReadPath.empty()) {
            std::cout << "empty file. Create new Weight."<<std::endl;
            return;
        }
        std::ifstream input(m_sReadPath);
        if (!input) {
            return;
        }

        std::cout << "reading from: " << m_sReadPath << std::endl;
        input >> TWord::getTokenizer();
        input >> TPOSTag::getTokenizer();

#define READ(feat) \
        for(int i = 0; i < NUM_TRANSITION_TYPES; i++) { \
            input >> feat[i];    \
        }



        READ(S0w);
        READ(S0p);
        READ(S1w);
        READ(S1p);
        READ(B0w);
        READ(B0p);

        // unigram contexts
        // z contexts:
        READ(S0iw);
        READ(S0iw);
        READ(S0iw);
        READ(S0iw);

        READ(S0iwS0jw);
        READ(S0iwS0jw);
        READ(S0iwS0jw);

        // j contexts
        READ(B0iw);
        READ(B0iw);
        READ(B0iw);
        READ(B0iw);

        READ(B0iwB0jw);
        READ(B0iwB0jw);
        READ(B0iwB0jw);

        // z contexts:
        READ(S0ip);
        READ(S0ip);
        READ(S0ip);
        READ(S0ip);

        READ(S0ipS0jp);
        READ(S0ipS0jp);
        READ(S0ipS0jp);

        // j contexts
        READ(B0ip);
        READ(B0ip);
        READ(B0ip);
        READ(B0ip);

        READ(B0ipB0jp);
        READ(B0ipB0jp);
        READ(B0ipB0jp);


        // bigrams
        // z + j
        READ(S0wB0w);
        READ(S0wB0p);
        READ(B0wS0p);
        READ(S0pB0p);
        READ(S0wB0wS0p);
        READ(S0wB0wB0p);
        READ(S0wS0pB0p);
        READ(B0wS0pB0p);
        READ(S0wB0wS0pB0p);

        // z + j + dis
        READ(S0wB0wD);
        READ(S0wB0pD);
        READ(B0wS0pD);
        READ(S0pB0pD);

        // y + j
        READ(S1wB0w);
        READ(S1wB0p);
        READ(B0wS1p);
        READ(S1pB0p);
        READ(S1wB0wS1p);
        READ(S1wB0wB0p);
        READ(S1wS1pB0p);
        READ(B0wS1pB0p);
        READ(S1wB0wS1pB0p);

        // y + j + dis
        READ(S1wB0wD);
        READ(S1wB0pD);
        READ(B0wS1pD);
        READ(S1pB0pD);

        // z + y
        READ(S0wS1w);
        READ(S0wS1p);
        READ(S1wS0p);
        READ(S0pS1p);
        READ(S0wS1wS0p);
        READ(S0wS1wS1p);
        READ(S0wS0pS1p);
        READ(S1wS0pS1p);
        READ(S0wS1wS0pS1p);

        // trigram
        // y + j + z
        READ(S0wS1pB0p);
        READ(S1wS0pB0p);
        READ(B0wS0pS1p);
        READ(S0pS1pB0p);

        input.close();
#undef READ
    }

    void Weight::saveScores() const {
        if (m_sRecordPath.empty()) {
            return;
        }
        std::ofstream output(m_sRecordPath);
        if (!output) {
            return;
        }

        output << TWord::getTokenizer();
        output << TPOSTag::getTokenizer();
#define SAVE(feat) \
        for(int i = 0; i < NUM_TRANSITION_TYPES; i++) { \
            output << feat[i];    \
        }

        SAVE(S0w);
        SAVE(S0p);
        SAVE(S1w);
        SAVE(S1p);
        SAVE(B0w);
        SAVE(B0p);

        // unigram contexts
        // z contexts:
        SAVE(S0iw);
        SAVE(S0iw);
        SAVE(S0iw);
        SAVE(S0iw);

        SAVE(S0iwS0jw);
        SAVE(S0iwS0jw);
        SAVE(S0iwS0jw);

        // j contexts
        SAVE(B0iw);
        SAVE(B0iw);
        SAVE(B0iw);
        SAVE(B0iw);

        SAVE(B0iwB0jw);
        SAVE(B0iwB0jw);
        SAVE(B0iwB0jw);

        // z contexts:
        SAVE(S0ip);
        SAVE(S0ip);
        SAVE(S0ip);
        SAVE(S0ip);

        SAVE(S0ipS0jp);
        SAVE(S0ipS0jp);
        SAVE(S0ipS0jp);

        // j contexts
        SAVE(B0ip);
        SAVE(B0ip);
        SAVE(B0ip);
        SAVE(B0ip);

        SAVE(B0ipB0jp);
        SAVE(B0ipB0jp);
        SAVE(B0ipB0jp);


        // bigrams
        // z + j
        SAVE(S0wB0w);
        SAVE(S0wB0p);
        SAVE(B0wS0p);
        SAVE(S0pB0p);
        SAVE(S0wB0wS0p);
        SAVE(S0wB0wB0p);
        SAVE(S0wS0pB0p);
        SAVE(B0wS0pB0p);
        SAVE(S0wB0wS0pB0p);

        // z + j + dis
        SAVE(S0wB0wD);
        SAVE(S0wB0pD);
        SAVE(B0wS0pD);
        SAVE(S0pB0pD);

        // y + j
        SAVE(S1wB0w);
        SAVE(S1wB0p);
        SAVE(B0wS1p);
        SAVE(S1pB0p);
        SAVE(S1wB0wS1p);
        SAVE(S1wB0wB0p);
        SAVE(S1wS1pB0p);
        SAVE(B0wS1pB0p);
        SAVE(S1wB0wS1pB0p);

        // y + j + dis
        SAVE(S1wB0wD);
        SAVE(S1wB0pD);
        SAVE(B0wS1pD);
        SAVE(S1pB0pD);

        // z + y
        SAVE(S0wS1w);
        SAVE(S0wS1p);
        SAVE(S1wS0p);
        SAVE(S0pS1p);
        SAVE(S0wS1wS0p);
        SAVE(S0wS1wS1p);
        SAVE(S0wS0pS1p);
        SAVE(S1wS0pS1p);
        SAVE(S0wS1wS0pS1p);

        // trigram
        // y + j + z
        SAVE(S0wS1pB0p);
        SAVE(S1wS0pB0p);
        SAVE(B0wS0pS1p);
        SAVE(S0pS1pB0p);

        output.close();
#undef SAVE
    }

    void Weight::computeAverageFeatureWeights(const int &round) {
#define AVG(feat) \
        for(int i = 0; i < NUM_TRANSITION_TYPES; i++) { \
            feat[i].computeAverage(round);    \
        }

        AVG(S0w);
        AVG(S0p);
        AVG(S1w);
        AVG(S1p);
        AVG(B0w);
        AVG(B0p);

        // unigram contexts
        // z contexts:
        AVG(S0iw);
        AVG(S0iw);
        AVG(S0iw);
        AVG(S0iw);

        AVG(S0iwS0jw);
        AVG(S0iwS0jw);
        AVG(S0iwS0jw);

        // j contexts
        AVG(B0iw);
        AVG(B0iw);
        AVG(B0iw);
        AVG(B0iw);

        AVG(B0iwB0jw);
        AVG(B0iwB0jw);
        AVG(B0iwB0jw);

        // z contexts:
        AVG(S0ip);
        AVG(S0ip);
        AVG(S0ip);
        AVG(S0ip);

        AVG(S0ipS0jp);
        AVG(S0ipS0jp);
        AVG(S0ipS0jp);

        // j contexts
        AVG(B0ip);
        AVG(B0ip);
        AVG(B0ip);
        AVG(B0ip);

        AVG(B0ipB0jp);
        AVG(B0ipB0jp);
        AVG(B0ipB0jp);


        // bigrams
        // z + j
        AVG(S0wB0w);
        AVG(S0wB0p);
        AVG(B0wS0p);
        AVG(S0pB0p);
        AVG(S0wB0wS0p);
        AVG(S0wB0wB0p);
        AVG(S0wS0pB0p);
        AVG(B0wS0pB0p);
        AVG(S0wB0wS0pB0p);

        // z + j + dis
        AVG(S0wB0wD);
        AVG(S0wB0pD);
        AVG(B0wS0pD);
        AVG(S0pB0pD);

        // y + j
        AVG(S1wB0w);
        AVG(S1wB0p);
        AVG(B0wS1p);
        AVG(S1pB0p);
        AVG(S1wB0wS1p);
        AVG(S1wB0wB0p);
        AVG(S1wS1pB0p);
        AVG(B0wS1pB0p);
        AVG(S1wB0wS1pB0p);

        // y + j + dis
        AVG(S1wB0wD);
        AVG(S1wB0pD);
        AVG(B0wS1pD);
        AVG(S1pB0pD);

        // z + y
        AVG(S0wS1w);
        AVG(S0wS1p);
        AVG(S1wS0p);
        AVG(S0pS1p);
        AVG(S0wS1wS0p);
        AVG(S0wS1wS1p);
        AVG(S0wS0pS1p);
        AVG(S1wS0pS1p);
        AVG(S0wS1wS0pS1p);

        // trigram
        // y + j + z
        AVG(S0wS1pB0p);
        AVG(S1wS0pB0p);
        AVG(B0wS0pS1p);
        AVG(S0pS1pB0p);

#undef AVG
    }

}
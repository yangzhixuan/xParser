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
        READ(S0wd);
        READ(S0p);
        READ(S0pd);
        READ(S0wp);

        READ(S1w);
        READ(S1p);
        READ(S1wp);

        READ(B0w);
        READ(B0wd);
        READ(B0p);
        READ(B0pd);
        READ(B0wp);

        READ(HS0w);
        READ(HS0p);
        READ(HS0wp);

        READ(S0wB0w);
        READ(S0wB0wd);
        READ(S1wB0w);

        READ(S0wB0p);
        READ(B0wS0p);

        READ(S1wB0p);
        READ(B0wS1p);

        READ(S0pB0p);
        READ(S0pB0pd);
        READ(S1pB0p);

        READ(S0wS1wB0w);

        READ(B0Lw);
        READ(B0Lp);
        READ(B0Rw);
        READ(B0Rp);

        READ(B1w);
        READ(B1p);
        READ(B1wp);

        READ(B2w);
        READ(B2p);
        READ(B2wp);
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
        SAVE(S0wd);
        SAVE(S0p);
        SAVE(S0pd);
        SAVE(S0wp);

        SAVE(S1w);
        SAVE(S1p);
        SAVE(S1wp);

        SAVE(B0w);
        SAVE(B0wd);
        SAVE(B0p);
        SAVE(B0pd);
        SAVE(B0wp);

        SAVE(HS0w);
        SAVE(HS0p);
        SAVE(HS0wp);

        SAVE(S0wB0w);
        SAVE(S0wB0wd);
        SAVE(S1wB0w);

        SAVE(S0wB0p);
        SAVE(B0wS0p);

        SAVE(S1wB0p);
        SAVE(B0wS1p);

        SAVE(S0pB0p);
        SAVE(S0pB0pd);
        SAVE(S1pB0p);

        SAVE(S0wS1wB0w);

        SAVE(B0Lw);
        SAVE(B0Lp);
        SAVE(B0Rw);
        SAVE(B0Rp);

        SAVE(B1w);
        SAVE(B1p);
        SAVE(B1wp);

        SAVE(B2w);
        SAVE(B2p);
        SAVE(B2wp);
        output.close();
#undef SAVE
    }

    void Weight::computeAverageFeatureWeights(const int &round) {
#define AVG(feat) \
        for(int i = 0; i < NUM_TRANSITION_TYPES; i++) { \
            feat[i].computeAverage(round);    \
        }

        AVG(S0w);
        AVG(S0wd);
        AVG(S0p);
        AVG(S0pd);
        AVG(S0wp);

        AVG(S1w);
        AVG(S1p);
        AVG(S1wp);

        AVG(B0w);
        AVG(B0wd);
        AVG(B0p);
        AVG(B0pd);
        AVG(B0wp);

        AVG(HS0w);
        AVG(HS0p);
        AVG(HS0wp);

        AVG(S0wB0w);
        AVG(S0wB0wd);
        AVG(S1wB0w);

        AVG(S0wB0p);
        AVG(B0wS0p);

        AVG(S1wB0p);
        AVG(B0wS1p);

        AVG(S0pB0p);
        AVG(S0pB0pd);
        AVG(S1pB0p);

        AVG(S0wS1wB0w);

        AVG(B0Lw);
        AVG(B0Lp);
        AVG(B0Rw);
        AVG(B0Rp);

        AVG(B1w);
        AVG(B1p);
        AVG(B1wp);

        AVG(B2w);
        AVG(B2p);
        AVG(B2wp);
#undef AVG
    }

}
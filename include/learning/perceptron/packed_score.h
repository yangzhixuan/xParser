#ifndef _PACKED_SCORE_H
#define _PACKED_SCORE_H

#include <string>
#include <unordered_map>

#include "score.h"
#include "common/token/token.h"

template<typename KEY_TYPE>
class PackedScoreMap {
private:
    std::unordered_map<KEY_TYPE, Score> m_mapScores;
    std::string m_sName;
    int m_nCount;

public:
    PackedScoreMap(const std::string &input_name = "") : m_sName(input_name), m_nCount(0) { }

    ~PackedScoreMap() = default;

    bool hasKey(const KEY_TYPE &key) {
        return m_mapScores.find(key) != m_mapScores.end();
    }

    void getScore(tscore &ts, const KEY_TYPE &key, int which) {
        if (m_mapScores.find(key) != m_mapScores.end()) {
            ts += m_mapScores[key][which];
        }
    }

    void updateScore(const KEY_TYPE &key, const int &amount, const int &round) {
        m_mapScores[key].updateCurrent(amount, round);
    }

    void getOrUpdateScore(tscore &ts, const KEY_TYPE &key, const int &which, const int &amount = 0,
                          const int &round = 0) {
        if (amount == 0) {
            if (m_mapScores.find(key) != m_mapScores.end()) {
                ts += m_mapScores[key][which];
            }
        } else {
            updateScore(key, amount, round);
        }
    }

    void computeAverage(const int &round) {
        m_nCount = 0;
        for (auto itr = m_mapScores.begin(); itr != m_mapScores.end(); ++itr) {
            itr->second.updateAverage(round);
            if (!itr->second.zero()) {
                ++m_nCount;
            }
        }
    }

    void clearScore() {
        for (auto itr = m_mapScores.begin(); itr != m_mapScores.end(); ++itr) {
            itr->second.reset();
        }
    }

    friend std::istream &operator>>(std::istream &is, PackedScoreMap<KEY_TYPE> &psm) {
        Score sc;
        int number;
        KEY_TYPE key;
        ttoken token;

        psm.m_mapScores.clear();
        is >> psm.m_sName >> psm.m_nCount;
        number = psm.m_nCount;
        while (number--) {
            is >> key >> token >> sc;
            psm.m_mapScores[key] = sc;
        }
        return is;
    }

    friend std::ostream &operator<<(std::ostream &os, const PackedScoreMap<KEY_TYPE> &psm) {
        os << psm.m_sName << " " << psm.m_nCount << std::endl;
        for (auto itr = psm.m_mapScores.begin(); itr != psm.m_mapScores.end(); ++itr) {
            if (!itr->second.zero()) {
                os << itr->first << " : " << itr->second << std::endl;
            }
        }
        return os;
    }
};

#endif
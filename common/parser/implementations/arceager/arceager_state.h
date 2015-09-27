#ifndef _ARCEAGER_STATE_H
#define _ARCEAGER_STATE_H

#include "arceager_macros.h"

namespace arceager {
    class StateItem {
    public:
        int m_nStackBack;
        int m_nHeadStackBack;
        int m_nNextWord;
        int m_nLastAction;
        tscore score;

        int m_lLabels[MAX_SENTENCE_SIZE];
        int m_lStack[MAX_SENTENCE_SIZE];
        int m_lHeadStack[MAX_SENTENCE_SIZE];
        int m_lHeads[MAX_SENTENCE_SIZE];
        int m_lDepsL[MAX_SENTENCE_SIZE];
        int m_lDepsR[MAX_SENTENCE_SIZE];
        int m_lDepsNumL[MAX_SENTENCE_SIZE];
        int m_lDepsNumR[MAX_SENTENCE_SIZE];
        int m_lSibling[MAX_SENTENCE_SIZE];

        SetOfDepLabels m_lDepTagL[MAX_SENTENCE_SIZE];
        SetOfDepLabels m_lDepTagR[MAX_SENTENCE_SIZE];

    public:
        StateItem();

        ~StateItem();

        void arcLeft(const int &l);

        void arcRight(const int &l);

        void shift();

        void reduce();

        void popRoot();

        void clear();

        void clearNext();

        void move(const int &action);

        void generateTree(const Sentence &sent, DependencyTree &tree);

        bool standardMove(const DependencyTree &tree);

        int followMove(const StateItem &item);
    };
}

#endif
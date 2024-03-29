#ifndef _EMPTY_EISNERGC3RD_STATE_H
#define _EMPTY_EISNERGC3RD_STATE_H

#include "emptyeisnergc3rd_macros.h"
#include "common/parser/agenda.h"
#include "include/learning/perceptron/score.h"

namespace emptyeisnergc3rd {

    class StateItem {
    public:
        int type;
        int left, right;
        std::vector<ScoreWithSplit> jux;
        std::vector<ScoreAgenda> l2r_solid_both, r2l_solid_both;
        std::vector<ScoreAgenda> l2r_empty_outside, r2l_empty_outside;
        std::vector<ScoreAgenda> l2r_empty_inside, r2l_empty_inside;
        std::vector<ScoreAgenda> l2r_solid_outside, r2l_solid_outside;
        std::vector<ScoreWithSplit> l2r, r2l;

    public:
        enum STATE {
            JUX = 1,
            L2R_SOLID_BOTH,
            R2L_SOLID_BOTH,
            L2R_EMPTY_OUTSIDE,
            R2L_EMPTY_OUTSIDE,
            L2R_SOLID_OUTSIDE,
            R2L_SOLID_OUTSIDE,
            L2R_EMPTY_INSIDE,
            R2L_EMPTY_INSIDE,
            L2R,
            R2L
        };

        StateItem();

        ~StateItem();

        void init(const int &l, const int &r, const int &len);

        void updateJUX(const int &grand, const int &split, const tscore &score);

        void updateL2RSolidBoth(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateR2LSolidBoth(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateL2REmptyOutside(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateR2LEmptyOutside(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateL2RSolidOutside(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateR2LSolidOutside(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateL2REmptyInside(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateR2LEmptyInside(const int &grand, const int &split, const int &innersplit, const tscore &score);

        void updateL2R(const int &grand, const int &split, const tscore &score);

        void updateR2L(const int &grand, const int &split, const tscore &score);

        void print(const int &grand);
    };
}

#endif
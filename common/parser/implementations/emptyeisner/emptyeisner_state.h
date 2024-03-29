#ifndef _EMPTY_EISNER_STATE_H
#define _EMPTY_EISNER_STATE_H

#include "emptyeisner_macros.h"
#include "common/parser/agenda.h"
#include "include/learning/perceptron/score.h"

namespace emptyeisner {

    class StateItem {
    public:
        int type;
        int left, right;
        ScoreWithSplit jux;
        ScoreAgenda l2r_solid_both, r2l_solid_both, l2r_empty_outside, r2l_empty_outside, l2r_empty_inside, r2l_empty_inside;
        SolidOutsideAgenda l2r_solid_outside, r2l_solid_outside;
        ScoreWithSplit l2r, r2l;

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

        void init(const int &l, const int &r);

        void updateJUX(const int &split, const tscore &score);

        void updateL2RSolidBoth(const int &split, const int &innersplit, const tscore &score);

        void updateR2LSolidBoth(const int &split, const int &innersplit, const tscore &score);

        void updateL2REmptyOutside(const int &split, const int &innersplit, const tscore &score);

        void updateR2LEmptyOutside(const int &split, const int &innersplit, const tscore &score);

        void updateL2RSolidOutside(const int &split, const int &innersplit, const tscore &score);

        void updateR2LSolidOutside(const int &split, const int &innersplit, const tscore &score);

        void updateL2REmptyInside(const int &split, const int &innersplit, const tscore &score);

        void updateR2LEmptyInside(const int &split, const int &innersplit, const tscore &score);

        void updateL2R(const int &split, const tscore &score);

        void updateR2L(const int &split, const tscore &score);

        void print();
    };
}

#endif
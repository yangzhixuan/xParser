#include "eisner3rd_state.h"

namespace eisner3rd {
    StateItem::StateItem() = default;

    StateItem::~StateItem() = default;

    void StateItem::init(const int &l, const int &r) {
        type = 0;
        left = l;
        right = r;
        jux.reset();
        l2r_solid_both.clear();
        r2l_solid_both.clear();
        l2r.reset();
        r2l.reset();
    }

    void StateItem::updateJUX(const int &split, const tscore &score) {
        if (jux < score) {
            jux.refer(split, score);
        }
    }

    void StateItem::updateL2RSolidBoth(const int &split, const int &innersplit, const tscore &score) {
        l2r_solid_both.insertItem(ScoreWithBiSplit(split, innersplit, score));
    }

    void StateItem::updateR2LSolidBoth(const int &split, const int &innersplit, const tscore &score) {
        r2l_solid_both.insertItem(ScoreWithBiSplit(split, innersplit, score));
    }

    void StateItem::updateL2R(const int &split, const tscore &score) {
        if (l2r < score) {
            l2r.refer(split, score);
        }
    }

    void StateItem::updateR2L(const int &split, const tscore &score) {
        if (r2l < score) {
            r2l.refer(split, score);
        }
    }
}
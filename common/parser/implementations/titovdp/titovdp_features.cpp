//
// Created by 杨至轩 on 9/11/15.
//

#include "titovdp_depparser.h"
#include "titovdp_weight.h"

namespace titovdp {
    const WordPOSTag &DepParser::getWordinStack(int index) {
        // return the word in the sentence (maybe EMPTY)
        if (index == -1) {
            return empty_word;
        } else {
            return sent[index];
        }
    }

    const WordPOSTag &DepParser::getWordinSentence(int index) {
        // return the word in the sentence (maybe START or END)
        if (index < 0) {
            return start_word;
        } else if (index > sentenceLength) {
            return end_word;
        } else {
            return sent[index];
        }
    }

    tscore DepParser::getOrUpdateScore(const PushComputation &pc, Transition t, tscore amount) {
        Weight *weight = (Weight *) m_Weight;
        tscore retval = 0;
        WordInt w;
        POSTagInt p;
        WordPOSTagInt wp;
        TwoWordsInt w2;
        POSTagSet2Int p2;
        ThreeWordsInt w3;

        int zword = getWordinStack(pc.z).first();
        int ztag = getWordinStack(pc.z).second();

        int yword = getWordinStack(pc.y).first();
        int ytag = getWordinStack(pc.y).second();

        int jword = getWordinSentence(pc.j).first();
        int jtag = getWordinSentence(pc.j).second();

        // int xword = getWordinSentence(pc.x).first();
        // int xtag = getWordinSentence(pc.x).second();
        int distance = pc.j - pc.z;

#define GETUPDATE(feat, key) (weight -> feat.getOrUpdateScore(retval, key, m_nScoreIndex, amount, nRound))
        // weight->S0w[t].getOrUpdateScore(retval, w, m_nScoreIndex, amount, nRound);

        w.refer(zword, 0);
        GETUPDATE(S0w[t], w);
        w.refer(zword, distance);
        GETUPDATE(S0wd[t], w);
        p.refer(ztag, 0);
        GETUPDATE(S0p[t], p);
        p.refer(ztag, distance);
        GETUPDATE(S0pd[t], p);
        wp.refer(zword, ztag, 0);
        GETUPDATE(S0wp[t], wp);

        w.refer(yword, 0);
        GETUPDATE(S1w[t], w);
        p.refer(ytag, 0);
        GETUPDATE(S1p[t], p);
        wp.refer(yword, ytag, 0);
        GETUPDATE(S1wp[t], wp);

        w.refer(jword, 0);
        GETUPDATE(B0w[t], w);
        w.refer(jword, distance);
        GETUPDATE(B0wd[t], w);
        p.refer(jtag, 0);
        GETUPDATE(B0p[t], p);
        p.refer(jtag, distance);
        GETUPDATE(B0pd[t], p);
        wp.refer(jword, jtag, 0);
        GETUPDATE(B0wp[t], wp);

        // w.refer(xword, 0); GETUPDATE(HS0w[t], w);
        // p.refer(xtag, 0); GETUPDATE(HS0p[t], w);
        // wp.refer(xword, xtag, 0); GETUPDATE(HS0wp[t], wp);

        w2.refer(zword, jword, 0);
        GETUPDATE(S0wB0w[t], w2);
        w2.refer(zword, jword, distance);
        GETUPDATE(S0wB0wd[t], w2);
        w2.refer(yword, jword, 0);
        GETUPDATE(S1wB0w[t], w2);

        wp.refer(zword, jtag, 0);
        GETUPDATE(S0wB0p[t], wp);
        wp.refer(jword, ztag, 0);
        GETUPDATE(B0wS0p[t], wp);

        wp.refer(yword, jtag, 0);
        GETUPDATE(S1wB0p[t], wp);
        wp.refer(jword, ytag, 0);
        GETUPDATE(B0wS1p[t], wp);

        p2.refer(ztag, jtag, 0);
        GETUPDATE(S0pB0p[t], p2);
        p2.refer(ztag, jtag, distance);
        GETUPDATE(S0pB0pd[t], p2);
        p2.refer(ytag, jtag, 0);
        GETUPDATE(S1pB0p[t], p2);

        w3.refer(zword, yword, jword);
        GETUPDATE(S0wS1wB0w[t], w3);
        return retval;
    }

    // for reduce transition
    tscore DepParser::getOrUpdateScore(const PushComputation &pc1, const PushComputation &pc2, tscore amount) {
        Weight *weight = (Weight *) m_Weight;
        tscore retval = 0;
        WordInt w;
        POSTagInt p;
        WordPOSTagInt wp;
        TwoWords w2;

        retval = getOrUpdateScore(pc2, REDUCE, amount);

        return retval;
    }

#undef GETUPDATE

}

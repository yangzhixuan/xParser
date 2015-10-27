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
        Word w;
        POSTag p;
        WordPOSTag wp;
        TwoWords w2;
        POSTagSet2 p2;
        ThreeWords w3;

        // z
        int zword = getWordinStack(pc.z).first();
        int ztag = getWordinStack(pc.z).second();

        int zlsword = getWordinStack(pc.zls).first();
        int zlstag = getWordinStack(pc.zls).second();

        int zrsword = getWordinStack(pc.zrs).first();
        int zrstag = getWordinStack(pc.zrs).second();

        int zl2word = getWordinSentence(pc.z - 2).first();
        int zl1word = getWordinSentence(pc.z - 1).first();
        int zr1word = getWordinSentence(pc.z + 1).first();
        int zr2word = getWordinSentence(pc.z + 2).first();

        int zl2tag = getWordinSentence(pc.z - 2).second();
        int zl1tag = getWordinSentence(pc.z - 1).second();
        int zr1tag = getWordinSentence(pc.z + 1).second();
        int zr2tag = getWordinSentence(pc.z + 2).second();

        // y
        int yword = getWordinStack(pc.y).first();
        int ytag = getWordinStack(pc.y).second();

        int ylsword = getWordinStack(pc.yls).first();
        int ylstag = getWordinStack(pc.yls).second();

        int yrsword = getWordinStack(pc.yrs).first();
        int yrstag = getWordinStack(pc.yrs).second();

        // j
        int jword = getWordinSentence(pc.j).first();
        int jtag = getWordinSentence(pc.j).second();

        int jlsword = getWordinSentence(pc.jls).first();
        int jlstag = getWordinSentence(pc.jls).second();

        int jl2word = getWordinSentence(pc.j - 2).first();
        int jl1word = getWordinSentence(pc.j - 1).first();
        int jr1word = getWordinSentence(pc.j + 1).first();
        int jr2word = getWordinSentence(pc.j + 2).first();

        int jl2tag = getWordinSentence(pc.j - 2).second();
        int jl1tag = getWordinSentence(pc.j - 1).second();
        int jr1tag = getWordinSentence(pc.j + 1).second();
        int jr2tag = getWordinSentence(pc.j + 2).second();

#define GETUPDATE(feat, key) (weight -> feat.getOrUpdateScore(retval, key, m_nScoreIndex, amount, nRound))
        // weight->S0w[t].getOrUpdateScore(retval, w, m_nScoreIndex, amount, nRound);

        // unigrams
        GETUPDATE(S0w[t], zword);
        GETUPDATE(S0p[t], ztag);
        GETUPDATE(S1w[t], yword);
        GETUPDATE(S1p[t], ytag);
        GETUPDATE(B0w[t], jword);
        GETUPDATE(B0p[t], jtag);

        // unigram contexts
        // z contexts:
        GETUPDATE(S0iw[t], WordInt(zl2word, -2));
        GETUPDATE(S0iw[t], WordInt(zl1word, -1));
        GETUPDATE(S0iw[t], WordInt(zr1word, 1));
        GETUPDATE(S0iw[t], WordInt(zr2word, 2));

        GETUPDATE(S0iwS0jw[t], TwoWordsInt(zl2word, zl1word, -1));
        GETUPDATE(S0iwS0jw[t], TwoWordsInt(zl1word, zr1word, 0));
        GETUPDATE(S0iwS0jw[t], TwoWordsInt(zr2word, zr1word, 1));

        // j contexts
        GETUPDATE(B0iw[t], WordInt(jl2word, -2));
        GETUPDATE(B0iw[t], WordInt(jl1word, -1));
        GETUPDATE(B0iw[t], WordInt(jr1word, 1));
        GETUPDATE(B0iw[t], WordInt(jr2word, 2));

        GETUPDATE(B0iwB0jw[t], TwoWordsInt(jl2word, jl1word, -1));
        GETUPDATE(B0iwB0jw[t], TwoWordsInt(jl1word, jr1word, 0));
        GETUPDATE(B0iwB0jw[t], TwoWordsInt(jr2word, jr1word, 1));

        // z contexts:
        GETUPDATE(S0ip[t], POSTagInt(zl2tag, -2));
        GETUPDATE(S0ip[t], POSTagInt(zl1tag, -1));
        GETUPDATE(S0ip[t], POSTagInt(zr1tag, 1));
        GETUPDATE(S0ip[t], POSTagInt(zr2tag, 2));

        GETUPDATE(S0ipS0jp[t], POSTagSet2Int(zl2tag, zl1tag, -1));
        GETUPDATE(S0ipS0jp[t], POSTagSet2Int(zl1tag, zr1tag, 0));
        GETUPDATE(S0ipS0jp[t], POSTagSet2Int(zr2tag, zr1tag, 1));

        // j contexts
        GETUPDATE(B0ip[t], POSTagInt(jl2tag, -2));
        GETUPDATE(B0ip[t], POSTagInt(jl1tag, -1));
        GETUPDATE(B0ip[t], POSTagInt(jr1tag, 1));
        GETUPDATE(B0ip[t], POSTagInt(jr2tag, 2));

        GETUPDATE(B0ipB0jp[t], POSTagSet2Int(jl2tag, jl1tag, -1));
        GETUPDATE(B0ipB0jp[t], POSTagSet2Int(jl1tag, jr1tag, 0));
        GETUPDATE(B0ipB0jp[t], POSTagSet2Int(jr2tag, jr1tag, 1));


        // bigrams
        // z + j
        GETUPDATE(S0wB0w[t], TwoWords(zword, jword));
        GETUPDATE(S0wB0p[t], WordPOSTag(zword, jtag));
        GETUPDATE(B0wS0p[t], WordPOSTag(jword, ztag));
        GETUPDATE(S0pB0p[t], POSTagSet2(ztag, jtag));
        GETUPDATE(S0wB0wS0p[t], WordWordPOSTag(zword, jword, ztag));
        GETUPDATE(S0wB0wB0p[t], WordWordPOSTag(zword, jword, jtag));
        GETUPDATE(S0wS0pB0p[t], WordPOSTagPOSTag(zword, ztag, jtag));
        GETUPDATE(B0wS0pB0p[t], WordPOSTagPOSTag(zword, ztag, jtag));
        GETUPDATE(S0wB0wS0pB0p[t], WordWordPOSTagPOSTag(zword, jword, ztag, jtag));

        // z + j + dis
        int dis = encodeDistance(pc.z, pc.j);
        GETUPDATE(S0wB0wD[t], TwoWordsInt(zword, jword, dis));
        GETUPDATE(S0wB0pD[t], WordPOSTagInt(zword, jtag, dis));
        GETUPDATE(B0wS0pD[t], WordPOSTagInt(jword, ztag, dis));
        GETUPDATE(S0pB0pD[t], POSTagSet2Int(ztag, jtag, dis));

        // y + j
        GETUPDATE(S1wB0w[t], TwoWords(yword, jword));
        GETUPDATE(S1wB0p[t], WordPOSTag(yword, jtag));
        GETUPDATE(B0wS1p[t], WordPOSTag(jword, ytag));
        GETUPDATE(S1pB0p[t], POSTagSet2(ytag, jtag));
        GETUPDATE(S1wB0wS1p[t], WordWordPOSTag(yword, jword, ytag));
        GETUPDATE(S1wB0wB0p[t], WordWordPOSTag(yword, jword, jtag));
        GETUPDATE(S1wS1pB0p[t], WordPOSTagPOSTag(yword, ytag, jtag));
        GETUPDATE(B0wS1pB0p[t], WordPOSTagPOSTag(yword, ytag, jtag));
        GETUPDATE(S1wB0wS1pB0p[t], WordWordPOSTagPOSTag(yword, jword, ytag, jtag));

        // y + j + dis
        dis = encodeDistance(pc.y, pc.j);
        GETUPDATE(S1wB0wD[t], TwoWordsInt(yword, jword, dis));
        GETUPDATE(S1wB0pD[t], WordPOSTagInt(yword, jtag, dis));
        GETUPDATE(B0wS1pD[t], WordPOSTagInt(jword, ytag, dis));
        GETUPDATE(S1pB0pD[t], POSTagSet2Int(ytag, jtag, dis));

        // z + y
        GETUPDATE(S0wS1w[t], TwoWords(zword, yword));
        GETUPDATE(S0wS1p[t], WordPOSTag(zword, ytag));
        GETUPDATE(S1wS0p[t], WordPOSTag(yword, ztag));
        GETUPDATE(S0pS1p[t], POSTagSet2(ztag, ytag));
        GETUPDATE(S0wS1wS0p[t], WordWordPOSTag(zword, yword, ztag));
        GETUPDATE(S0wS1wS1p[t], WordWordPOSTag(zword, yword, ytag));
        GETUPDATE(S0wS0pS1p[t], WordPOSTagPOSTag(zword, ytag, ztag));
        GETUPDATE(S1wS0pS1p[t], WordPOSTagPOSTag(yword, ytag, ztag));
        GETUPDATE(S0wS1wS0pS1p[t], WordWordPOSTagPOSTag(zword, yword, ztag, ytag));

        // trigram
        // y + j + z
        GETUPDATE(S0wS1pB0p[t], WordPOSTagPOSTag(zword, ytag, jtag));
        GETUPDATE(S1wS0pB0p[t], WordPOSTagPOSTag(yword, jtag, ztag));
        GETUPDATE(B0wS0pS1p[t], WordPOSTagPOSTag(jword, ztag, ytag));
        GETUPDATE(S0pS1pB0p[t], POSTagSet3(ztag, jtag, ytag));

        return retval;
    }

    // for reduce transition
    tscore DepParser::getOrUpdateScore(const PushComputation &pc1, const PushComputation &pc2, Transition act, tscore amount) {
        Weight *weight = (Weight *) m_Weight;
        tscore retval = 0;
        WordInt w;
        POSTagInt p;
        WordPOSTagInt wp;
        TwoWords w2;

        retval = getOrUpdateScore(pc2, act, amount);

        return retval;
    }

#undef GETUPDATE

}

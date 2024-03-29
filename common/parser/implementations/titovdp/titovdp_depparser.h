//
// Created by 杨至轩 on 9/5/15.
//

#ifndef XPARSER_TITOVDP_DEPPARSER_H
#define XPARSER_TITOVDP_DEPPARSER_H

#include "common/parser/macros_base.h"
#include "common/parser/depparser_base.h"
#include "titovdp_state.h"
#include "common/parser/agenda.h"

namespace titovdp {
    const int BEAM_SIZE = 16;
    const int OUTPUT_STEP = 50;

    class DepParser : public DepParserBase {
    public:
        DepParser(const std::string &sFeatureInput, const std::string &sFeatureOutput, const ParserState &state);

        void train(const DependencyGraph &ref_sent);

        void parse(const Sentence &sent, DependencyGraph &retval);

        void decode() override;

        void decodeArcs() override;

        void decodeTransitions();

    private:
        PCHashT candidates[MAX_SENTENCE_SIZE];
        WordPOSTag sent[MAX_SENTENCE_SIZE];
        WordPOSTag empty_word, start_word, end_word;
        int sentenceLength;

        DeductionSet decodedDeductions, goldDeductions;
        int numGoldDecutions, numDecodedDeductions, numCorrectDeductions;

        ArcSet decodedArcs;
        int numDecodedArcs, numGoldArcs, numCorrectArcs;

        AgendaBeam<SPCPtr, BEAM_SIZE, PtrSPCCompare> chart_mem[MAX_SENTENCE_SIZE + 5][MAX_SENTENCE_SIZE + 5];
        AgendaBeam<SPCPtr, BEAM_SIZE, PtrSPCCompare> (*chart)[MAX_SENTENCE_SIZE];

        int nRound;

        void addItem(PCHashT &candidates, const PushComputation &pc, const ScoreInformation &score_info);
        void updateItem(PCHashT &candidates, const PushComputation &pc, const ScoreInformation &score_info);

        void gatherBeam(const PCHashT &candidates, int i, int j);

        tscore getOrUpdateScore(const PushComputation &pc, Transition t, tscore amount);

        tscore getOrUpdateScore(const PushComputation &pc1, const PushComputation &pc2, Transition t, tscore amount);

        const WordPOSTag &getWordinStack(int index);

        const WordPOSTag &getWordinSentence(int index);

        bool getGoldTransitions(const DependencyGraph &gold);

        void traversePC(cSPCPtr pc);
        bool actionApplicable(const PushComputation & pc, Transition t);

    };
}


#endif //XPARSER_TITOVDP_DEPPARSER_H

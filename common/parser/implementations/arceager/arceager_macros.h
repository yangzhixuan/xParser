#ifndef _ARCEAGER_MACROS_H
#define _ARCEAGER_MACROS_H

#include "common/parser/agenda.h"
#include "common/parser/macros_base.h"
#include "include/learning/perceptron/packed_score.h"

namespace arceager {
#define OUTPUT_STEP 1000

#define AGENDA_SIZE 64

#define GOLD_POS_SCORE 10
#define GOLD_NEG_SCORE -50

	enum Action {
		NO_ACTION = 0,
		ARC_LEFT,
		ARC_RIGHT,
		SHIFT,
		REDUCE,
		POP_ROOT,
	};

	static int AL_FIRST = POP_ROOT + 1;
	static int AR_FIRST;

#define ENCODE_ACTION(A,L)	((A) == ARC_LEFT ? AL_FIRST + (L) - 1 : AR_FIRST + (L) - 1)
#define DECODE_ACTION(A)	((A) >= AR_FIRST ? ARC_RIGHT : (A) >= AL_FIRST ? ARC_LEFT : (A))
#define DECODE_LABEL(A)		((A) >= AR_FIRST ? (A) - AR_FIRST + 1 : (A) - AL_FIRST + 1)

	typedef SetOfLabels<int, int> SetOfDepLabels;

	typedef PackedScoreMap<Int> IntMap;
	typedef PackedScoreMap<Word> WordMap;
	typedef PackedScoreMap<POSTag> POSTagMap;
	typedef PackedScoreMap<TwoWords> TwoWordsMap;
	typedef PackedScoreMap<WordPOSTag> POSTaggedWordMap;
	typedef PackedScoreMap<WordWordPOSTag> WordWordPOSTagMap;
	typedef PackedScoreMap<WordPOSTagPOSTag> WordPOSTagPOSTagMap;
	typedef PackedScoreMap<WordWordPOSTagPOSTag> TwoPOSTaggedWordsMap;

	typedef PackedScoreMap<POSTagSet2> POSTagSet2Map;
	typedef PackedScoreMap<POSTagSet3> POSTagSet3Map;

	typedef PackedScoreMap<WordSetOfDepLabels> WordSetOfDepLabelsMap;
	typedef PackedScoreMap<POSTagSetOfDepLabels> POSTagSetOfDepLabelsMap;

	typedef PackedScoreMap<WordInt> WordIntMap;
	typedef PackedScoreMap<POSTagInt> POSTagIntMap;

	typedef PackedScoreMap<TwoWordsInt> WordWordIntMap;
	typedef PackedScoreMap<POSTagSet2Int> POSTagPOSTagIntMap;

	//typedef AgendaBeam<StateItem, AGENDA_SIZE> StateAgenda;
	//typedef AgendaBeam<ScoredAction, AGENDA_SIZE> ScoreAgenda;
}

#endif
//#ifndef _ARCEAGER_DEPPARSER_H
//#define _ARCEAGER_DEPPARSER_H
//
//#include <vector>
//#include <unordered_set>
//
//#include "arceager_state.h"
//#include "arceager_weight.h"
//#include "common/parser/depparser_base.h"
//
//namespace arceager {
//	class DepParser : public DepParserBase {
//	private:
//		StateAgenda m_abGenerator;
//		StateAgenda m_abGenerated;
//		ScoreAgenda m_abScores;
//
//		DependencyTree m_lCache;
//
//		int m_nTrainingRound;
//		int m_nTotalErrors;
//		int m_nScoreIndex;
//
//		StateItem itemForState;
//		StateItem itemForStates;
//
//		StateItem pCandidate;
//		StateItem correctState;
//		PackedScoreType packed_scores;
//
//		private TwoStringsVector trainSentence;
//
//		private TwoPOSTaggedWords st_word_tag_n0_word_tag;
//		private TwoWords st_word_n0_word;
//
//		private WordInt word_int;
//		private POSTagInt tag_int;
//		private WordPOSTagPOSTag word_tag_tag;
//		private WordWordPOSTag word_word_tag;
//		private WordWordInt word_word_int;
//		private POSTagPOSTagInt tag_tag_int;
//		private WordSetOfDepLabels word_tagset;
//		private POSTagSetOfDepLabels tag_tagset;
//		private POSTagSet2 set_of_2_tags;
//		private POSTagSet3 set_of_3_tags;
//
//		private ScoredAction scoredaction;
//
//		void update();
//		void generate(DependencyTree * retval, const DependencyTree & correct);
//		void goldCheck();
//
//		tscore arcScore(const int & p, const int & c);
//		void initFirstOrderScore(const int & d);
//
//		void getOrUpdateStackScore(const int & p, const int & c, const int & amount = 0);
//
//	public:
//		DepParser(const std::string & sFeatureInput, const std::string & sFeatureOut, int nState);
//		~DepParser();
//
//		void decode() override;
//		void decodeArcs() override;
//
//		void train(const DependencyTree & correct, const int & round);
//		void parse(const Sentence & sentence, DependencyTree * retval);
//		void work(DependencyTree * retval, const DependencyTree & correct);
//	};
//}
//
//#endif
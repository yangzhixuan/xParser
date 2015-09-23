#ifndef _EISNERGC3RD_DEPPARSER_H
#define _EISNERGC3RD_DEPPARSER_H

#include <vector>
#include <unordered_set>

#include "eisnergc3rd_state.h"
#include "eisnergc3rd_weight.h"
#include "common/parser/depparser_base.h"

namespace eisnergc3rd {

	class DepParser : public DepParserBase {
	private:
		static WordPOSTag empty_taggedword;
		static WordPOSTag start_taggedword;
		static WordPOSTag end_taggedword;

		std::vector<StateItem> m_lItems[MAX_SENTENCE_SIZE];
		WordPOSTag m_lSentence[MAX_SENTENCE_SIZE];
		std::vector<Arc> m_vecCorrectArcs;
		std::vector<QuarArc> m_vecCorrectQuarArcs;
		std::vector<Arc> m_vecTrainArcs;
		std::vector<QuarArc> m_vecTrainQuarArcs;
		int m_nSentenceLength;

		tscore m_nRetval;
		tscore m_lArcScore[MAX_SENTENCE_SIZE << 1];
		tscore m_lBiSiblingArcScore[(MAX_SENTENCE_SIZE << MAX_SENTENCE_BITS) << 1];
		tscore m_lGrandSiblingArcScore[MAX_SENTENCE_SIZE << 1];
		tscore m_lGrandBiSiblingArcScore[MAX_SENTENCE_SIZE << 1];

		int m_nDis, m_nDir;

		Word g_word, p_word, c_word, c2_word, c3_word;
		POSTag g_tag, p_tag, c_tag, c2_tag, c3_tag;

		POSTag p_1_tag, p1_tag, c_1_tag, c1_tag, b_tag;

		WordInt word_int;
		POSTagInt tag_int;

		TwoWordsInt word_word_int;
		POSTagSet2Int tag_tag_int;
		WordPOSTagInt word_tag_int;

		POSTagSet3Int tag_tag_tag_int;
		WordPOSTagPOSTagInt word_tag_tag_int;
		WordWordPOSTagInt word_word_tag_int;

		POSTagSet4Int tag_tag_tag_tag_int;
		WordWordPOSTagPOSTagInt word_word_tag_tag_int;
		WordPOSTagPOSTagPOSTagInt word_tag_tag_tag_int;

		POSTagSet5Int tag_tag_tag_tag_tag_int;
		WordPOSTagPOSTagPOSTagPOSTagInt word_tag_tag_tag_tag_int;

		std::unordered_set<BiGram<int>> m_setArcGoldScore;
		std::unordered_set<TriGram<int>> m_setBiSiblingArcGoldScore;
		std::unordered_set<QuarGram<int>> m_setTriSiblingArcGoldScore;
		std::unordered_set<TriGram<int>> m_setGrandSiblingArcGoldScore;
		std::unordered_set<QuarGram<int>> m_setGrandBiSiblingArcGoldScore;
		std::unordered_set<QuinGram<int>> m_setGrandTriSiblingArcGoldScore;

		void update();
		void generate(DependencyTree * retval, const DependencyTree & correct);
		void goldCheck();

		const tscore & arcScore(const int & p, const int & c);
		const tscore & biSiblingArcScore(const int & p, const int & c, const int & c2);
		const tscore & triSiblingArcScore(const int & p, const int & c, const int & c2, const int & c3);
		const tscore & grandSiblingArcScore(const int & g, const int & p, const int & c);
		const tscore & grandBiSiblingArcScore(const int & g, const int & p, const int & c, const int & c2);
		const tscore & grandTriSiblingArcScore(const int & g, const int & p, const int & c, const int & c2, const int & c3);
		void initArcScore(const int & d);
		void initBiSiblingArcScore(const int & d);
		void initGrandSiblingArcScore(const int & d, const int & i);
		void initGrandBiSiblingArcScore(const int & d, const int & i, const int & k);

		void getOrUpdateGrandScore(const int & g, const int & p, const int & c, const int & amount);
		void getOrUpdateGrandScore(const int & g, const int & p, const int & c, const int & c2, const int & amount);
		void getOrUpdateGrandScore(const int & g, const int & p, const int & c, const int & c2, const int & c3, const int & amount);
		void getOrUpdateSiblingScore(const int & p, const int & c, const int & amount);
		void getOrUpdateSiblingScore(const int & p, const int & c, const int & c2, const int & amount);
		void getOrUpdateSiblingScore(const int & p, const int & c, const int & c2, const int & c3, const int & amount);

	public:
		DepParser(const std::string & sFeatureInput, const std::string & sFeatureOut, int nState);
		~DepParser();

		void decode() override;
		void decodeArcs() override;

		void train(const DependencyTree & correct, const int & round);
		void parse(const Sentence & sentence, DependencyTree * retval);
		void work(DependencyTree * retval, const DependencyTree & correct);
	};
}

#endif
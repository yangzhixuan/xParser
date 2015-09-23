#include <cmath>
#include <stack>
#include <algorithm>
#include <unordered_set>

#include "eisnergc3rd_depparser.h"
#include "common/token/word.h"
#include "common/token/pos.h"

namespace eisnergc3rd {

	WordPOSTag DepParser::empty_taggedword = WordPOSTag();
	WordPOSTag DepParser::start_taggedword = WordPOSTag();
	WordPOSTag DepParser::end_taggedword = WordPOSTag();

	DepParser::DepParser(const std::string & sFeatureInput, const std::string & sFeatureOut, int nState) :
		DepParserBase(sFeatureInput, nState) {

		m_nSentenceLength = 0;

		m_Weight = new Weight(sFeatureInput, sFeatureOut);

		for (int i = 0; i < MAX_SENTENCE_SIZE; ++i) {
			m_lItems[1].push_back(StateItem());
			m_lItems[1][i].init(i, i, MAX_SENTENCE_SIZE);
		}

		DepParser::empty_taggedword.refer(TWord::code(EMPTY_WORD), TPOSTag::code(EMPTY_POSTAG));
		DepParser::start_taggedword.refer(TWord::code(START_WORD), TPOSTag::code(START_POSTAG));
		DepParser::end_taggedword.refer(TWord::code(END_WORD), TPOSTag::code(END_POSTAG));
	}

	DepParser::~DepParser() {
		delete m_Weight;
	}

	void DepParser::train(const DependencyTree & correct, const int & round) {
		// initialize
		int idx = 0;
		m_vecCorrectArcs.clear();
		m_nTrainingRound = round;
		m_nSentenceLength = correct.size();
		// for normal sentence
		for (const auto & node : correct) {
			m_lSentence[idx].refer(TWord::code(std::get<0>(std::get<0>(node))), TPOSTag::code(std::get<1>(std::get<0>(node))));
			m_vecCorrectArcs.push_back(Arc(std::get<1>(node), idx++));
		}
		m_lSentence[idx].refer(TWord::code(ROOT_WORD), TPOSTag::code(ROOT_POSTAG));
		Arcs2QuarArcs(m_vecCorrectArcs, m_vecCorrectQuarArcs);

		if (m_nState == ParserState::GOLDTEST) {
			m_setArcGoldScore.clear();
			m_setBiSiblingArcGoldScore.clear();
			m_setTriSiblingArcGoldScore.clear();
			m_setGrandSiblingArcGoldScore.clear();
			m_setGrandBiSiblingArcGoldScore.clear();
			m_setGrandTriSiblingArcGoldScore.clear();
			for (const auto & quararc : m_vecCorrectQuarArcs) {
				m_setArcGoldScore.insert(BiGram<int>(quararc.second(), quararc.fifth()));
				m_setBiSiblingArcGoldScore.insert(TriGram<int>(quararc.second(), quararc.forth(), quararc.fifth()));
				m_setTriSiblingArcGoldScore.insert(QuarGram<int>(quararc.second(), quararc.third(), quararc.forth(), quararc.fifth()));
				if (quararc.first() != -1) {
					m_setGrandSiblingArcGoldScore.insert(TriGram<int>(quararc.first(), quararc.second(), quararc.fifth()));
					m_setGrandBiSiblingArcGoldScore.insert(QuarGram<int>(quararc.first(), quararc.second(), quararc.forth(), quararc.fifth()));
					m_setGrandTriSiblingArcGoldScore.insert(QuinGram<int>(quararc.first(), quararc.second(), quararc.third(), quararc.forth(), quararc.fifth()));
				}
			}
		}

		work(nullptr, correct);
		if (m_nTrainingRound % OUTPUT_STEP == 0) {
			std::cout << m_nTotalErrors << " / " << m_nTrainingRound << std::endl;
		}
	}

	void DepParser::parse(const Sentence & sentence, DependencyTree * retval) {
		int idx = 0;
		m_nTrainingRound = 0;
		DependencyTree correct;
		m_nSentenceLength = sentence.size();
		for (const auto & token : sentence) {
			m_lSentence[idx++].refer(TWord::code(std::get<0>(token)), TPOSTag::code(std::get<1>(token)));
			correct.push_back(DependencyTreeNode(token, -1, "NMOD"));
		}
		m_lSentence[idx].refer(TWord::code(ROOT_WORD), TPOSTag::code(ROOT_POSTAG));
		work(retval, correct);
	}

	void DepParser::work(DependencyTree * retval, const DependencyTree & correct) {

		decode();

		decodeArcs();

		switch (m_nState) {
		case ParserState::TRAIN:
			update();
			break;
		case ParserState::PARSE:
			generate(retval, correct);
			break;
		case ParserState::GOLDTEST:
			goldCheck();
			break;
		default:
			break;
		}

		for (int d = 2; d <= m_nSentenceLength + 1; ++d) {
			m_lItems[d].clear();
		}
	}

	void DepParser::decode() {

		for (int d = 2; d <= m_nSentenceLength; ++d) {

			initArcScore(d);
			initBiSiblingArcScore(d);

			for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {

				m_lItems[d].push_back(StateItem());

				int l = i + d - 1;
				StateItem & item = m_lItems[d][i];
				const tscore & l2r_arc_score = m_lArcScore[ENCODE_L2R(i)];
				const tscore & r2l_arc_score = m_lArcScore[ENCODE_R2L(i)];

				// initialize
				item.init(i, l, m_nSentenceLength);
				initGrandSiblingArcScore(d, i);

				// jux
				for (int s = i; s < l; ++s) {
					int g = 0;
					do {
						if (g >= i && g <= l) {
							continue;
						}
						item.updateJUX(GRAND_INDEX(g, i, d), s, m_lItems[s - i + 1][i].l2r[GRAND_INDEX(g, i, s - i + 1)].getScore() + m_lItems[l - s][s + 1].r2l[GRAND_INDEX(g, s + 1, l - s)].getScore());
					} while (++g <= m_nSentenceLength);
				}

				for (int k = i + 1; k < l; ++k) {

					int lgrand = GRAND_INDEX(l, i, k - i + 1);
					int rgrand = GRAND_INDEX(i, k, l - k + 1);

					StateItem & litem = m_lItems[k - i + 1][i];
					StateItem & ritem = m_lItems[l - k + 1][k];

					tscore l_base_jux = ritem.jux[rgrand].getScore() + l2r_arc_score + m_lBiSiblingArcScore[ENCODE_2ND_L2R(i, k)];
					tscore r_base_jux = litem.jux[lgrand].getScore() + r2l_arc_score + m_lBiSiblingArcScore[ENCODE_2ND_R2L(i, k)];

					initGrandBiSiblingArcScore(d, i, k);

					int g = 0;
					do {
						if (g >= i && g <= l) {
							continue;
						}

						int grand = GRAND_INDEX(g, i, d);

						const auto & l_solid_both_beam = litem.l2r_solid_both[GRAND_INDEX(g, i, k - i + 1)];
						const auto & r_solid_both_beam = ritem.r2l_solid_both[GRAND_INDEX(g, k, l - k + 1)];

						tscore l_base_score = l_base_jux + m_lGrandSiblingArcScore[ENCODE_L2R(g)] + m_lGrandBiSiblingArcScore[ENCODE_L2R(g)];

						for (const auto & sws : l_solid_both_beam) {
							const int & j = sws.getSplit();
							item.updateL2RSolidBoth(grand, k, j, l_base_score +
																sws.getScore() +
																triSiblingArcScore(i, j == i ? -1 : j, k, l) +
																grandTriSiblingArcScore(g, i, j == i ? -1 : j , k, l));
						}

						tscore r_base_score = r_base_jux + m_lGrandSiblingArcScore[ENCODE_R2L(g)] + m_lGrandBiSiblingArcScore[ENCODE_R2L(g)];

						for (const auto & sws : r_solid_both_beam) {
							const int & j = sws.getSplit();
							item.updateR2LSolidBoth(grand, k, j, r_base_score +
																sws.getScore() +
																triSiblingArcScore(l, j == l ? -1 : j, k, i) +
																grandTriSiblingArcScore(g, l, j == l ? -1 : j, k, i));
						}

						// complete
						item.updateL2R(grand, k, litem.l2r_solid_both[GRAND_INDEX(g, i, k - i + 1)].bestItem().getScore() + ritem.l2r[rgrand].getScore());
						item.updateR2L(grand, k, ritem.r2l_solid_both[GRAND_INDEX(g, k, l - k + 1)].bestItem().getScore() + litem.r2l[lgrand].getScore());

					} while (++g <= m_nSentenceLength);
				}
				// solid both
				int g = 0;
				do {
					if (g >= i && g <= l) {
						continue;
					}
					int grand = GRAND_INDEX(g, i, d);

					item.updateL2RSolidBoth(grand, i, i, m_lItems[d - 1][i + 1].r2l[GRAND_INDEX(i, i + 1, d - 1)].getScore() +
						l2r_arc_score +
						m_lBiSiblingArcScore[ENCODE_2ND_L2R(i, i)] +
						triSiblingArcScore(i, -1, -1, l) +
						m_lGrandSiblingArcScore[ENCODE_L2R(g)] +
						grandBiSiblingArcScore(g, i, -1, l) +
						grandTriSiblingArcScore(g, i, -1, -1, l));
					item.updateR2LSolidBoth(grand, l, l, m_lItems[d - 1][i].l2r[GRAND_INDEX(l, i, d - 1)].getScore() +
						r2l_arc_score +
						m_lBiSiblingArcScore[ENCODE_2ND_R2L(i, i)] +
						triSiblingArcScore(l, -1, -1, i) +
						m_lGrandSiblingArcScore[ENCODE_R2L(g)] +
						grandBiSiblingArcScore(g, l, -1, i) +
						grandTriSiblingArcScore(g, l, -1, -1, i));

					// complete
					item.updateL2R(grand, l, item.l2r_solid_both[grand].bestItem().getScore());
					item.updateR2L(grand, i, item.r2l_solid_both[grand].bestItem().getScore());
				} while (++g <= m_nSentenceLength);
			}
		}
		// best root
		m_lItems[m_nSentenceLength + 1].push_back(StateItem());
		StateItem & item = m_lItems[m_nSentenceLength + 1][0];
		for (int i = 0; i <= m_nSentenceLength; ++i) {
			item.r2l.push_back(ScoreWithSplit());
		}
		for (int m = 0; m < m_nSentenceLength; ++m) {
			item.updateR2L(m_nSentenceLength, m, m_lItems[m + 1][0].r2l[GRAND_INDEX(m_nSentenceLength, 0, m + 1)].getScore() +
				m_lItems[m_nSentenceLength - m][m].l2r[GRAND_INDEX(m_nSentenceLength, m, m_nSentenceLength - m)].getScore() +
				arcScore(m_nSentenceLength, m));
		}
	}

	void DepParser::decodeArcs() {

		m_vecTrainArcs.clear();
		std::stack<std::tuple<int, int, int, int>> stack;

		int s = m_lItems[m_nSentenceLength + 1][0].r2l[m_nSentenceLength].getSplit();
		m_vecTrainArcs.push_back(Arc(-1, s));

		m_lItems[s + 1][0].type = StateItem::R2L;
		stack.push(std::tuple<int, int, int, int>(m_nSentenceLength, s + 1, -1, 0));
		m_lItems[m_nSentenceLength - s][s].type = StateItem::L2R;
		stack.push(std::tuple<int, int, int, int>(m_nSentenceLength, m_nSentenceLength - s, -1, s));

		while (!stack.empty()) {

			auto span = stack.top();
			stack.pop();

			int is = -1;
			int g = std::get<0>(span), s = std::get<2>(span);
			StateItem & item = m_lItems[std::get<1>(span)][std::get<3>(span)];
			int gid = GRAND_INDEX(g, item.left, item.right - item.left + 1);

			if (item.left == item.right) {
				continue;
			}
			switch (item.type) {
			case StateItem::JUX:
				if (item.left < item.right - 1) {
					s = item.jux[gid].getSplit();
					m_lItems[s - item.left + 1][item.left].type = StateItem::L2R;
					stack.push(std::tuple<int, int, int, int>(g, s - item.left + 1, -1, item.left));
					m_lItems[item.right - s][s + 1].type = StateItem::R2L;
					stack.push(std::tuple<int, int, int, int>(g, item.right - s, -1, s + 1));
				}
				break;
			case StateItem::L2R:
				s = item.l2r[gid].getSplit();

				if (item.left < item.right - 1) {
					m_lItems[s - item.left + 1][item.left].type = StateItem::L2R_SOLID_BOTH;
					stack.push(std::tuple<int, int, int, int>(g, s - item.left + 1, m_lItems[s - item.left + 1][item.left].l2r_solid_both[GRAND_INDEX(g, item.left, s - item.left + 1)].bestItem().getSplit(), item.left));
					m_lItems[item.right - s + 1][s].type = StateItem::L2R;
					stack.push(std::tuple<int, int, int, int>(item.left, item.right - s + 1, -1, s));
				}
				else {
					m_vecTrainArcs.push_back(Arc(item.left, item.right));
				}
				break;
			case StateItem::R2L:
				s = item.r2l[gid].getSplit();

				if (item.left < item.right - 1) {
					m_lItems[item.right - s + 1][s].type = StateItem::R2L_SOLID_BOTH;
					stack.push(std::tuple<int, int, int, int>(g, item.right - s + 1, m_lItems[item.right - s + 1][s].r2l_solid_both[GRAND_INDEX(g, s, item.right - s + 1)].bestItem().getSplit(), s));
					m_lItems[s - item.left + 1][item.left].type = StateItem::R2L;
					stack.push(std::tuple<int, int, int, int>(item.right, s - item.left + 1, -1, item.left));
				}
				else {
					m_vecTrainArcs.push_back(Arc(item.right == m_nSentenceLength ? -1 : item.right, item.left));
				}
				break;
			case StateItem::L2R_SOLID_BOTH:
				m_vecTrainArcs.push_back(Arc(item.left, item.right));

				if (s == item.left) {
					m_lItems[item.right - s][s + 1].type = StateItem::R2L;
					stack.push(std::tuple<int, int, int, int>(item.left, item.right - s, -1, s + 1));
				}
				else {
					is = findInnerSplit(item.l2r_solid_both[gid], s);

					m_lItems[s - item.left + 1][item.left].type = StateItem::L2R_SOLID_BOTH;
					stack.push(std::tuple<int, int, int, int>(g, s - item.left + 1, is, item.left));
					m_lItems[item.right - s + 1][s].type = StateItem::JUX;
					stack.push(std::tuple<int, int, int, int>(item.left, item.right - s + 1, -1, s));
				}
				break;
			case StateItem::R2L_SOLID_BOTH:
				m_vecTrainArcs.push_back(Arc(item.right == m_nSentenceLength ? -1 : item.right, item.left));

				if (s == item.right) {
					m_lItems[s - item.left][item.left].type = StateItem::L2R;
					stack.push(std::tuple<int, int, int, int>(item.right, s - item.left, -1, item.left));
				}
				else {
					is = findInnerSplit(item.r2l_solid_both[gid], s);

					m_lItems[item.right - s + 1][s].type = StateItem::R2L_SOLID_BOTH;
					stack.push(std::tuple<int, int, int, int>(g, item.right - s + 1, is, s));
					m_lItems[s - item.left + 1][item.left].type = StateItem::JUX;
					stack.push(std::tuple<int, int, int, int>(item.right, s - item.left + 1, -1, item.left));
				}
				break;
			default:
				break;
			}
		}
	}

	void DepParser::update() {
		Arcs2QuarArcs(m_vecTrainArcs, m_vecTrainQuarArcs);

		std::unordered_set<QuarArc> positiveArcs;
		positiveArcs.insert(m_vecCorrectQuarArcs.begin(), m_vecCorrectQuarArcs.end());
		for (const auto & arc : m_vecTrainQuarArcs) {
			positiveArcs.erase(arc);
		}
		std::unordered_set<QuarArc> negativeArcs;
		negativeArcs.insert(m_vecTrainQuarArcs.begin(), m_vecTrainQuarArcs.end());
		for (const auto & arc : m_vecCorrectQuarArcs) {
			negativeArcs.erase(arc);
		}
		if (!positiveArcs.empty() || !negativeArcs.empty()) {
			++m_nTotalErrors;
		}
		for (const auto & arc : positiveArcs) {
			if (arc.first() != -1) {
				getOrUpdateGrandScore(arc.first(), arc.second(), arc.fifth(), 1);
				getOrUpdateGrandScore(arc.first(), arc.second(), arc.forth(), arc.fifth(), 1);
				getOrUpdateGrandScore(arc.first(), arc.second(), arc.third(), arc.forth(), arc.fifth(), 1);
			}
			getOrUpdateSiblingScore(arc.second(), arc.third(), arc.forth(), arc.fifth(), 1);
			getOrUpdateSiblingScore(arc.second(), arc.forth(), arc.fifth(), 1);
			getOrUpdateSiblingScore(arc.second(), arc.fifth(), 1);
		}
		for (const auto & arc : negativeArcs) {
			if (arc.first() != -1) {
				getOrUpdateGrandScore(arc.first(), arc.second(), arc.fifth(), -1);
				getOrUpdateGrandScore(arc.first(), arc.second(), arc.forth(), arc.fifth(), -1);
				getOrUpdateGrandScore(arc.first(), arc.second(), arc.third(), arc.forth(), arc.fifth(), -1);
			}
			getOrUpdateSiblingScore(arc.second(), arc.third(), arc.forth(), arc.fifth(), -1);
			getOrUpdateSiblingScore(arc.second(), arc.forth(), arc.fifth(), -1);
			getOrUpdateSiblingScore(arc.second(), arc.fifth(), -1);
		}
	}

	void DepParser::generate(DependencyTree * retval, const DependencyTree & correct) {
		for (int i = 0; i < m_nSentenceLength; ++i) {
			retval->push_back(DependencyTreeNode(std::get<0>(correct[i]), -1, "NMOD"));
		}
		for (const auto & arc : m_vecTrainArcs) {
			std::get<1>(retval->at(arc.second())) = arc.first();
		}
	}

	void DepParser::goldCheck() {
		Arcs2QuarArcs(m_vecTrainArcs, m_vecTrainQuarArcs);
		if (m_vecCorrectArcs.size() != m_vecTrainArcs.size() || m_lItems[m_nSentenceLength + 1][0].r2l[m_nSentenceLength].getScore() / GOLD_POS_SCORE != 6 * m_vecTrainArcs.size() - 5) {
			std::cout << "gold parse len error at " << m_nTrainingRound << std::endl;
			std::cout << "score is " << m_lItems[m_nSentenceLength + 1][0].r2l[m_nSentenceLength].getScore() << std::endl;
			std::cout << "len is " << m_vecTrainArcs.size() << std::endl;
			++m_nTotalErrors;
		}
		else {
			int i = 0;
			std::sort(m_vecCorrectArcs.begin(), m_vecCorrectArcs.end(), [](const Arc & arc1, const Arc & arc2){ return arc1 < arc2; });
			std::sort(m_vecTrainArcs.begin(), m_vecTrainArcs.end(), [](const Arc & arc1, const Arc & arc2){ return arc1 < arc2; });
			for (int n = m_vecCorrectArcs.size(); i < n; ++i) {
				if (m_vecCorrectArcs[i].first() != m_vecTrainArcs[i].first() || m_vecCorrectArcs[i].second() != m_vecTrainArcs[i].second()) {
					break;
				}
			}
			if (i != m_vecCorrectArcs.size()) {
				std::cout << "gold parse tree error at " << m_nTrainingRound << std::endl;
				++m_nTotalErrors;
			}
		}
	}

	const tscore & DepParser::arcScore(const int & p, const int & c) {
		if (m_nState == ParserState::GOLDTEST) {
			m_nRetval = m_setArcGoldScore.find(BiGram<int>(p, c)) == m_setArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return m_nRetval;
		}
		m_nRetval = 0;
		getOrUpdateSiblingScore(p, c, 0);
		return m_nRetval;
	}

	const tscore & DepParser::biSiblingArcScore(const int & p, const int & c, const int & c2) {
		if (m_nState == ParserState::GOLDTEST) {
			m_nRetval = m_setBiSiblingArcGoldScore.find(TriGram<int>(p, c, c2)) == m_setBiSiblingArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return m_nRetval;
		}
		m_nRetval = 0;
		getOrUpdateSiblingScore(p, c, c2, 0);
		return m_nRetval;
	}

	const tscore & DepParser::triSiblingArcScore(const int & p, const int & c, const int & c2, const int & c3) {
		if (m_nState == ParserState::GOLDTEST) {
			m_nRetval = m_setTriSiblingArcGoldScore.find(QuarGram<int>(p, c, c2, c3)) == m_setTriSiblingArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return m_nRetval;
		}
		m_nRetval = 0;
		getOrUpdateSiblingScore(p, c, c2, c3, 0);
		return m_nRetval;
	}

	const tscore & DepParser::grandSiblingArcScore(const int & g, const int & p, const int & c) {
		if (m_nState == ParserState::GOLDTEST) {
			m_nRetval = m_setGrandSiblingArcGoldScore.find(TriGram<int>(g, p, c)) == m_setGrandSiblingArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return m_nRetval;
		}
		m_nRetval = 0;
		getOrUpdateGrandScore(g, p, c, 0);
		return m_nRetval;
	}

	const tscore & DepParser::grandBiSiblingArcScore(const int & g, const int & p, const int & c, const int & c2) {
		if (m_nState == ParserState::GOLDTEST) {
			m_nRetval = m_setGrandBiSiblingArcGoldScore.find(QuarGram<int>(g, p, c, c2)) == m_setGrandBiSiblingArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return m_nRetval;
		}
		m_nRetval = 0;
		getOrUpdateGrandScore(g, p, c, c2, 0);
		return m_nRetval;
	}

	const tscore & DepParser::grandTriSiblingArcScore(const int & g, const int & p, const int & c, const int & c2, const int & c3) {
		if (m_nState == ParserState::GOLDTEST) {
			m_nRetval = m_setGrandTriSiblingArcGoldScore.find(QuinGram<int>(g, p, c, c2, c3)) == m_setGrandTriSiblingArcGoldScore.end() ? GOLD_NEG_SCORE : GOLD_POS_SCORE;
			return m_nRetval;
		}
		m_nRetval = 0;
		getOrUpdateGrandScore(g, p, c, c2, c3, 0);
		return m_nRetval;
	}

	void DepParser::initArcScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			m_lArcScore[ENCODE_L2R(i)] = arcScore(i, i + d - 1);
			m_lArcScore[ENCODE_R2L(i)] = arcScore(i + d - 1, i);
		}
		int i = m_nSentenceLength - d + 1;
		m_lArcScore[ENCODE_R2L(i)] = arcScore(m_nSentenceLength, i);
	}

	void DepParser::initBiSiblingArcScore(const int & d) {
		for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
			int l = i + d - 1;
			m_lBiSiblingArcScore[ENCODE_2ND_L2R(i, i)] = biSiblingArcScore(i, -1, l);
			m_lBiSiblingArcScore[ENCODE_2ND_R2L(i, i)] = biSiblingArcScore(l, -1, i);
			for (int k = i + 1, l = i + d - 1; k < l; ++k) {
				m_lBiSiblingArcScore[ENCODE_2ND_L2R(i, k)] = biSiblingArcScore(i, k, l);
				m_lBiSiblingArcScore[ENCODE_2ND_R2L(i, k)] = biSiblingArcScore(l, k, i);
			}
		}
		int i = m_nSentenceLength - d + 1;
		m_lBiSiblingArcScore[ENCODE_2ND_R2L(i, i)] = biSiblingArcScore(m_nSentenceLength, -1, i);
	}

	void DepParser::initGrandSiblingArcScore(const int & d, const int & i) {
		int l = i + d - 1;
		int g = 0;
		do {
			if (g >= i && g <= l) {
				continue;
			}
			m_lGrandSiblingArcScore[ENCODE_L2R(g)] = grandSiblingArcScore(g, i, l);
			m_lGrandSiblingArcScore[ENCODE_R2L(g)] = grandSiblingArcScore(g, l, i);
		} while (++g <= m_nSentenceLength);
	}

	void DepParser::initGrandBiSiblingArcScore(const int & d, const int & i, const int & k) {
		int l = i + d - 1;
		int g = 0;
		do {
			if (g >= i && g <= l) {
				continue;
			}
			m_lGrandBiSiblingArcScore[ENCODE_L2R(g)] = grandBiSiblingArcScore(g, i, k, l);
			m_lGrandBiSiblingArcScore[ENCODE_R2L(g)] = grandBiSiblingArcScore(g, l, k, i);
		} while (++g <= m_nSentenceLength);
	}

	void DepParser::getOrUpdateSiblingScore(const int & p, const int & c, const int & amount) {

		Weight * cweight = (Weight*)m_Weight;

		p_1_tag = p > 0 ? m_lSentence[p - 1].second() : start_taggedword.second();
		p1_tag = p < m_nSentenceLength - 1 ? m_lSentence[p + 1].second() : end_taggedword.second();
		c_1_tag = c > 0 ? m_lSentence[c - 1].second() : start_taggedword.second();
		c1_tag = c < m_nSentenceLength - 1 ? m_lSentence[c + 1].second() : end_taggedword.second();

		p_word = m_lSentence[p].first();
		p_tag = m_lSentence[p].second();

		c_word = m_lSentence[c].first();
		c_tag = m_lSentence[c].second();

		m_nDis = encodeLinkDistanceOrDirection(p, c, false);
		m_nDir = encodeLinkDistanceOrDirection(p, c, true);

		word_int.refer(p_word, 0);
		cweight->m_mapPw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_int.referLast(m_nDis);
		cweight->m_mapPw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_int.referLast(m_nDir);
		cweight->m_mapPw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_int.refer(p_tag, 0);
		cweight->m_mapPp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_int.referLast(m_nDis);
		cweight->m_mapPp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_int.referLast(m_nDir);
		cweight->m_mapPp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(p_word, p_tag, 0);
		cweight->m_mapPwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDis);
		cweight->m_mapPwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapPwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_int.refer(c_word, 0);
		cweight->m_mapCw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_int.referLast(m_nDis);
		cweight->m_mapCw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_int.referLast(m_nDir);
		cweight->m_mapCw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_int.refer(c_tag, 0);
		cweight->m_mapCp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_int.referLast(m_nDis);
		cweight->m_mapCp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_int.referLast(m_nDir);
		cweight->m_mapCp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(c_word, c_tag, 0);
		cweight->m_mapCwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDis);
		cweight->m_mapCwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapCwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(p_word, c_word, p_tag, c_tag, 0);
		cweight->m_mapPwpCwp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPwpCwp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwpCwp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_int.refer(p_word, p_tag, c_tag, 0);
		cweight->m_mapPwpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPwpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_int.refer(c_word, p_tag, c_tag, 0);
		cweight->m_mapPpCwp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpCwp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpCwp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_int.refer(p_word, c_word, p_tag, 0);
		cweight->m_mapPwpCw.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDis);
		cweight->m_mapPwpCw.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapPwpCw.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_int.refer(p_word, c_word, c_tag, 0);
		cweight->m_mapPwCwp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDis);
		cweight->m_mapPwCwp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapPwCwp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_int.refer(p_word, c_word, 0);
		cweight->m_mapPwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_int.referLast(m_nDis);
		cweight->m_mapPwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_int.referLast(m_nDir);
		cweight->m_mapPwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_int.refer(p_tag, c_tag, 0);
		cweight->m_mapPpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_1_tag, c_tag, 0);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_1_tag, c_tag, 0);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_tag, c1_tag, 0);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_tag, c1_tag, 0);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(empty_taggedword.second(), p1_tag, c_1_tag, c_tag, 0);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.refer(empty_taggedword.second(), p1_tag, c_1_tag, c_tag, m_nDir);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_1_tag, empty_taggedword.second(), c_1_tag, c_tag, 0);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(empty_taggedword.second(), p1_tag, c_tag, c1_tag, 0);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_1_tag, empty_taggedword.second(), c_tag, c1_tag, 0);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_1_tag, empty_taggedword.second(), 0);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_1_tag, empty_taggedword.second(), 0);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_tag, p1_tag, empty_taggedword.second(), c1_tag, 0);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_1_tag, p_tag, empty_taggedword.second(), c1_tag, 0);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_tag, empty_taggedword.second(), c_1_tag, c_tag, 0);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(empty_taggedword.second(), p_tag, c_1_tag, c_tag, 0);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_tag, p1_tag, c_tag, empty_taggedword.second(), 0);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_int.refer(p_1_tag, p_tag, c_tag, empty_taggedword.second(), 0);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDis);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		for (int b = (int)std::fmin(p, c) + 1, e = (int)std::fmax(p, c); b < e; ++b) {
			b_tag = m_lSentence[b].second();
			tag_tag_tag_int.refer(p_tag, b_tag, c_tag, 0);
			cweight->m_mapPpBpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
			tag_tag_tag_int.refer(p_tag, b_tag, c_tag, m_nDis);
			cweight->m_mapPpBpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
			tag_tag_tag_int.refer(p_tag, b_tag, c_tag, m_nDir);
			cweight->m_mapPpBpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		}
	}

	void DepParser::getOrUpdateSiblingScore(const int & p, const int & c, const int & c2, const int & amount) {
		Weight * cweight = (Weight*)m_Weight;

		p_tag = m_lSentence[p].second();

		c_word = IS_NULL(c) ? empty_taggedword.first() : m_lSentence[c].first();
		c_tag = IS_NULL(c) ? empty_taggedword.second() : m_lSentence[c].second();

		c2_word = m_lSentence[c2].first();
		c2_tag = m_lSentence[c2].second();

		m_nDir = encodeLinkDistanceOrDirection(p, c2, true);

		tag_tag_int.refer(c_tag, c2_tag, 0);
		cweight->m_mapC1pC2p.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_int.referLast(m_nDir);
		cweight->m_mapC1pC2p.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_int.refer(p_tag, c_tag, c2_tag, 0);
		cweight->m_mapPpC1pC2p.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpC1pC2p.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_int.refer(c_word, c2_word, 0);
		cweight->m_mapC1wC2w.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_int.referLast(m_nDir);
		cweight->m_mapC1wC2w.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(c_word, c2_tag, 0);
		cweight->m_mapC1wC2p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC2p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(c2_word, c_tag, 0);
		cweight->m_mapC1wC2p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC2p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	}

	void DepParser::getOrUpdateSiblingScore(const int & p, const int & c, const int & c2, const int & c3, const int & amount) {
		Weight * cweight = (Weight*)m_Weight;

		p_word = m_lSentence[p].first();
		p_tag = m_lSentence[p].second();

		c_word = IS_NULL(c) ? empty_taggedword.first() : m_lSentence[c].first();
		c_tag = IS_NULL(c) ? empty_taggedword.second() : m_lSentence[c].second();

		c2_word = IS_NULL(c2) ? empty_taggedword.first() : m_lSentence[c2].first();
		c2_tag = IS_NULL(c2) ? empty_taggedword.second() : m_lSentence[c2].second();

		c3_word = m_lSentence[c3].first();
		c3_tag = m_lSentence[c3].second();

		m_nDir = encodeLinkDistanceOrDirection(p, c3, true);

		// word tag tag tag
		word_tag_tag_tag_int.refer(p_word, c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapPwC1pC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwC1pC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c_word, p_tag, c2_tag, c3_tag, 0);
		cweight->m_mapC1wPpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC1wPpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c2_word, p_tag, c_tag, c3_tag, 0);
		cweight->m_mapC2wPpC1pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC2wPpC1pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c3_word, p_tag, c_tag, c2_tag, 0);
		cweight->m_mapC3wPpC1pC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC3wPpC1pC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// word word tag tag
		word_word_tag_tag_int.refer(p_word, c_word, c2_tag, c3_tag, 0);
		cweight->m_mapPwC1wC2pC3p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwC1wC2pC3p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(p_word, c2_word, c_tag, c3_tag, 0);
		cweight->m_mapPwC2wC1pC3p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwC2wC1pC3p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(p_word, c3_word, c_tag, c2_tag, 0);
		cweight->m_mapPwC3wC1pC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwC3wC1pC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(c_word, c2_word, p_tag, c3_tag, 0);
		cweight->m_mapC1wC2wPpC3p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC2wPpC3p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(c_word, c3_word, p_tag, c2_tag, 0);
		cweight->m_mapC1wC3wPpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC3wPpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(c2_word, c3_word, p_tag, c_tag, 0);
		cweight->m_mapC2wC3wPpC1p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC2wC3wPpC1p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// tag tag tag tag
		tag_tag_tag_tag_int.refer(p_tag, c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapPpC1pC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPpC1pC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// word tag tag
		word_tag_tag_int.refer(c_word, c2_tag, c3_tag, 0);
		cweight->m_mapC1wC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_int.refer(c2_word, c_tag, c3_tag, 0);
		cweight->m_mapC2wC1pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC2wC1pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_int.refer(c3_word, c_tag, c2_tag, 0);
		cweight->m_mapC3wC1pC2p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC3wC1pC2p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// word word tag
		word_word_tag_int.refer(c_word, c2_word, c3_tag, 0);
		cweight->m_mapC1wC2wC3p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC2wC3p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_int.refer(c_word, c3_word, c2_tag, 0);
		cweight->m_mapC1wC3wC2p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC3wC2p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_int.refer(c2_word, c3_word, c_tag, 0);
		cweight->m_mapC2wC3wC1p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapC2wC3wC1p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// tag tag tag
		tag_tag_tag_int.refer(c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapC1pC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC1pC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// other
		word_word_int.refer(c_word, c3_word, 0);
		cweight->m_mapC1wC3w.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_int.referLast(m_nDir);
		cweight->m_mapC1wC3w.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(c_word, c3_tag, 0);
		cweight->m_mapC1wC3p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapC1wC3p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(c3_word, c_tag, 0);
		cweight->m_mapC3wC1p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapC3wC1p.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_int.refer(c_tag, c3_tag, 0);
		cweight->m_mapC1pC3p.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_int.referLast(m_nDir);
		cweight->m_mapC1pC3p.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	}

	void DepParser::getOrUpdateGrandScore(const int & g, const int & p, const int & c, const int & amount) {
		Weight * cweight = (Weight*)m_Weight;

		g_word = m_lSentence[g].first();
		g_tag = m_lSentence[g].second();

		p_tag = m_lSentence[p].second();

		c_word = m_lSentence[c].first();
		c_tag = m_lSentence[c].second();

		m_nDir = encodeLinkDistanceOrDirection(g, p, true);

		tag_tag_tag_int.refer(g_tag, p_tag, c_tag, 0);
		cweight->m_mapGpPpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGpPpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_int.refer(g_tag, c_tag, 0);
		cweight->m_mapGpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_int.referLast(m_nDir);
		cweight->m_mapGpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_int.refer(g_word, c_word, 0);
		cweight->m_mapGwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_int.referLast(m_nDir);
		cweight->m_mapGwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(g_word, c_tag, 0);
		cweight->m_mapGwCp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapGwCp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_int.refer(c_word, g_tag, 0);
		cweight->m_mapCwGp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_int.referLast(m_nDir);
		cweight->m_mapCwGp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	}

	void DepParser::getOrUpdateGrandScore(const int & g, const int & p, const int & c, const int & c2, const int & amount) {
		Weight * cweight = (Weight*)m_Weight;

		g_word = m_lSentence[g].first();
		g_tag = m_lSentence[g].second();

		p_word = m_lSentence[p].first();
		p_tag = m_lSentence[p].second();

		c_word = IS_NULL(c) ? empty_taggedword.first() : m_lSentence[c].first();
		c_tag = IS_NULL(c) ? empty_taggedword.second() : m_lSentence[c].second();

		c2_word = m_lSentence[c2].first();
		c2_tag = m_lSentence[c2].second();

		m_nDir = encodeLinkDistanceOrDirection(g, p, true);

		// word tag tag tag
		word_tag_tag_tag_int.refer(g_word, p_tag, c_tag, c2_tag, 0);
		cweight->m_mapGwPpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGwPpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(p_word, g_tag, c_tag, c2_tag, 0);
		cweight->m_mapPwGpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwGpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c_word, g_tag, p_tag, c2_tag, 0);
		cweight->m_mapCwGpPpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapCwGpPpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c2_word, g_tag, p_tag, c_tag, 0);
		cweight->m_mapC2wGpPpCp.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC2wGpPpCp.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// word word tag tag
		word_word_tag_tag_int.refer(g_word, p_word, c_tag, c2_tag, 0);
		cweight->m_mapGwPwCpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGwPwCpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(g_word, c_word, p_tag, c2_tag, 0);
		cweight->m_mapGwCwPpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGwCwPpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(g_word, c2_word, p_tag, c_tag, 0);
		cweight->m_mapGwC2wPpCp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGwC2wPpCp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(p_word, c_word, g_tag, c2_tag, 0);
		cweight->m_mapPwCwGpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwCwGpC2p.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(p_word, c2_word, g_tag, c_tag, 0);
		cweight->m_mapCwC2wGpPp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapCwC2wGpPp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_tag_int.refer(c_word, c2_word, g_tag, p_tag, 0);
		cweight->m_mapCwC2wGpPp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapCwC2wGpPp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// tag tag tag tag
		tag_tag_tag_tag_int.refer(g_tag, p_tag, c_tag, c2_tag, 0);
		cweight->m_mapGpPpCpC2p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGpPpCpC2p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// word tag tag
		word_tag_tag_int.refer(g_word, c_tag, c2_tag, 0);
		cweight->m_mapGwCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGwCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_int.refer(c_word, g_tag, c2_tag, 0);
		cweight->m_mapCwGpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapCwGpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_int.refer(c2_word, g_tag, c_tag, 0);
		cweight->m_mapC2wGpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC2wGpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// word word tag
		word_word_tag_int.refer(g_word, c_word, c2_tag, 0);
		cweight->m_mapGwCwC2p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapGwCwC2p.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_int.refer(g_word, c2_word, c_tag, 0);
		cweight->m_mapGwC2wCp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapGwC2wCp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_word_tag_int.refer(c_word, c2_word, g_tag, 0);
		cweight->m_mapCwC2wGp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_word_tag_int.referLast(m_nDir);
		cweight->m_mapCwC2wGp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// tag tag tag
		tag_tag_tag_int.refer(g_tag, c_tag, c2_tag, 0);
		cweight->m_mapGpCpC2p.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGpCpC2p.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

	}

	void DepParser::getOrUpdateGrandScore(const int & g, const int & p, const int & c, const int & c2, const int & c3, const int & amount) {
		Weight * cweight = (Weight*)m_Weight;

		g_word = m_lSentence[g].first();
		g_tag = m_lSentence[g].second();

		p_word = m_lSentence[p].first();
		p_tag = m_lSentence[p].second();

		c_word = IS_NULL(c) ? empty_taggedword.first() : m_lSentence[c].first();
		c_tag = IS_NULL(c) ? empty_taggedword.second() : m_lSentence[c].second();

		c2_word = IS_NULL(c2) ? empty_taggedword.first() : m_lSentence[c2].first();
		c2_tag = IS_NULL(c2) ? empty_taggedword.second() : m_lSentence[c2].second();

		c3_word = m_lSentence[c3].first();
		c3_tag = m_lSentence[c3].second();

		m_nDir = encodeLinkDistanceOrDirection(g, p, true);

		word_tag_tag_tag_tag_int.refer(g_word, p_tag, c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapGwPpCpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGwPpCpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_tag_int.refer(p_word, g_tag, c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapPwGpCpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapPwGpCpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_tag_int.refer(c_word, g_tag, p_tag, c2_tag, c3_tag, 0);
		cweight->m_mapCwGpPpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapCwGpPpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_tag_int.refer(c2_word, g_tag, p_tag, c_tag, c3_tag, 0);
		cweight->m_mapC2wGpPpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC2wGpPpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_tag_int.refer(c3_word, g_tag, p_tag, c_tag, c2_tag, 0);
		cweight->m_mapC3wGpPpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC3wGpPpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		tag_tag_tag_tag_tag_int.refer(g_tag, p_tag, c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapGpPpCpC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGpPpCpC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// word tag tag tag
		word_tag_tag_tag_int.refer(g_word, c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapGwCpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGwCpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c_word, g_tag, c2_tag, c3_tag, 0);
		cweight->m_mapCwGpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapCwGpC2pC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c2_word, g_tag, c_tag, c3_tag, 0);
		cweight->m_mapC2wGpCpC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC2wGpCpC3p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		word_tag_tag_tag_int.refer(c3_word, g_tag, c_tag, c2_tag, 0);
		cweight->m_mapC3wGpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		word_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapC3wGpCpC2p.getOrUpdateScore(m_nRetval, word_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

		// tag tag tag tag
		tag_tag_tag_tag_int.refer(g_tag, c_tag, c2_tag, c3_tag, 0);
		cweight->m_mapGpCpC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
		tag_tag_tag_tag_int.referLast(m_nDir);
		cweight->m_mapGpCpC2pC3p.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
	}

}

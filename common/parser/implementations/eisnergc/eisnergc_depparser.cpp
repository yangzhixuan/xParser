#include <cmath>
#include <stack>
#include <algorithm>
#include <unordered_set>

#include "eisnergc_depparser.h"
#include "common/token/word.h"
#include "common/token/pos.h"

namespace eisnergc {

    WordPOSTag DepParser::empty_taggedword = WordPOSTag();
    WordPOSTag DepParser::start_taggedword = WordPOSTag();
    WordPOSTag DepParser::end_taggedword = WordPOSTag();

    DepParser::DepParser(const std::string &sFeatureInput, const std::string &sFeatureOut, int nState) :
            DepParserBase(sFeatureInput, nState) {

        m_nSentenceLength = 0;

        m_Weight = new Weight(sFeatureInput, sFeatureOut);

        for (int i = 0; i < MAX_SENTENCE_SIZE; ++i) {
            m_lItems[1][i].init(i, i, MAX_SENTENCE_SIZE - 1);
        }

        DepParser::empty_taggedword.refer(TWord::code(EMPTY_WORD), TPOSTag::code(EMPTY_POSTAG));
        DepParser::start_taggedword.refer(TWord::code(START_WORD), TPOSTag::code(START_POSTAG));
        DepParser::end_taggedword.refer(TWord::code(END_WORD), TPOSTag::code(END_POSTAG));
    }

    DepParser::~DepParser() {
        delete m_Weight;
    }

    void DepParser::train(const DependencyTree &correct, const int &round) {
        // initialize
        int idx = 0;
        m_vecCorrectArcs.clear();
        m_nTrainingRound = round;
        m_nSentenceLength = correct.size();
        // for normal sentence
        for (const auto &node : correct) {
            m_lSentence[idx].refer(TWord::code(std::get<0>(std::get<0>(node))),
                                   TPOSTag::code(std::get<1>(std::get<0>(node))));
            m_vecCorrectArcs.push_back(Arc(std::get<1>(node), idx++));
        }
        m_lSentence[idx].refer(TWord::code(ROOT_WORD), TPOSTag::code(ROOT_POSTAG));
        Arcs2BiArcs(m_vecCorrectArcs, m_vecCorrectBiArcs);

        if (m_nState == ParserState::GOLDTEST) {
            m_setFirstGoldScore.clear();
            m_setSecondGoldScore.clear();
            for (const auto &arc : m_vecCorrectArcs) {
                m_setFirstGoldScore.insert(
                        BiGram<int>(arc.first() == -1 ? m_nSentenceLength : arc.first(), arc.second()));
            }
            for (const auto &biarc : m_vecCorrectBiArcs) {
                m_setSecondGoldScore.insert(TriGram<int>(biarc.first(), biarc.second(), biarc.third()));
            }
        }

        work(nullptr, correct);

        if (m_nTrainingRound % OUTPUT_STEP == 0) {
            std::cout << m_nTotalErrors << " / " << m_nTrainingRound << std::endl;
        }
    }

    void DepParser::parse(const Sentence &sentence, DependencyTree *retval) {
        int idx = 0;
        m_nTrainingRound = 0;
        DependencyTree correct;
        m_nSentenceLength = sentence.size();
        for (const auto &token : sentence) {
            m_lSentence[idx++].refer(TWord::code(std::get<0>(token)), TPOSTag::code(std::get<1>(token)));
            correct.push_back(DependencyTreeNode(token, -1, "NMOD"));
        }
        m_lSentence[idx].refer(TWord::code(ROOT_WORD), TPOSTag::code(ROOT_POSTAG));
        work(retval, correct);
    }

    void DepParser::work(DependencyTree *retval, const DependencyTree &correct) {

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
    }

    void DepParser::decode() {
        for (int d = 2; d <= m_nSentenceLength; ++d) {

            initFirstOrderScore(d);

            initSecondOrderScore(d);

            for (int l = 0, n = m_nSentenceLength - d + 1; l < n; ++l) {
                int r = l + d - 1;
                StateItem &item = m_lItems[d][l];
                item.init(l, r, m_nSentenceLength);
                for (int m = l + 1; m < r; ++m) {
                    StateItem &ritem = m_lItems[r - m + 1][m];
                    StateItem &litem = m_lItems[m - l + 1][l];
                    tscore rPartialScore = ritem.l2r[l].getScore();
                    tscore lPartialScore = litem.r2l[r].getScore();
                    for (int g = 0; g < l; ++g) {
                        item.updateL2R(g, m, litem.l2r_im[g].getScore() + rPartialScore);
                        item.updateR2L(g, m, ritem.r2l_im[g].getScore() + lPartialScore);
                    }
                    for (int g = r + 1; g <= m_nSentenceLength; ++g) {
                        item.updateL2R(g, m, litem.l2r_im[g].getScore() + rPartialScore);
                        item.updateR2L(g, m, ritem.r2l_im[g].getScore() + lPartialScore);
                    }
                }
                tscore l2rArcScore = m_lFirstOrderScore[ENCODE_L2R(l)];
                tscore r2lArcScore = m_lFirstOrderScore[ENCODE_R2L(l)];

                for (int m = l; m < r; ++m) {
                    StateItem &ritem = m_lItems[r - m][m + 1];
                    StateItem &litem = m_lItems[m - l + 1][l];
                    tscore l2rPartialScore = ritem.r2l[l].getScore() + l2rArcScore;
                    tscore r2lPartialScore = litem.l2r[r].getScore() + r2lArcScore;
                    for (int g = 0; g < l; ++g) {
                        item.updateL2RIm(g, m, litem.l2r[g].getScore() + l2rPartialScore +
                                               m_lSecondOrderScore[ENCODE_2ND_L2R(l, g)]);
                        item.updateR2LIm(g, m, ritem.r2l[g].getScore() + r2lPartialScore +
                                               m_lSecondOrderScore[ENCODE_2ND_R2L(l, g)]);
                    }
                    for (int g = r + 1; g <= m_nSentenceLength; ++g) {
                        item.updateL2RIm(g, m, litem.l2r[g].getScore() + l2rPartialScore +
                                               m_lSecondOrderScore[ENCODE_2ND_L2R(l, g)]);
                        item.updateR2LIm(g, m, ritem.r2l[g].getScore() + r2lPartialScore +
                                               m_lSecondOrderScore[ENCODE_2ND_R2L(l, g)]);
                    }
                }
                for (int g = 0; g < l; ++g) {
                    item.updateL2R(g, r, item.l2r_im[g].getScore());
                    item.updateR2L(g, l, item.r2l_im[g].getScore());
                }
                for (int g = r + 1; g <= m_nSentenceLength; ++g) {
                    item.updateL2R(g, r, item.l2r_im[g].getScore());
                    item.updateR2L(g, l, item.r2l_im[g].getScore());
                }
            }
        }
        // best root
        StateItem &item = m_lItems[m_nSentenceLength + 1][0];
        item.r2l[m_nSentenceLength].reset();
        for (int m = 0; m < m_nSentenceLength; ++m) {
            item.updateR2L(m_nSentenceLength, m, m_lItems[m + 1][0].r2l[m_nSentenceLength].getScore() +
                                                 m_lItems[m_nSentenceLength - m][m].l2r[m_nSentenceLength].getScore() +
                                                 arcScore(m_nSentenceLength, m));
        }
    }

    void DepParser::decodeArcs() {

        m_vecTrainArcs.clear();
        std::stack<std::tuple<int, int, int>> stack;

        int s = m_lItems[m_nSentenceLength + 1][0].r2l[m_nSentenceLength].getSplit();
        m_vecTrainArcs.push_back(Arc(-1, s));

        m_lItems[s + 1][0].type = StateItem::R2L_COMP;
        stack.push(std::tuple<int, int, int>(s + 1, m_nSentenceLength, 0));
        m_lItems[m_nSentenceLength - s][s].type = StateItem::L2R_COMP;
        stack.push(std::tuple<int, int, int>(m_nSentenceLength - s, m_nSentenceLength, s));

        while (!stack.empty()) {

            auto span = stack.top();
            stack.pop();

            int split, grand = std::get<1>(span);
            StateItem &item = m_lItems[std::get<0>(span)][std::get<2>(span)];

            if (item.left == item.right) {
                continue;
            }

            switch (item.type) {
                case StateItem::L2R_COMP:
                    split = item.l2r[grand].getSplit();

                    m_lItems[split - item.left + 1][item.left].type = StateItem::L2R_IM_COMP;
                    stack.push(std::tuple<int, int, int>(split - item.left + 1, grand, item.left));
                    m_lItems[item.right - split + 1][split].type = StateItem::L2R_COMP;
                    stack.push(std::tuple<int, int, int>(item.right - split + 1, item.left, split));
                    break;
                case StateItem::R2L_COMP:
                    split = item.r2l[grand].getSplit();

                    m_lItems[item.right - split + 1][split].type = StateItem::R2L_IM_COMP;
                    stack.push(std::tuple<int, int, int>(item.right - split + 1, grand, split));
                    m_lItems[split - item.left + 1][item.left].type = StateItem::R2L_COMP;
                    stack.push(std::tuple<int, int, int>(split - item.left + 1, item.right, item.left));
                    break;
                case StateItem::L2R_IM_COMP:
                    m_vecTrainArcs.push_back(Arc(item.left, item.right));

                    split = item.l2r_im[grand].getSplit();

                    m_lItems[split - item.left + 1][item.left].type = StateItem::L2R_COMP;
                    stack.push(std::tuple<int, int, int>(split - item.left + 1, grand, item.left));
                    m_lItems[item.right - split][split + 1].type = StateItem::R2L_COMP;
                    stack.push(std::tuple<int, int, int>(item.right - split, item.left, split + 1));
                    break;
                case StateItem::R2L_IM_COMP:
                    m_vecTrainArcs.push_back(Arc(item.right, item.left));

                    split = item.r2l_im[grand].getSplit();

                    m_lItems[item.right - split][split + 1].type = StateItem::R2L_COMP;
                    stack.push(std::tuple<int, int, int>(item.right - split, grand, split + 1));
                    m_lItems[split - item.left + 1][item.left].type = StateItem::L2R_COMP;
                    stack.push(std::tuple<int, int, int>(split - item.left + 1, item.right, item.left));
                    break;
                default:
                    break;
            }
        }
    }

    void DepParser::update() {
        Arcs2BiArcs(m_vecTrainArcs, m_vecTrainBiArcs);

        std::unordered_set<BiArc> positiveArcs;
        positiveArcs.insert(m_vecCorrectBiArcs.begin(), m_vecCorrectBiArcs.end());
        for (const auto &arc : m_vecTrainBiArcs) {
            positiveArcs.erase(arc);
        }
        std::unordered_set<BiArc> negativeArcs;
        negativeArcs.insert(m_vecTrainBiArcs.begin(), m_vecTrainBiArcs.end());
        for (const auto &arc : m_vecCorrectBiArcs) {
            negativeArcs.erase(arc);
        }
        if (!positiveArcs.empty() || !negativeArcs.empty()) {
            ++m_nTotalErrors;
        }
        for (const auto &arc : positiveArcs) {
            if (arc.first() != -1) {
                getOrUpdateStackScore(arc.first(), arc.second(), arc.third(), 1);
            }
            getOrUpdateStackScore(arc.second(), arc.third(), 1);
        }
        for (const auto &arc : negativeArcs) {
            if (arc.first() != -1) {
                getOrUpdateStackScore(arc.first(), arc.second(), arc.third(), -1);
            }
            getOrUpdateStackScore(arc.second(), arc.third(), -1);
        }
    }

    void DepParser::generate(DependencyTree *retval, const DependencyTree &correct) {
        for (int i = 0; i < m_nSentenceLength; ++i) {
            retval->push_back(DependencyTreeNode(std::get<0>(correct[i]), -1, "NMOD"));
        }
        for (const auto &arc : m_vecTrainArcs) {
            std::get<1>(retval->at(arc.second())) = arc.first();
        }
    }

    void DepParser::goldCheck() {
        Arcs2BiArcs(m_vecTrainArcs, m_vecTrainBiArcs);
        if (m_vecCorrectArcs.size() != m_vecTrainArcs.size() ||
            m_lItems[m_nSentenceLength + 1][0].r2l[m_nSentenceLength].getScore() / GOLD_POS_SCORE !=
            2 * m_vecTrainArcs.size() - 1) {
            std::cout << "gold parse len error at " << m_nTrainingRound << std::endl;
            std::cout << "score is " << m_lItems[m_nSentenceLength + 1][0].r2l[m_nSentenceLength].getScore() <<
            std::endl;
            std::cout << "len is " << m_vecTrainArcs.size() << std::endl;
            ++m_nTotalErrors;
        }
        else {
            int i = 0;
            std::sort(m_vecCorrectArcs.begin(), m_vecCorrectArcs.end(),
                      [](const Arc &arc1, const Arc &arc2) { return arc1 < arc2; });
            std::sort(m_vecTrainArcs.begin(), m_vecTrainArcs.end(),
                      [](const Arc &arc1, const Arc &arc2) { return arc1 < arc2; });
            for (int n = m_vecCorrectArcs.size(); i < n; ++i) {
                if (m_vecCorrectArcs[i].first() != m_vecTrainArcs[i].first() ||
                    m_vecCorrectArcs[i].second() != m_vecTrainArcs[i].second()) {
                    break;
                }
            }
            if (i != m_vecCorrectArcs.size()) {
                std::cout << "gold parse tree error at " << m_nTrainingRound << std::endl;
                ++m_nTotalErrors;
            }
        }
    }

    const tscore &DepParser::arcScore(const int &h, const int &m) {
        if (m_nState == ParserState::GOLDTEST) {
            m_nRetval = m_setFirstGoldScore.find(BiGram<int>(h, m)) == m_setFirstGoldScore.end() ? GOLD_NEG_SCORE
                                                                                                 : GOLD_POS_SCORE;
            return m_nRetval;
        }
        m_nRetval = 0;
        getOrUpdateStackScore(h, m, 0);
        return m_nRetval;
    }

    const tscore &DepParser::biArcScore(const int &g, const int &h, const int &m) {
        if (m_nState == ParserState::GOLDTEST) {
            m_nRetval = m_setSecondGoldScore.find(TriGram<int>(g, h, m)) == m_setSecondGoldScore.end() ? GOLD_NEG_SCORE
                                                                                                       : GOLD_POS_SCORE;
            return m_nRetval;
        }
        m_nRetval = 0;
        getOrUpdateStackScore(g, h, m, 0);
        return m_nRetval;
    }

    void DepParser::initFirstOrderScore(const int &d) {
        for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
            m_lFirstOrderScore[ENCODE_L2R(i)] = arcScore(i, i + d - 1);
            m_lFirstOrderScore[ENCODE_R2L(i)] = arcScore(i + d - 1, i);
        }
    }

    void DepParser::initSecondOrderScore(const int &d) {
        for (int i = 0, max_i = m_nSentenceLength - d + 1; i < max_i; ++i) {
            int l = i + d - 1;
            for (int g = 0; g < i; ++g) {
                m_lSecondOrderScore[ENCODE_2ND_L2R(i, g)] = biArcScore(g, i, l);
                m_lSecondOrderScore[ENCODE_2ND_R2L(i, g)] = biArcScore(g, l, i);
            }
            for (int g = l + 1; g <= m_nSentenceLength; ++g) {
                m_lSecondOrderScore[ENCODE_2ND_L2R(i, g)] = biArcScore(g, i, l);
                m_lSecondOrderScore[ENCODE_2ND_R2L(i, g)] = biArcScore(g, l, i);
            }
        }
    }

    void DepParser::getOrUpdateStackScore(const int &h, const int &m, const int &amount) {

        Weight *cweight = (Weight *) m_Weight;

        h_1_tag = h > 0 ? m_lSentence[h - 1].second() : start_taggedword.second();
        h1_tag = h < m_nSentenceLength - 1 ? m_lSentence[h + 1].second() : end_taggedword.second();
        m_1_tag = m > 0 ? m_lSentence[m - 1].second() : start_taggedword.second();
        m1_tag = m < m_nSentenceLength - 1 ? m_lSentence[m + 1].second() : end_taggedword.second();

        h_word = m_lSentence[h].first();
        h_tag = m_lSentence[h].second();

        m_word = m_lSentence[m].first();
        m_tag = m_lSentence[m].second();

        m_nDis = encodeLinkDistanceOrDirection(h, m, false);
        m_nDir = encodeLinkDistanceOrDirection(h, m, true);

        word_int.refer(h_word, 0);
        cweight->m_mapPw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_int.referLast(m_nDis);
        cweight->m_mapPw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_int.referLast(m_nDir);
        cweight->m_mapPw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);

        tag_int.refer(h_tag, 0);
        cweight->m_mapPp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_int.referLast(m_nDis);
        cweight->m_mapPp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_int.referLast(m_nDir);
        cweight->m_mapPp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_tag_int.refer(h_word, h_tag, 0);
        cweight->m_mapPwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_int.referLast(m_nDis);
        cweight->m_mapPwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_int.referLast(m_nDir);
        cweight->m_mapPwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_int.refer(m_word, 0);
        cweight->m_mapCw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_int.referLast(m_nDis);
        cweight->m_mapCw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_int.referLast(m_nDir);
        cweight->m_mapCw.getOrUpdateScore(m_nRetval, word_int, m_nScoreIndex, amount, m_nTrainingRound);

        tag_int.refer(m_tag, 0);
        cweight->m_mapCp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_int.referLast(m_nDis);
        cweight->m_mapCp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_int.referLast(m_nDir);
        cweight->m_mapCp.getOrUpdateScore(m_nRetval, tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_tag_int.refer(m_word, m_tag, 0);
        cweight->m_mapCwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_int.referLast(m_nDis);
        cweight->m_mapCwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_int.referLast(m_nDir);
        cweight->m_mapCwp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_word_tag_tag_int.refer(h_word, m_word, h_tag, m_tag, 0);
        cweight->m_mapPwpCwp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount,
                                              m_nTrainingRound);
        word_word_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPwpCwp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount,
                                              m_nTrainingRound);
        word_word_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPwpCwp.getOrUpdateScore(m_nRetval, word_word_tag_tag_int, m_nScoreIndex, amount,
                                              m_nTrainingRound);

        word_tag_tag_int.refer(h_word, h_tag, m_tag, 0);
        cweight->m_mapPwpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPwpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPwpCp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_tag_tag_int.refer(m_word, h_tag, m_tag, 0);
        cweight->m_mapPpCwp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpCwp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpCwp.getOrUpdateScore(m_nRetval, word_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_word_tag_int.refer(h_word, m_word, h_tag, 0);
        cweight->m_mapPwpCw.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_word_tag_int.referLast(m_nDis);
        cweight->m_mapPwpCw.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_word_tag_int.referLast(m_nDir);
        cweight->m_mapPwpCw.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_word_tag_int.refer(h_word, m_word, m_tag, 0);
        cweight->m_mapPwCwp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_word_tag_int.referLast(m_nDis);
        cweight->m_mapPwCwp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_word_tag_int.referLast(m_nDir);
        cweight->m_mapPwCwp.getOrUpdateScore(m_nRetval, word_word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_word_int.refer(h_word, m_word, 0);
        cweight->m_mapPwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_word_int.referLast(m_nDis);
        cweight->m_mapPwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_word_int.referLast(m_nDir);
        cweight->m_mapPwCw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

        tag_tag_int.refer(h_tag, m_tag, 0);
        cweight->m_mapPpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpCp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_tag, h1_tag, m_1_tag, m_tag, 0);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_1_tag, h_tag, m_1_tag, m_tag, 0);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_tag, h1_tag, m_tag, m1_tag, 0);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_1_tag, h_tag, m_tag, m1_tag, 0);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        tag_tag_tag_tag_int.refer(empty_taggedword.second(), h1_tag, m_1_tag, m_tag, 0);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_1_tag, empty_taggedword.second(), m_1_tag, m_tag, 0);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);

        tag_tag_tag_tag_int.refer(empty_taggedword.second(), h1_tag, m_tag, m1_tag, 0);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_1_tag, empty_taggedword.second(), m_tag, m1_tag, 0);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_tag, h1_tag, m_1_tag, empty_taggedword.second(), 0);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_1_tag, h_tag, m_1_tag, empty_taggedword.second(), 0);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_tag, h1_tag, empty_taggedword.second(), m1_tag, 0);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_1_tag, h_tag, empty_taggedword.second(), m1_tag, 0);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_tag, empty_taggedword.second(), m_1_tag, m_tag, 0);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1Cp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        tag_tag_tag_tag_int.refer(empty_taggedword.second(), h_tag, m_1_tag, m_tag, 0);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCp_1Cp.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                    m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_tag, h1_tag, m_tag, empty_taggedword.second(), 0);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPpPp1CpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                  m_nTrainingRound);

        tag_tag_tag_tag_int.refer(h_1_tag, h_tag, m_tag, empty_taggedword.second(), 0);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDis);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);
        tag_tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapPp_1PpCpCp1.getOrUpdateScore(m_nRetval, tag_tag_tag_tag_int, m_nScoreIndex, amount,
                                                   m_nTrainingRound);

        for (int b = (int) std::fmin(h, m) + 1, e = (int) std::fmax(h, m); b < e; ++b) {
            b_tag = m_lSentence[b].second();
            tag_tag_tag_int.refer(h_tag, b_tag, m_tag, 0);
            cweight->m_mapPpBpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
            tag_tag_tag_int.referLast(m_nDis);
            cweight->m_mapPpBpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
            tag_tag_tag_int.referLast(m_nDir);
            cweight->m_mapPpBpCp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        }
    }

    void DepParser::getOrUpdateStackScore(const int &g, const int &h, const int &m, const int &amount) {
        Weight *cweight = (Weight *) m_Weight;

        g_word = m_lSentence[g].first();
        g_tag = m_lSentence[g].second();

        h_tag = m_lSentence[h].second();

        m_word = m_lSentence[m].first();
        m_tag = m_lSentence[m].second();

        m_nDir = encodeLinkDistanceOrDirection(g, h, true);

        tag_tag_tag_int.refer(g_tag, h_tag, m_tag, 0);
        cweight->m_mapGpHpMp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_tag_tag_int.referLast(m_nDir);
        cweight->m_mapGpHpMp.getOrUpdateScore(m_nRetval, tag_tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        tag_tag_int.refer(g_tag, m_tag, 0);
        cweight->m_mapGpMp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        tag_tag_int.referLast(m_nDir);
        cweight->m_mapGpMp.getOrUpdateScore(m_nRetval, tag_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_word_int.refer(g_word, m_word, 0);
        cweight->m_mapGwMw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_word_int.referLast(m_nDir);
        cweight->m_mapGwMw.getOrUpdateScore(m_nRetval, word_word_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_tag_int.refer(g_word, m_tag, 0);
        cweight->m_mapGwMp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_int.referLast(m_nDir);
        cweight->m_mapGwMp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);

        word_tag_int.refer(m_word, g_tag, 0);
        cweight->m_mapMwGp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
        word_tag_int.referLast(m_nDir);
        cweight->m_mapMwGp.getOrUpdateScore(m_nRetval, word_tag_int, m_nScoreIndex, amount, m_nTrainingRound);
    }
}
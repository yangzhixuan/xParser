#include "score.h"
#include "common/parser/macros_base.h"

Score::Score(const cscore & c, const tscore & t) : m_nCurrent(c), m_nTotal(t), m_nLastUpdate(0) {}

Score::Score(const Score & s) : m_nCurrent(s.m_nCurrent), m_nTotal(s.m_nTotal), m_nLastUpdate(s.m_nLastUpdate) {}

Score::~Score() = default;

void Score::reset() {
	m_nCurrent = 0;
	m_nTotal = 0;
	m_nLastUpdate = 0;
}

bool Score::empty() const {
	return m_nCurrent == 0 && m_nTotal == 0 && m_nLastUpdate == 0;
}

bool Score::zero() const {
	return m_nCurrent == 0 && m_nTotal == 0;
}

tscore Score::operator[](const int & which) const {
	switch (which) {
	case eNonAverage:
		return (tscore)m_nCurrent;
	case eAverage:
		return m_nTotal;
	default:
		return (tscore)m_nCurrent;
	}
}

void Score::updateCurrent(const int & added, const int & round) {
	if (round > m_nLastUpdate) {
		updateAverage(round);
		m_nLastUpdate = round;
	}
	m_nCurrent += added;
	m_nTotal += added;
}

void Score::updateAverage(const int & round) {
	if (round > m_nLastUpdate) {
		m_nTotal += m_nCurrent * (round - m_nLastUpdate);
	}
}

ScoreWithSplit::ScoreWithSplit(const int & sp, const tscore & sc) : m_nSplit(sp), m_nScore(sc) {}

ScoreWithSplit::ScoreWithSplit(const ScoreWithSplit & sws) : ScoreWithSplit(sws.m_nSplit, sws.m_nScore) {}

ScoreWithSplit::~ScoreWithSplit() = default;

const int & ScoreWithSplit::getSplit() const {
	return m_nSplit;
}

const tscore & ScoreWithSplit::getScore() const {
	return m_nScore;
}

void ScoreWithSplit::reset() {
	m_nSplit = -1;
	m_nScore = 0;
}

void ScoreWithSplit::refer(const int & sp, const tscore & sc) {
	m_nSplit = sp;
	m_nScore = sc;
}

ScoreWithSplit & ScoreWithSplit::operator=(const ScoreWithSplit & sws) {
	m_nSplit = sws.m_nSplit;
	m_nScore = sws.m_nScore;
	return *this;
}

bool ScoreWithSplit::operator<(const tscore & s) const {
	return m_nSplit == -1 || m_nScore < s;
}

bool ScoreWithSplit::operator<(const ScoreWithSplit & sws) const {
	return m_nScore < sws.m_nScore;
}

bool ScoreWithSplit::operator<=(const ScoreWithSplit & sws) const {
	return m_nScore <= sws.m_nScore;
}

bool ScoreWithSplit::operator>(const ScoreWithSplit & sws) const {
	return m_nScore > sws.m_nScore;
}

ScoreWithBiSplit::ScoreWithBiSplit(const int & sp, const int & isp, const tscore & sc) : m_nSplit(sp), m_nInnerSplit(isp), m_nScore(sc) {}

ScoreWithBiSplit::ScoreWithBiSplit(const ScoreWithBiSplit & swbs) : ScoreWithBiSplit(swbs.m_nSplit, swbs.m_nInnerSplit, swbs.m_nScore) {}

ScoreWithBiSplit::~ScoreWithBiSplit() = default;

const int & ScoreWithBiSplit::getSplit() const {
	return m_nSplit;
}

const int & ScoreWithBiSplit::getInnerSplit() const {
	return m_nInnerSplit;
}

const tscore & ScoreWithBiSplit::getScore() const {
	return m_nScore;
}

void ScoreWithBiSplit::reset() {
	m_nSplit = -1;
	m_nInnerSplit = -1;
	m_nScore = 0;
}

void ScoreWithBiSplit::refer(const int & sp, const int & isp, const tscore & sc) {
	m_nSplit = sp;
	m_nInnerSplit = isp;
	m_nScore = sc;
}

ScoreWithBiSplit & ScoreWithBiSplit::operator=(const ScoreWithBiSplit & swbs) {
	m_nSplit = swbs.m_nSplit;
	m_nInnerSplit = swbs.m_nInnerSplit;
	m_nScore = swbs.m_nScore;
	return *this;
}

bool ScoreWithBiSplit::operator<(const tscore & s) const {
	return m_nSplit == -1 || m_nScore < s;
}

bool ScoreWithBiSplit::operator<(const ScoreWithBiSplit & sws) const {
	if (m_nScore != sws.m_nScore) {
		return m_nScore < sws.m_nScore;
	}
	int l = IS_EMPTY(m_nSplit) ? 0 : 1;
	int r = IS_EMPTY(m_nSplit) ? 0 : 1;
	if (l != r || l == 0) {
		return l < r;
	}
	l = IS_EMPTY(m_nInnerSplit) ? 0 : 1;
	r = IS_EMPTY(m_nInnerSplit) ? 0 : 1;
	return l < r;
}

bool ScoreWithBiSplit::operator<=(const ScoreWithBiSplit & sws) const {
	if (m_nScore != sws.m_nScore) {
		return m_nScore < sws.m_nScore;
	}
	int l = IS_EMPTY(m_nSplit) ? 0 : 1;
	int r = IS_EMPTY(m_nSplit) ? 0 : 1;
	if (l != r || l == 0) {
		return l <= r;
	}
	l = IS_EMPTY(m_nInnerSplit) ? 0 : 1;
	r = IS_EMPTY(m_nInnerSplit) ? 0 : 1;
	return l <= r;
}

bool ScoreWithBiSplit::operator>(const ScoreWithBiSplit & sws) const {
	return !(*this <= sws);
}

ScoreWithType::ScoreWithType(const int & st, const tscore & sc) : m_nType(st), m_nScore(sc) {}

ScoreWithType::ScoreWithType(const ScoreWithType & swt) : ScoreWithType(swt.m_nType, swt.m_nScore) {}

ScoreWithType::~ScoreWithType() = default;

const int & ScoreWithType::getType() const {
	return m_nType;
}

const tscore & ScoreWithType::getScore() const {
	return m_nScore;
}

ScoreWithType & ScoreWithType::operator=(const ScoreWithType & sws) {
	m_nType = sws.m_nType;
	m_nScore = sws.m_nScore;
	return *this;
}

bool ScoreWithType::operator<(const ScoreWithType & swt) const {
	return m_nScore < swt.m_nScore;
}

bool ScoreWithType::operator<=(const ScoreWithType & swt) const {
	return m_nScore <= swt.m_nScore;
}

bool ScoreWithType::operator>(const ScoreWithType & swt) const {
	return m_nScore > swt.m_nScore;
}

ScoredAction::ScoredAction(const int & ac, const tscore & sc) : m_nAction(ac), m_nScore(sc) {}

ScoredAction::ScoredAction(const ScoredAction & sa) : ScoredAction(sa.m_nAction, sa.m_nScore) {}

ScoredAction::~ScoredAction() = default;

const int & ScoredAction::getAction() const {
	return m_nAction;
}

const tscore & ScoredAction::getScore() const {
	return m_nScore;
}

ScoredAction & ScoredAction::operator=(const ScoredAction & sa) {
	m_nAction = sa.m_nAction;
	m_nScore = sa.m_nScore;
	return *this;
}

bool ScoredAction::operator<(const ScoredAction & sa) const {
	return m_nScore < sa.m_nScore;
}

bool ScoredAction::operator<=(const ScoredAction & sa) const {
	return m_nScore <= sa.m_nScore;
}

bool ScoredAction::operator>(const ScoredAction & sa) const {
	return m_nScore > sa.m_nScore;
}
#ifndef _SCORE_H
#define _SCORE_H

#include <string>
#include <iostream>

#include "common/token/token.h"

typedef int cscore;
typedef long long tscore;

enum ScoreType {
	eNonAverage,
	eAverage,
};

class Score {

private:
	cscore m_nCurrent;
	tscore m_nTotal;

protected:
	int m_nLastUpdate;

public:
	Score(const cscore & c = 0, const tscore & t = 0);
	Score(const Score & s);
	~Score();
	
	void reset();
	bool empty() const;
	bool zero() const;
	tscore operator[](const int & which) const;
	void updateCurrent(const int & added = 0, const int & round = 0);
	void updateAverage(const int & round);

	friend std::istream & operator>>(std::istream & is, Score & s) {
		ttoken token;
		is >> s.m_nCurrent >> token >> s.m_nTotal;
		return is;
	}

	friend std::ostream & operator<<(std::ostream & os, const Score & s) {
		os << s.m_nCurrent << " / " << s.m_nTotal;
		return os;
	}
};

class ScoreWithSplit {
private:
	int m_nSplit;
	tscore m_nScore;

public:
	ScoreWithSplit(const int & sp = -1, const tscore & sc = 0);
	ScoreWithSplit(const ScoreWithSplit & sws);
	~ScoreWithSplit();

	void reset();
	const int & getSplit() const;
	const tscore & getScore() const;
	void refer(const int & sp, const tscore & sc);
	ScoreWithSplit & operator=(const ScoreWithSplit & sws);

	bool operator<(const tscore & sc) const;
	bool operator<(const ScoreWithSplit & sws) const;
	bool operator<=(const ScoreWithSplit & sws) const;
	bool operator>(const ScoreWithSplit & sws) const;
};

class ScoreWithBiSplit {
private:
	int m_nSplit;
	int m_nInnerSplit;
	tscore m_nScore;

public:
	ScoreWithBiSplit(const int & sp = -1, const int & isp = -1, const tscore & sc = 0);
	ScoreWithBiSplit(const ScoreWithBiSplit & swbs);
	~ScoreWithBiSplit();

	void reset();
	const int & getSplit() const;
	const int & getInnerSplit() const;
	const tscore & getScore() const;
	void refer(const int & sp, const int & isp, const tscore & sc);
	ScoreWithBiSplit & operator=(const ScoreWithBiSplit & swbs);

	bool operator<(const tscore & sc) const;
	bool operator<(const ScoreWithBiSplit & sws) const;
	bool operator<=(const ScoreWithBiSplit & sws) const;
	bool operator>(const ScoreWithBiSplit & sws) const;
};

class ScoreWithType {
private:
	int m_nType;
	tscore m_nScore;

public:
	ScoreWithType(const int & st = 1, const tscore & sc = 0);
	ScoreWithType(const ScoreWithType & swt);
	~ScoreWithType();

	const int & getType() const;
	const tscore & getScore() const;
	ScoreWithType & operator=(const ScoreWithType & swt);

	bool operator<(const ScoreWithType & swt) const;
	bool operator<=(const ScoreWithType & swt) const;
	bool operator>(const ScoreWithType & swt) const;
};

class ScoredAction {
private:
	int m_nAction;
	tscore m_nScore;

public:
	ScoredAction(const int & ac = 0, const tscore & sc = 0);
	ScoredAction(const ScoredAction & sa);
	~ScoredAction();

	const int & getAction() const;
	const tscore & getScore() const;
	ScoredAction & operator=(const ScoredAction & sa);

	bool operator<(const ScoredAction & swt) const;
	bool operator<=(const ScoredAction & swt) const;
	bool operator>(const ScoredAction & swt) const;
};

#endif
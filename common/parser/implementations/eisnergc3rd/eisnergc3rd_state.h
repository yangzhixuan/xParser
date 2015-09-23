#ifndef _EISNERGC3RD_STATE_H
#define _EISNERGC3RD_STATE_H

#include "eisnergc3rd_macros.h"
#include "common/parser/agenda.h"
#include "include/learning/perceptron/score.h"

namespace eisnergc3rd {

	class StateItem {
	public:
		int type;
		int left, right;
		std::vector<ScoreWithSplit> jux;
		std::vector<ScoreAgenda> l2r_solid_both, r2l_solid_both;
		std::vector<ScoreWithSplit> l2r, r2l;

	public:
		static enum STATE{
			JUX = 1,
			L2R_SOLID_BOTH,
			R2L_SOLID_BOTH,
			L2R,
			R2L
		};

		StateItem();
		~StateItem();

		void init(const int & l, const int & r, const int & len);

		void updateJUX(const int & grand, const int & split, const tscore & score);
		void updateL2RSolidBoth(const int & grand, const int & split, const int & innersplit, const tscore & score);
		void updateR2LSolidBoth(const int & grand, const int & split, const int & innersplit, const tscore & score);
		void updateL2R(const int & grand, const int & split, const tscore & score);
		void updateR2L(const int & grand, const int & split, const tscore & score);

		void print(const int & grand);
	};
}

#endif
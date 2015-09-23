#ifndef _EISNERGC_STATE_H
#define _EISNERGC_STATE_H

#include "eisnergc_macros.h"
#include "include/learning/perceptron/score.h"

namespace eisnergc {

	class StateItem {
	public:
		int type;
		int left, right;
		ScoreWithSplit l2r[MAX_SENTENCE_SIZE], r2l[MAX_SENTENCE_SIZE], l2r_im[MAX_SENTENCE_SIZE], r2l_im[MAX_SENTENCE_SIZE];

	public:
		static enum State {
			L2R_COMP = 1,
			R2L_COMP,
			L2R_IM_COMP,
			R2L_IM_COMP
		};

		StateItem();
		~StateItem();

		void init(const int & l, const int & r, const int & len);

		void updateL2R(const int & grand, const int & split, const tscore & score);
		void updateR2L(const int & grand, const int & split, const tscore & score);
		void updateL2RIm(const int & grand, const int & split, const tscore & score);
		void updateR2LIm(const int & grand, const int & split, const tscore & score);

		void print(const int & grand);
	};
}

#endif
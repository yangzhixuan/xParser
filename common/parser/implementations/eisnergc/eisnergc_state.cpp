#include "eisnergc_state.h"

namespace eisnergc {
	StateItem::StateItem() = default;
	StateItem::~StateItem() = default;

	void StateItem::init(const int & l, const int & r, const int & len) {
		type = 0;
		left = l;
		right = r;
		for (int i = 0; i < l; ++i) {
			l2r[i].reset();
			r2l[i].reset();
			l2r_im[i].reset();
			r2l_im[i].reset();
		}
		for (int i = r + 1; i <= len; ++i) {
			l2r[i].reset();
			r2l[i].reset();
			l2r_im[i].reset();
			r2l_im[i].reset();
		}
	}

	void StateItem::updateL2R(const int & grand, const int & split, const tscore & score) {
		if (l2r[grand] < score) {
			l2r[grand].refer(split, score);
		}
	}

	void StateItem::updateR2L(const int & grand, const int & split, const tscore & score) {
		if (r2l[grand] < score) {
			r2l[grand].refer(split, score);
		}
	}

	void StateItem::updateL2RIm(const int & grand, const int & split, const tscore & score) {
		if (l2r_im[grand] < score) {
			l2r_im[grand].refer(split, score);
		}
	}

	void StateItem::updateR2LIm(const int & grand, const int & split, const tscore & score) {
		if (r2l_im[grand] < score) {
			r2l_im[grand].refer(split, score);
		}
	}

	void StateItem::print(const int & grand) {
		std::cout << "[" << left << "," << right << "]" << std::endl;
		std::cout << "type is: ";
		switch (type) {
		case L2R_COMP:
			std::cout << "L2R_COMP" << std::endl;
			std::cout << "split: " << l2r[grand].getSplit() << " score: " << l2r[grand].getScore() << std::endl;
			break;
		case R2L_COMP:
			std::cout << "R2L_COMP" << std::endl;
			std::cout << "split: " << r2l[grand].getSplit() << " score: " << r2l[grand].getScore() << std::endl;
			break;
		case L2R_IM_COMP:
			std::cout << "L2R_IM_COMP" << std::endl;
			std::cout << "split: " << l2r_im[grand].getSplit() << " score: " << l2r_im[grand].getScore() << std::endl;
			break;
		case R2L_IM_COMP:
			std::cout << "R2L_IM_COMP" << std::endl;
			std::cout << "split: " << r2l_im[grand].getSplit() << " score: " << r2l_im[grand].getScore() << std::endl;
			break;
		default:
			std::cout << "ZERO" << std::endl;
			std::cout << "L2R_COMP" << std::endl;
			std::cout << "split: " << l2r[grand].getSplit() << " score: " << l2r[grand].getScore() << std::endl;
			std::cout << "R2L_COMP" << std::endl;
			std::cout << "split: " << r2l[grand].getSplit() << " score: " << r2l[grand].getScore() << std::endl;
			std::cout << "L2R_IM_COMP" << std::endl;
			std::cout << "split: " << l2r_im[grand].getSplit() << " score: " << l2r_im[grand].getScore() << std::endl;
			std::cout << "R2L_IM_COMP" << std::endl;
			std::cout << "split: " << r2l_im[grand].getSplit() << " score: " << r2l_im[grand].getScore() << std::endl;
			break;
		}
	}
}
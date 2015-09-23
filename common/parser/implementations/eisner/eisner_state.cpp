#include "eisner_state.h"

namespace eisner {
	StateItem::StateItem() = default;
	StateItem::~StateItem() = default;

	void StateItem::init(const int & l, const int & r) {
		type = 0;
		left = l;
		right = r;
		l2r.reset();
		r2l.reset();
		l2r_im.reset();
		r2l_im.reset();
	}

	void StateItem::updateL2R(const int & split, const tscore & score) {
		if (l2r < score) {
			l2r.refer(split, score);
		}
	}

	void StateItem::updateR2L(const int & split, const tscore & score) {
		if (r2l < score) {
			r2l.refer(split, score);
		}
	}

	void StateItem::updateL2RIm(const int & split, const tscore & score) {
		if (l2r_im < score) {
			l2r_im.refer(split, score);
		}
	}

	void StateItem::updateR2LIm(const int & split, const tscore & score) {
		if (r2l_im < score) {
			r2l_im.refer(split, score);
		}
	}

	void StateItem::print() {
		std::cout << "[" << left << "," << right << "]" << std::endl;
		std::cout << "type is: ";
		switch (type) {
		case L2R_COMP:
			std::cout << "L2R_COMP" << std::endl;
			std::cout << "split: " << l2r.getSplit() << " score: " << l2r.getScore() << std::endl;
			break;
		case R2L_COMP:
			std::cout << "R2L_COMP" << std::endl;
			std::cout << "split: " << r2l.getSplit() << " score: " << r2l.getScore() << std::endl;
			break;
		case L2R_IM_COMP:
			std::cout << "L2R_IM_COMP" << std::endl;
			std::cout << "split: " << l2r.getSplit() << " score: " << l2r.getScore() << std::endl;
			break;
		case R2L_IM_COMP:
			std::cout << "R2L_IM_COMP" << std::endl;
			std::cout << "split: " << r2l.getSplit() << " score: " << r2l.getScore() << std::endl;
			break;
		default:
			std::cout << "ZERO" << std::endl;
			std::cout << "L2R_COMP" << std::endl;
			std::cout << "split: " << l2r.getSplit() << " score: " << l2r.getScore() << std::endl;
			std::cout << "R2L_COMP" << std::endl;
			std::cout << "split: " << r2l.getSplit() << " score: " << r2l.getScore() << std::endl;
			std::cout << "L2R_IM_COMP" << std::endl;
			std::cout << "split: " << l2r.getSplit() << " score: " << l2r.getScore() << std::endl;
			std::cout << "R2L_IM_COMP" << std::endl;
			std::cout << "split: " << r2l.getSplit() << " score: " << r2l.getScore() << std::endl;
			break;
		}
	}
}
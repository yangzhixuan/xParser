#include "emptyeisner_state.h"

namespace emptyeisner {
	StateItem::StateItem() = default;
	StateItem::~StateItem() = default;

	void StateItem::init(const int & l, const int & r) {
		type = 0;
		left = l;
		right = r;
		jux.reset();
		l2r_solid_both.clear();
		r2l_solid_both.clear();
		l2r_empty_outside.clear();
		r2l_empty_outside.clear();
		l2r_solid_outside.clear();
		r2l_solid_outside.clear();
		l2r_empty_inside.clear();
		r2l_empty_inside.clear();
		l2r.reset();
		r2l.reset();
	}

	void StateItem::updateJUX(const int & split, const tscore & score) {
		if (jux < score) {
			jux.refer(split, score);
		}
	}

	void StateItem::updateL2RSolidBoth(const int & split, const int & innersplit, const tscore & score) {
		l2r_solid_both.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	void StateItem::updateR2LSolidBoth(const int & split, const int & innersplit, const tscore & score) {
		r2l_solid_both.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	void StateItem::updateL2REmptyInside(const int & split, const int & innersplit, const tscore & score) {
		l2r_empty_inside.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	void StateItem::updateR2LEmptyInside(const int & split, const int & innersplit, const tscore & score) {
		r2l_empty_inside.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	void StateItem::updateL2RSolidOutside(const int & split, const int & innersplit, const tscore & score) {
		l2r_solid_outside.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	void StateItem::updateR2LSolidOutside(const int & split, const int & innersplit, const tscore & score) {
		r2l_solid_outside.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	void StateItem::updateL2REmptyOutside(const int & split, const int & innersplit, const tscore & score) {
		l2r_empty_outside.insertItem(ScoreWithBiSplit(split, innersplit, score));
	}

	void StateItem::updateR2LEmptyOutside(const int & split, const int & innersplit, const tscore & score) {
		r2l_empty_outside.insertItem(ScoreWithBiSplit(split, innersplit, score));
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

	void StateItem::print() {
		std::cout << "[" << left << "," << right << "]" << std::endl;
		std::cout << "type is: ";
		switch (type) {
		case JUX:
			std::cout << "JUX" << std::endl;
			std::cout << "split: " << jux.getSplit() << " score: " << jux.getScore() << std::endl;
			break;
		case L2R:
			std::cout << "L2R" << std::endl;
			std::cout << "split: " << l2r.getSplit() << " score: " << l2r.getScore() << std::endl;
			break;
		case R2L:
			std::cout << "R2L" << std::endl;
			std::cout << "split: " << r2l.getSplit() << " score: " << r2l.getScore() << std::endl;
			break;
		case L2R_SOLID_BOTH:
			std::cout << "L2R_SOLID_BOTH" << std::endl;
			for (const auto & agenda : l2r_solid_both) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		case R2L_SOLID_BOTH:
			std::cout << "R2L_SOLID_BOTH" << std::endl;
			for (const auto & agenda : r2l_solid_both) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		case L2R_SOLID_OUTSIDE:
			std::cout << "L2R_SOLID_OUTSIDE" << std::endl;
			for (const auto & agenda : l2r_solid_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		case R2L_SOLID_OUTSIDE:
			std::cout << "R2L_SOLID_OUTSIDE" << std::endl;
			for (const auto & agenda : r2l_solid_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		case L2R_EMPTY_INSIDE:
			std::cout << "L2R_EMPTY_INSIDE" << std::endl;
			for (const auto & agenda : l2r_empty_inside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		case R2L_EMPTY_INSIDE:
			std::cout << "R2L_EMPTY_INSIDE" << std::endl;
			for (const auto & agenda : r2l_empty_inside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		case L2R_EMPTY_OUTSIDE:
			std::cout << "L2R_EMPTY_OUTSIDE" << std::endl;
			for (const auto & agenda : l2r_empty_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		case R2L_EMPTY_OUTSIDE:
			std::cout << "R2L_EMPTY_OUTSIDE" << std::endl;
			for (const auto & agenda : r2l_empty_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		default:
			std::cout << "ZERO" << std::endl;
			std::cout << "JUX" << std::endl;
			std::cout << "split: " << jux.getSplit() << " score: " << jux.getScore() << std::endl;
			std::cout << "L2R" << std::endl;
			std::cout << "split: " << l2r.getSplit() << " score: " << l2r.getScore() << std::endl;
			std::cout << "R2L" << std::endl;
			std::cout << "split: " << r2l.getSplit() << " score: " << r2l.getScore() << std::endl;
			std::cout << "L2R_SOLID_BOTH" << std::endl;
			for (const auto & agenda : l2r_solid_both) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			std::cout << "R2L_SOLID_BOTH" << std::endl;
			for (const auto & agenda : r2l_solid_both) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			std::cout << "L2R_SOLID_OUTSIDE" << std::endl;
			for (const auto & agenda : l2r_solid_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			std::cout << "R2L_SOLID_OUTSIDE" << std::endl;
			for (const auto & agenda : r2l_solid_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			std::cout << "L2R_EMPTY_INSIDE" << std::endl;
			for (const auto & agenda : l2r_empty_inside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			std::cout << "R2L_EMPTY_INSIDE" << std::endl;
			for (const auto & agenda : r2l_empty_inside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			std::cout << "L2R_EMPTY_OUTSIDE" << std::endl;
			for (const auto & agenda : l2r_empty_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			std::cout << "R2L_EMPTY_OUTSIDE" << std::endl;
			for (const auto & agenda : r2l_empty_outside) {
				std::cout << "split: " << agenda.getSplit() << " innersplit: " << agenda.getInnerSplit() << " score: " << agenda.getScore() << std::endl;
			}
			break;
		}
	}
}
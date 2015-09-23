#include "arceager_state.h"
#include "common/token/deplabel.h"

namespace arceager {
	StateItem::StateItem() {
		clear();
	}

	StateItem::~StateItem() = default;

	void StateItem::arcLeft(const int & l) {
		int left = m_lStack[m_nStackBack--];
		--m_nHeadStackBack;
		m_lHeads[left] = m_nNextWord;
		m_lLabels[left] = l;
		m_lDepTagL[m_nNextWord].add(l);
		m_lSibling[left] = m_lDepsL[m_nNextWord];
		m_lDepsL[m_nNextWord] = left;
		++m_lDepsNumL[m_nNextWord];
		m_nLastAction = ENCODE_ACTION(ARC_LEFT, l);
	}

	void StateItem::arcRight(const int & l) {
		int left = m_lStack[m_nStackBack];
		m_lStack[++m_nStackBack] = m_nNextWord;
		m_lHeads[m_nNextWord] = left;
		m_lLabels[m_nNextWord] = l;
		m_lDepTagR[left].add(l);
		m_lSibling[m_nNextWord] = m_lDepsR[left];
		m_lDepsR[left] = m_nNextWord++;
		++m_lDepsNumR[left];
		clearNext();
		m_nLastAction = ENCODE_ACTION(ARC_RIGHT, l);
	}

	void StateItem::shift() {
		m_lStack[++m_nStackBack] = m_nNextWord;
		m_lHeadStack[++m_nHeadStackBack] = m_nNextWord++;
		clearNext();
		m_nLastAction = SHIFT;
	}

	void StateItem::reduce() {
		--m_nStackBack;
		m_nLastAction = REDUCE;
	}

	void StateItem::popRoot() {
		m_lLabels[m_lStack[m_nStackBack--]] = TDepLabel::ROOT;
		m_nLastAction = POP_ROOT;
	}

	void StateItem::clear() {
		m_nNextWord = 0;
		m_nStackBack = -1;
		m_nHeadStackBack = -1;
		score = 0;
		m_nLastAction = NO_ACTION;
		clearNext();
	}

	void StateItem::clearNext() {
		m_lHeads[m_nNextWord] = -1;
		m_lDepsL[m_nNextWord] = -1;
		m_lDepsR[m_nNextWord] = -1;
		m_lDepsNumL[m_nNextWord] = 0;
		m_lDepsNumR[m_nNextWord] = 0;
		m_lDepTagL[m_nNextWord].clear();
		m_lDepTagR[m_nNextWord].clear();
		m_lSibling[m_nNextWord] = -1;
		m_lLabels[m_nNextWord] = 0;
	}

	void StateItem::move(const int & action) {
		switch (DECODE_ACTION(action)) {
		case NO_ACTION:
			return;
		case SHIFT:
			shift();
			return;
		case REDUCE:
			reduce();
			return;
		case ARC_LEFT:
			arcLeft(DECODE_LABEL(action));
			return;
		case ARC_RIGHT:
			arcRight(DECODE_LABEL(action));
			return;
		case POP_ROOT:
			popRoot();
			return;
		}
	}

	void StateItem::generateTree(const Sentence & sent, DependencyTree & tree) {
		int i = 0;
		tree.clear();
		for (const auto & token : sent) {
			tree.push_back(DependencyTreeNode(token, m_lHeads[i], TDepLabel::key(m_lLabels[i])));
			++i;
		}
	}

	bool StateItem::standardMove(const DependencyTree & tree) {
		int top;
		if (m_nNextWord == tree.size()) {
			if (m_nStackBack > 0) {
				reduce();
				return true;
			}
			else {
				popRoot();
				return true;
			}
		}
		if (m_nStackBack >= 0) {
			top = m_lStack[m_nStackBack];
			while (!(m_lHeads[top] == -1)) {
				top = m_lHeads[top];
			}
			if (TREENODE_HEAD(tree[top]) == m_nNextWord) {
				if (top == m_lStack[m_nStackBack]) {
					arcLeft(TDepLabel::code(TREENODE_LABEL(tree[top])));
					return true;
				}
				else {
					reduce();
					return true;
				}
			}
		}
		if (TREENODE_HEAD(tree[m_nNextWord]) == -1 || TREENODE_HEAD(tree[m_nNextWord]) > m_nNextWord) {
			shift();
			return true;
		}
		else {
			top = m_lStack[m_nStackBack];
			if (TREENODE_HEAD(tree[m_nNextWord]) == top) {
				arcRight(TDepLabel::code(TREENODE_LABEL(tree[m_nNextWord])));
				return true;
			}
			else {
				reduce();
				return true;
			}
		}
	}

	int StateItem::followMove(const StateItem & item) {
		int top;
		if (m_nNextWord == item.m_nNextWord) {
			top = m_lStack[m_nStackBack];
			if (item.m_lHeads[top] == m_nNextWord) {
				return ENCODE_ACTION(ARC_LEFT, item.m_lLabels[top]);
			}
			else if (item.m_lHeads[top] != -1) {
				return REDUCE;
			}
			else {
				return POP_ROOT;
			}
		}
		if (m_nStackBack >= 0) {
			top = m_lStack[m_nStackBack];
			while (!(m_lHeads[top] == -1)) {
				top = m_lHeads[top];
			}
			if (item.m_lHeads[top] == m_nNextWord) {
				if (top == m_lStack[m_nStackBack]) {
					ENCODE_ACTION(ARC_LEFT, item.m_lLabels[top]);
				}
				else {
					return REDUCE;
				}
			}
		}
		if (item.m_lHeads[m_nNextWord] == -1 || item.m_lHeads[m_nNextWord] > m_nNextWord) {
			return SHIFT;
		}
		else {
			top = m_lStack[m_nStackBack];
			if (item.m_lHeads[m_nNextWord] == top) {
				ENCODE_ACTION(ARC_RIGHT, item.m_lLabels[m_nNextWord]);
			}
			else {
				return REDUCE;
			}
		}
	}
}
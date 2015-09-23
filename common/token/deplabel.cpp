#include "deplabel.h"

Token TDepLabel::tokenizer = Token(TDepLabel::ROOT);

TDepLabel::TDepLabel() = default;

TDepLabel::~TDepLabel() = default;

int TDepLabel::start() {
	return TDepLabel::ROOT;
}

int TDepLabel::count() {
	return TDepLabel::tokenizer.count();
}

int TDepLabel::code(const ttoken & s) {
	return TDepLabel::tokenizer.lookup(s);
}

const ttoken & TDepLabel::key(int index) {
	return TDepLabel::tokenizer.key(index);
}

Token & TDepLabel::getTokenizer() {
	return tokenizer;
}
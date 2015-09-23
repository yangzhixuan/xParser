#include "pos.h"

Token TPOSTag::tokenizer = Token(1);

TPOSTag::TPOSTag() = default;

TPOSTag::~TPOSTag() = default;

int TPOSTag::count() {
	return TPOSTag::tokenizer.count();
}

int TPOSTag::code(const ttoken & s) {
	return TPOSTag::tokenizer.lookup(s);
}

const ttoken & TPOSTag::key(int index) {
	return TPOSTag::tokenizer.key(index);
}

Token & TPOSTag::getTokenizer() {
	return tokenizer;
}
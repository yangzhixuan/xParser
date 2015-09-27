#include "emp.h"

Token TEmptyTag::tokenizer = Token(TEmptyTag::START);

TEmptyTag::TEmptyTag() = default;

TEmptyTag::~TEmptyTag() = default;

int TEmptyTag::start() {
    return TEmptyTag::START;
}

int TEmptyTag::count() {
    return TEmptyTag::tokenizer.count();
}

int TEmptyTag::code(const ttoken &s) {
    return TEmptyTag::tokenizer.lookup(s);
}

const ttoken &TEmptyTag::key(int index) {
    return TEmptyTag::tokenizer.key(index);
}

Token &TEmptyTag::getTokenizer() {
    return tokenizer;
}
#include "word.h"

Token TWord::tokenizer = Token(1);

TWord::TWord() = default;

TWord::~TWord() = default;

int TWord::code(const ttoken &s) {
    return tokenizer.lookup(s);
}

int TWord::count() {
    return TWord::tokenizer.count();
}

const ttoken &TWord::key(int index) {
    return TWord::tokenizer.key(index);
}

Token &TWord::getTokenizer() {
    return tokenizer;
}
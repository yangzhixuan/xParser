#ifndef _WORD_H
#define _WORD_H

#include "token.h"

class TWord {
protected:
    static Token tokenizer;
    static const int UNKNOWN = 0;

public:
    TWord();

    ~TWord();

    static int code(const ttoken &s);

    static const ttoken &key(int token);

    static int count();

    static Token &getTokenizer();
};

#endif
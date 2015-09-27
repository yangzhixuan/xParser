#ifndef _EMP_H
#define _EMP_H

#include "token.h"

class TEmptyTag {
protected:
    static Token tokenizer;
    static const int UNKNOWN = 0;
    static const int START = 1;

public:
    TEmptyTag();

    ~TEmptyTag();

    static int code(const ttoken &s);

    static const ttoken &key(int token);

    static int count();

    static int start();

    static Token &getTokenizer();
};

#endif
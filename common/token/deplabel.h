#ifndef _DEPLABEL_H
#define _DEPLABEL_H
#include "token.h"

class TDepLabel {
protected:
	static Token tokenizer;
	static const int UNKNOWN = 0;

public:
	TDepLabel();
	~TDepLabel();

	static const int ROOT = 1;
	static int code(const ttoken & s);
	static const ttoken & key(int token);
	static int count();
	static int start();

	static Token & getTokenizer();
};

#endif
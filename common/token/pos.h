#ifndef _POS_H
#define _POS_H
#include "token.h"

class TPOSTag {
protected:
	static Token tokenizer;
	static const int UNKNOWN = 0;

public:
	TPOSTag();
	~TPOSTag();

	static int code(const ttoken & s);
	static const ttoken & key(int token);
	static int count();

	static Token & getTokenizer();
};

#endif
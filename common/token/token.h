#ifndef _TOKEN_H
#define _TOKEN_H

#include <vector>
#include <iostream>
#include <unordered_map>

typedef std::string ttoken;

class Token {
protected:
    std::unordered_map<ttoken, int> m_mapTokens;
    std::vector<ttoken> m_vecKeys;
    int m_nStartingToken;
    int m_nWaterMark;

public:
    Token(int nTokenStartFrom);

    int lookup(const ttoken &key);

    int find(const ttoken &key, int val = 0);

    const ttoken &key(int token) const;

    int count() const;

    friend std::istream &operator>>(std::istream &is, Token &token);

    friend std::ostream &operator<<(std::ostream &os, const Token &token);
};

#endif
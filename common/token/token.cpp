#include <string>

#include "token.h"

Token::Token(int nTokenStartFrom) {
    m_nWaterMark = m_nStartingToken = nTokenStartFrom;
}

int Token::lookup(const ttoken &key) {
    if (m_mapTokens.find(key) == m_mapTokens.end()) {
        m_mapTokens[key] = m_nWaterMark++;
        m_vecKeys.push_back(key);
    }
    return m_mapTokens[key];
}

int Token::find(const ttoken &key, int val) {
    return m_mapTokens.find(key) == m_mapTokens.end() ? val : m_mapTokens[key];
}

const ttoken &Token::key(int token) const {
    return m_vecKeys[token - m_nStartingToken];
}

int Token::count() const {
    return m_nWaterMark;
}

std::istream &operator>>(std::istream &is, Token &token) {
    int count;
    is >> count;
    ttoken t;
    while (count--) {
        is >> t;
        token.lookup(t);
    }
    return is;
}

std::ostream &operator<<(std::ostream &os, const Token &token) {
    os << token.m_vecKeys.size() << std::endl;
    for (auto t : token.m_vecKeys) {
        os << t << std::endl;
    }
    return os;
}
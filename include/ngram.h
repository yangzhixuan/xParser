#ifndef _NGRAM_H
#define _NGRAM_H

#include <string>
#include <iostream>
#include <common/token/token.h>
template<typename LABEL, typename HASH>
class SetOfLabels {
public:
	HASH m_nHash;

public:
	SetOfLabels(HASH h = 0) : m_nHash(h) {}
	SetOfLabels(const SetOfLabels<LABEL, HASH> & sol) : m_nHash(sol.m_nHash) {}
	~SetOfLabels() = default;

	void add(const LABEL & l) {
		m_nHash |= (1 << l);
	}

	void remove(const LABEL & l) {
		m_nHash &= ~(1 << l);
	}

	void clear() {
		m_nHash = 0;
	}

	bool contains(const LABEL & l) {
		return m_nHash & (1 << l);
	}

	std::size_t hash() const {
		return m_nHash;
	}

	SetOfLabels<LABEL, HASH> & operator=(const SetOfLabels<LABEL, HASH> & sol) {
		m_nHash = sol.m_nHash;
		return *this;
	}

	friend bool operator==(const SetOfLabels<LABEL, HASH> & sol1, const SetOfLabels<LABEL, HASH> & sol2) {
		return sol1.m_nHash == sol2.m_nHash;
	}
};

template<int SIZE, int WIDE, typename TAG>
class SetOfTags {
public:
	TAG m_nHash;

public:
	SetOfTags(int h = 0) : m_nHash(h) {}
	SetOfTags(const SetOfTags<SIZE, WIDE, TAG> & sot) : m_nHash(sot.m_nHash) {}
	~SetOfTags() = default;

	void add(const TAG & t) {
		m_nHash = (m_nHash << WIDE) | t;
	}

	std::size_t hash() const {
		return m_nHash;
	}	

	SetOfTags<SIZE, WIDE, TAG> & operator=(const SetOfTags<SIZE, WIDE, TAG> & sot) {
		m_nHash = sot.m_nHash;
		return *this;
	}

	friend bool operator==(const SetOfTags<SIZE, WIDE, TAG> & sot1, const SetOfTags<SIZE, WIDE, TAG> & sot2) {
		return sot1.m_nHash == sot2.m_nHash;
	}
};

// 64-bit version hash_combine
template <class T> inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	const std::size_t kMul = 0x9ddfea08eb382d69ULL;
	std::size_t a = (hasher(v) ^ seed) * kMul;
	a ^= (a >> 47);
	std::size_t b = (seed ^ a) * kMul;
	b ^= (b >> 47);
	seed = b * kMul;
}

template<typename KEY_TYPE>
class BiGram {
public:
	KEY_TYPE gram1, gram2;
	std::size_t m_nHash;

public:
	BiGram(const BiGram<KEY_TYPE> & bigram) : m_nHash(bigram.m_nHash), gram1(bigram.gram1), gram2(bigram.gram2) {}

	BiGram(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0) : gram1(_gram1), gram2(_gram2) {
		computeHash();
	}

	~BiGram() = default;

	void refer(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0) {
		gram1 = _gram1;
		gram2 = _gram2;
		computeHash();
	}

	void referLast(KEY_TYPE _gram2 = 0) {
		gram2 = _gram2;
		computeHash();
	}

	std::size_t hash() const {
		return m_nHash;
	}

	void computeHash() {
		std::size_t seed = 0;
		hash_combine<KEY_TYPE>(seed, gram1);
		hash_combine<KEY_TYPE>(seed, gram2);
		m_nHash = seed;
	}

	const KEY_TYPE & first() const {
		return gram1;
	}

	const KEY_TYPE & second() const {
		return gram2;
	}

	BiGram<KEY_TYPE> & operator=(const BiGram<KEY_TYPE> & gram) {
		gram1 = gram.gram1;
		gram2 = gram.gram2;
		computeHash();
		return *this;
	}

	friend std::istream & operator>>(std::istream & is, BiGram<KEY_TYPE>& bigram) {
		ttoken token;
		is >> token >> bigram.gram1 >> bigram.gram2 >> token;
		bigram.computeHash();
		return is;
	}
	friend std::ostream & operator<<(std::ostream & os, const BiGram<KEY_TYPE>& bigram) {
		os << "[ " << bigram.gram1 << " " << bigram.gram2 << " ]";
		return os;
	}
	friend bool operator==(const BiGram<KEY_TYPE> g1, const BiGram<KEY_TYPE> g2) {
		return g1.m_nHash == g2.m_nHash && g1.gram1 == g2.gram1 && g1.gram2 == g2.gram2;
	}
};

template<typename KEY_TYPE>
class TriGram {
public:
	KEY_TYPE gram1, gram2, gram3;
	std::size_t m_nHash;

public:
	TriGram(const TriGram<KEY_TYPE> & trigram) : gram1(trigram.gram1), gram2(trigram.gram2), gram3(trigram.gram3), m_nHash(trigram.m_nHash) {}

	TriGram(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0) : gram1(_gram1), gram2(_gram2), gram3(_gram3) {
		computeHash();
	}

	~TriGram() = default;

	void refer(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0) {
		gram1 = _gram1;
		gram2 = _gram2;
		gram3 = _gram3;
		computeHash();
	}

	void referLast(KEY_TYPE _gram3 = 0) {
		gram3 = _gram3;
		computeHash();
	}

	std::size_t hash() const {
		return m_nHash;
	}

	void computeHash() {
		std::size_t seed = 0;
		hash_combine<KEY_TYPE>(seed, gram1);
		hash_combine<KEY_TYPE>(seed, gram2);
		hash_combine<KEY_TYPE>(seed, gram3);
		m_nHash = seed;
	}

	const KEY_TYPE & first() const {
		return gram1;
	}

	const KEY_TYPE & second() const {
		return gram2;
	}

	const KEY_TYPE & third() const {
		return gram3;
	}

	TriGram<KEY_TYPE> & operator=(const TriGram<KEY_TYPE> & gram) {
		gram1 = gram.gram1;
		gram2 = gram.gram2;
		gram3 = gram.gram3;
		computeHash();
		return *this;
	}

	friend std::istream & operator>>(std::istream & is, TriGram<KEY_TYPE>& trigram) {
		ttoken token;
		is >> token >> trigram.gram1 >> trigram.gram2 >> trigram.gram3 >> token;
		trigram.computeHash();
		return is;
	}
	friend std::ostream & operator<<(std::ostream & os, const TriGram<KEY_TYPE>& trigram) {
		os << "[ " << trigram.gram1 << " " << trigram.gram2 << " " << trigram.gram3 << " ]";
		return os;
	}
	friend bool operator==(const TriGram<KEY_TYPE> g1, const TriGram<KEY_TYPE> g2) {
		return g1.m_nHash == g2.m_nHash && g1.gram1 == g2.gram1 && g1.gram2 == g2.gram2 && g1.gram3 == g2.gram3;
	}
};

template<typename KEY_TYPE>
class QuarGram {
public:
	KEY_TYPE gram1, gram2, gram3, gram4;
	std::size_t m_nHash;

public:
	QuarGram(const QuarGram<KEY_TYPE> & quargram) : gram1(quargram.gram1), gram2(quargram.gram2), gram3(quargram.gram3), gram4(quargram.gram4), m_nHash(quargram.m_nHash) {}

	QuarGram(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0) : gram1(_gram1), gram2(_gram2), gram3(_gram3), gram4(_gram4) {
		computeHash();
	}

	~QuarGram() = default;

	void refer(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0) {
		gram1 = _gram1;
		gram2 = _gram2;
		gram3 = _gram3;
		gram4 = _gram4;
		computeHash();
	}

	void referLast(KEY_TYPE _gram4 = 0) {
		gram4 = _gram4;
		computeHash();
	}

	std::size_t hash() const {
		return m_nHash;
	}

	void computeHash() {
		std::size_t seed = 0;
		hash_combine<KEY_TYPE>(seed, gram1);
		hash_combine<KEY_TYPE>(seed, gram2);
		hash_combine<KEY_TYPE>(seed, gram3);
		hash_combine<KEY_TYPE>(seed, gram4);
		m_nHash = seed;
	}

	const KEY_TYPE & first() const {
		return gram1;
	}

	const KEY_TYPE & second() const {
		return gram2;
	}

	const KEY_TYPE & third() const {
		return gram3;
	}

	const KEY_TYPE & forth() const {
		return gram4;
	}

	QuarGram<KEY_TYPE> & operator=(const QuarGram<KEY_TYPE> & gram) {
		gram1 = gram.gram1;
		gram2 = gram.gram2;
		gram3 = gram.gram3;
		gram4 = gram.gram4;
		computeHash();
		return *this;
	}

	friend std::istream & operator>>(std::istream & is, QuarGram<KEY_TYPE>& quargram) {
		ttoken token;
		is >> token >> quargram.gram1 >> quargram.gram2 >> quargram.gram3 >> quargram.gram4 >> token;
		quargram.computeHash();
		return is;
	}
	friend std::ostream & operator<<(std::ostream & os, const QuarGram<KEY_TYPE>& quargram) {
		os << "[ " << quargram.gram1 << " " << quargram.gram2 << " " << quargram.gram3 << " " << quargram.gram4 << " ]";
		return os;
	}
	friend bool operator==(const QuarGram<KEY_TYPE> g1, const QuarGram<KEY_TYPE> g2) {
		return g1.m_nHash == g2.m_nHash && g1.gram1 == g2.gram1 && g1.gram2 == g2.gram2 && g1.gram3 == g2.gram3 && g1.gram4 == g2.gram4;
	}
};

template<typename KEY_TYPE>
class QuinGram {
public:
	KEY_TYPE gram1, gram2, gram3, gram4, gram5;
	std::size_t m_nHash;

public:
	QuinGram(const QuinGram<KEY_TYPE> & quingram) : gram1(quingram.gram1), gram2(quingram.gram2), gram3(quingram.gram3), gram4(quingram.gram4), gram5(quingram.gram5), m_nHash(quingram.m_nHash) {}

	QuinGram(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0, KEY_TYPE _gram5 = 0) : gram1(_gram1), gram2(_gram2), gram3(_gram3), gram4(_gram4), gram5(_gram5) {
		computeHash();
	}

	~QuinGram() = default;

	void refer(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0, KEY_TYPE _gram5 = 0) {
		gram1 = _gram1;
		gram2 = _gram2;
		gram3 = _gram3;
		gram4 = _gram4;
		gram5 = _gram5;
		computeHash();
	}

	void referLast(KEY_TYPE _gram5 = 0) {
		gram5 = _gram5;
		computeHash();
	}

	std::size_t hash() const {
		return m_nHash;
	}

	void computeHash() {
		std::size_t seed = 0;
		hash_combine<KEY_TYPE>(seed, gram1);
		hash_combine<KEY_TYPE>(seed, gram2);
		hash_combine<KEY_TYPE>(seed, gram3);
		hash_combine<KEY_TYPE>(seed, gram4);
		hash_combine<KEY_TYPE>(seed, gram5);
		m_nHash = seed;
	}

	const KEY_TYPE & first() const {
		return gram1;
	}

	const KEY_TYPE & second() const {
		return gram2;
	}

	const KEY_TYPE & third() const {
		return gram3;
	}

	const KEY_TYPE & forth() const {
		return gram4;
	}

	const KEY_TYPE & fifth() const {
		return gram5;
	}

	QuinGram<KEY_TYPE> & operator=(const QuinGram<KEY_TYPE> & gram) {
		gram1 = gram.gram1;
		gram2 = gram.gram2;
		gram3 = gram.gram3;
		gram4 = gram.gram4;
		gram5 = gram.gram5;
		computeHash();
		return *this;
	}

	friend std::istream & operator>>(std::istream & is, QuinGram<KEY_TYPE>& quingram) {
		ttoken token;
		is >> token >> quingram.gram1 >> quingram.gram2 >> quingram.gram3 >> quingram.gram4 >> quingram.gram5 >> token;
		quingram.computeHash();
		return is;
	}
	friend std::ostream & operator<<(std::ostream & os, const QuinGram<KEY_TYPE>& quingram) {
		os << "[ " << quingram.gram1 << " " << quingram.gram2 << " " << quingram.gram3 << " " << quingram.gram4 << " " << quingram.gram5 << " ]";
		return os;
	}
	friend bool operator==(const QuinGram<KEY_TYPE> g1, const QuinGram<KEY_TYPE> g2) {
		return g1.m_nHash == g2.m_nHash && g1.gram1 == g2.gram1 && g1.gram2 == g2.gram2 && g1.gram3 == g2.gram3 && g1.gram4 == g2.gram4 && g1.gram5 == g2.gram5;
	}
};

template<typename KEY_TYPE>
class HexaGram {
public:
	KEY_TYPE gram1, gram2, gram3, gram4, gram5, gram6;
	std::size_t m_nHash;

public:
	HexaGram(const HexaGram<KEY_TYPE> & hexagram) : gram1(hexagram.gram1), gram2(hexagram.gram2), gram3(hexagram.gram3), gram4(hexagram.gram4), gram5(hexagram.gram5), gram6(hexagram.gram6), m_nHash(hexagram.m_nHash) {}

	HexaGram(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0, KEY_TYPE _gram5 = 0, KEY_TYPE _gram6 = 0) : gram1(_gram1), gram2(_gram2), gram3(_gram3), gram4(_gram4), gram5(_gram5), gram6(_gram6) {
		computeHash();
	}

	~HexaGram() = default;

	void refer(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0, KEY_TYPE _gram5 = 0, KEY_TYPE _gram6 = 0) {
		gram1 = _gram1;
		gram2 = _gram2;
		gram3 = _gram3;
		gram4 = _gram4;
		gram5 = _gram5;
		gram6 = _gram6;
		computeHash();
	}

	void referLast(KEY_TYPE _gram6 = 0) {
		gram6 = _gram6;
		computeHash();
	}

	std::size_t hash() const {
		return m_nHash;
	}

	void computeHash() {
		std::size_t seed = 0;
		hash_combine<KEY_TYPE>(seed, gram1);
		hash_combine<KEY_TYPE>(seed, gram2);
		hash_combine<KEY_TYPE>(seed, gram3);
		hash_combine<KEY_TYPE>(seed, gram4);
		hash_combine<KEY_TYPE>(seed, gram5);
		hash_combine<KEY_TYPE>(seed, gram6);
		m_nHash = seed;
	}

	HexaGram<KEY_TYPE> & operator=(const HexaGram<KEY_TYPE> & gram) {
		gram1 = gram.gram1;
		gram2 = gram.gram2;
		gram3 = gram.gram3;
		gram4 = gram.gram4;
		gram5 = gram.gram5;
		gram6 = gram.gram6;
		computeHash();
		return *this;
	}

	friend std::istream & operator>>(std::istream & is, HexaGram<KEY_TYPE>& hexagram) {
		ttoken token;
		is >> token >> hexagram.gram1 >> hexagram.gram2 >> hexagram.gram3 >> hexagram.gram4 >> hexagram.gram5 >> hexagram.gram6 >> token;
		hexagram.computeHash();
		return is;
	}
	friend std::ostream & operator<<(std::ostream & os, const HexaGram<KEY_TYPE>& hexagram) {
		os << "[ " << hexagram.gram1 << " " << hexagram.gram2 << " " << hexagram.gram3 << " " << hexagram.gram4 << " " << hexagram.gram5 << " " << hexagram.gram6 << " ]";
		return os;
	}
	friend bool operator==(const HexaGram<KEY_TYPE> g1, const HexaGram<KEY_TYPE> g2) {
		return g1.m_nHash == g2.m_nHash && g1.gram1 == g2.gram1 && g1.gram2 == g2.gram2 && g1.gram3 == g2.gram3 && g1.gram4 == g2.gram4 && g1.gram5 == g2.gram5 && g1.gram6 == g2.gram6;
	}
};

template<typename KEY_TYPE>
class SeptGram {
public:
	KEY_TYPE gram1, gram2, gram3, gram4, gram5, gram6, gram7;
	std::size_t m_nHash;

public:
	SeptGram(const SeptGram<KEY_TYPE> & septgram) : gram1(septgram.gram1), gram2(septgram.gram2), gram3(septgram.gram3), gram4(septgram.gram4), gram5(septgram.gram5), gram6(septgram.gram6), gram7(septgram.gram7), m_nHash(septgram.m_nHash) {}

	SeptGram(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0, KEY_TYPE _gram5 = 0, KEY_TYPE _gram6 = 0, KEY_TYPE _gram7 = 0) : gram1(_gram1), gram2(_gram2), gram3(_gram3), gram4(_gram4), gram5(_gram5), gram6(_gram6), gram7(_gram7) {
		computeHash();
	}

	~SeptGram() = default;

	void refer(KEY_TYPE _gram1 = 0, KEY_TYPE _gram2 = 0, KEY_TYPE _gram3 = 0, KEY_TYPE _gram4 = 0, KEY_TYPE _gram5 = 0, KEY_TYPE _gram6 = 0, KEY_TYPE _gram7 = 0) {
		gram1 = _gram1;
		gram2 = _gram2;
		gram3 = _gram3;
		gram4 = _gram4;
		gram5 = _gram5;
		gram6 = _gram6;
		gram7 = _gram7;
		computeHash();
	}

	void referLast(KEY_TYPE _gram7 = 0) {
		gram7 = _gram7;
		computeHash();
	}

	std::size_t hash() const {
		return m_nHash;
	}

	void computeHash() {
		std::size_t seed = 0;
		hash_combine<KEY_TYPE>(seed, gram1);
		hash_combine<KEY_TYPE>(seed, gram2);
		hash_combine<KEY_TYPE>(seed, gram3);
		hash_combine<KEY_TYPE>(seed, gram4);
		hash_combine<KEY_TYPE>(seed, gram5);
		hash_combine<KEY_TYPE>(seed, gram6);
		hash_combine<KEY_TYPE>(seed, gram7);
		m_nHash = seed;
	}

	SeptGram<KEY_TYPE> & operator=(const SeptGram<KEY_TYPE> & gram) {
		gram1 = gram.gram1;
		gram2 = gram.gram2;
		gram3 = gram.gram3;
		gram4 = gram.gram4;
		gram5 = gram.gram5;
		gram6 = gram.gram6;
		gram7 = gram.gram7;
		computeHash();
		return *this;
	}

	friend std::istream & operator>>(std::istream & is, SeptGram<KEY_TYPE>& septgram) {
		ttoken token;
		is >> token >> septgram.gram1 >> septgram.gram2 >> septgram.gram3 >> septgram.gram4 >> septgram.gram5 >> septgram.gram6 >> septgram.gram7 >> token;
		septgram.computeHash();
		return is;
	}
	friend std::ostream & operator<<(std::ostream & os, const SeptGram<KEY_TYPE>& septgram) {
		os << "[ " << septgram.gram1 << " " << septgram.gram2 << " " << septgram.gram3 << " " << septgram.gram4 << " " << septgram.gram5 << " " << septgram.gram6 << " " << septgram.gram7 << " ]";
		return os;
	}
	friend bool operator==(const SeptGram<KEY_TYPE> g1, const SeptGram<KEY_TYPE> g2) {
		return g1.m_nHash == g2.m_nHash && g1.gram1 == g2.gram1 && g1.gram2 == g2.gram2 && g1.gram3 == g2.gram3 && g1.gram4 == g2.gram4 && g1.gram5 == g2.gram5 && g1.gram6 == g2.gram6 && g1.gram7 == g2.gram7;
	}
};

namespace std {
	template <typename KEY_TYPE>
	struct hash < BiGram<KEY_TYPE> > {
		typedef size_t result_type;
		typedef BiGram<KEY_TYPE> argument_type;
		size_t operator() (const BiGram<KEY_TYPE> & g) const {
			return g.hash();
		}
	};

	template <typename KEY_TYPE>
	struct hash < TriGram<KEY_TYPE> > {
		typedef size_t result_type;
		typedef TriGram<KEY_TYPE> argument_type;
		size_t operator() (const TriGram<KEY_TYPE> & g) const {
			return g.hash();
		}
	};

	template <typename KEY_TYPE>
	struct hash < QuarGram<KEY_TYPE> > {
		typedef size_t result_type;
		typedef QuarGram<KEY_TYPE> argument_type;
		size_t operator() (const QuarGram<KEY_TYPE> & g) const {
			return g.hash();
		}
	};

	template <typename KEY_TYPE>
	struct hash < QuinGram<KEY_TYPE> > {
		typedef size_t result_type;
		typedef QuinGram<KEY_TYPE> argument_type;
		size_t operator() (const QuinGram<KEY_TYPE> & g) const {
			return g.hash();
		}
	};

	template <typename KEY_TYPE>
	struct hash < HexaGram<KEY_TYPE> > {
		typedef size_t result_type;
		typedef HexaGram<KEY_TYPE> argument_type;
		size_t operator() (const HexaGram<KEY_TYPE> & g) const {
			return g.hash();
		}
	};

	template <typename KEY_TYPE>
	struct hash < SeptGram<KEY_TYPE> > {
		typedef size_t result_type;
		typedef SeptGram<KEY_TYPE> argument_type;
		size_t operator() (const SeptGram<KEY_TYPE> & g) const {
			return g.hash();
		}
	};

	template <typename TAG, typename HASH>
	struct hash < SetOfLabels<TAG, HASH> > {
		typedef size_t result_type;
		typedef SetOfLabels<TAG, HASH> argument_type;
		size_t operator() (const SetOfLabels<TAG, HASH> & s) const {
			return s.hash();
		}
	};
}

#endif
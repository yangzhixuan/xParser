#include <cmath>

template<typename KEY_TYPE, int N>
class Ngram {
private:
	KEY_TYPE grams[N];

public:
	Ngram(initializer_list<KEY_TYPE> args) {
		int i = 0;
		for (auto beg = args.begin(); beg != args.end(); ++beg) {
			grams[i++] = *beg;
		}
	}

};
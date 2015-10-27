#ifndef _MACROS_H
#define _MACROS_H

#include <tuple>
#include <vector>

#include "include/ngram.h"
#include "common/token/token.h"

#define MAX_SENTENCE_SIZE 256
#define MAX_SENTENCE_BITS 8

#define ROOT_WORD        "-ROOT-"
#define ROOT_POSTAG        "-ROOT-"
#define EMPTY_WORD        "-EMPTY-"
#define EMPTY_POSTAG    "-EMPTY-"
#define START_WORD        "-START-"
#define START_POSTAG    "-START-"
#define END_WORD        "-END-"
#define END_POSTAG        "-END-"

#define SENT_SPTOKEN    "/"
#define SENT_WORD(X)    std::get<0>(X)
#define SENT_POSTAG(X)    std::get<1>(X)

#define TREENODE_WORD(X)            std::get<0>(std::get<0>(X))
#define TREENODE_POSTAG(X)            std::get<1>(std::get<0>(X))
#define TREENODE_POSTAGGEDWORD(X)    std::get<0>(X)
#define TREENODE_HEAD(X)            std::get<1>(X)
#define TREENODE_LABEL(X)            std::get<2>(X)

#define IS_EMPTY(X)                ((X) >= MAX_SENTENCE_SIZE)

typedef int gtype;

typedef gtype Int;
typedef gtype Word;
typedef gtype POSTag;

typedef BiGram<gtype> TwoWords;
typedef BiGram<gtype> POSTagSet2;
typedef BiGram<gtype> WordPOSTag;
typedef BiGram<gtype> WordSetOfDepLabels;
typedef BiGram<gtype> POSTagSetOfDepLabels;

typedef TriGram<gtype> ThreeWords;
typedef TriGram<gtype> POSTagSet3;
typedef TriGram<gtype> WordWordPOSTag;
typedef TriGram<gtype> WordPOSTagPOSTag;

typedef QuarGram<gtype> POSTagSet4;
typedef QuarGram<gtype> WordWordPOSTagPOSTag;

typedef BiGram<gtype> WordInt;
typedef BiGram<gtype> POSTagInt;

typedef TriGram<gtype> TwoWordsInt;
typedef TriGram<gtype> POSTagSet2Int;
typedef TriGram<gtype> WordPOSTagInt;

typedef QuarGram<gtype> ThreeWordsInt;

typedef QuarGram<gtype> POSTagSet3Int;
typedef QuarGram<gtype> WordWordPOSTagInt;
typedef QuarGram<gtype> WordPOSTagPOSTagInt;

typedef QuinGram<gtype> POSTagSet4Int;
typedef QuinGram<gtype> WordWordPOSTagPOSTagInt;
typedef QuinGram<gtype> WordPOSTagPOSTagPOSTagInt;

typedef HexaGram<gtype> POSTagSet5Int;
typedef HexaGram<gtype> WordPOSTagPOSTagPOSTagPOSTagInt;

typedef std::tuple<ttoken, ttoken> POSTaggedWord;
typedef std::tuple<POSTaggedWord, int, ttoken> DependencyTreeNode;

typedef std::vector<POSTaggedWord> Sentence;
typedef std::vector<DependencyTreeNode> DependencyTree;

class DependencyGraph {
public:
    int num_words;
    int num_arcs;
    // word at index 0 is root, actual words start from 1
    std::vector<POSTaggedWord> pos_words;
    std::vector<int> predicates_index;
    int graph[MAX_SENTENCE_SIZE][MAX_SENTENCE_SIZE];

    void clear();
};

int encodeLinkDistanceOrDirection(const int &hi, const int &di, bool dir);
int encodeDistance(int st, int ed);

std::istream &operator>>(std::istream &input, DependencyGraph &graph);

std::istream &operator>>(std::istream &input, Sentence &sentence);

std::istream &operator>>(std::istream &input, DependencyTree &tree);

std::ostream &operator<<(std::ostream &output, const Sentence &sentence);

std::ostream &operator<<(std::ostream &output, const DependencyTree &tree);

#endif
#ifndef _SVM_MACROS_H
#define _SVM_MACROS_H

#include <vector>

// point for sparse vector
// first pair is < y : alpha >
// else represent < id : value >
typedef std::vector<std::pair<int, double>> point;

#endif
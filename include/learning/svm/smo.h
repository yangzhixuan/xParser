#ifndef _SMO_H
#define _SMO_H

#include <numeric>

#include "svm_macros.h"

#define K(X, Y)        (kernal(X, Y))
#define U(X)        (std::accumulate(points.begin(), points.end(), 0.0, [&X](const double & v, const point & p){ return v + K(p, X) * (p[0].first > 0 ? p[0].second : -p[0].second) }))
#define OPSIGN(X, Y)    ((X) != (Y))
#define L_OP(X, Y, C)    (std::fmax(0.0, Y[0].second - X[0].second))
#define H_OP(X, Y, C)    (std::fmax(C, C + Y[0].second - X[0].second))
#define L_EQ(X, Y, C)    (std::fmax(0.0, Y[0].second + X[0].second - C))
#define H_EQ(X, Y, C)    (std::fmax(C, Y[0].second + X[0].second))

#define YITA(X, Y)        (K(X,X) + K(Y,Y) - 2.0 * K(X,Y))
#define ERROR(X)        (U(X) - X[1])
#define NEWALPHA(X, Y)    (Y[0].second + (ERROR(X) - ERROR(Y)) / (Y[0].first > 0 ? YITA(X,Y) : -YITA(X,Y)))

template<typename Kernal>
class SMO {
private:
    double bound;
    param alpha;

public:
    SMO() { }

    ~SMO() = default;

    void train(const std::vector<point> &points, Kernal kernal = Kernal()) {
        alpha.clear();
        alpha.reserve(points.size());
        alpha.insert(alpha.end(), points.size(), 0.0);


    }
};

#endif
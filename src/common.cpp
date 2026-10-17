#include "common.hpp"

#include <cmath>
#include <cassert>


double euclideanDistance(const Vector& a, const Vector& b) {
    assert(a.size() == b.size());

    double sum = 0.0;
    for (size_t i=0; i<a.size(); i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }

    return std::sqrt(sum);
}
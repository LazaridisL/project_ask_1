#include <cmath>

#include "common.hpp"

double euclideanDistance(const Vector& a, const Vector& b) {
    double sum = 0.0;
    for (size_t i=0; i<a.size(); i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }

    return std::sqrt(sum);
}
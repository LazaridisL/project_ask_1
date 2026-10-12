#include <cmath>

#include "common.hpp"

double euclideanDistance(const std::vector<float>& a, const std::vector<float>& b) {
    double sum = 0.0;
    for (size_t i=0; i<a.size(); i++) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }

    return std::sqrt(sum);
}
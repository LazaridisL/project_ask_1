#include "metrics.hpp"

#include <algorithm>


double recall(const std::vector<bool>& isRelevant, int n, int relevantCount) {
    int ranksToCheck = std::min(n, static_cast<int>(isRelevant.size()));
    int hits = 0;
    for (int r = 0; r < ranksToCheck; r++) {
        if (isRelevant[r]) {
            hits++;
        }
    }

    return static_cast<double>(hits) / relevantCount;
}

double averagePrecision(const std::vector<bool>& isRelevant, int n, int relevantCount) {
    int ranksToCheck = std::min(n, static_cast<int>(isRelevant.size()));
    int hits = 0;
    double sum = 0.0;
    for (int r = 0; r < ranksToCheck; r++) {
        if (isRelevant[r]) {
            hits++;
            int rank = r + 1;
            sum += static_cast<double>(hits) / rank;
        }
    }

    return sum / relevantCount;
}
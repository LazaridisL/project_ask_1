#include "exact_search.hpp"

#include <algorithm>
#include <utility>


void ExactSearch::build(const VectorList& database) {
    database_ = database;
}

std::vector<int> ExactSearch::search(const Vector& query) {
    std::vector<std::pair<double, int>> distances;
    distances.reserve(database_.size());
    for (size_t i = 0; i < database_.size(); i++) {
        distances.push_back({squaredDistance(query, database_[i]), static_cast<int>(i)});
    }

    size_t count = std::min(static_cast<size_t>(resultsPerQuery), distances.size());
    std::partial_sort(distances.begin(), distances.begin() + count, distances.end());

    std::vector<int> result;
    result.reserve(count);
    for (size_t i = 0; i < count; i++) {
        result.push_back(distances[i].second);
    }
    
    return result;
}
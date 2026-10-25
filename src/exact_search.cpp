#include "exact_search.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

bool ExactSearch::build(const VectorList& database) {
    if (database.empty() || database.front().empty() ||
        database.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return false;
    }

    const std::size_t dimensions = database.front().size();
    for (const Vector& vector : database) {
        if (vector.size() != dimensions) {
            return false;
        }
        for (float value : vector) {
            if (!std::isfinite(value)) {
                return false;
            }
        }
    }

    database_ = database;
    dimensions_ = dimensions;
    return true;
}

std::vector<int> ExactSearch::search(const Vector& query, int top_n) const {
    if (database_.empty() || top_n <= 0 || query.size() != dimensions_) {
        return {};
    }
    for (float value : query) {
        if (!std::isfinite(value)) {
            return {};
        }
    }

    std::vector<int> indices(database_.size());
    for (std::size_t index = 0; index < indices.size(); ++index) {
        indices[index] = static_cast<int>(index);
    }

    const auto distance_order = [this, &query](int left, int right) {
        const double left_distance = squaredDistance(
            query, database_[static_cast<std::size_t>(left)]
        );
        const double right_distance = squaredDistance(
            query, database_[static_cast<std::size_t>(right)]
        );
        if (left_distance != right_distance) {
            return left_distance < right_distance;
        }
        return left < right;
    };

    const std::size_t result_count = std::min(
        static_cast<std::size_t>(top_n), indices.size()
    );
    std::partial_sort(
        indices.begin(),
        indices.begin() + static_cast<std::ptrdiff_t>(result_count),
        indices.end(),
        distance_order
    );
    indices.resize(result_count);
    return indices;
}

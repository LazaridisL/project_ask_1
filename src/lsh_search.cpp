#include "lsh_search.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

#include "common.hpp"

namespace {
    constexpr int maxIntIndex = std::numeric_limits<int>::max();
}

LSHSearch::LSHSearch(int number_tables, int hashes_per_table, double bucket_width, unsigned int seed)
    : number_tables_(number_tables),
      hashes_per_table_(hashes_per_table),
      bucket_width_(bucket_width),
      seed_(seed) {

    if (number_tables_ <= 0 || hashes_per_table_ <= 0 ||
        !std::isfinite(bucket_width_) || bucket_width_ <= 0.0)
        throw std::invalid_argument("Invalid LSH parameters.");
}

void LSHSearch::build(const VectorList& database) {
    database_.clear();
    tables_.clear();
    dimensions_ = 0;

    if (database.empty() || database.front().empty() ||
        database.size() > static_cast<std::size_t>(maxIntIndex))
        return;

    dimensions_ = database.front().size();

    for (const auto& v : database) {
        if (v.size() != dimensions_ ||
            !std::all_of(v.begin(), v.end(),
                         [](float x) { return std::isfinite(x); })) {
            dimensions_ = 0;
            return;
        }
    }

    std::mt19937 generator(seed_);
    std::normal_distribution<double> normal(0.0, 1.0);
    std::uniform_real_distribution<double> uniform(0.0, bucket_width_);

    tables_.reserve(number_tables_);

    for (int t = 0; t < number_tables_; ++t) {
        HashTable table;

        for (int h = 0; h < hashes_per_table_; ++h) {
            Vector projection(dimensions_);

            for (float& value : projection)
                value = static_cast<float>(normal(generator));

            table.projections.push_back(std::move(projection));
            table.offsets.push_back(uniform(generator));
        }

        for (std::size_t i = 0; i < database.size(); ++i)
            table.buckets[hash_vector(database[i], table)].push_back(
                static_cast<int>(i));

        tables_.push_back(std::move(table));
    }

    database_ = database;
}

std::vector<int> LSHSearch::search(const Vector& query) {
    if (tables_.empty() || database_.empty() ||
        query.size() != dimensions_)
        return {};

    if (!std::all_of(query.begin(), query.end(),
                     [](float x) { return std::isfinite(x); }))
        return {};

    std::vector<bool> seen(database_.size(), false);
    std::vector<int> candidates;

    for (const auto& table : tables_) {
        auto bucket = table.buckets.find(hash_vector(query, table));
        if (bucket == table.buckets.end())
            continue;

        for (int index : bucket->second) {
            if (!seen[index]) {
                seen[index] = true;
                candidates.push_back(index);
            }
        }
    }

    std::cerr << "Database size: " << database_.size()
              << ", candidates found: " << candidates.size() << '\n';

    if (candidates.empty())
        return {};

    // Calculate each candidate's distance only once.
    std::vector<std::pair<double, int>> ranked;
    ranked.reserve(candidates.size());

    for (int index : candidates)
        ranked.emplace_back(
            squaredDistance(query, database_[index]), index);

    const std::size_t count =
        std::min<std::size_t>(resultsPerQuery, ranked.size());

    std::partial_sort(ranked.begin(), ranked.begin() + count, ranked.end());

    std::vector<int> results;
    results.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
        results.push_back(ranked[i].second);

    return results;
}

std::string LSHSearch::hash_vector(const Vector& vector, const HashTable& table) const {

    std::string key;

    for (std::size_t h = 0; h < table.projections.size(); ++h) {
        double value = table.offsets[h];
        const auto& projection = table.projections[h];

        for (std::size_t d = 0; d < dimensions_; ++d)
            value += static_cast<double>(vector[d]) * projection[d];

        const double bucketValue = std::floor(value / bucket_width_);

        if (!std::isfinite(bucketValue) ||
            bucketValue < static_cast<double>(
                std::numeric_limits<std::int64_t>::min()) ||
            bucketValue >= static_cast<double>(
                std::numeric_limits<std::int64_t>::max()))
            throw std::overflow_error("LSH bucket index overflow.");

        if (h > 0)
            key += ',';

        key += std::to_string(static_cast<std::int64_t>(bucketValue));
    }

    return key;
}

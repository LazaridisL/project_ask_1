#include "lsh_search.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

//constructor
LSHSearch::LSHSearch(
    int number_tables,
    int hashes_per_table,
    double bucket_width,
    unsigned int seed
) :
    number_tables_(number_tables),
    hashes_per_table_(hashes_per_table),
    bucket_width_(bucket_width),
    seed_(seed)
{
    if (number_tables_ <= 0 || hashes_per_table_ <= 0 ||
        !std::isfinite(bucket_width_) || bucket_width_ <= 0.0) {
        throw std::invalid_argument("LSH parameters must be positive and finite.");
    }
}

bool LSHSearch::build(const VectorList& database) {
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

    std::mt19937 generator(seed_);
    std::normal_distribution<double> projection_distribution(0.0, 1.0);
    std::uniform_real_distribution<double> offset_distribution(0.0, bucket_width_);
    std::vector<HashTable> tables;
    tables.reserve(static_cast<std::size_t>(number_tables_));

    for (int table_index = 0; table_index < number_tables_; ++table_index) {
        HashTable table;
        table.projections.reserve(static_cast<std::size_t>(hashes_per_table_));
        table.offsets.reserve(static_cast<std::size_t>(hashes_per_table_));

        for (int hash_index = 0; hash_index < hashes_per_table_; ++hash_index) {
            Vector projection;
            projection.reserve(dimensions);
            for (std::size_t dimension = 0; dimension < dimensions; ++dimension) {
                projection.push_back(static_cast<float>(projection_distribution(generator)));
            }
            table.projections.push_back(std::move(projection));
            table.offsets.push_back(offset_distribution(generator));
        }

        for (std::size_t vector_index = 0; vector_index < database.size(); ++vector_index) {
            table.buckets[hash_vector(database[vector_index], table)]
                .push_back(static_cast<int>(vector_index));
        }
        tables.push_back(std::move(table));
    }

    database_ = database;
    dimensions_ = dimensions;
    tables_ = std::move(tables);
    return true;
}

std::vector<int> LSHSearch::search(const Vector& query, int top_n) const {
    if (tables_.empty() || top_n <= 0 || query.size() != dimensions_) {
        return {};
    }
    for (float value : query) {
        if (!std::isfinite(value)) {
            return {};
        }
    }

    std::vector<bool> seen(database_.size(), false);
    std::vector<int> candidates;
    for (const HashTable& table : tables_) {
        const auto bucket = table.buckets.find(hash_vector(query, table));
        if (bucket == table.buckets.end()) {
            continue;
        }
        for (int index : bucket->second) {
            if (!seen[static_cast<std::size_t>(index)]) {
                seen[static_cast<std::size_t>(index)] = true;
                candidates.push_back(index);
            }
        }
    }

    //No exact bucket match gets nearest result
    if (candidates.empty()) {
        candidates.reserve(database_.size());
        for (std::size_t index = 0; index < database_.size(); ++index) {
            candidates.push_back(static_cast<int>(index));
        }
    }

    std::sort(candidates.begin(), candidates.end(), [this, &query](int left, int right) {
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
    });

    if (candidates.size() > static_cast<std::size_t>(top_n)) {
        candidates.resize(static_cast<std::size_t>(top_n));
    }
    return candidates;
}

std::string LSHSearch::hash_vector(
    const Vector& vector,
    const HashTable& table
) const {
    std::string key;
    for (std::size_t hash_index = 0; hash_index < table.projections.size(); ++hash_index) {
        double projection_value = table.offsets[hash_index];
        const Vector& projection = table.projections[hash_index];
        for (std::size_t dimension = 0; dimension < dimensions_; ++dimension) {
            projection_value += static_cast<double>(vector[dimension]) *
                                projection[dimension];
        }
        const auto bucket = static_cast<std::int64_t>(
            std::floor(projection_value / bucket_width_)
        );
        if (hash_index > 0) {
            key.push_back(',');
        }
        key += std::to_string(bucket);
    }
    return key;
}

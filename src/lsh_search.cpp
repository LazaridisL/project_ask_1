#include "lsh_search.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <iostream>

#include "common.hpp"
#include "search_method.hpp"

// Initialise LSH
LSHSearch::LSHSearch(
    int number_tables,
    int hashes_per_table,
    double bucket_width,
    unsigned int seed
)
    : number_tables_(number_tables),
      hashes_per_table_(hashes_per_table),
      bucket_width_(bucket_width),
      seed_(seed)
{
    if (number_tables_ <= 0 ||
        hashes_per_table_ <= 0 ||
        !std::isfinite(bucket_width_) ||
        bucket_width_ <= 0.0) {
        throw std::invalid_argument(
            "LSH parameters must be positive and finite."
        );
    }
}

// Build L independent hash tables
void LSHSearch::build(const VectorList& database) {
    // Clear any previously built index first.
    database_.clear();
    tables_.clear();
    dimensions_ = 0;

    if (database.empty() ||
        database.front().empty() ||
        database.size() >
            static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return;
    }

    const std::size_t dimensions = database.front().size();

    // All vectors must have the same dimension and finite values
    for (const Vector& vector : database) {
        if (vector.size() != dimensions) {
            return;
        }

        for (float value : vector) {
            if (!std::isfinite(value)) {
                return;
            }
        }
    }

    dimensions_ = dimensions;

    std::mt19937 generator(seed_);
    std::normal_distribution<double> projection_distribution(0.0, 1.0);
    std::uniform_real_distribution<double> offset_distribution(
        0.0, bucket_width_
    );

    std::vector<HashTable> new_tables;
    new_tables.reserve(static_cast<std::size_t>(number_tables_));

    for (int table_index = 0;
         table_index < number_tables_;
         ++table_index) {

        HashTable table;

        table.projections.reserve(
            static_cast<std::size_t>(hashes_per_table_)
        );
        table.offsets.reserve(
            static_cast<std::size_t>(hashes_per_table_)
        );

        // Generate k random projection vectors and offsets
        for (int hash_index = 0;
             hash_index < hashes_per_table_;
             ++hash_index) {

            Vector projection;
            projection.reserve(dimensions);

            for (std::size_t dimension = 0;
                 dimension < dimensions;
                 ++dimension) {
                projection.push_back(
                    static_cast<float>(
                        projection_distribution(generator)
                    )
                );
            }

            table.projections.push_back(std::move(projection));
            table.offsets.push_back(offset_distribution(generator));
        }

        // Insert every database vector into its bucket
        for (std::size_t vector_index = 0;
             vector_index < database.size();
             ++vector_index) {

            const std::string key =
                hash_vector(database[vector_index], table);

            table.buckets[key].push_back(
                static_cast<int>(vector_index)
            );
        }

        new_tables.push_back(std::move(table));
    }

    database_ = database;
    dimensions_ = dimensions;
    tables_ = std::move(new_tables);
}

// Retrieve candidates from matching buckets and rank them by distance
std::vector<int> LSHSearch::search(const Vector& query) {
    if (tables_.empty() ||
        database_.empty() ||
        query.size() != dimensions_) {
        return {};
    }

    for (float value : query) {
        if (!std::isfinite(value)) {
            return {};
        }
    }

    // Avoid evaluating the same candidate more than once
    std::vector<bool> seen(database_.size(), false);
    std::vector<int> candidates;

    // Collect candidates from the matching bucket in each table
    for (const HashTable& table : tables_) {
        const std::string key = hash_vector(query, table);
        const auto bucket = table.buckets.find(key);

        if (bucket == table.buckets.end()) {
            continue;
        }

        for (int index : bucket->second) {
            const std::size_t position =
                static_cast<std::size_t>(index);

            if (!seen[position]) {
                seen[position] = true;
                candidates.push_back(index);
            }
        }
    }

    std::cerr << "Database size: " << database_.size()
          << ", candidates found: " << candidates.size()
          << '\n';

    // No fallback
    if (candidates.empty()) {
        return {};
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [this, &query](int left, int right) {
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
        }
    );

    // The interface requests up to resultsPerQuery results
    if (candidates.size() >
        static_cast<std::size_t>(resultsPerQuery)) {
        candidates.resize(
            static_cast<std::size_t>(resultsPerQuery)
        );
    }

    return candidates;
}

// Convert a vector into its concatenated LSH bucket key
std::string LSHSearch::hash_vector(
    const Vector& vector,
    const HashTable& table
) const {
    std::string key;

    for (std::size_t hash_index = 0;
         hash_index < table.projections.size();
         ++hash_index) {

        double projection_value = table.offsets[hash_index];
        const Vector& projection = table.projections[hash_index];

        for (std::size_t dimension = 0;
             dimension < dimensions_;
             ++dimension) {

            projection_value +=
                static_cast<double>(vector[dimension]) *
                static_cast<double>(projection[dimension]);
        }

        const double bucket_value =
            std::floor(projection_value / bucket_width_);

        // Protect the integer conversion from overflow
        if (bucket_value <
                static_cast<double>(
                    std::numeric_limits<std::int64_t>::min()
                ) ||
            bucket_value >
                static_cast<double>(
                    std::numeric_limits<std::int64_t>::max()
                )) {
            throw std::overflow_error("LSH bucket index overflow.");
        }

        const auto bucket =
            static_cast<std::int64_t>(bucket_value);

        if (hash_index > 0) {
            key.push_back(',');
        }

        key += std::to_string(bucket);
    }

    return key;
}

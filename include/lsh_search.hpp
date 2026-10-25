
#ifndef LSH_SEARCH_HPP
#define LSH_SEARCH_HPP

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.hpp"

class LSHSearch {
public:
    // Create an LSH index with configurable parameters.
    LSHSearch(
        int number_tables = 10,
        int hashes_per_table = 4,
        double bucket_width = 0.25,
        unsigned int seed = 67
    );

    // Build the hash tables from the database vectors.
    bool build(const VectorList& database);

    // Return the indices of the closest candidate vectors.
    std::vector<int> search(
        const Vector& query,
        int top_n
    ) const;

private:
    struct HashTable {
        std::vector<Vector> projections;
        std::vector<double> offsets;

        std::unordered_map<std::string, std::vector<int>> buckets;
    };

    int number_tables_;
    int hashes_per_table_;
    double bucket_width_;
    unsigned int seed_;
    std::size_t dimensions_ = 0;

    VectorList database_;
    std::vector<HashTable> tables_;

    std::string hash_vector(
        const Vector& vector,
        const HashTable& table
    ) const;
};

#endif

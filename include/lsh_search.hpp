#ifndef LSH_SEARCH_HPP
#define LSH_SEARCH_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "search_method.hpp"

class LSHSearch : public SearchMethod {
public:
    LSHSearch(
        int number_tables = 5,
        int hashes_per_table = 4,
        double bucket_width = 0.05,
        unsigned int seed = 67
    );

    void build(const VectorList& database) override;
    std::vector<int> search(const Vector& query) override;

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

    std::string hash_vector(const Vector& vector, const HashTable& table) const;
};

#endif
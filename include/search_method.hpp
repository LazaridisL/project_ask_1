#ifndef SEARCH_METHOD_HPP
#define SEARCH_METHOD_HPP

#include <vector>

#include "common.hpp"

inline constexpr int resultsPerQuery = 10;

class SearchMethod {
public:
    virtual ~SearchMethod() = default;

    // Builds the index. Gets called before any search
    virtual void build(const VectorList& database) = 0;

    // Returns the top resultsPerQuery indices from the database
    virtual std::vector<int> search(const Vector& query) = 0;
};

#endif
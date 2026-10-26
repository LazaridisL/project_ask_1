#ifndef EXACT_SEARCH_HPP
#define EXACT_SEARCH_HPP

#include <vector>

#include "common.hpp"

class ExactSearch {
public:
    // Builds the database used by subsequent searches.
    bool build(const VectorList& database);

    // Returns the closest database vector indices in ascending distance order.
    std::vector<int> search(const Vector& query, int top_n) const;

private:
    VectorList database_;
    std::size_t dimensions_ = 0;
};

#endif

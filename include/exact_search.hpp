#ifndef EXACT_SEARCH_HPP
#define EXACT_SEARCH_HPP

#include "search_method.hpp"


// Compares the query with every database vector
class ExactSearch : public SearchMethod {
public:
    void build(const VectorList& database) override;
    std::vector<int> search(const Vector& query) override;

private:
    VectorList database_;
};


#endif
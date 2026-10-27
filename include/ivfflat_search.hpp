#ifndef IVFFLAT_SEARCH_HPP
#define IVFFLAT_SEARCH_HPP

#include <vector>

#include "search_method.hpp"


class IvfFlatSearch : public SearchMethod {
public:
    IvfFlatSearch(int kclusters, int nprobe, unsigned int seed);

    void build(const VectorList& database) override;
    std::vector<int> search(const Vector& query) override;

private:
    int kclusters_;
    int nprobe_;
    unsigned int seed_;

    VectorList database_;
    VectorList centroids_;
    std::vector<std::vector<int>> clusterMembers_;
};

#endif
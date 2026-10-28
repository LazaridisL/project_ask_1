#ifndef IVFPQ_SEARCH_HPP
#define IVFPQ_SEARCH_HPP

#include <cstdint>
#include <vector>

#include "common.hpp"
#include "search_method.hpp"


class IvfPqSearch : public SearchMethod {
public:
    IvfPqSearch(int kclusters, int nprobe, int M, int nbits, unsigned int seed);

    void build(const VectorList& database) override;
    std::vector<int> search(const Vector& query) override;

private:
    std::vector<std::vector<double>> distanceTable(const Vector& residual) const;

    int kclusters_;
    int nprobe_;
    int subvectorCount_;
    int nbits_;
    unsigned int seed_;
    int subvectorDimension_ = 0;

    VectorList centroids_;
    std::vector<std::vector<int>> clusterMembers_;
    std::vector<VectorList> codebooks_;
    std::vector<std::vector<std::uint8_t>> codes_;
};

#endif
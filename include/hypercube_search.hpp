#ifndef HYPERCUBE_SEARCH_HPP
#define HYPERCUBE_SEARCH_HPP

#include <random>
#include <unordered_map>
#include <vector>

#include "common.hpp"
#include "search_method.hpp"


class HypercubeSearch : public SearchMethod {
public:
    HypercubeSearch(int kproj, double w, int M, int probes, unsigned int seed);

    void build(const VectorList& database) override;
    std::vector<int> search(const Vector& query) override;

private:
    // The LSH function behind bit i
    long long projectionHash(int functionIndex, const Vector& point) const;

    // Random bit chosen the first time and then remembered
    int bitOf(int functionIndex, long long hashValue);

    // The vertex that a point falls on
    int vertexOf(const Vector& point);

    int kproj_;
    double w_;
    int maxCandidates_;
    int probes_;
    std::mt19937 generator_;

    VectorList database_;
    VectorList projections_;
    std::vector<double> offsets_;
    std::vector<std::unordered_map<long long, int>> bitOfValue_;
    std::vector<std::vector<int>> vertices_;
    std::vector<int> probeOrder_;
};


#endif
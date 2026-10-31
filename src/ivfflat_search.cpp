#include "ivfflat_search.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>
#include <stdexcept>

#include "kmeans.hpp"


static const int maxKMeansIterations = 50;

IvfFlatSearch::IvfFlatSearch(int kclusters, int nprobe, unsigned int seed)
    : kclusters_(kclusters), nprobe_(nprobe), seed_(seed) {

    if (nprobe < 1) {
        throw std::invalid_argument("IVFFlat: nprobe must be at least 1");
    }
}

void IvfFlatSearch::build(const VectorList& database) {
    database_ = database;

    int n = static_cast<int>(database.size());
    // Default number of clusters = sqrt(n)
    if (kclusters_ <= 0) {
        kclusters_ = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(n))));
    }
    kclusters_ = std::min(kclusters_, n);   // No more clusters than images

    KMeansResult clustering = kmeans(database_, kclusters_, maxKMeansIterations, seed_);
    centroids_ = clustering.centroids;

    // Inverted lists
    clusterMembers_.assign(kclusters_, std::vector<int>());
    for (int i=0; i<n; i++) {
        clusterMembers_[clustering.assignments[i]].push_back(i);
    }

    std::cerr << "IVFFlat: " << kclusters_ << " clusters, nprobe = " << nprobe_
              << ", k-means " << clustering.iterations << " iterations\n";
}

std::vector<int> IvfFlatSearch::search(const Vector& query) {
    std::vector<std::pair<double, int>> clusterDistances;
    clusterDistances.reserve(centroids_.size());
    for (size_t c=0; c<centroids_.size(); c++) {
        clusterDistances.push_back({squaredDistance(query, centroids_[c]), static_cast<int>(c)});
    }

    size_t probes = std::min(static_cast<size_t>(nprobe_), clusterDistances.size());
    std::partial_sort(clusterDistances.begin(), clusterDistances.begin() + probes, clusterDistances.end());

    std::vector<std::pair<double, int>> candidates;
    for (size_t p=0; p<probes; p++) {
        int cluster = clusterDistances[p].second;
        for (int index : clusterMembers_[cluster]) {
            candidates.push_back({squaredDistance(query, database_[index]), index});
        }
    }

    size_t count = std::min(static_cast<size_t>(resultsPerQuery), candidates.size());
    std::partial_sort(candidates.begin(), candidates.begin() + count, candidates.end());

    std::vector<int> result;
    result.reserve(count);
    for (size_t i=0; i<count; i++) {
        result.push_back(candidates[i].second);
    }

    return result;
}
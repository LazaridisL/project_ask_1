#include "hypercube_search.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>


static constexpr int maxKproj = 20;

HypercubeSearch::HypercubeSearch(int kproj, double w, int M, int probes, unsigned int seed)
    : kproj_(kproj), w_(w), maxCandidates_(M), probes_(probes), generator_(seed) {

        if ((kproj_ < 1) || (kproj_ > maxKproj)) {
            throw std::invalid_argument("Hypercube: kproj must be between 1 and 20");
        }
        if (w_ <= 0.0) {
            throw std::invalid_argument("Hypercube: w must be positive");
        }
        if ((maxCandidates_ < 1) || (probes_ < 1)) {
            throw std::invalid_argument("Hypercube: M and probes must be at least 1");
        }
}

void HypercubeSearch::build(const VectorList& database) {
    database_ = database;
    const size_t dimension = database_.empty() ? 0 : database_[0].size();

    // Draw the kproj LSH functions
    std::normal_distribution<float> gaussian(0.0f, 1.0f);
    std::uniform_real_distribution<double> uniformOffset(0.0, w_);
    projections_.assign(kproj_, Vector(dimension));
    offsets_.assign(kproj_, 0.0);
    for (int i = 0; i < kproj_; ++i) {
        for (float& coordinate : projections_[i]) {
            coordinate = gaussian(generator_);
        }
        offsets_[i] = uniformOffset(generator_);
    }
    bitOfValue_.assign(kproj_, {});

    // Put every image on its vertex
    const int vertexCount = 1 << kproj_;
    vertices_.assign(vertexCount, {});
    for (size_t index = 0; index < database_.size(); ++index) {
        vertices_[vertexOf(database_[index])].push_back(static_cast<int>(index));
    }

    // Shuffle the list because the HPatches images are always first
    for (std::vector<int>& images : vertices_) {
        std::shuffle(images.begin(), images.end(), generator_);
    }

    // Probe order
    probeOrder_.resize(vertexCount);
    for (int mask = 0; mask < vertexCount; ++mask) {
        probeOrder_[mask] = mask;
    }
    std::stable_sort(probeOrder_.begin(), probeOrder_.end(), [](int a, int b) {
        return std::popcount(static_cast<unsigned int>(a)) <
               std::popcount(static_cast<unsigned int>(b));
    });

    size_t nonEmptyVertices = 0;
    size_t largestVertex = 0;
    for (const std::vector<int>& images : vertices_) {
        if (!images.empty()) {
            ++nonEmptyVertices;
        }
        largestVertex = std::max(largestVertex, images.size());
    }
    std::cerr << "Hypercube: kproj = " << kproj_ << " (" << vertexCount << " vertices), w = " << w_
              << ", M = " << maxCandidates_ << ", probes = " << probes_ << "\n"
              << "Hypercube: " << nonEmptyVertices << " non-empty vertices, largest holds "
              << largestVertex << " images\n";
}

long long HypercubeSearch::projectionHash(int functionIndex, const Vector& point) const {
    const Vector& direction = projections_[functionIndex];
    double dotProduct = 0.0;
    for (size_t j = 0; j < point.size(); ++j) {
        dotProduct += static_cast<double>(direction[j]) * point[j];
    }

    return static_cast<long long>(std::floor((dotProduct + offsets_[functionIndex]) / w_));
}

int HypercubeSearch::bitOf(int functionIndex, long long hashValue) {
    std::unordered_map<long long, int>& remembered = bitOfValue_[functionIndex];
    auto found = remembered.find(hashValue);
    if (found != remembered.end()) {
        return found->second;
    }
    std::bernoulli_distribution coin(0.5);
    const int bit = coin(generator_) ? 1 : 0;
    remembered.emplace(hashValue, bit);

    return bit;
}

int HypercubeSearch::vertexOf(const Vector& point) {
    int vertex = 0;
    for (int i = 0; i < kproj_; ++i) {
        vertex = (vertex << 1) | bitOf(i, projectionHash(i, point));
    }

    return vertex;
}

std::vector<int> HypercubeSearch::search(const Vector& query) {
    const int queryVertex = vertexOf(query);

    std::vector<std::pair<double, int>> candidates;
    int verticesVisited = 0;
    for (int flipMask : probeOrder_) {
        if ((verticesVisited == probes_) || (static_cast<int>(candidates.size()) >= maxCandidates_)) {
            break;
        }
        for (int index : vertices_[queryVertex ^ flipMask]) {
            if (static_cast<int>(candidates.size()) >= maxCandidates_) {
                break;
            }
            candidates.emplace_back(euclideanDistance(query, database_[index]), index);
        }
        ++verticesVisited;
    }

    const size_t keep = std::min(candidates.size(), static_cast<size_t>(resultsPerQuery));
    std::partial_sort(candidates.begin(), candidates.begin() + keep, candidates.end());

    std::vector<int> ranking;
    ranking.reserve(keep);
    for (size_t i = 0; i < keep; ++i) {
        ranking.push_back(candidates[i].second);
    }
    
    return ranking;
}
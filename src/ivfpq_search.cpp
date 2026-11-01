#include "ivfpq_search.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "kmeans.hpp"


static constexpr int maxCoarseIterations = 50;
static constexpr int maxCodebookIterations = 25;

static Vector subvector(const Vector& v, int piece, int pieceSize) {
    const auto first = v.begin() + static_cast<long>(piece) * pieceSize;

    return Vector(first, first + pieceSize);
}

static Vector subtract(const Vector& a, const Vector& b) {
    Vector difference(a.size());
    for (size_t j=0; j<a.size(); j++) {
        difference[j] = a[j] - b[j];
    }

    return difference;
}

IvfPqSearch::IvfPqSearch(int kclusters, int nprobe, int M, int nbits, unsigned int seed)
    : kclusters_(kclusters), nprobe_(nprobe), subvectorCount_(M), nbits_(nbits), seed_(seed) {

    if (subvectorCount_ < 1) {
        throw std::invalid_argument("IVFPQ: M must be at least 1");
    }

    if ((nbits_ < 1) || (nbits_ > 8)) {
        throw std::invalid_argument("IVFPQ: nbits must be between 1 and 8");
    }

    if (nprobe_ < 1) {
        throw std::invalid_argument("IVFPQ: nprobe must be at least 1");
    }
}

void IvfPqSearch::build(const VectorList& database) {
    const int imageCount = static_cast<int>(database.size());
    const int dimension = database.empty() ? 0 : static_cast<int>(database[0].size());
    if (dimension % subvectorCount_ != 0) {
        throw std::invalid_argument("IVFPQ: the vector dimension must be divisible by M");
    }
    subvectorDimension_ = dimension / subvectorCount_;

    if (kclusters_ <= 0) {
        kclusters_ = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(imageCount))));
    }
    kclusters_ = std::min(kclusters_, imageCount);
    const KMeansResult coarse = kmeans(database, kclusters_, maxCoarseIterations, seed_);
    centroids_ = coarse.centroids;

    clusterMembers_.assign(kclusters_, {});
    VectorList residuals(imageCount);
    for (int index=0; index<imageCount; index++) {
        const int cluster = coarse.assignments[index];
        clusterMembers_[cluster].push_back(index);
        residuals[index] = subtract(database[index], centroids_[cluster]);
    }

    const int codebookSize = std::min(1 << nbits_, imageCount);
    codebooks_.assign(subvectorCount_, {});
    for (int piece=0; piece<subvectorCount_; piece++) {
        VectorList pieces;
        pieces.reserve(imageCount);
        for (const Vector& residual : residuals) {
            pieces.push_back(subvector(residual, piece, subvectorDimension_));
        }

        const unsigned int pieceSeed = seed_ + 1 + static_cast<unsigned int>(piece);
        codebooks_[piece] = kmeans(pieces, codebookSize, maxCodebookIterations, pieceSeed).centroids;
    }

    // Encode every image
    codes_.assign(imageCount, std::vector<std::uint8_t>(subvectorCount_));
    for (int index = 0; index < imageCount; index++) {
        for (int piece = 0; piece < subvectorCount_; piece++) {
            const Vector part = subvector(residuals[index], piece, subvectorDimension_);
            codes_[index][piece] = static_cast<std::uint8_t>(findNearestCentroid(part, codebooks_[piece]));
        }
    }

    std::cerr << "IVFPQ: " << kclusters_ << " clusters, nprobe = " << nprobe_ << ", k-means "
              << coarse.iterations << " iterations\n"
              << "IVFPQ: M = " << subvectorCount_ << " pieces of " << subvectorDimension_
              << " dimensions, " << codebookSize << " codewords each, " << subvectorCount_
              << " bytes per image (instead of " << dimension * sizeof(float) << ")\n";
}

std::vector<std::vector<double>> IvfPqSearch::distanceTable(const Vector& residual) const {
    std::vector<std::vector<double>> table(subvectorCount_);
    for (int piece = 0; piece < subvectorCount_; piece++) {
        const Vector part = subvector(residual, piece, subvectorDimension_);
        table[piece].reserve(codebooks_[piece].size());
        for (const Vector& codeword : codebooks_[piece]) {
            table[piece].push_back(squaredDistance(part, codeword));
        }
    }

    return table;
}

std::vector<int> IvfPqSearch::search(const Vector& query) {
    // The coarse centers closest to the query
    std::vector<std::pair<double, int>> clusterDistances;
    clusterDistances.reserve(centroids_.size());
    for (size_t cluster = 0; cluster < centroids_.size(); cluster++) {
        clusterDistances.emplace_back(euclideanDistance(query, centroids_[cluster]), static_cast<int>(cluster));
    }
    const size_t probeCount = std::min(clusterDistances.size(), static_cast<size_t>(nprobe_));
    std::partial_sort(clusterDistances.begin(), clusterDistances.begin() + probeCount, clusterDistances.end());

    // Approximate distance to every image in those lists
    std::vector<std::pair<double, int>> candidates;
    for (size_t probe = 0; probe < probeCount; probe++) {
        const int cluster = clusterDistances[probe].second;

        // The images in this list are stored relative to its centre, so the query must be too
        const Vector queryResidual = subtract(query, centroids_[cluster]);
        const std::vector<std::vector<double>> table = distanceTable(queryResidual);

        for (int index : clusterMembers_[cluster]) {
            double distance = 0.0;
            for (int piece = 0; piece < subvectorCount_; piece++) {
                distance += table[piece][codes_[index][piece]];
            }
            candidates.emplace_back(std::sqrt(distance), index);
        }
    }

    const size_t keep = std::min(candidates.size(), static_cast<size_t>(resultsPerQuery));
    std::partial_sort(candidates.begin(), candidates.begin() + keep, candidates.end());

    std::vector<int> ranking;
    ranking.reserve(keep);
    for (size_t i = 0; i < keep; i++) {
        ranking.push_back(candidates[i].second);
    }

    return ranking;
}
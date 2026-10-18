#include "kmeans.hpp"

#include <algorithm>
#include <cassert>
#include <random>


// Returns the index of the centroid closest to the point
static int findNearestCentroid(const Vector& point, const VectorList& centroids) {
    int nearestIndex = 0;
    double nearestDistance = squaredDistance(point, centroids[0]);

    for (size_t c=1; c<centroids.size(); c++) {
        double distance = squaredDistance(point, centroids[c]);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestIndex = static_cast<int>(c);
        }
    }
    
    return nearestIndex;
}

// Pick the initial k points at random
static VectorList chooseInitialCentroids(const VectorList& points, int k, std::mt19937& generator) {
    std::vector<int> order(points.size());
    for (size_t i=0; i<order.size(); i++) {
        order[i] = static_cast<int>(i);
    }
    std::shuffle(order.begin(), order.end(), generator);

    VectorList centroids;
    centroids.reserve(k);
    for (int c=0; c<k; c++) {
        centroids.push_back(points[order[c]]);
    }

    return centroids;
}

// Move each cetroid to the average of its assigned points
static void updateCentroids(const VectorList& points, const std::vector<int>& assignments, VectorList& centroids) {
    size_t dimension = points[0].size();
    std::vector<std::vector<double>> sums(centroids.size(), std::vector<double>(dimension, 0.0));
    std::vector<int> counts(centroids.size(), 0);

    for (size_t i=0; i<points.size(); i++) {
        int cluster = assignments[i];
        counts[cluster]++;
        for (size_t d=0; d<dimension; d++) {
            sums[cluster][d] += points[i][d];
        }
    }

    for (size_t c=0; c<centroids.size(); c++) {
        if (counts[c] == 0) {
            continue;
        }
        for (size_t d=0; d<dimension; d++) {
            centroids[c][d] = static_cast<float>(sums[c][d] / counts[c]);
        }
    }
}

KMeansResult kmeans(const VectorList& points, int k, int maxIterations, unsigned int seed) {
    assert(k >= 1);
    assert(static_cast<size_t>(k) <= points.size());

    std::mt19937 generator(seed);

    KMeansResult result;
    result.centroids = chooseInitialCentroids(points, k, generator);
    result.assignments.assign(points.size(), -1);

    for (int iteration=1; iteration<=maxIterations; iteration++) {
        bool pointMoved = false;

        for (size_t i=0; i<points.size(); i++) {
            int nearest = findNearestCentroid(points[i], result.centroids);
            if (nearest != result.assignments[i]) {
                result.assignments[i] = nearest;
                pointMoved = true;
            }
        }

        result.iterations = iteration;
        if (!pointMoved) {
            break;
        }

        updateCentroids(points, result.assignments, result.centroids);
    }

    return result;
}
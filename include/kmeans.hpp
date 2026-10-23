#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <vector>

#include "common.hpp"


struct KMeansResult {
    VectorList centroids;           // The k centers
    std::vector<int> assignments;   // Each input point's index of its center
    int iterations = 0;
};

// Groups the points into k clusters
// Stops when either no point changes cluster or at _maxIterations_
// Requires that all points have the same length and the k clusters are less than points.size()
KMeansResult kmeans(const VectorList& points, int k, int maxIterations, unsigned int seed);

// Returns the index of the point's nearest centroid
int findNearestCentroid(const Vector& point, const VectorList& centroids);

#endif
#include <cmath>
#include <iostream>
#include <string>

#include "kmeans.hpp"


static int failures = 0;

static void check(bool condition, const std::string& description) {
    if (!condition) {
        std::cerr << "FAILED: " << description << "\n";
        failures++;
    }
}

int main() {
    VectorList points = {
        {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f},
        {10.0f, 10.0f}, {10.0f, 11.0f}, {11.0f, 10.0f}
    };

    KMeansResult result = kmeans(points, 2, 100, 42);

    check(result.centroids.size() == 2, "exactly 2 centroids");
    check(result.assignments.size() == points.size(), "every point has an assignment");

    // Check that points in the same group also share a cluster
    check(result.assignments[0] == result.assignments[1], "point 0 and 1 same cluster");
    check(result.assignments[0] == result.assignments[2], "point 0 and 2 same cluster");
    check(result.assignments[3] == result.assignments[4], "point 3 and 4 same cluster");
    check(result.assignments[3] == result.assignments[5], "point 3 and 5 same cluster");
    check(result.assignments[0] != result.assignments[3], "two groups separated");

    // Make sure results are reproducable
    KMeansResult again = kmeans(points, 2, 100, 42);
    check(again.assignments == result.assignments, "same seed gets same assignments");

    if (failures == 0) {
        std::cout << "kmeans tests passed (" << result.iterations << " iterations)\n";

        return 0;
    }

    return 1;
}
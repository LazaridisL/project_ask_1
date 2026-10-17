#ifndef COMMON_HPP
#define COMMON_HPP

#include <vector>
#include <string>

// To be used in SIFT descriptors, image vectors etc
using Vector = std::vector<float>;

// To be used in the descriptors of an image etc
using VectorList = std::vector<Vector>;

// Represents one image in the database
struct ImageEntry {
    std::string imageId;    // Name of the image
    std::string sequenceId;
    Vector imageVector;
};

double euclideanDistance(const Vector& a, const Vector& b);

#endif
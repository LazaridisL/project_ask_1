#ifndef COMMON_HPP
#define COMMON_HPP

#include <vector>
#include <string>

// Represents one image in the database
struct ImageEntry {
    std::string imageId;    // Name of the image
    std::string sequenceId;
    std::vector<float> imageVector;
};

double euclideanDistance(const std::vector<float>& a, const std::vector<float>& b);

#endif
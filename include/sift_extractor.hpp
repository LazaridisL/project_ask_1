#ifndef SIFT_EXTRACTOR_HPP
#define SIFT_EXTRACTOR_HPP

#include <string>

#include "common.hpp"

// Reads an image in grayscale and returns its SIFT descriptors.
// Returns an empty list if the image cannot be read or has no keypoints.
VectorList extractSiftDescriptors(const std::string& imagePath);

#endif
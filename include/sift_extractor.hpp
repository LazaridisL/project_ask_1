#ifndef SIFT_EXTRACTOR_HPP
#define SIFT_EXTRACTOR_HPP

#include <string>

#include <opencv2/core.hpp>

#include "common.hpp"

// Reads an image in grayscale and returns its SIFT descriptors.
// Returns an empty list if the image cannot be read or has no keypoints.
VectorList extractSiftDescriptors(const std::string& imagePath);

bool save_descriptors(const std::string& path, const VectorList& descriptors);

bool load_descriptors(const std::string& path, VectorList& descriptors);


#endif
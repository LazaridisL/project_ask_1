#ifndef SIFT_EXTRACTOR_HPP
#define SIFT_EXTRACTOR_HPP

#include <string>
#include <vector>

#include <common.hpp>

VectorList extractSiftDescriptors(const std::string& imagePath);

#endif
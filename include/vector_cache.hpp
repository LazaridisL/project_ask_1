#ifndef VECTOR_CACHE_HPP
#define VECTOR_CACHE_HPP

#include <string>
#include <vector>

#include "common.hpp"


// A fingerprint for the cache file so different parameters cannot use
// the wrong cache file
std::string cacheFingerprint(const std::vector<std::string>& texts);

// Loads the vocabulary from cacheFile if it exists and has K words
// Builds it and saves it there otherwise
// Returns an empty list if it cannot be built
VectorList loadOrBuildVocabulary(const std::vector<std::string>& trainingImagePaths,
                                 int K, int S, unsigned int seed,
                                 const std::string& cacheFile);

// Loads the image vectors from cacheFile
// Otherwise computes (SIFT + histogram) and saves it there
VectorList loadOrComputeImageVectors(const std::vector<std::string>& imagePaths,
                                     const VectorList& vocabulary,
                                     const std::string& cacheFile,
                                     const std::string& label);

// True for the all-zero vector given to images without descriptors
bool isEmptyImageVector(const Vector& vector);

#endif
#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <vector>

#include "common.hpp"


// Builds the image database that contains images 2-6 of every sequence and the first D MIRFlickr images
std::vector<ImageEntry> buildDatabase(const std::vector<std::string>& querySequences,
                                      const VectorList& hpatchesVectors,
                                      const std::vector<std::string>& mirflickrPaths,
                                      const VectorList& mirflickrVectors,
                                      int D);

// Stores the images used as queries in a list of ImageEntry
std::vector<ImageEntry> buildQueries(const std::vector<std::string>& querySequences, const VectorList& hpatchesVectors);

// Returns the vectors of the entries in a list
VectorList vectorsOf(const std::vector<ImageEntry>& entries);

#endif
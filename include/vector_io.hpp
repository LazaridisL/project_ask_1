#ifndef VECTOR_IO_HPP
#define VECTOR_IO_HPP

#include <string>

#include "common.hpp"


// Saves a list of vectors to a binary file in the following format:
// count, dimension and count * dimension floats
// Returns false if the vector lengths differ or the file is not writeable
bool saveVectors(const std::string& path, const VectorList& vectors);

// Loads a file created by saveVectors into the "vectors" variable.
// Returns false if any error occurs during loading
bool loadVectors(const std::string& path, VectorList& vectors);

#endif
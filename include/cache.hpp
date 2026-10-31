#ifndef CACHE_HPP
#define CACHE_HPP

#include <string.h>
#include <vector>

#include "common.hpp"


// Returns 
std::string describeList(const std::string& title, const std::vector<std::string>& items);

// Loads the vectors from the file "name" if it matches the current "description" 
bool loadFromCache(const std::string& cacheFolder, const std::string& name, const std::string& description, VectorList& vectors);

// Saves the vectors to the file "name"
void saveToCache(const std::string& cacheFolder, const std::string& name, const std::string& description, const VectorList& vectors);

#endif
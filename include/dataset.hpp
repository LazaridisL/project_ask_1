#ifndef DATASET_HPP
#define DATASET_HPP

#include <string>
#include <vector>


// HPatches sequence name
struct Split {
    std::vector<std::string> train;
    std::vector<std::string> validation;
    std::vector<std::string> test;
};

// Reads the split file
bool readSplit(const std::string& path, Split& split);

// Helper for HPatches image number
std::string hpatchesImagePath(const std::string& hpatchesFolder, const std::string& sequence, int imageNumber);

// Distinction between illumination (true) and viewpoint (false) sequences
bool isIlluminationSequence(const std::string& sequence);

// Paths for the mirflickr images in the folder provided
std::vector<std::string> listMirflickrImages(const std::string& mirflickrFolder);
                    
#endif
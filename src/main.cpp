#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#include "sift_extractor.hpp"
#include "vector_io.hpp"
#include "dataset.hpp"


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: ./search <hpatches folder> <mirflickr folder> <split file>\n";

        return 1;
    }

    std::string hpatchesFolder = argv[1];
    std::string mirflickrFolder = argv[2];
    std::string splitPath = argv[3];

    // Read the split file
    Split split;
    if (!readSplit(splitPath, split)) {
        return 1;
    }
    std::cout << "Sequences: " << split.train.size() << " train, " << split.validation.size() << " validation, " << split.test.size() << " test\n";

    // HPatches images
    int missingImages = 0;
    std::vector<std::vector<std::string>> allSets = {split.train, split.validation, split.test};
    for (const std::vector<std::string>& set : allSets) {
        for (const std::string& sequence : set) {
            for (int imageNumber=1; imageNumber<=6;imageNumber++) {
                std::string path = hpatchesImagePath(hpatchesFolder, sequence, imageNumber);
                if (!std::filesystem::exists(path)) {
                    std::cerr << "Missing: " << path << "\n";
                    missingImages++;
                }
            }
        }
    }
    std::cout << "Missing HPatches images: " << missingImages << "\n";

    // MIRFlickr images
    std::vector<std:: string> mirflickrImages = listMirflickrImages(mirflickrFolder);
    std::cout << "MIRFlickr images: " << mirflickrImages.size() << "\n";
    if (!mirflickrImages.empty()) {
        std::cout << "First: " << mirflickrImages.front() << "\n";
        std::cout << "Last: " << mirflickrImages.back() << "\n";
    }

    // SIFT save and load on the first validation query
    if (split.validation.empty()) {
        std::cerr << "No validation sequences\n";
        return 1;
    }
    std::string imagePath = hpatchesImagePath(hpatchesFolder, split.validation[0], 1);
    VectorList descriptors = extractSiftDescriptors(imagePath);
    std::cout << "\n" << imagePath << ": " << descriptors.size() << " descriptors\n";

    std::filesystem::create_directories("cache");
    std::string cachePath = "cache/test_save_load.bin";

    if (!saveVectors(cachePath, descriptors)) {
        std::cerr << "Could not save " << cachePath << "\n";
        return 1;
    }

    VectorList loaded;
    if (!loadVectors(cachePath, loaded)) {
        std::cerr << "Could not load " << cachePath << "\n";
        return 1;
    }

    if (loaded == descriptors) {
        std::cout << "Test OK: " << loaded.size() << " descriptors saved and loaded unchanged\n";
    } else {
        std::cerr << "Test FAILED: loaded data differs from original\n";
        return 1;
    }

    std::filesystem::remove(cachePath);
    
    return 0;
}
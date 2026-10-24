#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

#include "dataset.hpp"
#include "options.hpp"
#include "vector_cache.hpp"


// Paths for all 6 images in a sequence
static std::vector<std::string> allImagesOf(const std::vector<std::string>& sequences,
                                            const std::string& hpatchesFolder)
{
    std::vector<std::string> paths;
    for (const std::string& sequence : sequences) {
        for (int imageNumber = 1; imageNumber <= 6; imageNumber++) {
            paths.push_back(hpatchesImagePath(hpatchesFolder, sequence, imageNumber));
        }
    }
    return paths;
}


int main(int argc, char* argv[]) {
    Options options;
    if (!parseOptions(argc, argv, options)) {
        printUsage();
        
        return 1;
    }

    Split split;
    if (!readSplit(options.splitPath, split)) {
        return 1;
    }
    const std::vector<std::string>& querySequence = (options.querySet == "validation") ? split.validation : split.test;

    std::vector<std::string> trainingPaths = allImagesOf(split.train, options.hpatchesFolder);
    std::vector<std::string> hpatchesPaths = allImagesOf(querySequence, options.hpatchesFolder);
    std::vector<std::string> mirflickrPaths = listMirflickrImages(options.mirflickrFolder);

    // Cache files are named based on the parameters used to produce them and the fingerprint of the exact images used
    // Different K, S or seed cannot use the same file
    std::string vocabularyFile;
    std::string hpatchesFile;
    std::string mirflickrFile;

    if (!options.cacheFolder.empty()) {
        std::filesystem::create_directories(options.cacheFolder);
        std::string parameters = "K" + std::to_string(options.vocabularySize)
                               + "S" + std::to_string(options.samplesPerImage)
                               + "seed" + std::to_string(options.seed);

        std::vector<std::string> vocabularyInputs = trainingPaths;
        vocabularyInputs.push_back(parameters);
        std::string vocabularyKey = cacheFingerprint(vocabularyInputs);

        std::vector<std::string> hpatchesInputs = hpatchesPaths;
        hpatchesInputs.push_back(vocabularyKey);

        std::vector<std::string> mirflickrInputs = mirflickrPaths;
        mirflickrInputs.push_back(vocabularyKey);

        std::filesystem::path folder = options.cacheFolder;
        vocabularyFile = (folder / ("vocabulary_" + parameters + "_" + vocabularyKey + ".bin")).string();
        hpatchesFile = (folder / ("hpatches_" + options.querySet + "_" + parameters + "_"
                                  + cacheFingerprint(hpatchesInputs) + ".bin")).string();
        mirflickrFile = (folder / ("mirflickr_" + parameters + "_"
                                   + cacheFingerprint(mirflickrInputs) + ".bin")).string();
    }



    // Create the vocabulary from the training images
    VectorList vocabulary = loadOrBuildVocabulary(trainingPaths, options.vocabularySize,
                                                  options.samplesPerImage, options.seed,
                                                  vocabularyFile);
    if (vocabulary.empty()) {
        return 1;
    }

    // Vectors from the HPatches images query set
    VectorList hpatchesVectors = loadOrComputeImageVectors(hpatchesPaths, vocabulary, hpatchesFile, "HPatches " + options.querySet);
    
    // Vectors of the MIRFlickr images
    VectorList mirflickrVectors = loadOrComputeImageVectors(mirflickrPaths, vocabulary, mirflickrFile, "MIRFlickr");

    int emptyMirflickr = 0;
    for (const Vector& vector : mirflickrVectors) {
        if (isEmptyImageVector(vector)) {
            emptyMirflickr++;
        }
    }

    std::cout << "Vocabulary " << vocabulary.size() << " words\n";
    std::cout << "HPatches " << options.querySet << ": " << hpatchesVectors.size() << " vectors\n";
    std::cout << "MIRFlickr " << mirflickrVectors.size() << " vectors " << emptyMirflickr << " without descriptors\n";
    
    return 0;
}
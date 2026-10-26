#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "database.hpp"
#include "dataset.hpp"
#include "evaluation.hpp"
#include "exact_search.hpp"
#include "options.hpp"
#include "vector_cache.hpp"


static std::unique_ptr<SearchMethod> createSearchMethod(const Options& options) {
    if (options.method == "exact") {
        return std::make_unique<ExactSearch>();
    }

    return nullptr;
}

// The method name written on the first line of the output file.
static std::string methodDisplayName(const std::string& method) {
    if (method == "exact") return "Exact";
    if (method == "lsh") return "LSH";
    if (method == "hypercube") return "Hypercube";
    if (method == "ivfflat") return "IVFFlat";
    if (method == "ivfpq") return "IVFPQ";

    return method;
}

static std::vector<std::string> allImagesOf(const std::vector<std::string>& sequences, const std::string& hpatchesFolder) {
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
    const std::vector<std::string>& querySequences =
        (options.querySet == "validation") ? split.validation : split.test;

    std::vector<std::string> trainingPaths = allImagesOf(split.train, options.hpatchesFolder);
    std::vector<std::string> hpatchesPaths = allImagesOf(querySequences, options.hpatchesFolder);
    std::vector<std::string> mirflickrPaths = listMirflickrImages(options.mirflickrFolder);

    // Cache files are named based on the parameters used to produce them and the fingerprint of the exact images used
    // Different K, S or seed cannot use the same file
    std::string vocabularyFile;
    std::string hpatchesFile;
    std::string mirflickrFile;

    if (!options.cacheFolder.empty()) {
        std::filesystem::create_directories(options.cacheFolder);
        std::string parameters = "K" + std::to_string(options.vocabularySize)
                               + "_S" + std::to_string(options.samplesPerImage)
                               + "_seed" + std::to_string(options.seed);

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
    VectorList hpatchesVectors = loadOrComputeImageVectors(
        hpatchesPaths, vocabulary, hpatchesFile, "HPatches " + options.querySet);

    // Vectors of the MIRFlickr images
    VectorList mirflickrVectors = loadOrComputeImageVectors(
        mirflickrPaths, vocabulary, mirflickrFile, "MIRFlickr");

    // Create the images database
    std::vector<ImageEntry> database = buildDatabase(querySequences, hpatchesVectors, mirflickrPaths, mirflickrVectors, options.distractorCount);
    std::vector<ImageEntry> queries = buildQueries(querySequences, hpatchesVectors);
    std::cerr << "Database: " << database.size() << " images, "
              << queries.size() << " queries\n";

    // Build the index of the chosen method
    std::unique_ptr<SearchMethod> method = createSearchMethod(options);
    if (!method) {
        std::cerr << "Error: method \"" << options.method << "\" is not implemented yet\n";
        return 1;
    }
    method->build(vectorsOf(database));

    // Run and evaluate every query
    std::vector<QueryResult> results = runQueries(*method, database, queries);

    if (!writeOutputFile(options.outputPath, methodDisplayName(options.method), database, results)) {
        std::cerr << "Error: could not write " << options.outputPath << "\n";
        return 1;
    }
    printSummary(std::cout, results);
    return 0;
}
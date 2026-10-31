#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "database.hpp"
#include "dataset.hpp"
#include "evaluation.hpp"
#include "exact_search.hpp"
#include "options.hpp"
#include "cache.hpp"
#include "vocabulary.hpp"
#include "lsh_search.hpp"
#include "ivfflat_search.hpp"
#include "hypercube_search.hpp"
#include "ivfpq_search.hpp"


static const int vocabularyKMeansIterations = 50;

static std::unique_ptr<SearchMethod> createSearchMethod(const Options& options) {
    if (options.method == "exact") {
        return std::make_unique<ExactSearch>();
    }

    if (options.method == "lsh") {
    return std::make_unique<LSHSearch>(
        options.L,
        options.k,
        options.w,
        options.seed
    );
}

    if (options.method == "ivfflat") {
        return std::make_unique<IvfFlatSearch>(options.kclusters, options.nprobe, options.seed);
    }

    if (options.method == "hypercube") {
        return std::make_unique<HypercubeSearch>(options.kproj, options.w, options.M, options.probes, options.seed);
    }

    if (options.method == "ivfpq") {
        return std::make_unique<IvfPqSearch>(options.kclusters, options.nprobe, options.M, options.nbits, options.seed);
    }

    return nullptr;
}

// Used to write the method name on the first line of the output file
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
        for (int imageNumber=1; imageNumber<=6; imageNumber++) {
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
    const std::vector<std::string>& querySequences = (options.querySet == "validation") ? split.validation : split.test;

    std::vector<std::string> trainingPaths = allImagesOf(split.train, options.hpatchesFolder);
    std::vector<std::string> hpatchesPaths = allImagesOf(querySequences, options.hpatchesFolder);
    std::vector<std::string> mirflickrPaths = listMirflickrImages(options.mirflickrFolder);

    // Cache files are named based on the parameters used to produce them 
    // Different K, S or seed cannot use the same file
    const std::string parameters = "K" + std::to_string(options.vocabularySize)
                                 + "_S" + std::to_string(options.samplesPerImage)
                                 + "_seed" + std::to_string(options.seed);

    const std::string vocabularyDescription = "K " + std::to_string(options.vocabularySize) + "\n"
                                            + "S " + std::to_string(options.samplesPerImage) + "\n"
                                            + "seed " + std::to_string(options.seed) + "\n"
                                            + describeList("training sequences", split.train);

    // Create the vocabulary from the training images
    const std::string vocabularyName = "vocabulary_" + parameters;
    VectorList vocabulary;
    if (!loadFromCache(options.cacheFolder, vocabularyName, vocabularyDescription, vocabulary)) {
        std::cerr << "Building vocabulary from " << trainingPaths.size() << " training images\n";
        vocabulary = buildVocabulary(trainingPaths, options.vocabularySize, options.samplesPerImage, vocabularyKMeansIterations, options.seed);

        if (vocabulary.empty()) {
            return 1;
        }
        saveToCache(options.cacheFolder, vocabularyName, vocabularyDescription, vocabulary);
    }

    // Vectors from the HPatches images query set
    const std::string hpatchesName = "hpatches_" + options.querySet + "_" + parameters;
    const std::string hpatchesDescription = vocabularyDescription + describeList(options.querySet + " sequences", querySequences);
    VectorList hpatchesVectors;
    if (!loadFromCache(options.cacheFolder, hpatchesName, hpatchesDescription, hpatchesVectors)) {
        hpatchesVectors = computeImageVectors(hpatchesPaths, vocabulary, "HPatches " + options.querySet);
        saveToCache(options.cacheFolder, hpatchesName, hpatchesDescription, hpatchesVectors);
    }

    // Vectors of the MIRFlickr images
    const std::string mirflickrName = "mirflickr_" + parameters;
    const std::string mirflickrDescription = vocabularyDescription + "mirflickr images: " + std::to_string(mirflickrPaths.size()) + "\n";
    VectorList mirflickrVectors;
    if (!loadFromCache(options.cacheFolder, mirflickrName, mirflickrDescription, mirflickrVectors)) {
        mirflickrVectors = computeImageVectors(mirflickrPaths, vocabulary, "MIRFlickr");
        saveToCache(options.cacheFolder, mirflickrName, mirflickrDescription, mirflickrVectors);
    }

    // Create the images database
    std::vector<ImageEntry> database = buildDatabase(querySequences, hpatchesVectors, mirflickrPaths, mirflickrVectors, options.distractorCount);
    std::vector<ImageEntry> queries = buildQueries(querySequences, hpatchesVectors);
    std::cerr << "Database: " << database.size() << " images, " << queries.size() << " queries\n";

    // Build the index of the chosen method
    std::unique_ptr<SearchMethod> method = createSearchMethod(options);
    if (!method) {
        std::cerr << "Error: No method \"" << options.method << "\"\n";
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
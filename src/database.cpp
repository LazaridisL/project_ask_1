#include "database.hpp"

#include <filesystem>
#include <iostream>

#include "vector_cache.hpp"


static size_t hpatchesIndex(size_t sequencePosition, int imageNumber) {
    return sequencePosition * 6 + (imageNumber - 1);
}

std::vector<ImageEntry> buildDatabase(const std::vector<std::string>& querySequences,
                                      const VectorList& hpatchesVectors,
                                      const std::vector<std::string>& mirflickrPaths,
                                      const VectorList& mirflickrVectors,
                                      int D) {
    
    std::vector<ImageEntry> database;

    // HPatches images
    for (size_t s=0; s<querySequences.size(); s++) {
        for (int imageNumber=2; imageNumber<=6; imageNumber++) {
            ImageEntry entry;
            entry.imageId = querySequences[s] + "/" + std::to_string(imageNumber);
            entry.sequenceId = querySequences[s];
            entry.imageVector = hpatchesVectors[hpatchesIndex(s, imageNumber)];
            database.push_back(entry);
        }
    }

    // D MIRFlickr images
    int added = 0;
    for (size_t i=0; i<mirflickrPaths.size() && added<D; i++) {
        if (isEmptyImageVector(mirflickrVectors[i])) {
            continue;
        }
        ImageEntry entry;
        entry.imageId = std::filesystem::path(mirflickrPaths[i]).filename().string();
        entry.sequenceId = "";
        entry.imageVector = mirflickrVectors[i];
        database.push_back(entry);
        added++;
    }

    return database;
}

std::vector<ImageEntry> buildQueries(const std::vector<std::string>& querySequences, const VectorList& hpatchesVectors) {
    std::vector<ImageEntry> queries;

    for (size_t s=0; s<querySequences.size(); s++) {
        ImageEntry query;
        query.imageId = querySequences[s] + "/1";
        query.sequenceId = querySequences[s];
        query.imageVector = hpatchesVectors[hpatchesIndex(s, 1)];
        queries.push_back(query);
    }

    return queries;
}

VectorList vectorsOf(const std::vector<ImageEntry>& entries) {
    VectorList vectors;
    vectors.reserve(entries.size());
    for (const ImageEntry& entry : entries) {
        vectors.push_back(entry.imageVector);
    }

    return vectors;
}
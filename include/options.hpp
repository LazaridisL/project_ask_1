#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <string>


// Struct for the command line parameters
// The defaults are used when no specific parameters given
struct Options {
    std::string hpatchesFolder;     // -hp
    std::string mirflickrFolder;    // -mir
    std::string splitPath;          // -split
    int vocabularySize = 256;       // -vocab
    int samplesPerImage = 500;      // -S
    int distractorCount = 1000;     // -D
    std::string querySet;           // -set 
    std::string outputPath;         // -o
    unsigned int seed = 1;          // -seed
    std::string cacheFolder;        // -cache, in case it is missing, everything is computed from scratch

    std::string method;             // Search method selection

    // For LSH and Hypercube
    int k = 4;                      // -k
    int L = 5;                      // -L
    double w = 0.05;                // -w
    int kproj = 10;                 // -kproj
    int M = 0;                      // -M (Hypercube max candidates (500) or IVFPQ subvectors (16). Default is set later)
    int probes = 10;                // -probes

    // IVFFlat and IVFPQ
    int kclusters = 0;              // -kclusters (sqrt(n)) decided when n is known
    int nprobe = 5;                  // -nprobe
    int nbits = 8;                  // -nbits
};

// Parses the command line arguments into Options struct
// Returns false for unknown arguments, missing value
bool parseOptions(int argc, char* argv[], Options& options);

// Prints how to run the program
void printUsage();

#endif
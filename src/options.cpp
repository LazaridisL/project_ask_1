#include "options.hpp"

#include <iostream>
#include <stdexcept>


static bool toInt(const std::string& text, const std::string& name, int& result) {
    try {
        size_t used = 0;
        result = std::stoi(text, &used);
        if (used == text.size()) {
            return true;
        }
    } catch (const std::exception&) {

    }
    std::cerr << "Error: " << name << " expects a whole number. Got \"" << text << "\n";
    return false;
}

static bool toDouble(const std::string& text, const std::string& name, double& result) {
    try {
        size_t used = 0;
        result = std::stod(text, &used);
        if (used == text.size()) {
            return true;
        }
    } catch (const std::exception&) {

    }
    std::cerr << "Error: " << name << " expects a number. Got \"" << text << "\"\n";
    return false;
}

static bool isMethodFlag(const std::string& argument) {
    return ((argument == "-exact") || (argument == "-lsh") || (argument == "-hypercube") || (argument == "-ivfflat") || (argument == "-ivfpq"));
}

bool parseOptions(int argc, char* argv[], Options& options) {
    bool mGiven = false;

    for (int i=1; i<argc; i++) {
        std::string argument = argv[i];

        if (isMethodFlag(argument)) {
            if (!options.method.empty()) {
                std::cerr << "Error: choose only one search method\n";
                return false;
            }
            options.method = argument.substr(1);    // Remove the dash "-"
            continue;
        }

        if (i+1 >= argc) {
            std::cerr << "Error: " << argument << " needs a value\n";
            return false;
        }
        std::string value = argv[++i];
        bool ok = true;

        if (argument == "-hp") {
            options.hpatchesFolder = value;
        } else if (argument == "-mir") {
            options.mirflickrFolder = value;
        } else if (argument == "-split") {
            options.splitPath = value;
        } else if (argument == "-vocab") {
            ok = toInt(value, argument, options.vocabularySize);
        } else if (argument == "-S") {
            ok = toInt(value, argument, options.samplesPerImage);
        } else if (argument == "-D") {
            ok = toInt(value, argument, options.distractorCount);
        } else if (argument == "-set") {
            options.querySet = value;
        } else if (argument == "-o") {
            options.outputPath = value;
        } else if (argument == "-seed") {
            int seed = 0;
            ok = toInt(value, argument, seed);
            options.seed = static_cast<unsigned int>(seed);
        } else if (argument == "-cache") {
            options.cacheFolder = value;
        } else if (argument == "-k") {
            ok = toInt(value, argument, options.k);
        } else if (argument == "-L") {
            ok = toInt(value, argument, options.L);
        } else if (argument == "-w") {
            ok = toDouble(value, argument, options.w);
        } else if (argument == "-kproj") {
            ok = toInt(value, argument, options.kproj);
        } else if (argument == "-M") {
            ok = toInt(value, argument, options.M);
            mGiven = true;
        } else if (argument == "-probes") {
            ok = toInt(value, argument, options.probes);
        } else if (argument == "-kclusters") {
            ok = toInt(value, argument, options.kclusters);
        } else if (argument == "-nprobe") {
            ok = toInt(value, argument, options.nprobe);
        } else if (argument == "-nbits") {
            ok = toInt(value, argument, options.nbits);
        } else {
            std::cerr << "Error: unknown argument " << argument << "\"\n";
            return false;
        }

        if (!ok) {
            return false;
        }
    }

    if (options.hpatchesFolder.empty() || options.mirflickrFolder.empty()
        || options.splitPath.empty() || options.querySet.empty()
        || options.outputPath.empty() || options.method.empty()) {
            std::cerr << "Error: -hp, -mir, -split, -set, -o and a search method are required\n";
            return false;
    }

    if ((options.querySet != "validation") && (options.querySet != "test")) {
        std::cerr << "Error: -set must be \"validation\" or \"test\"\n";
        return false;
    }    

    // The default value for M depending on the search method
    if (!mGiven) {
        options.M = (options.method == "hypercube") ? 500 : 16;
    }

    return true;
}

void printUsage() {
    std::cerr << "Usage: ./search -hp <path> -mir <path> -split <file> -set <validation|test>\n"
              << "                -o <output file> [-vocab K] [-S S] [-D D] [-seed seed]\n"
              << "                <-exact | -lsh | -hypercube | -ivfflat | -ivfpq> [method parameters]\n"
              << "                [-cache folder]\n";
}
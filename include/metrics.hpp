#ifndef METRICS_HPP
#define METRICS_HPP

#include <vector>


// Recall @ n 
double recall(const std::vector<bool>& isRelevant, int n, int relevantCount);

// AP @ n
double averagePrecision(const std::vector<bool>& isRelevant, int n, int relevantCount);

#endif
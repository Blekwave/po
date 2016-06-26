#pragma once

#include <vector>

using std::vector;

struct FLPData {
    int locations;
    int customers;
    vector<int> supplies; // per location
    vector<double> build_costs; // per location
    vector<int> demands; // per customer
    vector<vector<double>> ship_costs; // per location, per customer
};

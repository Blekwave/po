#pragma once

#include <glpk.h>
#include <ctime>

#include <string>
#include <vector>

#include "FLPData.h"
#include "LPP.h"

using std::string;
using std::vector;

class FLP {
    string title;
    FLPData data;

    int master_solutions;
    double total_time;
    double pricing_time;

    LPP initializeMaster();
    LPP initializeSub(vector<int> built_locations);
    void updateSubLocations(LPP &sub, vector<int> built_locations);
    void printSolution(LPP &master, LPP &sub, bool debug);
    double totalBuildCost(vector<int> built_locations);

    LPP constrainedMaster(vector<double> &constants,
        vector<vector<double>> &rows);

    public:
        FLP(string title, FLPData pd);
        void printProblemData();
        void solve(bool debug);
};

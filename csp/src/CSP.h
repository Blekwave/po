#pragma once

#include <vector>

#include <glpk.h>
#include <ctime>
#include <string>

#include "ProblemData.h"
#include "LPP.h"

using std::string;

class CSP {
    string title;
    ProblemData pd;

    int master_solutions;
    double total_time;
    double pricing_time;

    LPP initializeLPP();
    void printSolution(LPP &lpp, bool debug);

    public:
        CSP(string title, ProblemData pd);
        void printProblemData();
        void solve(bool debug);
};

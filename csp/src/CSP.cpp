#include "CSP.h"
#include "LPP.h"
#include "Knapsack.h"

#include <iostream>
#include <string>
#include <cmath>
#include <ctime>

using std::cout;
using std::string;
using std::abs;

const double PRECISION = 0.001;

CSP::CSP(string title, ProblemData pd) : title(title), pd(pd) {
    // Do nothing
}

void CSP::printProblemData() {
    cout << pd.cuts << " cuts:" << "\n";
    for (int i = 0; i < pd.cuts; i++) {
        cout << "Width " << pd.widths[i]
             << ": " << pd.demands[i] << " units \n";
    }
}

vector<vector<int>> genTrivialPatterns(ProblemData &pd) {
    vector<vector<int>> patterns;
    for (int c = 0; c < pd.cuts; c++) {
        vector<int> pattern(pd.cuts, 0);
        pattern[c] = pd.stock_width / pd.widths[c];
        patterns.push_back(pattern);
    }
    return patterns;
}

LPP CSP::initializeLPP() {
    LPP lpp(ObjDir::min);

    for (int demand : pd.demands) {
        lpp.addRow(LPBounds::fixed, demand, demand);
    }

    vector<vector<int>> patterns = genTrivialPatterns(pd);
    for (vector<int> &pattern : patterns) {
        lpp.addCol(1, LPBounds::lower, 0, 0);
        lpp.addConstrCol(pattern);
    }

    return lpp;
}

// Required data:
// - Number of master problems solved
// - Total CPU time
// - CPU time for pricing problems
// - Final master problem objective value
void CSP::printSolution(LPP &lpp, bool debug) {
    if (debug) {
        cout << "Solution found!\n";

        cout << "Patterns:\n";
        vector<double> pattern_counts = lpp.primalVars();
        int variables = pattern_counts.size();
        for (int i = 0; i < variables; i++) {
            cout << "x" << i << ": " << pattern_counts[i] << "\n";
        }

        cout << "Master problem solutions: " << master_solutions << "\n";
        cout << "Total CPU time: " << total_time << " seconds\n";
        cout << "Pricing CPU time: " << pricing_time << " seconds\n";
        cout << "Objective function value: " << lpp.objective() << "\n";
    } else {
        cout << title << " & "
            << master_solutions << " & "
            << total_time << " & "
            << pricing_time << " & "
            << lpp.objective() << "\n";
    }
}

void CSP::solve(bool debug) {
    bool silent = !debug;

    if (!silent) {
        cout << "Solve " << title << "\n";
        printProblemData();
    } else {
        LPP::termOut(silent);
    }

    clock_t begin = std::clock();

    LPP lpp = initializeLPP();
    master_solutions = 0;
    // total_time not initialized because it's not computed incrementally
    pricing_time = 0;

    while (true) {
        lpp.simplex();
        master_solutions++;

        clock_t pricing_begin = std::clock();
        Knapsack ks(pd.stock_width, pd.widths, lpp.dualVars());
        clock_t pricing_end = std::clock();
        pricing_time += double(pricing_end - pricing_begin) / CLOCKS_PER_SEC;


        if (abs(ks.solution() - 1) <= PRECISION) {
            clock_t end = std::clock();
            total_time = double(end - begin) / CLOCKS_PER_SEC;

            printSolution(lpp, debug);
            return;
        } else {
            if (!silent) {
                cout << "Knapsack value: " << ks.solution() << "\n";
            }
            lpp.addCol(1, LPBounds::lower, 0, 0);
            lpp.addConstrCol(ks.solution_counts());
        }
    }
}

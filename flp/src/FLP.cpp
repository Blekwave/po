#include "FLP.h"
#include "FLPData.h"
#include "utility.h"

#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include <cmath>
#include <ctime>

using std::cout;
using std::string;
using std::vector;
using std::abs;
using std::numeric_limits;

const double PRECISION = 0.001;

const bool CAPACITATED = false;

FLP::FLP(string title, FLPData data) : title(title), data(data) {
    // Do nothing
}

void FLP::printProblemData() {
    cout << data.locations << " locations, "
        << data.customers << " customers\n";

    for (int i = 0; i < data.locations; i++) {
        cout << "Location " << i << ": "
            << "supply " << data.supplies[i] << ", "
            << "build cost " << data.build_costs[i] << "\n";

        cout << "Shipping costs:\n";
        for (int j = 0; j < data.customers; j++) {
            cout << j << ": " << data.ship_costs[i][j];
            if (j % 6 == 5) {
                cout << "\n";
            } else {
                cout << "; ";
            }
        }
        cout << "\n\n";
    }

    cout << "Customer demands:\n";
    for (int j = 0; j < data.customers; j++) {
        cout << j << ": " << data.demands[j];
        if (j % 6 == 5) {
            cout << "\n";
        } else {
            cout << "; ";
        }
    }
    cout << "\n\n";
}

LPP FLP::initializeMaster() {
    LPP master(ObjDir::min);

    master.addCol(1, LPBounds::free, 0, 0); // z_0 (objective)

    for (int i = 0; i < data.locations; i++) {
        master.addBinaryCol(0); // y_i
    }

    // y_0 + y_1 + ... + y_i >= 1
    master.addRow(LPBounds::lower, 1, 1);

    vector<double> constraint_row{0}; // 0*z_0

    for (int i = 0; i < data.locations; i++) {
        constraint_row.push_back(1); // 1 for each y_i
    }

    master.addConstrRow(constraint_row);

    if (CAPACITATED) {
        // Capacity constraint: there must be enough capacity for the required
        // demand
        double total_demand = 0;
        for (int demand : data.demands) {
            total_demand += demand;
        }
        master.addRow(LPBounds::lower, total_demand, total_demand);
        vector<double> cap_row{0};
        cap_row.insert(cap_row.end(), data.supplies.begin(),
                data.supplies.end());
        master.addConstrRow(cap_row);
    }

    return master;
}

double FLP::totalBuildCost(vector<int> built_locations) {
    double total_build_cost = 0;
    for (int i = 0; i < data.locations; i++) {
        total_build_cost += data.build_costs[i] * built_locations[i];
    }
    return total_build_cost;
}

LPP FLP::initializeSub(vector<int> built_locations) {

    LPP sub(ObjDir::min);
    sub.setConstantTerm(totalBuildCost(built_locations));

    for (int i = 0; i < data.locations; i++) {
        for (int j = 0; j < data.customers; j++) {
            double cost = data.ship_costs[i][j];
            if (CAPACITATED) {
                cost *= data.demands[j];
            }
            sub.addCol(cost, LPBounds::lower, 0, 0);
        }
    }

    for (int j = 0; j < data.customers; j++) {
        sub.addRow(LPBounds::lower, 1, 1);
        vector<double> row(data.locations * data.customers, 0);
        for (int i = 0; i < data.locations; i++) {
            row[i * data.customers + j] = 1;
        }
        sub.addConstrRow(row);
    }

    for (int i = 0; i < data.locations; i++) {
        for (int j = 0; j < data.customers; j++) {
            int built = built_locations[i];
            sub.addRow(LPBounds::upper, built, built);
            vector<double> row(data.locations * data.customers, 0);
            int col = j + i * data.customers;
            row[col] = 1;
            sub.addConstrRow(row);
        }
    }

    if (CAPACITATED) {
        // Capacity constraints: for each location, the amount of units sent
        // from it must not exceed its capacity.

        for (int i = 0; i < data.locations; i++) {
            sub.addRow(LPBounds::upper, data.supplies[i], data.supplies[i]);
            vector<double> cap_row(data.locations * data.customers, 0);
            for (int j = 0; j < data.customers; j++) {
                cap_row[i * data.customers + j] = data.ship_costs[i][j];
            }
            sub.addConstrRow(cap_row);
        }
    }

    return sub;
}

void FLP::updateSubLocations(LPP &sub, vector<int> built_locations) {
    sub.setConstantTerm(totalBuildCost(built_locations));

    for (int i = 0; i < data.locations; i++) {
        for (int j = 0; j < data.customers; j++) {
            // The first term below is the offset for the first m constraints
            int constraint = data.customers + j + i * data.customers;
            int built = built_locations[i];
            sub.setRowBounds(constraint, LPBounds::upper, built, built);
        }
    }
}

// Required data:
// - Number of master problems solved
// - Total CPU time
// - CPU time for pricing problems
// - Final master problem objective value
void FLP::printSolution(LPP &master, LPP &sub, bool debug) {
    if (debug) {
        cout << "Solution found!\n";
        cout << "UB: " << sub.objective() << ", "
            << "LB: " << master.objective() << "\n";

        cout << "z, Built fclties.: ";
        vector<double> master_primal = master.primalVars();
        prettyPrintVector(master_primal, 10);

        cout << "Sub primal solution: \n";
        vector<double> sub_primal = sub.primalVars();
        prettyPrintVector(sub_primal, 10);

        cout << "Sub dual solution: \n";
        vector<double> sub_dual = sub.dualVars();
        prettyPrintVector(sub_dual, 10);

        master.saveProblemInfo("last_master.txt");
        sub.saveProblemInfo("last_sub.txt");
    } else {
        cout << "Silence\n";
    }
}

void FLP::solve(bool debug) {
    bool silent = !debug;

    if (!silent) {
        cout << "Solve " << title << "\n";
        printProblemData();
    } else {
        LPP::termOut(silent);
    }

    clock_t begin = std::clock();

    LPP master = initializeMaster();

    if (!silent) {
        cout << "Master problem initialized.\n";
        master.saveProblemInfo("first_master.txt");
    }

    vector<int> initialLocations;
    // All 1s when capacitated
    if (CAPACITATED) {
        initialLocations = vector<int>(data.locations, 1);
    } else {
        initialLocations = vector<int>(data.locations, 0);
        initialLocations[0] = 1;
    }

    LPP sub = initializeSub(initialLocations);
    if (!silent) {
        cout << "Subproblem initialized.\n";
        sub.saveProblemInfo("first_sub.txt");
    }

    master_solutions = 0;
    // total_time not initialized because it's not computed incrementally
    pricing_time = 0;

    double upper_bound = numeric_limits<double>::max();
    double lower_bound = numeric_limits<double>::lowest();

    int cycle = 0;

    while (true) {
        if (!silent) {
            cout << "=============================\n";
            cout << "Begin cycle " << cycle << "\n";
            cout << "=============================\n";
        }

        if (!silent) {
            cout << "About to solve subproblem for cycle " << cycle << "\n";
            string path = "cycle" + std::to_string(cycle) + "_sub.txt";
            sub.saveProblemInfo(path);
        }

        sub.simplex();
        upper_bound = sub.objective();
        vector<double> dual = sub.dualVars(); // u = (v, w)

        if (!silent) {
            cout << "UB: " << upper_bound << "\n";
            cout << "Sub primal vars:\n";
            vector<double> primal = sub.primalVars();
            prettyPrintVector(primal, 10);
            cout << "Sub dual vars:\n";
            prettyPrintVector(dual, 10);
        }

        // v_j - one for each customer
        vector<double> demand_dual(dual.begin(),
                dual.begin() + data.customers);

        // w_ij - one for each location-customer pair
        vector<double> setup_dual(dual.begin() + data.customers,
                dual.begin() + data.customers +
                data.customers * data.locations);

        // New constraint
        double constant_term = 0;
        for (double vj : demand_dual) {
            constant_term += vj;
        }

        vector<double> constraint_row{1}; // 1 * z
        for (int i = 0; i < data.locations; i++) {
            double y_coef = data.build_costs[i];
            for (int j = 0; j < data.customers; j++) {
                y_coef += setup_dual[i * data.customers + j];
            }
            y_coef *= -1; // bring it to the other side of the inequality
                          // z >= c + a_1y_1 + a_2y_2 + ... + a_iy_i
                          // z - a_1y_1 - a_2y_2 - ... - a_iy_i >= c
            constraint_row.push_back(y_coef);
        }

        if (!silent) {
            cout << "New master constraint:\n";
            cout << "Constant " << constant_term << ";\n";
            prettyPrintVector(constraint_row, 10);
        }


        master.addRow(LPBounds::lower, constant_term, constant_term);
        master.addConstrRow(constraint_row);


        if (!silent) {
            cout << "About to solve master for cycle " << cycle << "\n";
            string path = "cycle" + std::to_string(cycle) + "_master.txt";
            master.saveProblemInfo(path);
        }

        master.simplex();

        lower_bound = master.objective();
        if (!silent) {
            cout << "LB: " << lower_bound << "\n";
        }

        if (upper_bound - lower_bound <= PRECISION) {
            clock_t end = std::clock();
            total_time = double(end - begin) / CLOCKS_PER_SEC;

            cout << "DONE!\n";
            printSolution(master, sub, debug);
            return;
        }

        vector<double> primal = master.primalVars();
        vector<int> built_locations(primal.begin() + 1, primal.end());

        if (!silent) {
            cout << "Built: ";
            prettyPrintVector(built_locations, 10);
        }

        updateSubLocations(sub, built_locations);

        cycle++;

        /* vector<double> best_costs; // v_j */
        /* for (int j = 0; j < data.customers; j++) { */
        /*     double best = data.ship_costs[0][j]; */
        /*     for (int i = 1; i < data.locations; i++) { */
        /*         if (data.ship_costs[i][j] < best) { */
        /*             best = data.ship_costs[i][j]; */
        /*         } */
        /*     } */
        /*     best_costs.push_back(best); */
        /* } */
    }
}

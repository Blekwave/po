#include "FLP.h"
#include "FLPData.h"
#include "utility.h"

#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include <cmath>
#include <ctime>
#include <cassert>

using std::cout;
using std::string;
using std::vector;
using std::abs;
using std::numeric_limits;

const double PRECISION = 0.001;

const bool CAPACITATED = true;

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

    LPP sub(ObjDir::max);
    sub.setConstantTerm(totalBuildCost(built_locations));

    // Initialize v_j columns: one per customer, all ones in objective
    for (int j = 0; j < data.customers; j++) {
        sub.addCol(1, LPBounds::lower, 0, 0);
    }

    // Initialize w_ij columns: one per pair
    // -1 in objective if i has been built, else 0
    for (int i = 0; i < data.locations; i++) {
        for (int j = 0; j < data.customers; j++) {
            sub.addCol(built_locations[i] * -1, LPBounds::lower, 0, 0);
        }
    }

    if (CAPACITATED) {
        // Initialize alpha_i columns: one per location
        // value equals supply for that i
        for (int i = 0; i < data.locations; i++) {
            sub.addCol(data.supplies[i], LPBounds::lower, 0, 0);
        }

        // Initialize beta column
        // value equals the sum of all demands minus the supply for every
        // built location
        double beta_obj = 0;
        for (double demand : data.demands) {
            beta_obj -= demand;
        }
        for (int i = 0; i < data.locations; i++) {
            if (built_locations[i]) {
                beta_obj += data.supplies[i];
            }
        }
        sub.addCol(beta_obj, LPBounds::lower, 0, 0);
    }

    // n * m constraints, for each c
    // v_i - w_ij <= cij (not capacitated)
    // v_i - w_ij + alpha_i < cij (capacitated)
    for (int i = 0; i < data.locations; i++) {
        for (int j = 0; j < data.customers; j++) {
            double cost = data.ship_costs[i][j];
            if (CAPACITATED) {
                cost *= data.demands[j];
            }

            sub.addRow(LPBounds::upper, cost, cost);

            // v[m], w[n][m]
            int columns = (data.locations + 1) * data.customers;
            if (CAPACITATED) {
                columns += data.locations + 1; // alpha[n] + beta[1]
            }

            vector<double> row(columns, 0);
            row[j] = 1;
            row[data.customers * (i + 1) + j] = -1;
            if (CAPACITATED) {
                row[data.customers * (data.locations + 1) + i] = 1;
            }
            sub.addConstrRow(row);
        }
    }

    return sub;
}


LPP FLP::initializeRayGen(vector<int> built_locations, double upper_bound) {
    LPP ray(ObjDir::max);

    // Initialize v_j columns: one per customer, all ones in objective
    for (int j = 0; j < data.customers; j++) {
        ray.addCol(1, LPBounds::double_bound, 0, upper_bound);
    }

    // Initialize w_ij columns: one per pair
    // -1 in objective if i has been built, else 0
    for (int i = 0; i < data.locations; i++) {
        for (int j = 0; j < data.customers; j++) {
            ray.addCol(built_locations[i] * -1, LPBounds::double_bound,
                    0, upper_bound);
        }
    }

    if (CAPACITATED) {
        // Initialize alpha_i columns: one per location
        // value equals supply for that i
        for (int i = 0; i < data.locations; i++) {
            ray.addCol(data.supplies[i], LPBounds::double_bound, 0,
                    upper_bound);
        }

        // Initialize beta column
        // value equals the sum of all demands minus the supply for every
        // built location
        double beta_obj = 0;
        for (double demand : data.demands) {
            beta_obj -= demand;
        }
        for (int i = 0; i < data.locations; i++) {
            if (built_locations[i]) {
                beta_obj += data.supplies[i];
            }
        }
        ray.addCol(beta_obj, LPBounds::double_bound, 0, upper_bound);
    }

    // n * m constraints, for each c
    // v_i - w_ij <= cost (not capacitated)
    // v_i - w_ij + alpha_i <= cost (capacitated)
    for (int i = 0; i < data.locations; i++) {
        for (int j = 0; j < data.customers; j++) {
            double cost = 0;
            ray.addRow(LPBounds::upper, cost, cost);

            // v[m], w[n][m]
            int columns = (data.locations + 1) * data.customers;
            if (CAPACITATED) {
                columns += data.locations + 1; // alpha[n] + beta[1]
            }

            vector<double> row(columns, 0);
            row[j] = 1;
            row[data.customers * (i + 1) + j] = -1;
            if (CAPACITATED) {
                row[data.customers * (data.locations + 1) + i] = 1;
            }
            ray.addConstrRow(row);
        }
    }

    return ray;
}

vector<double> FLP::constraintFromSub(LPP &sub, bool extreme_ray) {
    vector<double> sub_vars = sub.primalVars();

    // w_ij - one for each location-customer pair
    vector<double> setup_vars(sub_vars.begin() + data.customers,
            sub_vars.begin() + data.customers * (data.locations + 1));

    vector<double> constraint_row{extreme_ray ? 0. : 1.}; // z

    double supply_demand_var;
    if (CAPACITATED){
        supply_demand_var = sub_vars.back(); // beta
    }

    for (int i = 0; i < data.locations; i++) {
        double coef = data.build_costs[i];
        for (int j = 0; j < data.customers; j++) {
            coef -= setup_vars[i * data.customers + j];
        }

        if (CAPACITATED) {
            coef += data.supplies[i] * supply_demand_var;
        }

        // -coef is added because it goes to the other side of the inequality
        constraint_row.push_back(-coef);
    }

    return constraint_row;
}

double FLP::constraintConstantFromSub(LPP &sub) {
    vector<double> sub_vars = sub.primalVars();

    // v_j - one for each customer
    vector<double> demand_vars(sub_vars.begin(),
            sub_vars.begin() + data.customers);

    vector<double> supply_vars; // alpha[n]
    double supply_demand_var; // beta
    if (CAPACITATED) {
        auto begin = sub_vars.begin() +
            data.customers * (data.locations + 1);
        auto end = begin + data.locations;
        supply_vars = vector<double>(begin, end);
        supply_demand_var = sub_vars.back();
    }

    double sum = 0;
    for (double demand : demand_vars) {
        sum += demand;
    }

    if (CAPACITATED) {
        for (int i = 0; i < data.locations; i++) {
            sum += data.supplies[i] * supply_vars[i];
        }

        double demand_sum = 0;
        for (double demand : data.demands) {
            demand_sum += demand;
        }
        sum += (-1) * demand_sum * supply_demand_var;
    }

    return sum;
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

    LPP master = initializeMaster();

    if (!silent) {
        cout << "Master problem initialized.\n";
        master.saveProblemInfo("first_master.txt");
    }

    vector<int> initial_locations;
    // All 1s when capacitated
    if (CAPACITATED) {
        initial_locations = vector<int>(data.locations, 1);
    } else {
        initial_locations = vector<int>(data.locations, 0);
        initial_locations[0] = 1;
    }


    master_solutions = 0;
    // total_time not initialized because it's not computed incrementally
    pricing_time = 0;

    double upper_bound = numeric_limits<double>::max();
    double lower_bound = numeric_limits<double>::lowest();

    vector<double> constraint_constants;
    vector<vector<double>> constraint_rows;

    int cycle = 0;
    vector<int> built = initial_locations;

    while (true) {
        if (!silent) {
            cout << "=============================\n";
            cout << "Begin cycle " << cycle << "\n";
            cout << "=============================\n";
        }

        LPP sub = initializeSub(built);
        if (!silent) {
            cout << "About to solve subproblem for cycle " << cycle << "\n";
            string path = "cycle" + std::to_string(cycle) + "_sub.txt";
            sub.saveProblemInfo(path);
        }

        sub.simplex();
        vector<double> sub_vars = sub.primalVars(); // u = (v, w)
        vector<double> x_vals = sub.dualVars();

        if (!silent) {
            cout << "Sub vars:\n";
            prettyPrintVector(sub_vars, 10);
            cout << "X vals:\n";
            prettyPrintVector(x_vals, 10);
        }

        vector<double> constraint_row;
        double constant_term;

        if (!sub.unboundedPrimal()) {
            upper_bound = sub.objective();
            if (!silent) {
                cout << "Sub is bounded, UB: " << upper_bound << "\n";
            }
            constraint_row = constraintFromSub(sub);
            constant_term = constraintConstantFromSub(sub);
        } else {
            if (!silent) {
                cout << "Sub is unbounded, solving extreme ray problem\n";
            }
            LPP ray = initializeRayGen(built, 10000000);
            ray.simplex();

            if (!silent) {
                cout << "Objective: " << ray.objective() << "\n";
                vector<double> ray_primal = ray.primalVars();
                cout << "Ray primal vars: ";
                prettyPrintVector(ray_primal, 10);
                vector<double> ray_dual = ray.dualVars();
                cout << "Ray dual vars: ";
                prettyPrintVector(ray_dual, 10);
                string path = "cycle" + std::to_string(cycle) + "_ray.txt";
                ray.saveProblemInfo(path);
            }

            constraint_row = constraintFromSub(ray, true);
            constant_term = 0;
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

        master.integer();
        vector<double> primal = master.intPrimalVars();

        lower_bound = master.intObjective();
        if (!silent) {
            cout << "LB: " << lower_bound << "\n";
            cout << "Primal sol: ";
            prettyPrintVector(primal, 10);
        }

        if (upper_bound - lower_bound <= PRECISION) {
            cout << "DONE!\n";
            /* printSolution(master, sub, debug); */
            return;
        }

        vector<int> built_locations(primal.begin() + 1, primal.end());

        if (!silent) {
            cout << "Built: ";
            prettyPrintVector(built_locations, 10);
        }

        built = built_locations;

        cycle++;
    }
}

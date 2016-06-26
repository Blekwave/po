#include <iostream>
#include <vector>
#include <glpk.h>

#include "../src/LPP.h"

using std::vector;
using std::cout;

void printOut(LPP &lpp) {
    cout << "Objective: " << lpp.objective() << "\n";

    cout << "Primal solution:\n";

    for (auto var : lpp.primalVars()) {
        cout << var << "\n";
    }

    cout << "Dual solution:\n";

    for (auto var : lpp.dualVars()) {
        cout << var << "\n";
    }
}

void masterProblem() {
    int locations = 3;
    int customers = 5;

    LPP master(ObjDir::min);

    master.addCol(1, LPBounds::free, 0, 0); // z_0 (objective)

    for (int i = 0; i < locations; i++) {
        master.addBinaryCol(0); // y_i
    }

    // y_0 + y_1 + ... + y_i >= 1
    master.addRow(LPBounds::lower, 1, 1);

    vector<double> row{0}; // 0*z_0

    for (int i = 0; i < locations; i++) {
        row.push_back(1); // 1 for each y_i
    }

    master.addConstrRow(row);

    master.simplex();
    printOut(master);

    master.addRow(LPBounds::lower, 21, 21);
    master.addConstrRow(vector<double>{1, -2, 4, 7});

    master.simplex();
    printOut(master);

    master.saveProblemInfo("master_output.txt");
}

typedef vector<double> vi;

void subProblem() {
    int locations = 3;
    int customers = 5;

    vector<vector<double>> ship_costs{
        vi{2, 3, 4, 5, 7},
        vi{4, 3, 1, 2, 6},
        vi{5, 4, 2, 1, 3}
    };

    vector<double> build_costs{2, 3, 3};

    vector<int> y{1, 0, 0};

    double constant = 0;
    for (int i = 0; i < locations; i++){
        constant += build_costs[i] * y[i];
    }

    LPP sub(ObjDir::min);
    sub.setConstantTerm(constant);

    for (int i = 0; i < locations; i++) {
        for (int j = 0; j < customers; j++) {
            sub.addCol(ship_costs[i][j], LPBounds::lower, 0, 0);
        }
    }

    for (int j = 0; j < customers; j++) {
        sub.addRow(LPBounds::lower, 1, 1);
        vector<double> row(locations * customers, 0);
        for (int i = 0; i < locations; i++) {
            row[i * customers + j] = 1;
        }
        sub.addConstrRow(row);
    }

    for (int i = 0; i < locations; i++) {
        for (int j = 0; j < customers; j++) {
            sub.addRow(LPBounds::upper, y[i], y[i]);
            vector<double> row(locations * customers, 0);
            row[i * customers + j] = 1;
            sub.addConstrRow(row);
        }
    }


    sub.simplex();
    printOut(sub);

    vector<int> built_locations{0, 1, 1};

    sub.setConstantTerm(6);

    for (int i = 0; i < locations; i++) {
        for (int j = 0; j < customers; j++) {
            int row = 5 + j + i * customers;
            int built = built_locations[i];
            sub.setRowBounds(row, LPBounds::upper, built, built);
        }
    }

    sub.simplex();
    printOut(sub);

    sub.saveProblemInfo("sub_output.txt");
}

int main() {
    masterProblem();
    subProblem();
    return 0;
}

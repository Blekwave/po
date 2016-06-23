#include "../csp/LPP.h"

#include <iostream>
#include <vector>

using namespace std;

int main() {
    LPP lpp(ObjDir::min);
    lpp.addRow(LPBounds::fixed, 7, 7); // p
    lpp.addRow(LPBounds::fixed, 3, 3); // q
    lpp.addCol(1, LPBounds::lower, 0, 0); // x1
    lpp.addCol(1, LPBounds::lower, 0, 0); // x2
    lpp.addCol(1, LPBounds::lower, 0, 0); // x3

    /* vector<vector<double>> m{ */
    /*     vector<double>{4, 0}, */
    /*     vector<double>{0, 7} */
    /* }; */

    /* lpp.loadMatrix(m); */

    lpp.addConstrCol(vector<int>{4, 0});
    lpp.addConstrCol(vector<int>{0, 7});
    lpp.addConstrCol(vector<int>{2, 4});

    lpp.simplex();

    double obj = lpp.objective();
    cout << "Objective: " << obj << "\n";

    vector<double> primal, dual;

    primal = lpp.primalVars();
    cout << "Primal solution: \n";
    for (double x : primal) {
        cout << x << "\n";
    }

    dual = lpp.dualVars();
    cout << "Dual solution: \n";
    for (double y : dual) {
        cout << y << "\n";
    }

    return 0;
}

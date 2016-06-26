#pragma once

#include <glpk.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

enum class ObjDir : int {
    min = GLP_MIN,
    max = GLP_MAX
};

enum class LPBounds : int {
    free = GLP_FR,
    lower = GLP_LO,
    upper = GLP_UP,
    double_bound = GLP_DB,
    fixed = GLP_FX
};

class LPP {
    glp_prob *lp;
    int rows, cols;
    int constr_rows, constr_cols;

    public:
        LPP(ObjDir d);
        ~LPP();

        void setConstantTerm(double value);

        void addRow(LPBounds bounds, double from, double to);
        void addCol(double obj, LPBounds bounds, double from, double to);
        void addBinaryCol(double obj);

        void setRowBounds(int row, LPBounds bounds, double from, double to);

        void addConstrCol(vector<double> col);
        void addConstrRow(vector<double> col);
        void loadMatrix(vector<vector<double>> m);

        void simplex();

        double objective();
        vector<double> primalVars();
        vector<double> dualVars();

        void integer();
        double intObjective();
        vector<double> intPrimalVars();

        void saveProblemInfo(const string path);

        static void termOut(bool silent);
};


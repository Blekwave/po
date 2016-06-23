#include "LPP.h"

#include <glpk.h>
#include <cassert>
#include <vector>

using std::vector;

LPP::LPP(ObjDir d)
    : rows(0), cols(0), constr_rows(0), constr_cols(0) {
    lp = glp_create_prob();
    glp_set_obj_dir(lp, static_cast<int>(d));
}

LPP::~LPP() {
    glp_delete_prob(lp);
    // no glp_free_env, there may be other LPP objects
}

void LPP::addRow(LPBounds bounds, double from, double to) {
    glp_add_rows(lp, 1);
    rows++;
    glp_set_row_bnds(lp, rows, static_cast<int>(bounds), from, to);
}

void LPP::addCol(double obj, LPBounds bounds, double from, double to) {
    glp_add_cols(lp, 1);
    cols++;
    glp_set_col_bnds(lp, cols, static_cast<int>(bounds), from, to);
    glp_set_obj_coef(lp, cols, obj);
}

void LPP::addConstrCol(vector<int> col) {
    if (constr_rows == 0) {
        constr_rows = col.size();
    }
    assert(static_cast<int>(col.size()) == constr_rows);

    int len = constr_rows;
    int array_len = len + 1;

    int *indices = new int[array_len];
    double *coef = new double[array_len];

    for (int i = 0; i < len; i++) {
        indices[i + 1] = i + 1;
        coef[i + 1] = col[i];
    }

    constr_cols++;
    glp_set_mat_col(lp, constr_cols, constr_rows, indices, coef);

    delete[] indices;
}

void LPP::loadMatrix(vector<vector<double>> m) {
    int matrix_len = rows * cols;
    // GLPK works with 1-based indexing
    int serial_array_len = matrix_len + 1;

    // Arrays to which the matrix and its indices will be serialized.
    // (this is necessary due to GLPK's input format)
    double *coef = new double[serial_array_len];
    int *row_indices = new int[serial_array_len];
    int *col_indices = new int[serial_array_len];

    int p = 1; // Index for the arrays above, 1-based

    assert(static_cast<int>(m.size()) == rows);
    for (int i = 0; i < rows; i++) {
        assert(static_cast<int>(m[i].size()) == cols);

        for (int j = 0; j < cols; j++) {
            coef[p] = m[i][j];
            row_indices[p] = i + 1;
            col_indices[p] = j + 1;
            p++;
        }
    }

    glp_load_matrix(lp, matrix_len, row_indices, col_indices, coef);

    constr_rows = rows;
    constr_cols = cols;

    delete[] coef;
    delete[] row_indices;
    delete[] col_indices;
}

void LPP::simplex() {
    glp_simplex(lp, NULL);
}

double LPP::objective() {
    return glp_get_obj_val(lp);
}

vector<double> LPP::primalVars() {
    vector<double> out;
    for (int j = 1; j <= cols; j++) { // 1-based
        out.push_back(glp_get_col_prim(lp, j));
    }
    return out;
}

vector<double> LPP::dualVars() {
    vector<double> out;
    for (int i = 1; i <= rows; i++) { // 1-based
        out.push_back(glp_get_row_dual(lp, i));
    }
    return out;

}

void LPP::termOut(bool silent) {
    glp_term_out(silent ? GLP_OFF : GLP_ON);
}

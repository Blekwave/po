#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#include "ProblemData.h"
#include "CSP.h"

using std::strcmp;
using std::string;
using std::vector;
using std::cout;


ProblemData readProblemData(std::ifstream &file) {
    ProblemData pd;
    file >> pd.stock_width >> pd.cuts;

    for (int i = 0; i < pd.cuts; i++){
        int width, demand;
        file >> width >> demand;
        pd.widths.push_back(width);
        pd.demands.push_back(demand);
    }

    return pd;
}



void singleProblem(const char *path, bool debug) {
    if (debug) {
        string title = "Input file: ";
        title += string(path);
        cout << title << "\n";
    }

    std::ifstream file(path);
    ProblemData pd = readProblemData(file);

    if (debug) {
        cout << "Read!\n";
    }

    string filename(std::strrchr(path, '/') + 1);

    CSP csp(filename, pd);
    csp.solve(debug);
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        singleProblem(argv[1], false);
    } else if (argc == 3) {
        singleProblem(argv[1], true);
    } else {
        cout << "ERROR: Bad input format\n"
            << "Format: ./csp <input_file> [debug]\n";
    }
    return 0;
}

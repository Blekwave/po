#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#include "FLPData.h"
#include "FLP.h"

using std::strcmp;
using std::string;
using std::vector;
using std::cout;

FLPData readProblemData(std::ifstream &file) {
    FLPData data;

    file >> data.locations >> data.customers;

    for (int i = 0; i < data.locations; i++) {
        int supply;
        double build_cost;
        file >> supply >> build_cost;
        data.supplies.push_back(supply);
        data.build_costs.push_back(build_cost);
    }

    data.ship_costs = vector<vector<double>>(data.locations, vector<double>{});

    // For consistency, i always indexes locations, and j, customers.
    for (int j = 0; j < data.customers; j++) {
        int demand;
        file >> demand;
        data.demands.push_back(demand);

        for (int i = 0; i < data.locations; i++) {
            double ship_cost;
            file >> ship_cost;
            data.ship_costs[i].push_back(ship_cost);
        }
    }

    return data;
}

void singleProblem(const char *path, bool debug) {
    if (debug) {
        string title = "Input file: ";
        title += string(path);
        cout << title << "\n";
    }

    std::ifstream file(path);
    FLPData data = readProblemData(file);

    if (debug) {
        cout << "Read!\n";
    }

    string filename(std::strrchr(path, '/') + 1);

    FLP flp(filename, data);
    if (debug) {
        flp.printProblemData();
    }
    flp.solve(debug);
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        singleProblem(argv[1], false);
    } else if (argc == 3) {
        singleProblem(argv[1], true);
    } else {
        cout << "ERROR: Bad input format\n"
            << "Format: ./flp <input_file> [debug]\n";
    }
    return 0;
}

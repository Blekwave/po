#pragma once

#include <vector>

using std::vector;

struct ProblemData {
    int stock_width;
    int cuts;
    vector<int> widths;
    vector<int> demands;
};

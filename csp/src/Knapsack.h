#pragma once

#include <vector>

using std::vector;

class Knapsack {
    vector<int> m_solution_counts;
    double m_solution;

    public:
        Knapsack(int cap, const vector<int> &weights, const vector<double> &values);
        vector<int> solution_counts();
        double solution();
};

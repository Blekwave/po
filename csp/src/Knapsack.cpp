#include "Knapsack.h"

#include <vector>
#include <cassert>

using std::vector;

Knapsack::Knapsack(int cap, const vector<int> &weights,
        const vector<double> &values) {
    assert(weights.size() == values.size());
    int items = weights.size();

    vector<double> partial_max{0};
    vector<int> initial_count(items, 0);
    vector<vector<int>> counts{initial_count};

    for (int w = 1; w <= cap; w++) {
        double local_max = partial_max[w - 1];
        int max_item = -1;
        for (int i = 0; i < items; i++) {
            if (weights[i] <= w) {
                double candidate = partial_max[w - weights[i]] + values[i];
                if (candidate > local_max) {
                    local_max = candidate;
                    max_item = i;
                }
            }
        }

        partial_max.push_back(local_max);

        if (max_item == -1) {
            counts.push_back(counts[w - 1]);
        } else {
            vector<int> new_counts = counts[w - weights[max_item]];
            new_counts[max_item]++;
            counts.push_back(new_counts);
        }
    }

    m_solution_counts = counts.back();
    m_solution = partial_max.back();
}

vector<int> Knapsack::solution_counts() {
    return m_solution_counts;
}

double Knapsack::solution() {
    return m_solution;
}

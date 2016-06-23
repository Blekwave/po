#include "../csp/Knapsack.h"

#include <iostream>
#include <vector>

using std::vector;
using std::cout;

int main() {
    vector<double> values{(double)1/4, (double)1/7};
    vector<int> weights{5, 3};
    int cap = 22;

    Knapsack k(cap, weights, values);

    cout << "Solution: " << k.solution() << "\n";

    cout << "Counts:\n";
    for (int i : k.solution_counts()) {
        cout << i << "\n";
    }

    return 0;
}

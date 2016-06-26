#pragma once

#include <iostream>
#include <vector>

using std::vector;
using std::cout;

template<typename T>
void prettyPrintVector(vector<T> &v, int per_line){
    int limit = static_cast<int>(v.size());
    for (int i = 0; i < limit; i++){
        cout << v[i];
        if (i % per_line == per_line - 1 or i == limit - 1) {
            cout << "\n";
        } else {
            cout << "; ";
        }
    }
}

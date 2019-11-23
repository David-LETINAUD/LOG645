#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>
#include <stdio.h>

#include "sequential.hpp"

using std::memcpy;
using std::min;

using std::this_thread::sleep_for;
using std::chrono::microseconds;

double calculate(double c, double l, double r, double t, double b, double td, double h_square) {
    return c * (1.0 - 4.0 * td / h_square) + (t + b + l + r) * (td / h_square);
}

void solveSeq(int rows, int cols, int iterations, double td, double h, double ** matrix) {
    double h_sq = h * h;

    double * prev = new double[cols];
    double * curr = new double[cols];

    for(int k = 0; k < iterations; k++) {

        memcpy(prev, matrix[0], cols * sizeof(double));
        for(int i = 1; i < rows - 1; i++) {

            memcpy(curr, matrix[i], cols * sizeof(double));
            for(int j = 1; j < cols - 1; j++) {
                matrix[i][j] = calculate(curr[j], curr[j - 1], curr[j + 1], prev[j], matrix[i + 1][j], td, h_sq);
            }

            memcpy(prev, curr, cols * sizeof(double));
        }
    }
}

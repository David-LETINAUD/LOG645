#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "matrix/matrix.hpp"
#include "output/output.hpp"
#include "solver/sequential.hpp"
#include "solver/parallel.hpp"

void usage();
void command(int argc, char* argv[]);

void initial(int rows, int cols, double ** matrix);
long sequential(int rows, int cols, int iters, double td, double h, double ** matrix);
long parallel(int rows, int cols, int iters, double td, double h, double ** matrix, const char * kernelFileName);

using namespace std::chrono;

using std::cout;
using std::endl;
using std::flush;
using std::setprecision;
using std::setw;
using std::stod;
using std::stoi;

int main(int argc, char* argv[]) {
    // Arguments.
    int rows;
    int cols;
    int iters;
    int minDisplayRow;
    int maxDisplayRow;
    int minDisplayCol;
    int maxDisplayCol;
    double td;
    double h;
    char * kernelFileName;

    // Timing variables.
    long runtime_seq = 0;
    long runtime_par = 0;

    if(11 != argc) {
        usage();
        return EXIT_FAILURE;
    }

    rows = stoi(argv[1], nullptr, 10);
    cols = stoi(argv[2], nullptr, 10);
    iters = stoi(argv[3], nullptr, 10);
    td = stod(argv[4], nullptr);
    h = stod(argv[5], nullptr);
    minDisplayRow = stoi(argv[6], nullptr, 10);
    maxDisplayRow = stoi(argv[7], nullptr, 10);
    minDisplayCol = stoi(argv[8], nullptr, 10);
    maxDisplayCol = stoi(argv[9], nullptr, 10);
    kernelFileName = argv[10];

    command(argc, argv);
    double ** matrix = allocateMatrix(rows, cols);

    cout << "-----  INITIAL   -----" << endl << flush;
    initial(rows, cols, matrix);
    printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);

    cout << "----- SEQUENTIAL -----" << endl << flush;
    runtime_seq = sequential(rows, cols, iters, td, h, matrix);
    printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);

    cout << "-----  PARALLEL  -----" << endl << flush;
    runtime_par = parallel(rows, cols, iters, td, h, matrix, kernelFileName);
    printMatrixPartial(minDisplayRow, maxDisplayRow, minDisplayCol, maxDisplayCol, matrix);

    printStatistics(runtime_seq, runtime_par);

    deallocateMatrix(rows, matrix);

    return EXIT_SUCCESS;
}

void usage() {
    cout << "Invalid arguments." << endl << flush;
    cout << "Arguments: m n np td h minDisplayRow maxDisplayRow minDisplayCol maxDisplayCol kernelFileName" << endl << flush;
}

void command(int argc, char* argv[]) {
    cout << "Command:" << flush;

    for(int i = 0; i < argc; i++) {
        cout << " " << argv[i] << flush;
    }

    cout << endl << flush;
}

void initial(int rows, int cols, double ** matrix) {
    fillMatrix(rows, cols, matrix);
}

long sequential(int rows, int cols, int iters, double td, double h, double ** matrix) {
    fillMatrix(rows, cols, matrix);

    time_point<high_resolution_clock> timepoint_s = high_resolution_clock::now();
    solveSeq(rows, cols, iters, td, h, matrix);
    time_point<high_resolution_clock> timepoint_e = high_resolution_clock::now();

    return duration_cast<microseconds>(timepoint_e - timepoint_s).count();
}

long parallel(int rows, int cols, int iters, double td, double h, double ** matrix, const char * kernelFileName) {
    fillMatrix(rows, cols, matrix);

    time_point<high_resolution_clock> timepoint_s = high_resolution_clock::now();
    solvePar(rows, cols, iters, td, h, matrix, kernelFileName);
    time_point<high_resolution_clock> timepoint_e = high_resolution_clock::now();

    return duration_cast<microseconds>(timepoint_e - timepoint_s).count();
}

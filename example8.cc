#include <iostream>
#include <vector>
#include <ctime>
#include <memory>

#include"matrix.h"
#include "matrix_sum.h"
#include "matrix_product.h"

template<typename T>
void initializeCells(matrix<T> &m) {
    for (unsigned row = 0; row < m.get_height(); ++row) {
        for (unsigned col = 0; col < m.get_width(); ++col) {
            m(row, col) = 1;
        }
    }
}

template<typename T, unsigned h, unsigned w>
void initializeCells(matrix<T, h, w> &m) {
    for (unsigned row = 0; row < m.get_height(); ++row) {
        for (unsigned col = 0; col < m.get_width(); ++col) {
            m(row, col) = 1;
        }
    }
}
template<typename T>
void print_matrix(matrix<T> &m) {
    for (int i = 0; i != m.get_height(); ++i) {
        for (int j = 0; j != m.get_width(); ++j)
            std::cout << m(i, j) << ' ';
        std::cout << '\n';
    }
    std::cout << std::endl;
}

template<typename T, unsigned h, unsigned w>
void print_matrix(matrix<T, h, w> &m) {
    for (int i = 0; i != h; ++i) {
        for (int j = 0; j != w; ++j)
            std::cout << m(i, j) << ' ';
        std::cout << '\n';
    }
    std::cout << std::endl;
}


int main() {
    matrix<int, 15000, 15000> mA;
    initializeCells(mA);
    matrix<int, 15000, 15000> mB;
    initializeCells(mB);
    matrix<int, 15000, 15000> mC;
    initializeCells(mC);
    matrix<int, 15000, 15000> mD;
    initializeCells(mD);


    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    matrix<int, 15000, 15000> multiplication = mA + mB + mC + mD;
    int first = multiplication(345, 456);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "The first element is " << first << ", and it took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.0 << " seconds"
              << std::endl;

    std::cout << "A * B" << std::endl;
    std::cout << "Oks" << std::endl;

    return 0;
}
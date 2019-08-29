//
// Created by Filippo Maganza on 2019-08-29.
//

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

int main() {
    matrix<int, 3000, 3000> mA;
    initializeCells(mA);
    matrix<int, 3000, 3000> mB;
    initializeCells(mB);


    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    matrix<int, 3000, 3000> multiplication = mA * mB;
    int first = multiplication(0, 0);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "The first element is " << first << ", and it took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.0 << " seconds"
              << std::endl;

    std::cout << "A * B" << std::endl;
    std::cout << "Oks" << std::endl;

    return 0;
}
//
// Created by Gianmarco Callegher on 2019-07-22.
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

template<typename T>
void initializeCellsRand(matrix<T> &m) {
    for (unsigned row = 0; row < m.get_height(); ++row) {
        for (unsigned col = 0; col < m.get_width(); ++col) {
            m(row, col) = (int) std::rand() * 100;
        }
    }
}

template<typename T, unsigned h, unsigned w>
void initializeCellsRand(matrix<T, h, w> &m) {
    for (unsigned row = 0; row < m.get_height(); ++row) {
        for (unsigned col = 0; col < m.get_width(); ++col) {
            m(row, col) = (int) std::rand() * 100;
        }
    }
}

int main() {
    int i = 0;
    matrix<int, 10, 10> mA;
    initializeCells(mA);
    matrix<int, 10, 10> mB;
    initializeCells(mB);
    matrix<int, 10, 10> mC;
    initializeCells(mC);

    matrix<int, 10, 10> res1 = mC + mB;
    matrix<int, 10, 10> res2 = mC + mB + mA;
    matrix<int, 10, 10> res3 = mC + (mB + mA);
    matrix<int, 10, 10> res4 = (mC + mB) + (mC + mB);

    matrix<int> m1(10, 10);
    initializeCells(m1);
    matrix<int> m2(10, 10);
    initializeCells(m2);
    matrix<int> m3(10, 10);
    initializeCells(m3);

    matrix<int> res5 = m1 + m2;
    matrix<int> res6 = m1 + m2 + m3;
    matrix<int> res7 = m1 + (m1 + m2);
    matrix<int> res8 = (m1 + m1) + (m1 + m1);

    matrix<int> res9 = mA + m1;
    matrix<int> res11 = m1 + (mA + m2);

    matrix<float> m5(10, 10);

    matrix<float> res12 = m5 + m1;


}
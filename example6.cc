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
            m(row, col) = (int) std::rand() * 1000;
        }
    }
}

template<typename T, unsigned h, unsigned w>
void initializeCellsRand(matrix<T, h, w> &m) {
    for (unsigned row = 0; row < m.get_height(); ++row) {
        for (unsigned col = 0; col < m.get_width(); ++col) {
            m(row, col) = (int) std::rand() * 1000;
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

    matrix<int, 100, 100> mA;
    initializeCells(mA);
    matrix<int, 100, 100> mB;
    initializeCells(mB);
    matrix<int, 100, 100> mC;
    initializeCells(mC);
    matrix<int, 100, 100> mD;
    initializeCells(mD);
    matrix<int> m1(100, 100);
    initializeCells(m1);
    matrix<int> m2(100, 100);
    initializeCells(m2);
    matrix<int> m3(100, 100);
    initializeCells(m3);
    /**
     * Static matrix_sum
     */
    matrix<int, 100, 100> res1 = mC + mB;
    print_matrix(res1);
    matrix<int, 100, 100> res2 = mC + mB + mA;
    print_matrix(res2);
    matrix<int, 100, 100> res3 = mC + (mB + mA);
    print_matrix(res3);
    matrix<int, 100, 100> res4 = (mC + mB) + (mC + mB);
    print_matrix(res4);
    /**
    * Dynamic matrix_sum
    */
    matrix<int> res5 = m1 + m2;
    print_matrix(res5);
    matrix<int> res6 = m1 + m2 + m3;
    print_matrix(res6);
    matrix<int> res7 = m1 + (m1 + m2);
    print_matrix(res7);
    matrix<int> res8 = (m1 + m1) + (m1 + m1);
    print_matrix(res8);
    /**
    * Dynamic e static matrix_sum
    */
    matrix<int> res9 = mA + m1;
    print_matrix(res9);
    matrix<int> res10 = m1 + (mA + m2);
    print_matrix(res10);

    /**
     * Static matrix_product
     */
    matrix<int> res11 = (mA * mD) + mA;
    print_matrix(res11);

    matrix<int> res12 = mA + (mA * mD);
    print_matrix(res12);

    matrix<int> res13 = m1 + (mA * mD);
    print_matrix(res13);

    matrix<int> res14 = (m1 * mD) + m1;
    print_matrix(res14);

    matrix<int> res15 = (m1 + m1) * (mA + mA);
    print_matrix(res15);

    matrix<int> res16 = (m1 + m1) * (mA * mA);
    print_matrix(res16);

    matrix<int, 100, 20> mE;
    initializeCells(mE);

    matrix<int, 100, 20> mF;
    initializeCells(mE);

    matrix<int, 20, 200> mG;
    initializeCells(mG);

    matrix<int, 20, 200> mH;
    initializeCells(mH);

    matrix<int, 200, 40> mI;
    initializeCells(mI);

    matrix<int, 200, 40> mJ;
    initializeCells(mJ);

    matrix<int> res17 = (mI.transpose() + mJ.transpose()) * (mG.transpose() * mH);
    print_matrix(res17);

}
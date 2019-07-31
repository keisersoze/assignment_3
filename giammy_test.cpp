//
// Created by Gianmarco Callegher on 2019-07-22.
//

#include <iostream>
#include <vector>
#include <ctime>
#include <memory>

#include"matrix.h"
#include"matrix_sum.h"

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
            m(row, col) = (int) std::rand()*100;
        }
    }
}
template<typename T, unsigned h, unsigned w>
void initializeCellsRand(matrix<T, h, w> &m) {
    for (unsigned row = 0; row < m.get_height(); ++row) {
        for (unsigned col = 0; col < m.get_width(); ++col) {
            m(row, col) = (int) std::rand()*100;
        }
    }
}

int main() {
    int i =0 ;
    matrix<int, 10, 13> mA;
    initializeCells(mA);
    matrix<int, 10, 13> mB;
    initializeCells(mB);
    matrix<int, 10, 13> mC;
    initializeCellsRand(mC);

    matrix<int, 10, 13> res1 = mC + mB;
    matrix<int, 10, 13> res2 = mC + mB + mA;
    matrix<int, 10, 13> res3 = (mC + mB) + mA.transpose().transpose();
    matrix<int, 10, 13> res4 = mC + (mB + mA);

    matrix<int> m1(10,13);
    initializeCells(m1);
    matrix<int> m2(10,13);
    initializeCells(m2);
    matrix<int> m3(10,13);
    initializeCells(m3);
    matrix<int> res11 = m1 + m2;
    matrix<int> res22 = m1 + m2 + m1;
    matrix<int> res33 = (m1 + m2) + m1;
    matrix<int> res44 = m1 + (m2 + m1);

    //matrix<int> XD = result + result;

}
//
// Created by Gianmarco Callegher on 2019-07-22.
//

#include <iostream>
#include <vector>
#include <ctime>
#include <memory>
#include <thread>

#include"matrix.h"
#include"matrix_wrap.h"
#include"matrix_operation.h"
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
    std::cout << 5 << std::endl;
    matrix<int, 10, 13> mA;
    initializeCells(mA);
    matrix<int, 10, 13> mB;
    initializeCells(mB);
    matrix<int, 10, 13> mC;
    initializeCellsRand(mC);

    matrix<int, 10, 13> result = mC + mB;
    //matrix<int> XD = result + result;

}
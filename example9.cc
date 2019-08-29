//
// Created by Filippo Maganza on 2019-08-29.
//


#include <iostream>
#include <vector>
#include <ctime>
#include <memory>

#include"matrix.h"

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
    mB = mA;
    int i;

}
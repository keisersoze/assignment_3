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

int main() {
    matrix<int> mA(10, 13);
    initializeCells(mA);
    matrix<int> mB(10, 13);
    initializeCells(mB);

    matrix<int> result = mA + mB;
}
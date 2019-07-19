//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_OPERATION_H
#define ASSIGNMENT_3_MATRIX_OPERATION_H

#include "matrix_wrap.h"

template<typename T, unsigned h, unsigned w>
class matrix_operation {
    virtual operator matrix<T>() = 0;

    virtual operator matrix<T, h, w>() = 0;
};

template<typename T, unsigned h, unsigned w>
class matrix_singleton : matrix_operation<T, h, w> {
    matrix_singleton(matrix_wrap<T> &m) {
        this->singleton = m;
    }

    operator matrix<T>() {
        return this->singleton;
    }

    operator matrix<T, h, w>() {
        return this->singleton;
    }


private:
    matrix_wrap<T> singleton;
};


#endif //ASSIGNMENT_3_MATRIX_OPERATION_H

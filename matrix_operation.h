//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_OPERATION_H
#define ASSIGNMENT_3_MATRIX_OPERATION_H

#include "matrix_wrap.h"

template<typename T, unsigned H, unsigned W>
class matrix_operation {
    virtual matrix<T> resolve_all() = 0;

    virtual unsigned get_height() = 0;

    virtual unsigned get_width() = 0;
};

template<typename T>
class matrix_operation<T, 0, 0> {
    virtual matrix<T> resolve_all() = 0;

    virtual unsigned get_height() = 0;

    virtual unsigned get_width() = 0;
};

template<typename T>
class matrix_singleton : matrix_operation<T, 0, 0> {
    matrix_singleton(matrix<T> &m) {
        this->singleton = m;
    }

    matrix<T> resolve_all() {
        return this->singleton;
    }

    virtual unsigned get_height() {
        return this->singleton.get_height();
    }

    virtual unsigned get_width() {
        return this->singleton.get_width();
    }


private:
    matrix<T> singleton;
};


#endif //ASSIGNMENT_3_MATRIX_OPERATION_H

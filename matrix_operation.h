//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_OPERATION_H
#define ASSIGNMENT_3_MATRIX_OPERATION_H

#include "matrix_wrap.h"


template<typename T>
class matrix_operation {
    virtual matrix<T> resolve_all() = 0;

    virtual unsigned get_height() = 0;

    virtual unsigned get_width() = 0;
};

template<typename T, unsigned h, unsigned w>
class matrix_operation_s : matrix_operation<T> {
    static constexpr unsigned H = h;
    static constexpr unsigned W = w;

    virtual matrix<T> resolve_all() = 0;

    virtual unsigned get_height() = 0;

    virtual unsigned get_width() = 0;
};


template<typename T>
class matrix_singleton : matrix_operation<T> {
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

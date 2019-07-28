//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_OPERATION_H
#define ASSIGNMENT_3_MATRIX_OPERATION_H

#include "matrix_wrap.h"

template<typename T>
class matrix_operation {
public:
    virtual matrix_wrap<T> resolve_all() = 0;

    //virtual unsigned get_height() = 0;

    //virtual unsigned get_width() = 0;
};

template<typename T, unsigned h, unsigned w>
class matrix_operation_s : matrix_operation<T> {
    static constexpr unsigned H = h;
    static constexpr unsigned W = w;


    //virtual unsigned get_height() = 0;

    //virtual unsigned get_width() = 0;
};


template<typename T>
class matrix_singleton : public matrix_operation<T> {
public:
    template<class matrix_type>
    matrix_singleton(const matrix_ref<T, matrix_type> &m): singleton(m){};

    matrix_wrap<T> resolve_all() {
        return this->singleton;
    }

    virtual unsigned get_height() {
        return this->singleton.get_height();
    }

    virtual unsigned get_width() {
        return this->singleton.get_width();
    }

private:
    matrix_wrap<T> singleton;
};


#endif //ASSIGNMENT_3_MATRIX_OPERATION_H
//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_OPERATION_H
#define ASSIGNMENT_3_MATRIX_OPERATION_H

#include "matrix_wrap.h"

template<typename T>
class matrix_operation {
public:
    virtual matrix<T> resolve_all() const = 0;

    virtual unsigned get_height() const = 0 ;

    virtual unsigned get_width() const = 0;
};

template<typename T>
class matrix_singleton : public matrix_operation<T> {
public:
    template<class matrix_type>
    matrix_singleton(const matrix_ref<T, matrix_type> &m): singleton(m){};

    matrix<T> resolve_all() const {
        return this->singleton;
    }

    unsigned get_height() const {
        return this->singleton.get_height();
    }

    unsigned get_width() const {
        return this->singleton.get_width();
    }

private:
    matrix<T> singleton;
};


#endif //ASSIGNMENT_3_MATRIX_OPERATION_H

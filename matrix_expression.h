//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_EXPRESSION_H
#define ASSIGNMENT_3_MATRIX_EXPRESSION_H

#include "matrix_wrap.h"

template<typename T>
class matrix_expression {
public:
    virtual matrix_wrap<T> resolve_all() = 0;

    virtual unsigned get_height() const = 0 ;

    virtual unsigned get_width() const = 0;

    virtual ~matrix_expression() {};

};

template<typename T>
class matrix_singleton : public matrix_expression<T> {
public:
    template<class matrix_type>
    matrix_singleton(matrix_ref<T, matrix_type> m): singleton(m){};

    matrix_wrap<T> resolve_all() {
        return this->singleton;
    }

    unsigned get_height() const {
        return this->singleton.get_height();
    }

    unsigned get_width() const {
        return this->singleton.get_width();
    }

    ~matrix_singleton() = default;

private:
    matrix_wrap<T> singleton;
};


#endif //ASSIGNMENT_3_MATRIX_EXPRESSION_H

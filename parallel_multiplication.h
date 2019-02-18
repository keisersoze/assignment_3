//
// Created by Filippo Maganza on 2019-02-09.
//

#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include"matrix.h"
#include"matrix_wrap.h"
#include"matrix_fwd.h"
#include <thread>

#define BLOCK_DIM 10

template<T, LType, Rtype>
void row_multiply(window_spec window_spec1, const matrix_ref<T, LType> &lhs, const matrix_ref<T, Rtype> &rhs) {
    for (int i = 0; i < lhs.get_width();i += BLOCK_DIM) {
        std::thread t(row_multiply({i, i + BLOCK_DIM, 0, lhs.get_width}, lhs, rhs))
    }
    // join threads
}

template<T, LType, Rtype>
void real_multiplication(window_spec window_spec1, const matrix_ref<T, LType> &lhs, const matrix_ref<T, Rtype> &rhs) {
    const unsigned height = result.get_height();
    const unsigned width = result.get_width();
    const unsigned span = lhs.get_width();
    assert(span == rhs.get_height());
    for (unsigned i = 0; i != height; ++i)
        for (unsigned j = 0; j != width; ++j) {
            result(i, j) = 0;
            for (unsigned k = 0; k != span; ++k)
                result(i, j) += lhs(i, k) * rhs(k, j);
        }

}


template<T, LType, Rtype>
void do_multiply(const matrix_ref<T, LType> &lhs, const matrix_ref<T, Rtype> &rhs) {

    for (int i = 0; i < lhs.get_height(); i += BLOCK_DIM) {
        window_spec w();
        std::thread t(row_multiply({i, i + BLOCK_DIM, 0, lhs.get_width}, lhs, rhs))
    }

    // join threads

}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

//
// Created by Filippo Maganza on 2019-02-09.
//

#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include"matrix.h"
#include"matrix_wrap.h"
#include"matrix_fwd.h"
#include <thread>


template<typename T, class LType, class RType>
void real_multiplication(matrix_wrap<T> &result, int row_start, int col_start,
        const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    int l_real_block_height = BLOCK_DIM, l_real_block_width = BLOCK_DIM;
    int r_real_block_height = BLOCK_DIM, r_real_block_width = BLOCK_DIM;

    if (row_start + BLOCK_DIM > lhs.get_height()) {
        l_real_block_height = lhs.get_height() - row_start;
    }
    if (col_start + BLOCK_DIM > lhs.get_width()) {
        l_real_block_width = lhs.get_width() - col_start;
    }

    if (row_start + BLOCK_DIM > rhs.get_height()) {
        r_real_block_height = rhs.get_height() - col_start;
    }
    if (col_start + BLOCK_DIM > rhs.get_width()) {
        r_real_block_width = rhs.get_width() - row_start;
    }


    for (unsigned i = window_spec.row_start; i != window_spec.row_end; ++i) {
        for (unsigned j = window_spec.col_start; j != window_spec.col_end; ++j) {
            for (unsigned k = 0; k != span; ++k)
                result(i, j) += lhs(i, k) * rhs(k, j);
        }
    }
}

template<typename T>
void row_multiply(matrix_wrap<T> &result, int row_start, const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    std::vector<std::thread> v;
    int i;

    for (i = 0; i < lhs.get_width(); i += BLOCK_DIM) {
        std::thread t(real_multiplication(result, row_start, i, lhs, rhs));
        v.push_back(t);
    }

    for (int j = 0; j < v.size(); ++j) {
        v[j].join();
    }
}


template<typename T>
void do_multiply(matrix_wrap<T> result, matrix_wrap<T> lhs, matrix_wrap<T> rhs) {
    const unsigned height = result.get_height();
    const unsigned width = result.get_width();
    const unsigned span = lhs.get_width();

    assert(span == rhs.get_height());

    std::vector<std::thread> v;
    int i;

    for (i = 0; i != height; ++i) {
        for (unsigned j = 0; j != width; ++j) {
            result(i, j) = 0;
        }
    }

    for (i = 0; i < lhs.get_height(); i += BLOCK_DIM) {
        std::thread t(row_multiply(result, i, lhs, rhs));
        v.push_back(t);
    }

    // join threads
    for (int j = 0; j < v.size(); ++j) {
        v[j].join();
    }
}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

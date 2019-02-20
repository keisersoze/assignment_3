//
// Created by Filippo Maganza on 2019-02-09.
//

#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include"matrix.h"
#include"matrix_wrap.h"
#include"matrix_fwd.h"
#include <thread>

#define BLOCK_DIM 2

template<typename T, class LType, class RType>
void real_multiplication(matrix_wrap<T> &result, window_spec window_spec, const matrix_ref<T, LType> &lhs, const matrix_ref<T, RType> &rhs) {
    int span = window_spec.col_end - window_spec.col_start;
    for (unsigned i = window_spec.row_start; i != window_spec.row_end; ++i) {
        for (unsigned j = window_spec.col_start; j != window_spec.col_end; ++j) {
            for (unsigned k = 0; k != span; ++k)
                result(i, j) += lhs(i, k) * rhs(k, j);
        }
    }

}

template<typename T, class LType, class RType>
void row_multiply(matrix_wrap<T> &result, int row_start, int row_end, const matrix_ref<T, LType> &lhs, const matrix_ref<T, RType> &rhs) {
    std::vector<std::thread> v;
    for (int i = 0; i < lhs.get_width(); i += BLOCK_DIM) {
        std::thread t(real_multiplication(result, {row_start, row_end, i, i + BLOCK_DIM}, lhs, rhs));
        v.push_back(t);
    }
    for (int j = 0; j < v.size(); ++j) {
        v[j].join();
    }
}


template<typename T, class LType, class RType>
void do_multiply(matrix_wrap<T> result, matrix_wrap<T> lhs, matrix_wrap<T> rhs) {
    const unsigned height = result.get_height();
    const unsigned width = result.get_width();
    const unsigned span = lhs.get_width();
    assert(span == rhs.get_height());
    auto unwrap_lhs = lhs.get_mat();
    auto unwrap_rhs = rhs.get_mat();
    std::vector<std::thread> v;
    for (unsigned i = 0; i != height; ++i) {
        for (unsigned j = 0; j != width; ++j) {
            result(i, j) = 0;
        }
    }
    for (int i = 0; i < lhs.get_height(); i += BLOCK_DIM) {
        std::thread t(row_multiply(i, i + BLOCK_DIM, unwrap_lhs, unwrap_rhs));
        v.push_back(t);
    }

    for (int j = 0; j < v.size(); ++j) {
        v[j].join();
    }
    // join threads

}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

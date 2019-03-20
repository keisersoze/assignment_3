#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include "matrix.h"
#include "matrix_wrap.h"
#include "matrix_fwd.h"
#include <thread>
#include <iostream>

template<typename T>
void real_multiplication(const matrix_wrap<T> &result, window_spec result_window_spec, const matrix<T> &lhs, const matrix<T> &rhs) {
    int height = result_window_spec.row_end - result_window_spec.row_start + 1;
    int width = result_window_spec.col_end - result_window_spec.col_start + 1;
    int span = lhs.get_height();
    auto sub_result = result.get_submatrix(result_window_spec);
    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            for (unsigned k = 0; k != span; ++k)
                sub_result(i, j) += lhs(i, k) * rhs(k, j);
        }
    }
    std::cout <<"Go finio " << "result:" << result_window_spec.row_start << "," << result_window_spec.row_end << "," << result_window_spec.col_start << "," << result_window_spec.col_end <<  std::endl;
}

template<typename T>
void thread_multiplication(const matrix_wrap<T> &result, window_spec l_window_spec, const matrix_wrap<T> &lhs,
                           const matrix_wrap<T> &rhs) {
    unsigned int row_start_rhs = l_window_spec.col_start;
    std::vector<std::thread> v;
    matrix<T> lhs_sub = lhs.get_submatrix(l_window_spec);

    window_spec r_window_spec = {0, 0, 0, 0};
    r_window_spec.row_start = row_start_rhs;

    if (row_start_rhs + BLOCK_DIM < rhs.get_height() - 1) {
        r_window_spec.row_end = row_start_rhs + BLOCK_DIM - 1;
    } else {
        r_window_spec.row_end = rhs.get_height() - 1;
    }

    for (unsigned int i = 0; i < rhs.get_width(); i += BLOCK_DIM) {
        window_spec result_window_spec = {0,0,0,0};
        r_window_spec.col_start = i;

        if (i + BLOCK_DIM < rhs.get_height() - 1) {
            r_window_spec.col_end = i + BLOCK_DIM - 1;
        } else {
            r_window_spec.col_end = rhs.get_width() - 1;
        }

        result_window_spec.row_start = l_window_spec.row_start;
        result_window_spec.col_start = r_window_spec.col_start;
        if (result_window_spec.row_start + BLOCK_DIM < result.get_height() - 1) {
            result_window_spec.row_end = result_window_spec.row_start + BLOCK_DIM - 1;
        } else {
            result_window_spec.row_end = result.get_height() - 1;
        }

        if (result_window_spec.col_start + BLOCK_DIM < result.get_width() - 1) {
            result_window_spec.col_end = result_window_spec.col_start + BLOCK_DIM - 1;
        } else {
            result_window_spec.col_end = result.get_width() - 1;
        }
        std::thread t(real_multiplication<T>, result, result_window_spec, lhs_sub, rhs.get_submatrix(r_window_spec));
        v.push_back(std::move(t));
    }

    for (auto &j : v) {
        j.join();
    }

}

template<typename T>
void row_multiply(const matrix_wrap<T> &result, unsigned int row_start, const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    window_spec l_wlindow_spec = {0,0,0,0};
    l_wlindow_spec.row_start = row_start;

    if (row_start + BLOCK_DIM < lhs.get_height() - 1) {
        l_wlindow_spec.row_end = row_start + BLOCK_DIM - 1;
    } else {
        l_wlindow_spec.row_end = lhs.get_height() - 1;
    }

    for (unsigned int i = 0; i < lhs.get_width(); i += BLOCK_DIM) {
        l_wlindow_spec.col_start = i;

        //set window_spec col_end
        if (i + BLOCK_DIM < lhs.get_width() - 1) {
            l_wlindow_spec.col_end = i + BLOCK_DIM - 1;

        } else {
            l_wlindow_spec.col_end = lhs.get_width() - 1;
        }
        thread_multiplication(result, l_wlindow_spec, lhs, rhs);
    }
}


template<typename T>
void do_multiply(matrix_wrap<T> &result, matrix_wrap<T> lhs, matrix_wrap<T> rhs) {
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
        std::thread t(row_multiply<T>, result, i, lhs, rhs);
        v.push_back(std::move(t));
    }

    // join threads
    for (auto &j : v) {
        j.join();
    }
}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

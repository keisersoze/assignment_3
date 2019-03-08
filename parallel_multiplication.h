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
void real_multiplication(matrix_wrap<T> &result, window_spec result_window_spec, matrix<T> &l, matrix<T> &r) {
    int l_real_block_height = BLOCK_DIM, l_real_block_width = BLOCK_DIM;
    int r_real_block_height = BLOCK_DIM, r_real_block_width = BLOCK_DIM;


    /* for (unsigned i = window_spec.row_start; i != window_spec.row_end; ++i) {
        for (unsigned j = window_spec.col_start; j != window_spec.col_end; ++j) {
            for (unsigned k = 0; k != span; ++k)
                result(i, j) += lhs(i, k) * rhs(k, j);
        }
    } */
}

//TODO find a name for this function
template<typename T>
void function(matrix_wrap<T> &result, int row_start_rhs, matrix<T> lhs_sub, const matrix_wrap<T> &rhs) {
    std::vector<std::thread> v;

    window_spec r_wlindow_spec;
    r_wlindow_spec.row_start = row_start_rhs;

    if (row_start_rhs + BLOCK_DIM < rhs.get_height()) {
        r_wlindow_spec.row_end = row_start_rhs + BLOCK_DIM;
    } else {
        r_wlindow_spec.row_end = rhs.get_height() - 1;
    }

    for (int i = 0; i < rhs.get_width(); i += BLOCK_DIM) {
        window_spec result_window_spec;
        r_wlindow_spec.col_start = i;

        if (i + BLOCK_DIM < rhs.get_height()) {
            r_wlindow_spec.col_end = i + BLOCK_DIM;
        } else {
            r_wlindow_spec.col_end = rhs.get_width() - 1;
        }

        
        std::thread t(real_multiplication(result, , lhs_sub, rhs.get_submatrix(r_wlindow_spec)));
    }
}

template<typename T>
void row_multiply(matrix_wrap<T> &result, int row_start, const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    window_spec l_wlindow_spec;

    l_wlindow_spec.row_start = row_start;

    if (row_start + BLOCK_DIM < lhs.get_height()) {
        l_wlindow_spec.row_end = row_start + BLOCK_DIM;
    } else {
        l_wlindow_spec.row_end = lhs.get_height() - 1;
    }

    for (int i = 0; i < lhs.get_width(); i += BLOCK_DIM) {
        l_wlindow_spec.col_start = i;

        //set window_spec col_end
        if (i + BLOCK_DIM < lhs.get_width()) {
            l_wlindow_spec.col_end = i + BLOCK_DIM;

        } else {
            l_wlindow_spec.col_end = lhs.get_width() - 1;
        }
        function(result, lhs.get_submatrix(l_wlindow_spec), rhs);
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

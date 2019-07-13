#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include "matrix.h"
#include "matrix_wrap.h"
#include "matrix_fwd.h"
#include <thread>
#include <iostream>

template<typename T>
void real_multiplication(matrix_wrap<T> result, window_spec result_window_spec, const matrix<T> &lhs,
                         const matrix<T> &rhs) {
    int height = result_window_spec.row_end - result_window_spec.row_start + 1;
    int width = result_window_spec.col_end - result_window_spec.col_start + 1;
    int span = lhs.get_height();
    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j) {
            for (unsigned k = 0; k != span; ++k) {
                result(i + result_window_spec.row_start, j + result_window_spec.col_start) += lhs(i, k) * rhs(k, j);
            }
        }
    }
}

template<typename T>
void thread_multiplication(matrix_wrap<T> result, window_spec l_window_spec, const matrix_wrap<T> &lhs,
                           const matrix_wrap<T> &rhs) {
    unsigned int row_start_rhs = l_window_spec.col_start;
    std::vector<std::thread> v;
    matrix<T> lhs_sub = lhs.get_submatrix(l_window_spec);
    const unsigned int max_thread = std::thread::hardware_concurrency();

    window_spec r_window_spec = {0, 0, 0, 0};
    r_window_spec.row_start = row_start_rhs;

    if (row_start_rhs + BLOCK_DIM < rhs.get_height() - 1) {
        r_window_spec.row_end = row_start_rhs + BLOCK_DIM - 1;
    } else {
        r_window_spec.row_end = rhs.get_height() - 1;
    }
    unsigned int allocated_threads = 0;
    for (unsigned int i = 0; i < rhs.get_width(); i += BLOCK_DIM) {
        window_spec result_window_spec = {0, 0, 0, 0};
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
//        allocated_threads += 1;
//        if (allocated_threads == max_thread) {
//            // join threads
//            for (auto &j : v) {
//                j.join();
//            }
//            v.clear();
//            allocated_threads = 0;
//        }

    }
    for (auto &j : v) {
        j.join();
    }
}

/**
 * Multiply a row of blocks of lhs with a column of blocks of rhs
 * @tparam T
 * @param result the matrix where the result of the multiplication are saved
 * @param row_start the start index of the row of blocks
 * @param lhs the left matrix
 * @param rhs the right matrix
 */
template<typename T>
void
compute_resulting_row(matrix_wrap<T> result,
                      unsigned int row_start,
                      const matrix_wrap<T> &lhs,
                      const matrix_wrap<T> &rhs) {
    window_spec l_wlindow_spec = {0, 0, 0, 0};
    l_wlindow_spec.row_start = row_start;
    std::vector<std::thread> v;
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
        thread_multiplication<T>(result, l_wlindow_spec, lhs, rhs);
    }
}



template<typename T>
void do_multiply(matrix_wrap<T> result, matrix_wrap<T> lhs, matrix_wrap<T> rhs) {
    const unsigned height = result.get_height();
    const unsigned width = result.get_width();

    assert(lhs.get_width() == rhs.get_height());

    std::vector<std::thread> v;
    int i;

    // Initialize the result matrix with all 0s
    for (i = 0; i != height; ++i) {
        for (unsigned j = 0; j != width; ++j) {
            result(i, j) = 0;
        }
    }

    for (i = 0; i < lhs.get_height(); i += BLOCK_DIM) {
        compute_resulting_row<T>(result, i, lhs, rhs);
    }

}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

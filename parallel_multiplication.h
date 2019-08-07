#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include "matrix.h"
#include "matrix_wrap.h"
#include "matrix_fwd.h"
#include "thread_pool.h"
#include <thread>
#include <iostream>


#define BLOCK_DIM 64

template<typename T>
void block_product_block(matrix_wrap<T> result, window_spec lhs_window, window_spec rhs_window,
                         const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {

    assert(lhs_window.col_end - lhs_window.col_start == rhs_window.row_end - rhs_window.row_start);

    window_spec result_window = {0, 0, 0, 0};
    result_window.row_start = lhs_window.row_start;
    result_window.row_end = lhs_window.row_end;
    result_window.col_start = rhs_window.col_start;
    result_window.col_end = rhs_window.col_end;

    unsigned res_height = result_window.row_end - result_window.row_start + 1;
    unsigned res_width = result_window.col_end - result_window.col_start + 1;
    unsigned span = lhs_window.col_end - lhs_window.col_start + 1;

    //Only here we load real data
    matrix<T> lhs_sub = lhs.get_submatrix(lhs_window);
    matrix<T> rhs_sub = rhs.get_submatrix(rhs_window);

    for (unsigned i = 0; i < res_height; ++i) {
        for (unsigned j = 0; j < res_width; ++j) {
            for (unsigned k = 0; k < span; ++k) {
                result(result_window.row_start + i, result_window.col_start + j) += lhs_sub(i, k) * rhs_sub(k, j);
            }
        }
    }
}


template<typename T>
void blockrow_product_blockcolumn(matrix_wrap<T> result, window_spec lhs_window, window_spec rhs_window,
                                  const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {

    assert (lhs_window.col_start == 0 && rhs_window.row_start == 0 && lhs_window.col_end == lhs.get_width() - 1 &&
            rhs_window.row_end == rhs.get_height() - 1);

    unsigned i = BLOCK_DIM - 1;
    for (; i < lhs_window.col_end; i += BLOCK_DIM) {
        window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i - (BLOCK_DIM - 1), i};
        window_spec rhs_window_matrix = {i - (BLOCK_DIM - 1), i, rhs_window.col_start, rhs_window.col_end};
        block_product_block(result, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
    }

    window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i - (BLOCK_DIM - 1), lhs_window.col_end};
    window_spec rhs_window_matrix = {i - (BLOCK_DIM - 1), rhs_window.row_end, rhs_window.col_start, rhs_window.col_end};
    block_product_block(result, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
}

template<typename T>
matrix_wrap<T> do_multiply(const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    matrix<T> result(lhs.get_height(), rhs.get_width());
    const unsigned height = result.get_height();
    const unsigned width = result.get_width();
    const unsigned span = lhs.get_width();
    assert(span == rhs.get_height());
    std::vector<std::future<void>> futures;
    for (unsigned i = 0; i != height; ++i) {
        for (unsigned j = 0; j != width; ++j) {
            result(i, j) = 0;
        }
    }
    matrix_wrap<T> result_wrap(result);
    unsigned i = BLOCK_DIM - 1;
    for (; i < lhs.get_height() - 1; i += BLOCK_DIM) {
        window_spec lhs_window = {i - (BLOCK_DIM - 1), i, 0, lhs.get_width() - 1};
        unsigned j = BLOCK_DIM - 1;
        for (; j < rhs.get_width() - 1; j += BLOCK_DIM) {
            window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), j};
            futures.push_back(
                    ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_wrap, lhs_window,
                                                       rhs_window,
                                                       lhs, rhs));
        }
        window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), rhs.get_width() - 1};
        futures.push_back(
                ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_wrap, lhs_window, rhs_window,
                                                   lhs, rhs));
    }
    window_spec lhs_window = {i - (BLOCK_DIM - 1), lhs.get_height() - 1, 0, lhs.get_width() - 1};
    unsigned j = BLOCK_DIM - 1;
    for (; j < rhs.get_width() - 1; j += BLOCK_DIM) {
        window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), j};
        futures.push_back(
                ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_wrap, lhs_window, rhs_window,
                                                   lhs, rhs));
    }
    window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), rhs.get_width() - 1};
    futures.push_back(
            ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_wrap, lhs_window, rhs_window,
                                               lhs, rhs));

    for (unsigned long i = 0; i < futures.size(); ++i) {
        futures[i].get();
    }
    return result_wrap;
}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

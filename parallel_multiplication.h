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
void block_product_block(std::shared_ptr<matrix<T>> result_ptr, window_spec lhs_window, window_spec rhs_window,
                         const matrix<T> &lhs, const matrix<T> &rhs) {

    assert(lhs_window.col_end - lhs_window.col_start == rhs_window.row_end - rhs_window.row_start);

    for (unsigned i = lhs_window.row_start; i <= lhs_window.row_end ; ++i) {
        for (unsigned j = rhs_window.col_start; j <= rhs_window.col_end; ++j) {
            for (unsigned k = lhs_window.col_start; k <= lhs_window.col_end; ++k) {
                (*result_ptr)(i, j) += lhs(i, k) * rhs(k, j);
            }
        }
    }
}


template<typename T>
void blockrow_product_blockcolumn(std::shared_ptr<matrix<T>> result_ptr, window_spec lhs_window, window_spec rhs_window,
                                  const matrix<T> &lhs, const matrix<T> &rhs) {

    assert (lhs_window.col_start == 0 && rhs_window.row_start == 0 && lhs_window.col_end == lhs.get_width() - 1 &&
            rhs_window.row_end == rhs.get_height() - 1);

    unsigned i = BLOCK_DIM - 1;
    for (; i < lhs_window.col_end; i += BLOCK_DIM) {
        window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i - (BLOCK_DIM - 1), i};
        window_spec rhs_window_matrix = {i - (BLOCK_DIM - 1), i, rhs_window.col_start, rhs_window.col_end};
        block_product_block(result_ptr, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
    }

    window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i - (BLOCK_DIM - 1), lhs_window.col_end};
    window_spec rhs_window_matrix = {i - (BLOCK_DIM - 1), rhs_window.row_end, rhs_window.col_start, rhs_window.col_end};
    block_product_block(result_ptr, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
}

template<typename T>
matrix_wrap<T> do_multiply(const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    std::shared_ptr<matrix<T>> result_ptr(new matrix<T>(lhs.get_height(), rhs.get_width()));
    const unsigned height = result_ptr->get_height();
    const unsigned width = result_ptr->get_width();
    const unsigned span = lhs.get_width();
    assert(span == rhs.get_height());
    std::vector<std::future<void>> futures;
    for (unsigned i = 0; i != height; ++i) {
        for (unsigned j = 0; j != width; ++j) {
            (*result_ptr)(i, j) = 0;
        }
    }
    matrix<T> lhs_2 = lhs.get_submatrix({0, lhs.get_height() - 1, 0, lhs.get_width() - 1});
    matrix<T> rhs_2 = rhs.get_submatrix({0, rhs.get_height() - 1, 0, rhs.get_width() - 1});
    unsigned i = BLOCK_DIM - 1;
    for (; i < lhs.get_height() - 1; i += BLOCK_DIM) {
        window_spec lhs_window = {i - (BLOCK_DIM - 1), i, 0, lhs.get_width() - 1};
        unsigned j = BLOCK_DIM - 1;
        for (; j < rhs.get_width() - 1; j += BLOCK_DIM) {
            window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), j};
            futures.push_back(
                    ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_ptr, lhs_window,
                                                       rhs_window,
                                                       lhs_2, rhs_2));
        }
        window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), rhs.get_width() - 1};
        futures.push_back(
                ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_ptr, lhs_window, rhs_window,
                                                   lhs_2, rhs_2));
    }
    window_spec lhs_window = {i - (BLOCK_DIM - 1), lhs.get_height() - 1, 0, lhs.get_width() - 1};
    unsigned j = BLOCK_DIM - 1;
    for (; j < rhs.get_width() - 1; j += BLOCK_DIM) {
        window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), j};
        futures.push_back(
                ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_ptr, lhs_window, rhs_window,
                                                   lhs_2, rhs_2));
    }
    window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), rhs.get_width() - 1};
    futures.push_back(
            ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result_ptr, lhs_window, rhs_window,
                                               lhs_2, rhs_2));

    for (unsigned long i = 0; i < futures.size(); ++i) {
        futures[i].get();
    }

    matrix_wrap<T> result_wrap(*result_ptr);
    return result_wrap;
}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

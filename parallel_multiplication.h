#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include <iostream>
#include "matrix.h"
#include "matrix_wrap.h"
#include "matrix_fwd.h"
#include "thread_pool.h"
#include <thread>
#include <iostream>

/*
template<typename T>
void row_product_column(matrix_wrap<T> result, unsigned row_index, unsigned  column_index, const matrix_wrap<T> &lhs,
                     const matrix_wrap<T> &rhs) {


 unsigned int row_start_rhs = l_window_spec.col_start;
 matrix<T> lhs_sub = lhs.get_submatrix(l_window_spec);
 window_spec r_window_spec = {0, 0, 0, 0};
 r_window_spec.row_start = row_start_rhs;
 std::vector<std::future<void>> futures;
 if (row_start_rhs + BLOCK_DIM < rhs.get_height() - 1) {
     r_window_spec.row_end = row_start_rhs + BLOCK_DIM - 1;
 } else {
     r_window_spec.row_end = rhs.get_height() - 1;
 }
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
     //futures.push_back(ThreadPool::getSingleton().enqueue(real_multiplication<T>, result, result_window_spec, lhs_sub,rhs.get_submatrix(r_window_spec)));
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
 for (unsigned long i = 0; i < futures.size(); ++i) {
     futures[i].get();
 }
  */




/*
template<typename T>
void
row_product_matrix(matrix_wrap<T> result, unsigned int row_start, const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    window_spec l_wlindow_spec = {0, 0, 0, 0};
    l_wlindow_spec.row_start = row_start;
    std::vector<std::future<void>> futures;
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

        futures.push_back(
                ThreadPool::getSingleton().enqueue(row_product_column<T>, result, l_wlindow_spec, lhs, rhs));
    }
    for (unsigned long i = 0; i < futures.size(); ++i) {
        futures[i].get();
    }
}
**/


/*
template<typename T>
void row_product_column(matrix<T> &result, unsigned lhs_row_index, unsigned  rhs_column_index, const matrix<T> &lhs,
                        const matrix<T> &rhs) {
    for (unsigned int j = 0; j < lhs.get_width(); j ++) {
        result += lhs(lhs_row_index,j) * rhs(j,rhs_column_index);
    }
}
 */

#define BLOCK_DIM 32

template<typename T>
void block_product_block(matrix_wrap<T> result, window_spec lhs_window, window_spec rhs_window,
                         const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {

    assert(lhs_window.row_end - lhs_window.row_start == rhs_window.col_end - lhs_window.col_start);

    window_spec result_window = {0, 0, 0, 0};
    result_window.row_start = lhs_window.row_start;
    result_window.row_end = lhs_window.row_end;
    result_window.col_start = rhs_window.col_start;
    result_window.col_end = rhs_window.col_end;

    //Only here we load real data
    matrix<T> lhs_sub = lhs.get_submatrix(lhs_window);
    matrix<T> rhs_sub = lhs.get_submatrix(rhs_window);

    for (unsigned i = 0; i < lhs_sub.get_width(); ++i) {
        for (unsigned j = 0; j < rhs_sub.get_width(); ++j) {
            for (unsigned k = 0; k < lhs_sub.get_height(); ++k) {
                result(result_window.row_start + i, result_window.col_start + j) += lhs_sub(i, k) * rhs_sub(k, j);
            }
        }
    }
}


template<typename T>
void blockrow_product_blockcolumn(matrix_wrap<T> &result, window_spec lhs_window, window_spec rhs_window,
                                  const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {

    assert ( lhs_window.col_start == 0 && rhs_window.row_start == 0 && lhs_window.col_end == lhs.get_width() && rhs_window.row_end == rhs.get_height());
    unsigned i = BLOCK_DIM - 1;
    for (; i < lhs_window.col_end ; i+= BLOCK_DIM){
        window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i - BLOCK_DIM, i};
        window_spec rhs_window_matrix = { i - BLOCK_DIM, i, lhs_window.col_start, lhs_window.col_end};
        block_product_block(result, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
    }
    window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i, lhs_window.col_end};
    window_spec rhs_window_matrix = { i, rhs_window.row_end, lhs_window.col_start, lhs_window.col_end};
    block_product_block(result, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
}

template<typename T>
void blockrow_product_matrix(matrix_wrap<T> &result, window_spec lhs_window, window_spec rhs_window,
                             const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {
    
}


template<typename T>
matrix<T> do_multiply(matrix<T> lhs, matrix<T> rhs) {
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

    for (unsigned i = 0; i < lhs.get_height(); i++) {
        for (unsigned j = 0; j < rhs.get_width(); j++) {
            futures.push_back(
                    ThreadPool::getSingleton().enqueue(blockrow_product_blockcolumn<T>, result, i, j, lhs, rhs));
        }
    }

    for (unsigned long i = 0; i < futures.size(); ++i) {
        futures[i].get();
    }
    return result;
}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

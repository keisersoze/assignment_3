#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include "matrix.h"
#include "matrix_wrap.h"
#include "matrix_fwd.h"
#include "thread_pool.h"
#include <thread>
#include <iostream>
#include <shared_mutex>
#include <map>


#define BLOCK_DIM 90

template<typename T>
class parallel_multiplication {

private:
    typedef std::map<std::string, std::shared_ptr<matrix<T>>> mapT;
    std::mutex mutex;
    mapT matrix_map;

    void block_product_block(matrix<T> &result, window_spec lhs_window, window_spec rhs_window,
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
        std::string s1 = std::to_string(lhs_window.row_start) + std::to_string(lhs_window.row_end)
                         + std::to_string(lhs_window.col_start) + std::to_string(lhs_window.col_end);
        std::string s2 = std::to_string(rhs_window.row_start) + std::to_string(rhs_window.row_end)
                         + std::to_string(rhs_window.col_start) + std::to_string(rhs_window.col_end);

        std::shared_ptr<matrix<T>> lhs_sub_ptr;
        std::shared_ptr<matrix<T>> rhs_sub_ptr;
        {
            std::unique_lock<std::mutex> lock(mutex);
            typename mapT::iterator lhs_it = matrix_map.find(s1);
            typename mapT::iterator rhs_it = matrix_map.find(s2);
            if (lhs_it == matrix_map.end()) {
                lhs_sub_ptr = std::make_shared<matrix<T>>(lhs_window.row_end - lhs_window.row_start, lhs_window.col_end - lhs_window.col_start);
                unsigned int height = lhs_window.row_end - lhs_window.row_start + 1;
                unsigned int width = lhs_window.col_end - lhs_window.col_start + 1;
                for (int i = 0; i < height; i++) {
                    for (int j = 0; j < width; j++) {
                        lhs_sub_ptr->operator()(i, j) = lhs(i + lhs_window.row_start, j + lhs_window.col_start);
                    }
                }
                matrix_map.insert(std::pair<std::string,std::shared_ptr<matrix<T>>>(s1,lhs_sub_ptr));
            } else{
                lhs_sub_ptr = lhs_it->second;
            }
            if (rhs_it == matrix_map.end()) {
                rhs_sub_ptr = std::make_shared<matrix<T>>(rhs_window.row_end - rhs_window.row_start, rhs_window.col_end - rhs_window.col_start);
                unsigned int height = rhs_window.row_end - rhs_window.row_start + 1;
                unsigned int width = rhs_window.col_end - rhs_window.col_start + 1;
                for (int i = 0; i < height; i++) {
                    for (int j = 0; j < width; j++) {
                        rhs_sub_ptr->operator()(i, j) = rhs(i + rhs_window.row_start, j + rhs_window.col_start);
                    }
                }
                matrix_map.insert(std::pair<std::string,std::shared_ptr<matrix<T>>>(s2,rhs_sub_ptr));
            } else{
                rhs_sub_ptr = rhs_it->second;
            }
        }

        for (unsigned i = 0; i < res_height; ++i) {
            for (unsigned j = 0; j < res_width; ++j) {
                for (unsigned k = 0; k < span; ++k) {
                    result(result_window.row_start + i, result_window.col_start + j) +=
                            lhs_sub_ptr->operator()(i, k) * rhs_sub_ptr->operator()(k, j);
                }
            }
        }
    }


    void blockrow_product_blockcolumn(matrix<T> &result, window_spec lhs_window, window_spec rhs_window,
                                      const matrix_wrap<T> &lhs, const matrix_wrap<T> &rhs) {

        assert (lhs_window.col_start == 0 && rhs_window.row_start == 0 && lhs_window.col_end == lhs.get_width() - 1 &&
                rhs_window.row_end == rhs.get_height() - 1);

        unsigned i = BLOCK_DIM - 1;
        for (; i < lhs_window.col_end; i += BLOCK_DIM) {
            window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i - (BLOCK_DIM - 1), i};
            window_spec rhs_window_matrix = {i - (BLOCK_DIM - 1), i, rhs_window.col_start, rhs_window.col_end};
            block_product_block(result, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
        }

        window_spec lhs_window_matrix = {lhs_window.row_start, lhs_window.row_end, i - (BLOCK_DIM - 1),
                                         lhs_window.col_end};
        window_spec rhs_window_matrix = {i - (BLOCK_DIM - 1), rhs_window.row_end, rhs_window.col_start,
                                         rhs_window.col_end};
        block_product_block(result, lhs_window_matrix, rhs_window_matrix, lhs, rhs);
    }


public:

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
        unsigned i = BLOCK_DIM - 1;
        for (; i < lhs.get_height() - 1; i += BLOCK_DIM) {
            window_spec lhs_window = {i - (BLOCK_DIM - 1), i, 0, lhs.get_width() - 1};
            unsigned j = BLOCK_DIM - 1;
            for (; j < rhs.get_width() - 1; j += BLOCK_DIM) {
                window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), j};
                futures.push_back(
                        ThreadPool::getSingleton().enqueue(&parallel_multiplication<T>::blockrow_product_blockcolumn,
                                                           std::ref(*this), std::ref(result), lhs_window,
                                                           rhs_window,
                                                           std::cref(lhs), std::cref(rhs)));
            }
            window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), rhs.get_width() - 1};
            futures.push_back(
                    ThreadPool::getSingleton().enqueue(&parallel_multiplication<T>::blockrow_product_blockcolumn,
                                                       std::ref(*this), std::ref(result), lhs_window, rhs_window,
                                                       std::cref(lhs), std::cref(rhs)));
        }
        window_spec lhs_window = {i - (BLOCK_DIM - 1), lhs.get_height() - 1, 0, lhs.get_width() - 1};
        unsigned j = BLOCK_DIM - 1;
        for (; j < rhs.get_width() - 1; j += BLOCK_DIM) {
            window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), j};
            futures.push_back(
                    ThreadPool::getSingleton().enqueue(&parallel_multiplication<T>::blockrow_product_blockcolumn,
                                                       std::ref(*this), std::ref(result), lhs_window, rhs_window,
                                                       std::cref(lhs), std::cref(rhs)));
        }
        window_spec rhs_window = {0, rhs.get_height() - 1, j - (BLOCK_DIM - 1), rhs.get_width() - 1};
        futures.push_back(
                ThreadPool::getSingleton().enqueue(&parallel_multiplication<T>::blockrow_product_blockcolumn,
                                                   std::ref(*this), std::ref(result), lhs_window, rhs_window,
                                                   std::cref(lhs), std::cref(rhs)));

        for (unsigned long i = 0; i < futures.size(); ++i) {
            futures[i].get();
        }

        matrix_wrap<T> result_wrap(result);
        return result_wrap;
    }

    parallel_multiplication() {};
};


#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

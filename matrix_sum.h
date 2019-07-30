//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_SUM_H
#define ASSIGNMENT_3_MATRIX_SUM_H

#include<type_traits>
#include<list>
#include <thread>
#include "parallel_multiplication.h"
#include "matrix_operation.h"

#include "thread_pool.h"
#include <stdlib.h>
#include <unistd.h>

template<typename T>
matrix<T> sum(const matrix<T> &m1, const matrix<T> &m2){
    unsigned height = m1.get_height();
    unsigned width = m1.get_width();
    matrix<T> result(height, width);
    for (unsigned j = 0; j < height; ++j) {
        for (unsigned k = 0; k < width; ++k) {
            result(j, k) = m1(j, k) + m2(j, k);
        }
    }
    return result;
}

template<typename T, unsigned h, unsigned w>
class matrix_sum : public matrix_operation<T> {
public:

    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_sum;

    matrix<T> resolve_all() {
        std::vector<std::future<matrix<T>>> futures;
        for (auto aux : operations) {
            futures.push_back(ThreadPool::getSingleton().enqueue([aux]() {return aux->resolve_all();}));
        }
        int abracadabra = 0;
        while (futures.size() > 1) {
            std::vector<std::future<matrix<T>>> futures_aux;
            for (unsigned i = 0; i < futures.size() - 1; i += 2) {
                matrix<T> m1 = futures[i].get();
                matrix<T> m2 = futures[i + 1].get();
                futures_aux.push_back(ThreadPool::getSingleton().enqueue(sum<T>, m1, m2));
            }
            if (futures.size() % 2){
                futures_aux.push_back(std::move(futures[futures.size()-1]));
            }
            futures = std::move(futures_aux);
            abracadabra++;
        }
        return futures[0].get();
    }

    operator matrix<T>() {
        return resolve_all();
    }

    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>(){
        static_assert((h == 0 || h == h2) && (w == 0 || w == w2), "sized product conversion to wrong sized matrix");
        return (matrix<T, h2, w2>)resolve_all();
    }

    matrix_sum() = default;

    void add(matrix_operation<T> *mat) {
        operations.push_back(mat);
    }


    matrix_sum(matrix_sum<T, h, w> &&X) : operations(std::move(X.operations)) {}

private:
    std::list<matrix_operation<T> *> operations;
};

/**
 * Static overload
 * @tparam T
 * @tparam U
 * @tparam LType
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return
 */
template<typename T, typename U, class LType, class RType>
std::enable_if_t
        <matrix_ref<T, LType>::H != 0 && matrix_ref<U, RType>::H != 0,
                matrix_sum<decltype(T() + U()), matrix_ref<T, LType>::H, matrix_ref<T, LType>::W>>
operator+(const matrix_ref<T, LType> &lhs, const matrix_ref<U, RType> &rhs) {
    static_assert((matrix_ref<T, LType>::W == matrix_ref<T, RType>::W) &&
                  (matrix_ref<T, LType>::H == matrix_ref<T, RType>::H),
                  "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), matrix_ref<T, LType>::H, matrix_ref<T, LType>::W> result;
    result.add(new matrix_singleton<decltype(T() + U())>(lhs));
    result.add(new matrix_singleton<decltype(T() + U())>(rhs));
    return result;
}

/**
 * Dynamic overload
 * @tparam T
 * @tparam U
 * @tparam LType
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return
 */
template<typename T, typename U, class LType, class RType>
std::enable_if_t
        <matrix_ref<T, LType>::H == 0 || matrix_ref<U, RType>::H == 0,
                matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_ref<T, LType> &lhs, const matrix_ref<U, RType> &rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width())
        throw std::domain_error("dimension mismatch in Matrix addition");
    // 0,0 because we loose the static information
    matrix_sum<decltype(T() + U()), 0, 0> result;
    //TODO matrix wrap without T
    result.add(new matrix_singleton<decltype(T() + U())>(lhs));
    result.add(new matrix_singleton<decltype(T() + U())>(rhs));
    return result;
}

/**
 * Static overload for sum operation between matrix_sum and matrix_ref
 * @tparam T
 * @tparam U
 * @tparam h
 * @tparam w
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t
        <h != 0 && matrix_ref<U, RType>::H != 0,
                matrix_sum<decltype(T() + U()), h, w>>
operator+(matrix_sum<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    static_assert(matrix_ref<T, RType>::W * w == 0 ||
                  (matrix_ref<T, RType>::H == h &&
                   matrix_ref<T, RType>::W == w),
                  "dimension mismatch in Matrix multiplication");

    matrix_sum<T, h, w> result(std::move(lhs));
    result.add(new matrix_singleton(rhs));
    return result;
}

/**
 * Dynamic overload for sum operation between matrix_operation and matrix_ref
 * @tparam T
 * @tparam U
 * @tparam h
 * @tparam w
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t
        <h == 0 || matrix_ref<U, RType>::H == 0,
                matrix_sum<decltype(T() + U()), 0, 0>>
operator+(matrix_sum<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    //if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
    //    throw std::domain_error("dimension mismatch in Matrix addition");
    //}
    matrix_sum<T, h, w> result(std::move(lhs));
    result.add(new matrix_singleton(rhs));
    return result;
}

/**
 * Static overload for sum operation between matrix_ref and matrix_operation
 * @tparam T
 * @tparam U
 * @tparam h
 * @tparam w
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return

template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t
        <h != 0 && matrix_ref<U, RType>::H != 0,
                matrix_sum<decltype(T() + U()), h, w>>
operator+(const matrix_ref<U, RType> &lhs, matrix_operation_s<T, h, w> &rhs) {
    return rhs + lhs;
}
 */

/**
 * Dynamic overload for sum operation between matrix_ref and matrix_operation
 * @tparam T
 * @tparam U
 * @tparam h
 * @tparam w
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return

template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t
        <h == 0 || matrix_ref<U, RType>::H == 0,
                matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_ref<U, RType> &lhs, matrix_operation_s<T, h, w> &rhs) {
    return rhs + lhs;
}*/

/**
 * Static overload for sum operation between matrix_operation_s and matrix_operation_s
 * @tparam T
 * @tparam U
 * @tparam h
 * @tparam w
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return

template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr, class RType>
std::enable_if_t
        <hl != 0 && hr != 0,
                matrix_sum<decltype(T() + U()), hl, wl>>
operator+(const matrix_operation_s<T, hl, wl> &lhs, const matrix_operation_s<U, hr, wr> &rhs) {
    static_assert(hl == hr && wl == wr, "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), hl, wl> result;
    result.add(lhs);
    result.add(rhs);
    return result;

}*/

/**
 * Dynamic overload for sum operation between matrix_ref and matrix_operation
 * @tparam T
 * @tparam U
 * @tparam h
 * @tparam w
 * @tparam RType
 * @param lhs
 * @param rhs
 * @return

template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr, class RType>
std::enable_if_t
        <hl == 0 || hr == 0,
                matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_operation_s<T, hl, wl> &lhs, const matrix_operation_s<U, hr, wr> &rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), 0, 0> result;
    result.add(lhs);
    result.add(rhs);
    return result;
}
 */


#endif //ASSIGNMENT_3_MATRIX_SUM_H


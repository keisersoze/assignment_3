//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_SUM_H
#define ASSIGNMENT_3_MATRIX_SUM_H

#include <type_traits>
#include <list>
#include <thread>
#include "parallel_multiplication.h"
#include "matrix_expression.h"

#include "thread_pool.h"
#include <unistd.h>

template<typename T>
matrix_wrap<T> sum(const matrix_wrap<T> &m1, const matrix_wrap<T> &m2) {
    unsigned height = m1.get_height();
    unsigned width = m1.get_width();
    matrix<T> result(height, width);
    for (unsigned j = 0; j < height; ++j) {
        for (unsigned k = 0; k < width; ++k) {
            result(j, k) = m1(j, k) + m2(j, k);
        }
    }
    return matrix_wrap(result);
}

template<typename T, unsigned h, unsigned w>
class matrix_sum : public matrix_expression<T> {
public:

    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_sum;

    matrix_wrap<T> resolve_all() {
        std::vector<std::future<matrix_wrap<T>>> futures;
        for (auto &&aux : operands) {
            futures.push_back(ThreadPool::getSingleton().enqueue([&aux]() { return aux->resolve_all(); }));
        }
        while (futures.size() > 1) {
            std::vector<std::future<matrix_wrap<T>>> futures_aux;
            for (unsigned i = 0; i < futures.size() - 1; i += 2) {
                matrix_wrap<T> m1 = futures[i].get();
                matrix_wrap<T> m2 = futures[i + 1].get();
                futures_aux.push_back(ThreadPool::getSingleton().enqueue(sum<T>, m1, m2));
            }
            if (futures.size() % 2) {
                futures_aux.push_back(std::move(futures[futures.size() - 1]));
            }
            futures = std::move(futures_aux);
        }
        return futures[0].get();
    }

    operator matrix<T>() {
        matrix_wrap <T> res = resolve_all();
        return res.get_submatrix({0, res.get_height() - 1, 0, res.get_width() - 1});
    }

    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>() {
        static_assert((h == 0 || h == h2) && (w == 0 || w == w2), "sized product conversion to wrong sized matrix");
        matrix_wrap <T> res = resolve_all();
        return (matrix<T, h2, w2>) res.get_submatrix({0, res.get_height() - 1, 0, res.get_width() - 1});
    }

    unsigned get_height() const {
        return operands.front()->get_height();
    }

    unsigned get_width() const {
        return operands.front()->get_width();
    }

    /**
     Declarations of friends functions
     **/
    template<typename V, typename U, class LType, class RType>
    std::enable_if_t<matrix_ref<V, LType>::H != 0 && matrix_ref<U, RType>::H != 0,
            matrix_sum<decltype(V() + U()), matrix_ref<V, LType>::H, matrix_ref<V, LType>::W>>
    friend operator+(const matrix_ref<V, LType> &lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, class LType, class RType>
    std::enable_if_t<
            matrix_ref<V, LType>::H == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(const matrix_ref<V, LType> &lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hf, unsigned wf, class RType>
    std::enable_if_t<hf != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(V() + U()), hf, wf>>
    friend operator+( matrix_sum<V, hf, wf> &&lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hf, unsigned wf, class RType>
    std::enable_if_t<hf == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+( matrix_sum<V, hf, wf> &&lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hf, unsigned wf, class RType>
    std::enable_if_t<hf != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(V() + U()), hf, wf>>
    friend operator+(const matrix_ref<U, RType> &lhs, matrix_sum<V, hf, wf> &&rhs);

    template<typename V, typename U, unsigned hf, unsigned wf, class RType>
    std::enable_if_t<hf == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(const matrix_ref<U, RType> &lhs, matrix_sum<V, hf, wf> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(V() + U()), hl, wl>>
    friend operator+( matrix_sum<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr, class RType>
    std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+( matrix_sum<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    matrix_sum(matrix_sum<T, h, w> &&X) : operands(std::move(X.operands)) {}

    matrix_sum(const matrix_sum<T, h, w> &X) = delete;

    ~matrix_sum() = default;


private:

    std::list<std::unique_ptr<matrix_expression<T>>> operands;

    matrix_sum() = default;

    void add(std::unique_ptr<matrix_expression<T>> &&mat) {
        operands.push_back(std::move(mat));
    }

};

/**
 * Static overload of sum operation between matrix_ref and matrix_ref
 * @tparam T elements type of the first matrix_ref
 * @tparam U elements type of the second matrix_ref
 * @tparam LType inner type of the first matrix_ref
 * @tparam RType inner type of the second matrix_ref
 * @param lhs first matrix_ref
 * @param rhs second matrix_ref
 * @return a \matrix_sum that contains the two matrix_refs
 */
template<typename T, typename U, class LType, class RType>
std::enable_if_t<matrix_ref<T, LType>::H != 0 && matrix_ref<U, RType>::H != 0,
        matrix_sum<decltype(T() + U()), matrix_ref<T, LType>::H, matrix_ref<T, LType>::W>>
operator+(const matrix_ref<T, LType> &lhs, const matrix_ref<U, RType> &rhs) {
    static_assert((matrix_ref<T, LType>::W == matrix_ref<U, RType>::W) &&
                  (matrix_ref<T, LType>::H == matrix_ref<U, RType>::H),
                  "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), matrix_ref<T, LType>::H, matrix_ref<T, LType>::W> result;
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(lhs)));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(rhs)));
    return result;
}

/**
 * Dynamic overload of sum operation between matrix_ref and matrix_ref
 * @tparam T elements type of the first matrix_ref
 * @tparam U elements type of the second matrix_ref
 * @tparam LType inner type of the first matrix_ref
 * @tparam RType inner type of the second matrix_ref
 * @param lhs first matrix_ref
 * @param rhs second matrix_ref
 * @return a \matrix_sum that contains the two matrix_refs
 */
template<typename T, typename U, class LType, class RType>
std::enable_if_t<matrix_ref<T, LType>::H == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_ref<T, LType> &lhs, const matrix_ref<U, RType> &rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width())
        throw std::domain_error("dimension mismatch in Matrix addition");
    // 0,0 because we loose the static information
    matrix_sum<decltype(T() + U()), 0, 0> result;
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(lhs)));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(rhs)));
    return result;
}

/**
 * Static overload for sum operation between matrix_sum and matrix_ref
 * @tparam T elements type of matrix_sum
 * @tparam U elements type of matrix_ref
 * @tparam h height of matrix_sum
 * @tparam w width of matrix_sum
 * @tparam RType inner type of matrix_ref
 * @param lhs matrix_sum
 * @param rhs matrix_ref
 * @return the same matrix_sum with a the matrix_ref appended
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(T() + U()), h, w>>
operator+(matrix_sum<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    static_assert(matrix_ref<T, RType>::W * w == 0 || (matrix_ref<T, RType>::H == h && matrix_ref<T, RType>::W == w),
                  "dimension mismatch in Matrix multiplication");
    matrix_sum<decltype(T() + U()), h, w> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(rhs)));
    return result;
}

/**
 * Dynamic overload for sum operation between matrix_sum and matrix_ref
 * @tparam T elements type of matrix_sum
 * @tparam U elements type of matrix_ref
 * @tparam h height of matrix_sum
 * @tparam w width of matrix_sum
 * @tparam RType inner type of matrix_ref
 * @param lhs matrix_sum
 * @param rhs matrix_ref
 * @return the same matrix_sum with a the matrix_ref appended
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(matrix_sum<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), h, w> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(rhs)));
    return result;
}

/**
 * Static overload for sum operation between matrix_ref and matrix_sum
 * @tparam T elements type of matrix_sum
 * @tparam U elements type of matrix_ref
 * @tparam h height of matrix_sum
 * @tparam w width of matrix_sum
 * @tparam RType inner type of matrix_ref
 * @param lhs matrix_ref
 * @param rhs matrix_sum
 * @return the same matrix_sum with a the matrix_ref appended
*/
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(T() + U()), h, w>>
operator+(const matrix_ref<U, RType> &lhs, matrix_sum<T, h, w> &&rhs) {
    static_assert(matrix_ref<T, RType>::W * w == 0 || (matrix_ref<T, RType>::H == h && matrix_ref<T, RType>::W == w),
                  "dimension mismatch in Matrix multiplication");
    matrix_sum<decltype(T() + U()), h, w> result(std::move(rhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(lhs)));
    return result;
}

/**
 * Dynamic overload for sum operation between matrix_ref and matrix_sum
 * @tparam T elements type of matrix_sum
 * @tparam U elements type of matrix_ref
 * @tparam h height of matrix_sum
 * @tparam w width of matrix_sum
 * @tparam RType inner type of matrix_ref
 * @param lhs matrix_ref
 * @param rhs matrix_sum
 * @return the same matrix_sum with a the matrix_ref appended
*/
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_ref<U, RType> &lhs, matrix_sum<T, h, w> &&rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), h, w> result(std::move(rhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(lhs)));
    return result;
}

/**
 * Static overload for sum operation between two matrix_sums
 * @tparam T elements type of the first matrix_sum
 * @tparam U elements type of the second matrix_sum
 * @tparam hl height of the first matrix_sum
 * @tparam wl width of the first matrix_sum
 * @tparam hr height of the second matrix_sum
 * @tparam wr width of the second matrix_sum
 * @param lhs the first matrix sum
 * @param rhs the second matrix sum
 * @return the first matrix_sum having as last element the second matrix_sum
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(T() + U()), hl, wl>>
operator+(matrix_sum<T, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs) {
    static_assert(hl == hr && wl == wr, "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), hl, wl> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;

}


/**
 * Static overload for sum operation between two matrix_sums
 * @tparam T elements type of the first matrix_sum
 * @tparam U elements type of the second matrix_sum
 * @tparam hl height of the first matrix_sum
 * @tparam wl width of the first matrix_sum
 * @tparam hr height of the second matrix_sum
 * @tparam wr width of the second matrix_sum
 * @param lhs the first matrix sum
 * @param rhs the second matrix sum
 * @return the first matrix_sum having as last element the second matrix_sum
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr, class RType>
std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+( matrix_sum<T, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), hl, wl> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;
}


#endif //ASSIGNMENT_3_MATRIX_SUM_H


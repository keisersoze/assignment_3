//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_PRODUCT_H
#define ASSIGNMENT_3_MATRIX_PRODUCT_H

#include<type_traits>
#include<list>
#include <thread>
#include "parallel_multiplication.h"
#include "matrix_expression.h"
#include "thread_pool.h"

template<typename T, unsigned h, unsigned w>
class matrix_product : public matrix_expression<T> {
public:

    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_product;

    void update_sizes() const;

    matrix<T> resolve_all() {
        std::vector<std::future<matrix<T>>> futures;
        for (auto &&aux : operands) {
            futures.push_back(ThreadPool::getSingleton().enqueue([&aux]() { return aux->resolve_all(); }));
        }
        while (futures.size() > 2) {
            unsigned lhs = find_max_and_update(futures);
            unsigned rhs = lhs + 1;
            futures.insert(futures.begin() + lhs, ThreadPool::getSingleton().enqueue(do_multiply<T>,
                    (*(futures.begin() + lhs)).get(), (*(futures.begin() + rhs)).get()));
            futures.erase(futures.begin() + rhs);
            futures.erase(futures.begin() + rhs);
        }
        return do_multiply(futures[0].get(), futures[1].get());
    }

    operator matrix<T>() {
        return resolve_all();
    }

    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>() {
        static_assert((h == 0 || h == h2) && (w == 0 || w == w2), "sized product conversion to wrong sized matrix");
        return (matrix<T, h2, w2>) resolve_all();
    }

    unsigned get_height() const { return operands.front()->get_height(); }

    unsigned get_width() const { return operands.back()->get_width(); }

    matrix_product(matrix_product<T, h, w> &&X) = default;

private:
    std::list<std::unique_ptr<matrix_expression<T>>> operands;
    std::vector<unsigned> sizes;

    matrix_product() = default;

    template<unsigned w2>
    matrix_product(matrix_product<T, h, w2> &&X) :operands(std::move(X.operands)), sizes(std::move(X.sizes)) {}

    void add(std::unique_ptr<matrix_expression<T>> &&mat) {
        sizes.push_back(mat->get_width());
        operands.push_back(std::move(mat));
    }


    unsigned find_max_and_update(
            std::vector<std::future<matrix<T>>> &futures) {
        auto mat_iter = futures.begin();
        auto size_iter = sizes.begin();
        auto size_iter_max = sizes.begin();
        auto last = --(sizes.end());
        unsigned mat_max = 0;
        unsigned size_max = *size_iter;
        unsigned i = 0;
        while (size_iter != last) {
            if (*size_iter > size_max) {
                size_max = *size_iter;
                size_iter_max = size_iter;
                mat_max = i;
            }
            ++mat_iter;
            ++size_iter;
            ++i;
        }
        sizes.erase(size_iter_max);
        return mat_max;
    }

    template<typename V, typename U, class LType, class RType>
    std::enable_if_t<matrix_ref<V, LType>::H != 0 && matrix_ref<U, RType>::H != 0,
            matrix_product<decltype(V() * U()), matrix_ref<V, LType>::H, matrix_ref<U, RType>::W>>
    friend operator*(const matrix_ref<V, LType> &lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, class LType, class RType>
    std::enable_if_t<
            matrix_ref<V, LType>::H == 0 || matrix_ref<U, RType>::H == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(const matrix_ref<V, LType> &lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<hl != 0 && matrix_ref<U, RType>::H != 0,
            matrix_product<decltype(V() + U()), hl, matrix_ref<V, RType>::W>>
    friend operator*(matrix_product<V, hl, wl> &&lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<hl == 0 || matrix_ref<U, RType>::H == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(matrix_product<V, hl, wl> &&lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<
            hl != 0 && matrix_ref<U, RType>::H != 0, matrix_product<decltype(V() * U()), hl, matrix_ref<U, RType>::W>>
    friend operator*(const matrix_ref<U, RType> &lhs, matrix_product<V, hl, wl> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<hl == 0 || matrix_ref<U, RType>::H == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(const matrix_ref<U, RType> &lhs, matrix_product<V, hl, wl> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<hl == 0 || matrix_ref<U, RType>::H == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(const matrix_ref<U, RType> &lhs, matrix_product<V, hl, wl> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_product<decltype(V() * U()), hl, wr>>
    friend operator*(matrix_product<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr, class RType>
    std::enable_if_t<hl == 0 || hr == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(matrix_product<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);
};

/**
 * Static overload of product operation between matrix_ref and matrix_ref
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
        matrix_product<decltype(T() * U()), matrix_ref<T, LType>::H, matrix_ref<U, RType>::W>>
operator*(const matrix_ref<T, LType> &lhs, const matrix_ref<U, RType> &rhs) {
    static_assert((matrix_ref<T, LType>::W == matrix_ref<U, RType>::H),
                  "dimension mismatch in Matrix addition");
    matrix_product<decltype(T() * U()), matrix_ref<T, LType>::H, matrix_ref<U, RType>::W> result;
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(lhs)));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(rhs)));
    return result;
}

/**
 * Dynamic overload of product operation between matrix_ref and matrix_ref
 * @tparam T elements type of the first matrix_ref
 * @tparam U elements type of the second matrix_ref
 * @tparam LType inner type of the first matrix_ref
 * @tparam RType inner type of the second matrix_ref
 * @param lhs first matrix_ref
 * @param rhs second matrix_ref
 * @return a \matrix_sum that contains the two matrix_refs
 */
template<typename T, typename U, class LType, class RType>
std::enable_if_t<
        matrix_ref<T, LType>::H == 0 || matrix_ref<U, RType>::H == 0, matrix_product<decltype(T() * U()), 0, 0>>
operator*(const matrix_ref<T, LType> &lhs, const matrix_ref<U, RType> &rhs) {
    if (lhs.get_width() != rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix addition");
    // 0,0 because we loose the static information
    matrix_product<decltype(T() * U()), 0, 0> result;
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(lhs)));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(rhs)));
    return result;
}

/**
 * Static overload for product operation between matrix_sum and matrix_ref
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
std::enable_if_t<h != 0 && matrix_ref<U, RType>::H != 0,
        matrix_product<decltype(T() + U()), h, matrix_ref<T, RType>::W>>
operator*(matrix_product<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    static_assert(matrix_ref<T, RType>::W * w == 0 || (matrix_ref<T, RType>::H == w),
                  "dimension mismatch in Matrix multiplication");
    matrix_product<decltype(T() + U()), h, matrix_ref<T, RType>::W> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(rhs))
    );
    return result;
}

/**
 * Dynamic overload for product operation between matrix_sum and matrix_ref
 * @tparam T elements type of matrix_sum
 * @tparam U elements type of matrix_ref
 * @tparam h height of matrix_sum
 * @tparam w width of matrix_sum
 * @tparam RType inner type of matrix_ref
 * @param lhs matrix_product
 * @param rhs matrix_ref
 * @return the same matrix_product with a the matrix_ref appended
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h == 0 || matrix_ref<U, RType>::H == 0, matrix_product<decltype(T() * U()), 0, 0>>
operator*(matrix_product<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    if (lhs.get_width() != rhs.get_height()) {
        throw std::domain_error("dimension mismatch in Matrix product");
    }
    matrix_product<decltype(T() + U()), h, w> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(rhs)));
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
std::enable_if_t<
        h != 0 && matrix_ref<U, RType>::H != 0, matrix_product<decltype(T() * U()), h, matrix_ref<U, RType>::W>>
operator*(const matrix_ref<U, RType> &lhs, matrix_product<T, h, w> &&rhs) {
    static_assert(matrix_ref<T, RType>::W * w == 0 || (matrix_ref<T, RType>::W == h),
                  "dimension mismatch in Matrix multiplication");
    matrix_product<decltype(T() * U()), h, matrix_ref<T, RType>::W> result(std::move(rhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(lhs)));
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
std::enable_if_t<h == 0 || matrix_ref<U, RType>::H == 0, matrix_product<decltype(T() * U()), 0, 0>>
operator*(const matrix_ref<U, RType> &lhs, matrix_product<T, h, w> &&rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_product<decltype(T() * U()), h, w> result(std::move(rhs));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() * U())>>(lhs)));
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
std::enable_if_t<hl != 0 && hr != 0, matrix_product<decltype(T() * U()), hl, wr>>
operator*(matrix_product<T, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs) {
    static_assert(wl == hr, "dimension mismatch in Matrix addition");
    matrix_product<decltype(T() * U()), hl, wr> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_product<decltype(T() * U()), hr, wr>>(std::move(rhs))));
    result.add(std::move(std::make_unique<matrix_product<decltype(T() * U()), hl, wl>>(std::move(lhs))));
    return result;
}


/**
 * Dynamic overload for sum operation between two matrix_products
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
std::enable_if_t<hl == 0 || hr == 0, matrix_product<decltype(T() * U()), 0, 0>>
operator*(matrix_product<T, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs) {
    if (lhs.get_width() != rhs.get_height()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_product<decltype(T() + U()), hl, wl> result(std::move(lhs));
    result.add(std::move(std::make_unique<matrix_product<decltype(T() * U()), hr, wr>>(std::move(rhs))));
    result.add(std::move(std::make_unique<matrix_product<decltype(T() * U()), hl, wl>>(std::move(lhs))));
    return result;
}

#endif //ASSIGNMENT_3_MATRIX_PRODUCT_H

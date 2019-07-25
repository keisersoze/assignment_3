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


template<typename T, unsigned h, unsigned w>
class matrix_sum : matrix_operation_s<T, h, w> {
public:

    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_sum;

    unsigned get_height() {
        return this->matrices.front().get_height();
    }

    unsigned get_width() {
        return this->matrices.front().get_width();
    }

    matrix<T> resolve_all() {
        std::vector<std::future<matrix_wrap<T>>> futures;
        for (auto aux : matrices) {
            futures.insert(ThreadPool::getSingleton().enqueue(aux.resolve_all));
        }

        unsigned height = get_height();
        unsigned width = get_width();

        matrix<T> result(get_height(), get_width());

        for (unsigned i; i < height; i++) {
            for (unsigned j; j < width; j++) {
                result[i][j] = 0;
            }
        }

        for (auto future : futures) {
            auto aux = future.get();
            for (unsigned i; i < height; i++) {
                for (unsigned j; j < width; j++) {
                    result[i][j] += aux[i][j];
                }
            }
        }

        // std::cerr << "addition conversion\n";
        return result;
    }

    operator matrix<T>() {
        return resolve_all();
    }

    unsigned get_height() const { return matrices.front().get_height(); }

    unsigned get_width() const { return matrices.back().get_width(); }

    //TODO related to matrix_sum() constructor
    //template<typename V, typename U, class LType, class RType>
    //friend matrix_sum<decltype(T() + U()), 0, 0> operator+(const matrix_ref<V, LType> &lhs, const matrix_ref<U, RType> &rhs);

    //template, typename V, typename U, unsigned h2, unsigned w2, class RType>
    //friend  operator+(matrix_operation_s<V, h2, w2> &&lhs, const matrix_ref<U, RType> &rhs);

    matrix_sum(matrix_sum<T, h, w> &&X) = default;


    //TODO make the constructor private and add function
    matrix_sum() = default;


    template<typename W>
    void add(matrix_operation<W> &mat) {
        matrices.emplace_back(&mat);
        sizes.push_back(mat.get_width());
    }


private:

    template<unsigned w2>
    matrix_sum(matrix_sum<T, h, w2> &&X) : matrices(std::move(X.matrices)), sizes(std::move(X.sizes)) {}


    void resolve() { while (matrices.size() > 2) resolve_one(); }

    void resolve_one() {
        typename std::list<matrix_wrap<T>>::iterator lhs = find_max();
        typename std::list<matrix_wrap<T>>::iterator rhs = lhs;
        ++rhs;
        typename std::list<matrix_wrap<T>>::iterator result = matrices.emplace(lhs, matrix<T>(lhs->get_height(),
                                                                                              rhs->get_width()));
        do_multiply(*result, *lhs, *rhs);
        matrices.erase(lhs);
        matrices.erase(rhs);
    }

    typename std::list<matrix_wrap<T>>::iterator find_max() {
        typename std::list<matrix_wrap<T>>::iterator mat_iter = matrices.begin();
        typename std::list<matrix_wrap<T>>::iterator mat_max = mat_iter;
        std::vector<unsigned>::iterator size_iter = sizes.begin();
        std::vector<unsigned>::iterator last = --(sizes.end());
        unsigned size_max = *size_iter;
        while (size_iter != last) {
            if (*size_iter > size_max) {
                size_max = *size_iter;
                mat_max = mat_iter;
            }
            ++mat_iter;
            ++size_iter;
        }
        return mat_max;
    }

    std::list<std::unique_ptr<matrix_operation<T>>> matrices;
    std::vector<unsigned> sizes;

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
    result.add(matrix_singleton(lhs));
    result.add(matrix_singleton(rhs));
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
    matrix_singleton<T> lhs_wrapped  (lhs);
    matrix_singleton<T> rhs_wrapped  (rhs);
    result.add(lhs_wrapped);
    result.add(rhs_wrapped);
    return result;
}
/**
 * Static overload for sum operation between matrix_operation and matrix_ref
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
operator+(matrix_operation_s<T, h, w> &lhs, const matrix_ref<U, RType> &rhs) {
    static_assert(matrix_ref<T, RType>::W * w == 0 ||
                  (matrix_ref<T, RType>::H == h &&
                   matrix_ref<T, RType>::W == w),
                  "dimension mismatch in Matrix multiplication");
    matrix_sum<decltype(T() + U()), matrix_ref<T, RType>::H, matrix_ref<T, RType>::W> result;
    result.add(lhs);
    result.add(matrix_singleton(rhs));
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
operator+(matrix_operation_s<T, h, w> &lhs, const matrix_ref<U, RType> &rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), 0, 0> result;
    result.add(lhs);
    result.add(matrix_singleton(rhs));
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
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t
        <h != 0 && matrix_ref<U, RType>::H != 0,
                matrix_sum<decltype(T() + U()), h, w>>
operator+(const matrix_ref<U, RType> &lhs, matrix_operation_s<T, h, w> &rhs) {
    return rhs + lhs;
}
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
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t
        <h == 0 || matrix_ref<U, RType>::H == 0,
                matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_ref<U, RType> &lhs, matrix_operation_s<T, h, w> &rhs) {
    return rhs + lhs;
}

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
 */
template<typename T, typename U, unsigned hl, unsigned wl,unsigned hr, unsigned wr, class RType>
std::enable_if_t
        <hl != 0 && hr != 0,
                matrix_sum<decltype(T() + U()), hl, wl>>
operator+(const matrix_operation_s<T, hl, wl> &lhs, const matrix_operation_s<U, hr, wr> &rhs) {
    static_assert(hl == hr && wl ==wr, "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), hl, wl> result;
    result.add(lhs);
    result.add(rhs);
    return result;

}
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
 */
template<typename T, typename U, unsigned hl, unsigned wl,unsigned hr, unsigned wr, class RType>
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



#endif //ASSIGNMENT_3_MATRIX_SUM_H


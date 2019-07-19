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

//TODO useless struct
template<typename T, typename U>
struct op_traits {
    typedef decltype(T() + U()) sum_type;
    typedef decltype(T() * U()) prod_type;
};

template<typename T, class LType, typename U, class RType>
std::enable_if_t<matrix_ref<T, LType>::H * matrix_ref<U, RType>::H == 0, matrix<typename op_traits<T, U>::sum_type>>
operator+(const matrix_ref<T, LType> &left, const matrix_ref<U, RType> &right) {
    if (left.get_height() != right.get_height() || left.get_width() != right.get_width())
        throw std::domain_error("dimension mismatch in Matrix addition");

    const unsigned height = left.get_height();
    const unsigned width = left.get_width();
    matrix<typename op_traits<T, U>::sum_type> result(height, width);
    for (unsigned i = 0; i != height; ++i)
        for (unsigned j = 0; j != width; j++)
            result(i, j) = left(i, j) + right(i, j);

    return result;
}


template<typename T, typename U, class RType>
std::enable_if_t<std::is_same<T, typename op_traits<T, U>::sum_type>::value, matrix<T>>
operator+(matrix<T> &&left, const matrix_ref<U, RType> &right) {
    if (left.get_height() != right.get_height() || left.get_width() != right.get_width())
        throw std::domain_error("dimension mismatch in Matrix addition");

    const unsigned height = left.get_height();
    const unsigned width = left.get_width();
    for (unsigned i = 0; i != height; ++i)
        for (unsigned j = 0; j != width; j++)
            left(i, j) += right(i, j);

    return std::move(left);
}


template<typename T, class LType, typename U, class RType>
std::enable_if_t<matrix_ref<T, LType>::H * matrix_ref<U, RType>::H !=
                 0, matrix<typename op_traits<T, U>::sum_type, matrix_ref<T, LType>::H, matrix_ref<T, LType>::W>>
operator+(const matrix_ref<T, LType> &left, const matrix_ref<U, RType> &right) {
    static_assert(
            matrix_ref<T, LType>::H == matrix_ref<U, RType>::H && matrix_ref<T, LType>::W == matrix_ref<U, RType>::W,
            "dimension mismatch in Matrix addition");

    const unsigned height = left.get_height();
    const unsigned width = left.get_width();
    matrix<typename op_traits<T, U>::sum_type, matrix_ref<T, LType>::H, matrix_ref<T, LType>::W> result;
    for (unsigned i = 0; i != height; ++i)
        for (unsigned j = 0; j != width; j++)
            result(i, j) = left(i, j) + right(i, j);

    return result;
}


template<typename T, unsigned H, unsigned W, typename U, class RType>
std::enable_if_t<
        matrix_ref<U, RType>::H != 0 && std::is_same<T, typename op_traits<T, U>::sum_type>::value, matrix<T, H, W>>
operator+(matrix<T, H, W> &&left, const matrix_ref<U, RType> &right) {
    static_assert(H == matrix_ref<U, RType>::H && W == matrix_ref<U, RType>::W,
                  "dimension mismatch in Matrix addition");

    for (unsigned i = 0; i != H; ++i) {
        for (unsigned j = 0; j != W; j++) {
            left(i, j) += right(i, j);
        }
    }

    return std::move(left);
}

template<typename T, unsigned h, unsigned w>
class matrix_sum : matrix_operation<T> {
public:

    static constexpr unsigned H = h;
    static constexpr unsigned W = w;

    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_sum;

    matrix_wrap<T> resolve_all() {
        std::vector<std::future<matrix_wrap<T>>> biscottiniAllaMarijuanaSativa;
        for (auto aux : matrices) {
            biscottiniAllaMarijuanaSativa.insert(ThreadPool::getSingleton().enqueue(aux.resolve_all));
        }
        matrix_wrap<T> result(lhsHeight, lhsWidth);


        matrix_wrap<T> lhs = matrices.front().resolve_all(), rhs = matrices.back().resolve_all();

        const unsigned lhsHeight = lhs.get_height();
        const unsigned lhsWidth = lhs.get_width();
        const unsigned rhsHeight = rhs.get_height();
        const unsigned rhsWidth = rhs.get_width();

        assert((lhsHeight == rhsHeight) && (lhsWidth == rhsWidth));
        matrix<T> result(lhsHeight, lhsWidth);
        for (unsigned i = 0; i != lhsHeight; ++i) {
            for (unsigned j = 0; j != lhsWidth; ++j) {
                result(i, j) += lhs(i, j) * rhs(i, j);
            }
        }

        std::cerr << "product conversion\n";
        return result;
    }

    operator matrix<T>() {
        resolve();
        matrix<T> lhs = matrices.front().resolve_all(), rhs = matrices.back().resolve_all();

        const unsigned lhsHeight = lhs.get_height();
        const unsigned lhsWidth = lhs.get_width();
        const unsigned rhsHeight = rhs.get_height();
        const unsigned rhsWidth = rhs.get_width();

        assert((lhsHeight == rhsHeight) && (lhsWidth == rhsWidth));
        matrix<T> result(lhsHeight, lhsWidth);
        for (unsigned i = 0; i != lhsHeight; ++i) {
            for (unsigned j = 0; j != lhsWidth; ++j) {
                result(i, j) += lhs(i, j) * rhs(i, j);
            }
        }

        std::cerr << "product conversion\n";
        return result;
    }

    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>() {
        static_assert((h == 0 || h == h2) && (w == 0 || w == w2), "sized product conversion to wrong sized matrix");
        assert(h2 == get_height() && w2 == get_width());
        resolve();
        matrix_wrap<T> lhs = matrices.front(), rhs = matrices.back();
        const unsigned span = lhs.get_width();
        assert(span == rhs.get_height());
        matrix<T, h2, w2> result;
        for (unsigned i = 0; i != h2; ++i)
            for (unsigned j = 0; j != w2; ++j) {
                result(i, j) = 0;
                for (unsigned k = 0; k != span; ++k)
                    result(i, j) += lhs(i, k) * rhs(k, j);
            }

        std::cerr << "sized product conversion\n";
        return result;
    }

    unsigned get_height() const { return matrices.front().get_height(); }

    unsigned get_width() const { return matrices.back().get_width(); }


    template<typename U, class LType, class RType>
    friend matrix_sum <U, matrix_ref<U, LType>::H, matrix_ref<U, RType>::W>
    operator*(const matrix_ref<U, LType> &lhs, const matrix_ref<U, RType> &rhs);

    template<typename U, unsigned h2, unsigned w2, class RType>
    friend matrix_sum <U, h2, matrix_ref<U, RType>::W>
    operator*(matrix_sum <U, h2, w2> &&lhs, const matrix_ref<U, RType> &rhs);


    matrix_sum(matrix_sum<T, h, w> &&X) = default;



private:

    matrix_sum() = default;

    template<unsigned w2>
    matrix_sum(matrix_sum<T, h, w2> &&X) : matrices (std::move(X.matrices)), sizes (std::move(X.sizes)) {}

    template<class matrix_type>
    void add(matrix_ref<T, matrix_type> mat) {
        matrices.emplace_back(mat);
        sizes.push_back(mat.get_width());
    }


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

    std::list<matrix_operation<T>> matrices;
    std::vector<unsigned> sizes;

};

#endif //ASSIGNMENT_3_MATRIX_SUM_H


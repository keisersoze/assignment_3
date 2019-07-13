#ifndef OPERATIONS_H
#define OPERATIONS_H

#include<type_traits>
#include<list>
#include <thread>
#include "parallel_multiplication.h"

#include"matrix.h"


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

    for (unsigned i = 0; i != H; ++i)
        for (unsigned j = 0; j != W; j++)
            left(i, j) += right(i, j);

    return std::move(left);
}


template<typename T, unsigned h, unsigned w>
class matrix_product {
public:
    std::list<matrix_wrap<T>> matrices;
    std::vector<unsigned> sizes;

    static constexpr unsigned H = h;
    static constexpr unsigned W = w;


    operator matrix<T>() {
        resolve();
        matrix_wrap<T> lhs = matrices.front(), rhs = matrices.back();
        const unsigned height = lhs.get_height();
        const unsigned width = rhs.get_width();
        const unsigned span = lhs.get_width();
        assert(span == rhs.get_height());
        matrix<T> result(height, width);
        for (unsigned i = 0; i != height; ++i)
            for (unsigned j = 0; j != width; ++j) {
                result(i, j) = 0;
                for (unsigned k = 0; k != span; ++k)
                    result(i, j) += lhs(i, k) * rhs(k, j);
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
    friend matrix_product<U, matrix_ref<U, LType>::H, matrix_ref<U, RType>::W>
    operator*(const matrix_ref<U, LType> &lhs, const matrix_ref<U, RType> &rhs);

    template<typename U, unsigned h2, unsigned w2, class RType>
    friend matrix_product<U, h2, matrix_ref<U, RType>::W>
    operator*(matrix_product<U, h2, w2> &&lhs, const matrix_ref<U, RType> &rhs);


    matrix_product(matrix_product<T, h, w> &&X) = default;

private:

    matrix_product() = default;

    template<unsigned w2>
    matrix_product(matrix_product<T, h, w2> &&X) : matrices(std::move(X.matrices)), sizes(std::move(X.sizes)) {}

    template<class matrix_type>
    void add(matrix_ref<T, matrix_type> mat) {
        matrices.emplace_back(mat);
        sizes.push_back(mat.get_width());
    }


    void resolve() {
        while (matrices.size() > 2) {
            resolve_one();
        }
    }

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
    /************************************************************
     ****	Previous implementation of parallel multiplication	*
     **** 	Ovverided in in parallel_multiplication.h			*
     ************************************************************
	void do_multiply(matrix_wrap<T> result, matrix_wrap<T> lhs, matrix_wrap<T> rhs) {
		const unsigned height = result.get_height();
		const unsigned width = result.get_width();
		const unsigned span = lhs.get_width();
		assert(span==rhs.get_height());
		for (unsigned i=0; i!=height; ++i)
			for (unsigned j=0; j!=width; ++j) {
				result(i,j)=0;
				for (unsigned k=0; k!=span; ++k)
					result(i,j) += lhs(i,k)*rhs(k,j);
				}
	}
    **/
};


template<typename T, class LType, class RType>
matrix_product<T, matrix_ref<T, LType>::H, matrix_ref<T, RType>::W>
operator*(const matrix_ref<T, LType> &lhs, const matrix_ref<T, RType> &rhs) {
    static_assert(matrix_ref<T, LType>::W * matrix_ref<T, LType>::H == 0 ||
                  matrix_ref<T, LType>::W == matrix_ref<T, RType>::H,
                  "dimension mismatch in Matrix multiplication");
    if (lhs.get_width() != rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    matrix_product<T, matrix_ref<T, LType>::H, matrix_ref<T, RType>::W> result;
    result.add(lhs);
    result.add(rhs);
    return result;
}


template<typename T, unsigned h, unsigned w, class RType>
matrix_product<T, h, matrix_ref<T, RType>::W>
operator*(matrix_product<T, h, w> &&lhs, const matrix_ref<T, RType> &rhs) {
    static_assert(w * matrix_ref<T, RType>::H == 0 || w == matrix_ref<T, RType>::H,
                  "dimension mismatch in Matrix multiplication");
    if (lhs.get_width() != rhs.get_height())
        throw std::domain_error("dimension mismatch in Matrix multiplication");
    matrix_product<T, h, matrix_ref<T, RType>::W> result(std::move(lhs));
    result.add(rhs);
    return result;
}


#endif // OPERATIONS_H

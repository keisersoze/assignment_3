//
// Created by Gianmarco Callegher on 2019-07-18.
//

#ifndef ASSIGNMENT_3_MATRIX_SUM_CC
#define ASSIGNMENT_3_MATRIX_SUM_CC

#include <type_traits>
#include <list>
#include <thread>
#include "parallel_multiplication.h"
#include "matrix_expression.h"
#include "matrix_fwd.h"
#include "thread_pool.h"
#include "matrix_product.h"
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

    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_product;

    matrix_wrap<T> resolve_all() {
        std::vector<std::future<matrix_wrap<T>>> futures;
        for (auto &&aux : operands) {
            futures.push_back(std::async(std::launch::async, &matrix_expression<T>::resolve_all, std::move(aux)));
        }
        while (futures.size() > 1) {
            int num_iter = futures.size() / 2;
            for (unsigned i = 0; i < num_iter; i++) {
                matrix_wrap<T> m1 = futures[0].get();
                matrix_wrap<T> m2 = futures[1].get();
                futures.push_back(std::async(std::launch::async, sum<T>, m1, m2));
                futures.erase(futures.begin());
                futures.erase(futures.begin());
            }
        }
        return futures[0].get();
    }

    operator matrix<T>() {
        matrix_wrap<T> res = resolve_all();
        return res.get_submatrix({0, res.get_height() - 1, 0, res.get_width() - 1});
    }

    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>() {
        static_assert((h == 0 || h == h2) && (w == 0 || w == w2), "sized product conversion to wrong sized matrix");
        matrix_wrap<T> res = resolve_all();
        return (matrix<T, h2, w2>) res.get_submatrix({0, res.get_height() - 1, 0, res.get_width() - 1});
    }

    unsigned get_height() const {
        return operands.front()->get_height();
    }

    unsigned get_width() const {
        return operands.front()->get_width();
    }

    matrix_sum(matrix_sum<T, h, w> &&X) : operands(std::move(X.operands)) {}

    matrix_sum(const matrix_sum<T, h, w> &X) = delete;

    ~matrix_sum() = default;

private:
    std::list<std::unique_ptr<matrix_expression<T>>> operands;

    matrix_sum() = default;

    void add(std::unique_ptr<matrix_expression<T>> &&mat) {
        operands.push_back(std::move(mat));
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
    friend operator+(matrix_sum<V, hf, wf> &&lhs,
                     const matrix_ref<U, RType> &rhs
    );

    template<typename V, typename U, unsigned hf, unsigned wf, class RType>
    std::enable_if_t<hf == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(matrix_sum<V, hf, wf> &&lhs,
                     const matrix_ref<U, RType> &rhs
    );

    template<typename V, typename U, unsigned hf, unsigned wf, class RType>
    std::enable_if_t<hf != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(V() + U()), hf, wf>>
    friend operator+(const matrix_ref<U, RType> &lhs, matrix_sum<V, hf, wf> &&rhs);

    template<typename V, typename U, unsigned hf, unsigned wf, class RType>
    std::enable_if_t<hf == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(const matrix_ref<U, RType> &lhs, matrix_sum<V, hf, wf> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(V() + U()), hl, wl>>
    friend operator+(matrix_sum<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(matrix_sum<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(V() + U()), hl, wl>>
    friend operator+(matrix_product<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(matrix_product<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(V() + U()), hl, wl>>
    friend operator+(matrix_sum<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(matrix_sum<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(V() + U()), hl, wl>>
    friend operator+(matrix_product<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(matrix_product<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<hl != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(V() + U()), hl, wl>>
    friend operator+(matrix_product<V, hl, wl> &&lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<hl == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(matrix_product<V, hl, wl> &&lhs, const matrix_ref<U, RType> &rhs);

    template<typename V, typename U, unsigned hr, unsigned wr, class RType>
    std::enable_if_t<hr != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(V() + U()), hr, wr>>
    friend operator+(const matrix_ref<U, RType> &lhs, matrix_product<V, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, class RType>
    std::enable_if_t<hl == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(V() + U()), 0, 0>>
    friend operator+(const matrix_ref<U, RType> &lhs, matrix_product<T, hl, wl> &&rhs);
};


/**
 * Static overload of sum operation between matrix_ref and matrix_ref
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
*/
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(T() + U()), h, w>>
operator+(const matrix_ref<U, RType> &lhs, matrix_sum<T, h, w> &&rhs) {
    return std::move(rhs) + lhs;
}

/**
 * Dynamic overload for sum operation between matrix_ref and matrix_sum
*/
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_ref<U, RType> &lhs, matrix_sum<T, h, w> &&rhs) {
    return std::move(rhs) + lhs;
}

/**
 * Static overload for sum operation between two matrix_sums
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(T() + U()), hl, wl>>
operator+(matrix_sum<T, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs) {
    static_assert(hl == hr && wl == wr, "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), hl, wl> result;
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;

}


/**
 * Dynamic overload for sum operation between two matrix_sums
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(matrix_sum<T, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), 0, 0> result;
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;
}

/**
 * Static overload for sum operation between matrix_product and matrix_sum
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(T() + U()), hl, wl>>
operator+(matrix_product<T, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs) {
    static_assert(hl == hr && wl == wr, "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), hl, wl> result;
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), hl, wl>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;

}


/**
 * Dynamic overload for sum operation between matrix_product and matrix_sum
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(matrix_product<T, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), 0, 0> result;
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), hl, wl>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_sum<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;
}

/**
 * Static overload for sum operation between matrix_product and matrix_sum
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(T() + U()), hl, wl>>
operator+(matrix_sum<T, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs) {
    return rhs + lhs;

}


/**
 * Dynamic overload for sum operation between matrix_product and matrix_sum
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(matrix_sum<T, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs) {
    return rhs + lhs;
}

/**
 * Static overload for sum operation between matrix_product and matrix_product
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl != 0 && hr != 0, matrix_sum<decltype(T() + U()), hl, wl>>
operator+(matrix_product<T, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs) {
    static_assert(hl == hr && wl == wr, "dimension mismatch in Matrix addition");
    matrix_sum<decltype(T() + U()), hl, wl> result;
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), hl, wl>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;

}


/**
 * Dynamic overload for sum operation between matrix_product and matrix_product
*/
template<typename T, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
std::enable_if_t<hl == 0 || hr == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(matrix_product<T, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), 0, 0> result;
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), hl, wl>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), hr, wr>>(std::move(rhs))));
    return result;
}


/**
 * Static overload for sum operation between matrix_product and matrix_ref
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(T() + U()), h, w>>
operator+(matrix_product<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    static_assert(matrix_ref<T, RType>::W * w == 0 || (matrix_ref<T, RType>::H == h && matrix_ref<T, RType>::W == w),
                  "dimension mismatch in Matrix multiplication");

    matrix_sum<decltype(T() + U()), h, w> result;
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), h, w>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(rhs)));
    return result;
}

/**
 * Dynamic overload for sum operation between matrix_product and matrix_ref
 */
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(matrix_product<T, h, w> &&lhs, const matrix_ref<U, RType> &rhs) {
    if (lhs.get_height() != rhs.get_height() && lhs.get_width() != rhs.get_width()) {
        throw std::domain_error("dimension mismatch in Matrix addition");
    }
    matrix_sum<decltype(T() + U()), 0, 0> result;
    result.add(std::move(std::make_unique<matrix_product<decltype(T() + U()), h, w>>(std::move(lhs))));
    result.add(std::move(std::make_unique<matrix_singleton<decltype(T() + U())>>(rhs)));
    return result;
}

/**
 * Static overload for sum operation between matrix_ref and matrix_product
*/
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h != 0 && matrix_ref<U, RType>::H != 0, matrix_sum<decltype(T() + U()), h, w>>
operator+(const matrix_ref<U, RType> &lhs, matrix_product<T, h, w> &&rhs) {
    return std::move(rhs) + lhs;
}

/**
 * Dynamic overload for sum operation between matrix_ref and matrix_product
*/
template<typename T, typename U, unsigned h, unsigned w, class RType>
std::enable_if_t<h == 0 || matrix_ref<U, RType>::H == 0, matrix_sum<decltype(T() + U()), 0, 0>>
operator+(const matrix_ref<U, RType> &lhs, matrix_product<T, h, w> &&rhs) {
    return std::move(rhs) + lhs;
}


#endif //ASSIGNMENT_3_MATRIX_SUM_CC


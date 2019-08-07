#ifndef ASSIGNMENT_3_MATRIX_EXPRESSION_H
#define ASSIGNMENT_3_MATRIX_EXPRESSION_H

#include "matrix_wrap.h"
#include "matrix_fwd.h"

template<typename T>
class matrix_expression {
public:
    virtual matrix_wrap<T> resolve_all() = 0;

    virtual unsigned get_height() const = 0;

    virtual unsigned get_width() const = 0;

    virtual ~matrix_expression() {};

};

template<typename T, unsigned h, unsigned w>
class matrix_product : public matrix_expression<T> {
public:
    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_sum;
    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_product;

    matrix_wrap<T> resolve_all();

    operator matrix<T>();

    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>();

    unsigned get_height() const;

    unsigned get_width() const;

    matrix_product(matrix_product<T, h, w> &&X) = default;

private:
    std::list<std::unique_ptr<matrix_expression<T>>> operands;
    std::vector<unsigned> sizes;

    matrix_product() = default;

    template<unsigned w2>
    matrix_product(matrix_product<T, h, w2> &&X);

    void add(std::unique_ptr<matrix_expression<T>> &&mat);

    unsigned find_max_and_update(std::vector<std::future<matrix_wrap<T>>> &futures);

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

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(matrix_product<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_product<decltype(V() * U()), hl, wr>>
    friend operator*(matrix_sum<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(matrix_sum<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_product<decltype(V() * U()), hl, wr>>
    friend operator*(matrix_sum<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(matrix_sum<V, hl, wl> &&lhs, matrix_product<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl != 0 && hr != 0, matrix_product<decltype(V() * U()), hl, wr>>
    friend operator*(matrix_product<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);

    template<typename V, typename U, unsigned hl, unsigned wl, unsigned hr, unsigned wr>
    std::enable_if_t<hl == 0 || hr == 0, matrix_product<decltype(V() * U()), 0, 0>>
    friend operator*(matrix_product<V, hl, wl> &&lhs, matrix_sum<U, hr, wr> &&rhs);
};

template<typename T, unsigned h, unsigned w>
class matrix_sum : public matrix_expression<T> {
public:
    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_sum;
    template<typename T2, unsigned h2, unsigned w2>
    friend
    class matrix_product;

    matrix_wrap<T> resolve_all();

    operator matrix<T>();

    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>();

    unsigned get_height() const;

    unsigned get_width() const;

    matrix_sum(matrix_sum<T, h, w> &&X);

    matrix_sum(const matrix_sum<T, h, w> &X) = delete;

    ~matrix_sum() = default;

private:
    std::list<std::unique_ptr<matrix_expression<T>>> operands;

    matrix_sum() = default;

    void add(std::unique_ptr<matrix_expression<T>> &&mat);

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
};

template<typename T>
class matrix_singleton : public matrix_expression<T> {
public:
    template<class matrix_type>
    matrix_singleton(matrix_ref<T, matrix_type> m): singleton(m) {};

    matrix_wrap<T> resolve_all() {
        return this->singleton;
    }

    unsigned get_height() const {
        return this->singleton.get_height();
    }

    unsigned get_width() const {
        return this->singleton.get_width();
    }

    ~matrix_singleton() = default;

private:
    matrix_wrap<T> singleton;
};


#endif //ASSIGNMENT_3_MATRIX_EXPRESSION_H

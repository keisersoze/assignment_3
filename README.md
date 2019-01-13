Filippo Maganza (858532) and Sebastiano Smaniotto (857744)

Assignment 2: Matrix Operations

Due date: 19/12/2018 

## REQUIREMENT SPECIFICATION

Extend the matrix library (either the one presented or, if you feel confident and lucky, your own) to include the following features:

1. Compile-time matrix dimensions: provide a variant of the matrix that maintains compile-time information about height and width. Allow accessors to be bounds-checked at compile time and maintain compile-time size information as much as possible through matrix operators.

2. Matrix operations: provide matrix addition and (row-by-column) multiplication. Operations should interact with compile-time dimensions correctly (dimensions checked at compile-time).

When dealing with multiple multiplications, the library must perform the sequence of multiplications in the most computationally efficient order, i.e., by multiplying matrices contracting the largest dimension first. For example, let A, B, and C be 2x3, 3x5, and 5x2 matrices respectively, then the expression A*B*C should be computed as if it where A*(B*C), performing first the multiplication B*C since it is contracting the dimension 5, and then multiplying A by the result, contracting the smaller dimension 3.

## USAGE

The client is now able to use a specialized version of the `matrix<E>` class which takes two other parameter `matrix<E,H,W>` in the template which represents the height and the width of the matrix. By doing so the static information will be maintained throughout the templated decorator chain until the method `window(...)` will be invoked. The absence of a static window class does not allow to establish the matrix dimension at compile-time, and so the compile-time knowledge is lost.

Since the matrix specialization does not depend on a change in the template structure of `matrix_ref<T,type>` (being `type` both the placeholder for the type and the type decoration) then `matrix_wrap<E>` is oblivious to any change in the static variant, meaning that no change has been necessary and the client is free to pass a static `matrix_ref<T,type>` to any `matrix_wrap<E>` as in the previous assignment.

Operators for matrix sum and multiplication are now available. Their behaviour is described in their own Design section.

Below are presented all the operation offered by matrix classes.

### Examples

Sum some matrixes:

``` c++
matrix<float> res = A + C.transpose();
```

Multiply some matrices:

``` c++
matrix<int> res = A * B + B + B;
```

Sum some matrixes (with compile-time size):

``` c++
matrix<int,5,5> res = Z.transpose()+Z.transpose();
```

Multiply some matrixes (with compile-time size):

``` c++
matrix<int,2,2> res = A*Z*A*Z;
```

Sum and multiply matrices:
``` c++
matrix <int>  B = G + A.window({1,3,2,5}) * C.transpose() * D.transpose() * F + D.transpose().window({0,2,0,4})
```

Sum and multiply matrices (with compile-time size):
``` c++
matrix<int,4,4> res = A + A * B.transpose() + B + B ;
```

## DESIGN

In this section we explain our design for the project.
 
### Static matrices

We have defined a new generic template for the `matrix` class so as to be able to create a static specialization by allowing to the client to specify the dimensions of the matrix directly in the template, and added a new policy `CT_matrix<H,W>` to provide a variant to the `Plain` base `matrix_ref<T,type>` templated class where we can store static information at compile-time on the given matrix size. Such information is carried through the static decoration chain by the methods:

``` c++
static constexpr bool is_ct(); //return true if the matrix has compile time dimensions
```

``` c++
static constexpr unsigned get_ct_height(): //only available if is_ct()=true
```

``` c++
static constexpr unsigned get_ct_width(): //only available if is_ct()=true
```

Since no static variant to `window_spec` is provided, the static information on the dimensions is lost when the method `window(window_spec w)` is invoked .Using this information, we can check at compile-time wether the new static getters shall perform a bound check or delegate the access to the `operator()`.

The `matrix_wrap<E>` required no changes since the type knowledge on the wrapped matrix is restricted to the templated class `matrix_ref<E,type>` and so the wrapper is oblivious to the new features.


### Operators 

All operators have been defined externally from the classes. We have provided an overload for each possible combination of type of matrix (`matrix<E, ...>`, `matrix_ref<E,type>`, `matrix_wrap<E>`). If an expression contains either only sums or multiplications between compile-time size matrices, then the type of the returned matrix will also be determined at compile-time. If the expression contains a mix of sums and multiplications then our operators are able to determine the size at compile-time if all the matrices sizes are known at compile-time.

The `operator*` allows the client to multiply two matrices having the same type. A sequence of multiplication produces a `multiplication_proxy<E>` object which stores all the matrices converting them into a `matrix_wrap<E>` and the quasi-optimal multiplication order is decided at runtime using the largest-dimension compression heuristic. Similarly to the `matrix` class, `multiplication_proxy` comes in a variant for multiplications between matrices with dimensions known at compile-time, i.e. `matrix<E,H,W>`.

The sum can be performed using the `operator+` between any two matrices of type `T` and `U`, as long as the `operator+` is defined between two expressions having those two types, and returns a new matrix having as element type the type that one would get by summing two expressions of type `T` and `U`.

### Multiplication proxy

The role of the `multiplication_proxy` is to store the sequence of matrices in a chain of multiplications and suspend the evaluation of the expression until no more matrices are available. Its responsibility is to enforce the sub-optimal largest-dimension compression heuristic, and this process happens at runtime. Similarly to the `matrix` class, `multiplication_proxy` comes in a variant for multiplications between matrices with dimensions known at compile-time, i.e. `matrix<E,H,W>`.

//
// Created by Filippo Maganza on 2019-02-09.
//

#ifndef ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H
#define ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

#include"matrix.h"
#include"matrix_wrap.h"
#include"matrix_fwd.h"
#include <thread>

#define BLOCK_DIM 10

void row_multiply(window_spec window_spec1){

}

template <T,LType,Rtype>
void do_multiply(matrix_ref<T,LType> lhs, matrix_ref<T,Rtype> rhs) {

    for (int i = 0; i < lhs.get_height() ; i += BLOCK_DIM) {
        window_spec w();
        std::thread t(row_multiply({i, i + BLOCK_DIM, 0, lhs.get_width}))
    }

}

#endif //ASSIGNMENT_3_PARALLEL_MULTIPLICATION_H

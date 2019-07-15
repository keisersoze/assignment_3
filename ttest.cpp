//
// Created by Filippo Maganza on 2019-07-14.
//

#include <iostream>
#include "thread_pool.h"

int plus(const int x, const int y){
    return x + y ;
}

int main (){
    int i;
    std::cout << "Oks";
    ThreadPool threadPool(8);
    auto result = threadPool.enqueue(plus,2,2);
    // get result from future
    std::cout << result.get() << std::endl;
    return 1;

}


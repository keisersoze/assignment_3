//
// Created by Filippo Maganza on 2019-07-14.
//

#ifndef ASSIGNMENT_3_THREAD_POOL_H
#define ASSIGNMENT_3_THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#define N_THREAD 8

class ThreadPool {
public:
    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> enqueue(F &&f, Args &&... args);

    ~ThreadPool();

    static ThreadPool &getSingleton();

private:
    ThreadPool(size_t);

    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue, function<void()> cause in this case it's a "wrapper" for package_task making this
    // implementation of ThreadPool generic. The result will be aviable via std::future
    std::queue<std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    // used to wait for new tasks
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
        : stop(false) {
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back(
                [this] {
                    while (true) {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            // the thread wait until the queue ha been stopped or there's something to do
                            this->condition.wait(lock,
                                                 [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        task();
                    }
                }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&... args)
-> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        //from C++14
        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

inline ThreadPool &ThreadPool::getSingleton() {
    static ThreadPool singleton(N_THREAD);

    return singleton;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker: workers) {
        worker.join();
    }
}


#endif //ASSIGNMENT_3_THREAD_POOL_H

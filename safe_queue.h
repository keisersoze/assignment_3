//
// Created by Filippo Maganza on 2019-07-14.
//

#ifndef ASSIGNMENT_3_SAFE_QUEUE_H
#define ASSIGNMENT_3_SAFE_QUEUE_H

#pragma once

#include <mutex>
#include <queue>

// Thread safe implementation of a Queue using a std::queue
template <typename T>
class SafeQueue {
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
public:
    SafeQueue() {

    }

    SafeQueue(SafeQueue& other) {
        //TODO:
    }

    ~SafeQueue() {

    }


    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
    }

    bool dequeue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front());

        m_queue.pop();
        return true;
    }
};

#endif //ASSIGNMENT_3_SAFE_QUEUE_H

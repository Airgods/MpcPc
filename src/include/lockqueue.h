#pragma once
#include <queue>
#include <thread>
#include <mutex>              // pthread_mutex_t
#include <condition_variable> // pthread_condition_t

// 异步写日志的日志队列->实现在多个线程之间安全地传递日志数据
template<typename T>
class LockQueue
{
public:
    // 多个worker线程都会写日志queue  
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex); // 上锁保证线程安全
        m_queue.push(data);  // 用于将数据data推入队列
        m_condvariable.notify_one();   // 唤醒可能等待的线程,通知队列中有新的数据
    }
    
    // 一个线程读日志queue,写日志文件 
    T Pop()
    {   
        std::unique_lock<std::mutex> lock(m_mutex); // 解锁
        while(m_queue.empty())
        {
            // 日志队列为空,线程进入wait状态 
            m_condvariable.wait(lock);
        }

        T data = m_queue.front(); // 从队列中获取队首的元素,并将其赋值给data
        m_queue.pop();
        return data;
    }
private:    
    std::queue<T> m_queue;  // 用于存储队列的数据元素
    std::mutex m_mutex;     // 互斥锁,用于实现对队列的互斥访问,确保线程安全
    std::condition_variable m_condvariable;  // 条件变量,用于线程的等待和通知机制。在数据推入队列时,通过notify_one()来通知等待中的新数据可用
};
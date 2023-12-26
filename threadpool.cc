#include "threadpool.h"
#include <cassert>

ThreadPool::ThreadPool(size_t pool_size) 
    : is_shutdown_(false)
    , pool_size_(pool_size)
{
    for (size_t i = 0; i < pool_size_; ++i) {
        threads_.emplace_back(&ThreadPool::WorkerThread, this);
    }
}
ThreadPool::~ThreadPool(){
    Shutdown();
}
void ThreadPool::WorkerThread() {
    int i = 0;
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_); 
            condition_.wait(lock, [&i, this] { 
                return is_shutdown_ || !task_queue_.empty(); 
            });
            if (is_shutdown_ && task_queue_.empty()) {
                return;
            }
            task = std::move(task_queue_.front());
            task_queue_.pop();
        }
        task();
    }
}
// 关闭线程池
void ThreadPool::Shutdown() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        is_shutdown_ = true;
    }

    // 唤醒所有等待中的线程
    condition_.notify_all();

    // 等待所有工作线程结束
    for (auto& thread : threads_) {
        thread.join();
    }
}
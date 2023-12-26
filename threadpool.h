#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

class ThreadPool {
public:
    explicit ThreadPool(size_t pool_size);
    ~ThreadPool();
    // 添加任务到线程池   
    template<class F, class... Args>
    void AddTask(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            task_queue_.emplace(std::move(task));
        }
        condition_.notify_one();
    }
    void Shutdown();
private:
    void WorkerThread();
private:
    size_t pool_size_;
    bool is_shutdown_;
    std::mutex queue_mutex_;
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> task_queue_;
    std::condition_variable condition_;
};
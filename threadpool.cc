#include "threadpool.h"

static const size_t ExtendSize = 2;
static size_t DestorySize = 2;

ThreadPool::ThreadPool(size_t minthreadnum, size_t maxthreadnum) 
    : is_shutdown_(false)
    , min_thread_(minthreadnum)
    , max_thread_(maxthreadnum)
    , idle_thread_(0) 
    , work_thread_(0)
    , destory_thread_(0)
    , is_destory_(false)
{
    for (size_t i = 0; i < minthreadnum; ++i) {
        threads_.emplace_back(&ThreadPool::WorkerThread, this);
    }
    mananger_ = std::move(std::thread(&ThreadPool::ManagerThread, this));
}
ThreadPool::~ThreadPool(){
    Shutdown();
}
void ThreadPool::WorkerThread() {
    while (true) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        ++idle_thread_; 
        condition_.wait(lock, [this] { return is_destory_ || is_shutdown_ || !task_queue_.empty(); });
        --idle_thread_; 
        if(destory_thread_ > 0){
            --destory_thread_;
            return;
        }
        if (is_shutdown_ && task_queue_.empty()) {
            return;
        }
        auto task = std::move(task_queue_.front());
        task_queue_.pop();
        ++work_thread_;
        lock.unlock();  // 解锁互斥量，允许其他线程添加任务
        task();
        std::unique_lock<std::mutex> lock2(queue_mutex_);
        --work_thread_;
        lock2.unlock();
    }
}
void ThreadPool::ManagerThread() {
    while(!is_shutdown_) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if(task_queue_.size() > idle_thread_ && idle_thread_ < max_thread_) {
            for (size_t i = 0; i < ExtendSize && idle_thread_ < max_thread_; ++i, ++idle_thread_) {
                threads_.emplace_back(&ThreadPool::WorkerThread, this);
            }
        }
        lock.unlock();
        std::unique_lock<std::mutex> lock2(queue_mutex_);
        
        if (work_thread_ * 2 < idle_thread_ && idle_thread_ > min_thread_){
            destory_thread_ = DestorySize;
            for (int i = 0; i < DestorySize; ++i){
                condition_.notify_one();
            }
        }
        lock2.unlock();
        std::unique_lock<std::mutex> lock3(queue_mutex_);
        
        for(auto &thread : threads_) {
            if(thread.joinable())
                thread.join();
        }
        lock3.unlock();
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
    mananger_.join();
}
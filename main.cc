#include "threadpool.h"
#include <iostream>

// template<> ThreadPool::AddTask<std::function<void()>>();

int main() {
    // 创建动态线程池，最小线程数为2，最大线程数为10
    {
        ThreadPool pool(5);
        std::mutex coutmutex;
        // 示例：向线程池添加任务并执行
        for (int i = 0; i < 20; ++i) {
            auto func = [&](int id) {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                {   
                    std::unique_lock<std::mutex> lock(coutmutex);
                    std::cout << "Task " << id << " executed by thread " << std::this_thread::get_id() << std::endl;
                }
            };
            pool.AddTask(func, i);
        }

        // 等待片刻以观察任务执行情况
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return 0;
}
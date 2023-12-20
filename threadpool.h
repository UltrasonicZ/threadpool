#include <vector>
#include <thread>

class ThreadPool {
public:
    explicit ThreadPool();
    ~ThreadPool();
private:
    std::vector<std::thread> threads;
};


#include "threadpool.h"

int main()
{
    ThreadPool threadPool;
    threadPool.start(10);

    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
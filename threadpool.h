#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <thread>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <chrono>

//任务类
class Task
{
    public:
        Task();
        ~Task();
        //虚基类实现多态
        virtual void run() = 0;
    private:
};

//线程类型
enum Threadmode
{
    FIX_MODE,
    CACHED_MODE,
};

//线程类
class Thread
{
   using threadFunc = std::function<void()>;
    public:
        Thread(threadFunc func);
        ~Thread();
        void start();
    private:
    threadFunc func_;
};

//线程池类
class ThreadPool
{
    public:
    ThreadPool();
    ~ThreadPool();
    //设置模式
    void setMode(Threadmode mode);
    //开启线程池,并设置task上限
    void start(int inithread_num);
    //提交任务
    void submitTask(std::unique_ptr<Task> task);

    //禁止拷贝构造，赋值
    ThreadPool(const ThreadPool&)=delete;
    ThreadPool& operator=(const ThreadPool&)=delete;

    private:
    //将线程函数放在线程池中，从而能够实现访问线程池中的数据
    void threadFunc();

    private:
    //传智能指针给vector对象
    std::vector<std::unique_ptr<Thread>> thread_;
    //任务队列
    std::queue<std::unique_ptr<Task>> task_queue_;
    //初始线程个数
    size_t thread_num_;
    //线程类型
    Threadmode thread_mode_;
    //两个条件变量一个不空一个不满
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    //一把锁
    std::mutex queue_mutex_;
    //设置任务队列阈值
    std::atomic_int taskSize_;
};
#endif

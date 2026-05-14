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
#include <unordered_map>

//实现any类
class Any
{
public:
//默认构造函数
Any() = default;
~Any() = default;

//模板化，使得可以根据传入类型来决定赋值,使得可以接受任意数据类型的数据
template<typename T>
Any(T value):base_(std::make_shared<Derived<T>>(value))
{}
//将any对象中储存的数据提取出来
template<typename T>
T cast()
{
    Derived<T> * pd = dynamic_cast<Derived<T>*>(base_.get());
    if(pd == nullptr)
    {
        throw std::bad_cast();
    }
    return pd->data_;
}
private:
//定义一个基类
class Base
{
public:
//定义虚析构，使得继承类可以析构，防止内存泄露
virtual ~Base() = default; 

};
//定义派生类
template<typename T> 
class Derived : public Base
{
    public:
    //可以接受任意类型数据
    Derived(T data):data_(data)
    {}
    private:
    T data_;
};

private:
//定义一个指向基类的指针，用来存派生类的地址
std::shared_ptr<Base> base_;
};
//实现信号量类
class Semaphore
{
    public:
    //默认构造函数
    Semaphore() = default;
    //默认析构函数
    ~Semaphore() = default;
    //等待信号量
    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock,[&]()->bool{return count_>0;});
        count_--;
    }
    //信号量加一
    void post()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        count_++;
        cond_.notify_all();
    }
    private:
    int count_ = 0;
    std::mutex mtx_;
    std::condition_variable cond_;
};
class Result;
//任务类
class Task
{
    public:
        Task();
        ~Task();
        //虚基类实现多态
        virtual void run() = 0;

        void exec();
    private:
    Result * result_;
};
//result类型接收任务返回值
class Result
{
    public:
    Result(std::shared_ptr<Task> task,bool _set_value);
    ~Result()
    {}
    //获取任务返回值
    Any get();
    //设置任务返回值
    void set(Any any);
    private:
    //接收任务返回值
    Any any_;
    //信号量
    Semaphore sem_;
    //指向获取对象值的指针
    std::shared_ptr<Task> task_;
    //检查是否有返回值
    bool set_value_;
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
   using threadFunc = std::function<void(int)>;
    public:
        Thread(threadFunc func);
        ~Thread();
        void start();
        //获取id
        int getID() const;
    private:
    threadFunc func_;
    static int static_thread_id_;
    //记录id
    int thread_id_;
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
    //查看是否运行
    bool isRunning() const;

    //禁止拷贝构造，赋值
    ThreadPool(const ThreadPool&)=delete;
    ThreadPool& operator=(const ThreadPool&)=delete;

     //将线程函数放在线程池中，从而能够实现访问线程池中的数据
    void threadFunc(int id);

    private:
   
    private:
    //创建unordered_map对象，用来存储线程id和线程智能指针的映射关系，方便根据线程id来删除线程
    std::unordered_map<int,std::unique_ptr<Thread>> thread_map_;
       //设置运行状态
    std::atomic_bool running_;
    //任务队列
    std::queue<std::unique_ptr<Task>> task_queue_;
    //设置初始线程个数
    std::atomic_int initThreadNum_;
    //线程个数
    size_t thread_num_;
    //设置空闲线程个数
    std::atomic_int idleThreadNum_;
    //设置当前运行任务数
    std::atomic_int runningTaskNum_;
    //设置当前任务数量
    std::atomic_int taskNum_;
    //设置线程阈值
    std::atomic_int threadThreshold_;

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

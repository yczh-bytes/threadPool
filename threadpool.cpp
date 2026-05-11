#include "threadpool.h"

const int MAX_SIZE = 1024;
ThreadPool::ThreadPool():
    thread_num_(0),
    thread_mode_(FIX_MODE),
    taskSize_(MAX_SIZE)
{}

ThreadPool::~ThreadPool()
{}

void ThreadPool::start(int inithread_num)
{
    //设置初始线程数
    thread_num_ = inithread_num;

    //创建线程
    for(size_t i = 0;i<thread_num_;i++)
    {
        //将线程与线程函数绑定
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
       thread_.emplace_back(std::move(ptr));
    }

    //启动线程
    for(size_t i = 0;i<thread_num_;i++)
{
    thread_[i]->start();
}
}

void ThreadPool::submitTask(std::unique_ptr<Task> task)
{
    //获取锁
    std::unique_lock<std::mutex> lock(queue_mutex_);//unique_lock可以被wait释放锁，而lock_guard不能
    //线程通信，等待队列空余
    not_full_.wait(lock,[&](){return task_queue_.size()<taskSize_;});
    //有空余就放入队列中
    task_queue_.emplace(std::move(task));

    //有任务了，通知队列分配任务
    not_empty_.notify_all();
}

void ThreadPool::setMode(Threadmode mode)
{
    thread_mode_ = mode;
}

void Thread::start()
{
    //启动线程
    //调用线程函数
    std::thread t(func_);//创建线程对象
    t.detach();//分离线程

}
//线程函数，用于消费队列中的任务
void ThreadPool::threadFunc()
{
    //死循环，用于消费任务
    for(;;)
    {
        //创建任务智能指针，使得任务队列中的任务能够被多个线程共享
        std::shared_ptr<Task> task;
        //加一个括号，使得锁能在获取之后被释放，使得其他任务队列能够获取锁
    {
   //获取锁
   std::unique_lock<std::mutex> lock(queue_mutex_);
   //线程通信，等待队列有任务
   not_empty_.wait(lock,[&](){return !task_queue_.empty();});
   //从任务队列中取出任务
   task = std::move(task_queue_.front());
   task_queue_.pop();

   if(task_queue_.size()<taskSize_)
   {
        //有空余，通知队列分配任务
        not_empty_.notify_all();
   }
   {
   //唤醒其他线程，可以继续生产任务
   not_full_.notify_all();
    }
    if(task!=nullptr)
    {
   //当前线程执行这个任务
   task->run();
    }
    }
}
}

Thread::Thread(threadFunc func):
    func_(func)
{}

Thread::~Thread()
{}

Task::Task()
{}

Task::~Task()
{}
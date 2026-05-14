#include "threadpool.h"

const int MAX_SIZE = 1024;
const int MIX_TIME = 60;
ThreadPool::ThreadPool():
    thread_num_(0),
    thread_mode_(FIX_MODE),
    taskSize_(0),
    initThreadNum_(MAX_SIZE),
    running_(false),
    idleThreadNum_(0),
    runningTaskNum_(0),
    threadThreshold_(200)
{}

ThreadPool::~ThreadPool()
{
    running_ = false;
    //通知所有线程结束
    not_empty_.notify_all();
    if(thread_num_ == 0)
    {
        return;
    }
}

void ThreadPool::start(int inithread_num)
{
    if(running_)
    {
        return;
    }
    //设置初始线程数
    initThreadNum_ = inithread_num;
    //设置运行状态
    running_ = true;
    //创建线程
    for(size_t i = 0;i<initThreadNum_;i++)
    {
        //将线程与线程函数绑定
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this,std::placeholders::_1));
       thread_map_.emplace(ptr->getID(),std::move(ptr));
       //线程数量加一
       thread_num_++;
    }

    //启动线程
    for(auto& pair : thread_map_)
{
    pair.second->start();
}
}

void ThreadPool::setMode(Threadmode mode)
{
    if(running_)
    {
        return;
    }
    thread_mode_ = mode;
}

bool ThreadPool::isRunning() const
{
    return running_;
}

void ThreadPool::submitTask(std::unique_ptr<Task> task)
{
    if (task == nullptr || !running_)
    {
        return;
    }

    // 获取锁
    std::unique_lock<std::mutex> lock(queue_mutex_); // unique_lock可以被wait释放锁，而lock_guard不能
    // 线程通信，等待队列空余
    if(taskSize_ > 0)
    {
        not_full_.wait(lock, [&]() { return !running_ || task_queue_.size() < static_cast<size_t>(taskSize_.load()); });
    }
   
    if (!running_)
    {
        return;
    }
    // 有空余就放入队列中
    std::cout<<"放入任务"<<std::this_thread::get_id()<<std::endl;
    task_queue_.emplace(std::move(task));
    //任务数量加一
    runningTaskNum_++;
    //如果当前是缓存模式、没有空闲线程、线程数量还没超过上限，就临时创建新线程处理任务。
    if (thread_mode_ == CACHED_MODE &&
        idleThreadNum_ == 0 &&
        thread_num_ < static_cast<size_t>(threadThreshold_.load()))
    {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this,std::placeholders::_1));//预留出线程id
        int thread_id = ptr->getID();
        thread_map_.emplace(thread_id,std::move(ptr));
        //线程数量加一
        thread_num_++;
        thread_map_[thread_id]->start();
    }

    // 有任务了，通知队列分配任务
    not_empty_.notify_all();
}


void Thread::start()
{
    //启动线程
    //调用线程函数
    std::thread t(func_,thread_id_);//_创建线程对象
    t.detach();//分离线程
}
//线程函数，用于消费队列中的任务
void ThreadPool::threadFunc(int id)
{
    //死循环，用于消费任务
    for(;;)
    {
    
        //记录一下开始时间
        auto start = std::chrono::high_resolution_clock::now();
        //创建任务智能指针，使得任务队列中的任务能够被多个线程共享
        std::unique_ptr<Task> task;
        //加一个括号，使得锁能在获取之后被释放，使得其他任务队列能够获取锁
    {
   //获取锁
   std::unique_lock<std::mutex> lock(queue_mutex_);
   idleThreadNum_++;
   //如果在catch模式下，超过六十秒没有任务，就退出线程
   if(thread_mode_ == CACHED_MODE)
   {
        bool timeout = !not_empty_.wait_for(lock, std::chrono::seconds(60), [&]() {
            return !running_ || !task_queue_.empty();
        });
        if(timeout && thread_num_ > initThreadNum_)
        {
            thread_num_--;
            thread_map_.erase(id);
            return;
        }
   }
   else
   {   //线程通信，等待队列有任务
   not_empty_.wait(lock,[&](){return !running_ || !task_queue_.empty();});}

    idleThreadNum_--;
   if(!running_)
   {
       return;
   }
   if(task_queue_.empty())
   {
       continue;
   }
   //从任务队列中取出任务
   task = std::move(task_queue_.front());
   task_queue_.pop();
   runningTaskNum_++;
   not_full_.notify_all();
   }
    if(task!=nullptr)
    {
   //当前线程执行这个任务
   task->exec();
    }
    runningTaskNum_--;
    }
}

 int Thread::static_thread_id_ = 0;

Thread::Thread(threadFunc func):
    func_(func),
    thread_id_(static_thread_id_++)
   {}

Thread::~Thread()
{}

int Thread::getID() const
{
    return thread_id_;
}

Task::Task()
{}

Task::~Task()
{}

 Result::Result(std::shared_ptr<Task> Task,bool _set_value):
        task_(Task),
        set_value_(_set_value)
    {}

Any Result::get()
{
    sem_.wait();
    return any_;
}

void Result::set(Any any)
{
    this->any_ = any;
    sem_.post(); 
}

void Task::exec()
{
        run();
    
}
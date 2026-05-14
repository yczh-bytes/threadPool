#include "threadpool.h"

class Mytask:public Task
{
    public:
    Mytask(int id):id_(id){}

    void run() override
    {
        std::cout<<"任务"<<id_<<"开始执行"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout<<"任务"<<id_<<"执行完成"<<std::endl;
    }
    private:
    int id_;
};

class Sumtask : public Task
{
    public:
    Sumtask(int a,int b,int id):a_(a),b_(b),id_(id){}
    void run() override
    {
        int result = a_ + b_;
        std::cout<<"Sumtask"<<id_<<":"<<a_<<"+"<<b_<<"="<<result<<std::endl;
    }
    private:
    int a_;
    int b_;
    int id_;
};

int main()
{
    std::cout<<"测试线程池"<<std::endl;
    ThreadPool threadPool;
   
    std::cout << "设置线程池模式为FIXED_MODE" << std::endl;
    threadPool.setMode(FIX_MODE);
    threadPool.start(4);

    for(int i=0;i<20;i++)
    {
        auto task = std::make_unique<Mytask>(i);
        threadPool.submitTask(std::move(task));
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));

    for(int i=0;i<10;i++)
    {
        auto task = std::make_unique<Sumtask>(i,i*2,i);
        threadPool.submitTask(std::move(task));
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "\n=== 测试CACHED模式 ===" << std::endl;
    ThreadPool cachedPool;
    cachedPool.setMode(CACHED_MODE);
    cachedPool.start(5);

    for(int i=0;i<15;i++)
    {
        auto task = std::make_unique<Mytask>(i);
        cachedPool.submitTask(std::move(task));
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    

    return 0;
}
#ifndef THREAD_H
#define THREAD_H

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <thread>

class Thread;
class TaskThread;
class ThreadPool;

class Thread
{
public:
    explicit Thread();
    virtual void start();
    virtual ~Thread();

protected:
    virtual void run() = 0;
    std::thread internal_thread;
};

struct ThreadStore
{
};

typedef std::function<void(ThreadStore &)> Task;

class TaskThread : public Thread
{
public:
    explicit TaskThread(ThreadPool &pool, ThreadStore *thread_store);

protected:
    std::future<Task> future_task;
    void run() final;
    ThreadStore *thread_store;
    ThreadPool &pool;
};

class ThreadPool : public Thread
{
public:
    explicit ThreadPool(uint thread_count,
                        std::function<ThreadStore *()> thread_store_factory);
    void push_task(Task &&task);
    std::future<Task> push_free_thread();

    virtual void start() override;

protected:
    virtual void run() override;
    uint thread_count;
    std::mutex new_task_mutex, free_thread_mutex;
    std::condition_variable new_task_condition, free_thread_condition;
    std::deque<std::promise<Task>> task_promises;
    std::deque<Task> tasks;
    TaskThread **threads;
};

#endif //THREAD_H
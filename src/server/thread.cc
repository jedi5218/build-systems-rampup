#include "thread.h"

Thread::Thread()
{
}

void Thread::start()
{
    internal_thread = std::move(std::thread(&Thread::run, this));
}

Thread::~Thread()
{
}

TaskThread::TaskThread(ThreadPool &pool, ThreadStore *thread_store) : pool(pool),
                                                                      thread_store(thread_store)
{
}

void TaskThread::run()
{
    //printf("Thread 0x%x started\n", std::this_thread::get_id());
    for (;;)
    {
        future_task = pool.push_free_thread();
        //printf("Thread 0x%x generated a promise\n", std::this_thread::get_id());
        auto task = future_task.get();
        //printf("Thread 0x%x recieved a task!\n", std::this_thread::get_id());
        task(*thread_store);
    }
}

ThreadPool::ThreadPool(
    uint thread_count,
    std::function<ThreadStore *()> thread_store_factory = []() { return new ThreadStore(); }) : thread_count(thread_count)
{
    threads = new TaskThread *[thread_count];
    for (uint i = 0; i < thread_count; i++)
    {
        threads[i] = new TaskThread(*this, thread_store_factory());
    }
}

void ThreadPool::start()
{
    for (uint i = 0; i < thread_count; i++)
    {
        threads[i]->start();
    }
    Thread::start();
}

void ThreadPool::push_task(Task &&task)
{
    std::lock_guard<std::mutex> guard(new_task_mutex);
    tasks.push_back(task);
    new_task_condition.notify_all();
}

void ThreadPool::run()
{
    for (;;)
    {

        {
            std::unique_lock<std::mutex> guard(new_task_mutex);
            while (tasks.empty())
            {
                new_task_condition.wait(guard); //, [this]() -> bool { !this->tasks.empty(); });
            }
        }
        {
            std::unique_lock<std::mutex> guard(free_thread_mutex);
            while (task_promises.empty())
            {
                free_thread_condition.wait(guard); //, [this]() -> bool { !this->task_promises.empty(); });
            }
        }
        auto task = tasks.front();
        task_promises.front().set_value(task);
        tasks.pop_front();
        task_promises.pop_front();
    }
}

std::future<Task> ThreadPool::push_free_thread()
{
    std::lock_guard<std::mutex> guard(free_thread_mutex);
    task_promises.push_back(std::promise<Task>());
    free_thread_condition.notify_all();
    return task_promises.back().get_future();
}

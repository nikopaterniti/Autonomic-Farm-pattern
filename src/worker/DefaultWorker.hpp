#ifndef DEFAULT_WORKER_HPP
#define DEFAULT_WORKER_HPP

#include <functional>
#include <vector>
#include <iostream>
#include "../worker/IWorker.hpp"
#include "../master/WorkerPool.hpp"
#include "../common/Flags.hpp"

using namespace std;

template <typename TIN, typename TOUT>
class DefaultWorker : public IWorker
{

/*
The workers run in their own thread. 
When the WorkerPool asssign a task to a worker it sets the value for the task and notify the condition variable that awake the worker. Once the work is done the task is set to null again waiting for the next.
*/


private:
    // Wait until the task to compute is non-null
    void wait_for_task()
    {
        unique_lock<mutex> lock(this->task_mutex);
        if (task_ == nullptr)
            this->task_condition.wait(lock, [&] { return this->task_ != nullptr; });
    }

protected:
    // The mutex used to freeze this worker
    mutex task_mutex;

    // The condition that indicates whiter this workers has a non-null task or not
    condition_variable task_condition;

    // The task to be computed
    TIN *task_ = nullptr;

    // The pool that spawned me
    WorkerPool<TIN, TOUT> *pool_;

    // The function to compute
    function<TOUT *(TIN *)> func_;

    void run() override
    {
        bool eos = false;
        while (!eos)
        {
            wait_for_task();

            if (task_ == (TIN *)END_OF_STREAM)
            {
                eos = true;
                pool_->collect(this, (TOUT *)END_OF_STREAM);
            }
            else
            {
                // Compute result
                auto result = func_(task_);

                // Set to null
                task_ = nullptr;

                // Notify to pool
                pool_->collect(this, result);
            }
        }
    }

public:
    DefaultWorker(WorkerPool<TIN, TOUT> *pool, function<TOUT *(TIN *)> func) : pool_(pool), func_(func)
    {
    }

    // Give a task to this workers and unfreeze it
    void accept(TIN *task)
    {
        unique_lock<mutex> lock(this->task_mutex);
        task_ = task;

        // unlock the wait_for_task condition
        this->task_condition.notify_one();
    }
};

#endif
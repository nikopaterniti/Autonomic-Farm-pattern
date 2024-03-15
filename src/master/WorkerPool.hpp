#ifndef WORKER_queue_HPP
#define WORKER_queue_HPP

#include "MasterWorker.hpp"
#include "../worker/DefaultWorker.hpp"
#include "../common/ThreadSafeQueue.hpp"
#include "../common/Flags.hpp"
#include "IPool.hpp"
#include <vector>
#include <iostream>
#include <thread>
#include <cassert>

using namespace std;

/*
It runs on its own thread and spawns the initial number of workers requested by the user and adds them in a queue of available workers.
When the MaterWorker request the pool to assing the next task it pops an element from the queue(if available) otherwise it waits.
When a worker finishes his task it gets added again to the queue and the Master notifies this event.


*/

#pragma region Template declarations

// MasterWorker
template <typename TIN, typename TOUT>
class MasterWorker;

// DefaultWorker
template <typename TIN, typename TOUT>
class DefaultWorker;

#pragma endregion Template declarations

template <typename TIN, typename TOUT>
class WorkerPool : public IWorker, public IPool
{

private:
    // The initial value of concurrency
    int initial_nw_;

    // The total amount of spawned workers
    int total_spawned_workers = 0;

    // The max number of threads this machine supports (max nw)
    int max_concurrency;

    // The queue of commands received from the monitor of this pool
    ThreadSafeQueue<int> monitor_commands;

    // The function this pool workers have to compute
    function<TOUT *(TIN *)> func_;

    // Available workers pool
    ThreadSafeQueue<DefaultWorker<TIN, TOUT> *> available_workers_pool_;

    // Freezed workers pool
    ThreadSafeQueue<DefaultWorker<TIN, TOUT> *> waiting_workers_pool_;

    // The master worker that own this pool
    MasterWorker<TIN, TOUT> *master_;

    // The mutex used to lock the pool until all threads have joined
    mutex join_all_mutex;

    // The condition variable used to wait until all threads have completed (also by EOS)
    condition_variable all_joined_condition;

public:
    WorkerPool(MasterWorker<TIN, TOUT> *master, int nw, function<TOUT *(TIN *)> func) : master_(master), func_(func), initial_nw_(nw)
    {
        max_concurrency = thread::hardware_concurrency();
        for (auto i = 0; i < initial_nw_; i++)
        {
            auto worker = new DefaultWorker<TIN, TOUT>(this, func_);
            available_workers_pool_.push(worker);
            worker->start();
            total_spawned_workers++;
        }
        this->start(); // start run() method in separate thread
    }

    // Assign a task to free worker or waits until one is available.
    void assign(TIN *task)
    {
        // pop a free worker or wait
        auto worker = available_workers_pool_.pop();
        available_workers_pool_.notify();

        // give it a task
        worker->accept(task);
    }

    // Collect the result from a worker and make it availabe again to receive another task.
    void collect(DefaultWorker<TIN, TOUT> *worker, TOUT *result)
    {
        if (result != (TOUT *)END_OF_STREAM)
        {
            available_workers_pool_.push(worker);
            available_workers_pool_.notify();
            master_->collect(result);

            // notify the condition that might be waiting for remaining workers
            all_joined_condition.notify_one(); 
        }
    }

    // Joins all workers and return their number
    int join_all()
    {
        unique_lock<mutex> lock(this->join_all_mutex);

        // send the EOS to the command thread (run() method)
        this->notify_command(END_OF_STREAM);
        // join the command thread
        this->join();

        int available_pool_size = available_workers_pool_.size();
        int waiting_pool_size = waiting_workers_pool_.size();

        // wait until all workers that were computing
        // gets collected and pushed again in the available_workers_pool_
        if (available_pool_size != (total_spawned_workers - waiting_pool_size))
            this->all_joined_condition.wait(lock, [&] {
                available_pool_size = available_workers_pool_.size();
                return available_pool_size == (total_spawned_workers - waiting_pool_size);
            });

        int total_oes_sent = 0;

        if (waiting_pool_size > 0)
        {
            // join all the waiting workers
            auto workers = waiting_workers_pool_.pop_all();
            for (auto worker : workers)
            {
                worker->accept((TIN *)END_OF_STREAM);
                worker->join();
                total_oes_sent++;
            }
        }

        auto workers = available_workers_pool_.pop_all();
        for (auto worker : workers)
        {
            // join all available workers
            worker->accept((TIN *)END_OF_STREAM);
            worker->join();
            total_oes_sent++;
        }

        assert(total_spawned_workers == total_oes_sent);

        return total_oes_sent;
    }

    int get_actual_workers_number() override
    {
        int waiting_pool_size = waiting_workers_pool_.size();

        return total_spawned_workers - waiting_pool_size;
    }

    // Notify a command for the commands thread
    void notify_command(int cmd) override
    {
        monitor_commands.push(cmd);
        monitor_commands.notify();
    }

    // Move a worker from the available_workers_pool_ to the waiting_workers_pool_
    void remove_worker()
    {

        if (get_actual_workers_number() < 2)
            return;

        auto worker = available_workers_pool_.pop();
        waiting_workers_pool_.push(worker);
        waiting_workers_pool_.notify();
    }

    // Move a worker from the waiting_workers_pool_ to the available_workers_pool_
    // if possible, otherwise spawns a new worker and pushes it to the available_workers_pool_
    void add_worker()
    {
        if (!waiting_workers_pool_.is_empty())
        {
            auto worker = waiting_workers_pool_.pop();
            available_workers_pool_.push(worker);
        }
        else
        {
            if (total_spawned_workers >= max_concurrency) // do not spawn more, uselss
                return;

            auto worker = new DefaultWorker<TIN, TOUT>(this, func_);
            available_workers_pool_.push(worker);
            worker->start();
            total_spawned_workers++;
        }
        available_workers_pool_.notify();
    }

    // The commands thread, which listens for commands from the monitor
    // and add/remove a workers to/from this pool
    void run() override
    {
        bool eos = false;
        while (!eos)
        {
            int cmd = monitor_commands.pop();
            if (cmd == END_OF_STREAM)
                eos = true;
            else if (FlagUtils::is(cmd, ADD_WORKER))
                add_worker();
            else if (FlagUtils::is(cmd, REMOVE_WORKER))
                remove_worker();
        }
    }
};

#endif
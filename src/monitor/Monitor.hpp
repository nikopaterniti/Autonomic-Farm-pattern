#ifndef MONITOR_HPP
#define MONITOR_HPP

#include "../master/IPool.hpp"
#include "../common/Flags.hpp"
#include "../common/Constants.hpp"
#include "IStrategy.hpp"
#include "DefaultStrategy.hpp"
#include <iostream>

// Refresh the throughput every 10 milliseconds
#define MONITOR_THROUGHPUT_WINDOW 5.0

using namespace std;

class Monitor
{
private:
    // The pool to monitor
    IPool *pool_;

    // The number of task collected in the current interval
    int task_collected = 0;

    // The total number of task collected since the monitor has been started
    int total_collected_task = 0;

    // The throughput in the previous window
    float prev_throughput_ = 0;

    // The expected throughput from the user
    float expected_throughput_;

    // The strategy that decides when to add or remove a worker
    IStrategy *strategy;

    // The mutex used to ensure the collect is called thread-safe
    mutex notify_mutex;

    // The starting time of the current window
    chrono::high_resolution_clock::time_point monitor_start;

    chrono::high_resolution_clock::time_point time_zero;
public:
    Monitor(IPool *pool, float expected_throughput) : pool_(pool), expected_throughput_(expected_throughput)
    {
        strategy = new DefaultStrategy(expected_throughput);
    }

    void init()
    {
        monitor_start = time_zero = chrono::high_resolution_clock::now();
        cout << "time,tasks,expected_throughput,actual_throughput,nw" << endl;
    }

    // Called every time a task has been collected
    void notify()
    {
        unique_lock<mutex> lock(this->notify_mutex);
        task_collected++;
        total_collected_task++;

        auto now = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::microseconds>(now - monitor_start).count() / 1000.0;

        float actual_throughput = prev_throughput_;

        if (elapsed >= MONITOR_THROUGHPUT_WINDOW)
        {
            // compute throughput
            actual_throughput = task_collected / elapsed;
            prev_throughput_ = actual_throughput;
            task_collected = 0;
            monitor_start = chrono::high_resolution_clock::now();
        }

        auto actual_workers_number = pool_->get_actual_workers_number();
        auto cmd = strategy->get(actual_throughput, actual_workers_number);

        auto elapsed_from_time_zero = chrono::duration_cast<std::chrono::microseconds>(now - time_zero).count();
        cout.precision(2);
        cout << std::fixed << elapsed_from_time_zero << "," << total_collected_task << "," << expected_throughput_ << "," << actual_throughput << "," << actual_workers_number << "\n";

        if (FlagUtils::is(cmd, ADD_WORKER) || FlagUtils::is(cmd, REMOVE_WORKER))
        {
            if (FlagUtils::is(cmd, MULTIPLE_COMMANDS))
            {
                // notify multiple times
                int cmd_number = FlagUtils::is(cmd, DOUBLE_COMMAND) ? 2 : 4;
                for (auto i = 0; i < cmd_number; i++)
                    pool_->notify_command(cmd);
            }
            else
            {
                pool_->notify_command(cmd);
            }
        }
    }
};

#endif
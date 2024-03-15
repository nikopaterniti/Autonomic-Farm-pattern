#ifndef DEFAULT_STRATEGY_HPP
#define DEFAULT_STRATEGY_HPP

#include "IStrategy.hpp"
#include "../common/Flags.hpp"
#include "../common/Constants.hpp"
#include <vector>
#include <numeric>
#include <cmath>

using namespace std;
/*
The monitor receives a notification every time a DefaultWorker completes its current task, keeping a counter of the task collected so far.
Once the next notification comes the Monitor checks if 5ms are passed from the previous call, if so computes the current throughput otherwise it considers it equal to the previous one.
So it provides the current throughput and the actual number of workers in the farm.

The DefaultStrategy has a window of throughputs that is filled at every invocation by the Monitor, once the window is full(some throughputs have been collected), it computes 2 measures:
-avg of the window
-slope of the regression between the window's elements.
Based on this indicators it decides if the farm should adjust the number of actual workers.
If slope>some threshold it could remove/add more than one worker.
Then returns an iteger to the Monitor representing the commands the Monitor should communicate to the WorkerPool.
*/

// The default strategy for the monitor
class DefaultStrategy : public IStrategy
{

private:
    // The throughput to achieve
    float expected_throughput_;

    // The slope threshold (does not depend on the expected_throughput_)
    float slope_threshold_;

    // The average threshold (does depend on the expected_throughput_)
    float avg_threshold_;

    // The window containing past throughputs
    vector<float> current_window_;

    // Add the actual throughput tho the window. Returns true if the winodw is full, false otherwise
    bool add_to_window(float actual_throughput)
    {
        if (current_window_.size() < STRATEGY_WINDOW_SIZE)
            current_window_.push_back(actual_throughput);

        return current_window_.size() >= STRATEGY_WINDOW_SIZE;
    }

    // Calculate the slope for the current window (ignore the intercept term).
    float get_slope()
    {
        // our window is a set of points where
        // the throughput is on the y axis while the time
        // (index of the array) is on the x axis

        auto size = current_window_.size();

        // fill the x array with values 0 to size - 1
        vector<int> x_window(size);
        iota(x_window.begin(), x_window.end(), 0);

        // calculate average
        float avgX = accumulate(x_window.begin(), x_window.end(), 0.0) / size;
        float avgY = accumulate(current_window_.begin(), current_window_.end(), 0.0) / size;

        float numerator = 0.0;
        float denominator = 0.0;

        for (int i = 0; i < size; ++i)
        {
            // https://en.wikipedia.org/wiki/Simple_linear_regression
            numerator += (x_window[i] - avgX) * (current_window_[i] - avgY); // \sum (x_i - x_avg)
            denominator += (x_window[i] - avgX) * (x_window[i] - avgX);      // \sum (y_i - y_avg)
        }

        if (denominator == 0.0)
            return 0;

        return numerator / denominator;
    }

    // Returns the average of the current window
    float get_average()
    {
        return accumulate(current_window_.begin(), current_window_.end(), 0.0) / current_window_.size();
    }

    bool is_in_average(float avg)
    {
        return avg >= expected_throughput_ - avg_threshold_ &&
               avg <= expected_throughput_ + avg_threshold_;
    }

    bool is_above_average(float avg)
    {
        return avg > expected_throughput_ + avg_threshold_;
    }

    bool is_under_average(float avg)
    {
        return avg < expected_throughput_ - avg_threshold_;
    }

    bool is_costant_slope(float slope)
    {
        //cout<<slope<<endl;
        float abs_slope = abs(slope);
        return abs_slope <= slope_threshold_;
    }

    bool is_positive_slope(float slope)
    {
        //cout<<slope<<endl;
        return slope > 0 && slope > slope_threshold_;
    }

    bool is_negative_slope(float slope)
    {
        //cout<<slope<<endl;
        return slope < 0 && slope < -slope_threshold_;
    }

public:
    DefaultStrategy(float expected_throughput) : expected_throughput_(expected_throughput)
    {
        this->slope_threshold_ = SLOPE_THRESHOLD;
        this->avg_threshold_ = AVERAGE_THROUGHPUT_THRESHOLD * expected_throughput_;
    }

    // Retruns a command represented as a integer flag containing the instruction
    // for the pool (add/remove worker) every time the window gets filled.
    // If the window is not filled yet, the NONE command is returned (do not add/remove workers).
    int get(float actual_throughput, int actual_workers_number) override
    {
        auto cmd = NONE;
        if (add_to_window(actual_throughput))
        {
            // window is full here

            auto slope = get_slope();
            auto average = get_average();
            
            //cout<<slope<<endl;
            //cout<<average<<endl;

            if (!is_in_average(average))
            {
                // could be either above or under average, we need to do something

                if (is_costant_slope(slope))
                {
                    // the slope does not tell anything here, decide only on the average param
                    if (is_under_average(average))
                        cmd = ADD_WORKER;
                    else if (is_above_average(average))
                        cmd = REMOVE_WORKER;      
                }
                                                       
                else if (is_negative_slope(slope))
                {
                    // the throughput is decreasing, but double check with average before adding a worker
                    if (is_under_average(average))
                    {
                        cmd = ADD_WORKER;
                        if (slope <= -QUAD_COMMAND_SLOPE_THRESHOLD) // the slope is very leaning, add 4 workers
                            cmd = FlagUtils::combine(cmd, MULTIPLE_COMMANDS, QUAD_COMMAND);
                        else if (slope <= -DOUBLE_COMMAND_SLOPE_THRESHOLD) // the slope is quite leaning, add 2 workers
                            cmd = FlagUtils::combine(cmd, MULTIPLE_COMMANDS, DOUBLE_COMMAND);
                    }
                }
                else if (is_positive_slope(slope))
                {
                    // the throughput is increasing
                    if (is_above_average(average))
                    {
                        cmd = REMOVE_WORKER;

                        if (slope >= QUAD_COMMAND_SLOPE_THRESHOLD) // the slope is very leaning, remove 4 workers
                            cmd = FlagUtils::combine(cmd, MULTIPLE_COMMANDS, QUAD_COMMAND);
                        else if (slope >= DOUBLE_COMMAND_SLOPE_THRESHOLD)
                            cmd = FlagUtils::combine(cmd, MULTIPLE_COMMANDS, DOUBLE_COMMAND);
                    }
                }
            }

            // use bitmask to notify more than one command
            cmd = FlagUtils::combine(cmd, WINDOW_FULL);

            current_window_.clear();
        }

        return cmd;
    };
};

#endif
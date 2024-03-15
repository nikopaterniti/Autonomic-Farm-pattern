#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>

#ifndef TIMER_HPP
#define TIMER_HPP

class Timer
{
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
    std::string message;
    using usecs = std::chrono::microseconds;
    using msecs = std::chrono::milliseconds;

public:
    Timer(const std::string m) : message(m)
    {
        start = std::chrono::system_clock::now();
    }

    ~Timer()
    {
        stop = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = stop - start;
        auto musec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        std::cout << message << " computed in " << musec << " ms " << std::endl;
    }
};

#endif
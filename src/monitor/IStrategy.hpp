#ifndef I_STRATEGY_HPP
#define I_STRATEGY_HPP

// The strategy for the monitor
class IStrategy
{
public:
    
    virtual int get(float actual_throughput, int actual_workers_number) = 0;
};

#endif
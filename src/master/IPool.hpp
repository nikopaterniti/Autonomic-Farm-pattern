#ifndef I_IPOOL_HPP
#define I_IPOOL_HPP

class IPool
{
public:
    // The monitor invokes this method every time the window is filled
    virtual void notify_command(int command) = 0;
    virtual int get_actual_workers_number() = 0;
};

#endif
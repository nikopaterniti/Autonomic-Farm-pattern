
#ifndef FLAGS_HPP
#define FLAGS_HPP

#define END_OF_STREAM -1
#define ADD_WORKER 1
#define REMOVE_WORKER 2
#define NONE 4
#define WINDOW_FULL 8
#define MULTIPLE_COMMANDS 16
#define DOUBLE_COMMAND 32
#define QUAD_COMMAND 64

class FlagUtils
{
public:
    static bool is(int value, int flag)
    {
        return (value & flag) == flag;
    }

    static int combine(int value, int flag)
    {
        return (value | flag);
    }

    static int combine(int value, int flag, int flag1)
    {
        return (value | flag | flag1);
    }
};

#endif
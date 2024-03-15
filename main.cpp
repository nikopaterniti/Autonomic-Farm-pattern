#include <iostream>
#include "src/master/MasterWorker.hpp"
#include "src/master/WorkerPool.hpp"
#include "src/emitter/DefaultEmitter.hpp"
#include "src/common/Timer.hpp"
#include "src/common/inputs.hpp"
#include "src/common/InputType.hpp"

using namespace std;

int *func(int *x)
{
    auto start = chrono::high_resolution_clock::now();
    auto end = false;
    while (!end)
    {
        auto elapsed = chrono::high_resolution_clock::now() - start;
        auto ms = chrono::duration_cast<chrono::microseconds>(elapsed).count();
        if (ms >= *x * 1000) // x milliseconds
            end = true;
    }
    return x;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "\nUsage is " << argv[0] << " nw throughput [inputType]\n\n";
        cout << "Input type could be: " << endl;
        cout << "\t- 1 = 4L 1L 8L \t(default)" << endl;
        cout << "\t- 2 = 4L 4L 4L \t(constant)" << endl;
        cout << "\t- 3 = 8L 1L 4L \t(reverse default)" << endl;
        cout << "\t- 4 = 1L 8L \t(low high)" << endl;
        cout << "\t- 5 = 8L 1L \t(high low)" << endl;
        return 0;
    }

    int nw = atoi(argv[1]);
    float expected_throughput = atof(argv[2]);

    auto input_vec = get_default();

    if (argc == 4)
    {
        // input provided
        int input_type = atoi(argv[3]);
        switch (input_type)
        {
        case InputType::Constant:
            input_vec = get_constant();
            break;
        case InputType::ReverseDefault:
            input_vec = get_reverse_default();
            break;
        case InputType::LowHigh:
            input_vec = get_lowhigh();
            break;
        case InputType::HighLow:
            input_vec = get_highlow();
            break;
        }
    }

    auto emitter = new DefaultEmitter<int>(input_vec);
    auto master = new MasterWorker<int, int>(emitter, nw, &func, expected_throughput);

    Timer t("Farm");
    auto results = master
                       ->run()
                       ->get_results();

    cout << "Count: " << results->size() << endl;
}
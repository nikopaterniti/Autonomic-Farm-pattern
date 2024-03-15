#ifndef COLLECTOR_HPP
#define COLLECTOR_HPP

#include <vector>
#include <mutex>

using namespace std;

//The collector uses the collect method called from the MasterWorker every time a task is completed

// Uses an array as data container
template <typename TOUT>
class Collector
{
private:
    // The data container
    vector<TOUT *> vector_;

    // The mutex used to ensure the collect is called thread-safe
    mutex collect_mutex;

public:

    void collect(TOUT *result)
    {
        unique_lock<mutex> lock(this->collect_mutex);
        vector_.push_back(result);
    }

    vector<TOUT *> *get_results()
    {
        return new vector(vector_);
    }
};

#endif
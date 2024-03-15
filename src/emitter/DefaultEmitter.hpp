#ifndef DEFAULT_EMITTER_HPP
#define DEFAULT_EMITTER_HPP

#include "IEmitter.hpp"
#include <vector>

using namespace std;


//The Default Emitter generates a new task every time the MasterWorker requests it.

// Uses an array as data container
template <typename TIN>
class DefaultEmitter : public IEmitter<TIN>
{
private:
    // The data container
    vector<TIN *> *vector_;

    // The current index
    int index_;

public:
    // Creates a new DefaultEmitter with the provided data
    DefaultEmitter(vector<TIN *> *vector)
    {
        vector_ = vector;
        index_ = 0;
    }

    // Retuns the next item in the collection or nullptr
    TIN *get_next()
    {
        if (index_ < vector_->size())
        {
            // There is more data in the array, return the current index position
            // and increase the index to the next position
            return vector_->at(index_++);
        }

        return nullptr;
    }
};

#endif
#ifndef EMITTER_HPP
#define EMITTER_HPP

// The emitter of the input collection
template <typename TIN>
class IEmitter
{
public:
    // Retruns the next item that has to be given as input to the input function
    virtual TIN* get_next() = 0;
};

#endif
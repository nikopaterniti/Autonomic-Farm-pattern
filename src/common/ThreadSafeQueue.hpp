#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <deque>
#include <thread>

using namespace std;
using namespace std::literals::chrono_literals;

template <typename T>
class ThreadSafeQueue
{
private:
    mutex mutex_;
    condition_variable not_empty_condition_;
    deque<T> queue_;

public:
    ThreadSafeQueue() {}

    void push(T const &value)
    {
        std::unique_lock<std::mutex> lock(this->mutex_);
        this->queue_.push_front(value);
    }

    // Notify that the size of the queue has changed
    void notify()
    {
        this->not_empty_condition_.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(this->mutex_);
        this->not_empty_condition_.wait(lock, [=] { return !this->queue_.empty(); });

        auto value = this->queue_.back();
        this->queue_.pop_back();

        return value;
    }

    bool is_empty()
    {
        std::unique_lock<std::mutex> lock(this->mutex_);
        return this->queue_.empty();
    }

    vector<T> pop_all()
    {
        unique_lock<mutex> lock(this->mutex_);

        vector<T> vec_to_return;

        this->not_empty_condition_.wait(lock, [=] { return !this->queue_.empty(); });

        int size = this->queue_.size();
        for (int i = 0; i < size; i++)
        {
            auto value = this->queue_.front();
            this->queue_.pop_front();
            vec_to_return.push_back(value);
        }
        return vec_to_return;
    }

    int size()
    {
        std::unique_lock<std::mutex> lock(this->mutex_);
        int size = this->queue_.size();
        return size;
    }
};

#endif
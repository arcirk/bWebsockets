#pragma once

#include <future>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

class thread_pool
{
public:

    thread_pool(std::size_t threads, std::size_t capacity)
    : threads(threads)
    , queue  (capacity)
    {
        reset();
    }

    template <typename F>
    void post(F&& f)
    {
        queue.wait();

        boost::asio::post(*workers, [this, f = std::forward<F>(f)]
        {
            f();
            queue.post();
        });
    }

    template <typename F>
    auto submit(F&& f) -> std::future<decltype(f())>
    {
        queue.wait();

        std::promise<decltype(f())> promise;
        auto future = promise.get_future();
        boost::asio::post(*workers, [this, promise = std::move(promise), f = std::forward<F>(f)] () mutable
        {
            promise.set_value(f());
            queue.post();
        });
        return future;
    }

    void wait()
    {
        workers->wait();
    }

    void reset()
    {
        if(workers)
        {
            wait();
            workers.reset();
        }
        workers = std::make_unique<boost::asio::thread_pool>(threads);
    }

private:

    std::size_t const threads;
    std::unique_ptr<boost::asio::thread_pool> workers;
    boost::interprocess::interprocess_semaphore queue;
};

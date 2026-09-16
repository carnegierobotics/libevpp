/**
 * @file loop.hh
 *
 * C++ wrapper around a libev Loop
 *
 * Copyright 2021
 * Carnegie Robotics, LLC
 * 4501 Hatfield Street, Pittsburgh, PA 15201
 * https://www.carnegierobotics.com
 *
 * Significant history (date, user, job code, action):
 *   2022-08-23, jlyda@carnegierobotics.com, <2051.01.4>, Created file.
 *   2024-04-18, jlyda@carnegierobotics.com, 2078, Adapted for GEARS
 */

#ifndef LIBEV_PLUSPLUS_LOOP_HH_
#define LIBEV_PLUSPLUS_LOOP_HH_

#include "idle.hh"
#include "io.hh"
#include "timer.hh"

#include <ev.h>

#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

namespace ev {
class Loop
{
public:
    Loop() : m_watcher({}), m_quitting(false)
    {
        auto flush_deferred = [](EV_P_ ev_async *w, int revents)
        {
            std::queue<std::function<void(void)>> calls;

            {
                auto lock = std::scoped_lock<std::mutex>(static_cast<Loop *>(w->data)->m_mutex);

                calls = std::move(static_cast<Loop *>(w->data)->m_deferred_calls);
            }

            while (!calls.empty())
            {
                auto &c = calls.front();

                c();

                calls.pop();
            }
        };

        ev_async_init(&m_watcher, flush_deferred);

        m_watcher.data = this;

        ev_async_start(EV_DEFAULT, &m_watcher);

        m_future_dispatcher = std::thread(&Loop::DispatchFutures, this);
    }

    Loop(const Loop &) = delete;

    ~Loop()
    {
        ev_async_stop(EV_DEFAULT, &m_watcher);

        m_future_dispatcher.join();
    }

    auto operator=(const Loop &) = delete;

    auto Start(void)
    {
        ev_run(EV_DEFAULT, 0);
    }

    auto Quit(void)
    {
        m_quitting.store(true);

        ev_break(EV_DEFAULT, EVBREAK_ALL);
    }

    template <typename Callable, typename... Args>
    auto Call(Callable func, Args &&...args) -> void
    {
        auto lock = std::scoped_lock<std::mutex>(m_mutex);

        m_deferred_calls.emplace(std::bind(func, std::forward<Args>(args)...));

        ev_async_send(EV_DEFAULT, &m_watcher);
    }

    template <typename Callable, typename... Args>
    auto CallAfter(std::chrono::microseconds after, Callable &&func, Args... args) -> void
    {
        auto wrapped_func = [func, args = std::tuple<Args...>(args...)](Timer &t)
        {
            std::apply(func, args);

            t.Stop();

            delete &t;
        };

        auto t = new Timer(wrapped_func, after);

        t->Start();
    }

    template <typename T, typename Callable>
    auto CallWithFuture(std::future<T> &&future, Callable &&func) -> void
    {
        auto dispatch = [this, func, future = std::shared_future<T>(std::move(future))]()
        {
            if (const auto status = future.wait_for(std::chrono::seconds(0));
                status == std::future_status::ready || status == std::future_status::deferred)
            {
                Call(func, std::shared_future<T>(std::move(future)));

                return true;
            }

            return false;
        };

        {
            auto lock = std::scoped_lock<std::mutex>(m_futures_mutex);

            m_future_calls.push_back(std::move(dispatch));
        }

        m_cv.notify_all();
    }

    template <typename Callable, typename... Args>
    auto RegisterIdle(Callable func, Args... args) -> std::unique_ptr<Idle>
    {
        auto wrapped_func = [func, args = std::tuple<Args...>(args...)]() { std::apply(func, args); };

        return std::make_unique<Idle>(wrapped_func);
    }

    template <typename Callable, typename... Args>
    auto RegisterInterval(std::chrono::microseconds interval, Callable func, Args... args) -> std::unique_ptr<Timer>
    {
        auto wrapped_func = [func, args = std::tuple<Args...>(args...)](Timer &) { std::apply(func, args); };

        return std::make_unique<Timer>(wrapped_func, interval);
    }

    template <typename Callable, typename... Args>
    auto RegisterFileDescriptor(int fd, unsigned int events, Callable func, Args... args) -> std::unique_ptr<IO>
    {
        auto wrapped_func = [func, args = std::tuple<Args...>(args...)](unsigned int events)
        { std::apply(func, std::tuple_cat(std::tuple(events), args)); };

        return std::make_unique<IO>(wrapped_func, fd, events);
    }

private:
    auto DispatchFutures(void) -> void
    {
        std::vector<std::function<bool(void)>> calls;

        while (!m_quitting.load())
        {
            {
                auto lock = std::unique_lock<std::mutex>(m_futures_mutex);

                if (!m_future_calls.empty())
                {
                    calls.insert(calls.end(),
                                 std::make_move_iterator(m_future_calls.begin()),
                                 std::make_move_iterator(m_future_calls.end()));

                    m_future_calls.clear();
                }
            }

            if (calls.empty())
            {
                auto lock = std::unique_lock<std::mutex>(m_futures_mutex);

                m_cv.wait_for(lock, std::chrono::milliseconds(125));
            }
            else
            {
                for (auto it = calls.begin(); it != calls.end();)
                {
                    if ((*it)())
                    {
                        it = calls.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }
    }

private:
    struct ev_async m_watcher;

    std::queue<std::function<void(void)>> m_deferred_calls;

    std::vector<std::function<bool(void)>> m_future_calls;

    std::thread m_future_dispatcher;

    std::mutex m_mutex;
    std::mutex m_futures_mutex;

    std::condition_variable m_cv;

    std::atomic_bool m_quitting;
};
} // namespace ev

#endif
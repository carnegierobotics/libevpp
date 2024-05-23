/**
 * @file timer.hh
 *
 * C++ wrapper around a libev Timer Watcher
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

#ifndef LIBEV_PLUSPLUS_TIMER_HH_
#define LIBEV_PLUSPLUS_TIMER_HH_

#include "watcher.hh"

#include <chrono>
#include <functional>

#include <ev.h>

class Timer : public Watcher<Timer> {
public:
    template<typename Callable>
    Timer(Callable &&func, std::chrono::microseconds after, std::chrono::microseconds repeat = std::chrono::microseconds(0))
        : m_watcher({})
        , m_callback(std::move(func)) {

        auto wrapped_cb = [](EV_P_ ev_timer *w, int revents) {
            auto &t = *static_cast<Timer*>(w->data);

            t.m_callback(t);
        };

        ev_timer_init(
            &m_watcher,
            wrapped_cb,
            std::chrono::duration<float>(after).count(),
            std::chrono::duration<float>(repeat).count());

        m_watcher.data = this;
    }

    virtual ~Timer() override final = default;

    auto Start(void) noexcept -> void override final {
        ev_timer_start(EV_DEFAULT, &m_watcher);
    }

    auto Stop(void) noexcept -> void override final {
        ev_timer_stop(EV_DEFAULT, &m_watcher);
    }

    auto Reset(std::chrono::microseconds after, std::chrono::microseconds repeat = std::chrono::microseconds(0)) noexcept -> void {
        ev_timer_set(
            &m_watcher,
            std::chrono::duration<float>(after).count(),
            std::chrono::duration<float>(repeat).count()
        );
    }

private:
    struct ev_timer m_watcher;

    std::function<void(Timer &)> m_callback;
};

#endif

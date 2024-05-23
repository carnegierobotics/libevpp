/**
 * @file idle.hh
 *
 * C++ wrapper around a libev Idle Watcher
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

#ifndef LIBEV_PLUSPLUS_IDLE_HH_
#define LIBEV_PLUSPLUS_IDLE_HH_

#include "watcher.hh"

#include <functional>

#include <ev.h>

class Idle : public Watcher<Idle> {
public:
    template<typename Callable>
    Idle(Callable &&func)
        : m_watcher({})
        , m_callback(std::move(func)) {

        auto wrapped_cb = [](EV_P_ ev_idle *w, int revents) {
            auto &idle = *static_cast<Idle*>(w->data);

            idle.m_callback();
        };

        ev_idle_init(
            &m_watcher,
            wrapped_cb
        );

        m_watcher.data = this;
    }

    virtual ~Idle() override final = default;


    virtual auto Start(void) noexcept -> void override final {
        ev_idle_start(EV_DEFAULT, &m_watcher);
    }

    auto Stop(void) noexcept -> void override final {
        ev_idle_stop(EV_DEFAULT, &m_watcher);
    }

private:
    struct ev_idle m_watcher;

    std::function<void()> m_callback;
};

#endif

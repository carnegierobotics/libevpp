/**
 * @file io.hh
 *
 * C++ wrapper around a libev IO Watcher
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

#ifndef LIBEV_PLUSPLUS_IO_HH_
#define LIBEV_PLUSPLUS_IO_HH_

#include "watcher.hh"

#include <ev.h>

#include <functional>

namespace ev {
class IO : public Watcher<IO>
{
public:
    template <typename Callable>
    IO(Callable &&func, int fd, int events) : m_watcher({})
                                            , m_callback(std::move(func))
    {
        auto wrapped_cb = [](EV_P_ ev_io *w, int revents)
        {
            auto &io = *static_cast<IO *>(w->data);

            io.m_callback(revents);
        };

        ev_io_init(&m_watcher, wrapped_cb, fd, events);

        m_watcher.data = this;
    }

    virtual ~IO() override final = default;

    virtual auto Start(void) noexcept -> void override final
    {
        ev_io_start(EV_DEFAULT, &m_watcher);
    }

    virtual auto Stop(void) noexcept -> void override final
    {
        ev_io_stop(EV_DEFAULT, &m_watcher);
    }

private:
    struct ev_io m_watcher;

    std::function<void(int)> m_callback;
};
} // namespace ev

#endif

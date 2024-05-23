/**
 * @file watcher.hh
 *
 * C++ wrapper around libev Watcher
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

#ifndef LIBEV_PLUSPLUS_WATCHER_HH_
#define LIBEV_PLUSPLUS_WATCHER_HH_

template<typename T>
class Watcher {
public:
    Watcher(const Watcher &) = delete;
    Watcher(Watcher &&) = delete;

    virtual ~Watcher() {
        static_cast<T *>(this)->Stop();
    }

    auto operator=(const Watcher &) = delete;

    auto operator=(T &&other) -> T & = delete;

    virtual auto Start(void) noexcept -> void = 0;
    virtual auto Stop(void) noexcept -> void = 0;

protected:
    Watcher() = default;
};

#endif

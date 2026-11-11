#include <ctime>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <libev++/loop.hh>

void Require(bool value, const char *message)
{
    if (!value)
    {
        throw std::runtime_error(message);
    }
}

int main()
{
    try
    {
        {
            ev::Loop loop;
            std::promise<int> pending;
            bool dispatched = false;
            loop.CallWithFuture(pending.get_future(), [&](auto ready) { dispatched = ready.get() == 7; });
            const auto cpu_before = std::clock();
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            const auto cpu_seconds = double(std::clock() - cpu_before) / CLOCKS_PER_SEC;
            Require(cpu_seconds < 0.06, "pending future busy-spins");
            pending.set_value(7);
            for (int i = 0; i != 100 && !dispatched; ++i)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                ev_run(EV_DEFAULT, EVRUN_NOWAIT);
            }
            Require(dispatched, "ready future not dispatched");
            // Destruction without Quit must terminate even with an unresolved future.
            std::promise<int> unresolved;
            loop.CallWithFuture(unresolved.get_future(), [](auto) {});
        }
        // Exercise idle dispatcher shutdown both before and after it can enter
        // its condition-variable wait. The CTest timeout catches a lost wake.
        for (int iteration = 0; iteration < 256; ++iteration)
        {
            ev::Loop idle_loop;
            if (iteration % 3 == 0)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            else
            {
                std::this_thread::yield();
            }
            if (iteration % 2 == 0)
            {
                idle_loop.Quit();
            }
        }
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

#include "UnixSignalListener.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

static mutex m;
static condition_variable cv;
static UnixSignalListener sl;

void worker1()
{
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk);
    cout << "Worker1 stopped\n";
}

void worker2()
{
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk);
    cout << "Worker2 stopped\n";
}

// for testing just use: kill <pid>
int main()
{
    sl.init(); // block all signals
    std::thread w1(worker1);
    std::thread w2(worker2);

    sl.setHandler(SIGTERM, [=, &w1, &w2] {
        // You can do here async unsafe things
        cout << "Notify workers\n";
        {
            std::unique_lock<std::mutex> lk(m);
            cv.notify_all();
        }
        w1.join();
        w2.join();
    }, true);

    sl.listen();
    return 0;
}

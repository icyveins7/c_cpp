#include "timer.h"
#include <thread>


int main(){
    using namespace std::chrono_literals;

    {
        HighResolutionTimer<> timer;

        timer.start("custom start");
        std::this_thread::sleep_for(100ms);
        timer.event("after sleep");
        std::this_thread::sleep_for(200ms);
        timer.stop("end after sleep again");

    }
    {
        HighResolutionTimer<std::chrono::seconds> timer;

        timer.start("custom start");
        std::this_thread::sleep_for(100ms);
        timer.event("after sleep");
        std::this_thread::sleep_for(200ms);
        timer.stop("end after sleep again");
    }

    return 0;
}

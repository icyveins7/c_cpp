#include <thread>
#include <future>
#include <iostream>

void thread_func(int tidx){
    throw std::runtime_error("throw from thread " + std::to_string(tidx));
}

int main(int argc, char *argv[]){

    // this obviously doesn't work; new thread has new stack. exception isn't propagated to main thread
    // try{
    //     std::thread t1(thread_func, 0);
    //     std::thread t2(thread_func, 1);

    //     t1.join();
    //     t2.join();
    // }
    // catch(std::exception &e){
    //     std::cout << e.what() << std::endl;
    // }
    // compiling this will cause std threads to terminate and error in the main thread

    // try using future async
    std::future<void> f1 = std::async(thread_func, 0);
    std::future<void> f2 = std::async(thread_func, 1);
    try{
        f1.get();
        f2.get();
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
    }
    // this works as expected!
    // but this will make it so that the main thread only sees exception at the end. you may want to use shared state
    // to terminate threads in an inner loop


 
    return 0;
}
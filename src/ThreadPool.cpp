#include "ThreadPool.h"
#include <spdlog/spdlog.h>
#include <iostream>
ThreadPool::ThreadPool(size_t n): stop(false) {
    for (size_t i=0;i<n;i++) 
        workers.emplace_back(
            [this]{ this->workerLoop(); }
        );
}
ThreadPool::~ThreadPool(){ 
    stop = true;
    //wake up all threads
    cv.notify_all(); 
    for(auto &t: workers) 
        if(t.joinable())
        //join: wait for thread to finish
        //detach: let thread run in background
            t.join(); 
}
//producer
void ThreadPool::enqueue(std::function<void()> f){
    { 
        //lock will be freed when exiting this scope
        //mutex lock,use unique_lock not lock_guard
        std::unique_lock<std::mutex> lk(m); 
        tasks.push(std::move(f)); 
    }
    //notify one waiting thread
    cv.notify_one();
}
//consumer
void ThreadPool::workerLoop(){
    //atomic bool stop,equals to stop.load(std::memory_order_acquire)
    while(!stop){
        std::function<void()> task;
        { 
            //unique lock works with condition_variable
            std::unique_lock<std::mutex> lk(m); 
            //wake up after lambda return true
            /*
                equals to:
                while(!predicate()){
                    cv.wait(lk);
                }
            */
            //if returned false,wait function will release the mutex lock related to lk,which is
            //the one on m,enabling tasks.push()
            cv.wait(lk, [this]{ return stop || !tasks.empty(); }); 
            //if stop and no tasks, exit
            if(stop && tasks.empty()) return; 
            //task not empty
            task = std::move(tasks.front()); 
            tasks.pop(); 
        }//lock released
        try{ 
            task(); 
        }catch(...){ spdlog::error("Exception in thread pool task");}
    }
}

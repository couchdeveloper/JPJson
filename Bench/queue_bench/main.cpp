//
//  main.cpp
//  queue_bench
//
//  Created by Andreas Grosam on 9/14/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "json/utility/producer_consumer_queue2.hpp"
#include "json/utility/synchronous_queue.hpp"
#include "json/objc/mutex.hpp"
#include "json/objc/semaphore.hpp"
#include <dispatch/dispatch.h>
#include "utilities/timer.hpp"


using json::utility::producer_consumer_queue;
using json::objc::gcd::mutex;
using json::objc::gcd::semaphore;
using utilities::timer;




namespace {
    
    void test1() 
    {
        std::cout << "--------------------------------------------------------\n";
        std::cout << "                  producer_consumer_queue               \n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::endl;
        
        // Multiple producer, multiple consumer, blocking FIFO:
        typedef producer_consumer_queue<0, int, mutex, semaphore, true, true>     queue_t;
        
        queue_t queue;
        queue_t* qp = &queue;
        
        // Run one consumer and one producer concurrently:
        
        dispatch_group_t group = dispatch_group_create();
        dispatch_queue_t gc_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_queue_t s1_queue = dispatch_queue_create("com.queue_bench.s1_queue", DISPATCH_QUEUE_SERIAL);
        dispatch_queue_t s2_queue = dispatch_queue_create("com.queue_bench.s1_queue", DISPATCH_QUEUE_SERIAL);
        
        // Producer:
        __block bool producer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            double timeout = 1.0;
            for (int i = 0; i < 1000000; ++i) {
                bool produced = qp->produce(i, timeout);
                if (not produced) {
                    producer_timeout_occured = true;
                    return;
                }
            }
            qp->produce(-1);
        });
        
        // Consumer
        __block bool consumer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            double timeout = 1.0;
            while (1) {
                int* p = qp->aquire(timeout);
                if (p == NULL) {
                    consumer_timeout_occured = true;
                    return;
                } else {
                    int v = *p;
                    qp->commit(p);
                    if (v == -1) {
                        return; // finished.
                    }
                }
            }
        });
        
        
        timer t;
        t.start();
        bool completed = (0 == dispatch_group_wait(group, DISPATCH_TIME_FOREVER));
        t.stop();
        dispatch_release(group);
        dispatch_release(s1_queue);
        dispatch_release(s2_queue);
        
        std::cout << "producer_timeout_occured: " << producer_timeout_occured << "\n";
        std::cout << "consumer_timeout_occured: " << consumer_timeout_occured << "\n";
        std::cout << "completed: " << completed << std::endl;
        std::cout << "elapsed time: " << t.seconds() << "seconds" << std::endl;
    }


    void test2() 
    {
        std::cout << "--------------------------------------------------------\n";
        std::cout << "                    synchronous_queue                   \n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::endl;
        
        // Multiple producer, multiple consumer, blocking FIFO:
        typedef json::utility::synchronous_queue<int>     queue_t;
        
        queue_t queue;
        queue_t* qp = &queue;
        
        // Run one consumer and one producer concurrently:
        
        dispatch_group_t group = dispatch_group_create();
        dispatch_queue_t gc_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        
        // Producer:
        __block bool producer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            double timeout = 1.0;
            for (int i = 0; i < 1000000; ++i) {
                if (qp->put(i, timeout) != queue_t::OK) {
                    return;
                }
            }
            qp->put(-1, timeout);
        });
        
        // Consumer
        __block bool consumer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            double timeout = 1.0;
            while (1) {
                std::pair<queue_t::result_type, queue_t::value_type> result = qp->aquire(timeout);
                if (result.first != queue_t::OK) {
                    return;
                }
                qp->commit();
                if (result.second == -1) {
                    return; // finished.
                }
            }
        });
        
        
        timer t;
        t.start();
        bool completed = (0 == dispatch_group_wait(group, DISPATCH_TIME_FOREVER));
        t.stop();
        dispatch_release(group);
        
        std::cout << "producer_timeout_occured: " << producer_timeout_occured << "\n";
        std::cout << "consumer_timeout_occured: " << consumer_timeout_occured << "\n";
        std::cout << "completed: " << completed << std::endl;
        std::cout << "elapsed time: " << t.seconds() << "seconds" << std::endl;
    }
    
    
    void test3() 
    {
        std::cout << "--------------------------------------------------------\n";
        std::cout << "         dispatch_queue and dispatch_semaphore          \n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::endl;
        
        
        // Run one consumer and one producer concurrently:
        
        dispatch_group_t group = dispatch_group_create();
        dispatch_queue_t gc_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_semaphore_t sync = dispatch_semaphore_create(1);
        
        // Producer and Consumer
        dispatch_group_async(group, gc_queue, ^{
            for (int i = 0; i < 1000000; ++i) {
                dispatch_semaphore_wait(sync, DISPATCH_TIME_FOREVER);
                dispatch_group_async(group, gc_queue, ^{
                    // Consumer, access i:
                    int n = i;
                    dispatch_semaphore_signal(sync);
                });
            }
        });
                
        timer t;
        t.start();
        bool completed = (0 == dispatch_group_wait(group, DISPATCH_TIME_FOREVER));
        t.stop();
        dispatch_release(group);
        dispatch_release(sync);
        
        std::cout << "completed: " << completed << std::endl;
        std::cout << "elapsed time: " << t.seconds() << "seconds" << std::endl;
    }
    
    

    void test4() 
    {
        std::cout << "--------------------------------------------------------\n";
        std::cout << "                  dispatch_sync                         \n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::endl;
        
        
        // Run one consumer and one producer concurrently:
        
        dispatch_queue_t gc_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_semaphore_t completed = dispatch_semaphore_create(0);
        dispatch_semaphore_t send = dispatch_semaphore_create(1);
        dispatch_semaphore_t recv = dispatch_semaphore_create(0);
        
        
        __block int q;      // queue with capacity 1        
        __block int r = 0;  // the result

        // Producer
        dispatch_async(gc_queue, ^{
            for (int i = 0; i < 1000000; ++i) {
                dispatch_semaphore_wait(send, DISPATCH_TIME_FOREVER);
                q = i;
                dispatch_semaphore_signal(recv);
            }
            dispatch_semaphore_wait(send, DISPATCH_TIME_FOREVER);
            q = -1;
            dispatch_semaphore_signal(recv);
        });

        // Consumer
        dispatch_async(gc_queue, ^{
            bool done = false;
            while (!done) {
                dispatch_semaphore_wait(recv, DISPATCH_TIME_FOREVER);
                if (q == -1) {
                    done = true;
                } 
                else {
                    r += q;
                }
                dispatch_semaphore_signal(send);
            }
            dispatch_semaphore_signal(completed);
        });
        
        
        
        timer t;
        t.start();
        dispatch_semaphore_wait(completed, DISPATCH_TIME_FOREVER);
        t.stop();
        dispatch_release(completed);
        
        std::cout << "completed with r = " << r << std::endl;
        std::cout << "elapsed time: " << t.seconds() << "seconds" << std::endl;
    }
    
    void test5() 
    {
        std::cout << "--------------------------------------------------------\n";
        std::cout << "                  dispatch mutex                        \n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::endl;
        
        
        // Run one consumer and one producer concurrently:
        
        dispatch_queue_t gc_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_semaphore_t completed = dispatch_semaphore_create(0);
        dispatch_semaphore_t mutex = dispatch_semaphore_create(1);
        
        // Producer and Consumer
        __block int n = 0;
        dispatch_async(gc_queue, ^{
            for (int i = 0; i < 1000000; ++i) {
                dispatch_semaphore_wait(mutex, DISPATCH_TIME_FOREVER);
                    // Consumer, access i:
                    n += i;
                dispatch_semaphore_signal(mutex);
            }
            dispatch_semaphore_signal(completed);
        });
        
        timer t;
        t.start();
        dispatch_semaphore_wait(completed, DISPATCH_TIME_FOREVER);
        t.stop();
        dispatch_release(completed);
        
        std::cout << "completed with n = " << n << std::endl;
        std::cout << "elapsed time: " << t.seconds() << "seconds" << std::endl;
    }
    
    

        
} // namespace 
    
    
int main (int argc, const char * argv[])
{    
    //test1();
    //test2();
    //test3();
    test4();
    //test5();
}


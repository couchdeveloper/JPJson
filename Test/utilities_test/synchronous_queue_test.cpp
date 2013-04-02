//
//  synchronous_queue_test.cpp
//  Test
//
//  Created by Andreas Grosam on 9/18/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <gtest/gtest.h>
#include "json/utility/synchronous_queue.hpp"


#include "json/objc/CFDataBuffer.hpp"
#include "json/objc/mutex.hpp"
#include "json/objc/semaphore.hpp"
#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>


#include <fstream>

// for testing


namespace {
    
    using json::utility::synchronous_queue;
    using json::objc::gcd::mutex;
    using json::objc::gcd::semaphore;
    
    
    
    
    class synchronous_queue_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        synchronous_queue_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~synchronous_queue_test() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
#pragma mark -

    TEST_F(synchronous_queue_test, Constructor) 
    {
        typedef synchronous_queue<int>  queue_t;
        typedef std::pair<queue_t::result_type, queue_t::value_type> result_t;
        
        queue_t q;        
        EXPECT_EQ(true, q.empty());
        
    }    
    
    
    TEST_F(synchronous_queue_test, GetAccessEmptyQueue) 
    {
        typedef synchronous_queue<int>  queue_t;
        typedef std::pair<queue_t::result_type, queue_t::value_type> get_result_t;
        
        // result values:
        //
        // queue_t::OK
        // queue_t::TIMEOUT_NOT_DELIVERED
        // queue_t::TIMEOUT_NOT_PICKED
        // queue_t::TIMEOUT_NOTHING_OFFERED
        
        //
        //  Empty queue. get() shall return time out.
        //
        
        queue_t q;        
        
        get_result_t r = q.get(0.01);
        EXPECT_TRUE(r.first != queue_t::OK);
        EXPECT_TRUE(r.first == queue_t::TIMEOUT_NOTHING_OFFERED);
        
        for (int i = 0; i < 1000; ++i) {
            get_result_t r = q.get(0);
            EXPECT_TRUE(r.first == queue_t::TIMEOUT_NOTHING_OFFERED);
        }
    }    
    
    TEST_F(synchronous_queue_test, PutGetAccess) 
    {
        typedef synchronous_queue<int>  queue_t;
        typedef std::pair<queue_t::result_type, queue_t::value_type> get_result_t;
        typedef queue_t::result_type                                 put_result_t;
        
        // result values:
        //
        // queue_t::OK
        // queue_t::TIMEOUT_NOT_DELIVERED
        // queue_t::TIMEOUT_NOT_PICKED
        // queue_t::TIMEOUT_NOTHING_OFFERED
        
        //dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{ });

        //
        //  With no consumer setup, and an empty queue, put() shall block 
        //  infinitely, or timeout with TIMEOUT_NOT_PICKED.
        //
        queue_t q1;        
        double timeout = 0.1;
        put_result_t r = q1.put(17, timeout);
        EXPECT_TRUE(r == queue_t::TIMEOUT_NOT_PICKED);
        
        //
        // The object has been delivered, though:        
        //
        EXPECT_TRUE(not q1.empty());
        get_result_t gr = q1.get(0.1);
        EXPECT_TRUE(gr.first == queue_t::OK);
        EXPECT_EQ(17, gr.second);
        EXPECT_TRUE(q1.empty());
        
        //
        //  Full queue, put() shall timeout with TIMEOUT_NOT_DELIVERED
        //
        queue_t q;
        q.put(7, 0.1);  // timed out
        for (int i = 0; i < 1000; ++i) {
            put_result_t r = q.put(0, 0.0);
            EXPECT_TRUE(r == queue_t::TIMEOUT_NOT_DELIVERED);
        }
        //
        // The object '7' has been delivered, though:        
        //
        EXPECT_TRUE(not q.empty());
        gr = q.get(0.1);
        EXPECT_TRUE(gr.first == queue_t::OK);
        EXPECT_EQ(7, gr.second);
        EXPECT_TRUE(q.empty());
    }    
    
    
    
    
    
    TEST_F(synchronous_queue_test, ConcurrentOneProducerOneConsumer) 
    {        
        typedef synchronous_queue<int>  queue_t;
        typedef std::pair<queue_t::result_type, queue_t::value_type> get_result_t;
        typedef queue_t::result_type                                 put_result_t;
        
        queue_t queue;
        queue_t* qp = &queue;
        
        // Run one consumer and one producer concurrently:
        
        dispatch_group_t group = dispatch_group_create();
        dispatch_queue_t gc_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

        double timeout = -1;
        
        
        // Producer:
        __block bool producer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            put_result_t result;
            for (int i = 0; i <= 1000000; ++i) {
                if (i != 1000000)
                    result = qp->put(i, timeout);
                else
                    result = qp->put(-1, timeout);
                if (result != queue_t::OK) {
                    producer_timeout_occured = true;
                    printf("producer timed out at count = %d\n", i);
                    return;
                }
            }
            
        });
        
        // Consumer
        __block bool consumer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            size_t count = 0;
            while (1) {
                get_result_t result = qp->get(timeout);
                if (result.first != queue_t::OK) {
                    consumer_timeout_occured = true;
                    printf("consumer timed out at count = %lu\n", count);
                    return;
                } else {
                    ++count;
                    int v = result.second;
                    if (v == -1) {
                        return; // finished.
                    }
                }
            }
        });
        
        
        bool completed = (0 == dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 20*NSEC_PER_SEC)));
        dispatch_release(group);
        
        EXPECT_EQ(false, producer_timeout_occured);
        EXPECT_EQ(false, consumer_timeout_occured);
        EXPECT_EQ(true, completed);
    } 
    


    
    
}
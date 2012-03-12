//
//  producer_consumer_queue_test.cpp
//  Test
//
//  Created by Andreas Grosam on 9/13/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#error Obsolete
#warning This file will become obsolete

#include "gtest/gtest.h"

#include "json/utility/producer_consumer_queue2.hpp"
#include "json/objc/mutex.hpp"
#include "json/objc/semaphore.hpp"
#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>




namespace {
    
    using json::utility::producer_consumer_queue;
    using json::objc::gcd::mutex;
    using json::objc::gcd::semaphore;
    
    
    class producer_consumer_queue_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        producer_consumer_queue_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~producer_consumer_queue_test() {
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
    
    TEST_F(producer_consumer_queue_test, Test1) {
        
        // Multiple producer, multiple consumer, blocking FIFO:
        typedef producer_consumer_queue<0, int, mutex, semaphore, true, true>     queue_t;
        
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
                bool produced = qp->produce(i);
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
                int* p = qp->aquire();
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
        
        
        bool completed = (0 == dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 20*NSEC_PER_SEC)));
        dispatch_release(group);
        
        EXPECT_EQ(false, producer_timeout_occured);
        EXPECT_EQ(false, consumer_timeout_occured);
        EXPECT_EQ(true, completed);
    } 

    
    
    TEST_F(producer_consumer_queue_test, Test1) {
        
        // Multiple producer, multiple consumer, blocking FIFO:
        typedef producer_consumer_queue<0, int, mutex, semaphore, true, true>     queue_t;
        
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
                bool produced = qp->produce(i);
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
                int* p = qp->aquire();
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
        
        
        bool completed = (0 == dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 20*NSEC_PER_SEC)));
        dispatch_release(group);
        
        EXPECT_EQ(false, producer_timeout_occured);
        EXPECT_EQ(false, consumer_timeout_occured);
        EXPECT_EQ(true, completed);
    } 
    
}
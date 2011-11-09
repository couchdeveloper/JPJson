//
//  buffer_queue_Test.cpp
//  Test
//
//  Created by Andreas Grosam on 7/18/11.
//  Copyright 2011 Andreas Grosam
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//


#warning unused testee


#include "json/utility/buffer_queue.hpp"
#include "gtest/gtest.h"



// mutex and semaphores
#include "json/ObjC/mutex.hpp"
#include "json/ObjC/semaphore.hpp"


// for testing
#include <dispatch/dispatch.h>
#include <string.h>
#include <stdexcept>





using namespace json;

using json::buffer_queue;
using json::objc::gcd::mutex;
using json::objc::gcd::semaphore;


namespace {
    
        
    class buffer_queue_Test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        buffer_queue_Test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~buffer_queue_Test() {
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
    
    
    
    /*        

     buffer_queue(size_t capacity = 10);
     
     bool produce(const buffer_type& buffer, double timeout = 0);     
     buffer_pointer aquire(double timeout = 0.0);     
     void commit(buffer_pointer buffer);
     size_t  avail() const;     
     size_t  processing() const;                 
     bool empty() const;
     void clear();     
     void commit();
     
     
     */    
    
    
    struct buffer_t {
        buffer_t(std::size_t id) : id_(id), data_ (0) {}
        std::size_t id_;
        int data_;
        
        friend inline 
        bool operator==(const buffer_t& lhv, const buffer_t& rhv)
        {
            return lhv.id_ == rhv.id_;
        }
    };
    
    
    typedef buffer_queue<buffer_t, mutex, semaphore> BufferQueue;
    
    
    TEST_F(buffer_queue_Test, DefaultCtor) 
    {
        BufferQueue queue; 
        
        EXPECT_TRUE( queue.empty() );
        EXPECT_EQ(0, queue.avail() );
        EXPECT_EQ(0, queue.processing() );
    }
    
    TEST_F(buffer_queue_Test, Produce) 
    {
        // Create a queue with max 3 buffers:
        const int capacity = 3;
        BufferQueue queue(capacity);
        
        int count = 0;
        
        while (queue.produce(buffer_t(count), 0) == true) {
            ++count;
        }
        
        EXPECT_EQ(capacity, count);
        EXPECT_EQ(capacity, queue.avail());
        EXPECT_FALSE( queue.empty() );
        EXPECT_EQ(0, queue.processing() );
   }
    
    
    TEST_F(buffer_queue_Test, ProduceConsume) 
    {
        // Create a queue with max 100 buffers:
        const int capacity = 100;
        BufferQueue queue(capacity);
        
        int produceCount = 0;        
        while (queue.produce(buffer_t(produceCount), 0.0) == true) {
            ++produceCount;
            EXPECT_EQ(produceCount, queue.avail());
            EXPECT_EQ(0, queue.processing() );
        }        
        EXPECT_EQ(capacity, produceCount);
        EXPECT_EQ(capacity, queue.avail());
        EXPECT_FALSE( queue.empty() );
        EXPECT_EQ(0, queue.processing() );
        
        
        int consumeCount = 0;
        buffer_t* p = NULL;
        while ( (p = queue.aquire(0.0)) != NULL) {
            ++consumeCount;
            EXPECT_EQ(capacity - consumeCount, queue.avail());
            EXPECT_EQ(1, queue.processing() );
            queue.commit(p);
            EXPECT_EQ(capacity - consumeCount, queue.avail());
            EXPECT_EQ(0, queue.processing() );
        }
        EXPECT_EQ(capacity, consumeCount);
        EXPECT_TRUE( queue.empty() );
        EXPECT_EQ(0, queue.processing() );
        EXPECT_EQ(0, queue.avail());
    }
    
    
    TEST_F(buffer_queue_Test, ProduceConsumeDispatch) 
    {
        // Use a concurrent queue
        dispatch_queue_t concurrentQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_group_t group = dispatch_group_create();
        
        std::size_t N = 1000000;
        
        BufferQueue* queue = new BufferQueue(100);
        
        __block std::size_t produceCount = 0;
        
        // Producer
        dispatch_group_async(group, concurrentQueue, ^{
            while (produceCount < N) {
                std::size_t count_timeouts = 0;
                while (queue->produce(buffer_t(produceCount), 0.1) == false) {
                    ++count_timeouts;
                    if (count_timeouts > 10)
                        std::cout << "timeout while producing" << std::endl;
                }
                ++produceCount;
            }            
        });
        
        
        // Consumer 1:
        dispatch_group_async(group, concurrentQueue, ^{
            buffer_t* p = NULL;
            while ( (p = queue->aquire(2.0)) != NULL) {
                //++consumeCount;
                queue->commit(p);
            }
        });
        
        // Consumer 2:
        dispatch_group_async(group, concurrentQueue, ^{
            buffer_t* p = NULL;
            while ( (p = queue->aquire(2.0)) != NULL) {
                //++consumeCount;
                queue->commit(p);
            }
        });
        
        // Consumer 3:
        dispatch_group_async(group, concurrentQueue, ^{
            buffer_t* p = NULL;
            while ( (p = queue->aquire(2.0)) != NULL) {
                //++consumeCount;
                queue->commit(p);
            }
        });
        
        
        dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
        EXPECT_EQ(N, produceCount);        
        //EXPECT_EQ(N, consumeCount);        
        EXPECT_TRUE( queue->empty() );
        EXPECT_EQ(0, queue->processing() );
        EXPECT_EQ(0, queue->avail());
        
        delete queue;
    }

    
    
}  //namespace
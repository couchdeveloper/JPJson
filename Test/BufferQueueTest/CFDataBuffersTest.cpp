//
//  BufferQueue.cpp
//
//  Created by Andreas Grosam on 6/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "json/buffer_queue.hpp"
#include "json/buffer_queue_iterator.hpp"
#include "gtest/gtest.h"

// for testing
#include <dispatch/semaphore.h>
#include <string.h>





using namespace json;

using json::buffer_queue;
using json::buffer_queue_iterator;



namespace {
    
    
    // The fixture for testing class json::Boolean:
    
    class BufferQueue : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        BufferQueue() {
            // You can do set-up work for each test here.
        }
        
        virtual ~BufferQueue() {
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
    
    TEST_F(BufferQueueTest, DefaultCtor) 
    {
        // Create a buffers instance with at may 100 buffers.
        CFDataConstBuffers<char> buffers(100);
        EXPECT_EQ(0, buffers.avail());
    }


    TEST_F(BufferQueueTest, SimpleProduceConsume) 
    {
        typedef std::pair<CFDataConstBuffer<char>, bool> result_t;
        
        // Create a buffers instance with at max 100 buffers:
        const size_t N = 10000;
        CFDataConstBuffers<char> buffers(N);
        
        const UInt8 data[1024*4] = {1, 2, 3, 4};
        
        for (int i = 0; i < N; ++i) {
            CFDataRef d = CFDataCreate(NULL, data, sizeof(data));
            CFDataConstBuffer<char> buffer = d;
            CFRelease(d);
            buffers.produce(buffer);
            EXPECT_EQ(i + 1, buffers.avail());
        }
        
        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(N-i, buffers.avail());
            result_t result = buffers.consume();
            if (result.second) {
                result.first.release();
            }
            EXPECT_EQ(N-1-i, buffers.avail());
        }
        
    }
    
    
    
}
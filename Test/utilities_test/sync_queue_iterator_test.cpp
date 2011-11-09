//
//  sync_queue_iterator_test.cpp
//  Test
//
//  Created by Andreas Grosam on 9/18/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//


#error This file is obsolete



#include "json/utility/synchronous_queue.hpp"
#include "json/objc/CFDataBuffer.hpp"
#include "json/utility/sync_queue_iterator.hpp"
#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>



// for testing


namespace {
    
    using json::utility::synchronous_queue;
    using json::utility::sync_queue_iterator;
    using json::utility::CFDataBuffer;
    
    
    
    
    class sync_queue_iterator_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        sync_queue_iterator_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~sync_queue_iterator_test() {
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
    
    TEST_F(sync_queue_iterator_test, Constructor) 
    {
        typedef CFDataBuffer<char>                  CFDataBuffer;
        typedef synchronous_queue<CFDataBufferInt>  queue_t;
        typedef sync_queue_iterator<char, queue_t>  iterator;
        
        queue_t q;        
        iterator eof;
        iterator iter(q);
        
        const char* s = "0123456789";
        CFDataBuffer buffer(s, strlen(s));
        
        
        
        basic_streambuf
        streambuf
        filebuf
        basic_filebuf
        
        EXPECT_EQ(true, q.empty());
        
    }    
    
}
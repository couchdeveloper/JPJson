//
//  atomic_test.cpp
//  Test
//
//  Created by Andreas Grosam on 9/13/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//



#include "json/utility/atomic.hpp"
#include <gtest/gtest.h>

#include <iostream>
#include <iomanip>


// for testing


namespace {
    
    using json::utility::atomic;
    
    class atomic_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        atomic_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~atomic_test() {
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
#pragma mark Unicode Code Point
    
    TEST_F(atomic_test, Test1) {
        
        atomic<int32_t> x = 1;
        
        int32_t y = x;
        EXPECT_EQ(y, x);
        EXPECT_EQ(++y, ++x);
        EXPECT_EQ(y, x);
        EXPECT_EQ(y++, x++);
        EXPECT_EQ(y, x);
        EXPECT_EQ(y--, x--);
        EXPECT_EQ(y, x);
        EXPECT_EQ(--y, --x);
        EXPECT_EQ(y, x);
        
        x = 0;
        EXPECT_EQ(0, x);
        
    } 
    
}
//
//  string_to_number_test.cpp
//  Test
//
//  Created by Andreas Grosam on 5/1/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//


#include "json/utility/string_to_number.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <iomanip>


// for testing


namespace {
        
    using json::utility::string_to_number;
    
    class StringToNumberTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        StringToNumberTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~StringToNumberTest() {
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
    
    
    
    TEST_F(StringToNumberTest, Test1) {
        
        const char* string = "1234";
        
        int int_value = string_to_number<int>(string);
        EXPECT_EQ(1234, int_value);
        
        double double_value = string_to_number<double>(string);
        EXPECT_EQ(1234.0, double_value);
        
        
        
        
    } 
    
}
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
#include <boost/lexical_cast.hpp>
#include <limits>


// for testing


namespace {
        
    using json::utility::string_to_number;
    
    template <typename T>
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
    
    
    TYPED_TEST_CASE_P(StringToNumberTest);
    
    
    TYPED_TEST_P(StringToNumberTest, BasicTest)
    {
        typedef TypeParam  number_type;
        
        number_type i = string_to_number<number_type>("0", 1);
        EXPECT_EQ(0, i);
        i = string_to_number<number_type>("1", 1);
        EXPECT_EQ(1, i);
        if (std::numeric_limits<number_type>::is_signed) {
            i = string_to_number<number_type>("-1", 2);
            EXPECT_EQ(-1, i);
        }
        if (std::numeric_limits<number_type>::is_exact) {
            std::string str = boost::lexical_cast<std::string>(std::numeric_limits<number_type>::min());
            i = string_to_number<number_type>(str.c_str(), str.size());
            EXPECT_EQ(std::numeric_limits<number_type>::min(), i);
            str = boost::lexical_cast<std::string>(std::numeric_limits<number_type>::max());
            i = string_to_number<number_type>(str.c_str(), str.size());
            EXPECT_EQ(std::numeric_limits<number_type>::max(), i);
        }
        else {
            number_type value = std::numeric_limits<number_type>::max()*0.1;
            std::string str = boost::lexical_cast<std::string>(value);
            i = string_to_number<number_type>(str.c_str(), str.size());
            EXPECT_NEAR(1.0, value/i, 1e-6);
            value = std::numeric_limits<number_type>::min()*10.0;
            str = boost::lexical_cast<std::string>(value);
            i = string_to_number<number_type>(str.c_str(), str.size());
            EXPECT_NEAR(1.0, value/i, 1e-6);
        }
    }
    
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(StringToNumberTest,
                               BasicTest);
    // Instantiate test cases:
    typedef ::testing::Types<
        int,
        unsigned int,
        long,
        unsigned long,
        long long,
        unsigned long long,
        float,
        double,
        long double
    >  number_types;

    INSTANTIATE_TYPED_TEST_CASE_P(BasicTests, StringToNumberTest, number_types);
    
    
    
}
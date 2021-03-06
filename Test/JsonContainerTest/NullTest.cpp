//
//  NullTest.cpp
//
//  Created by Andreas Grosam on 4/29/11.
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

#include "json/value/Null.hpp"
#include <gtest/gtest.h>

#include <type_traits>


using namespace json;


namespace {
    
    
    // The fixture for testing class json::Number:
    
    class NullTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        NullTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~NullTest() {
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
    

    TEST_F(NullTest, TypeTraits)
    {
        EXPECT_TRUE( (std::is_trivially_copyable<Null>::value) );
        EXPECT_TRUE( (std::is_trivially_default_constructible<Null>::value) );
        EXPECT_TRUE( (std::is_trivial<Null>::value) );
    }
    
    TEST_F(NullTest, Construction)
    {
        EXPECT_TRUE( (std::is_default_constructible<Null>::value) );
        EXPECT_TRUE( (std::is_nothrow_default_constructible<Null>::value) );
        EXPECT_TRUE( (std::is_copy_constructible<Null>::value) );
        EXPECT_TRUE( (std::is_nothrow_copy_constructible<Null>::value) );
        EXPECT_TRUE( (std::is_nothrow_move_constructible<Null>::value == true) );
    }
    
    // Assignment
    TEST_F(NullTest, Assignment)
    {
        EXPECT_TRUE( (std::is_nothrow_copy_assignable<Null>::value == true) );
        EXPECT_TRUE( (std::is_nothrow_move_assignable<Null>::value == true) );
        
        EXPECT_TRUE( (std::is_assignable<Null&, Null>::value) );
        EXPECT_TRUE( (std::is_nothrow_assignable<Null&, Null>::value) );
        EXPECT_TRUE( (std::is_trivially_assignable<Null&, Null>::value) );        
    }
    
    
    
    TEST_F(NullTest, NullConvertible) 
    {
        EXPECT_TRUE( (std::is_convertible<Null, int>::value) == false );
        EXPECT_TRUE( (std::is_convertible<Null, void*>::value) == false );
        EXPECT_TRUE( (std::is_convertible<Null, bool>::value) == false );
        EXPECT_TRUE( (std::is_convertible<Null, std::string>::value) == false );
        EXPECT_TRUE( (std::is_convertible<Null, decltype("abc")>::value) == false );
    }
    
//    // A Null shall be a json type:
//    TEST_F(NullTest, IsJsonType) 
//    {
//        EXPECT_TRUE( (is_json_type<Null>::value) );        
//    }
    
    
    TEST_F(NullTest, NullHasLiteral_null) {
        Null n;
        n = null;
        SUCCEED();
    }
    
    
    TEST_F(NullTest, NullComparison) 
    { 
        Null n1;
        Null n2;
        EXPECT_TRUE( (n1 == n2) );
        EXPECT_TRUE( (n1 == null) );
        
//        EXPECT_FALSE( (null == 1) );
//        EXPECT_FALSE( (null == 0) );
//        EXPECT_FALSE( (null == 1.0) );
//        EXPECT_FALSE( (null == 0.0) );
//        EXPECT_FALSE( (null == "a") );
//        EXPECT_FALSE( (null == false) );
//        EXPECT_FALSE( (null == true) );
//        EXPECT_FALSE( (null == (void*)(0)) );
        
/*        
        EXPECT_FALSE( (null == Boolean(false)) );
        EXPECT_FALSE( (null == Boolean(true)) );
*/ 
    }
    
    
    
    
} // anonymous namespace
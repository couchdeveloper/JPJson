//
//  JsonNumberTest.cpp
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

#include "json/value/number2.hpp"
#include <gtest/gtest.h>

#include <type_traits>


using namespace json;


namespace {
    
    
    // The fixture for testing class json::Number:
    
    class JsonNumberTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonNumberTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonNumberTest() {
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
    
    
    TEST_F(JsonNumberTest, IsNumericTypeTraits) {
        EXPECT_TRUE( json::is_numeric<int>::value );
        EXPECT_TRUE( json::is_numeric<short>::value );
        EXPECT_TRUE( json::is_numeric<long>::value );
        EXPECT_TRUE( json::is_numeric<long long>::value );
        EXPECT_TRUE( json::is_numeric<double>::value );
        EXPECT_TRUE( json::is_numeric<long double>::value );
        EXPECT_TRUE( json::is_numeric<float>::value );

        EXPECT_FALSE( json::is_numeric<bool>::value );

        EXPECT_FALSE( json::is_numeric<char>::value );
        EXPECT_FALSE( json::is_numeric<char*>::value );
        EXPECT_FALSE( json::is_numeric<char16_t>::value );
        EXPECT_FALSE( json::is_numeric<char16_t*>::value );
        EXPECT_FALSE( json::is_numeric<char32_t>::value );
        EXPECT_FALSE( json::is_numeric<char32_t*>::value );
        EXPECT_FALSE( json::is_numeric<void*>::value );
    }
    
    
    TEST_F(JsonNumberTest, NothrowGuarantees)
    {
        EXPECT_TRUE( (std::is_nothrow_default_constructible<Number>::value == true) );
        EXPECT_TRUE( (std::is_nothrow_move_constructible<Number>::value == true) );
        EXPECT_TRUE( (std::is_nothrow_default_constructible<Number>::value == true) );
    }
    
    
    TEST_F(JsonNumberTest, NumberDefaultCtor) {
        Number n0;
        EXPECT_EQ("NaN", n0.string());
    }    
    
    // A Number shall be a json type:
    TEST_F(JsonNumberTest, IsJsonType) 
    {
        EXPECT_TRUE( (is_json_type<Number>::value) );
    }
    
    
    TEST_F(JsonNumberTest, NumberCtorIntegers) {
        EXPECT_EQ("0", Number(0).string());
        EXPECT_EQ(0, Number(0).as<int>());
        EXPECT_EQ("0", Number(0L).string());
        EXPECT_EQ(0, Number(0L).as<int>());
        EXPECT_EQ("0", Number(0UL).string());
        EXPECT_EQ(0, Number(0UL).as<int>());
        EXPECT_EQ("0", Number(0ULL).string());
        EXPECT_EQ(0, Number(0ULL).as<int>());
    }
    
    TEST_F(JsonNumberTest, NumberCtorFloats) {
        EXPECT_EQ("0", Number(0.0).string());
        EXPECT_EQ(0, Number(0.0).as<double>());
        EXPECT_EQ("-1", Number(-1.0).string());
        EXPECT_EQ(-1, Number(-1.0).as<double>());
    }
    
    TEST_F(JsonNumberTest, NumberCtorNumberStrings) {
        const char* s = "123456";
        EXPECT_EQ(s, Number(s).string());
        EXPECT_EQ(123456, Number(s).as<int>());
        
        Number n = "1234";
        EXPECT_EQ("1234", n.string());
        EXPECT_EQ(1234, n.as<int>());
        
        EXPECT_EQ("0", Number("0").string());
        EXPECT_EQ(0, Number("0").as<int>());
        
        EXPECT_EQ("0.00", Number("0.00").string());
        EXPECT_EQ(0.0, Number("0.00").as<double>());
        
        EXPECT_EQ("-0.123e-12", Number("-0.123e-12").string());
        EXPECT_EQ(-0.123e-12, Number("-0.123e-12").as<double>());
    }
    
    TEST_F(JsonNumberTest, NumberCtor) {
        Number n1(1.2);
        EXPECT_EQ("1.2", n1.string());
        Number n2(-0.1234567890000);
        EXPECT_EQ("-0.123456789", n2.string());
        Number n4("123456789012345678901234567890");
        EXPECT_EQ("123456789012345678901234567890", n4.string());
        Number n5(0.0000);
        EXPECT_EQ("0", n5.string());
    }
    
    TEST_F(JsonNumberTest, UnsaneCtors) {
        // Must not compile or should yield warnings
        // Number(false);
        // EXPECT_EQ(1, Number(true).as<int>());
        // Number("abc");
    }    
    
    
    TEST_F(JsonNumberTest, NumberAssignment) {
        Number n;
        EXPECT_EQ("NaN", n.string());
        n = 1.3;
        EXPECT_EQ("1.3", n.string());        
    }
    
    TEST_F(JsonNumberTest, NumberConversion) {
        Number n0 = -0.1234567890L;
        EXPECT_EQ(-0.1234567890L, n0.as<long double>());        
        Number n1 = 12345678901234567890ULL;
        EXPECT_EQ(12345678901234567890ULL, n1.as<unsigned long long>());
        long double x = 0.000000000000000000001234567890L;
        Number n2 = x;
        EXPECT_EQ(x, n2.as<long double>());
        x = 0.1234567890123456789012345678901234567890L;
        Number n3 = x;
        EXPECT_EQ(x, n3.as<long double>());
    }
    
    TEST_F(JsonNumberTest, NumberPrecisionPreserving) {
        Number n0 = "1.00";
        EXPECT_EQ("1.00", n0.string());
    }
    
    TEST_F(JsonNumberTest, NumberComparisons) 
    {
        // Currently, only equality operators for Numbers containing
        // an integral number are implemented. Comparing other types
        // of numbers is undefined.
        //
        // Other operators are not implemented and cause a compiler error.
        
                
        Number n = 1;
        EXPECT_TRUE( Number(1) == n );
        EXPECT_TRUE( n == Number(1) );
        EXPECT_TRUE( Number(0) != n );
        EXPECT_TRUE( n != Number(0) );
        
        n = -1;
        EXPECT_TRUE( Number(-1) == n );
        EXPECT_TRUE( n == Number(-1) );
        EXPECT_TRUE( Number(0) != n );
        EXPECT_TRUE( n != Number(0) );

        n = 0;
        EXPECT_TRUE( Number(0) == n );
        EXPECT_TRUE( n == Number(0) );
        
        n = "123456789012345678901234567890";
        EXPECT_TRUE( Number("123456789012345678901234567890") == n );
        EXPECT_TRUE( n == Number("123456789012345678901234567890") );
        n = "-123456789012345678901234567890";
        EXPECT_TRUE( Number("-123456789012345678901234567890") == n );
        EXPECT_TRUE( n == Number("-123456789012345678901234567890") );
        
        int x = 1;
        EXPECT_TRUE( Number(1) == x );
        EXPECT_TRUE( x == Number(1) );
        EXPECT_TRUE( Number(0) != x );
        EXPECT_TRUE( x != Number(0) );
        
        x = -1;
        EXPECT_TRUE( Number(-1) == x );
        EXPECT_TRUE( x == Number(-1) );
        EXPECT_TRUE( Number(0) != x );
        EXPECT_TRUE( x != Number(0) );

        x = 0;
        EXPECT_TRUE( Number(0) == 0 );
        EXPECT_TRUE( x == Number(0) );
    }
    
}  // anonymous namespace

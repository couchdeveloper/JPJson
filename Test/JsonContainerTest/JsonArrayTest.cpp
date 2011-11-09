//
//  JsonArrayTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/4/11.
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

#include "json/value/value.hpp"
#include "gtest/gtest.h"


#include <stdio.h>
#include <time.h>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>


using namespace json;


namespace {
    
    
    typedef json::value<>   Value;
    typedef Value::array_t  Array;
    typedef Value::object_t Object;

    
    // The fixture for testing class json::Boolean:
    
    class JsonArrayTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonArrayTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonArrayTest() {
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
    
    // An Array shall be a json type:
    TEST_F(JsonArrayTest, IsJsonType) 
    {
        EXPECT_TRUE( (is_json_type<Array>::value) );
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<Array> >::value) );
    }
    
    
    TEST_F(JsonArrayTest, ArrayCtor) 
    {
        Array a;
        EXPECT_EQ(0, a.imp().size());
    }
    
    TEST_F(JsonArrayTest, ArrayVerifyProperSwap) 
    {
        // To check whether swap is implemented properly, we compare runtime characteristics:
        // A properly implemented swap is assumed to be very fast, where a std::swap just
        // uses a copy which is considerable slower.
        
        // Create and Array with 10000 strings:
        Array a;
        for (int i = 0; i < 10000; ++i) {
            a.push_back(Value("To check whether swap is implemented properly, we compare runtime characteristics"));
        }
        
        // Make a copy and measure time:
        clock_t t0 = clock();
        Array a_copy = a;
        t0 = clock() -  t0;

        // Make a swap and measure time:
        clock_t t1 = clock();
        Array a2;
        swap(a, a2);
        t1 = clock() -  t1;
        
        std::cout << "INFO: elapsed time for Array copy: " << t0 / (double)(CLOCKS_PER_SEC) * 1.0e3 << " ms" << std::endl;
        std::cout << "INFO: elapsed time for Array swap: " << t1 / (double)(CLOCKS_PER_SEC) * 1.0e3 << " ms" << std::endl;
        // swap shall be much faster than copy:
        EXPECT_TRUE( t1 * 100 < t0 );
    }
    
#if !defined (BOOST_NO_RVALUE_REFERENCES)
    
    TEST_F(JsonArrayTest, testMoveSemanticsForArray)
    {
        Array a;
        a.push_back(Value("abcd"));
        a.push_back(Value(1.0));
        a.push_back(Value(false));
        a.push_back(Value(null));
        
        Array safedCopy = a;
        EXPECT_TRUE(a == safedCopy);
        
        Array a2(std::move(a));        
        EXPECT_TRUE(a2 == safedCopy);
        
        // original array a shall be moved - thus it shall be empty:
        EXPECT_TRUE(a.size() == 0);
    }

#endif    
}
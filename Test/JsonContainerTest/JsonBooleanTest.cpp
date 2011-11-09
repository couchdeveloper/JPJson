//
//  JsonBooleanTest.cpp
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

#include "json/value/Boolean.hpp"
#include "gtest/gtest.h"

// for testing
#include "SafeBool.hpp"
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>


using namespace json;


namespace {
    
    
    // The fixture for testing class json::Boolean:
    
    class JsonBooleanTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonBooleanTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonBooleanTest() {
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
    
    // A Boolean shall be convertible to a bool:
    TEST_F(JsonBooleanTest, BooleanIsConvertibleTo_bool) 
    {
        EXPECT_TRUE( (boost::is_convertible<Boolean, bool>::value) );
    }
    
    // A Boolean shall be a json type:
    TEST_F(JsonBooleanTest, IsJsonType) 
    {
        EXPECT_TRUE( (is_json_type<Boolean>::value) );
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<Boolean> >::value) );
    }
    
    // A Boolean should not be convertible to a int or pointer:
    TEST_F(JsonBooleanTest, BooleanShouldNotConvertibleTo_int_or_pointter) 
    {
        EXPECT_TRUE( (boost::is_convertible<Boolean, int>::value) == false );
        EXPECT_TRUE( (boost::is_convertible<Boolean, void*>::value) == false );
    }

    
    TEST_F(JsonBooleanTest, BooleanCtor) 
    {
        Boolean b; // ctor
        EXPECT_TRUE( bool(b) == false);
        
        // Boolean(const bool& b)
        EXPECT_TRUE( bool(Boolean(true)) == true );
        EXPECT_TRUE( bool(Boolean(false)) == false );
        
        
        Boolean b1 = true;
        EXPECT_TRUE( bool(b1) == true );
        Boolean b2 = false;
        EXPECT_TRUE( bool(b2) == false );
        
        // Boolean(const Boolean&)
        Boolean b3 = b1;
        EXPECT_TRUE( bool(b1) == bool(b3) );
        
        Boolean b4(b1);
        EXPECT_TRUE( bool(b1) == bool(b4) );
    }
    
    
    
    TEST_F(JsonBooleanTest, BooleanCtorWithUserDefinedConvertibleToBool) 
    {
        // Check if custom types which are convertible to bool can be used in ctor
        // This is primarely a compile time test, it shall compile. However, implicit 
        // conversion e.g. int -> bool shall not be allowed.
        
        Boolean bt(SafeBool(true));
        Boolean bf(SafeBool(false));
        EXPECT_EQ(true,  (bool(bt) == true));
        EXPECT_EQ(true,  (bool(bf) == false));
        
        // Boolean is as well a SafeBool, so test it as well:
        EXPECT_EQ(true,  (bool(Boolean(bt)) == true));
        EXPECT_EQ(true,  (bool(Boolean(bf)) == false));
        
        //Boolean b_wrong(0);  // if this compiles, implicit conversions from int to bool are enabled, which shall not be allowed!
    } 
    
    TEST_F(JsonBooleanTest, BooleanInBoolExpressions) {
        Boolean bt = true;
        Boolean bf = false;
        EXPECT_EQ(1, (bt ? 1 : 0) );
        EXPECT_EQ(1, (bf ? 0 : 1) );
    } 
    
    
    TEST_F(JsonBooleanTest, BooleanCtorMustGiveCompilerErrors) 
    {
#if 0
        enum { False, True, };
        Boolean b0(0);
        Boolean b1(1);
        Boolean b2((void*)0);
        Boolean b3("1");
        Boolean b4(False);
        
        Boolean b = true; // shall compile!
        if (b > 0) {}
        if (b < 0) {}
        if (b >= 0) {}
        if (b <= 0) {}
        if (b != 0) {}  // shall compile!
        if (b == 0) {}  // shall compile!
        
        if (b > false) {}
        if (b < false) {}
        if (b >= false) {}
        if (b <= false) {}
        if (b != false) {} // shall compile!
        if (b == false) {} // shall compile!
        
        delete b;
        
        // Boolean -> T  conversions:
        int x = b;
#endif    
    }
    
    TEST_F(JsonBooleanTest, BooleanBoolExpression) 
    {
        Boolean bt = true;
        Boolean bf = false;
        const bool true_ = true;
        const bool false_ = false;
        
        if (bt == true) {}
        EXPECT_EQ(true, bt == true );
        EXPECT_EQ(true, (bt != false) );
        EXPECT_EQ(true, (true == bt) );
        EXPECT_EQ(true, (false != bt) );        
        
        
        EXPECT_EQ(true, (bt == true_) );
        EXPECT_EQ(true, (bt != false_) );
        EXPECT_EQ(true, (true_ == bt) );
        EXPECT_EQ(true, (false_ != bt) );
        
        EXPECT_EQ(true, (bt != true_) == false);
        EXPECT_EQ(true, (bt == false_) == false);
        EXPECT_EQ(true, (true_ != bt) == false);
        EXPECT_EQ(true, (false_ == bt) == false);
        
        EXPECT_EQ(true, (bt == bt) );
        EXPECT_EQ(true, (bt != bf) );
        EXPECT_EQ(true, (bt != bt) == false );
        EXPECT_EQ(true, (bt == bf) == false );
    }
    
    TEST_F(JsonBooleanTest, BooleanBoolUnaryExpression) 
    {
        Boolean bt = true;
        Boolean bf = false;
        
        EXPECT_EQ(true, (bt) );
        EXPECT_EQ(true, (!bf) );
        EXPECT_EQ(true, (!bt) == false);
        EXPECT_EQ(true, (bf) == false);
        
    }
    

}  // anonymous namespace
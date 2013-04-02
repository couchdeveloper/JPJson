//
//  BooleanTest.cpp
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
#include <gtest/gtest.h>

// for testing
#include "SafeBool.hpp"
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <type_traits>


using namespace json;


namespace {
    
    
    // The fixture for testing class json::Boolean:
    
    class BooleanTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        BooleanTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~BooleanTest() {
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
    
    
//    // A Boolean shall be a json type:
//    TEST_F(BooleanTest, IsJsonType)
//    {
//        EXPECT_TRUE( (is_json_type<Boolean>::value) );
//    }
    
    // Construction
    TEST_F(BooleanTest, Construction)
    {
        EXPECT_TRUE( (std::is_default_constructible<Boolean>::value) );
        EXPECT_TRUE( (std::is_nothrow_default_constructible<Boolean>::value) );
        EXPECT_TRUE( (std::is_copy_constructible<Boolean>::value) );
        EXPECT_TRUE( (std::is_nothrow_copy_constructible<Boolean>::value) );
        EXPECT_TRUE( (std::is_nothrow_move_constructible<Boolean>::value) );

        EXPECT_TRUE( (std::is_constructible<Boolean, bool>::value) );
        EXPECT_TRUE( (std::is_nothrow_constructible<Boolean, bool>::value) );
        
        EXPECT_FALSE( (std::is_constructible<Boolean, int>::value) );
        EXPECT_FALSE( (std::is_constructible<Boolean, void*>::value) );
        EXPECT_FALSE( (std::is_constructible<Boolean, decltype("abc")>::value) );
        EXPECT_FALSE( (std::is_constructible<Boolean, char*>::value) );
        

#if 0
        Boolean b10 = Boolean();// OK
        Boolean b11 = b10;      // OK
        Boolean b12;            // OK - but state is undetermined
        const char* s = "abc";
        Boolean b0 = s;         // error
        Boolean b1 = "0";       // error
        Boolean b3 = U"1";      // error
        Boolean b4 = 1;         // error
        Boolean b5 = (void*)(0);// error
        Boolean b6 = 0L;        // error
        Boolean b7 = 'a';       // error
#endif
    }
    
    
    // Assignment
    TEST_F(BooleanTest, Assignment)
    {
        EXPECT_TRUE( (std::is_nothrow_copy_assignable<Boolean>::value == true) );
        EXPECT_TRUE( (std::is_nothrow_move_assignable<Boolean>::value == true) );
        
        EXPECT_TRUE( (std::is_assignable<Boolean&, Boolean>::value) );
        EXPECT_TRUE( (std::is_nothrow_assignable<Boolean&, Boolean>::value) );
        EXPECT_TRUE( (std::is_assignable<Boolean&, bool>::value) );
        EXPECT_TRUE( (std::is_nothrow_assignable<Boolean&, bool>::value) );
        
        EXPECT_TRUE( (std::is_assignable<Boolean&, int>::value == false) );
        EXPECT_TRUE( (std::is_assignable<Boolean&, void*>::value == false) );
    }
    
    
    // A Boolean shall be convertible and assignable to a bool:
    TEST_F(BooleanTest, BooleanIsConvertibleTo_bool) 
    {
        EXPECT_TRUE( (std::is_convertible<Boolean, bool>::value) );
        EXPECT_TRUE( (std::is_assignable<bool&, Boolean>::value) );
    }
    
    // A Boolean shall not be convertible or assignable to an int or pointer:
    TEST_F(BooleanTest, BooleanShouldNotConvertibleTo_int_or_pointter) 
    {
        EXPECT_TRUE( (std::is_convertible<Boolean, int>::value) == false );
        EXPECT_TRUE( (std::is_convertible<Boolean, void*>::value) == false );
        
        EXPECT_TRUE( (std::is_assignable<int&, Boolean>::value == false) );
        EXPECT_TRUE( (std::is_assignable<void*&, Boolean>::value == false) );
    }

    // An int shall not be convertible to a Boolean:
    TEST_F(BooleanTest, intShallNotBeConvertibleToBoolean)
    {
        EXPECT_TRUE( (std::is_convertible<int, Boolean>::value) == false );
    }
    
    // A pointer shall not be convertible to a Boolean:
    TEST_F(BooleanTest, pointerShallNotBeConvertibleToBoolean)
    {
        EXPECT_TRUE( (std::is_convertible<void*, Boolean>::value) == false );
    }

    

    TEST_F(BooleanTest, ZeroInitialization)
    {
        // Zero initialization
        uint8_t buffer0[sizeof(Boolean)];
        memset(buffer0, 0x17, sizeof(buffer0));
        new (buffer0) Boolean();  // default ctor
        EXPECT_EQ(0, buffer0[0]);
        uint8_t buffer1[sizeof(bool)];
        memset(buffer1, 0x17, sizeof(buffer1));
        new (buffer1) bool();
        EXPECT_EQ(0, buffer1[0]);
    }
    
    
    TEST_F(BooleanTest, BooleanCtor)
    {
        Boolean b = Boolean(true); // default ctor
        EXPECT_TRUE( bool(b) == true);
        
        Boolean b1 = b; // copy ctor
        EXPECT_TRUE( bool(b1) == true);
        
        // Boolean(const bool& b)
        EXPECT_TRUE( bool(Boolean(true)) == true );
        EXPECT_TRUE( bool(Boolean(false)) == false );
        
        Boolean b2 = true;
        EXPECT_TRUE( bool(b2) == true );
        Boolean b3 = false;
        EXPECT_TRUE( bool(b3) == false );
        
        // Boolean(const Boolean&)
        Boolean b4 = b1;
        EXPECT_TRUE( bool(b1) == bool(b4) );
        
        Boolean b5(b1);
        EXPECT_TRUE( bool(b1) == bool(b5) );
    }
    
    

    TEST_F(BooleanTest, BooleanCtorWithUserDefinedConvertibleToBool)
    {
        // !! Changed *)
        // Check if custom types which are convertible to bool can be used in ctor
        // This is primarely a compile time test, it shall compile. However, implicit 
        // conversion e.g. int -> bool shall not be allowed.
        
        // *) Custum types which are convertible to bool must use an explicit
        // static_cast to bool:
        bool OK = SafeBool(true);
        Boolean bt(OK);
        Boolean bf(static_cast<bool>(SafeBool(false)));
        
        
        //EXPECT_TRUE( (std::is_constructible<Boolean, SafeBool>::value) );
        EXPECT_FALSE( (std::is_constructible<Boolean, SafeBool>::value) );
        
        //Boolean bt(SafeBool(true));  // *) Shall not compile
        //Boolean bf(SafeBool(false)); // *) Shall not compile
        EXPECT_EQ(true,  (bool(bt) == true));
        EXPECT_EQ(true,  (bool(bf) == false));
        
        // Boolean is as well a SafeBool, so test it as well:
        EXPECT_EQ(true,  (bool(Boolean(bt)) == true));
        EXPECT_EQ(true,  (bool(Boolean(bf)) == false));
        
        //Boolean b_wrong(0);  // if this compiles, implicit conversions from int to bool are enabled, which shall not be allowed!
    }

    
    TEST_F(BooleanTest, BooleanInBoolExpressions) {
        Boolean bt = true;
        Boolean bf = false;
        EXPECT_EQ(1, (bt ? 1 : 0) );
        EXPECT_EQ(1, (bf ? 0 : 1) );
    } 
    
    
    TEST_F(BooleanTest, BooleanCtorMustGiveCompilerErrors) 
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
    
    TEST_F(BooleanTest, BooleanBoolExpression) 
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
    
    TEST_F(BooleanTest, BooleanBoolUnaryExpression) 
    {
        Boolean bt = true;
        Boolean bf = false;
        
        EXPECT_EQ(true, (bt) );
        EXPECT_EQ(true, (!bf) );
        EXPECT_EQ(true, (!bt) == false);
        EXPECT_EQ(true, (bf) == false);
        
    }
    

}  // anonymous namespace
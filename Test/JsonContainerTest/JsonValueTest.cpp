//
//  JsonValueTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/10/11.
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

// for testing
#include "SafeBool.hpp"
#include "utilities/timer.hpp"
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>


using namespace json;


namespace {
    
    typedef json::value<>   Value;
    typedef Value::array_t  Array;
    typedef Value::object_t Object;
    typedef Value::string_t String;
    

    // The fixture for testing class json::Boolean:
    
    class JsonValueTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonValueTest() {
            // You can do set-up work for each test here.
            // A Value shall be a json type:
            Value v1 = Array();
            Value v2(move(v1));
        }
        
        virtual ~JsonValueTest() {
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
    
    
    // A Value shall be a json type:
    TEST_F(JsonValueTest, IsJsonType) 
    {
        EXPECT_TRUE( (is_json_type<Value>::value) );
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<Value> >::value) );
    }
    

    
    //
    // Construction
    //
    // A Value shall be constructable from the representation of primitive
    // json types, bool, Null, string, number and from a Value instance.
    //
    
    TEST_F(JsonValueTest, ConstructorsQuickTest) 
    {
        // Default
        Value v1;                       
        EXPECT_TRUE( (v1.is_type<Null>()) );
        
        // Copy
        Value v2(v1);                   
        EXPECT_TRUE( (v2.is_type<Null>()) );
        Value v3 = v1;                  
        EXPECT_TRUE( (v3.is_type<Null>()) );

        // Explicit Type Conversions Ctors with json types:
        Null n;
        Boolean b;
        String s;
        Number x;
        Array a;
        Object o;
        Value v10(n);              
        EXPECT_TRUE( (v10.is_type<Null>()) );
        Value v11(b);           
        EXPECT_TRUE( (v11.is_type<Boolean>()));
        Value v12(x);           
        EXPECT_TRUE( (v12.is_type<Number>()) );
        Value v13(s);       
        EXPECT_TRUE( (v13.is_type<String>()) );
        Value v14(a);             
        EXPECT_TRUE( (v14.is_type<Array>()) );
        Value v15(o);            
        EXPECT_TRUE( (v15.is_type<Object>()) );
        
        
        // Explicit Type Conversions Ctors with non-json-types
        Value v20(true);                
        EXPECT_TRUE( (v20.is_type<Boolean>()) );
        Value v21("abc");               
        EXPECT_TRUE( (v21.is_type<String>()) );
        Value v22(1.0);                 
        EXPECT_TRUE( (v22.is_type<Number>()) );
        Value v23(1);                   
        EXPECT_TRUE( (v23.is_type<Number>()) );
        
        
        // Implicit Type Conversion Ctors with json types:
        Value v30 = n;              
        EXPECT_TRUE( (v30.is_type<Null>()) );
        Value v31 = b;           
        EXPECT_TRUE( (v31.is_type<Boolean>()));
        Value v32 = x;           
        EXPECT_TRUE( (v32.is_type<Number>()) );
        Value v33 = s;       
        EXPECT_TRUE( (v33.is_type<String>()) );
        Value v34 = a;             
        EXPECT_TRUE( (v34.is_type<Array>()) );
        Value v35 = o;           
        EXPECT_TRUE( (v35.is_type<Object>()) );
    
        // Implicit Type Conversions Ctors with non-json-types
        Value v40 = true;                
        EXPECT_TRUE( (v40.is_type<Boolean>()) );
        Value v41 = "abc";               
        EXPECT_TRUE( (v41.is_type<String>()) );
        Value v42 = 1.0;                 
        EXPECT_TRUE( (v42.is_type<Number>()) );
        Value v43 = 1;                   
        EXPECT_TRUE( (v43.is_type<Number>()) );
        
    } 
    
    
    
    TEST_F(JsonValueTest, VariantTypeNames) 
    {
        EXPECT_EQ("Null", Value( (Null()) ).type_name());
        EXPECT_EQ("Boolean", Value( (Boolean()) ).type_name());
        EXPECT_EQ("Number", Value( (Number()) ).type_name());
        EXPECT_EQ("String", Value( (String()) ).type_name());
        EXPECT_EQ("Array", Value( (Array()) ).type_name());
        EXPECT_EQ("Object", Value( (Object()) ).type_name());
    }
    
    
    

    
    // Default ctor shall contruct a Null.
    TEST_F(JsonValueTest, DefaultCtor) {
        Value v;
        EXPECT_TRUE(v.is_type<Null>());
    }
    
    TEST_F(JsonValueTest, BooleanConstruction) 
    {
        // A Value shall be constructable from the representation of primitive
        // json types, bool, Null, string, number and from a Value instance.
        
        // Boolean
        Value b0(true);
        Value b1 (false);
        Value b2 = true;
        Value b3 = false; 
        Value b4(b0);
        Value b5 = b0;
        
        EXPECT_TRUE( (b0.is_type<Boolean>() and b0.as<Boolean>() == true) );
        EXPECT_TRUE( (b1.is_type<Boolean>() and b1.as<Boolean>() == false) );
        EXPECT_TRUE( (b2.is_type<Boolean>() and b2.as<Boolean>() == true) );
        EXPECT_TRUE( (b3.is_type<Boolean>() and b3.as<Boolean>() == false) );
        EXPECT_TRUE( (b4.is_type<Boolean>() and b4.as<Boolean>() == b0.as<Boolean>()) );
        EXPECT_TRUE( (b5.is_type<Boolean>() and b5.as<Boolean>() == b0.as<Boolean>()) );
    }
    
    TEST_F(JsonValueTest, NullConstruction) 
    {
        // Boolean
        Value n0(null);
        Value n1 = null;
        Value n2(n0);
        Value n3 = n0;
        
        EXPECT_TRUE( n0.is_type<Null>() );
        EXPECT_TRUE( n1.is_type<Null>() );
        EXPECT_TRUE( n2.is_type<Null>() );
        EXPECT_TRUE( n3.is_type<Null>() );
    }
    
    TEST_F(JsonValueTest, StringConstruction) 
    {
        // String
        Value s0("0abcd");
        Value s1 = "1abcd";        
        Value s2(std::string("2abcd"));
        Value s3 = std::string("3abcd");
        Value s4(s0);
        Value s5 = s0;
        
        EXPECT_TRUE( s0.is_type<String>() );
        EXPECT_TRUE( s1.is_type<String>() );
        EXPECT_TRUE( s2.is_type<String>() );
        EXPECT_TRUE( s3.is_type<String>() );
        EXPECT_TRUE( s4.is_type<String>() );
        
        EXPECT_TRUE( (s0.as<String>() == "0abcd") );
        EXPECT_TRUE( (s1.as<String>() == "1abcd") );
        EXPECT_TRUE( (s2.as<String>() == "2abcd") );
        EXPECT_TRUE( (s3.as<String>() == "3abcd") );
        EXPECT_TRUE( (s4.as<String>() == s0.as<String>()) );
        EXPECT_TRUE( (s5.as<String>() == s0.as<String>()) );
    }
    
    TEST_F(JsonValueTest, NumberConstruction) 
    {
        // Number
        Value x0(1);
        Value x1 = 2;
        
        Value x2(1.1);
        Value x3 = 2.1;
        
        
        EXPECT_TRUE( x0.is_type<Number>() );
        EXPECT_TRUE( x1.is_type<Number>() );
        EXPECT_TRUE( x2.is_type<Number>() );
        EXPECT_TRUE( x3.is_type<Number>() );

        EXPECT_TRUE( x0.as<int>() == 1);
        EXPECT_TRUE( x1.as<int>() == 2);
        EXPECT_DOUBLE_EQ( 1.1, x2.as<double>() );
        EXPECT_DOUBLE_EQ( 2.1, x3.as<double>() );
    }
    
    TEST_F(JsonValueTest, ArrayConstruction) 
    {
        // Test whether a Value can be constructer from an Array.
        // This test should not test the constrution of an Array, though.
        
        // Create an example array:
        Array a;
        a.push_back("String 1");
        a.push_back(1);
        a.push_back(false);
        
        Value v0(a);
        EXPECT_TRUE( v0.is_type<Array>() );
        EXPECT_TRUE( v0.as<Array>() == a );
                
        Value v1 = a;
        EXPECT_TRUE( v1.is_type<Array>() );
        EXPECT_TRUE( v0.as<Array>() == a );
    }
    
    
    TEST_F(JsonValueTest, ObjectConstruction) 
    {
        // Test whether a Value can be constructer from an Object.
        // This test should not test the features of an Object, but
        // it is not avoidable to rely on propper functionality of
        // Objects.
        
        // Create an example object:
        Object o;
        o.insert("key1", 1);
        o.insert("key2", 2);
        o.insert("key3", 3);
        
        Value v0(o);
        EXPECT_TRUE( v0.is_type<Object>() );
        EXPECT_TRUE( v0.as<Object>()["key1"].as<int>() == 1 );
        EXPECT_TRUE( v0.as<Object>()["key2"].as<int>() == 2 );
        EXPECT_TRUE( v0.as<Object>()["key3"].as<int>() == 3 );
        
        Value v1 = o;
        EXPECT_TRUE( v1.is_type<Object>() );
        EXPECT_TRUE( v1.as<Object>()["key1"].as<int>() == 1 );
        EXPECT_TRUE( v1.as<Object>()["key2"].as<int>() == 2 );
        EXPECT_TRUE( v1.as<Object>()["key3"].as<int>() == 3 );
    }
    

    TEST_F(JsonValueTest, ComparisonOperators1)
    {
        // Compare a Value v1 with a Value v2
        
        Value v1;
        Value v2 = 1.0;
        
        EXPECT_FALSE(  v1 == v2  );
        EXPECT_TRUE(  v1 != v2  );
        
        Value v3;
        EXPECT_TRUE(  v3 == v1  );
        EXPECT_FALSE(  v3 != v1  );
    }

    TEST_F(JsonValueTest, ComparisonOperatorsWithNull) 
    {
        // Comapre a Value v with a value of Null
        
        Null x;
        Value v; // which equals Null
        // lhv = v:
        EXPECT_TRUE(   v   ==   Value(null)  );
        EXPECT_TRUE(   v   ==   null  );
        EXPECT_FALSE(  v   !=   Value(null)  );
        EXPECT_FALSE(  v   !=   null  );
        
        // lhv = Value or a value
        EXPECT_TRUE(  Value(null)  ==  v  );
        EXPECT_TRUE(  null         ==  v  );
        EXPECT_FALSE( Value(null)  !=  v  );
        EXPECT_FALSE( null         !=  v  );
        

        Value v2 = "abcde"; // which actual type is a string
    
        EXPECT_FALSE(  v2   ==   Value(null)  );
        EXPECT_FALSE(  v2   ==   null  );
        EXPECT_TRUE(   v2   !=   Value(null)  );
        EXPECT_TRUE(   v2   !=   null  );
        
        EXPECT_FALSE( Value(null)  ==  v2  );
        EXPECT_FALSE( null         ==  v2  );
        EXPECT_TRUE(  Value(null)  !=  v2  );
        EXPECT_TRUE(  null         !=  v2  );
    } 

    TEST_F(JsonValueTest, ComparisonOperatorsWithBoolean) 
    {
        // Comapre a Value v with a value of Null
        
        Boolean bf = false;
        Boolean bt = true;
        Value vf = false;
        Value vt = true;
        
        EXPECT_TRUE(   vt   ==   Value(true)  );
        EXPECT_TRUE(   vt   !=   Value(false)  );
        EXPECT_TRUE(   vf   ==   Value(false)  );
        EXPECT_TRUE(   vf   !=   Value(true)  );
        
        EXPECT_TRUE(  Value(true)   ==   vt  );
        EXPECT_TRUE(  Value(false)  !=   vt  );
        EXPECT_TRUE(  Value(false)  ==   vf  );
        EXPECT_TRUE(  Value(true)   !=   vf  );
        
        
        EXPECT_TRUE(   vt   ==   bt  );
        EXPECT_TRUE(   vt   !=   bf  );
        EXPECT_TRUE(   vf   ==   bf  );
        EXPECT_TRUE(   vf   !=   bt  );
        
        EXPECT_TRUE(   bt   ==   vt  );
        EXPECT_TRUE(   bf   !=   vt  );
        EXPECT_TRUE(   bf   ==   vf  );
        EXPECT_TRUE(   bt   !=   vf  );
        
        EXPECT_TRUE(   vt   ==   true  );
        EXPECT_TRUE(   vt   !=   false );
        EXPECT_TRUE(   vf   ==   false );
        EXPECT_TRUE(   vf   !=   true  );
        
        EXPECT_TRUE(   true    ==   vt  );
        EXPECT_TRUE(   false   !=   vt  );
        EXPECT_TRUE(   false   ==   vf  );
        EXPECT_TRUE(   true    !=   vf  );
        
        Value vs = "abcde"; // which actual type is a string
        EXPECT_FALSE(  vs   ==   vt  );
        EXPECT_FALSE(  vs   ==   vf  );
        EXPECT_TRUE(   vs   !=   vt  );
        EXPECT_TRUE(   vs   !=   vf  );

        EXPECT_FALSE(  vt   ==   vs  );
        EXPECT_FALSE(  vf   ==   vs  );
        EXPECT_TRUE(   vt   !=   vs  );
        EXPECT_TRUE(   vf   !=   vs  );
        
    } 
    
    TEST_F(JsonValueTest, ComparisonOperatorsWithIntegralNumbers)
    {
        Value v1 = 1;
        Value v2 = 2;
        Value v3 = 1;
        
        EXPECT_FALSE(  v1  ==  v2  );
        EXPECT_TRUE (  v1  !=  v2  );
        EXPECT_TRUE (  v1  ==  v3  );
        EXPECT_FALSE ( v1  !=  v3  );
        
        Number n1 = 1;
        Number n2 = 2;
        
        EXPECT_TRUE (  v1  ==  n1  );
        EXPECT_FALSE(  v1  !=  n1  );
        EXPECT_TRUE (  n1  ==  v1  );
        EXPECT_FALSE(  n1  !=  v1  );
        
        EXPECT_FALSE(  v1  ==  n2  );
        EXPECT_TRUE (  v1  !=  n2  );
        EXPECT_FALSE(  n2  ==  v1  );
        EXPECT_TRUE (  n2  !=  v1  );

        EXPECT_TRUE (  v1  ==  1  );
        EXPECT_FALSE(  v1  !=  1  );
        EXPECT_TRUE (  1  ==  v1  );
        EXPECT_FALSE(  1  !=  v1  );
        
        EXPECT_FALSE(  v1  ==  2  );
        EXPECT_TRUE (  v1  !=  2  );
        EXPECT_FALSE(  2  ==  v1  );
        EXPECT_TRUE (  2  !=  v1  );
    }

    
    TEST_F(JsonValueTest, Swap)
    {
        // swapping two boost::variants has a significant performance issue:
        // If the concrete variant types do not match, a "general variant swap"
        // is performen. While this may lead to acceptable performance if
        // move semantic is implemented, for non-C++0x compilers and libraries
        // this becomes especially costly if the types have expensive 
        // constructors/destructors. Usually this involves 3x copy.
        //
        // In order to alleviate the problem, a free swap template function for 
        // json::value<P, E> has been implemented in namespace json.
        // It uses a special member function make_default() which returns a 
        // default constructed variant with the same actual type as *this. 
        // Creation of a default constructed variant and assigment to another 
        // default constructed variant of another tpye is assumed to be fast.
        //
        // As a result the function swap(variant1, variant2) should be acceptable
        // fast, althoug it is not as cheap as when swapping the concrete types
        // directly.
        
        // Create an Array with 10000 strings:
        Value v1 = Array();
        Array& a = v1.as<Array>();
        for (int i = 0; i < 10000; ++i) {
            a.push_back(Value("To check whether swap is implemented properly, we compare runtime characteristics"));
        }

        Value v2;
        
        utilities::timer t;
        bool result = true;
        t.start();
        for (int i = 0; i < 1000; ++i) {
            swap(v1, v2);   // should be acceptable fast
            
            if (i % 2) {
                if (v1.as<Array>().size() != 10000) {
                    result = false;
                    break;
                }
            }
            
        }
        t.stop();
        double sec = t.seconds();
        
        EXPECT_TRUE( result );
#if defined DEBUG        
        EXPECT_LT( sec, 0.01 );
#else
        EXPECT_LT( sec, 0.001 );
#endif        
        
    }
    
}
//
//  JsonObjectTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/5/11.
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
#include <sstream>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <typeinfo>

#include "json/unicode/unicode_traits.hpp"



//
//  Objective:  Test instantiations of json::object template class,
//              in the context of json.
//

using namespace json;


namespace {
    
    // The json::object class is defined through json::value.
    
    typedef json::value<>   Value;
    typedef Value::array_t  Array;
    typedef Value::object_t Object;
    typedef Value::string_t String;
    
    
    // The fixture for testing class Object:
    
    class JsonObjectTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonObjectTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonObjectTest() {
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
    
    // An Object shall be a json type:
    TEST_F(JsonObjectTest, IsJsonType) 
    {
        EXPECT_TRUE( (is_json_type<Object>::value) );
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<Object> >::value) );
    }

    
    TEST_F(JsonObjectTest, ExposedTypes) {        
        typedef Object::imp_t imp_t;
        typedef Object::key_t key_t;    // shall be json_policies::string_imp_t<string_encoding_t<char_type> >
        typedef Object::value_t value_t;   // shall be Value
        typedef Object::iterator iterator;
        typedef Object::const_iterator const_iterator;
        typedef Object::element element;
        
        typedef unicode::encoding_traits<Value::string_encoding_t>::code_unit_type char_t;
        typedef boost::mpl::apply<Value::policies_t::string_imp_tt, char_t>::type string_t;
        
        EXPECT_TRUE( (boost::is_same<key_t, String>::value) );
        EXPECT_TRUE( (boost::is_same<String, string_t>::value) );
        EXPECT_TRUE( (boost::is_same<String::char_type, char_t>::value) );
        
        EXPECT_TRUE( (boost::is_same<value_t, Value>::value) );
        EXPECT_TRUE( (boost::is_same<value_t, Value>::value) );        
        EXPECT_TRUE( (boost::is_same< boost::remove_const<element::first_type>::type, key_t>::value) );
        EXPECT_TRUE( (boost::is_same<element::second_type, value_t>::value) );        
    }
    
    TEST_F(JsonObjectTest, ObjectCtor) 
    {
        Object o0;
        EXPECT_EQ(0, o0.size());
        
        Object o1(o0);
        EXPECT_TRUE(o1.size() == o0.size());
        
        Object o2 = o0;
        EXPECT_TRUE(o2.size() == o0.size());
    }

    
    TEST_F(JsonObjectTest, ElementConstruction) 
    {
        // This test should merely compile
        
        typedef Object::element element_t;        
        
        element_t e0("key", Value(1));
        element_t e1("key", 1);
        element_t e2("key", "a string");
        element_t e3("key", false);
    }
 
    
    TEST_F(JsonObjectTest, Insertion) 
    {
        // Note: this test requires that Value functions
        // properly.

        typedef Object::element element;
        typedef std::pair<Object::iterator, bool> result_t;   
        
        Object o;
        result_t r = o.insert(element("key0", 0));
        EXPECT_TRUE(  r.second  );
        EXPECT_EQ( "key0" ,     (*(r.first)).first );
        EXPECT_EQ(  0 ,         (*(r.first)).second.as<int>()  );
        EXPECT_EQ(  1 ,          o.size());
        
        r = o.insert(element("key1", 1));
        EXPECT_TRUE(  r.second  );
        EXPECT_EQ( "key1" ,     (*(r.first)).first );
        EXPECT_EQ(  1 ,         (*(r.first)).second.as<int>()  );
        EXPECT_EQ(  2,          o.size());
        
        r = o.insert(element("key0", -1));
        EXPECT_TRUE(  r.second == false  );
        EXPECT_EQ( "key0" ,     (*(r.first)).first );
        EXPECT_EQ(  0 ,         (*(r.first)).second.as<int>()  );
        EXPECT_EQ(  2,          o.size());
        
        
        // TODO: add a couple more insertion tests
        
    }
    
    
    TEST_F(JsonObjectTest, QueryAccessors)
    {
        // Test the functions size(), has_key() and find()

        typedef Object::element element;
        typedef Object::iterator iterator;
        typedef std::pair<Object::iterator, bool> result_t;
        
        char buffer[256];
        std::vector<String> keys;
        const int N = 100;
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key%d", i);
            keys.push_back(buffer);
        }
        
        std::vector<String>::iterator first = keys.begin();
        std::vector<String>::iterator last = keys.end();
        
        // test size()
        Object o;        
        EXPECT_EQ(0,  o.size() );
        int i = 0;
        while (first != last) {
            result_t result = o.insert(element(*first, i));
            EXPECT_TRUE( result.second );
            ++i;
            EXPECT_EQ(i,  o.size() );
            
            if (not result.second) {
                std::cout << "key-value pair which shall be inserted: ";
                std::cout << "'" << (*first) << "': " << i << std::endl;
                std::cout << "Current object content: " << std::endl;
                iterator i = o.begin();
                iterator e = o.end();
                while (i != e) {
                    std::cout << "'" << (*i).first.c_str() << "': " << (*i).second << std::endl;
                    ++i;
                }
                break;
            }
            
            ++first;
        }
        
        ASSERT_EQ( 100, o.size() );
        
        // test has_key() and find()
        first = keys.begin();
        int v = 0;
        while (first != last) {
            iterator iter = o.find(*first);
            EXPECT_TRUE( iter != o.end() );
            EXPECT_TRUE( o.has_key(*first) );
            EXPECT_TRUE( (*iter).second == Value(v) );
            ++first;
            ++v;
        }
        EXPECT_TRUE( o.has_key("other") == false );
        EXPECT_TRUE( o.has_key("") == false );
        EXPECT_TRUE( o.find("xxx") == o.end() );
        
        
        iterator iter = o.find("");
        iterator end = o.end();
        const char* s = (*iter).first.c_str();
        
        EXPECT_TRUE( o.find("") == o.end() );
    }

    
    TEST_F(JsonObjectTest, ObjectVerifyProperSwap) 
    {
        // To check whether swap is implemented properly, we compare runtime characteristics:
        // A properly implemented swap is assumed to be very fast, where a std::swap just
        // uses a copy which is considerable slower.
        
        // Create an Object with 1000 key value pairs:
        Object o;
        for (int i = 0; i < 1000; ++i) {
            std::stringstream ss;
            ss << "key_" << i;
            o.insert(ss.str().c_str(), 
                     Value("To check whether swap is implemented properly, we compare runtime characteristics"));
        }
        
        // Make a copy and measure time:
        clock_t t0 = clock();
        Object o_copy = o;
        t0 = clock() -  t0;
        
        // Make a swap and measure time:
        clock_t t1 = clock();
        Object o2;
        swap(o, o2);
        t1 = clock() -  t1;
        
        std::cout << "INFO: elapsed time for Object copy: " << t0 / (double)(CLOCKS_PER_SEC) * 1.0e3 << " ms" << std::endl;
        std::cout << "INFO: elapsed time for Object swap: " << t1 / (double)(CLOCKS_PER_SEC) * 1.0e3 << " ms" << std::endl;
        // swap shall be much faster than copy:
        EXPECT_TRUE( t1 * 100 < t0 );
    }

    TEST_F(JsonObjectTest, testSubscriptionOperator)
    {
        Object o;
        o.insert("key0", Value("abcd"));
        o.insert("key1", Value(1.0));
        o.insert("key2", Value(false));
        o.insert("key3", Value(null));
        
        EXPECT_TRUE(o["key0"] == Value("abcd"));
        EXPECT_TRUE(o["key1"] == Value(1.0));
        EXPECT_TRUE(o["key2"] == Value(false));
        EXPECT_TRUE(o["key3"] == Value(null));
    }

 
 
#if !defined (BOOST_NO_RVALUE_REFERENCES)
    
    TEST_F(JsonObjectTest, testMoveSemanticsForObject)
    {
        Object o;
        o.insert("key0", Value("abcd"));
        o.insert("key1", Value(1.0));
        o.insert("key2", Value(false));
        o.insert("key3", Value(null));
                
        Object o2(std::move(a));        
        
        // original object o shall be moved - thus it shall be empty:
        EXPECT_TRUE(o.size() == 0);
        
        EXPECT_TRUE(o["key0"] == "abcd");
        EXPECT_TRUE(o["key1"] ==  1.0);
        EXPECT_TRUE(o["key2"] == false);
        EXPECT_TRUE(o["key3"] ==  null);
    }
    
#endif    
}
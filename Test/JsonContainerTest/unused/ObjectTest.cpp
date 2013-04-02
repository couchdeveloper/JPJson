//
//  ObjectTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/27/11.
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

#include "json/value/object.hpp"
#include <gtest/gtest.h>


//
//  Objective:  Test instantiations of json::object template class,
//              independently on other json classe, in particular
//              ckeck the type interface.
//
//              Only basic tests are performed here - more test are
//              defined in JsonObjectTest.cpp.
//


// for testing
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>


using namespace json;


namespace {
    
    
    // The fixture for testing class json::object:
    
    class ObjectTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        ObjectTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~ObjectTest() {
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
    
    // An object shall be a json type:
    TEST_F(ObjectTest, IsJsonType) 
    {
        typedef json::object<std::string, int, std::map<mpl::_, mpl::_> > object_t1;
        typedef json::object<std::string, int > object_t2;
        
        EXPECT_FALSE( (is_json_type<int>::value) );

        EXPECT_TRUE( (is_json_type<object_t1>::value) );
        EXPECT_TRUE( (is_json_type<object_t2>::value) );
        
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<object_t1> >::value) );
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<object_t2> >::value) );
        
    }
    
    struct test_value {};

    TEST_F(ObjectTest, ObjectTypes) 
    {
        typedef json::object<std::string, test_value, std::map<mpl::_, mpl::_> > object_t;
        
        EXPECT_TRUE( (boost::is_same< object_t::imp_t::key_type, 
                      std::map<std::string, test_value>::key_type >::value) );
        
        EXPECT_TRUE( (boost::is_same< object_t::imp_t::mapped_type, 
                      std::map<std::string, test_value>::mapped_type >::value) );
        
        EXPECT_TRUE( (boost::is_same<object_t::key_t, std::string>::value) );
        
        EXPECT_TRUE( (boost::is_same<object_t::value_t, test_value >::value) );
        
        
        EXPECT_TRUE( (boost::is_same<object_t::element, 
                      std::pair<const std::string, test_value> >::value) );
    }
    
    
    TEST_F(ObjectTest, DefaultConstructor) 
    {
        typedef json::object<std::string, test_value, std::map<mpl::_, mpl::_> > object_t;
        
        object_t o;
        
        EXPECT_EQ( 0, o.size() );        
        EXPECT_TRUE (o.begin() == o.end() );
    } 

    TEST_F(ObjectTest, CopyConstructor) 
    {
        typedef json::object<std::string, test_value, std::map<mpl::_, mpl::_> > object_t;
        
        object_t o1;
        object_t o2(o1);
        
        EXPECT_EQ( o1.size(), o2.size() );        
        EXPECT_TRUE (o2.begin() == o2.end() );
    } 
    

}
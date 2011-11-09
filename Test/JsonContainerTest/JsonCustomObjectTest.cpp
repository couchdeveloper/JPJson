//
//  JsonCustomObjectTest.cpp
//
//  Created by Andreas Grosam on 5/7/11.
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
#include "json/value/string.hpp"
#include "gtest/gtest.h"

#include <stdio.h>
#include <time.h>
#include <sstream>
#include <string>

#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>

#include <boost/tr1/unordered_map.hpp>


#warning The customization is currently broken
#if 0


using namespace json;

// -----------------------------------------------------------------------------
// Defining a custom implementation for json::Object
//
// Custom implementation types for the array and object classes can be
// defined using json policies, which is a template argument for the
// template class value.
//
//  The default policies are defined in struct json_policies as follows:
// 
// namespace json {
//    struct json_policies 
//    {
//        typedef std::vector<boost::mpl::_> array_imp_t;
//        typedef std::map<boost::mpl::_, boost::mpl::_> object_imp_t;
//    };
// }
//
// json::object is just a container facade. It's underlaying container requires
// to be signature compatible with a std::map. Although, only a few signatures 
// are used. 
//
// The default implementation is a std::map. It can be exchanged with any
// other associative container, though. If the other container is std::map
// signature-compatible, the required changes are quite easy to apply.
//
// For any other none-std associative container this can be accomplished by a 
// suitable adapter quite easily.
//
// In order to change the underlaying container for json::object with a 
// signature compatible associative container you just need to define your 
// own policy and pass it as a template argument to json::value:
//
//    namespace custom {
//        
//        class MyPolicies 
//        {
//            typedef std::vector<boost::mpl::_> array_imp_t;
//            typedef boost::unordered_map<boost::mpl::_, boost::mpl::_> object_imp_t;
//        };
//        
//    }
//
// The above example uses a boost::unordered_map as the underlaying container
// implementation for the json::object.
// json::value requires the policy to supply the type definitions array_imp_t and 
// object_imp_t as shown above.
//
// json:.value's type mechanerie ensures that json::value::array_t and 
// json::value::object_t are correctly calculated as a facade using the 
// specified underlaying container types. For convenience, the following 
// type definitions should be declared:
// 
//  typedef value<MyPolicies> Value;
//  typedef Value::array_t  Array;
//  typedef Value::object_t Object;
//  typedef Value::string_t String;
//
//
//
// For this test, we use a boost associative container, unordered_map,
// which is already signature compatible with std::map.
//
// Note:
// unordered_map requires a hash function for the key to be defined.
// For an Object, the key is type json::String, which is actually a typedef 
// for std::string. If the key is a std::string, a hash function is already
// defined. Otherwise, you must must be a hash function defined for String via 
// a custom overload:
//      std::size_t hash_value(const String& val);
//
// Defining Object with an unordered_map as the associative container type is
// now quite easy:

namespace custom {
    
    class MyPolicies 
    {
    public:
        typedef std::vector<boost::mpl::_>                          array_imp_tt;
        // Original: typedef std::map<boost::mpl::_, boost::mpl::_> object_imp_tt;
        typedef boost::unordered_map<boost::mpl::_, boost::mpl::_>  object_imp_tt;
        typedef std::basic_string<boost::mpl::_>                    string_imp_tt;        
    };
    
}

namespace {
    
    typedef json::value<custom::MyPolicies>   Value;
    typedef Value::array_t  Array;
    typedef Value::object_t Object;
    typedef Value::string_t String;

    
    class JsonCustomObjectTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonCustomObjectTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonCustomObjectTest() {
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
    TEST_F(JsonCustomObjectTest, IsJsonType) 
    {
        EXPECT_TRUE( (is_json_type<Object>::value) );
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<Object> >::value) );
    }
    
    
    TEST_F(JsonCustomObjectTest, ObjectCtor) 
    {
        Object o;
        EXPECT_EQ(0, o.imp().size());
        std::cout << "INFO: sizeof(Object) = " << sizeof(o) << std::endl;
    }
    
    TEST_F(JsonCustomObjectTest, ObjectVerifyProperSwap) 
    {
        // To check whether swap is implemented properly, we compare runtime characteristics:
        // A properly implemented swap is assumed to be very fast, where a std::swap just
        // uses a copy which is considerable slower.
        
        // Create an Object with 1000 key value pairs:
        Object o;
        for (int i = 0; i < 1000; ++i) {
            std::stringstream key;
            key << "key_" << i;
            o.insert(String(key.str()), 
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
    
    
    TEST_F(JsonCustomObjectTest, testSubscriptionOperator)
    {
        Object o;
        o.insert("key0", Value("abcd"));
        o.insert("key1", Value(1.0));
        o.insert("key2", Value(false));
        o.insert("key3", Value(null));
                
        EXPECT_TRUE(o["key0"] == "abcd");
        EXPECT_TRUE(o["key1"] == 1.0);
        EXPECT_TRUE(o["key2"] == false);
        EXPECT_TRUE(o["key3"] == null);
    }
    
#if !defined (BOOST_NO_RVALUE_REFERENCES)
    
    TEST_F(JsonCustomObjectTest, testMoveSemanticsForObject)
    {
        Object o;
        o.insert("key0", Value("abcd"));
        o.insert("key1", Value(1.0));
        o.insert("key2", Value(false));
        o.insert("key3", Value(null));
        
        Object o2(std::move(a));        
        
        // original object o shall be moved - thus it shall be empty:
        EXPECT_TRUE(o.size() == 0);
    }
#endif    
    
} // namespace 

#endif 

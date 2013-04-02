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
#include <gtest/gtest.h>
#include <vector>
#include <map>
#include <string>
#include <cassert>
// for testing
#include "SafeBool.hpp"
#include "utilities/timer.hpp"
#include <cstdlib>
#include "json/utility/mpl.hpp"


using namespace json;



namespace {
    
//    typedef json::value<>   Value;
//    
//    typedef Value::null_type Null;
//    typedef Value::boolean_type Boolean;
//    typedef Value::float_number_type FloatNumber;
//    typedef Value::integral_number_type IntNumber;
//    typedef Value::string_type String;
//    typedef Value::object_type Object;
//    typedef Value::array_type  Array;
//    
//    typedef Value::key_type     Key;
    
    

    // The fixture for testing class json::Boolean:
    
    class JsonValueTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonValueTest() {
            // You can do set-up work for each test here.
            // A Value shall be a json type:
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
    
    
//    // A Value shall be a json type:
//    TEST_F(JsonValueTest, IsJsonType) 
//    {
//        EXPECT_TRUE( (is_json_type<Value>::value) );
//    }
    

    TEST_F(JsonValueTest, TypeGeneration)
    {
        // Declare a JSON value type with default policies:
        typedef json::value<> value_t;
        
        
        typedef value_t::key_type key_t;
        typedef json::Null null_t;
        typedef json::Boolean boolean_t;
        typedef json::float_number<> float_number_t;
        typedef json::integral_number<> integral_number_t;
        typedef std::string string_t;
        typedef std::map<key_t, value_t> object_t;
        typedef std::vector<value_t>  array_t;
        
        EXPECT_TRUE( (std::is_same<null_t,              typename value_t::null_type>::value) );
        EXPECT_TRUE( (std::is_same<boolean_t,           typename value_t::boolean_type>::value) );
        EXPECT_TRUE( (std::is_same<float_number_t,      typename value_t::float_number_type>::value) );
        EXPECT_TRUE( (std::is_same<integral_number_t,   typename value_t::integral_number_type>::value) );
        EXPECT_TRUE( (std::is_same<string_t,            typename value_t::string_type>::value) );
        EXPECT_TRUE( (std::is_same<object_t,            typename value_t::object_type>::value) );
        EXPECT_TRUE( (std::is_same<array_t,             typename value_t::array_type>::value) );
    }
    
    
    TEST_F(JsonValueTest, DefaultPolicies)
    {
        // Class template json::value template parameter are "policies" which
        // determine the underlaying implementations for the JSON primitives
        // and JSON containers.
        
        // The defaul template parameters instantiate the following types:
        
        
        // Declare a JSON value type with default policies:
        typedef json::value<> value_t;
        
        typedef value_t::key_type key_t;
        typedef json::Null null_t;
        typedef json::Boolean boolean_t;
        typedef json::float_number<> float_number_t;
        typedef json::integral_number<> integral_number_t;
        typedef std::string string_t;
        typedef std::map<key_t, value_t> object_t;
        typedef std::vector<value_t>  array_t;
        
        
        EXPECT_TRUE( (std::is_same<json::Null,                  null_t>::value) );
        EXPECT_TRUE( (std::is_same<json::Boolean,               boolean_t>::value) );
        EXPECT_TRUE( (std::is_same<json::float_number<>,        float_number_t>::value) );
        EXPECT_TRUE( (std::is_same<json::integral_number<>,     integral_number_t>::value) );
        EXPECT_TRUE( (std::is_same<std::string,                 string_t>::value) );
        EXPECT_TRUE( (std::is_same<std::map<key_t, value_t>,    object_t>::value) );
        EXPECT_TRUE( (std::is_same<std::vector<value_t>,        array_t>::value) );
        EXPECT_TRUE( (std::is_same<std::string,                 key_t>::value) );
    }


    TEST_F(JsonValueTest, TypeTraits)
    {
        typedef json::value<> value_t;
        
//        EXPECT_TRUE( (json::is_json_type<typename value_t::null_type>::value) );
//        EXPECT_TRUE( (json::is_json_type<typename value_t::boolean_type>::value) );
//        EXPECT_TRUE( (json::is_json_type<typename value_t::float_number_type>::value) );
//        EXPECT_TRUE( (json::is_json_type<typename value_t::integral_number_type>::value) );
//        EXPECT_TRUE( (json::is_json_type<typename value_t::string_type>::value) );
//        EXPECT_TRUE( (json::is_json_type<typename value_t::object_type>::value) );
//        EXPECT_TRUE( (json::is_json_type<typename value_t::array_type>::value) );
    }
    
    
    //
    // Construction
    //
    // A Value shall be constructable from the representation of primitive
    // json types, bool, Null, string, number and from a Value instance.
    //
    
    TEST_F(JsonValueTest, ConstructorsQuickTest) 
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
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
        IntNumber i;
        FloatNumber x;
        Array a;
        Object o;
        Value v10(n);              
        EXPECT_TRUE( (v10.is_type<Null>()) );
        Value v11(b);           
        EXPECT_TRUE( (v11.is_type<Boolean>()));
        Value v12(i);
        EXPECT_TRUE( (v12.is_type<IntNumber>()) );
        Value v13(x);
        EXPECT_TRUE( (v13.is_type<FloatNumber>()) );        
        Value v14(s);
        EXPECT_TRUE( (v14.is_type<String>()) );
        Value v15(a);
        EXPECT_TRUE( (v15.is_type<Array>()) );
        Value v16(o);
        EXPECT_TRUE( (v16.is_type<Object>()) );
        
        
        // Explicit Type Conversions Ctors with non-json-types
        Value v20(true);                
        EXPECT_TRUE( (v20.is_type<Boolean>()) );
        Value v21("abc");               
        EXPECT_TRUE( (v21.is_type<String>()) );
        Value v22(1.0);                 
        EXPECT_TRUE( (v22.is_type<FloatNumber>()) );
        Value v23(1);                   
        EXPECT_TRUE( (v23.is_type<IntNumber>()) );
        
        
        // Implicit Type Conversion Ctors with json types:
        typedef Value::variant_type var;
        var sxy = "abc";
        static_assert(std::is_constructible<var, decltype("ab")>::value, "");
        
        Value v30 = n;              
        EXPECT_TRUE( (v30.is_type<Null>()) );
        Value v31 = b;           
        EXPECT_TRUE( (v31.is_type<Boolean>()));
        Value v32 = x;           
        EXPECT_TRUE( (v32.is_type<FloatNumber>()) );
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
        EXPECT_TRUE( (v42.is_type<FloatNumber>()) );
        Value v43 = 1;                   
        EXPECT_TRUE( (v43.is_type<IntNumber>()) );
        
    } 
    
    
    TEST_F(JsonValueTest, VariantTypeNames) 
    {
        // TODO: type_name is not yet implemented
        
//        EXPECT_EQ("Null", Value( (Null()) ).type_name());
//        EXPECT_EQ("Boolean", Value( (Boolean()) ).type_name());
//        EXPECT_EQ("Number", Value( (Number()) ).type_name());
//        EXPECT_EQ("String", Value( (String()) ).type_name());
//        EXPECT_EQ("Array", Value( (Array()) ).type_name());
//        EXPECT_EQ("Object", Value( (Object()) ).type_name());
    }

    
    // Default ctor shall contruct a Null.
    TEST_F(JsonValueTest, DefaultCtor)
    {
        typedef json::value<>   Value;
        
        Value v;
        EXPECT_TRUE(v.is_type<Null>());
    }
    
    
    TEST_F(JsonValueTest, ValueBooleanImpConstruction)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
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
    
    
    TEST_F(JsonValueTest, ValueNullImpConstruction)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        // Boolean
        Value n0(null);
        Value n1 = null;
        Value n2(n0);
        Value n3 = n0;
        
        bool isNull0 = n0.is_type<Null>();
        bool isNull1 = n1.is_type<Null>();
        bool isNull2 = n2.is_type<Null>();
        bool isNull3 = n3.is_type<Null>();
        
        EXPECT_TRUE(isNull0);
        EXPECT_TRUE(isNull1);
        EXPECT_TRUE(isNull2);
        EXPECT_TRUE(isNull3);
        
        EXPECT_TRUE( (n1.is_type<Null>()) );
        EXPECT_TRUE( (n2.is_type<Null>()) );
        EXPECT_TRUE( (n3.is_type<Null>()) );
        EXPECT_TRUE( (n0.is_type<Null>()) );
    }
    
    
    TEST_F(JsonValueTest, ValueStringImpConstruction)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
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
        
        EXPECT_EQ("0abcd", (s0.as<String>()) );
        EXPECT_EQ("1abcd", (s1.as<String>()) );
        EXPECT_EQ("2abcd", (s2.as<String>()) );
        EXPECT_EQ("3abcd", (s3.as<String>()) );
        EXPECT_EQ("0abcd", (s4.as<String>()) );
        EXPECT_EQ("0abcd", (s5.as<String>()) );
        
        EXPECT_TRUE( (s4.as<String>() == s0.as<String>()) );
        EXPECT_TRUE( (s5.as<String>() == s0.as<String>()) );
    }
    
    
    TEST_F(JsonValueTest, ValueNumberImpConstruction)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        // Integral Number
        Value x0(1);
        Value x1 = 2;
        EXPECT_TRUE( x0.is_type<IntNumber>() );
        EXPECT_TRUE( x1.is_type<IntNumber>() );
        EXPECT_TRUE( x0.as<long>() == 1);
        EXPECT_TRUE( x1.as<long>() == 2);
        
        // Float Number
        Value x2(1.1);
        Value x3 = 2.1;
        EXPECT_TRUE( x2.is_type<FloatNumber>() );
        EXPECT_TRUE( x3.is_type<FloatNumber>() );
        EXPECT_DOUBLE_EQ( 1.1, x2.as<double>() );
        EXPECT_DOUBLE_EQ( 2.1, x3.as<double>() );
    }
    
    
    TEST_F(JsonValueTest, ArrayConstruction1)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        // If we require a JSON Value whose type is a JSON Array, we usually
        // create a default constructed array type first. Type Array is the
        // underlaying implementation type for arrays - which is a std::vector
        // per defualt whose embedded type value_type equals Value.
                
        Array a;  // default ctor
        
        // ... and then subsequently insert elements. We know, Array is a
        // std::vector - thus the usual modfier member functions should work
        // as expected. More interestingly are the implicit conversions from
        // some argument to Value:
        a.push_back("String 1");  // implicitly converts "String 1" to Value whose imp type is String.
        EXPECT_TRUE(a.back().is_string());
        
        a.emplace_back(1); // implicitly converts 1 to Value whose imp type is IntNumber
        EXPECT_TRUE(a.back().is_integral_number());
        
        a.push_back(false); // implicitly converts false to Value whose imp type is Boolean
        EXPECT_TRUE(a.back().is_boolean());
        
        Value v = "string";
        EXPECT_TRUE(v.is_string());
        a.emplace_back(v);
        EXPECT_TRUE(a.back().is_string());
        
        a.emplace_back(std::move(v));
        EXPECT_TRUE(a.back().is_string());
        
        
        Value v0(a);
        EXPECT_TRUE( v0.is_type<Array>() );
        EXPECT_TRUE( v0.as<Array>() == a );
                
        Value v1 = a;
        EXPECT_TRUE( v1.is_type<Array>() );
        EXPECT_TRUE( v0.as<Array>() == a );

    
        Value v3(std::move(a));
        EXPECT_TRUE( v3.is_type<Array>() );
    }
    
    
    TEST_F(JsonValueTest, ArrayConstruction2)
    {
        typedef json::value<> Value;
        typedef typename Value::string_type String;
        typedef typename Value::null_type Null;
        typedef typename Value::boolean_type Boolean;
        typedef typename Value::integral_number_type IntNumber;
        typedef typename Value::float_number_type FloatNumber;
        typedef typename Value::object_type Object;
        
        typedef typename Value::array_type Array;
        
        Array array1;
        
        Array array2({json::null, false, "String", 1, 1.1});
        EXPECT_TRUE(array2[0].is_null());
        EXPECT_TRUE(array2[1].is_boolean());
        EXPECT_TRUE(array2[2].is_string());
        EXPECT_TRUE(array2[3].is_integral_number());
        EXPECT_TRUE(array2[4].is_float_number());
        
        Value v;
        array1.push_back(v);
        EXPECT_TRUE(array1.back().is_null());
        
        array1.push_back(array2);
        EXPECT_TRUE(array1.back().is_array());
        
        String s1("string 1");
        Value v1(s1);
        EXPECT_TRUE(v1.is_string());
        
        array1.push_back(v1);
        EXPECT_TRUE(array1.back().is_string());
        
        String s2("string 2");
        Value v2(s2);
        EXPECT_TRUE(v2.is_string());
        array1.push_back(std::move(v2));
        EXPECT_TRUE(array1.back().is_string());
        
        String s3("string 3");
        array1.push_back(s3);
        EXPECT_TRUE(array1.back().is_string());
        
        array1.emplace_back("string 4");
        EXPECT_TRUE(array1.back().is_string());
        
        array1.push_back(1);
        EXPECT_TRUE(array1.back().is_integral_number());
        
        array1.emplace_back(1);
        EXPECT_TRUE(array1.back().is_integral_number());
        
        array1.push_back(1.3);
        EXPECT_TRUE(array1.back().is_float_number());
        
        array1.emplace_back(1.3);
        EXPECT_TRUE(array1.back().is_float_number());
    }
    
    
    TEST_F(JsonValueTest, ObjectConstruction)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        // If we require a JSON Value whose type is a JSON Object, we usually
        // create a default constructed object type first. Type Object is the
        // underlaying implementation type for objects - which is a std::map
        // per default whose embedded type key_type is a std::string and its
        /// embedded type mapped_type equals Value.
        
        // Create an example object:
        Object o;
        o.insert({"key1",1});
        EXPECT_TRUE(o["key1"].is_number());
        EXPECT_TRUE(o["key1"] == Value(1));
        
        o.emplace("key2", 2);
        o.insert({"key3", 3});
        
        Value v0(o);
        EXPECT_TRUE( v0.is_type<Object>() );
        EXPECT_TRUE( v0.as<Object>()["key1"].as<long>() == 1 );
        EXPECT_TRUE( v0.as<Object>()["key2"].as<long>() == 2 );
        EXPECT_TRUE( v0.as<Object>()["key3"].as<long>() == 3 );
        
        Value v1 = o;
        EXPECT_TRUE( v1.is_type<Object>() );
        EXPECT_TRUE( v1.as<Object>()["key1"].as<long>() == 1 );
        EXPECT_TRUE( v1.as<Object>()["key2"].as<long>() == 2 );
        EXPECT_TRUE( v1.as<Object>()["key3"].as<long>() == 3 );
    }
    
    
    TEST_F(JsonValueTest, GetAccessors)
    {
        namespace mpl = json::utility::mpl;
        
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        Value v1 = null;
        
        auto x = v1.as<Null>();
        EXPECT_TRUE( (std::is_same<Null, decltype(x)>::value ) );
                
        auto ap = v1.as<Array*>();
        EXPECT_FALSE( (std::is_same<decltype(nullptr), decltype(ap)>::value ) );
        
        
        Value v2 = 1;
        auto i = v2.as<IntNumber>();
        EXPECT_TRUE( (std::is_same<IntNumber, decltype(i)>::value ) );

        auto ip = v2.as<IntNumber*>();
        EXPECT_TRUE( (std::is_same<IntNumber*, decltype(ip)>::value ) );
        
        IntNumber& ri = v2.as<IntNumber>();
        EXPECT_TRUE( (std::is_same<IntNumber&, decltype(ri)>::value ) );
        
        int ii = static_cast<int>(v2.as<int>());
        EXPECT_TRUE( ii == 1 );
        
        long ll = static_cast<long>(v2.as<long>());
        EXPECT_TRUE( ll == 1 );
    }
    
    
    TEST_F(JsonValueTest, InterpretAs)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        Value v1 = 1;
        
        IntNumber& in = v1.interpret_as<IntNumber>();
        EXPECT_TRUE( v1.is_type<IntNumber>() );
        int i = static_cast<int>(in);
        EXPECT_EQ(1, i);
    }


    
    
    TEST_F(JsonValueTest, ComparisonOperators1)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
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
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        // Comapre a Value v with a value of Null
        
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
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
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
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        Value v1 = 1;
        Value v2 = 2;
        Value v3 = 1;
        
        EXPECT_FALSE(  v1  ==  v2  );
        EXPECT_TRUE (  v1  !=  v2  );
        EXPECT_TRUE (  v1  ==  v3  );
        EXPECT_FALSE ( v1  !=  v3  );
        
        IntNumber n1 = 1;
        IntNumber n2 = 2;
        
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
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
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
        a.reserve(10000);
        for (int i = 0; i < 10000; ++i) {
            //a.push_back(Value("To check whether swap is implemented properly, we compare runtime characteristics"));
            a.emplace_back("To check whether swap is implemented properly, we compare runtime characteristics");
        }

        Value v2;
        
        utilities::timer t;
        bool result = true;
        t.start();
        for (int i = 0; i < 1000; ++i) {
            swap(v1, v2);   // should be acceptable fast
            
            if (i == 0) {
                if (v2.as<Array>().size() != 10000) {
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
    
    
    TEST_F(JsonValueTest, ObjectTest)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        // Note: keys will be represented as String on a stack
        std::vector<Value> stack = {
            "null_key", json::null,
            "boolean_key", true,
            "integer_number_key", 1,
            "float_number_key", 1.1,
            "string_key", "The quick brown fox jumps over the lazy dog",
            "array_key", Array(),
            "object_key", Object()
        };
        
        typedef typename std::vector<Value>::iterator stack_iter;
        stack_iter first = stack.begin();
        stack_iter last = stack.end();
        
        ASSERT_TRUE( (std::distance(first, last)&0x01) == 0 ); 
        
        Value v = Object();
        Object& o = v.interpret_as<Object>();
        bool duplicateKeyError = false;
        while (first != last and not duplicateKeyError)
        {
            typedef typename Object::iterator obj_iter;            
            // get the key from the stack:
            Value& keyVal = *first;            
            ASSERT_TRUE(keyVal.is_type<String>());
            String& keyString = keyVal.interpret_as<String>();
            //Key key(std::move(keyString));
            
            ++first;
            // emplace the key-value pair
            std::pair<obj_iter, bool> result = o.emplace(std::move(keyString), std::move(*first));
            ASSERT_TRUE(keyString.size() == 0);
            if (not result.second)
            {
                duplicateKeyError = true;
            }
            ++first;
        }
        
        EXPECT_EQ(stack.size(), o.size()*2);
        stack.clear();
        
        EXPECT_EQ( o["null_key"],           json::null );
        EXPECT_EQ( o["boolean_key"],        true );
        EXPECT_EQ( o["integer_number_key"], 1 );
        EXPECT_EQ( o["float_number_key"],   1.1 );
        EXPECT_EQ( o["string_key"],         "The quick brown fox jumps over the lazy dog" );
        EXPECT_EQ( o["array_key"],          Array() );
        EXPECT_EQ( o["object_key"],         Object() );
                
    
    }
    
}
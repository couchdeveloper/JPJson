//
//  write_value_test.cpp
//  Test
//
//  Created by Andreas Grosam on 26.03.13.
//
//

#include "json/value/value.hpp"
#include "json/generator/write_value.hpp"
#include <gtest/gtest.h>

// for testing
#include "SafeBool.hpp"
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <type_traits>


using namespace json;


namespace {
    
    
    // The fixture for testing class json::Boolean:
    
    class WriteValueTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        WriteValueTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~WriteValueTest() {
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
    
    
    

    TEST_F(WriteValueTest, Test1)
    {
        typedef json::value<>   Value;
        typedef Value::null_type Null;
        typedef Value::boolean_type Boolean;
        typedef Value::float_number_type FloatNumber;
        typedef Value::integral_number_type IntNumber;
        typedef Value::string_type String;
        typedef Value::object_type Object;
        typedef Value::array_type  Array;
        
        
        Object o;
        o.emplace("id", 0);
        o.emplace("key2", "string 2");
        o.emplace("key3", "string 3");
        o.emplace("key4", "string 4");

        Array a;
        for (int i = 0; i < 10; ++i) {
            o["id"] = i;
            a.emplace_back(o);
        }
        
        Value v(std::move(a));
        
        std::string json;
        json::write_value(v, std::back_inserter(json), json::writer_base::pretty_print);
        std::cout << json << std::endl;
    }


}
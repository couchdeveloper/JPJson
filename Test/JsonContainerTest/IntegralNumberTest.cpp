//
//  IntegralNumberTest.cpp
//  Test
//
//  Created by Andreas Grosam on 27.02.13.
//
//

#include "json/value/integral_number.hpp"
//#include "json/utility/json_number.hpp"
//#include "json/utility/string_to_number.hpp"

#include <gtest/gtest.h>
#include <type_traits>



using namespace json;


namespace {
    
    using json::integral_number;
    
    typedef json::integral_number<int> Number;
    
    // The fixture for testing class json::Number:
    
    class JsonIntegralNumberTest: public ::testing::Test
    {
    protected:
        
        JsonIntegralNumberTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonIntegralNumberTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
//    // A Boolean shall be a json type:
//    TEST_F(JsonIntegralNumberTest, IsJsonType)
//    {
//        EXPECT_TRUE( (is_json_type<Number>::value) );
//    }
    
    
    TEST_F(JsonIntegralNumberTest, TypeTraits)
    {
    }
    
    
    TEST_F(JsonIntegralNumberTest, DefaultCtor)
    {
        Number d;
    }
    
    
    TEST_F(JsonIntegralNumberTest, Basic)
    {
        //Number n3 = ULONG_MAX; Fails to compile
        
        EXPECT_TRUE(Number(0).to_string() == "0");
        EXPECT_TRUE(Number(1).to_string() == "1");
        EXPECT_TRUE(Number(2).to_string() == "2");
        EXPECT_TRUE(Number(3).to_string() == "3");
        EXPECT_TRUE(Number(4).to_string() == "4");
        EXPECT_TRUE(Number(5).to_string() == "5");
        EXPECT_TRUE(Number(6).to_string() == "6");
        EXPECT_TRUE(Number(7).to_string() == "7");
        EXPECT_TRUE(Number(8).to_string() == "8");
        EXPECT_TRUE(Number(9).to_string() == "9");
        
        EXPECT_TRUE(Number(10).to_string() == "10");
        EXPECT_TRUE(Number(11).to_string() == "11");
        EXPECT_TRUE(Number(12).to_string() == "12");
        EXPECT_TRUE(Number(13).to_string() == "13");
        EXPECT_TRUE(Number(14).to_string() == "14");
        EXPECT_TRUE(Number(15).to_string() == "15");
        EXPECT_TRUE(Number(16).to_string() == "16");
        EXPECT_TRUE(Number(17).to_string() == "17");
        EXPECT_TRUE(Number(18).to_string() == "18");
        EXPECT_TRUE(Number(19).to_string() == "19");
        
        EXPECT_TRUE(Number(100).to_string() == "100");
        EXPECT_TRUE(Number(101).to_string() == "101");
        EXPECT_TRUE(Number(102).to_string() == "102");
        EXPECT_TRUE(Number(103).to_string() == "103");
        EXPECT_TRUE(Number(104).to_string() == "104");
        EXPECT_TRUE(Number(105).to_string() == "105");
        EXPECT_TRUE(Number(106).to_string() == "106");
        EXPECT_TRUE(Number(107).to_string() == "107");
        EXPECT_TRUE(Number(108).to_string() == "108");
        EXPECT_TRUE(Number(109).to_string() == "109");
                
        EXPECT_TRUE(Number(-1).to_string() == "-1");
        EXPECT_TRUE(Number(-2).to_string() == "-2");
        EXPECT_TRUE(Number(-3).to_string() == "-3");
        EXPECT_TRUE(Number(-4).to_string() == "-4");
        EXPECT_TRUE(Number(-5).to_string() == "-5");
        EXPECT_TRUE(Number(-6).to_string() == "-6");
        EXPECT_TRUE(Number(-7).to_string() == "-7");
        EXPECT_TRUE(Number(-8).to_string() == "-8");
        EXPECT_TRUE(Number(-9).to_string() == "-9");
        
    }
    
    
    
    
    
} // namespace
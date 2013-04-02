//
//  FloatNumberTest.cpp
//  Test
//
//  Created by Andreas Grosam on 27.02.13.
//
//

#include "json/value/float_number.hpp"

#include <gtest/gtest.h>
#include <type_traits>



using namespace json;


namespace {
    
    using json::float_number;
    
    typedef json::float_number<double> Number;
    
    // The fixture for testing class json::Number:
    
    class JsonFloatNumberTest: public ::testing::Test
    {
    protected:
        
        JsonFloatNumberTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonFloatNumberTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
//    // A Boolean shall be a json type:
//    TEST_F(JsonFloatNumberTest, IsJsonType)
//    {
//        EXPECT_TRUE( (is_json_type<Number>::value) );        
//    }
//    
    
    TEST_F(JsonFloatNumberTest, TypeTraits)
    {
    }
    
    
    TEST_F(JsonFloatNumberTest, Ctors)
    {
        Number f;
        
        //Number f1 = 1;
        Number(1.1);
    }
    
    
    TEST_F(JsonFloatNumberTest, Basic)
    {
        EXPECT_DOUBLE_EQ(0.0, std::stod(Number(0.0).to_string()));
        EXPECT_DOUBLE_EQ(1.0, std::stod(Number(1.0).to_string()));
        EXPECT_DOUBLE_EQ(2.0, std::stod(Number(2.0).to_string()));
        EXPECT_DOUBLE_EQ(3.0, std::stod(Number(3.0).to_string()));
        EXPECT_DOUBLE_EQ(4.0, std::stod(Number(4.0).to_string()));
        EXPECT_DOUBLE_EQ(5.0, std::stod(Number(5.0).to_string()));
        EXPECT_DOUBLE_EQ(6.0, std::stod(Number(6.0).to_string()));
        EXPECT_DOUBLE_EQ(7.0, std::stod(Number(7.0).to_string()));
        EXPECT_DOUBLE_EQ(8.0, std::stod(Number(8.0).to_string()));
        EXPECT_DOUBLE_EQ(9.0, std::stod(Number(9.0).to_string()));
        
        EXPECT_DOUBLE_EQ(-1.0, std::stod(Number(-1.0).to_string()));
        EXPECT_DOUBLE_EQ(-2.0, std::stod(Number(-2.0).to_string()));
        EXPECT_DOUBLE_EQ(-3.0, std::stod(Number(-3.0).to_string()));
        EXPECT_DOUBLE_EQ(-4.0, std::stod(Number(-4.0).to_string()));
        EXPECT_DOUBLE_EQ(-5.0, std::stod(Number(-5.0).to_string()));
        EXPECT_DOUBLE_EQ(-6.0, std::stod(Number(-6.0).to_string()));
        EXPECT_DOUBLE_EQ(-7.0, std::stod(Number(-7.0).to_string()));
        EXPECT_DOUBLE_EQ(-8.0, std::stod(Number(-8.0).to_string()));
        EXPECT_DOUBLE_EQ(-9.0, std::stod(Number(-9.0).to_string()));

        
        EXPECT_DOUBLE_EQ(1/3.0, std::stod(Number(1/3.0).to_string()));
        
        
    }
    
    
    
    
    
} // namespace
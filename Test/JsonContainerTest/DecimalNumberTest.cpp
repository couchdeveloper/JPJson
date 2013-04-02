//
//  DecimalNumberTest.cpp
//  Test
//
//  Created by Andreas Grosam on 22.02.13.
//
//

#include "json/value/decimal_number.hpp"
//#include "json/utility/json_number.hpp"
//#include "json/utility/string_to_number.hpp"

#include <gtest/gtest.h>
#include <type_traits>



using namespace json;


namespace {
    
    using json::decimal_number;
    
    typedef json::decimal_number<> Decimal;
    
    // The fixture for testing class json::Number:
    
    class JsonDecimalNumberTest: public ::testing::Test
    {
    protected:
    
        JsonDecimalNumberTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonDecimalNumberTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
//    // A Boolean shall be a json type:
//    TEST_F(JsonDecimalNumberTest, IsJsonType)
//    {
//        EXPECT_TRUE( (is_json_type<decimal_number<>>::value) );
//    }
    
    
    TEST_F(JsonDecimalNumberTest, TypeTraits)
    {
    }
    
    
    TEST_F(JsonDecimalNumberTest, DefaultCtor)
    {
        Decimal d;
        EXPECT_TRUE(d.to_string() == "NaN" );
        Decimal d1{};
        EXPECT_TRUE(d1.to_string() == "NaN" );
    }
    

    TEST_F(JsonDecimalNumberTest, StringCtor)
    {
        EXPECT_TRUE(Decimal{"0"}.to_string() == "0");
        EXPECT_TRUE(Decimal("1").to_string() == "1");
        EXPECT_TRUE(Decimal("2").to_string() == "2");
        EXPECT_TRUE(Decimal("3").to_string() == "3");
        EXPECT_TRUE(Decimal("4").to_string() == "4");
        EXPECT_TRUE(Decimal("5").to_string() == "5");
        EXPECT_TRUE(Decimal("6").to_string() == "6");
        EXPECT_TRUE(Decimal("7").to_string() == "7");
        EXPECT_TRUE(Decimal("8").to_string() == "8");
        EXPECT_TRUE(Decimal("9").to_string() == "9");
    
        EXPECT_TRUE(Decimal("10").to_string() == "10");
        EXPECT_TRUE(Decimal("11").to_string() == "11");
        EXPECT_TRUE(Decimal("12").to_string() == "12");
        EXPECT_TRUE(Decimal("13").to_string() == "13");
        EXPECT_TRUE(Decimal("14").to_string() == "14");
        EXPECT_TRUE(Decimal("15").to_string() == "15");
        EXPECT_TRUE(Decimal("16").to_string() == "16");
        EXPECT_TRUE(Decimal("17").to_string() == "17");
        EXPECT_TRUE(Decimal("18").to_string() == "18");
        EXPECT_TRUE(Decimal("19").to_string() == "19");
        
        EXPECT_TRUE(Decimal("100").to_string() == "100");
        EXPECT_TRUE(Decimal("101").to_string() == "101");
        EXPECT_TRUE(Decimal("102").to_string() == "102");
        EXPECT_TRUE(Decimal("103").to_string() == "103");
        EXPECT_TRUE(Decimal("104").to_string() == "104");
        EXPECT_TRUE(Decimal("105").to_string() == "105");
        EXPECT_TRUE(Decimal("106").to_string() == "106");
        EXPECT_TRUE(Decimal("107").to_string() == "107");
        EXPECT_TRUE(Decimal("108").to_string() == "108");
        EXPECT_TRUE(Decimal("109").to_string() == "109");
        
        
        EXPECT_TRUE(Decimal("102030406070809.010203040506070809").to_string() == "102030406070809.010203040506070809");
        EXPECT_TRUE(Decimal("-1").to_string() == "-1");
        EXPECT_TRUE(Decimal("-1.0e10").to_string() == "-1.0E10");

        
    }
    
//TEST_F(JsonDecimalNumberTest, detailIsValidNumberString)
//    {
//        using json::utility::is_valid_json_number;
//        
//        
//        EXPECT_TRUE(is_valid_json_number("0"));
//        EXPECT_TRUE(is_valid_json_number("-0"));
//        EXPECT_TRUE(is_valid_json_number("1.0"));
//        EXPECT_TRUE(is_valid_json_number("-1.0"));
//        EXPECT_TRUE(is_valid_json_number("1e1"));
//        EXPECT_TRUE(is_valid_json_number("1E1"));
//        EXPECT_TRUE(is_valid_json_number("1e10"));
//        EXPECT_TRUE(is_valid_json_number("1.0e1"));
//        EXPECT_TRUE(is_valid_json_number("0.0e+01"));
//        EXPECT_TRUE(is_valid_json_number("0.0e-1"));
//        EXPECT_TRUE(is_valid_json_number("0.0e+1"));
//        EXPECT_TRUE(is_valid_json_number("1.123E-120"));
//        EXPECT_TRUE(is_valid_json_number("0"));
//        EXPECT_TRUE(is_valid_json_number("0"));
//        
//        
//        
//        bool result = is_valid_json_number("-1.234567890123456e-123");
//        EXPECT_TRUE(result);
//        
//    }
    
    

//    TEST_F(JsonDecimalNumberTest, stringToUnsignedLongLong)
//    {
//        using json::utility::detail::str2ull;
//        
//        EXPECT_TRUE(str2ull("0") == 0);
//        EXPECT_TRUE(str2ull("1") == 1);
//        EXPECT_TRUE(str2ull("2") == 2);
//        EXPECT_TRUE(str2ull("3") == 3);
//        EXPECT_TRUE(str2ull("4") == 4);
//        EXPECT_TRUE(str2ull("5") == 5);
//        EXPECT_TRUE(str2ull("6") == 6);
//        EXPECT_TRUE(str2ull("7") == 7);
//        EXPECT_TRUE(str2ull("8") == 8);
//        EXPECT_TRUE(str2ull("9") == 9);
//
//        EXPECT_TRUE(str2ull("10") == 10);
//        EXPECT_TRUE(str2ull("11") == 11);
//        EXPECT_TRUE(str2ull("12") == 12);
//        EXPECT_TRUE(str2ull("13") == 13);
//        EXPECT_TRUE(str2ull("14") == 14);
//        EXPECT_TRUE(str2ull("15") == 15);
//        EXPECT_TRUE(str2ull("16") == 16);
//        EXPECT_TRUE(str2ull("17") == 17);
//        EXPECT_TRUE(str2ull("18") == 18);
//        EXPECT_TRUE(str2ull("19") == 19);
//        
//        EXPECT_TRUE(str2ull("110") == 110);
//        EXPECT_TRUE(str2ull("111") == 111);
//        EXPECT_TRUE(str2ull("112") == 112);
//        EXPECT_TRUE(str2ull("113") == 113);
//        EXPECT_TRUE(str2ull("114") == 114);
//        EXPECT_TRUE(str2ull("115") == 115);
//        EXPECT_TRUE(str2ull("116") == 116);
//        EXPECT_TRUE(str2ull("117") == 117);
//        EXPECT_TRUE(str2ull("118") == 118);
//        EXPECT_TRUE(str2ull("119") == 119);
//        
//        EXPECT_TRUE(str2ull("1110") == 1110);
//        EXPECT_TRUE(str2ull("1111") == 1111);
//        EXPECT_TRUE(str2ull("1112") == 1112);
//        EXPECT_TRUE(str2ull("1113") == 1113);
//        EXPECT_TRUE(str2ull("1114") == 1114);
//        EXPECT_TRUE(str2ull("1115") == 1115);
//        EXPECT_TRUE(str2ull("1116") == 1116);
//        EXPECT_TRUE(str2ull("1117") == 1117);
//        EXPECT_TRUE(str2ull("1118") == 1118);
//        EXPECT_TRUE(str2ull("1119") == 1119);
//        
//        EXPECT_TRUE(str2ull("1000100000") == 1000100000);
//        EXPECT_TRUE(str2ull("1000200000") == 1000200000);
//        EXPECT_TRUE(str2ull("1000300000") == 1000300000);
//        EXPECT_TRUE(str2ull("1000400000") == 1000400000);
//        EXPECT_TRUE(str2ull("1000500000") == 1000500000);
//        EXPECT_TRUE(str2ull("1000600000") == 1000600000);
//        EXPECT_TRUE(str2ull("1000700000") == 1000700000);
//        EXPECT_TRUE(str2ull("1000800000") == 1000800000);
//        EXPECT_TRUE(str2ull("1000900000") == 1000900000);
//        EXPECT_TRUE(str2ull("1000000000") == 1000000000);
//        
//    }
    

} // namespace
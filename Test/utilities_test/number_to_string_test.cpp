//
//  number_to_string_test.cpp
//  Test
//
//  Created by Andreas Grosam on 26.03.13.
//
//


#if !defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)
#define JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA
#endif

#include "json/utility/number_to_string.hpp"
#include <gtest/gtest.h>

#include <iostream>


// for testing


namespace {
    
    
    class NumberToStringTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        NumberToStringTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~NumberToStringTest() {
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
    
    
    
    TEST_F(NumberToStringTest, WriteNumberInteger)
    {
//        template <typename T, typename OutputIterator>
//        OutputIterator write_number(T const& value, OutputIterator dest, typename std::enable_if<std::is_integral<T>::value>::type* = 0);
        
        using json::utility::write_number;
        
        std::string str;
        
        auto outIter = write_number(0, std::back_inserter(str));
        outIter = write_number(1, outIter);
        outIter = write_number(0, outIter);
        outIter = write_number(1, outIter);
        EXPECT_EQ(std::string("0101"), str);
        str.clear();
        
        write_number(0, std::back_inserter(str));
        EXPECT_EQ(std::string("0"), str);
        str.clear();
        
        write_number(-1, std::back_inserter(str));
        EXPECT_EQ(std::string("-1"), str);
        str.clear();
        
        write_number(100, std::back_inserter(str));
        EXPECT_EQ(std::string("100"), str);
        str.clear();
        
        write_number(-100, std::back_inserter(str));
        EXPECT_EQ(std::string("-100"), str);
        str.clear();
        
        write_number(0L, std::back_inserter(str));
        EXPECT_EQ(std::string("0"), str);
        str.clear();
        
        write_number(-1L, std::back_inserter(str));
        EXPECT_EQ(std::string("-1"), str);
        str.clear();
        
        write_number(1234567890L, std::back_inserter(str));
        EXPECT_EQ(std::string("1234567890"), str);
        str.clear();
        
        write_number(-1234567890L, std::back_inserter(str));
        EXPECT_EQ(std::string("-1234567890"), str);
        str.clear();
    }
    
    
    TEST_F(NumberToStringTest, WriteNumberFloat)
    {
    // template <typename T, typename OutputIterator, typename Formatter = boost::spirit::karma::real_policies<T>>
    // OutputIterator write_number(T const& value, OutputIterator dest, Formatter frmt = Formatter(),
    //     typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
        
        
        
        using json::utility::write_number;
        
        std::string str;
        
        write_number(0.0, std::back_inserter(str));
        EXPECT_EQ(std::string("0.0e00"), str);
        str.clear();
        
        
        write_number(-0.0, std::back_inserter(str));
        EXPECT_EQ(std::string("0.0e00"), str);
        str.clear();
        
        write_number(-1.0, std::back_inserter(str));
        EXPECT_EQ(std::string("-1.0e00"), str);
        str.clear();
        
//        write_number(1.0/3.0, std::back_inserter(str));
//        EXPECT_EQ(std::string("0.3333333333"), str);
//        str.clear();
        
    }
    
    
//    TYPED_TEST_CASE_P(StringToNumberTest);
//    
//    
//    TYPED_TEST_P(StringToNumberTest, BasicTest)
//    {
//        typedef TypeParam  number_type;
//        
//        number_type i = string_to_number<number_type>("0", 1);
//        EXPECT_EQ(0, i);
//        i = string_to_number<number_type>("1", 1);
//        EXPECT_EQ(1, i);
//        if (std::numeric_limits<number_type>::is_signed) {
//            i = string_to_number<number_type>("-1", 2);
//            EXPECT_EQ(-1, i);
//        }
//        if (std::numeric_limits<number_type>::is_exact) {
//            std::string str = boost::lexical_cast<std::string>(std::numeric_limits<number_type>::min());
//            i = string_to_number<number_type>(str.c_str(), str.size());
//            EXPECT_EQ(std::numeric_limits<number_type>::min(), i);
//            str = boost::lexical_cast<std::string>(std::numeric_limits<number_type>::max());
//            i = string_to_number<number_type>(str.c_str(), str.size());
//            EXPECT_EQ(std::numeric_limits<number_type>::max(), i);
//        }
//        else {
//            number_type value = std::numeric_limits<number_type>::max()*0.1;
//            std::string str = boost::lexical_cast<std::string>(value);
//            i = string_to_number<number_type>(str.c_str(), str.size());
//            EXPECT_NEAR(1.0, value/i, 1e-6);
//            value = std::numeric_limits<number_type>::min()*10.0;
//            str = boost::lexical_cast<std::string>(value);
//            i = string_to_number<number_type>(str.c_str(), str.size());
//            EXPECT_NEAR(1.0, value/i, 1e-6);
//        }
//    }
//    
//    
//    // Register
//    REGISTER_TYPED_TEST_CASE_P(StringToNumberTest,
//                               BasicTest);
//    // Instantiate test cases:
//    typedef ::testing::Types<
//    int,
//    unsigned int,
//    long,
//    unsigned long,
//    long long,
//    unsigned long long,
//    float,
//    double,
//    long double
//    >  number_types;
//    
//    INSTANTIATE_TYPED_TEST_CASE_P(BasicTests, StringToNumberTest, number_types);
    
    
    
}
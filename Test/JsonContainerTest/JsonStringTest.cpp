//
//  JsonStringTest.cpp
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

#include "json/value/string.hpp"
#include "gtest/gtest.h"

// for testing
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <string.h>


using namespace json;


namespace {
    
    typedef json::string<unicode::UTF_8_encoding_tag> utf8_string;
    typedef json::string<unicode::UTF_16_encoding_tag> utf16_string;
    typedef json::string<unicode::UTF_32_encoding_tag> utf32_string;
        
    class JsonStringTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonStringTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonStringTest() {
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
    
    
    // A utf8_string shall be a json type:
    TEST_F(JsonStringTest, IsJsonType) 
    {
        EXPECT_TRUE( (is_json_type<utf8_string>::value) );
        EXPECT_TRUE( (boost::is_base_of< boost::mpl::true_, is_json_type<utf8_string> >::value) );
    }
    
    TEST_F(JsonStringTest, DefaultConstructor) 
    {
        utf8_string s;
        EXPECT_TRUE( s.c_str() != 0 );
        EXPECT_TRUE( std::strcmp("", s.c_str()) == 0 );
        EXPECT_EQ( 0, s.size() );
        
        // implementation specific:
        EXPECT_EQ( 0, s.ref_count() );  // zero string is not counted
    }
    
    TEST_F(JsonStringTest, CharPointerConstructor1) 
    {
        const char* cs0 = "abcdefg";
        utf8_string s1 = cs0;
        //utf16_string utf16_s = "abcdefg";
        EXPECT_TRUE( s1.c_str() != 0 );
        EXPECT_EQ( 1, s1.ref_count() );
        EXPECT_EQ( std::strlen(cs0), s1.size() );
        EXPECT_TRUE( std::strcmp(cs0, s1.c_str()) == 0  );
    }
    
#if 0
    TEST_F(JsonStringTest, CharPointerConstructor2) 
    {
        const char* cs0 = "abcdefg";
        const size_t len = std::strlen(cs0);
        utf8_string s1(cs0, len);
        EXPECT_TRUE( s1.c_str() != 0 );
        EXPECT_EQ( 1, s1.ref_count() );
        EXPECT_EQ( len, s1.size() );
        EXPECT_TRUE( std::strcmp(cs0, s1.c_str()) == 0  );
    }
    
    TEST_F(JsonStringTest, CopyConstructor) 
    {
        utf8_string s1;
        utf8_string s2(s1);
        EXPECT_TRUE( std::strcmp(s1.c_str(), s2.c_str()) == 0  );
        EXPECT_TRUE( s1.size() == s2.size() );
                
        // implementation specific:
        EXPECT_EQ( 0, s1.ref_count() );
        EXPECT_EQ( 0, s2.ref_count() );
        EXPECT_TRUE( s1.c_str() == s2.c_str() ); 
        
        
        
        utf8_string s3 = "abcdefg";
        utf8_string s4(s3);
        EXPECT_TRUE( std::strcmp(s3.c_str(), "abcdefg") == 0  );
        EXPECT_TRUE( std::strcmp(s3.c_str(), s4.c_str()) == 0  );
        EXPECT_TRUE( s3.size() == s4.size() );
                
        // implementation specific:
        EXPECT_EQ( 2, s3.ref_count() );
        EXPECT_EQ( 2, s4.ref_count() );
        EXPECT_TRUE( s3.c_str() == s4.c_str() );
        {
            utf8_string s5(s3);
            EXPECT_EQ( 3, s3.ref_count() );
            EXPECT_EQ( 3, s4.ref_count() );
            EXPECT_EQ( 3, s5.ref_count() );
            EXPECT_TRUE( s3.c_str() == s5.c_str() );
        }
        EXPECT_EQ( 2, s3.ref_count() );
        EXPECT_EQ( 2, s4.ref_count() );
        EXPECT_TRUE( s3.c_str() == s4.c_str() );
    }
    
    TEST_F(JsonStringTest, ConversionConstructor_std_string) 
    {
        std::string s1;
        utf8_string s2(s1);
        EXPECT_TRUE( std::strcmp(s1.c_str(), s2.c_str()) == 0  );
        EXPECT_TRUE( s1.size() == s2.size() );
        
        // implementation specific:
        EXPECT_EQ( 0, s2.ref_count() );   // refcount equals zero for empty strings
        EXPECT_TRUE( s1.c_str() != s2.c_str() ); 
        
        
        std::string s3 = "abcdefg";
        utf8_string s4(s3);
        EXPECT_TRUE( std::strcmp(s3.c_str(), "abcdefg") == 0  );
        EXPECT_TRUE( std::strcmp(s3.c_str(), s4.c_str()) == 0  );
        EXPECT_TRUE( s3.size() == s4.size() );
        
        // implementation specific:
        EXPECT_EQ( 1, s4.ref_count() );
        EXPECT_TRUE( s3.c_str() != s4.c_str() );
        {
            utf8_string s5(s4);
            EXPECT_EQ( 2, s4.ref_count() );
            EXPECT_EQ( 2, s5.ref_count() );
            EXPECT_TRUE( s4.c_str() == s5.c_str() );
        }
        EXPECT_EQ( 1, s4.ref_count() );
        EXPECT_TRUE( s3.c_str() != s4.c_str() );
        
    }
    
    TEST_F(JsonStringTest, Assignment) 
    {
        const char* p = "abcde";
        utf8_string s1 = p;
        utf8_string s2;
        s2 = s1;
        EXPECT_TRUE( std::strcmp(s1.c_str(), p) == 0  );
        EXPECT_TRUE( std::strcmp(s2.c_str(), p) == 0  );
        EXPECT_EQ( strlen(p), s1.size() );
        EXPECT_EQ( strlen(p), s2.size() );

        // implementation specific:
        EXPECT_EQ( 2, s1.ref_count() );
        EXPECT_EQ( 2, s2.ref_count() );
        EXPECT_TRUE( s1.c_str() == s2.c_str() );         
    }
    
    TEST_F(JsonStringTest, Swap) 
    {
        const char* a = "a";
        const char* b = "bb";
        
        utf8_string s1 = a;
        utf8_string s2 = b;
        utf8_string s3 = s1;
        
        swap(s1, s2);
        EXPECT_TRUE( std::strcmp(s1.c_str(), b) == 0  );
        EXPECT_TRUE( std::strcmp(s2.c_str(), a) == 0  );
        EXPECT_TRUE( std::strcmp(s3.c_str(), a) == 0  );
        EXPECT_EQ( strlen(b), s1.size() );
        EXPECT_EQ( strlen(a), s2.size() );
        EXPECT_EQ( strlen(a), s3.size() );
        
        // implementation specific:
        EXPECT_EQ( 1, s1.ref_count() );
        EXPECT_EQ( 2, s2.ref_count() );
        EXPECT_EQ( 2, s3.ref_count() );
    }


    TEST_F(JsonStringTest, RelationalOperators)
    {
        utf8_string s1 = "Abcdefg12345";
        utf8_string s2 = "abcdefg12345";
        utf8_string s3(s1);
        
        EXPECT_TRUE( s1 == s3 );
        EXPECT_FALSE( s1 != s3 );

        EXPECT_TRUE( s1 != s2 );
        EXPECT_FALSE( s1 == s2 );
        
        EXPECT_TRUE( utf8_string("A") < utf8_string("B") );
        EXPECT_TRUE( utf8_string("B") > utf8_string("A") );

        EXPECT_TRUE( utf8_string("Abc1") != utf8_string("Abc10") );
        EXPECT_FALSE( utf8_string("Abc1") == utf8_string("Abc10") );
        
        EXPECT_TRUE( utf8_string() == utf8_string("") );
        EXPECT_TRUE( utf8_string() == utf8_string("a", 0) );
        EXPECT_TRUE( utf8_string() == utf8_string(0, 1) );
        
        EXPECT_TRUE( utf8_string("") != utf8_string("abc") );
        EXPECT_FALSE( utf8_string("") == utf8_string("abc") );

        EXPECT_TRUE( utf8_string("Abc") < utf8_string("Bcd") );
        EXPECT_TRUE( utf8_string("Abc") < utf8_string("Abc1") );
        EXPECT_TRUE( utf8_string("Abc") < utf8_string("B") );
    
    }
    
    TEST_F(JsonStringTest, StreamOperators)
    {
        utf8_string s1 = "Abcdefg12345";
        std::cout << "xxx" << std::endl;
        std::cout << s1 << std::endl;
        std::cout << "xxx" << std::endl;        
    }
#endif    
    
}    

//
//  JsonTextTest.cpp
//
//  Created by Andreas Grosam on 5/17/11.
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

#error  This file is obsolete.


#include <boost/detail/endian.hpp>
#include "json/text.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <fstream>
#include <iomanip>

#include <stdlib.h>
#include <string.h>
#include <iterator>
#include <locale>



namespace {
    
    
    // The fixture for testing class json::Value:
    
    
    using namespace json;
    
    using json::byte_swap;
    using json::internal::run_time_host_endianness;
    using json::internal::host_endianness;
    using json::internal::little_endian_tag;
    using json::internal::big_endian_tag;
    
    
    
    class JsonTextTest : public ::testing::Test {
    protected:
        
        JsonTextTest() {
        }
        
        virtual ~JsonTextTest() {
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
    
    
    TEST_F(JsonTextTest, CppLocale) 
    {
        //std::codecvt<wchar_t, char, std::mbstate_t> utf8_codecvt_facet;
        //std::locale old_locale;
        //std::locale utf8_locale(old_locale, new std::codecvt<wchar_t, char, std::mbstate_t>);
        //std::locale::global(utf8_locale);        // set a new global locale
        std::wofstream ofs("test_utf8.txt");      // Create a file stream. Default external encoding is UTF-8
        ASSERT_TRUE( ofs.good() );
        //ofs.imbue(utf8_locale);                  // define the locale (namele, the codecvt facet) for the stream
        const wchar_t ws[] = L"abcd";//L"1234\u00FC\u00F6\u20AC\u00B5";  // create a wstring with content "1234üö€µ"        
        const size_t ws_len = sizeof(ws)/sizeof(wchar_t)  - 1; // length not counting the zero termination
        //const wchar_t* first = ws;
        //const wchar_t* last = ws + ws_len;
        //const char s[] = "1234\u00FC\u00F6\u20AC\u00B5";         // create a wstring with content "1234üö€µ" 
        //const char* first = s;
        //const char* last = s + sizeof(s);

        // write the string out to the file stream.
        //std::copy(first, last, std::ostream_iterator<wchar_t,wchar_t>(ofs));
        //std::copy(first, last, std::ostream_iterator<char,char>(ofs));
        ofs << ws;
        ASSERT_FALSE( ofs.fail() );
        ofs.close();
        
        // read the string back into a wchar_t buffer:
        wchar_t buffer[ws_len] = {0};
        std::wifstream ifs("test_utf8.txt");
        ASSERT_TRUE( ifs.good() );
        //ifs.imbue(utf8_locale);
        ifs.read(buffer, ws_len);
        EXPECT_FALSE( ifs.fail() );
        ifs.close();
        ASSERT_FALSE( ifs.fail() );
        
        // compare result:
        bool isEqual = memcmp(buffer, ws, sizeof(buffer)) == 0;
        EXPECT_TRUE(isEqual);
    } 
    
    
    TEST_F(JsonTextTest, ConstructorUTF_8) 
    {
        char buffer[256] = {'1', '2', '3', '4', '\n', 0x80, 0x81, 0xEF, 0xFF, 0 } ;        
        char* first = buffer;
        char* last = first + 256;
        
        typedef text<char*, UTF_8_encoding_tag> text_t;
        typedef text_t::char_type char_t;
        
        text_t txt(first, last);        
        
        EXPECT_EQ( '1', txt.ch_inc() );
        EXPECT_EQ( '2', txt.ch_inc() );
        EXPECT_EQ( '3', txt.ch_inc() );
        EXPECT_EQ( '4', txt.ch_inc() );        
        EXPECT_EQ( '\n', txt.ch_inc() );        
        EXPECT_EQ( 0x80, txt.ch_inc() );        
        EXPECT_EQ( 0x81, txt.ch_inc() );        
        EXPECT_EQ( 0xEF, txt.ch_inc() );        
        EXPECT_EQ( 0xFF, txt.ch_inc() );        
    }
    
    TEST_F(JsonTextTest, ConstructorUTF_16BE) 
    {
        char buffer[256*2] = {0x00,'1', 0x00,'2', 0x00,'3', 0x00,'4', 0x00,'\n', 0x00,0x80, 0x00,0x81, 0x00,0xEF, 0x00,0xFF, 0x00,0x00}; 
        uint16_t* first = (uint16_t*)buffer;
        uint16_t* last = first + 256;
        
        typedef text<uint16_t*, UTF_16BE_encoding_tag> text_t;
        typedef text_t::char_type char_t;
        
        text_t txt(first, last);        
        
        EXPECT_EQ( '1', txt.ch_inc() );
        EXPECT_EQ( '2', txt.ch_inc() );
        EXPECT_EQ( '3', txt.ch_inc() );
        EXPECT_EQ( '4', txt.ch_inc() );        
        EXPECT_EQ( '\n', txt.ch_inc() );        
        EXPECT_EQ( 0x80, txt.ch_inc() );        
        EXPECT_EQ( 0x81, txt.ch_inc() );        
        EXPECT_EQ( 0xEF, txt.ch_inc() );        
        EXPECT_EQ( 0xFF, txt.ch_inc() );        
    }
    
    TEST_F(JsonTextTest, ConstructorUTF_16LE) 
    {
        char buffer[256*2] = {'1',0x00, '2',0x00, '3',0x00, '4',0x00, '\n',0x00, 0x80,0x00, 0x81,0x00 ,0xEF,0x00, 0xFF,0x00, 0x00,0x00}; 
        uint16_t* first = (uint16_t*)buffer;
        uint16_t* last = first + 256;
        
        
        typedef text<uint16_t*, UTF_16LE_encoding_tag> text_t;
        typedef text_t::char_type char_t;
        
        text_t txt(first, last);        
        
        EXPECT_EQ( '1', txt.ch_inc() );
        EXPECT_EQ( '2', txt.ch_inc() );
        EXPECT_EQ( '3', txt.ch_inc() );
        EXPECT_EQ( '4', txt.ch_inc() );        
        EXPECT_EQ( '\n', txt.ch_inc() );        
        EXPECT_EQ( 0x80, txt.ch_inc() );        
        EXPECT_EQ( 0x81, txt.ch_inc() );        
        EXPECT_EQ( 0xEF, txt.ch_inc() );        
        EXPECT_EQ( 0xFF, txt.ch_inc() );        
    }
    
    
} //namespace 

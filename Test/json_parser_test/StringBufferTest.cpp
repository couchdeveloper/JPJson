//
//  StringBufferTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 4/8/11.
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

#include <boost/detail/endian.hpp>
#include "json/utility/string_buffer.hpp"

#include "gtest/gtest.h"


namespace {
    
    using namespace json;
    using json::internal::string_buffer_base;
    using json::internal::string_buffer;
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    using json::unicode::platform_encoding_tag;
    
    
    
    // The fixture for testing class JsonParser.
    class StringBufferTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        StringBufferTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~StringBufferTest() {
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
    
    TEST_F(StringBufferTest, TestCapazity0) {
        string_buffer<UTF_8_encoding_tag> buffer;
        //EXPECT_EQ(0, buffer.capazity());
        EXPECT_EQ(0, buffer.size());
        EXPECT_EQ(buffer.capazity(), buffer.left());
    }
        
    TEST_F(StringBufferTest, TestCapazity1) {
        string_buffer<UTF_8_encoding_tag> buffer;
        //EXPECT_EQ(1, buffer.capazity());
        EXPECT_EQ(0, buffer.size());
        EXPECT_EQ(buffer.capazity(), buffer.left());
    }
    TEST_F(StringBufferTest, TestCapazity2) {
        string_buffer<UTF_8_encoding_tag> buffer;
        //EXPECT_EQ(10, buffer.capazity());
        EXPECT_EQ(0, buffer.size());
        EXPECT_EQ(buffer.capazity(), buffer.left());
    }
    
    TEST_F(StringBufferTest, TestCapazity3) {
        string_buffer<UTF_8_encoding_tag> buffer;
        //EXPECT_EQ(2000, buffer.capazity());
        EXPECT_EQ(0, buffer.size());
        EXPECT_EQ(buffer.capazity(), buffer.left());
    }
    
    TEST_F(StringBufferTest, TestInvalidCapazity) {
        string_buffer<UTF_8_encoding_tag> buffer;
        //EXPECT_EQ(0, buffer.capazity());
        EXPECT_EQ(0, buffer.size());
        EXPECT_EQ(buffer.capazity(), buffer.left());        
    }

    TEST_F(StringBufferTest, TestAppendChar) {
        string_buffer<UTF_8_encoding_tag> buffer;
        buffer.append('c');
        EXPECT_EQ(1, buffer.size());
        EXPECT_EQ(true, buffer.capazity() > 0);
        EXPECT_EQ(buffer.capazity(), buffer.left() + 1);
    }

    TEST_F(StringBufferTest, TestAppendChars) {
        string_buffer<UTF_8_encoding_tag> buffer;
        buffer.append_cstr("123456");
        EXPECT_EQ(6, buffer.size());
        EXPECT_EQ(true, buffer.capazity() > 0);
        EXPECT_EQ(buffer.capazity(), buffer.left() + 6);
    }
    
    TEST_F(StringBufferTest, TestAppendMultible) {
        string_buffer<UTF_8_encoding_tag> buffer;
        buffer.append_cstr("123456");
        buffer.append('a');
        buffer.append('b');
        buffer.append('c');
        EXPECT_EQ(9, buffer.size());
        EXPECT_EQ(true, buffer.capazity() > 0);
        EXPECT_EQ(buffer.capazity(), buffer.left() + 9);
    }
    
    TEST_F(StringBufferTest, TestGrow1) {
        string_buffer<UTF_8_encoding_tag> buffer;
        for (int i = 0; i < 32; ++i)
        {
            buffer.append('a');            
            EXPECT_EQ(i + 1, buffer.size());
            EXPECT_EQ(buffer.capazity() - 1 - i, buffer.left());
        }
        
        buffer.append('x');            
        EXPECT_EQ(true, buffer.capazity() > 32);
        EXPECT_EQ(33, buffer.size());
        EXPECT_EQ(buffer.capazity(), buffer.left() + buffer.size());
        
        size_t capazity = buffer.capazity();
        size_t left = buffer.left();
        while (left-- > 0) {
            buffer.append('a');                        
        }
        EXPECT_EQ(capazity, buffer.capazity());
        EXPECT_EQ(capazity, buffer.size());
        EXPECT_EQ(buffer.capazity(), buffer.left() + buffer.size());
    }
    
    TEST_F(StringBufferTest, TestGrow2) {
        string_buffer<UTF_8_encoding_tag> buffer;
        for (int i = 0; i < 50000; ++i)
        {
            buffer.append('a');            
            EXPECT_EQ(i + 1, buffer.size());
            EXPECT_EQ(buffer.capazity(), buffer.left() + buffer.size());
        }
        
        typedef string_buffer<UTF_8_encoding_tag>::code_unit_t char_t;
        const char_t* b = buffer.buffer();
        for (int i = 0; i < 50000; ++i)
        {
            EXPECT_EQ(true, b[i] == 'a');      
        }
    }
    
    TEST_F(StringBufferTest, TestGrow3) {
        string_buffer<UTF_8_encoding_tag> buffer;
        const char* s = "0123456789abc";  // len = 13
        const size_t len = strlen(s);
        for (int i = 0; i < 5000; ++i)
        {
            buffer.append_cstr(s);            
            EXPECT_EQ( (i + 1)*len, buffer.size());
            EXPECT_EQ(buffer.capazity(), buffer.left() + buffer.size());
        }
        
        typedef string_buffer<UTF_8_encoding_tag>::code_unit_t char_t;
        const char_t* b = buffer.buffer();
        for (int i = 0; i < 5000; ++i)
        {
            for (int j = 0; j < len; ++j, b++) {
                EXPECT_EQ(true, *b == s[j]);
            }
        }
    }
    
    TEST_F(StringBufferTest, UTF_Conversion) 
    {
        typedef unicode::to_host_endianness<UTF_16_encoding_tag>::type encoding; 
        typedef string_buffer<encoding> buffer_t;
        typedef buffer_t::code_unit_t char_t;
        
        buffer_t buffer;
        const char* s = "0123456789abc";  // len = 13
        buffer.append_cstr(s);
        
        const char_t* p = buffer.buffer();
        
    }

    
}  // namespace

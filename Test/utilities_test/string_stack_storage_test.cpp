//
//  string_stack_storage_test.cpp
//  Test
//
//  Created by Andreas Grosam on 3/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "gtest/gtest.h"
#include "json/parser/string_stack_storage.hpp"
#include "json/unicode/unicode_traits.hpp"



#include <iostream>
#include <iomanip>


#include <fstream>

// for testing


namespace {
    
    using namespace json;
    using parser_internal::string_stack_storage;
    
    
    
    class StringStackStorageTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        StringStackStorageTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~StringStackStorageTest() {
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
    
    
#pragma mark -
    
    TEST_F(StringStackStorageTest, Constructor) 
    {
        typedef string_stack_storage<json::unicode::UTF_8_encoding_tag> keys_type;    

        keys_type keys0;        
        EXPECT_EQ(0, keys0.stack_size());
        EXPECT_EQ(0, keys0.storage_capacity());
        EXPECT_EQ(0, keys0.storage_size());
        EXPECT_EQ(0, keys0.storage_avail());        
        
        keys_type keys1(1024);        
        EXPECT_EQ(0, keys1.stack_size());
        EXPECT_EQ(1024, keys1.storage_capacity());
        EXPECT_EQ(0, keys1.storage_size());
        EXPECT_EQ(1024, keys1.storage_avail());
    }    
    
    
    TEST_F(StringStackStorageTest, PushPopZero) 
    {
        
        typedef string_stack_storage<json::unicode::UTF_8_encoding_tag> keys_type;    
        
        keys_type keys(1024);
        
        EXPECT_TRUE(keys.buffer().first == 0);
        EXPECT_EQ(0, keys.buffer().second);
        
        const int N = 10;
        for (int i = 0; i < N; ++i) {
            keys.stack_push();        
            EXPECT_EQ(i+1, keys.stack_size());
            EXPECT_EQ(1024, keys.storage_capacity());
            EXPECT_EQ(0, keys.storage_size());
            EXPECT_EQ(1024, keys.storage_avail());
            
            EXPECT_TRUE(keys.buffer().first != 0);
            EXPECT_EQ(0, keys.buffer().second);
        }
        
        EXPECT_EQ(N, keys.stack_size());
        int n = N;
        
        while (keys.stack_size() > 0) {
            EXPECT_EQ(n, keys.stack_size());
            keys.stack_pop();
            --n;
            EXPECT_EQ(n, keys.stack_size());
            EXPECT_EQ(1024, keys.storage_capacity());
            EXPECT_EQ(0, keys.storage_size());
            EXPECT_EQ(1024, keys.storage_avail());
            
            if (keys.stack_size()) {
                EXPECT_TRUE(keys.buffer().first != 0);
                EXPECT_EQ(0, keys.buffer().second);
            }
            else {
                EXPECT_TRUE(keys.buffer().first == 0);
                EXPECT_EQ(0, keys.buffer().second);
            }
        }
                
    }    
    

    TEST_F(StringStackStorageTest, Append)
    {
        // void append(const buffer_type& buffer);
        // void append(code_unit_type code_unit)

        
        typedef string_stack_storage<json::unicode::UTF_8_encoding_tag> keys_type;    
        typedef keys_type::buffer_type buffer_t;

        const char* s = "0123456789ABCDEF";
        const size_t s_len = strlen(s);
        
        keys_type keys(8);
        int const N = 100;
        
        keys.stack_push();
        for (int i = 0; i < N*s_len; ++i)
        {
            keys.append(s[i%16]);
        }
        
        buffer_t buffer = keys.buffer();
        
        EXPECT_TRUE(buffer.first != 0);
        EXPECT_TRUE(buffer.second == N*s_len);
        
        const char* p = buffer.first;
        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(0, strncmp(p, s, s_len));
            p += s_len;
        }
        
        // after reset(); the previous content shall be be valid:
        keys.reset();
        buffer_t buffer2 = keys.buffer();
        EXPECT_TRUE(buffer.first == buffer2.first);
        EXPECT_TRUE(buffer2.first != 0);
        EXPECT_TRUE(buffer2.second == 0);

        p = buffer.first;
        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(0, strncmp(p, s, s_len));
            p += s_len;
        }
        
    }

    
    
    TEST_F(StringStackStorageTest, PushPopSmallString) 
    {
        
        typedef string_stack_storage<json::unicode::UTF_8_encoding_tag> keys_t;    
        typedef keys_t::buffer_type  buffer_t;
        typedef keys_t::const_buffer_type  const_buffer_t;
        
        keys_t keys(1024);
        
        EXPECT_TRUE(keys.buffer().first == 0);
        EXPECT_EQ(0, keys.buffer().second);
        
        
        char const* s = "0123456789ABCDEF";
        
        const int N = 10;
        for (int i = 0; i < N; ++i) {
            keys.stack_push(const_buffer_t(s+i, 1));        
            EXPECT_EQ(i+1, keys.stack_size());
            EXPECT_EQ(1024, keys.storage_capacity());
            EXPECT_EQ(i+1, keys.storage_size());
            EXPECT_EQ(1024-(i+1), keys.storage_avail());
            
            EXPECT_TRUE(keys.buffer().first != 0);
            EXPECT_EQ(1, keys.buffer().second);            
            EXPECT_EQ(*(s+i), *(keys.buffer().first));
        }
        
        EXPECT_EQ(N, keys.stack_size());
        int n = N;
        
        while (keys.stack_size() > 0) {
            EXPECT_EQ(n, keys.stack_size());
            keys.stack_pop();
            --n;
            EXPECT_EQ(n, keys.stack_size());
            EXPECT_EQ(1024, keys.storage_capacity());
            EXPECT_EQ(n, keys.storage_size());
            EXPECT_EQ(1024-n, keys.storage_avail());
            
            if (keys.stack_size()) {
                EXPECT_TRUE(keys.buffer().first != 0);
                EXPECT_EQ(1, keys.buffer().second);
                EXPECT_EQ(*(s+(n-1)), *(keys.buffer().first));
            }
            else {
                EXPECT_TRUE(keys.buffer().first == 0);
                EXPECT_EQ(0, keys.buffer().second);
            }
        }
        
    }    

    
    
    TEST_F(StringStackStorageTest, PushPopLargeString) 
    {
        typedef string_stack_storage<json::unicode::UTF_8_encoding_tag> keys_t;    
        typedef keys_t::buffer_type  buffer_t;
        typedef keys_t::const_buffer_type  const_buffer_t;

        keys_t keys(16);
        
        EXPECT_TRUE(keys.buffer().first == 0);
        EXPECT_EQ(0, keys.buffer().second);
        
        
        const char* s = "0123456789ABCDEF";
        
        const int N = 1000;
        const int S = 11;
        size_t storage_size = 0;
        for (int i = 0; i < N; ++i) {
            keys.stack_push(const_buffer_t(s, S));        
            storage_size += S;
            EXPECT_EQ(i+1, keys.stack_size());            
            EXPECT_TRUE(keys.storage_capacity() >= 0);
            EXPECT_EQ(storage_size, keys.storage_size());
            EXPECT_TRUE(keys.storage_avail() >= 0);
            
            EXPECT_TRUE(keys.buffer().first != 0);
            EXPECT_EQ(S, keys.buffer().second);            
            EXPECT_EQ(0, strncmp(s, keys.buffer().first, S));
            
        }
        
        EXPECT_EQ(N, keys.stack_size());
        EXPECT_EQ(N*S, keys.storage_size());
        int n = N;
        const size_t cap = keys.storage_capacity();
        
        while (keys.stack_size() > 0) {
            EXPECT_EQ(n, keys.stack_size());
            if (keys.stack_size()) {
                EXPECT_TRUE(keys.buffer().first != 0);
                EXPECT_EQ(S, keys.buffer().second);
                EXPECT_EQ(0, strncmp(s, keys.buffer().first, S));
            }
            else {
                EXPECT_TRUE(keys.buffer().first == 0);
                EXPECT_EQ(0, keys.buffer().second);
            }
            EXPECT_EQ(cap, keys.storage_capacity());
            EXPECT_EQ(n*S, keys.storage_size());
            EXPECT_EQ(cap-(n*S), keys.storage_avail());

            keys.stack_pop();
            --n;
        }
        
    }    
    
    
}
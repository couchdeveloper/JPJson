//
//  string_storage_test.cpp
//  Test
//
//  Created by Andreas Grosam on 4/13/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "gtest/gtest.h"
#include "json/parser/string_storage.hpp"
#include "json/unicode/unicode_traits.hpp"



#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <fstream>

// for testing


namespace test {
    
    using namespace json;
    
    using unicode::encoding_traits;
    
    // A mock for a Semantic Actions class
    template <typename EncodingT>
    class SemanticActions
    {
    public:
        typedef EncodingT                                               encoding_t;
        typedef typename encoding_traits<EncodingT>::code_unit_type     char_t;        
        typedef std::vector<char_t>                                     vector_t;
        typedef std::pair<char_t*, size_t>                              buffer_t;
        typedef std::pair<char_t const*, size_t>                        const_buffer_t;
        
        
        SemanticActions()
        : start_(true), count_(0)
        {}
        
        
    public:
        void value_string_write(const const_buffer_t& buffer, bool hasMore) { 
            if (start_) {
                count_ = 0;
                str_.clear();
            }
            str_.insert(str_.end(), buffer.first, buffer.first + buffer.second);
            count_ += buffer.second;
            start_ = not hasMore;
        }      
        
        
        vector_t str() const { return str_; }
        
    private:
        bool start_;
        vector_t str_;
        size_t count_;
    };
    
}





namespace {
    
    using namespace json;
    
    using parser_internal::string_storage;
    
    
    class StringStorageTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        StringStorageTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~StringStorageTest() {
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
    
    TEST_F(StringStorageTest, Constructor1) 
    {
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        
        sa_t sa;
        storage_t storage(sa);
        
        EXPECT_FALSE(storage.partial_strings_enabled());
        
        buffer_t buffer = storage.buffer();
        EXPECT_EQ(NULL, buffer.first);
        EXPECT_EQ(0, buffer.second);
                
        EXPECT_EQ(0, storage.stack_size());
        EXPECT_EQ(0, storage.storage_capacity());
        EXPECT_EQ(0, storage.storage_size());
        EXPECT_EQ(0, storage.storage_avail());        
        
    }    

    TEST_F(StringStorageTest, Constructor2) 
    {
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        
        sa_t sa;
        storage_t storage(sa, 1024);
        
        EXPECT_FALSE(storage.partial_strings_enabled());
        
        buffer_t buffer = storage.buffer();
        EXPECT_EQ(NULL, buffer.first);
        EXPECT_EQ(0, buffer.second);
        
        EXPECT_EQ(0, storage.stack_size());
        EXPECT_EQ(1024, storage.storage_capacity());
        EXPECT_EQ(0, storage.storage_size());
        EXPECT_EQ(1024, storage.storage_avail());        
    }    
    
    
    TEST_F(StringStorageTest, PushPopZero) 
    {        
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        
        sa_t sa;
        storage_t storage(sa, 1024);
        
        EXPECT_TRUE(storage.buffer().first == 0);
        EXPECT_EQ(0, storage.buffer().second);
        
        const int N = 10;
        for (int i = 0; i < N; ++i) {
            storage.stack_push();        
            EXPECT_EQ(i+1, storage.stack_size());
            EXPECT_EQ(1024, storage.storage_capacity());
            EXPECT_EQ(0, storage.storage_size());
            EXPECT_EQ(1024, storage.storage_avail());
            
            EXPECT_TRUE(storage.buffer().first != 0);
            EXPECT_EQ(0, storage.buffer().second);
        }
        
        EXPECT_EQ(N, storage.stack_size());
        int n = N;
        
        while (storage.stack_size() > 0) {
            EXPECT_EQ(n, storage.stack_size());
            storage.stack_pop();
            --n;
            EXPECT_EQ(n, storage.stack_size());
            EXPECT_EQ(1024, storage.storage_capacity());
            EXPECT_EQ(0, storage.storage_size());
            EXPECT_EQ(1024, storage.storage_avail());
            
            if (storage.stack_size()) {
                EXPECT_TRUE(storage.buffer().first != 0);
                EXPECT_EQ(0, storage.buffer().second);
            }
            else {
                EXPECT_TRUE(storage.buffer().first == 0);
                EXPECT_EQ(0, storage.buffer().second);
            }
        }
        
    }    
    
    
    
    TEST_F(StringStorageTest, Append)
    {
        // void append(const buffer_type& buffer);
        // void append(code_unit_type code_unit)
        
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        
        sa_t sa;
        storage_t storage(sa, 8);
        
        const char* s = "0123456789ABCDEF";
        const size_t s_len = strlen(s);
        
        int const N = 1000;
        
        storage.stack_push();
        for (int i = 0; i < N*s_len; ++i)
        {
            storage.append(s[i%16]);
        }
        
        buffer_t buffer = storage.buffer();
        
        EXPECT_TRUE(buffer.first != 0);
        EXPECT_TRUE(buffer.second == N*s_len);
        
        const char* p = buffer.first;
        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(0, strncmp(p, s, s_len));
            p += s_len;
        }
        
        // after reset(); the previous content shall be be valid:
        storage.reset();
        buffer_t buffer2 = storage.buffer();
        EXPECT_TRUE(buffer.first == buffer2.first);
        EXPECT_TRUE(buffer2.first != 0);
        EXPECT_TRUE(buffer2.second == 0);
        
        p = buffer.first;
        for (int i = 0; i < N; ++i) {
            EXPECT_EQ(0, strncmp(p, s, s_len));
            p += s_len;
        }
        
    }
    
    
    
    TEST_F(StringStorageTest, PushPopTinyString) 
    {
        // Push N strings whose size is 1.
        // (this does not trigger a resize of the internal buffer)
        
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        typedef storage_t::const_buffer_type const_buffer_t;
        
        sa_t sa;
        storage_t storage(sa, 1024);

        
        EXPECT_TRUE(storage.buffer().first == 0);
        EXPECT_EQ(0, storage.buffer().second);
        
        
        char const* s = "0123456789ABCDEF";
        
        const int N = 10;
        for (int i = 0; i < N; ++i) {
            storage.stack_push(const_buffer_t(s+i, 1));        
            EXPECT_EQ(i+1, storage.stack_size());
            EXPECT_EQ(1024, storage.storage_capacity());
            EXPECT_EQ(i+1, storage.storage_size());
            EXPECT_EQ(1024-(i+1), storage.storage_avail());
            
            EXPECT_TRUE(storage.buffer().first != 0);
            EXPECT_EQ(1, storage.buffer().second);            
            EXPECT_EQ(*(s+i), *(storage.buffer().first));
        }
        
        EXPECT_EQ(N, storage.stack_size());
        int n = N;
        
        while (storage.stack_size() > 0) {
            EXPECT_EQ(n, storage.stack_size());
            storage.stack_pop();
            --n;
            EXPECT_EQ(n, storage.stack_size());
            EXPECT_EQ(1024, storage.storage_capacity());
            EXPECT_EQ(n, storage.storage_size());
            EXPECT_EQ(1024-n, storage.storage_avail());
            
            if (storage.stack_size()) {
                EXPECT_TRUE(storage.buffer().first != 0);
                EXPECT_EQ(1, storage.buffer().second);
                EXPECT_EQ(*(s+(n-1)), *(storage.buffer().first));
            }
            else {
                EXPECT_TRUE(storage.buffer().first == 0);
                EXPECT_EQ(0, storage.buffer().second);
            }
        }
        
    }    
    
    
    TEST_F(StringStorageTest, PushPopSmallString) 
    {
        // For N, push a string whose size equals S (11).
        // Since this will exceed the initial capacity,
        // an internal resize of the buffer will be triggered.
        
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        typedef storage_t::const_buffer_type const_buffer_t;
        
        sa_t sa;
        storage_t storage(sa, 16);
        storage.enable_partial_strings(false);
        
        
        EXPECT_TRUE(storage.buffer().first == 0);
        EXPECT_EQ(0, storage.buffer().second);
        
        
        const char* s = "0123456789ABCDEF";
        
        const int N = 1000;
        const int S = 11;
        size_t storage_size = 0;
        for (int i = 0; i < N; ++i) {
            storage.stack_push(const_buffer_t(s, S));        
            storage_size += S;
            EXPECT_EQ(i+1, storage.stack_size());            
            EXPECT_TRUE(storage.storage_capacity() >= 0);
            EXPECT_EQ(storage_size, storage.storage_size());
            EXPECT_TRUE(storage.storage_avail() >= 0);
            
            EXPECT_TRUE(storage.buffer().first != 0);
            EXPECT_EQ(S, storage.buffer().second);            
            EXPECT_EQ(0, strncmp(s, storage.buffer().first, S));
            
        }
        
        EXPECT_EQ(N, storage.stack_size());
        EXPECT_EQ(N*S, storage.storage_size());
        int n = N;
        const size_t cap = storage.storage_capacity();
        
        while (storage.stack_size() > 0) {
            EXPECT_EQ(n, storage.stack_size());
            if (storage.stack_size()) {
                EXPECT_TRUE(storage.buffer().first != 0);
                EXPECT_EQ(S, storage.buffer().second);
                EXPECT_EQ(0, strncmp(s, storage.buffer().first, S));
            }
            else {
                EXPECT_TRUE(storage.buffer().first == 0);
                EXPECT_EQ(0, storage.buffer().second);
            }
            EXPECT_EQ(cap, storage.storage_capacity());
            EXPECT_EQ(n*S, storage.storage_size());
            EXPECT_EQ(cap-(n*S), storage.storage_avail());
            
            storage.stack_pop();
            --n;
        }
        
    }    

    
    TEST_F(StringStorageTest, ExpectExceptionNoPartialKey) 
    {
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        typedef storage_t::const_buffer_type const_buffer_t;
        
        sa_t sa;

                
        storage_t storage(sa);
        storage.enable_partial_strings(true);
        storage.set_mode(storage_t::Key);
        
        
        const int N = 100000;  // large string
        size_t storage_size = 0;
        
        storage.stack_push();
        
        bool didThrow = false;
        
        // We expect that append() will fail after the internal
        // limit of the max buffer size has been exceeded
        try {
            for (int i = 0; i < N; ++i) {
                ++storage_size;
                storage.append('a');
            }
        }
        catch (json::parser_internal::no_partial_keystring_error& ex) {
            didThrow = true;            
        }

        EXPECT_TRUE(didThrow);        
    }
    
    
    TEST_F(StringStorageTest, LargeDataString) 
    {
        typedef json::unicode::UTF_8_encoding_tag encoding_t;
        typedef test::SemanticActions<encoding_t> sa_t;
        typedef string_storage<encoding_t, sa_t> storage_t;        
        typedef storage_t::buffer_type buffer_t;
        typedef storage_t::const_buffer_type const_buffer_t;
        
        sa_t sa;
        
        storage_t storage(sa);
        storage.enable_partial_strings(true);
        
        // push a key:        
        storage.stack_push();
        storage.set_mode(storage_t::Key);
        storage.append(const_buffer_t("key", 3));
        
        
        // push a large data string:  
        storage.stack_push();        
        storage.set_mode(storage_t::Data);
        const int N = 100000;  // large string
        for (int i = 0; i < N; ++i) {
            storage.append('a');
        }
        storage.flush();
        
        sa_t::vector_t str = sa.str();
        EXPECT_EQ(N, str.size());
        
        // pop the large data string:
        storage.stack_pop();
        
        const_buffer_t key = storage.buffer();
        EXPECT_EQ(std::string("key"), std::string(key.first, key.second));
    }
    

}

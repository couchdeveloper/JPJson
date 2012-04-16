//
//  string_buffer_test.cpp
//  Test
//
//  Created by Andreas Grosam on 3/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "gtest/gtest.h"
#include "json/parser/string_buffer.hpp"
#include "json/parser/string_storage.hpp"
#include "json/unicode/unicode_traits.hpp"



#include <iostream>
#include <iomanip>


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

        typedef std::pair<char_t*, size_t>                  buffer_t;
        typedef std::pair<char_t const*, size_t>            const_buffer_t;
        
        
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
    using parser_internal::string_buffer;
    
    
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
    
    
#pragma mark -
    
    
    TEST_F(StringBufferTest, Constructor) 
    {
    } 
    
    
#if 0
    TEST_F(StringBufferTest, AppendASCIIWithStackStorage) 
    {
        typedef string_stack_storage<json::unicode::UTF_32_encoding_tag> storage_t;        
        typedef string_buffer<storage_t> string_buffer_t;
        typedef string_buffer_t::buffer_type buffer_t;
        
        
        storage_t storage;
        string_buffer_t string_buffer(storage);
        
        const char* ascii = "abcdefghijklmnopqrstuvwxyz";
        const size_t len = strlen(ascii);
        
        const int N = 100;
        const int K = 10;
        
        for (int k = 0; k < K; ++k) 
        {
            storage.stack_push();
            for (int i = 0; i < N; ++i) {
                string_buffer.append_ascii(ascii[i%len]);
            }
            buffer_t buffer = string_buffer.buffer();
            EXPECT_TRUE(buffer.first != 0);            
            EXPECT_EQ(N, buffer.second);            
        }

        for (int k = 0; k < K; ++k) 
        {
            buffer_t buffer = string_buffer.buffer();
            ASSERT_TRUE(buffer.first != 0);            
            ASSERT_EQ(N, buffer.second);            
            for (int i = 0; i < N; ++i) {
                EXPECT_TRUE(static_cast<char>(static_cast<uint8_t>(buffer.first[i])) == ascii[i%len]);
            }
            storage.stack_pop();
        }
    }    

    
    TEST_F(StringBufferTest, AppendASCIIWithChunkStorage) 
    {
        typedef test::SemanticActions<unicode::UTF_16_encoding_tag> sa_t;
        typedef string_chunk_storage<unicode::UTF_16_encoding_tag, sa_t> storage_t;    
        typedef string_buffer<storage_t> string_buffer_t;
        typedef string_buffer_t::buffer_type buffer_t;
        
        typedef sa_t::vector_t char_vector_t;
        
        
        sa_t sa;
        storage_t storage(sa, 32);
        string_buffer_t string_buffer(storage);
        
        const char* ascii = "01234567890ABCDEF";
        const size_t len = strlen(ascii);
        
        const int N = 100;
        const int K = 10;
        for (int k = 0; k < K; ++k)
        {
            for (int i = 0; i < N; ++i) {
                string_buffer.append_ascii(ascii[i%len]);
            }
            storage.flush();            
            char_vector_t char_vector = sa.str();            
            ASSERT_EQ(N, char_vector.size()) << "with k = " << k;            
            for (int i = 0; i < N; ++i)
            {
                EXPECT_TRUE(static_cast<char>(static_cast<uint8_t>(char_vector[i])) == ascii[i%len]);
            }
        }
        
    }    
    
#endif    
    
}
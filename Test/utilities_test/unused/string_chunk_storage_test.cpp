//
//  string_chunk_storage_test.cpp
//  Test
//
//  Created by Andreas Grosam on 3/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <gtest/gtest.h>
#include "json/parser/string_chunk_storage.hpp"





// for testing
#include "json/unicode/unicode_traits.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>



namespace test {

    using namespace json;
    
    using unicode::encoding_traits;
    
    // A mock for a Semantic Actions class
    template <typename EncodingT>
    class SemanticActions
    {
    public:
        typedef EncodingT encoding_t;
        typedef typename encoding_traits<EncodingT>::code_unit_type char_t;
        typedef std::pair<char_t*, size_t>                          buffer_t;
        typedef std::pair<char_t const*, size_t>                    const_buffer_t;
        
        
        SemanticActions()
        : start_(true)
        {}
        
    public:
        void value_string_write(const const_buffer_t& buffer, bool hasMore) { 
            if (start_) {
                str_.clear();
            }
            str_.insert(str_.end(), buffer.first, buffer.first + buffer.second);            
            start_ = not hasMore;
        }      
        
        
        std::vector<char_t> str() const { return str_; }
        
    private:
        bool start_;
        std::vector<char_t> str_;
    };

}


namespace {
    
    using namespace json;
    using parser_internal::string_chunk_storage;
    
    
    
    class StringChunkStorageTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        StringChunkStorageTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~StringChunkStorageTest() {
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
    
    TEST_F(StringChunkStorageTest, Constructor) 
    {
        typedef test::SemanticActions<unicode::UTF_16_encoding_tag> sa_t;
        typedef string_chunk_storage<unicode::UTF_16_encoding_tag, sa_t> storage_type;    
        
        sa_t sa;
        storage_type buffer(sa);        
    }    
    
    
    // void append(const const_buffer_type& buffer)
    // void append(code_unit_type code_unit)
    TEST_F(StringChunkStorageTest, Append) 
    {
        typedef test::SemanticActions<unicode::UTF_16_encoding_tag> sa_t;
        typedef string_chunk_storage<unicode::UTF_16_encoding_tag, sa_t> storage_type;
        typedef storage_type::code_unit_type char_t;
        
        sa_t sa;
        storage_type buffer(sa, 8);        
        
        
        const int N = 123;
        
        for (int i = 0; i < N; ++i) {
            buffer.append (static_cast<char_t>('A'));
        }
        buffer.flush();
        
        std::vector<char_t> s = sa.str();
        EXPECT_EQ(N, s.size());
        
        
        buffer.reset();
        EXPECT_EQ(0, buffer.size());
        
        buffer.flush();
        s = sa.str();
        EXPECT_EQ(0, s.size());
    }    
    
    
}
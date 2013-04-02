//
//  string_buffer_test.cpp
//  Test
//
//  Created by Andreas Grosam on 11/19/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//




#include "json/parser/string_buffer.hpp"
#include <gtest/gtest.h>


#include <cstdlib>
#include <vector>

#include "json/unicode/unicode_traits.hpp"

//#include "json/unicode/unicode_conversions.hpp"



namespace test {
    
    using namespace json;
    
    using unicode::encoding_traits;
    
    
    // Unicode codepoint generator
    class codepoint_generator 
    {
    public:
        codepoint_generator(int seed = 0) {
            srand(seed);
        }
        
        json::unicode::code_point_t next() {
            json::unicode::code_point_t result = -1;
            while (!json::unicode::isUnicodeScalarValue(result)) {
                result = rand() % 0x10FFFFu;
            }
            return result;
        }        
    };
    
    
    
    
    
    // A mock for a Semantic Actions class
    template <typename EncodingT>
    class SemanticActions
    {
    public:
        typedef EncodingT                                           encoding_t;
        typedef typename encoding_traits<EncodingT>::code_unit_type char_t;
        typedef std::pair<char_t*, size_t>                          buffer_t;
        typedef std::pair<char_t const*, size_t>                    const_buffer_t;
        
        
        SemanticActions()
        : start_(true)
        {}
        
    public:
        void value_string(const const_buffer_t& buffer, bool hasMore) { 
            if (start_) {
                str_.clear();
            }
            str_.insert(str_.end(), buffer.first, buffer.first + buffer.second);            
            start_ = not hasMore;
        }      
        
        
        std::vector<char_t> str() const             { return str_; }
        size_t              str_size() const        { return str_.size(); }
        
    private:
        bool start_;
        std::vector<char_t> str_;
    };
    
}



namespace {
    
    using namespace json;
    using json::parser_internal::string_buffer;
        
    
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
    
    
    /**
     Synopsis 
     
     template <typename EncodingT, typename SemanticActionsT>
     class string_buffer
     {
     public:
         typedef type   code_unit_type;
         typedef type   buffer_type;
         typedef type   const_buffer_type;
         
         string_buffer(SemanticActionsT& sa);
     
         string_buffer(SemanticActionsT& sa, std::size_t initial_storage_capacity);
     
         
         // Returns the string as a buffer.
         const_buffer_type buffer() const;
         
         
         // Returns the size of the string (number of code units).
         size_t size() const;
         
         
         // Appends an Unicode code point to the string buffer. Unicode code 
         // points are always in host endianness and are assumed to be valid 
         // unicode scalar values.
         void        
         append_unicode(json::unicode::code_point_t codepoint);
         
         
         // Appends a code unit whose endianness equals the endiannes of the
         // underlaying string storage.
         // It does not check the validity of the code unit nor the validity of 
         // the code unit in the context of the string.
         void 
         append(code_unit_type cu);
         
         // Appends an ASCII character to its internal buffer. The value
         // of ch shall be in the range of valid ASCII characters, that is
         // [0 .. 0x7F]. The function does not check if the character is
         // actually valid.
         void        
         append_ascii(char ch);
         
         
         void clear();
         
         // Causes the remaining bytes from the internal string storage to be send 
         // to the semantic actions object through calling value_string(buffer, false).
         // flush() shall only be called when the parser finished parsing a **data** JSON string.        
         void flush();
    };
     */
    
    TEST_F(StringBufferTest, DefaultCtor) 
    {        
        typedef test::SemanticActions<unicode::UTF_8_encoding_tag>      sa_t;
        typedef string_buffer<unicode::UTF_8_encoding_tag, sa_t>        string_buffer_t;
        typedef string_buffer_t::const_buffer_type                      const_buffer_t;
        typedef string_buffer_t::code_unit_type                         char_t;
        
        
        sa_t sa;
        string_buffer_t stringBuffer(sa);
        EXPECT_EQ(0, stringBuffer.size());
        EXPECT_EQ(0, stringBuffer.capacity());
        EXPECT_EQ(0, stringBuffer.avail());
        
        const_buffer_t buf = stringBuffer.buffer();        
        EXPECT_EQ(0, buf.first);
        EXPECT_EQ(0, buf.second);
    }


    TEST_F(StringBufferTest, Ctor1) 
    {        
        typedef test::SemanticActions<unicode::UTF_8_encoding_tag>      sa_t;
        typedef string_buffer<unicode::UTF_8_encoding_tag, sa_t>        string_buffer_t;
        typedef string_buffer_t::const_buffer_type                      const_buffer_t;
        typedef string_buffer_t::code_unit_type                         char_t;
        
        
        sa_t sa;
        const size_t Capacity = 1024;
        string_buffer_t stringBuffer(sa, Capacity);
        EXPECT_EQ(0, stringBuffer.size());
        EXPECT_EQ(Capacity, stringBuffer.capacity());
        EXPECT_EQ(Capacity, stringBuffer.avail());
        
        const_buffer_t buf = stringBuffer.buffer();        
        EXPECT_TRUE(buf.first != NULL);
        EXPECT_EQ(0, buf.second);
    }
    
    
    TEST_F(StringBufferTest, AppendUnicode) 
    {
        typedef test::SemanticActions<unicode::UTF_8_encoding_tag>      sa_t;
        typedef string_buffer<unicode::UTF_8_encoding_tag, sa_t>        string_buffer_t;
        typedef string_buffer_t::const_buffer_type                      const_buffer_t;
        typedef string_buffer_t::code_unit_type                         char_t;
        
        
        sa_t sa;
        const size_t Capacity = 16;
        string_buffer_t stringBuffer(sa, Capacity);
        
        const size_t Count = 12345;
        for (int i = 0; i < Count; ++i) {
            stringBuffer.append_unicode(static_cast<char_t>('A'));
        }
        stringBuffer.flush();
        
        EXPECT_EQ(Count, sa.str_size());
    }
    
}
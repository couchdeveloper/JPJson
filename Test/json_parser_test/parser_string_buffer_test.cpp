//
//  parser_string_buffer_test.cpp
//  Test
//
//  Created by Andreas Grosam on 11/19/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//




#include "json/parser/string_buffer.hpp"
#include "gtest/gtest.h"


#include <cstdlib>
#include <vector>
#include "json/unicode/unicode_conversions.hpp"

namespace {
    
    using namespace json;
    using json::parser_internal::string_buffer;
    
    
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
    
    
    class parser_string_buffer_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        parser_string_buffer_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~parser_string_buffer_test() {
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
    
    
    /*
     string_buffer();
     size_type   capacity() const;
     size_type   size() const;     
     buffer_type buffer() const;   
     bool        append_unicode(json::unicode::code_point_t codepoint);
     buffer_type inplace_encode(Encoding encoding);
     
     */
    
    
    TEST_F(parser_string_buffer_test, Dtor) 
    {
        
        string_buffer<256> stringBuffer;
        
        EXPECT_EQ(0, stringBuffer.buffer().second);
        EXPECT_EQ(256, stringBuffer.capacity());
        EXPECT_EQ(0, stringBuffer.size());
    }


    TEST_F(parser_string_buffer_test, Size) 
    {
        using json::unicode::code_point_t;
        string_buffer<10> stringBuffer;
        
        for (int i = 0; i < 10; ++i) {
            bool result = stringBuffer.append_unicode(static_cast<code_point_t>('A'));
            EXPECT_EQ(i+1, stringBuffer.size());
            EXPECT_EQ(true, result);
        }
        
        for (int i = 0; i < 10; ++i) {
            bool result = stringBuffer.append_unicode(static_cast<code_point_t>('A'));
            EXPECT_EQ(10, stringBuffer.size());
            EXPECT_EQ(false, result);
        }
    }
    
    TEST_F(parser_string_buffer_test, InplaceEncode)
    {
        using json::unicode::to_host_endianness;
        using json::unicode::unicode_encoding_traits;
        using json::unicode::Encoding;
        using json::unicode::code_point_t;
        
        std::vector<code_point_t> codepoints;
        codepoint_generator cp_gen(0); 
#if defined (DEBUG)        
        const size_t K = 1000;
#else
        const size_t K = 100000;
#endif        
        const size_t N = 4*1024;
        string_buffer<N> stringBuffer;
        
        typedef std::vector<uint16_t> dest_buffer_t;
        dest_buffer_t destbuffer;
        
        typedef to_host_endianness<json::unicode::UTF_16_encoding_tag>::type dest_encoding_tag;
        typedef dest_encoding_tag::code_unit_type char_t;
        Encoding dest_encoding = unicode_encoding_traits<dest_encoding_tag>::value;

        for (int k = 0; k < K; ++k) {
            codepoints.clear();
            destbuffer.clear();
            for (int i = 0; i < N; ++i) {
                code_point_t cp = cp_gen.next();
                codepoints.push_back(cp);
                bool ok = stringBuffer.append_unicode(cp);
                ASSERT_TRUE(ok);
            }
            
            int error;
            std::back_insert_iterator<dest_buffer_t> dest(destbuffer);
            std::vector<code_point_t>::iterator first = codepoints.begin();
            // Convert sequence of codepoints into destbuffer using dest_encoding:
            size_t count = json::unicode::convert_codepoints_unsafe(first, codepoints.end(), dest, dest_encoding_tag(), error); 
            ASSERT_EQ(0, error);
            ASSERT_TRUE( count == destbuffer.size() );

            if (error == 0 and count == destbuffer.size()) {
                string_buffer<N>::buffer_type buffer = stringBuffer.inplace_encode(dest_encoding);        
                EXPECT_EQ(destbuffer.size()*sizeof(char_t), buffer.second);
                if (buffer.second*sizeof(char_t) == destbuffer.size()) {
                    EXPECT_EQ(0, std::memcmp(buffer.first, &destbuffer[0], buffer.second));
                }
            }
        }
    }
    


}
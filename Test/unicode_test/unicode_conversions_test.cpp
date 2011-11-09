//
//  unicode_conversions_test.cpp
//  Test
//
//  Created by Andreas Grosam on 8/3/11.
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

#include "json/unicode/unicode_conversions.hpp"
#include "gtest/gtest.h"

#include <iomanip>


// for testing
#include <unicode/ustring.h>
#include <unicode/utf.h>
#include <unicode/utf8.h>
#include <unicode/utf16.h>
#include <unicode/ucnv.h>
#include <limits>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>


using namespace json;



namespace json { namespace unicode { namespace test {

    

    using json::unicode::code_point_t;
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::utf_encoding_tag;
    using json::unicode::to_host_endianness;
    
    template <typename EncodingT, class Enable = void>
    class wellformed_generator 
    {
    public:
        typedef typename EncodingT::code_unit_type      code_unit_type;
        typedef boost::array<code_unit_type, 4>         buffer_type;
        typedef typename buffer_type::iterator          iterator;
        typedef std::pair<buffer_type, std::size_t>     value_type;
        
        wellformed_generator(code_point_t first = 0, code_point_t last = 0x110000);
        
        bool        hasNext();
        value_type  next();
        code_point_t current() const;
    };
        
    
    // Generates (all) sequences of UTF-8 from an Unicode code point which
    // is a valid "Unicode scalar value" (Surrogates are not generated).
    //
    // Requires that json::unicode::convert_one(code_point, dest)
    // performs correct.
    template <typename EncodingT>
    class wellformed_generator<
        EncodingT,
        typename boost::enable_if<
            boost::is_same<UTF_8_encoding_tag, EncodingT>
        >::type
    >
    {
    public:
        
        typedef typename EncodingT::code_unit_type      code_unit_type;
        typedef boost::array<code_unit_type, 4>         buffer_type;
        typedef typename buffer_type::iterator          iterator;
        typedef std::pair<buffer_type, std::size_t>     value_type;
        
        wellformed_generator(code_point_t first = 0, code_point_t last = 0x110000) 
        : first_(0), last_(0x110000), current_(0)
        {
        }
        
        bool hasNext() {
            while (current_ != last_ and isSurrogate(current_)) {
                ++current_;
            }
            return current_ != last_ and not isSurrogate(current_);
        }
        
        value_type next() {
            value_type result;
            iterator dest = result.first.begin();
            int res = convert_one_unsafe(current_, dest);
            assert( (utf8_encoded_length(current_) == res) and res <= 4 and res >= 1);
            assert( std::distance(result.first.begin(), dest) == res );
            ++current_;
            result.second = res;
            return result;
        }
        
        code_point_t current() const { return current_; }
        
    private:
        code_point_t first_;
        code_point_t last_;
        code_point_t current_;
    };

    
    // Generates (all) sequences of UTF-16 from an Unicode code point which
    // is a valid "Unicode scalar value" (Surrogates are not generated).
    //
    // Requires that json::unicode::convert_one(code_point, dest)
    // performs correct.
    template <typename EncodingT>
    class wellformed_generator<
        EncodingT,
        typename boost::enable_if<
            boost::is_same<UTF_16_encoding_tag, EncodingT>
        >::type
    > 
    {
    public:
        
        typedef typename EncodingT::code_unit_type      code_unit_type;
        typedef boost::array<code_unit_type, 2>         buffer_type;
        typedef typename buffer_type::iterator          iterator;
        typedef std::pair<buffer_type, std::size_t>     value_type;
        
        // Upgrade Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_16_encoding_tag, EncodingT>,
            typename to_host_endianness<EncodingT>::type,
            EncodingT 
        >::type                                         to_encoding_t;        

        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
                
        
        wellformed_generator(code_point_t first = 0, code_point_t last = 0x110000) 
        : first_(0), last_(0x110000), current_(0)
        {
        }
        
        bool hasNext() {
            while (current_ != last_ and isSurrogate(current_)) {
                ++current_;
            }
            return current_ != last_ and not isSurrogate(current_);
        }
        
        value_type next() {
            value_type result;
            iterator dest = result.first.begin();
            int res = convert_one_unsafe(current_, dest, to_encoding_t());
            assert( (utf16_encoded_length(current_) == res) and res <= 2 and res >= 1);
            assert( std::distance(result.first.begin(), dest) == res );
            ++current_;
            result.second = res;
            return result;
        }
        
        code_point_t current() const { return current_; }
        
    private:
        code_point_t first_;
        code_point_t last_;
        code_point_t current_;
    };
    
    
    // Generates (all) sequences of UTF-32 from an Unicode code point which
    // is a valid "Unicode scalar value" (Surrogates are not generated).
    //
    // Requires that json::unicode::convert_one(code_point, dest)
    // performs correct.
    template <typename EncodingT>
    class wellformed_generator<
        EncodingT,
        typename boost::enable_if<
            boost::is_same<UTF_32_encoding_tag, EncodingT>
        >::type
    > 
    {
    public:
        
        typedef typename EncodingT::code_unit_type      code_unit_type;
        typedef boost::array<code_unit_type, 1>         buffer_type;
        typedef typename buffer_type::iterator          iterator;
        typedef std::pair<buffer_type, std::size_t>     value_type;
        
        // Upgrade Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_32_encoding_tag, EncodingT>,
            typename to_host_endianness<EncodingT>::type,
            EncodingT 
        >::type                                         to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        
        wellformed_generator(code_point_t first = 0, code_point_t last = 0x110000) 
        : first_(0), last_(0x110000), current_(0)
        {
        }
        
        bool hasNext() {
            while (current_ != last_ and isSurrogate(current_)) {
                ++current_;
            }
            return current_ != last_ and not isSurrogate(current_);
        }
        
        value_type next() {
            value_type result;
            iterator dest = result.first.begin();
            int res = convert_one_unsafe(current_, dest, to_encoding_t());
            assert(1 == res);
            assert( std::distance(result.first.begin(), dest) == res );
            ++current_;
            result.second = res;
            return result;
        }
        
        code_point_t current() const { return current_; }
        
    private:
        code_point_t first_;
        code_point_t last_;
        code_point_t current_;
    };
    
    
    typedef wellformed_generator<UTF_8_encoding_tag> utf8_wellformed_generator;
    typedef wellformed_generator<UTF_16_encoding_tag> utf16_wellformed_generator;
    typedef wellformed_generator<UTF_32_encoding_tag> utf32_wellformed_generator;
    
    
}}}  // namespace json::unicode::test



namespace {
    
    
    using namespace json::unicode;
    
    
    class unicode_conversions_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        unicode_conversions_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~unicode_conversions_test() {
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
    

#pragma mark - UTF8_to_UTF8
    TEST_F(unicode_conversions_test, UTF8_to_UTF8) 
    {      
        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::utf8_wellformed_generator;
        using json::unicode::UTF_8_encoding_tag;
        
        typedef utf8_wellformed_generator generator_t;
        typedef generator_t::code_unit_type code_unit_t;
        typedef generator_t::buffer_type buffer_t;
        typedef generator_t::value_type value_t;
        
        generator_t generator;
        
        uint8_t result_buffer_a[4];
        uint8_t* dest_a = 0;
        uint8_t result_buffer_b[4];
        uint8_t* dest_b = 0;
        
        
                
        while (generator.hasNext()) 
        {
            const value_t source_buffer = generator.next();
            std::size_t reference_length = source_buffer.second;
            const char* reference_buffer = &(source_buffer.first[0]);

            // A.3
            dest_a = result_buffer_a;
            buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = convert_one(first_a, source_buffer.first.end(), dest_a, UTF_8_encoding_tag());
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.3
            dest_b = result_buffer_b;
            buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = convert_one_unsafe(first_b, source_buffer.first.end(), dest_b, UTF_8_encoding_tag());
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0);
            EXPECT_EQ(source_buffer.second, consumed_a);
            EXPECT_EQ(reference_length, result_a);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, reference_length) );
            
            
            EXPECT_TRUE(result_b > 0);
            EXPECT_EQ(source_buffer.second, consumed_b);
            EXPECT_EQ(reference_length, result_b);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, reference_length) );
        }
        
    } 
    
    
#pragma mark - UTF8_to_UTF16
    
    TEST_F(unicode_conversions_test, UTF8_to_UTF16) 
    {      
        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::utf8_wellformed_generator;
        
        typedef utf8_wellformed_generator generator_t;
        typedef generator_t::code_unit_type code_unit_t;
        typedef generator_t::buffer_type buffer_t;
        typedef generator_t::value_type value_t;
        
        generator_t generator; // = generator_t(0x10000);
        
        uint16_t result_buffer_a[4];
        uint16_t* dest_a = 0;
        uint16_t result_buffer_b[4];
        uint16_t* dest_b = 0;
        
        UChar reference_buffer[8];
        int32_t reference_buffer_size = static_cast<int32_t>(sizeof(reference_buffer)/sizeof(UChar));
        int32_t reference_length;
        
        while (generator.hasNext()) 
        {
            
            code_point_t code_point = generator.current();
            const value_t source_buffer = generator.next();
            
            // Create the reference from ICU
            const char* source = static_cast<const char*>(&(source_buffer.first[0]));
            int32_t source_length = static_cast<int32_t>(source_buffer.second);
            UErrorCode error = U_ZERO_ERROR;
            u_strFromUTF8(reference_buffer, reference_buffer_size, &reference_length, source, source_length, &error);
            assert(error == 0);
            
            
            // A.3
            dest_a = result_buffer_a;
            buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = convert_one(first_a, source_buffer.first.end(), dest_a);
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.3
            dest_b = result_buffer_b;
            buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = convert_one_unsafe(first_b, source_buffer.first.end(), dest_b);
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0);
            EXPECT_EQ(source_buffer.second, consumed_a);
            EXPECT_EQ(reference_length, result_a);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, reference_length*sizeof(UChar)) );

        
            EXPECT_TRUE(result_b > 0);
            EXPECT_EQ(source_buffer.second, consumed_b);
            EXPECT_EQ(reference_length, result_b) << "with code point: U+" << std::hex << code_point;
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, reference_length*sizeof(UChar)) );
        }
        
    } 
    
    
#pragma mark - UTF8_to_UTF32
    
    TEST_F(unicode_conversions_test, UTF8_to_UTF32) {
        
        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::wellformed_generator;
        
        typedef UTF_8_encoding_tag      InEncoding;
        typedef UTF_32_encoding_tag     OutEncoding;

        
        typedef wellformed_generator<InEncoding> generator_t;
        typedef generator_t::code_unit_type code_unit_t;
        typedef generator_t::buffer_type buffer_t;
        typedef generator_t::value_type value_t;

        typedef to_host_endianness<InEncoding>::type InHostEncoding;
        typedef InHostEncoding::code_unit_type in_code_unit_t;

        typedef to_host_endianness<OutEncoding>::type OutHostEncoding;
        typedef OutHostEncoding::code_unit_type out_code_unit_t;
        
        
        generator_t generator;
        
        // OutEncoding result
        out_code_unit_t result_buffer_a[4];
        out_code_unit_t* dest_a = 0;
        out_code_unit_t result_buffer_b[4];
        out_code_unit_t* dest_b = 0;
        
        // OutEncoding ICU reference
        out_code_unit_t reference_buffer[8];
        int32_t reference_length; // length in code units
        
        while (generator.hasNext()) 
        {
            const value_t source_buffer = generator.next();
            
            // Create the reference from ICU
            const char* source = static_cast<const char*>(&(source_buffer.first[0]));
            int32_t source_size = static_cast<int32_t>(source_buffer.second*sizeof(code_unit_t));
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert(OutHostEncoding().name() /*to*/, InHostEncoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    source,  source_size, &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes / sizeof(out_code_unit_t);
            
            
            // A.3
            dest_a = result_buffer_a;
            buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = convert_one(first_a, source_buffer.first.end(), dest_a);
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.3
            dest_b = result_buffer_b;
            buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = convert_one_unsafe(first_b, source_buffer.first.end(), dest_b);
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0);
            EXPECT_EQ(source_buffer.second, consumed_a);
            EXPECT_EQ(reference_length, result_a);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, icu_source_bytes) );
            
            
            EXPECT_TRUE(result_b > 0);
            EXPECT_EQ(source_buffer.second, consumed_b);
            EXPECT_EQ(reference_length, result_b);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, icu_source_bytes) );
        }
        
    } 
    
    
#pragma mark - UTF16_to_UTF8
    
    TEST_F(unicode_conversions_test, UTF16_to_UTF8) 
    {                
        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::utf16_wellformed_generator;
        using json::unicode::UTF_8_encoding_tag;
        
        typedef utf16_wellformed_generator generator_t;
        typedef generator_t::code_unit_type code_unit_t;
        typedef generator_t::buffer_type buffer_t;
        typedef generator_t::value_type value_t;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        generator_t generator;
        
        // UTF-8 result
        uint8_t result_buffer_a[8];
        uint8_t* dest_a = 0;
        uint8_t result_buffer_b[8];
        uint8_t* dest_b = 0;
        
        // UTF-8 ICU reference
        char* reference_buffer[8];
        int32_t reference_length; // length in code units
        
        while (generator.hasNext()) 
        {
            const value_t source_buffer = generator.next();
            
            // Create the reference from ICU
            const char* source = reinterpret_cast<const char*>(&(source_buffer.first[0]));
            int32_t source_size = static_cast<int32_t>(source_buffer.second*sizeof(code_unit_t));
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert("UTF-8" /*to*/, UTF16_Host_Encoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    source,  source_size, &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes / sizeof(uint8_t);
            
            
            // A.3
            dest_a = result_buffer_a;
            buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = convert_one(first_a, source_buffer.first.end(), dest_a, UTF_8_encoding_tag());
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.3
            dest_b = result_buffer_b;
            buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = convert_one_unsafe(first_b, source_buffer.first.end(), dest_b, UTF_8_encoding_tag());
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0);
            EXPECT_EQ(source_buffer.second, consumed_a);
            EXPECT_EQ(reference_length, result_a);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, icu_source_bytes) );
            
            
            EXPECT_TRUE(result_b > 0);
            EXPECT_EQ(source_buffer.second, consumed_b);
            EXPECT_EQ(reference_length, result_b);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, icu_source_bytes) );
        }
        
    } 
    

#pragma mark - UTF16_to_UTF32
    
    TEST_F(unicode_conversions_test, UTF16_to_UTF32) 
    {
        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::utf16_wellformed_generator;
        using json::unicode::UTF_32_encoding_tag;
        
        typedef utf16_wellformed_generator generator_t;
        typedef generator_t::code_unit_type code_unit_t;
        typedef generator_t::buffer_type buffer_t;
        typedef generator_t::value_type value_t;
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        generator_t generator;
        
        // UTF-32 result
        uint32_t result_buffer_a[8];
        uint32_t* dest_a = 0;
        uint32_t result_buffer_b[8];
        uint32_t* dest_b = 0;
        
        // UTF-32 ICU reference
        char* reference_buffer[8];
        int32_t reference_length; // length in code units
        
        while (generator.hasNext()) 
        {
            const value_t source_buffer = generator.next();
            
            // Create the reference from ICU
            const char* source = reinterpret_cast<const char*>(&(source_buffer.first[0]));
            int32_t source_size = static_cast<int32_t>(source_buffer.second*sizeof(code_unit_t));
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert(UTF32_Host_Encoding().name() /*to*/, UTF16_Host_Encoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    source,  source_size, &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes / sizeof(uint32_t);
            
            
            // A.3
            dest_a = result_buffer_a;
            buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = convert_one(first_a, source_buffer.first.end(), dest_a, UTF_32_encoding_tag());
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.3
            dest_b = result_buffer_b;
            buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = convert_one_unsafe(first_b, source_buffer.first.end(), dest_b, UTF_32_encoding_tag());
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0);
            EXPECT_EQ(source_buffer.second, consumed_a);
            EXPECT_EQ(reference_length, result_a);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, icu_source_bytes) );
            
            
            EXPECT_TRUE(result_b > 0);
            EXPECT_EQ(source_buffer.second, consumed_b);
            EXPECT_EQ(reference_length, result_b);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, icu_source_bytes) );
        }
        
    } 
    
    
#pragma mark - UTF32_to_UTF8
    
    TEST_F(unicode_conversions_test, UTF32_to_UTF8) 
    {
        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::utf32_wellformed_generator;
        using json::unicode::UTF_32_encoding_tag;
        using json::unicode::UTF_8_encoding_tag;
        
        typedef utf32_wellformed_generator generator_t;
        typedef generator_t::code_unit_type code_unit_t;
        typedef generator_t::buffer_type buffer_t;
        typedef generator_t::value_type value_t;
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        generator_t generator;
        
        // UTF-8 result
        uint8_t result_buffer_a[8];
        uint8_t* dest_a = 0;
        uint8_t result_buffer_b[8];
        uint8_t* dest_b = 0;
        
        // UTF-8 ICU reference
        char* reference_buffer[8];
        int32_t reference_length; // length in code units
        
        while (generator.hasNext()) 
        {
            const value_t source_buffer = generator.next();
            
            // Create the reference from ICU
            const char* source = reinterpret_cast<const char*>(&(source_buffer.first[0]));
            int32_t source_size = static_cast<int32_t>(source_buffer.second*sizeof(code_unit_t));
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert("UTF-8" /*to*/, UTF32_Host_Encoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    source,  source_size, &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes / sizeof(char);
            
            
            // A.3
            dest_a = result_buffer_a;
            buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = convert_one(first_a, source_buffer.first.end(), dest_a, UTF_8_encoding_tag());
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.3
            dest_b = result_buffer_b;
            buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = convert_one_unsafe(first_b, source_buffer.first.end(), dest_b, UTF_8_encoding_tag());
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0);
            EXPECT_EQ(source_buffer.second, consumed_a);
            EXPECT_EQ(reference_length, result_a);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, icu_source_bytes) );
            
            
            EXPECT_TRUE(result_b > 0);
            EXPECT_EQ(source_buffer.second, consumed_b);
            EXPECT_EQ(reference_length, result_b);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, icu_source_bytes) );
        }
        
    } 
    

#pragma mark - UTF32_to_UTF16
    
    TEST_F(unicode_conversions_test, UTF32_to_UTF16) 
    {
        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::utf32_wellformed_generator;
        using json::unicode::UTF_32_encoding_tag;
        using json::unicode::UTF_16_encoding_tag;
        
        typedef utf32_wellformed_generator generator_t;
        typedef generator_t::code_unit_type code_unit_t;
        typedef generator_t::buffer_type buffer_t;
        typedef generator_t::value_type value_t;
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        generator_t generator;
        
        // UTF-16 result
        uint16_t result_buffer_a[8];
        uint16_t* dest_a = 0;
        uint16_t result_buffer_b[8];
        uint16_t* dest_b = 0;
        
        // UTF-8 ICU reference
        char* reference_buffer[8];
        int32_t reference_length; // length in code units
        
        while (generator.hasNext()) 
        {
            const value_t source_buffer = generator.next();
            
            // Create the reference from ICU
            const char* source = reinterpret_cast<const char*>(&(source_buffer.first[0]));
            int32_t source_size = static_cast<int32_t>(source_buffer.second*sizeof(code_unit_t));
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert(UTF16_Host_Encoding().name() /*to*/, UTF32_Host_Encoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    source,  source_size, &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes / sizeof(uint16_t);
            
            
            // A.3
            dest_a = result_buffer_a;
            buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = convert_one(first_a, source_buffer.first.end(), dest_a, UTF_16_encoding_tag());
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.3
            dest_b = result_buffer_b;
            buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = convert_one_unsafe(first_b, source_buffer.first.end(), dest_b, UTF_16_encoding_tag());
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0);
            EXPECT_EQ(source_buffer.second, consumed_a);
            EXPECT_EQ(reference_length, result_a);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, icu_source_bytes) );
            
            
            EXPECT_TRUE(result_b > 0);
            EXPECT_EQ(source_buffer.second, consumed_b);
            EXPECT_EQ(reference_length, result_b);
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, icu_source_bytes) );
        }
        
    } 
    
    
    
    
#pragma mark -
    
    TEST_F(unicode_conversions_test, UTF8_to_UTF16_to_UTF8) 
    {
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        code_point_t cp = 0;
        code_point_t last = 0x10FFF + 1;
        
        char utf8_source_buffer[8];
        const char* utf8_source = utf8_source_buffer;
        int utf8_source_size;
        
        std::size_t fail_count = 0;
        
        while (cp != last) {
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert("UTF-8" /*to*/, UTF32_Host_Encoding().name() /*from*/, 
                                                   reinterpret_cast<char*>(utf8_source_buffer), sizeof(utf8_source_buffer), 
                                                   reinterpret_cast<const char*>(&cp),  sizeof(cp), &error); 
            assert(error == U_ZERO_ERROR);
            utf8_source_size = icu_source_bytes;
            
            uint16_t utf16_reference_buffer[4];
            const uint16_t* utf16_reference = utf16_reference_buffer;
            int utf16_reference_size;
            icu_source_bytes = ucnv_convert(UTF16_Host_Encoding().name() /*to*/, "UTF-8" /*from*/, 
                                                    reinterpret_cast<char*>(utf16_reference_buffer), sizeof(utf16_reference_buffer), 
                                                    reinterpret_cast<const char*>(utf8_source_buffer), utf8_source_size, &error); 
            assert(error == U_ZERO_ERROR);
            utf16_reference_size = icu_source_bytes/sizeof(uint16_t);
            
            const char* utf8_first = utf8_source_buffer;
            const char* const utf8_last = utf8_first + sizeof(utf8_source_buffer);
            uint16_t utf16_dest_buffer[8];
            uint16_t* utf16_dest = utf16_dest_buffer;
            int result1 = convert_one(utf8_first, utf8_last, UTF_8_encoding_tag(), utf16_dest, UTF16_Host_Encoding());
            if (isNonCharacter(cp)) {
                // ICU will convert noncharacters code points to corresponding
                // UTF-8. The reference buffer thus contains valid UTF-8.
                // Without filters set, convert_one() shall convert noncharacters
                // in UTF-8 to its corresponding UTF-16 form successfully.
                EXPECT_EQ(utf16_reference_size, result1);                   // expected result
                EXPECT_EQ(utf8_source_size, std::distance(utf8_source, utf8_first));  // number consumed
                EXPECT_EQ(utf16_reference_size, std::distance(utf16_dest_buffer, utf16_dest)); // produced
                EXPECT_TRUE(result1 == 1 or result1 == 2);
                EXPECT_TRUE( memcmp(utf16_reference_buffer, utf16_dest_buffer, utf16_reference_size*sizeof(uint16_t))== 0 );
            }
            else if (isSurrogate(cp) or !isCodePoint(cp)) {
                // ICU converted the source code point to an Unicode Replacement 
                // Character (U+FFFD) in UTF-8. The UTF-16 reference buffer thus 
                // contains this replacement character.
                EXPECT_EQ(0xFFFD, *utf16_reference_buffer);
                // Without filters set, convert_one() shall convert the replacement
                // character from UTF-8 to UTF-16 without errors.
                EXPECT_EQ(0xFFFD, *utf16_dest_buffer);
                EXPECT_EQ(1, result1);
                EXPECT_EQ(utf8_source_size, std::distance(utf8_source, utf8_first));  // number consumed
                EXPECT_EQ(1, std::distance(utf16_dest_buffer, utf16_dest)); // produced
                EXPECT_TRUE(result1 == 1);
                //EXPECT_TRUE( memcmp(utf16_reference_buffer, utf16_dest_buffer, utf16_reference_size*sizeof(uint16_t))== 0 );
            }
            else {
                if (utf16_reference_size != result1) {
                    ++fail_count;
                }
                // ICU will convert normal code points to corresponding
                // UTF-8. The reference buffer thus contains valid UTF-8.
                // Without filters set, convert_one() shall convert normal characters
                // in UTF-8 to its corresponding UTF-16 form successfully.
                EXPECT_EQ(utf16_reference_size, result1);                   // expected result
                EXPECT_EQ(utf8_source_size, std::distance(utf8_source, utf8_first));  // number consumed
                EXPECT_EQ(utf16_reference_size, std::distance(utf16_dest_buffer, utf16_dest)); // produced
                EXPECT_TRUE(result1 == 1 or result1 == 2);
                EXPECT_TRUE( memcmp(utf16_reference_buffer, utf16_dest_buffer, utf16_reference_size*sizeof(uint16_t))== 0 );
            }
            
            
            // Convert UTF-16 back to UTF-8
            uint8_t utf8_buffer[8];
            uint8_t* utf8_dest = utf8_buffer;
            const uint16_t* utf16_first = utf16_reference_buffer;
            const uint16_t* utf16_last = utf16_first + sizeof(utf16_reference_buffer);
            int result2 = convert_one(utf16_first, utf16_last, UTF_16_encoding_tag(), utf8_dest, UTF_8_encoding_tag());
            if (isNonCharacter(cp)) {
                // With no filter set, noncharacter UTF-8 has been converted
                // successful to UTF-16 with convert_one - now convert it back
                // from UTF-16 to UTF-8 which should not fail as well:                
                EXPECT_EQ(utf8_source_size, result2);  // num produced
                EXPECT_TRUE(result2 >= 1 and result2 <= 4);
                EXPECT_EQ(utf16_reference_size, std::distance(utf16_reference, utf16_first));  // consumed
                EXPECT_EQ(utf8_source_size, std::distance(utf8_buffer, utf8_dest));    // produced
                EXPECT_TRUE( memcmp(utf8_source_buffer, utf8_buffer, utf8_source_size)== 0 );
            }
            else if (isSurrogate(cp) or !isCodePoint(cp)) {
                // ICU converted this code point to an Unicode Replacement Character (U+FFFD)
                // The UTF-8 source should have been converted to UTF-16 without 
                // errors, so we convert it back to UTF-8 which should succeed as
                // well, and result in the replacement character:
                EXPECT_EQ(utf8_source_size, result2); // num produced
                EXPECT_EQ(utf16_reference_size, std::distance(utf16_reference, utf16_first));  // consumed
                EXPECT_EQ(utf8_source_size, std::distance(utf8_buffer, utf8_dest));    // produced
                EXPECT_TRUE( memcmp(utf8_source_buffer, utf8_buffer, utf8_source_size)== 0 );
            }
            else {
                if (utf8_source_size != result2) {
                    ++fail_count;
                }
                EXPECT_EQ(utf8_source_size, result2) << "with code point: U+" << std::hex << cp;                    
                EXPECT_EQ(utf16_reference_size, std::distance(utf16_reference, utf16_first));  // consumed
                EXPECT_EQ(utf8_source_size, std::distance(utf8_buffer, utf8_dest))  << "with code point: U+" << std::hex << cp;   // produced
                EXPECT_TRUE( memcmp(utf8_source_buffer, utf8_buffer, utf8_source_size)== 0 );
            }
            
            
            
            ++cp;
            EXPECT_TRUE(fail_count < 10) << "Test aborted due to too many failures.";
            if (fail_count >= 10)                
                break;
        }
        
    }
    
    
#pragma mark -
    
    TEST_F(unicode_conversions_test, UTF8_to_UTF32_to_UTF8) {
    }
    
    
#pragma mark -
    
    TEST_F(unicode_conversions_test, UTF16_to_UTF8_to_UTF16) {
    }
    

#pragma mark -

    TEST_F(unicode_conversions_test, UTF16_to_UTF32_to_UTF16) {
    }
    
#pragma mark -
    
    TEST_F(unicode_conversions_test, UTF32_to_UTF8_to_UTF32) {
    }
    
#pragma mark -
    
    TEST_F(unicode_conversions_test, UTF32_to_UTF16_to_UTF16) {
    }
    
    
    
    
#pragma mark -
    
    
#pragma mark -
#pragma mark Unicode Code Point
    

    TEST_F(unicode_conversions_test, UnicodeCodePointToUTF8) 
    {
        const char* utf8_string = "\u51fa\u5565\u72b6\u51b5\u4e86\uff1f"
                                  "   RT: @bzcai: "
                                  "\u62bd!!(\uffe3\u03b5(#\uffe3)\u2606\u2570\u256e("
                                  "\uffe3\u25bd\uffe3///) RT @ksky: @bzcai \u5305\u5b50,"
                                  "\u6211\u5929\u5929\u68a6\u89c1\u4f60...";

        const std::size_t utf8_string_len = strlen(utf8_string);

        
        // Compile the source string in encoding UTF-32 (host endianness) from 
        // the given string literal:
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;

        utf32_code_unit source[512];
        UErrorCode error = U_ZERO_ERROR;
        int32_t bytes = ucnv_convert(UTF32_Host_Encoding().name() /*to*/, "UTF-8" /*from*/, 
                                                reinterpret_cast<char*>(source), sizeof(source), 
                                                utf8_string,  -1, &error); 
        int utf32_string_len = bytes / sizeof(utf32_code_unit);
        assert(error == U_ZERO_ERROR);
        
        
        // Unicode code point array to UTF-8
        // ICU a appends the BOM in this case. We take this into account.
        
        utf8_code_unit dest[1024];
        utf8_code_unit* dest_p = dest;
        const utf32_code_unit* first = source;
        const utf32_code_unit* last = first + utf32_string_len;
        size_t count = 0;
        while (first != last) {
            int n = unicode::convert_one(*first++, dest_p, UTF_8_encoding_tag());
            EXPECT_TRUE(n>0) << "function unicode::convert failed";
            if (n <= 0) {
                // ERROR
                break;
            }
            else {
                count += n;
            }
        }
        
        // zero terminate:
        dest[count] = 0;
        EXPECT_EQ(utf8_string_len, count)  << "reference UTF-8: \"" << utf8_string << "\"\n" 
                                           << "generated UTF-8: \"" << dest << "\"" << std::endl;
        
        if (count == utf8_string_len) {
            EXPECT_EQ(0, memcmp(utf8_string, dest, utf8_string_len)) 
                    << "reference UTF-8: \"" << utf8_string << "\"\n" 
                    << "generated UTF-8: \"" << dest << "\"" << std::endl;
        }
    }

    
    
    TEST_F(unicode_conversions_test, UnicodeCodePointToUTF16) 
    {
#if 1       
        const char* utf8_string = "\u51fa\u5565\u72b6\u51b5\u4e86\uff1f"
        "   RT: @bzcai: "
        "\u62bd!!(\uffe3\u03b5(#\uffe3)\u2606\u2570\u256e("
        "\uffe3\u25bd\uffe3///) RT @ksky: @bzcai \u5305\u5b50,"
        "\u6211\u5929\u5929\u68a6\u89c1\u4f60...";
#else
        const char* utf8_string = "abcdefgh";
#endif        
        
        // Compile the source string encoded in UTF-32 (host-endian) from the 
        // given string literal:
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        
        utf32_code_unit source_buffer[512];
        utf32_code_unit* source = source_buffer;
        UErrorCode error = U_ZERO_ERROR;
        int32_t bytes = ucnv_convert(UTF32_Host_Encoding().name() /*to*/, "UTF-8" /*from*/, 
                                     reinterpret_cast<char*>(source_buffer), sizeof(source_buffer), 
                                     utf8_string,  -1, &error); 
        assert(error == U_ZERO_ERROR);
        int source_len = bytes / sizeof(utf32_code_unit);
        source_buffer[source_len] = 0;
//        // ICU a appends the BOM. We take this into account.
//        ++source;
//        --source_len;
        
        // Compile the reference string encoded in UTF-16 (Host endianness):
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        utf16_code_unit reference_buffer[512];
        utf16_code_unit* reference = reference_buffer;
        error = U_ZERO_ERROR;
        bytes = ucnv_convert(UTF16_Host_Encoding().name() /*to*/, "UTF-8" /*from*/, 
                                     reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                     utf8_string,  -1, &error); 
        assert(error == U_ZERO_ERROR);
        int reference_len = bytes / sizeof(utf16_code_unit);        
        reference_buffer[reference_len] = 0;
//        // ICU a appends the BOM. We take this into account.
//        ++reference;
//        --reference_len;
        
        
        
        
        // UTF-32 to UTF-16
        typedef to_host_endianness<UTF_16_encoding_tag>::type Encoding;
        
        utf16_code_unit dest[1024];
        utf16_code_unit* dest_p = dest;
        const utf32_code_unit* first = source;
        const utf32_code_unit* last = first + source_len;
        size_t dest_len = 0;
        while (first != last) {
            int n = unicode::convert_one(*first++, dest_p, Encoding());
            EXPECT_TRUE(n>0) << "function unicode::convert failed";
            if (n <= 0) {
                // ERROR
                break;
            }
            else {
                dest_len += n;
            }
        }
        // zero terminate:
        dest[dest_len] = 0;
        
        EXPECT_EQ(reference_len, dest_len);
        
        if (dest_len == reference_len) {
            EXPECT_EQ(0, memcmp(reference, dest, reference_len));
        }
    }
    
}

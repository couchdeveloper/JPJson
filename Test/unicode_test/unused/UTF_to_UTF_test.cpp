//
//  UTF_to_UTF_test.cpp
//  Test
//
//  Created by Andreas Grosam on 8/19/11.
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

#error Obsolete

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
    
    using namespace json;    
    
    // Return a vector of Unicode codepoints with
    std::vector<code_point_t> 
    create_codepoint_source(code_point_t lower_bound, code_point_t upper_bound, 
                            bool validOnly = true, bool shuffle = false)
    {
        using unicode::code_point_t;
        using unicode::isUnicodeScalarValue
        
        typename std::vector<code_point_t> result;
        
        for (code_point_t cp = lower_bound; cp <= upper_bound; ++cp) {
            if (validOnly and !isUnicodeScalarValue(cp))
                continue;
            result.push_back(cp);
        }
        if (shuffle) {
            std::random_shuffle(result.begin(), result.end());
        }
        return result;
    }
    
    
    // Return a vector of code units in encoding 'EncodingT', whose corresponding
    // Unicode code points are in the range [lower_bound .. upper_bound].
    template <typename EncodingT>
    std::vector<typename unicode::encoding_traits<EncodingT>::code_unit_type>
    create_utf_source(code_point_t lower_bound, code_point_t upper_bound, 
                      bool validOnly = true, bool shuffle = false)
    {
        using unicode::code_point_t;
        using unicode::add_endianness;
        using unicode::encoding_traits;
        
        typedef typename add_endianness<EncodingT>::type encoding_type;
        typedef typename encoding_traits<encoding_type>::code_unit_type char_t;
        typedef std::vector<char_t> result_t;
        
        
        std::vector<code_point_t> source = create_codepoint_source(lower_bound, upper_bound,
                                                                   validOnly, shuffle);
        
        // Now convert the code points into the specified encoding and return
        // the result. Note: Unicode code points can be seen as Unicode code 
        // units encoded in UTF-32 in host endianness.
        result_t result;        
        std::vector<code_point_t>::iterator first = source.begin();
        std::back_insert_iterator<result_t> dest = std::back_inserter(result);
        // convert code_point to UTF
        int state = 0; // state shall be initially zero
        int res = unicode::convert(first, source.end(), unicode::UTF_32_encoding_tag(), 
                                   dest, encoding_type(), 
                                   state);     
        if (res < 0) {
            throw std::runtime_error("error while creating UTF source");
        }
        
        return result;
    }
    
}}    
    
    
namespace json { namespace unicode { namespace test {
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
        : first_(first), last_(last), current_(first)
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
            int res = convert_from_codepoint_unsafe(current_, dest);
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
        boost::is_base_of<UTF_16_encoding_tag, EncodingT>
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
        : first_(first), last_(last), current_(first)
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
            int res = convert_from_codepoint_unsafe(current_, dest, to_encoding_t());
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
            boost::is_base_of<UTF_32_encoding_tag, EncodingT>
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
        : first_(first), last_(last), current_(first)
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
            int res = convert_from_codepoint_unsafe(current_, dest, to_encoding_t());
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
    
    
    template <typename InEncodingT, typename OutEncodingT>
    struct conversion {
        typedef InEncodingT     from_encoding_t;
        typedef OutEncodingT    to_encoding_t;
    };
    
    
    template <typename T>
    class UTF_to_UTF_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        UTF_to_UTF_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~UTF_to_UTF_test() {
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
    
    
    
    TYPED_TEST_CASE_P(UTF_to_UTF_test);

    
    TYPED_TEST_P(UTF_to_UTF_test, convert_one) 
    {
        // Inside a test, refer to TypeParam to get the type parameter.

        // Requires that json::unicode::convert_one(code_point, dest)
        // performs correct.
        using json::unicode::test::wellformed_generator;
        using json::unicode::add_endianness;
        
        typedef typename TypeParam::from_encoding_t         InEncoding;
        typedef typename TypeParam::to_encoding_t           OutEncoding;
        
        // Upgrade Encoding to include endianness if required:
        typedef typename add_endianness<InEncoding>::type   from_encoding_t;        
        typedef typename add_endianness<OutEncoding>::type  to_encoding_t;        
        
                
        typedef typename from_encoding_t::code_unit_type    in_code_unit_t;
        typedef typename to_encoding_t::code_unit_type      out_code_unit_t;
        

        
        
        
        generator_t generator = generator_t(0x000000);
        
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
            code_point_t code_point = generator.current();
            const value_t source_buffer = generator.next();
            
            // Create the reference from ICU
            const char* source = reinterpret_cast<const char*>(&(source_buffer.first[0]));
            int32_t source_size = static_cast<int32_t>(source_buffer.second*sizeof(code_unit_t));
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert(to_encoding_t().name() /*to*/, from_encoding_t().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    source,  source_size, &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes / sizeof(out_code_unit_t);
            
            
            // A.1
            dest_a = result_buffer_a;
            typename buffer_t::const_iterator first_a = source_buffer.first.begin();
            int result_a = unicode::convert_one(first_a, source_buffer.first.end(), InEncoding(), dest_a, OutEncoding());
            int consumed_a = (int)std::distance(source_buffer.first.begin(), first_a);
            
            // B.1
            dest_b = result_buffer_b;
            typename buffer_t::const_iterator first_b = source_buffer.first.begin();
            int result_b = unicode::convert_one_unsafe(first_b, source_buffer.first.end(), InEncoding(), dest_b, OutEncoding());
            int consumed_b = (int)std::distance(source_buffer.first.begin(), first_b);
            
            
            EXPECT_TRUE(result_a > 0) << "with code point: U+" << std::hex << code_point;
            EXPECT_EQ(source_buffer.second, consumed_a)  << "with code point: U+" << std::hex << code_point;
            EXPECT_EQ(reference_length, result_a)  << "with code point: U+" << std::hex << code_point;
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_a, icu_source_bytes) )  << "with code point: U+" << std::hex << code_point;
            
            
            EXPECT_TRUE(result_b > 0) << "with code point: U+" << std::hex << code_point;
            EXPECT_EQ(source_buffer.second, consumed_b) << "with code point: U+" << std::hex << code_point;
            EXPECT_EQ(reference_length, result_b) << "with code point: U+" << std::hex << code_point;
            EXPECT_EQ( 0, memcmp(reference_buffer, result_buffer_b, icu_source_bytes) ) << "with code point: U+" << std::hex << code_point;
        }
        
    
    }
    
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(UTF_to_UTF_test,
                               convert_one);
    
    
    // Instantiate test cases:
    typedef ::testing::Types<
    conversion<UTF_8_encoding_tag, UTF_8_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_16_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_32_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_32BE_encoding_tag>,    
    
    conversion<UTF_16_encoding_tag, UTF_8_encoding_tag>,
    conversion<UTF_16_encoding_tag, UTF_16_encoding_tag>,
    conversion<UTF_16_encoding_tag, UTF_32_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_32BE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_32BE_encoding_tag>,
    
    conversion<UTF_32_encoding_tag, UTF_8_encoding_tag>,
    conversion<UTF_32_encoding_tag, UTF_16_encoding_tag>,
    conversion<UTF_32_encoding_tag, UTF_32_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_32BE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_32BE_encoding_tag>
    >  UTF_conversions;
    

    INSTANTIATE_TYPED_TEST_CASE_P(Conversion, UTF_to_UTF_test, UTF_conversions);
    
    
}

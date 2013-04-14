//
//  unicode_converter_test.cpp
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

#include "json/unicode/unicode_conversion.hpp"
#include <gtest/gtest.h>

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
#include <stdlib.h>


using namespace json;

namespace test {

    using json::unicode::code_point_t;
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::utf_encoding_tag;
    using json::unicode::to_host_endianness;
    using json::unicode::encoding_traits;
    using json::unicode::add_endianness;

    
    enum Distribution {
        D0 = 0,
        D1,
        D2,
        D3,
        D4,
        D5,
        D6
    };
    
    struct distribution {double c0; double c1; double c2; double c3;};
    const distribution Distributions[7] = {
        {1, 0, 0, 0},               // D0
        {0, 1, 0, 0},               // D1
        {0, 0, 1, 0},               // D2
        {0, 0, 0, 1},               // D3
        {0.6, 0.25, 0.1, 0.05},     // D4
        {0.8, 0.15, 0.05, 0},       // D5
        {0.25, 0.25, 0.25, 0.25}    // D6
    }; 
    
    
    // Return a vector of Unicode codepoints with size N with a distribution d.
    // - c0% [1..0x1F]              (converts to single byte)
    // - c1% [0x80..0x7FF]          (converts to two bytes)
    // - c2% [0x800..0xFFFF]        (converts to three bytes)
    // - c3% [0x1000.. 0x10FFFF]    (converts to four bytes)
    inline
    std::vector<code_point_t> 
    __attribute__((noinline))    
    create_codepoint_source(std::size_t N, Distribution d, bool randomize = true, int seed = 0)
    {        
        // Valid ranges for Unicode code points encoded in UTF-8
        // 1 byte:  [0 .. 0x7F]
        // 2 byte:  [0xFF .. 0x7FF]
        // 3 bytes: [0x800 .. 0xD7FF], [0xE000 .. 0xFFFF]
        // 4 bytes: [0x10000 .. 0x10FFFF]

        typedef std::vector<code_point_t> result_t;
        
        srand(seed);
        
        // normalize:
        double C = Distributions[d].c0 + Distributions[d].c1 + Distributions[d].c2 + Distributions[d].c3;
        double c0 = Distributions[d].c0/C;
        double c1 = Distributions[d].c1/C;
        double c2 = Distributions[d].c2/C;
        double c3 = Distributions[d].c3/C;
        
        std::vector<code_point_t> source;
        source.reserve(N);
        
        std::size_t d0 = 0; 
        std::size_t d1 = 0;
        std::size_t d2 = 0;
        std::size_t d3 = 0;
        
        d0 = static_cast<std::size_t>((N * c0) + 0.5); 
        if (d0 < N) {
            d1 = std::min(std::size_t((N * c1) + 0.5), N-d0);
            if ((d0 + d1) < N) {
                d2 = std::min(std::size_t((N * c2) + 0.5), N-d0-d1);
                if ((d0+d1+d2) < N) {
                    d3 = std::min(std::size_t((N * c3) + 0.5), N-d0-d1-d2);
                }
            }                
        }
        for (std::size_t i = 0; i < d0; ++i) {
            code_point_t c = 0 + static_cast<uint8_t>(rand() % (0x7Fu+1-0));
            source.push_back(static_cast<code_point_t>(c));
        }
        for (std::size_t i = 0; i < d1; ++i) {
            code_point_t c = 0xFFu + static_cast<uint8_t>(rand() % (0x7FFu+1-0xFFu));
            source.push_back(c);
        }
        for (std::size_t i = 0; i < d2; ++i) {
            code_point_t c;
            while ( (c = 0x800u + static_cast<uint8_t>(rand() % (0xFFFF+1-0x800))) > 0xD7FF and c < 0xE000) {};
            source.push_back(c);
        }
        for (std::size_t i = 0; i < d3; ++i) {
            code_point_t c = 0x10000u + static_cast<uint8_t>(rand() % (0x10FFFFu+1-0x10000u));
            source.push_back(c); 
        }
        // The number of code points shall be N:
        assert(source.size() == N);
        if (randomize) {
            std::random_shuffle(source.begin(), source.end());
        }
        return source;
    }
    
    
    template <typename EncodingT>
    std::vector<typename encoding_traits<EncodingT>::code_unit_type>
    __attribute__((noinline))    
    convert_to_utf(std::vector<code_point_t> const& code_points, EncodingT encoding)
    {
        
        typedef typename add_endianness<EncodingT>::type encoding_type;
        typedef typename encoding_traits<encoding_type>::code_unit_type char_t;
        typedef std::vector<char_t> result_t;
        
        
        // Now convert the code points into the specified encoding and return
        // the result. Note: Unicode code points can be seen as Unicode code 
        // units encoded in UTF-32 in host endianness.
        result_t result;        
        std::vector<code_point_t>::const_iterator first = code_points.begin();
        std::back_insert_iterator<result_t> dest = std::back_inserter(result);
        // convert code_point to UTF
        int state = 0; // state shall be initially zero
        int res = unicode::convert(first, code_points.end(), unicode::UTF_32_encoding_tag(), 
                                   dest, encoding_type(), 
                                   state);     
        if (res < 0) {
            throw std::runtime_error("error while creating UTF source");
        }
        
        return result;
    }
    
    
    
}  // namespace test



namespace {
    
    
    using namespace json;
    
    using unicode::code_point_t;
    using unicode::converter;
    using unicode::isUnicodeScalarValue;
    using unicode::encoding_traits;
    using unicode::add_endianness;
    using unicode::UTF_8_encoding_tag;
    using unicode::UTF_16_encoding_tag;
    using unicode::UTF_32_encoding_tag;
    using unicode::UTF_16BE_encoding_tag;
    using unicode::UTF_16LE_encoding_tag;
    using unicode::UTF_32BE_encoding_tag;
    using unicode::UTF_32LE_encoding_tag;
    
    
    template <typename InEncodingT, typename OutEncodingT>
    struct conversion {
        typedef InEncodingT     from_encoding_t;
        typedef OutEncodingT    to_encoding_t;
    };
    
    
    template <typename T>
    class UtfToUtfConversionTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        UtfToUtfConversionTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~UtfToUtfConversionTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
            
            code_points_ = test::create_codepoint_source(1000*1000, test::D6);
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
        std::vector<code_point_t> code_points_;
    };
    
    
    TYPED_TEST_CASE_P(UtfToUtfConversionTest);
    
    
    TYPED_TEST_P(UtfToUtfConversionTest, WellformedUtfToUtfSafe) 
    {
        // This test requires that the previous test was successfully.
        
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        typedef typename TypeParam::from_encoding_t     from_encoding_t;
        typedef typename TypeParam::to_encoding_t       to_encoding_t;
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, from_encoding_t>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, to_encoding_t>::value) );
        
        
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type> target_vector_t;
        
        typedef converter<code_point_t, from_encoding_t, unicode::Validation::SAFE> tmp_converter_t;
        
        
        // Convert the code points to source_encoding:
        typename std::vector<code_point_t>::iterator cp_first = this->code_points_.begin();
        source_vector_t source_vector;
        std::back_insert_iterator<source_vector_t> source_dest(source_vector);
        int result = tmp_converter_t().convert(cp_first, this->code_points_.end(), source_dest);
        assert(result == 0);
        
        
        target_vector_t target_vector;
        // Convert the input in chunks of size 17
        typename source_vector_t::iterator first = source_vector.begin();
        typename source_vector_t::iterator last = source_vector.end();
        typename source_vector_t::iterator chunk_last = first;
        std::back_insert_iterator<target_vector_t> dest(target_vector);
        result = 0;
        unicode::mb_state<from_encoding_t> state;
        while (first != last and (result == 0 or result == unicode::E_UNEXPECTED_ENDOFINPUT)) {
            chunk_last += 17;
            if (chunk_last > last) {
                chunk_last = last;
            }
            assert(chunk_last <= last);
            result = unicode::convert(first, chunk_last, from_encoding_t(), dest, to_encoding_t(), state);        
        }
        EXPECT_TRUE(first == last) << "distance: " << std::distance(source_vector.begin(), first);
        EXPECT_FALSE( !state );
        EXPECT_EQ(0, result) << "*first = " << encoding_traits<from_encoding_t>::to_int(*first);
        if (result != 0) {
            while (0) {}
        }
        
        
        // Compare the converted bytes with ICU:
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert(unicode::encoding_traits<to_encoding_t>::name() /*to*/,
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()/sizeof(output_char_t)) {
            int memcmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, memcmp_result);
            
            // For better analysis check where the first difference occurred:
            typename target_vector_t::iterator first = target_vector.begin();
            typename target_vector_t::iterator last = target_vector.end();
            output_char_t* target_p_first = reinterpret_cast<output_char_t*>(static_cast<void*>(target));
            output_char_t* target_p_last = target_p_first + icu_output_bytes/sizeof(output_char_t);            
            size_t index = 0;
            while (first != last and target_p_first != target_p_last) {                
                if (*first != *target_p_first)                    
                {
                    std::cout << "differences at index: " << index << std::endl;
                    std::cout << std::hex << std::setw(2) << encoding_traits<to_encoding_t>::to_int(*first) << std::endl;
                    std::cout << std::hex << std::setw(2) << encoding_traits<to_encoding_t>::to_int(*target_p_first) << std::endl;
#if 0                    
                    typename target_vector_t::iterator s = first;
                    int offset = 0;
                    while (!encoding_traits<to_encoding_t>::is_single(*s)) {
                        --offset;
                        --s;
                    }
                    do {
                        std::cout << std::hex << std::setw(2) << *s << " ";
                        ++s;
                    } while (s != first);
                    std::cout << std::endl;
                    
                    output_char_t* st = target_p_first + offset;
                    do {
                        std::cout << std::hex << std::setw(2) << *st << " ";
                        ++st;
                    } while (st != target_p_first);
                    std::cout << std::endl;
#endif                    
                    
                    abort();
                }
                ++index;
                ++first;
                ++target_p_first;
            }
            
        }
        
        free(target);
    }
    
    
#if 0    
    TYPED_TEST_P(UtfToUtfConversionTest, WellformedUtfToUtfUnsafeNoCheckInputRange) 
    {
        // This test requires that the previous test was successfully.
        
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        typedef typename TypeParam::from_encoding_t     from_encoding_t;
        typedef typename TypeParam::to_encoding_t       to_encoding_t;
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, from_encoding_t>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, to_encoding_t>::value) );
        
        
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type> target_vector_t;
        
        typedef converter<code_point_t, from_encoding_t, unicode::Validation::SAFE> tmp_converter_t;
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::NO_CHECK_INPUT_RANGE> converter_t;
        
        
        // create a temp vector of valid code points, then convert them into the
        // required source encoding.
        std::vector<code_point_t> code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        typename std::vector<code_point_t>::iterator cp_first = code_points.begin();
        source_vector_t source_vector;
        std::back_insert_iterator<source_vector_t> source_dest(source_vector);
        int result = tmp_converter_t().convert(cp_first, code_points.end(), source_dest);
        assert(result == 0);
        
        
        target_vector_t target_vector;
        typename source_vector_t::iterator first = source_vector.begin();
        std::back_insert_iterator<target_vector_t> dest(target_vector);
        int state = 0;
        result = converter_t().convert(first, source_vector.end(), dest, state);
        
        EXPECT_EQ(0, state);
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert(unicode::encoding_traits<to_encoding_t>::name() /*to*/,
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()/sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToUtfConversionTest, WellformedUtfToUtfUnsafeNoValidation) 
    {
        // This test requires that the previous test was successfully.
        
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        typedef typename TypeParam::from_encoding_t     from_encoding_t;
        typedef typename TypeParam::to_encoding_t       to_encoding_t;
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, from_encoding_t>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, to_encoding_t>::value) );
        
        
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type> target_vector_t;
        
        typedef converter<code_point_t, from_encoding_t, unicode::Validation::SAFE> tmp_converter_t;
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::NO_VALIDATION> converter_t;
        
        
        // create a temp vector of valid code points, then convert them into the
        // required source encoding.
        std::vector<code_point_t> code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        typename std::vector<code_point_t>::iterator cp_first = code_points.begin();
        source_vector_t source_vector;
        std::back_insert_iterator<source_vector_t> source_dest(source_vector);
        int result = tmp_converter_t().convert(cp_first, code_points.end(), source_dest);
        assert(result == 0);
        
        
        target_vector_t target_vector;
        typename source_vector_t::iterator first = source_vector.begin();
        std::back_insert_iterator<target_vector_t> dest(target_vector);
        int state = 0;
        result = converter_t().convert(first, source_vector.end(), dest, state);
        
        EXPECT_EQ(0, state);
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert(unicode::encoding_traits<to_encoding_t>::name() /*to*/,
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()/sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToUtfConversionTest, WellformedUtfToUtfUnsafe) 
    {
        // This test requires that the previous test was successfully.
        
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        typedef typename TypeParam::from_encoding_t     from_encoding_t;
        typedef typename TypeParam::to_encoding_t       to_encoding_t;
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, from_encoding_t>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, to_encoding_t>::value) );
        
        
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type> target_vector_t;
        
        typedef converter<code_point_t, from_encoding_t, unicode::Validation::SAFE> tmp_converter_t;
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::UNSAFE> converter_t;
        
        
        // create a temp vector of valid code points, then convert them into the
        // required source encoding.
        std::vector<code_point_t> code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        typename std::vector<code_point_t>::iterator cp_first = code_points.begin();
        source_vector_t source_vector;
        std::back_insert_iterator<source_vector_t> source_dest(source_vector);
        int result = tmp_converter_t().convert(cp_first, code_points.end(), source_dest);
        assert(result == 0);
        
        
        target_vector_t target_vector;
        typename source_vector_t::iterator first = source_vector.begin();
        std::back_insert_iterator<target_vector_t> dest(target_vector);
        int state = 0;
        result = converter_t().convert(first, source_vector.end(), dest, state);
        
        EXPECT_EQ(0, state);
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()/sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
#endif 
    
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(UtfToUtfConversionTest, WellformedUtfToUtfSafe/*, WellformedUtfToUtfUnsafeNoCheckInputRange, WellformedUtfToUtfUnsafeNoValidation, WellformedUtfToUtfUnsafe*/);
    
    
    // Instantiate test cases:
    typedef ::testing::Types<
    conversion<UTF_8_encoding_tag, UTF_8_encoding_tag>,
    //    conversion<UTF_8_encoding_tag, UTF_16_encoding_tag>,
    //    conversion<UTF_8_encoding_tag, UTF_32_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_8_encoding_tag, UTF_32BE_encoding_tag>,    
    
    //    conversion<UTF_16_encoding_tag, UTF_8_encoding_tag>,
    //    conversion<UTF_16_encoding_tag, UTF_16_encoding_tag>,
    //    conversion<UTF_16_encoding_tag, UTF_32_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_8_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_16LE_encoding_tag, UTF_32BE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_8_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_16BE_encoding_tag, UTF_32BE_encoding_tag>,
    
    //    conversion<UTF_32_encoding_tag, UTF_8_encoding_tag>,
    //    conversion<UTF_32_encoding_tag, UTF_16_encoding_tag>,
    //    conversion<UTF_32_encoding_tag, UTF_32_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_8_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_32LE_encoding_tag, UTF_32BE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_8_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_16LE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_16BE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_32LE_encoding_tag>,
    conversion<UTF_32BE_encoding_tag, UTF_32BE_encoding_tag>
    >  UTF_conversions;
    
    
    INSTANTIATE_TYPED_TEST_CASE_P(UTFtoUTFTests, UtfToUtfConversionTest, UTF_conversions);
    
}

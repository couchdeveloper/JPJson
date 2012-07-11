//
//  unicode_converter_test.cpp
//  Test
//
//  Created by Andreas Grosam on 8/13/11.
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

#include "json/unicode/unicode_converter.hpp"
#include "json/unicode/unicode_traits.hpp"
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
#include <vector>
#include <iomanip>

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/array.hpp>


using namespace json;

//
//
//
namespace json { namespace unicode { namespace test {
    
    enum utf8_state {
        kUTF8_Wellformed = 0,
        kUTF8_UnexpectedEnd = 1,
        kUTF8_InvalidFirstTrailByte = 2,
        kUTF8_InvalidStartByte = 3,
        kUTF8_TrailByteExpected = 4
    };
        
    
    // Tests for a well-formed UTF-8 sequence according Unicode Standard 6.0.0
    
    // Note: The implementation does deliberately not use any of the existing
    // unicode functions in namespace json::unicode.

    // If the code sequence is ill-formed utf8_result_type.first contains 
    // the kind of error, and utf8_result_type.second points to the end of
    // the "maximal subpart of an ill-formed subsequence".
    
    template <typename InIterator>
    inline int 
    utf8_test_wellformed(InIterator& first, InIterator last) 
    {                    
        if (first == last) {
            return kUTF8_UnexpectedEnd;
        }

        if (*first <= 0x7F) {
            ++first;
            return kUTF8_Wellformed;
        }
        else if (*first >= 0xC2 and *first <= 0xDF) {
            ++first;
            if (first == last) {
                return kUTF8_UnexpectedEnd;
            }            
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
                return kUTF8_Wellformed;
            }
            else {
                // trail byte expected
                return kUTF8_TrailByteExpected;
            }
        }
        else if (*first == 0xE0) {
            ++first;
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0xA0 and *first <= 0xBF) {
                ++first;
            }
            else {
                if (*first >= 0x80 and *first <= 0xBF) {
                    return kUTF8_InvalidFirstTrailByte;
                }
                else {                    
                    return kUTF8_TrailByteExpected;
                }
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
                return kUTF8_Wellformed;
            }
            else {
                // trail byte expected
                return kUTF8_TrailByteExpected;
            }
        }
        else if ( (*first >= 0xE1 and *first <= 0xEC) or (*first >= 0xEE and *first <= 0xEF)) {
            ++first;
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
                return kUTF8_Wellformed;
            }
            else {
                return kUTF8_TrailByteExpected;
            }            
        }
        else if (*first == 0xED) {
            ++first;
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0x9F) {
                ++first;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
                return kUTF8_Wellformed;
            }
            else {
                if (*first >= 0x80 and *first <= 0xBF) {
                    return kUTF8_InvalidFirstTrailByte;
                }
                else {                    
                    return kUTF8_TrailByteExpected;
                }
            }
        }
        else if (*first == 0xF0) {
            ++first;
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x90 and *first <= 0xBF) {
                ++first;
            }
            else {
                if (*first >= 0x80 and *first <= 0xBF) {
                    return kUTF8_InvalidFirstTrailByte;
                }
                else {                    
                    return kUTF8_TrailByteExpected;
                }
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
                return kUTF8_Wellformed;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
        }
        else if (*first >= 0xF1 and *first <= 0xF3) {
            ++first;
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
                return kUTF8_Wellformed;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
        }
        else if (*first == 0xF4) {
            ++first;
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0x8F) {
                ++first;
            }
            else {
                if (*first >= 0x80 and *first <= 0xBF) {
                    return kUTF8_InvalidFirstTrailByte;
                }
                else {                    
                    return kUTF8_TrailByteExpected;
                }
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
            if (first == last)
                return kUTF8_UnexpectedEnd;
            if (*first >= 0x80 and *first <= 0xBF) {
                ++first;
                return kUTF8_Wellformed;
            }
            else {
                return kUTF8_TrailByteExpected;
            }
        }
        else {
            return kUTF8_InvalidStartByte;
        }        
    }
    
    
    
    class utf8_generator 
    {
    public:
        typedef boost::array<uint8_t, 4> buffer_type;
        typedef buffer_type::iterator iterator;
        
        utf8_generator() 
        :   begin_(buffer_.begin()), end_(buffer_.begin()),
            first_(0), last_(0xFFFFFFFF), current_(0)
        {
            to_buffer(first_);
        }
        
        utf8_generator(buffer_type first, buffer_type last) 
        : begin_(buffer_.begin()), end_(buffer_.begin())
        {
            first_ = from_buffer(first);
            last_ = from_buffer(last);
            current_ = first_;
            to_buffer(first_);
        }
        
        operator bool() const {
            return current_ != last_;
        }
        
        void next() {
            if (current_ != last_)
                to_buffer(current_++);
        }
        
        iterator begin() { return begin_; }
        iterator end() { return end_; }
    
    private:
        void to_buffer(uint32_t v)
        {
            buffer_[0] = uint8_t(v & 0x000000FF);
            int i = 1;
            for (int k = 1; k < 4; ++k) {
                if ( (v = v >> 8) ) {
                    buffer_[i] = uint8_t(v & 0x000000FF);
                    ++i;
                }
                else {
                    break;
                }
            }
            end_ = begin_ + i;    
        }

        uint32_t from_buffer(buffer_type buf)
        {
            uint32_t result = buf[0];
            result += uint32_t(buf[1]) << 8;
            result += uint32_t(buf[2]) << 16;
            result += uint32_t(buf[3]) << 24;
            return result;
        }
        
        void print(std::ostream& os) const {
            size_t i = std::distance(begin_, end_);
            iterator first = begin_;
            while (i--) {
                os << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*first) << " " << std::flush;
                ++first;
            }
        }
        
    private:
        friend inline
        std::ostream& operator<<(std::ostream& os, const utf8_generator& gen) {
            gen.print(os);
            return os;
        }
        
    private:
        
        iterator begin_;
        iterator end_;
        buffer_type buffer_;
        uint32_t first_;
        uint32_t last_;
        uint32_t current_;
        
    };
    
}}}  // namespace json::unicode::test


//
//  Unicode Code Point to UTF
//
namespace {
    
    
    using namespace json;
    
    
    template <typename T>
    class CodepointToUtfConverterTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        CodepointToUtfConverterTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~CodepointToUtfConverterTest() {
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
    
    
    TYPED_TEST_CASE_P(CodepointToUtfConverterTest);
    
    
    TYPED_TEST_P(CodepointToUtfConverterTest, ValidCodePpointsToUtfConvertSafe) 
    {
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef code_point_t from_encoding_t;
        typedef TypeParam to_encoding_t;
        typedef std::vector<code_point_t> input_buffer_t;
        
        typedef converter<
            from_encoding_t, to_encoding_t, unicode::Validation::SAFE> converter_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type > target_vector_t;

        
        // create a vector of valid code points, then convert them in one go.
        input_buffer_t code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        
        target_vector_t target_vector;
        typename input_buffer_t::iterator first = code_points.begin();
        std::back_insert_iterator<target_vector_t> dest(target_vector);
        int result = converter_t().convert(first, code_points.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef typename encoding_traits<host_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(code_points.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(code_points[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((code_points.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
            ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                            unicode::encoding_traits<host_encoding_t>::name() /*from*/, 
                            target, target_size, 
                            source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        bool equalSize = icu_output_bytes == target_vector.size()*sizeof(output_char_t);
        bool equal = false;
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (equalSize) {
            equal = (0 == memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes));
            EXPECT_TRUE(equal);
        }
        
        
        if (not equal) {
            typename target_vector_t::iterator f0 = target_vector.begin();
            output_char_t* f1 = static_cast<output_char_t*>(static_cast<void*>(target));
            output_char_t* f1_end = f1 + icu_output_bytes/sizeof(output_char_t);
            
            while (f0 != target_vector.end() and f1 != f1_end and *f0++ == *f1++) 
            {}
            
            std::cout << "Difference at index: " << std::distance(target_vector.begin(), f0) << 
            ", with expected code unit: " << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(output_char_t)*2) << *f1 << 
            ", and actual code unit: " << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(output_char_t)*2) << *f0 << std::endl;
        }

        
        free(target);
    }
            
    
    TYPED_TEST_P(CodepointToUtfConverterTest, ValidCodepointsToUtfConvertUnsafeNoCheckInputRange) 
    {
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef code_point_t from_encoding_t;
        typedef TypeParam to_encoding_t;
        typedef std::vector<code_point_t> input_buffer_t;
        
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::NO_CHECK_INPUT_RANGE> converter_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type > target_vector;
        
        
        // create a vector of valid code points, than convert them in one go.
        input_buffer_t code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        
        target_vector target_buffer;
        typename input_buffer_t::iterator first = code_points.begin();
        std::back_insert_iterator<target_vector> dest(target_buffer);
        int result = converter_t().convert(first, code_points.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef typename encoding_traits<host_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(code_points.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(code_points[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((code_points.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<host_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_buffer.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_buffer.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_buffer[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    

    TYPED_TEST_P(CodepointToUtfConverterTest, ValidCodepointsToUtfConvertUnsafeNoValidation) 
    {
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef code_point_t from_encoding_t;
        typedef TypeParam to_encoding_t;
        typedef std::vector<code_point_t> input_buffer_t;
        
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::NO_VALIDATION> converter_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type > target_vector;
        
        
        // create a vector of valid code points, than convert them in one go.
        input_buffer_t code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        
        target_vector target_buffer;
        typename input_buffer_t::iterator first = code_points.begin();
        std::back_insert_iterator<target_vector> dest(target_buffer);
        int result = converter_t().convert(first, code_points.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef typename encoding_traits<host_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(code_points.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(code_points[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((code_points.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<host_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_buffer.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_buffer.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_buffer[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    
    TYPED_TEST_P(CodepointToUtfConverterTest, ValidCodepointsToUtfConvertUnsafe) 
    {
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef code_point_t from_encoding_t;
        typedef TypeParam to_encoding_t;
        typedef std::vector<code_point_t> input_buffer_t;
        
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::UNSAFE> converter_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type > target_vector;
        
        
        // create a vector of valid code points, than convert them in one go.
        input_buffer_t code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        
        target_vector target_buffer;
        typename input_buffer_t::iterator first = code_points.begin();
        std::back_insert_iterator<target_vector> dest(target_buffer);
        int result = converter_t().convert(first, code_points.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef typename encoding_traits<host_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(code_points.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(code_points[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((code_points.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<host_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_buffer.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_buffer.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_buffer[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    
    
    
    TYPED_TEST_P(CodepointToUtfConverterTest, InvalidCodepointsToUtfConvertSafe) 
    {
        //  Allow input which is not strictly valid. In this test,
        //  we generate only invalid Unicode code point from range
        //  [0 .. 11000]
        //
        
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef code_point_t from_encoding_t;
        typedef TypeParam to_encoding_t;
        typedef std::vector<code_point_t> input_buffer_t;
        
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::SAFE> converter_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type > target_vector;
        
        
        // create a vector of invalid Unicode code points, than convert them 
        // catching any error and replacing it with the Unicode replacement 
        // character.
        input_buffer_t code_points;
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax + 1; ++cp) {
            if (not unicode::isUnicodeScalarValue(cp))
            code_points.push_back(cp);
        }
        
        std::cout << "number of invalid Unicode code points: " << code_points.size() << std::endl;
        
        target_vector target_buffer;
        typename input_buffer_t::iterator first = code_points.begin();
        std::back_insert_iterator<target_vector> dest(target_buffer);
        int result = 0;
        while (first != code_points.end() and result == 0) {
            result = converter_t().convert(first, code_points.end(), dest);
            if (result != 0) {
                // check error, and replace it with Unicode replacment character.
                EXPECT_EQ(unicode::E_INVALID_CODE_POINT, result);
                code_point_t tmp[1] = {unicode::kReplacementCharacter};
                code_point_t* p = tmp;
                result = converter_t().convert(p, p+1, dest);
                assert(result == 0);
                ++first;  // first still points to the invalid code point. Increment it now.
            }
        }
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef typename encoding_traits<host_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        int32_t source_length = (int)(code_points.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(code_points[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((code_points.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<host_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_buffer.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_buffer.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_buffer[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(CodepointToUtfConverterTest, ValidCodePpointsToUtfConvertSafe, ValidCodepointsToUtfConvertUnsafeNoCheckInputRange, ValidCodepointsToUtfConvertUnsafeNoValidation, ValidCodepointsToUtfConvertUnsafe, InvalidCodepointsToUtfConvertSafe);
    
    // Instantiate test cases:
    typedef ::testing::Types<
        unicode::UTF_8_encoding_tag, 
        unicode::UTF_16BE_encoding_tag,
        unicode::UTF_16LE_encoding_tag,
        unicode::UTF_32BE_encoding_tag,
        unicode::UTF_32LE_encoding_tag
    >  UTF_encodings;
    
    
    INSTANTIATE_TYPED_TEST_CASE_P(CodepointToUTFTests, CodepointToUtfConverterTest, UTF_encodings);
    
}

//
//  UTF to Unicode Code Point
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !! The following tests require that the previous test "Unicode code point to UTF-x"  !!
// !! passed,  otherwise the result is meaningless.                                     !!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
namespace {
    
    
    using namespace json;
    
    
    template <typename T>
    class UtfToCodepointConversionsTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        UtfToCodepointConversionsTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~UtfToCodepointConversionsTest() {
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
    
    
    TYPED_TEST_CASE_P(UtfToCodepointConversionsTest);
    
    
    TYPED_TEST_P(UtfToCodepointConversionsTest, WellformedUtfToCodepointConvertSafe) 
    {
        // This test requires that the previous test was successfully.
        
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef TypeParam from_encoding_t;
        typedef code_point_t to_encoding_t;
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<code_point_t> target_vector_t;
        
        typedef converter<code_point_t, from_encoding_t, unicode::Validation::SAFE> tmp_converter_t;
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::SAFE> converter_t;
        
        
        // Create a temp vector of valid code points, then convert them into the
        // required source encoding.
        // !! Note: this requires that the previous test "Unicode code point to UTF-x" passed,  !!
        // !! otherwise the result is meaningless.                                               !!
        // Create a vector of all Unicode code points which are valid Unicode scalar values:
        std::vector<code_point_t> code_points;
        code_points.push_back(0x02);
        for (code_point_t cp = 0; cp <= unicode::kUnicodeCodeSpaceMax; ++cp) {
            if (unicode::isUnicodeScalarValue(cp)) {
                code_points.push_back(cp);
            }
        }
        // Now, create our source from the code points in the required encoding:
        typename std::vector<code_point_t>::iterator cp_first = code_points.begin();
        source_vector_t source_vector;
        std::back_insert_iterator<source_vector_t> source_dest(source_vector);
        int result = tmp_converter_t().convert(cp_first, code_points.end(), source_dest);
        assert(result == 0);
        
        // Test conversion from UTF-x to Unicode code points:
        target_vector_t target_vector;
        typename source_vector_t::iterator first = source_vector.begin();
        std::back_insert_iterator<target_vector_t> dest(target_vector);
        result = converter_t().convert(first, source_vector.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        // The generated vector of code points shall be equal the temporary code point vector:
        bool equalVectors = code_points == target_vector;
        EXPECT_TRUE(equalVectors);
        
        if (not equalVectors) {
            bool equalSize = code_points.size() == target_vector.size();
            EXPECT_TRUE(equalSize);
            
            
            typename std::vector<code_point_t>::iterator f0 = code_points.begin();
            target_vector_t::iterator f1 = target_vector.begin();
            
            while (f0 != code_points.end() and f1 != target_vector.end() and *f0++ == *f1++) {
            }
            
            std::cout << "Difference at index: " << std::distance(code_points.begin(), f0) << 
            ", with source code point: " << "0x" << std::hex << *f0 << ", and generated code point: " << "0x" << std::hex << *f1 << std::endl;
            
        }
        
        
        // In order to perform a thorough test, compare the converted bytes with ICU:
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef code_point_t output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<host_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToCodepointConversionsTest, WellformedUtfToCodepointConvertUnsafeNoCheckInputRange) 
    {
        // This test requires that the previous test was successfully.
        
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef TypeParam from_encoding_t;
        typedef code_point_t to_encoding_t;
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<code_point_t> target_vector_t;
        
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
        result = converter_t().convert(first, source_vector.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef code_point_t output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<host_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToCodepointConversionsTest, WellformedUtfToCodepointConvertUnsafeNoValidation) 
    {
        // This test requires that the previous test was successfully.
        
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef TypeParam from_encoding_t;
        typedef code_point_t to_encoding_t;
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<code_point_t> target_vector_t;
        
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
        result = converter_t().convert(first, source_vector.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef code_point_t output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<host_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToCodepointConversionsTest, WellformedUtfToCodepointConvertUnsafe) 
    {
        // This test requires that the previous test was successfully.
        
        using unicode::code_point_t;
        using unicode::converter;
        using unicode::isUnicodeScalarValue;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using unicode::UTF_32_encoding_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        
        typedef TypeParam from_encoding_t;
        typedef code_point_t to_encoding_t;
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<code_point_t> target_vector_t;
        
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
        result = converter_t().convert(first, source_vector.end(), dest);
        
        EXPECT_EQ(0, result);  
        
        
        // Compare the converted bytes with ICU:
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename add_endianness<UTF_32_encoding_tag>::type  host_encoding_t;        // Unicode code point encoding
        typedef code_point_t output_char_t;
        
        int32_t source_length = (int)(source_vector.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_vector[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((source_vector.size()+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        UErrorCode error = U_ZERO_ERROR;
        int32_t icu_output_bytes = 
        ucnv_convert_48(unicode::encoding_traits<host_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(UtfToCodepointConversionsTest, 
                               WellformedUtfToCodepointConvertSafe, WellformedUtfToCodepointConvertUnsafeNoCheckInputRange, WellformedUtfToCodepointConvertUnsafeNoValidation, WellformedUtfToCodepointConvertUnsafe);
    
    
    
    
    
    // Instantiate test cases:
    typedef ::testing::Types<
    unicode::UTF_8_encoding_tag, 
    unicode::UTF_16BE_encoding_tag,
    unicode::UTF_16LE_encoding_tag,
    unicode::UTF_32BE_encoding_tag,
    unicode::UTF_32LE_encoding_tag
    >  UTF_encodings;
    
    
    INSTANTIATE_TYPED_TEST_CASE_P(UTFtoCodepointTests, UtfToCodepointConversionsTest, UTF_encodings);
    

}

//
//  UTF to UTF
//
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
    class UtfToUtfConversionsTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        UtfToUtfConversionsTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~UtfToUtfConversionsTest() {
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
    
    
    TYPED_TEST_CASE_P(UtfToUtfConversionsTest);
    
    
    TYPED_TEST_P(UtfToUtfConversionsTest, WellformedUtfToUtfSafe) 
    {
        // This test requires that the previous test was successful.
        
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        typedef typename TypeParam::from_encoding_t     from_encoding_t;
        typedef typename TypeParam::to_encoding_t       to_encoding_t;
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, from_encoding_t>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, to_encoding_t>::value) );
        
        
        typedef std::vector<typename encoding_traits<from_encoding_t>::code_unit_type > source_vector_t;
        typedef std::vector<typename encoding_traits<to_encoding_t>::code_unit_type> target_vector_t;
        
        typedef converter<code_point_t, from_encoding_t, unicode::Validation::SAFE> tmp_converter_t;
        typedef converter<from_encoding_t, to_encoding_t, unicode::Validation::SAFE> converter_t;
        
        
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
        
        //
        //   Test  Convert UTF to UTF
        //
        target_vector_t target_vector;
        typename source_vector_t::iterator first = source_vector.begin();
        std::back_insert_iterator<target_vector_t> dest(target_vector);
        result = converter_t().convert(first, source_vector.end(), dest);
        
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
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        bool equalSize = icu_output_bytes == target_vector.size()*sizeof(output_char_t);
        bool equal = false;
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (equalSize) {
            equal = (0 == memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes));
            EXPECT_TRUE(equal);
        }
        if (not equal) {
            typename target_vector_t::iterator f0 = target_vector.begin();
            output_char_t* f1 = static_cast<output_char_t*>(static_cast<void*>(target));
            output_char_t* f1_end = f1 + icu_output_bytes/sizeof(output_char_t);
            
            while (f0 != target_vector.end() and f1 != f1_end and *f0++ == *f1++) 
            {}
            
            std::cout << "Difference at index: " << std::distance(target_vector.begin(), f0) << 
            ", with expected code unit: " << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(output_char_t)*2) << *f1 << 
            ", and actual code unit: " << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(output_char_t)*2) << *f0 << std::endl;
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToUtfConversionsTest, WellformedUtfToUtfUnsafeNoCheckInputRange) 
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
        result = converter_t().convert(first, source_vector.end(), dest);
        
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
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToUtfConversionsTest, WellformedUtfToUtfUnsafeNoValidation) 
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
        result = converter_t().convert(first, source_vector.end(), dest);
        
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
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    TYPED_TEST_P(UtfToUtfConversionsTest, WellformedUtfToUtfUnsafe) 
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
        result = converter_t().convert(first, source_vector.end(), dest);
        
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
        ucnv_convert_48(unicode::encoding_traits<to_encoding_t>::name() /*to*/, 
                        unicode::encoding_traits<from_encoding_t>::name() /*from*/, 
                        target, target_size, 
                        source,  source_length, &error); 
        assert(error == U_ZERO_ERROR);
        
        EXPECT_EQ(icu_output_bytes, target_vector.size()*sizeof(output_char_t));
        
        if (icu_output_bytes == target_vector.size()*sizeof(output_char_t)) {
            int cmp_result = memcmp(target, static_cast<const void*>(&target_vector[0]), icu_output_bytes);
            EXPECT_EQ(0, cmp_result);
        }
        
        free(target);
    }
    
    
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(UtfToUtfConversionsTest, WellformedUtfToUtfSafe, WellformedUtfToUtfUnsafeNoCheckInputRange, WellformedUtfToUtfUnsafeNoValidation, WellformedUtfToUtfUnsafe);
    
    
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
    
    
    INSTANTIATE_TYPED_TEST_CASE_P(UTFtoUTFTests, UtfToUtfConversionsTest, UTF_conversions);
    
}

//
// UTF-8 Wellformdness Test
// Brute-Force test using every possibly UTF-8 sequence up to four bytes,
// and a few selected malformed samples for testing wellformdness.
//
namespace {
#pragma mark - UTF-8 Wellformdness
    
    
    using namespace json;
    
    using unicode::test::utf8_generator;
    using unicode::test::utf8_test_wellformed;
    
    using unicode::UTF_8_encoding_tag;
    using unicode::code_point_t;
    
    using unicode::converter;
    
    class Utf8WellformedTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        Utf8WellformedTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~Utf8WellformedTest() {
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
    
    
    // C10 
    // When a process interprets a code unit sequence which purports to be in a 
    // Unicode character encoding scheme, it shall treat ill-formed code unit se-
    // quences as an error condition and shall not interpret such sequences as 
    // characters.
    
    // When faced with ill-formed code unit sequences while transforming or 
    // interpreting text, a conformant process must treat the first code unit 
    // of an ill-formed sequence as an illegally terminated code unit sequence,
    // for example, by 
    //  - signaling an error, 
    //  - filtering the code unit out, or 
    //  - representing the code unit with a marker such as U+FFFD replacement 
    //    character.
    //
    // A conversion process is required not to consume any well-formed subse-
    // quence as part of its error handling for ill-formed subsequences.
    // 
    // An ill-formed subsequence consisting of more than one code unit may be
    // signaled as one error, or as multiple errors.
    
    
#if !defined (DEBUG)
    TEST_F(Utf8WellformedTest, UTF8_Wellformdness)     
#else
    TEST_F(Utf8WellformedTest, DISABLED_UTF8_Wellformdness)     
#endif
    {
        // We are using a "safe" version of a UTF-to-code_point converter which
        // consumes only one Unicode character.
        // This converter does not require to use state information. But if so,
        // we need to reset state after parsing an ill-formed Unicode sequence.
        typedef converter<UTF_8_encoding_tag, code_point_t, 
            unicode::Validation::SAFE, unicode::Stateful::No, unicode::ParseOne::Yes> 
        converter_t;

        
        typedef utf8_generator::buffer_type buffer_t;
        typedef utf8_generator generator_t;
        
//        generator_t::buffer_type from_utf = {0xf5, 0,0,0};
//        generator_t::buffer_type to_utf = {0xff, 0,0,0};
        generator_t generator;
        
        std::size_t count = 0;
        std::size_t fail_count = 0;
        const uint32_t marker = std::numeric_limits<uint32_t>::max() / 100;

        
        while (generator) 
        {
            code_point_t code_point_buffer[4];
            code_point_t* dest = code_point_buffer;
            
            ++count;
            if ((count % marker) == 0) {
                printf(".");
                fflush(stdout);
            }
            generator_t::iterator first = generator.begin();
            int result = converter_t().convert(first, generator.end(), dest);
            int consumed = (int)std::distance(generator.begin(), first);
            
            generator_t::iterator ref_first = generator.begin();            
            int reference_result = 0;
#if 1 // We only consume one Unicode character           
            reference_result = utf8_test_wellformed(ref_first, generator.end());
#else
            while (ref_first != generator.end() and reference_result == 0) {
                reference_result = utf8_test_wellformed(ref_first, generator.end());
            }
#endif            
            size_t reference_consumed = std::distance(generator.begin(), ref_first);
            if (reference_result == unicode::NO_ERROR) {
                // Conversion shall be successfull:
                if (reference_consumed != consumed or result != unicode::NO_ERROR) {
                    ++fail_count;
                    EXPECT_EQ(unicode::NO_ERROR, result);
                    EXPECT_EQ(reference_consumed, consumed); 
                    std::cout << generator << std::endl;
                    ASSERT_LT(fail_count, 100) << "*** too many failures ***\n";
                }
            }
            else {
                // Conversion shall fail:
                if (reference_consumed != consumed or result == unicode::NO_ERROR) {
                    ++fail_count;
                    EXPECT_TRUE(unicode::NO_ERROR != result);
                    EXPECT_TRUE(result < 0);
                    EXPECT_EQ(reference_consumed, consumed); 
                    std::cout << generator << std::endl;
                    ASSERT_LT(fail_count, 100) << "*** too many failures ***\n";
                }
            }
            
            generator.next();
        }  // while
        
    }
    
    
#pragma mark - Malformed UTF to Unicode Code Point
    struct test_item_t {
        uint8_t buffer[6];
        int result;
        int consumed;
        const char* desc;
    };
    
    TEST_F(Utf8WellformedTest, Malformed_UTF8)     
    {   
        // We are using a "safe" version of a UTF-to-code_point converter.
        // This converter uses state information, so we need to reset state
        // after parsing an ill-formed Unicode sequence.
        typedef converter<UTF_8_encoding_tag, code_point_t, unicode::Validation::SAFE> converter_t;
        
        
        test_item_t const test_items[] = {
            {{0x80, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 0
            {{0x90, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 1
            {{0xA0, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 2
            {{0xB0, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 3
            {{0xBF, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 4
            {{0xC0, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 5
            {{0xC1, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 6
            {{0xF5, 0x30, 0x31, 0x33, 0},       -1, 0, "invalid start byte" },  // 7
            {{0xF6, 0x80, 0x30, 0x31, 0},       -1, 0, "invalid start byte" },  // 8
            {{0xF7, 0x80, 0x80, 0x30, 0x31},    -1, 0, "invalid start byte" },  // 9
            {{0xF8, 0x80, 0x80, 0x30, 0x31},    -1, 0, "invalid start byte" },  // 10
            {{0xFF, 0x80, 0x80, 0x30, 0x31},    -1, 0, "invalid start byte" },  // 11
            
            // Lead(2)
            {{0xE1, 0x30, 0x31},                -1, 1, "expected trail at index 1."}, // 12  // lead(2)
            {{0xE1, 0x80, 0x30, 0x31},          -1, 2, "expected trail at index 2."}, // 13  // lead(2)
            
            // Lead(3)
            {{0xF1, 0x30, 0x31, 0x33, 0},       -1, 1, "expected trail at index 1." }, // 14 // lead(3)
            {{0xF1, 0x80, 0x30, 0x31, 0},       -1, 2, "expected trail at index 2." }, // 15 // lead(3)
            {{0xF1, 0x80, 0x80, 0x30, 0x31, 0}, -1, 3, "expected trail at index 3." }, // 16 // lead(3)
            
            // Unconvertable offsets:
            {{0xE0, 0x9F, 0x80, 0x30, 0x31},    -1, 1, "unconvertable offset at index 1."}, // 17 // lead(2)
            {{0xED, 0xA0, 0x80, 0x30, 0x31},    -1, 1, "unconvertable offset at index 1."}, // 18 // lead(2)
            {{0xF0, 0x8F, 0x80, 0x80, 0x30},    -1, 1, "unconvertable offset at index 1" }, // 19 // lead(3)
            {{0xF4, 0x90, 0x80, 0x80, 0x30},    -1, 1, "unconvertable offset at index 1" }, // 20 // lead(3)
            
            
            {{0xC2, 0x30, 0x31, 0x32, 0x33},    -1, 1, "expected trail at index 1."}, // 21 // lead(1)
            {{0xDF, 0x30, 0x31, 0x32, 0x33},    -1, 1, "expected trail at index 1."}, // 22 // lead(1)
            {{0xEC, 0x30, 0x31},                -1, 1, "expected trail at index 1."}, // 24
            {{0xEE, 0x30, 0x31},                -1, 1, "expected trail at index 1."}, // 25
            {{0xEF, 0x30, 0x31},                -1, 1, "expected trail at index 1."}, // 26
            {{0xF1, 0x30, 0x31},                -1, 1, "expected trail at index 1."}, // 27
            {{0xF3, 0x30, 0x31},                -1, 1, "expected trail at index 1."}, // 28
        };
        
        /*        
         uint8_t invalidNumTrail3[] = {0xB0, 0x30, 0x31};  // Invalid number of trails at index 3.
         uint8_t inValidOffset1[] = {0xE0, 0x80, 0x80};  // Invalid offset at index 1
         uint8_t inValidOffset2[] = {0xE0, 0x9F, 0x80};  // Invalid offset at index 1
         uint8_t inValidOffset3[] = {0xED, 0xA0, 0x80};  // Invalid offset at index 1
         uint8_t inValidOffset4[] = {0xED, 0xBF, 0x80};  // Invalid offset at index 1
         uint8_t inValidOffset5[] = {0xF0, 0x80, 0x80, 0x80};  // Invalid offset
         uint8_t inValidOffset6[] = {0xF0, 0x8F, 0x80, 0x80};  // Invalid offset
         uint8_t inValidOffset7[] = {0xF4, 0x90, 0x80, 0x80};  // Invalid offset
         uint8_t invalidCodePoint[] = "";
         
         */
        
        const test_item_t* first = test_items;
        const test_item_t* last = test_items + sizeof(test_items)/sizeof(test_item_t);
        
        while (first != last) 
        {
            code_point_t code_point_buffer[16];
            code_point_t* dest = code_point_buffer;
            const uint8_t* start = (*first).buffer;
            const uint8_t* end = start + sizeof((*first).buffer);
            int result = converter_t().convert(start, end, dest);
            int consumed = (int)std::distance((*first).buffer, start);
            if (result >= 0 or consumed != (*first).consumed) {
                std::cout << std::hex;
                std::copy((*first).buffer, end, std::ostream_iterator<int>(std::cout, " "));
                std::cout << std::endl;          
            }
            EXPECT_TRUE(result != unicode::NO_ERROR) << "at index " << std::distance(&test_items[0], first);
            EXPECT_TRUE(result < 0) << "at index " << std::distance(&test_items[0], first);
            //EXPECT_EQ((*first).result, result) << "at index " << std::distance(&test_items[0], first);
            EXPECT_EQ((*first).consumed, consumed) << "at index " << std::distance(&test_items[0], first);
            
            ++first;
        }    
        
        
    }
}

#if 0
namespace {    
    
#if 0    
#pragma mark - PrintInvalidUnicodeAsUTF8
    
    TEST_F(code_point_conversions_test, PrintInvalidUnicodeAsUTF8) {
        
        uint8_t buffer[8];
        uint8_t* first = buffer;
        std::ostream_iterator<int> out_it (std::cout, " ");
        std::cout.setf(std::ios::hex, std::ios::basefield);
        //std::cout.setf(std::ios::showbase);
        
        for (code_point_t cp = 0; cp <= 0x10FFFF; ++cp) {
            if (isHighSurrogate(cp)) {
                first = buffer;
                int result = json::unicode::convert_one_unsafe(cp, first, UTF_8_encoding_tag());
                assert(result > 0);
                std::cout << "HS: ";
                std::copy(buffer, buffer + result, out_it);
                std::cout << std::endl;
            }
            else if (isLowSurrogate(cp)) {
                first = buffer;
                int result = convert_one_unsafe(cp, first, UTF_8_encoding_tag());
                assert(result > 0);
                std::cout << "LS: ";
                std::copy(buffer, buffer + result, out_it);
                std::cout << std::endl;                
            }
            else if (isNonCharacter(cp)) {
                first = buffer;
                int result = convert_one_unsafe(cp, first, UTF_8_encoding_tag());
                assert(result > 0);
                std::cout << "NC: ";
                std::copy(buffer, buffer + result, out_it);
                std::cout << std::endl;
            }
        }
        
    }
#endif    
    
    
#pragma mark - Unicode Code Point  to UTF-8
    TEST_F(code_point_conversions_test, UnicodeCodePointToUTF8)     
    {
        uint8_t reference_buffer[8];
        int reference_length;
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
                
        uint8_t buffer1[8];
        uint8_t* dest1 = buffer1;
        uint8_t buffer2[8];
        uint8_t* dest2 = buffer2;
        uint8_t buffer3[8];
        uint8_t* dest3 = buffer3;
        uint8_t buffer4[8];
        uint8_t* dest4 = buffer4;
        
        for (code_point_t cp = 0; cp <= (0x10FFFF +10); ++cp) 
        {
            // Get the reference UTF-8
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert("UTF-8" /*to*/, UTF32_Host_Encoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    reinterpret_cast<const char*>(&cp),  sizeof(cp), &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes;
            
            
            dest1 = buffer1;
            dest2 = buffer2;
            dest3 = buffer3;
            dest4 = buffer4;
            int result1 = convert_from_codepoint_unsafe(cp, dest1);
            int result2 = convert_from_codepoint(cp, dest2);
            int result3 = convert_from_codepoint_unsafe(cp, dest3, filter::Noncharacter(0));
            int result4 = convert_from_codepoint(cp, dest4, filter::Noncharacter(0));
            
            if (isSurrogate(cp)) {
                // For surrogates, convert_one_unsafe() is undefined. Actually, 
                // a surrogate will be converted according the encoding
                // algorithm.
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), result1);
                EXPECT_TRUE(result1 > 0)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result1, std::distance(buffer1, dest1));
                
                // convert_one() shall reject Surrogates
                EXPECT_EQ(E_INVALID_CODE_POINT, result2) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer2, dest2));
                
                // For surrogates, convert_one_unsafe() is undefined. Actually, 
                // a surrogate will be converted according the encoding
                // algorithm.
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), result3);
                EXPECT_TRUE(result3 > 0) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result3, std::distance(buffer3, dest3));
                
                // convert_one(, filter) shall reject Surrogates
                EXPECT_EQ(E_INVALID_CODE_POINT, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer4, dest4));
            }
            else if (isNonCharacter(cp)) {
                //convert_one_unsafe() shall convert Noncharacters
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), result1);
                EXPECT_TRUE(result1 > 0) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result1, std::distance(buffer1, dest1));
                EXPECT_TRUE( memcmp(reference_buffer, buffer1, reference_length)== 0 );
                
                //convert_one() shall convert Noncharacters
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), result2);
                EXPECT_TRUE(result2 > 0) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result2, std::distance(buffer2, dest2));
                EXPECT_TRUE( memcmp(reference_buffer, buffer2, reference_length)== 0 );
                
                //convert_one_unsafe() with filter Noncharater shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result3)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer3, dest3));
                
                //convert_one() with filter Noncharater shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer4, dest4));
            }
            else if (!isCodePoint(cp)) {
                //convert_one_unsafe() shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result1) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer1, dest1));
                
                //convert_one() shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result2) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer2, dest2));
                
                //convert_one_unsafe() with filter shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result3) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer3, dest3));
                
                //convert_one_unsafe() with filter shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer4, dest4));
            }   
            else {
                int expected_result = U8_LENGTH(cp);
                EXPECT_EQ(expected_result, result1) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer1, dest1)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer1, reference_length)== 0 );
                
                EXPECT_EQ(expected_result, result2) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer2, dest2)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer2, reference_length)== 0 );
                
                EXPECT_EQ(expected_result, result3) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer3, dest3)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer3, reference_length)== 0 );
                
                EXPECT_EQ(expected_result, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer4, dest4)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer4, reference_length)== 0 );
            }
        }
    }
    
    
#pragma mark - Unicode Code Point  to UTF-16
    TEST_F(code_point_conversions_test, UnicodeCodePointToUTF16)     
    {
        uint16_t reference_buffer[4];
        int reference_length;
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        
        
        uint16_t buffer1[4];
        uint16_t* dest1 = buffer1;
        uint16_t buffer2[4];
        uint16_t* dest2 = buffer2;
        uint16_t buffer3[4];
        uint16_t* dest3 = buffer3;
        uint16_t buffer4[4];
        uint16_t* dest4 = buffer4;
        
        for (code_point_t cp = 0; cp <= (0x10FFFF +10); ++cp) 
        {
            typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
            
            // Get the reference UTF-16
            UErrorCode error = U_ZERO_ERROR;
            int32_t icu_source_bytes = ucnv_convert(UTF16_Host_Encoding().name() /*to*/, UTF32_Host_Encoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(reference_buffer), sizeof(reference_buffer), 
                                                    reinterpret_cast<const char*>(&cp),  sizeof(cp), &error); 
            assert(error == U_ZERO_ERROR);
            reference_length = icu_source_bytes/sizeof(uint16_t);
            
            dest1 = buffer1;
            dest2 = buffer2;
            dest3 = buffer3;
            dest4 = buffer4;
            int result1 = convert_from_codepoint_unsafe(cp, dest1);
            int result2 = convert_from_codepoint(cp, dest2);
            int result3 = convert_from_codepoint_unsafe(cp, dest3, filter::Noncharacter(0));
            int result4 = convert_from_codepoint(cp, dest4, filter::Noncharacter(0));
            
            if (isSurrogate(cp)) {
                // For surrogates, convert_one_unsafe() is undefined. Actually, 
                // a surrogate will be returned as is (possibly swapped).
                EXPECT_EQ(utf16_encoded_length_unsafe(cp), result1);
                EXPECT_TRUE(result1 > 0)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result1, std::distance(buffer1, dest1));
                
                // convert_one() shall reject Surrogates
                EXPECT_EQ(E_INVALID_CODE_POINT, result2) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer2, dest2));
                
                // For surrogates, convert_one_unsafe() is undefined. Actually, 
                // a surrogate will be returned as is (possibly swapped).
                EXPECT_EQ(utf16_encoded_length_unsafe(cp), result3);
                EXPECT_TRUE(result3 > 0) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result3, std::distance(buffer3, dest3));
                
                // convert_one(, filter) shall reject Surrogates
                EXPECT_EQ(E_INVALID_CODE_POINT, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer4, dest4));
            }
            else if (isNonCharacter(cp)) {
                //convert_one_unsafe() shall convert Noncharacters
                EXPECT_EQ(utf16_encoded_length_unsafe(cp), result1);
                EXPECT_TRUE(result1 > 0) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result1, std::distance(buffer1, dest1));
                EXPECT_TRUE( memcmp(reference_buffer, buffer1, reference_length*sizeof(uint16_t))== 0 );
                
                //convert_one() shall convert Noncharacters
                EXPECT_EQ(utf16_encoded_length_unsafe(cp), result2);
                EXPECT_TRUE(result2 > 0) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(result2, std::distance(buffer2, dest2));
                EXPECT_TRUE( memcmp(reference_buffer, buffer2, reference_length*sizeof(uint16_t))== 0 );
                
                //convert_one_unsafe() with filter Noncharater shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result3)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer3, dest3));
                
                //convert_one() with filter Noncharater shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer4, dest4));
            }
            else if (!isCodePoint(cp)) {
                //convert_one_unsafe() shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result1) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer1, dest1));
                
                //convert_one() shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result2) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer2, dest2));
                
                //convert_one_unsafe() with filter shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result3) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer3, dest3));
                
                //convert_one_unsafe() with filter shall reject invalid code points
                EXPECT_EQ(E_INVALID_CODE_POINT, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(0, std::distance(buffer4, dest4));
            }   
            else {
                int expected_result = U16_LENGTH(cp);
                EXPECT_EQ(expected_result, result1) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer1, dest1)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer1, reference_length*sizeof(uint16_t))== 0 );
                
                EXPECT_EQ(expected_result, result2) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer2, dest2)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer2, reference_length*sizeof(uint16_t))== 0 );
                
                EXPECT_EQ(expected_result, result3) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer3, dest3)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer3, reference_length*sizeof(uint16_t))== 0 );
                
                EXPECT_EQ(expected_result, result4) << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(expected_result, std::distance(buffer4, dest4)) << "with code point: U+" << std::hex << cp;
                EXPECT_TRUE( memcmp(reference_buffer, buffer4, reference_length*sizeof(uint16_t))== 0 );
            }
        }
    }
    
    
    
    
#pragma mark - UTF-8 to Unicode Code Point
    TEST_F(code_point_conversions_test, UTF8toUnicodeCodePoint)     
    {
        uint8_t source_buffer[8];
        int source_length;
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        
        
        for (code_point_t cp = 0; cp <= (0x10FFFF); ++cp) 
        {
            // Get the source UTF-8
            if (isSurrogate(cp)) {
                // ICU will not convert code points which are surrogates. This is 
                // correct, since this is malformed Unicode.
                // In order to test, the subsequent *unsafe* function actually
                // will try to convert the single unpaired high or low surrogate 
                // to UTF-8. The result may be undefined, though. If the 
                // source is undefined, the result of the functions under test
                // are of course undefined as well.
                uint8_t* p = source_buffer;
                int result = convert_from_codepoint_unsafe(cp, p);  // convert cp to UTF-8
                source_length = (int)std::distance(source_buffer, p);
                assert(result > 0);
            }
            else {
                UErrorCode error = U_ZERO_ERROR;
                int32_t icu_source_bytes = ucnv_convert("UTF-8" /*to*/, UTF32_Host_Encoding().name() /*from*/, 
                                                    reinterpret_cast<char*>(source_buffer), sizeof(source_buffer), 
                                                    reinterpret_cast<const char*>(&cp),  sizeof(cp), &error);                 
                assert(error == U_ZERO_ERROR);
                source_length = icu_source_bytes;
            }
            
            uint8_t* first1 = source_buffer;
            uint8_t* first2 = source_buffer;
            uint8_t* first3 = source_buffer;
            uint8_t* first4 = source_buffer;
            uint8_t* last = source_buffer + sizeof(source_buffer);
            code_point_t code_point1;
            code_point_t code_point2;
            code_point_t code_point3;
            code_point_t code_point4;
            int result1 = convert_to_codepoint_unsafe(first1, last, code_point1);
            int result2 = convert_to_codepoint(first2, last, code_point2);
            int result3 = convert_to_codepoint_unsafe(first3, last, code_point3, filter::Noncharacter(0));
            int result4 = convert_to_codepoint(first4, last, code_point4, filter::Noncharacter(0));
            
            if (isSurrogate(cp)) 
            {
                // source is undefined, so result is undefined, too.
            }
            else if (isNonCharacter(cp)) {
                //convert_one_unsafe() shall convert Noncharacters
                EXPECT_EQ(1, result1)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), std::distance(source_buffer, first1));
                EXPECT_EQ(cp, code_point1); 
                
                //convert_one() shall convert Noncharacters
                EXPECT_EQ(1, result2)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), std::distance(source_buffer, first2));
                EXPECT_EQ(cp, code_point2); 
                
                //convert_one_unsafe() with filter Noncharacter shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result3)  << "with code point: U+" << std::hex << cp;
                // The number of consumed bytes will not be returned diretcly. It actually equals 
                // the distance between the start of the sequence (source_buffer) and first.
                EXPECT_EQ(source_length, std::distance(source_buffer, first4));
                
                //convert_one() with filter Noncharater shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result4)  << "with code point: U+" << std::hex << cp;
                // The number of consumed bytes will not be returned directly. It actually equals 
                // the distance between the start of the sequence (source_buffer) and first.
                EXPECT_EQ(source_length, std::distance(source_buffer, first4));
            }
            else if (!isCodePoint(cp))
            {
                // ICU nor json::unicode can produce invalid UTF-8 sequences.
            }
            else {
                EXPECT_EQ(1, result1)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), std::distance(source_buffer, first1));
                EXPECT_EQ(cp, code_point1); 

                EXPECT_EQ(1, result2)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), std::distance(source_buffer, first2));
                EXPECT_EQ(cp, code_point2); 

                EXPECT_EQ(1, result3)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), std::distance(source_buffer, first3));
                EXPECT_EQ(cp, code_point3); 

                EXPECT_EQ(1, result4)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(utf8_encoded_length_unsafe(cp), std::distance(source_buffer, first4));
                EXPECT_EQ(cp, code_point4); 
            }
            
        }    
    }
        
        
#pragma mark - UTF-16 to Unicode Code Point
    TEST_F(code_point_conversions_test, UTF16toUnicodeCodePoint)     
    {
        uint16_t source_buffer[8];
        int source_length;
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        
        for (code_point_t cp = 0; cp <= 0x10FFFF; ++cp) 
        {
            // Get the source UTF-8
            if (isSurrogate(cp)) {
                // ICU will not convert code points which are surrogates. This is 
                // correct, since this is malformed Unicode.
                // In order to test, the subsequent *unsafe* function actually
                // will try to convert the single unpaired high or low surrogate 
                // to an UTF-16. The result may be undefined, though. If the 
                // source is undefined, the result of the functions under test
                // is of course undefined as well.
                uint16_t* p = source_buffer;
                int result = convert_from_codepoint_unsafe(cp, p);  // convert cp to UTF-16
                assert(result > 0);
                source_length = (int)std::distance(source_buffer, p);
            } else {            
                UErrorCode error = U_ZERO_ERROR;
                int32_t icu_source_bytes = ucnv_convert(UTF16_Host_Encoding().name() /*to*/, UTF32_Host_Encoding().name() /*from*/, 
                                                        reinterpret_cast<char*>(source_buffer), sizeof(source_buffer), 
                                                        reinterpret_cast<const char*>(&cp),  sizeof(cp), &error); 
                assert(error == U_ZERO_ERROR);
                source_length = icu_source_bytes/sizeof(uint16_t);
            }
            
            uint16_t* first1 = source_buffer;
            uint16_t* first2 = source_buffer;
            uint16_t* first3 = source_buffer;
            uint16_t* first4 = source_buffer;
            uint16_t* last = source_buffer + sizeof(source_buffer)/sizeof(uint16_t);
            code_point_t code_point1;
            code_point_t code_point2;
            code_point_t code_point3;
            code_point_t code_point4;
            int result1 = convert_to_codepoint_unsafe(first1, last, code_point1);
            int result2 = convert_to_codepoint(first2, last, code_point2);
            int result3 = convert_to_codepoint_unsafe(first3, last, code_point3, filter::Noncharacter(0));
            int result4 = convert_to_codepoint(first4, last, code_point4, filter::Noncharacter(0));
            
            if (isSurrogate(cp)) 
            {
                // source is undefined, so result is undefined, too.
            }
            else if (isNonCharacter(cp)) {
                //convert_one_unsafe() shall convert Noncharacters
                EXPECT_EQ(1, result1)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(source_length, std::distance(source_buffer, first1));
                EXPECT_EQ(cp, code_point1); 
                
                //convert_one() shall convert Noncharacters
                EXPECT_EQ(1, result2)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(source_length, std::distance(source_buffer, first2));
                EXPECT_EQ(cp, code_point2); 
                
                //convert_one_unsafe() with filter Noncharater shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result3)  << "with code point: U+" << std::hex << cp;
                // Input shall be consumed:
                EXPECT_EQ(source_length, std::distance(source_buffer, first3));
                
                //convert_one() with filter Noncharater shall reject Noncharacters
                EXPECT_EQ(E_PREDICATE_FAILED, result4)  << "with code point: U+" << std::hex << cp;
                // Input shall be consumed:
                EXPECT_EQ(source_length, std::distance(source_buffer, first4));
            }
            else if (!isCodePoint(cp))
            {
                // ICU nor json::unicode can produce invalid UTF-8 sequences.
            }
            else {
                EXPECT_EQ(1, result1)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(source_length, std::distance(source_buffer, first1));
                EXPECT_EQ(cp, code_point1); 
                
                EXPECT_EQ(1, result2)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(source_length, std::distance(source_buffer, first2));
                EXPECT_EQ(cp, code_point2); 
                
                EXPECT_EQ(1, result3)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(source_length, std::distance(source_buffer, first3));
                EXPECT_EQ(cp, code_point3); 
                
                EXPECT_EQ(1, result4)  << "with code point: U+" << std::hex << cp;
                EXPECT_EQ(source_length, std::distance(source_buffer, first4));
                EXPECT_EQ(cp, code_point4); 
            }
            
        }    
    }    

    
    
}
#endif

//
//  code_point_conversions_test.cpp.cpp
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


using namespace json;


namespace json { namespace unicode { namespace test {
    
    enum utf8_state {
        kUTF8_Wellformed = 0,
        kUTF8_UnexpectedEnd = 1,
        kUTF8_InvalidFirstTrailByte = 2,
        kUTF8_InvalidStartByte = 3,
        kUTF8_TrailByteExpected = 4
    };
    
    typedef std::pair<int, const uint8_t*> utf8_test_result_type;
    
    // Tests for a well-formed UTF-8 sequence according Unicode Standard 6.0.0
    
    // Note: The implementation does deliberately not use any of the existing
    // unicode functions in namespace json::unicode.

    // If the code sequence is ill-formed utf8_result_type.first contains 
    // the kind of error, and utf8_result_type.second points to the end of
    // the "maximal subpart of an ill-formed subsequence".
    
    inline utf8_test_result_type 
    utf8_test_wellformed(const uint8_t* first, const uint8_t* last) 
    {    
        const uint8_t* s = first;
                
        if (s == last) {
            return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
        }

        if (*s <= 0x7F) {
            ++s;
            return utf8_test_result_type(kUTF8_Wellformed, s); 
        }
        else if (*s >= 0xC2 and *s <= 0xDF) {
            ++s;
            if (s == last) {
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            }            
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
                return utf8_test_result_type(kUTF8_Wellformed, s);
            }
            else {
                // trail byte expected
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
        }
        else if (*s == 0xE0) {
            ++s;
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0xA0 and *s <= 0xBF) {
                ++s;
            }
            else {
                if (*s >= 0x80 and *s <= 0xBF) {
                    return utf8_test_result_type(kUTF8_InvalidFirstTrailByte, s);
                }
                else {                    
                    return utf8_test_result_type(kUTF8_TrailByteExpected, s);
                }
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
                return utf8_test_result_type(kUTF8_Wellformed, s);
            }
            else {
                // trail byte expected
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
        }
        else if ( (*s >= 0xE1 and *s <= 0xEC) or (*s >= 0xEE and *s <= 0xEF)) {
            ++s;
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
                return utf8_test_result_type(kUTF8_Wellformed, s);
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }            
        }
        else if (*s == 0xED) {
            ++s;
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0x9F) {
                ++s;
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
                return utf8_test_result_type(kUTF8_Wellformed, s);
            }
            else {
                if (*s >= 0x80 and *s <= 0xBF) {
                    return utf8_test_result_type(kUTF8_InvalidFirstTrailByte, s);
                }
                else {                    
                    return utf8_test_result_type(kUTF8_TrailByteExpected, s);
                }
            }
        }
        else if (*s == 0xF0) {
            ++s;
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x90 and *s <= 0xBF) {
                ++s;
            }
            else {
                if (*s >= 0x80 and *s <= 0xBF) {
                    return utf8_test_result_type(kUTF8_InvalidFirstTrailByte, s);
                }
                else {                    
                    return utf8_test_result_type(kUTF8_TrailByteExpected, s);
                }
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
                return utf8_test_result_type(kUTF8_Wellformed, s);
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
        }
        else if (*s >= 0xF1 and *s <= 0xF3) {
            ++s;
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
                return utf8_test_result_type(kUTF8_Wellformed, s);
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
        }
        else if (*s == 0xF4) {
            ++s;
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0x8F) {
                ++s;
            }
            else {
                if (*s >= 0x80 and *s <= 0xBF) {
                    return utf8_test_result_type(kUTF8_InvalidFirstTrailByte, s);
                }
                else {                    
                    return utf8_test_result_type(kUTF8_TrailByteExpected, s);
                }
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
            if (s == last)
                return utf8_test_result_type(kUTF8_UnexpectedEnd, s);
            if (*s >= 0x80 and *s <= 0xBF) {
                ++s;
                return utf8_test_result_type(kUTF8_Wellformed, s);
            }
            else {
                return utf8_test_result_type(kUTF8_TrailByteExpected, s);
            }
        }
        else {
            return utf8_test_result_type(kUTF8_InvalidStartByte, s);
        }        
    }
    
    
    
    class utf8_generator 
    {
    public:
        typedef boost::array<uint8_t, 4> buffer_type;
        
        
        utf8_generator(buffer_type first, buffer_type last) {
            first_ = from_buffer(first);
            last_ = from_buffer(last);
            current_ = first_;
        }
        
        bool hasNext() const {
            return current_ != last_;
        }
        
        buffer_type next() {
            return to_buffer(current_++);
        }
    
    private:
        buffer_type to_buffer(uint32_t v)
        {
            buffer_type result;
            result[0] = uint8_t(0x000000FF & v);
            result[1] = uint8_t((v >> 8) & 0x000000FF);
            result[2] = uint8_t((v >> 16) & 0x000000FF);
            result[3] = uint8_t((v >> 24) & 0x000000FF);
            return result;
        }
        uint32_t from_buffer(buffer_type buf)
        {
            uint32_t result = buf[0];
            result += uint32_t(buf[1]) << 8;
            result += uint32_t(buf[2]) << 16;
            result += uint32_t(buf[3]) << 24;
            return result;
        }
        
    private:
        
        uint32_t first_;
        uint32_t last_;
        uint32_t current_;
        
    };
    
}}}  // namespace json::unicode::test


namespace {
    
    
    using namespace json::unicode;
    
    
    class code_point_conversions_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        code_point_conversions_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~code_point_conversions_test() {
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
            int result1 = convert_one_unsafe(cp, dest1);
            int result2 = convert_one(cp, dest2);
            int result3 = convert_one_unsafe(cp, dest3, filter::Noncharacter(0));
            int result4 = convert_one(cp, dest4, filter::Noncharacter(0));
            
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
            int result1 = convert_one_unsafe(cp, dest1);
            int result2 = convert_one(cp, dest2);
            int result3 = convert_one_unsafe(cp, dest3, filter::Noncharacter(0));
            int result4 = convert_one(cp, dest4, filter::Noncharacter(0));
            
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
        
        
        for (code_point_t cp = 0xd780; cp <= (0x10FFFF); ++cp) 
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
                int result = convert_one_unsafe(cp, p);  // convert cp to UTF-8
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
            int result1 = convert_one_unsafe(first1, last, code_point1);
            int result2 = convert_one(first2, last, code_point2);
            int result3 = convert_one_unsafe(first3, last, code_point3, filter::Noncharacter(0));
            int result4 = convert_one(first4, last, code_point4, filter::Noncharacter(0));
            
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
                int result = convert_one_unsafe(cp, p);  // convert cp to UTF-16
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
            int result1 = convert_one_unsafe(first1, last, code_point1);
            int result2 = convert_one(first2, last, code_point2);
            int result3 = convert_one_unsafe(first3, last, code_point3, filter::Noncharacter(0));
            int result4 = convert_one(first4, last, code_point4, filter::Noncharacter(0));
            
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

    
    
#pragma mark - UTF-8 Wellformdness
    
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
    TEST_F(code_point_conversions_test, UTF8_Wellformdness)     
#else
    TEST_F(code_point_conversions_test, DISABLED_UTF8_Wellformdness)     
#endif
    {
        using json::unicode::test::utf8_generator;
        using json::unicode::test::utf8_test_wellformed;
        using json::unicode::test::utf8_test_result_type;
        
        
        typedef utf8_generator::buffer_type buffer_t;
        typedef utf8_generator generator_t;
        
        buffer_t first = {0, 0, 0, 0 };
        buffer_t last = {0xFF, 0xFF, 0xFF, 0xFF };
        
        generator_t generator = generator_t(first, last);
        
        std::size_t count = 0;
        const uint32_t marker = std::numeric_limits<uint32_t>::max() / 100;
        
        while (generator.hasNext()) 
        {
            code_point_t code_point;
            const buffer_t utf8 = generator.next();
            
            ++count;
            if ((count % marker) == 0) {
                printf(".");
                fflush(stdout);
            }
            const uint8_t* first = utf8.begin();
            int result = convert_one(first, utf8.end(), code_point);
            int consumed = (int)std::distance(utf8.begin(), first);
            
            utf8_test_result_type reference_result = utf8_test_wellformed(utf8.begin(), utf8.end());
            int reference_consumed = (int)std::distance(utf8.begin(), reference_result.second);
            if (reference_result.first == 0) {
                if (reference_consumed != consumed or result <= 0) {
                    EXPECT_TRUE( result > 0 );
                    EXPECT_EQ(reference_consumed, consumed); 
                    int a = utf8[0];
                    int b = utf8[1];
                    int c = utf8[2];
                    int d = utf8[3];
                    printf("with utf8: %x %x %x %x\n", a, b, c, d);
                }
            }
            else {
                if (reference_consumed != consumed or result >= 0) {
                    EXPECT_TRUE(result < 0);
                    EXPECT_EQ(reference_consumed, consumed);
                    int a = utf8[0];
                    int b = utf8[1];
                    int c = utf8[2];
                    int d = utf8[3];
                    printf("with utf8: %x %x %x %x\n", a, b, c, d);
                }
            }
        
        }
    
    }
    
    
#pragma mark - Ill-formed UTF to Unicode Code Point
    struct test_item_t {
        uint8_t buffer[6];
        int result;
        int consumed;
        const char* desc;
    };
    
    TEST_F(code_point_conversions_test, Malformed_UTF8)     
    {   
        
        test_item_t test_items[] = {
            {{0x80, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0x90, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xA0, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xB0, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xBF, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xC0, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xC0, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xC1, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xF5, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xFF, 0x30, 0x31, 0x33, 0},   -1, 0, "invalid start byte" },
            {{0xC2, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xDF, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xE0, 0x80, 0x30},            -1, 1, "Invalid number of trails at index 1."},
            {{0xE1, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xEC, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xED, 0xA0, 0x30},            -1, 1, "Invalid number of trails at index 1."},
            {{0xEE, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xEF, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xF0, 0x80, 0x30},            -1, 1, "Invalid number of trails at index 1."},
            {{0xF1, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xF3, 0x30, 0x31},            -1, 1, "Invalid number of trails at index 1."},
            {{0xF4, 0x90, 0x30},            -1, 1, "Invalid number of trails at index 1."}
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
            code_point_t code_point;
            const uint8_t* start = (*first).buffer;
            const uint8_t* end = start + sizeof((*first).buffer);
            int result = convert_one(start, end, code_point);
            int consumed = (int)std::distance((*first).buffer, start);
            EXPECT_TRUE(result < 0);
            //EXPECT_EQ((*first).result, result);
            EXPECT_EQ((*first).consumed, consumed);
            
            ++first;
        }    
        
        
    }
}

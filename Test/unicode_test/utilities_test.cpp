//
//  utilities_test.cpp
//  Test
//
//  Created by Andreas Grosam on 7/25/11.
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

#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_traits.hpp"
#include <gtest/gtest.h>

#include <iostream>
#include <iomanip>
#include <iterator>
#include <algorithm>

// for testing
#include <unicode/ustring.h>
#include <unicode/utf.h>
//#include <unicode/utf8.h>
//#include <unicode/utf16.h>
#include <limits>

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/array.hpp>

using namespace json;


namespace test {
    
    
    using namespace json;
    
    
    template <typename InEncodingT, typename OutEncodingT>
    struct conversion {
        typedef InEncodingT     from_encoding_t;
        typedef OutEncodingT    to_encoding_t;
    };
    
    
    int inline icu_utf8_count_trail_bytes(uint8_t leadByte) {
#if 0        
        uint8_t buffer[5] = {};
        UChar32 ignored;
        buffer[0] = leadByte;
        int offset = 0;
        U8_NEXT_UNSAFE(buffer, offset, ignored);
        return offset - 1;
#else
        return U8_COUNT_TRAIL_BYTES(leadByte);
#endif        
    }
    
}

#pragma mark - UTF Encoding Tag
    
 
namespace test {
    //    Bytes           Encoding Form
    //    ---------------------------------------
    //    00 00 FE FF     UTF-32, big-endian
    //    FF FE 00 00     UTF-32, little-endian
    //    FE FF           UTF-16, big-endian
    //    FF FE           UTF-16, little-endian
    //    EF BB BF        UTF-8
    
    
    template <typename Encoding>
    struct bom_bytes {
    };
    
    template <>
    struct bom_bytes<unicode::UTF_8_encoding_tag> {
        typedef boost::array<uint8_t, 3> array_t;
        static array_t bom() { 
            array_t result = {{0xEF, 0xBB, 0xBF}};
            return result;
        }
    };
        
        
    template <>
    struct bom_bytes<unicode::UTF_16_encoding_tag> {
        typedef boost::array<uint8_t, 2> array_t;
        static array_t bom() { 
#if defined(BOOST_LITTLE_ENDIAN)
            array_t result = {{0xFF, 0xFE}};
#else
            array_t result = {{0xFE, 0xFF}};
#endif            
            return result;
        }
    };
        
    
    template <>
    struct bom_bytes<unicode::UTF_16BE_encoding_tag> {
        typedef boost::array<uint8_t, 2> array_t;
        static array_t bom() { 
            array_t result = {{0xFE, 0xFF}};
            return result;
        }
    };
    
    template <>
    struct bom_bytes<unicode::UTF_16LE_encoding_tag> {
        typedef boost::array<uint8_t, 2> array_t;
        static array_t bom() { 
            array_t result = {{0xFF, 0xFE}};
            return result;
        }
    };
    

    template <>
    struct bom_bytes<unicode::UTF_32_encoding_tag> {
        typedef boost::array<uint8_t, 4> array_t;
        static array_t bom() { 
#if defined(BOOST_LITTLE_ENDIAN)
            array_t result = {{0xFF, 0xFE, 0x00, 0x00}};
#else
            array_t result = {{0x00, 0x00, 0xFE, 0xFF}};
#endif            
            return result;
        }
    };
                
    template <>
    struct bom_bytes<unicode::UTF_32BE_encoding_tag> {
        typedef boost::array<uint8_t, 4> array_t;
        static array_t bom() { 
            array_t result = {{0x00, 0x00, 0xFE, 0xFF}};
            return result;
        }
    };
    
    template <>
    struct bom_bytes<unicode::UTF_32LE_encoding_tag> {
        typedef boost::array<uint8_t, 4> array_t;
        static array_t bom() { 
            array_t result = {{0xFF, 0xFE, 0x00, 0x00}};
            return result;
        }
    };
    
    
}

namespace {
    
    using namespace json;
        
    typedef unicode::UTF_8_encoding_traits::code_unit_type utf8_code_unit;
    typedef unicode::UTF_16_encoding_traits::code_unit_type utf16_code_unit;
    typedef unicode::UTF_32_encoding_traits::code_unit_type utf32_code_unit;
    
    template <typename T>
    class UTF_Encoding_tag_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        UTF_Encoding_tag_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~UTF_Encoding_tag_test() {
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
    
    
    TYPED_TEST_CASE_P(UTF_Encoding_tag_test);
        
    
    
    
    TYPED_TEST_P(UTF_Encoding_tag_test, TestBOM) 
    {
        // Inside a test, refer to TypeParam to get the type parameter.
        
        typedef TypeParam                               EncodingTag;
        typedef typename unicode::encoding_traits<EncodingTag>::bom_type          bom_type;
        typedef typename unicode::encoding_traits<EncodingTag>::code_unit_type    code_unit_t;
        
        union bom_t {
            bom_type    bom;
            uint8_t     bytes[8];
        };        
        bom_t bom;
        bom.bom = unicode::encoding_traits<EncodingTag>::bom();
        
        const int test_bom_byte_size = unicode::encoding_traits<EncodingTag>::bom_byte_size;
        const int ref_bom_byte_size = (int)test::bom_bytes<EncodingTag>::bom().size();
        
        EXPECT_EQ(ref_bom_byte_size, test_bom_byte_size);
        for (int i = 0; i < ref_bom_byte_size; ++i) {
            EXPECT_EQ(test::bom_bytes<EncodingTag>::bom()[i], bom.bytes[i]);
        }
    }
    
    
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(UTF_Encoding_tag_test,
                               TestBOM);
    
    
    // Instantiate test cases:
    typedef ::testing::Types<
        unicode::UTF_8_encoding_tag, 
        unicode::UTF_16_encoding_tag,
        unicode::UTF_16BE_encoding_tag,
        unicode::UTF_16LE_encoding_tag,
        unicode::UTF_32_encoding_tag,
        unicode::UTF_32BE_encoding_tag,
        unicode::UTF_32LE_encoding_tag
    >  UTF_encodings;
    
    
    INSTANTIATE_TYPED_TEST_CASE_P(BOM_Tests, UTF_Encoding_tag_test, UTF_encodings);
    
}



namespace {
    
    
    using namespace json::unicode;
    
    
    class utilities_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        utilities_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~utilities_test() {
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
    

    TEST_F(utilities_test, UTFEncodingConstantToEncodingType) {
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<UnicodeEncoding_UTF8>::type,
                      UTF_8_encoding_tag
                      >::value ));        
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<UnicodeEncoding_UTF16>::type,
                      UTF_16_encoding_tag
                      >::value ));        
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<UnicodeEncoding_UTF16LE>::type,
                      UTF_16LE_encoding_tag
                      >::value ));        
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<UnicodeEncoding_UTF16BE>::type,
                      UTF_16BE_encoding_tag
                      >::value ));        
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<UnicodeEncoding_UTF32>::type,
                      UTF_32_encoding_tag
                      >::value ));        
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<UnicodeEncoding_UTF32LE>::type,
                      UTF_32LE_encoding_tag
                      >::value ));        
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<UnicodeEncoding_UTF32BE>::type,
                      UTF_32BE_encoding_tag
                      >::value ));        
        
        EXPECT_TRUE( (boost::is_same<
                      encoding_to_tag<PlatformEncoding>::type,
                      platform_encoding_tag
                      >::value ));        
        
    }
    
    
    
#pragma mark - Endian
    
    TEST_F(utilities_test, UTFtoHostEndianConversion) {

#if defined(BOOST_LITTLE_ENDIAN)
        typedef UTF_16LE_encoding_tag UTF_16host_encoding_tag;
        typedef UTF_32LE_encoding_tag UTF_32host_encoding_tag;
#else
        typedef UTF_16BE_encoding_tag UTF_16host_encoding_tag;
        typedef UTF_32BE_encoding_tag UTF_32host_encoding_tag;
#endif        
        
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<UTF_8_encoding_tag>::type,
                      UTF_8_encoding_tag
                      >::value ));
                
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<UTF_16_encoding_tag>::type,
                      UTF_16host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<UTF_16LE_encoding_tag>::type,
                      UTF_16host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<UTF_16BE_encoding_tag>::type,
                      UTF_16host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<UTF_32_encoding_tag>::type,
                      UTF_32host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<UTF_32LE_encoding_tag>::type,
                      UTF_32host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<UTF_32BE_encoding_tag>::type,
                      UTF_32host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      to_host_endianness<platform_encoding_tag>::type,
                      platform_encoding_tag
                      >::value ));
    }


    TEST_F(utilities_test, AddHostEndianConversion) {
        
#if defined(BOOST_LITTLE_ENDIAN)
        typedef UTF_16LE_encoding_tag UTF_16host_encoding_tag;
        typedef UTF_32LE_encoding_tag UTF_32host_encoding_tag;
#else
        typedef UTF_16BE_encoding_tag UTF_16host_encoding_tag;
        typedef UTF_32BE_encoding_tag UTF_32host_encoding_tag;
#endif        
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<UTF_8_encoding_tag>::type,
                      UTF_8_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<UTF_16_encoding_tag>::type,
                      UTF_16host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<UTF_16LE_encoding_tag>::type,
                      UTF_16LE_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<UTF_16BE_encoding_tag>::type,
                      UTF_16BE_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<UTF_32_encoding_tag>::type,
                      UTF_32host_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<UTF_32LE_encoding_tag>::type,
                      UTF_32LE_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<UTF_32BE_encoding_tag>::type,
                      UTF_32BE_encoding_tag
                      >::value ));
        
        EXPECT_TRUE( (boost::is_same<
                      add_endianness<platform_encoding_tag>::type,
                      platform_encoding_tag
                      >::value ));
    }
    

    
    
    
#pragma mark -
#pragma mark Unicode Code Point

    TEST_F(utilities_test, isCodePoint) {
        EXPECT_TRUE(isCodePoint(0x10FFFEu));
        EXPECT_TRUE(isCodePoint(0x10FFFFu));
        EXPECT_FALSE(isCodePoint(0x110000u));
    } 

    
    TEST_F(utilities_test, isHighSurrogate) {
        // High-surrogate code point: A Unicode code point in the range 
        // U+D800 to U+DBFF.
        for (UChar32 i = 0; i < 0xFFFFFFu; ++i) 
        {
            bool result = isHighSurrogate(i);
            bool icu_result = U_IS_LEAD(i);
            
            EXPECT_EQ(icu_result, result) 
                << "for code point: " << std::hex << "0x" << i;
            if (icu_result != result)
            {
                break;
            }
        }
    }
    
    TEST_F(utilities_test, isLowSurrogate) {
        // Low-surrogate code point: A Unicode code point in the range 
        // U+DC00 to U+DFFF.        
        for (UChar32 i = 0; i < 0xFFFFFFu; ++i) 
        {
            bool result = isLowSurrogate(i);
            bool icu_result = U_IS_TRAIL(i);
            
            EXPECT_EQ(icu_result, result) 
                << "for code point: " << std::hex << "0x" << i;
            if (icu_result != result)
            {
                break;
            }
        }
    }
    
    TEST_F(utilities_test, isUnicodeCharacter) 
    {        
        for (UChar32 i = 0; i < 0xFFFFFFu; ++i) 
        {
            bool result = isUnicodeCharacter(i);
            bool icu_result = U_IS_UNICODE_CHAR(i);
            
            int fail_count = 0;
            EXPECT_EQ(icu_result, result) 
            << "for code point: " << std::hex << "0x" << i;
            if (icu_result != result)
            {
                ++fail_count;
                if (fail_count > 0) {
                    break;
                }
            }
        }
    }
        
    
    TEST_F(utilities_test, isNonCharacter) 
    {
        for (UChar32 i = 0; i < 0xFFFFFFu; ++i) 
        {
            bool result = isNonCharacter(i);
            bool icu_result = U_IS_UNICODE_NONCHAR(i);
            
            EXPECT_EQ(icu_result, result) 
            << "for code point: " << std::hex << "0x" << i;
            if (icu_result != result)
            {
                break;
            }
        }
    }
    
//    TEST_F(utilities_test, DISABLED_isControlCode) {
//    }

    
    
#pragma mark -
#pragma mark utf8
    
    TEST_F(utilities_test, uft8_is_single) 
    {
        for (int i = 0; i <= 0xFF; ++i) {
            EXPECT_EQ((i >= 0 and i <= 0x7F), utf8_is_single(i));
        }
    }
    
    
    TEST_F(utilities_test, utf8_is_lead_int) 
    {
        // utf8_is_lead() shall return true if the given code unit is a valid 
        // "lead byte", e.g. a byte which is not a ASCII and which is not a 
        // "trail byte". Due to limitations in valid code sequences, the range 
        // of valid lead bytes is further restricted. 
        // (see. unicode.org, http://en.wikipedia.org/wiki/UTF-8#Invalid_byte_sequences)
        // So, in practice the valid range of lead bytes is as follows:
        // [0xC2 .. 0xF4]
        
        // with the bit pattern 
        // b110x.xxxx  or
        // b1110.xxxx  or
        // b1111.0xxx   
        
        int first = 0;
        int last = 0xFF;
        for (int i = first; i <= last; ++i) {
            int ch = i;
            bool is_lead = utf8_is_lead(ch);
            //printf("utf8_is_lead(%d) = %s\n", i, (utf8_is_lead_ ? "true":"false"));
            EXPECT_EQ(( uint8_t(ch) >= 0xC2u) and (uint8_t(ch) <= 0xF4u), is_lead) 
            << "with i: 0x" << std::hex << ch;
        }
    }
    
    TEST_F(utilities_test, utf8_is_lead_char) 
    {
        int first = std::numeric_limits<char>::min();
        int last = std::numeric_limits<char>::max();
        for (int i = first; i <= last; ++i) {
            char ch = i;
            bool is_lead = utf8_is_lead(ch);
            //printf("utf8_is_lead(%d) = %s\n", i, (utf8_is_lead_ ? "true":"false"));
            EXPECT_EQ(( uint8_t(ch) >= 0xC2u) and (uint8_t(ch) <= 0xF4u), is_lead) 
            << "with i: 0x" << std::hex << ch;
        }
    }
    
    TEST_F(utilities_test, utf8_is_lead_unsigned_char) {
        int first = std::numeric_limits<unsigned char>::min();
        int last = std::numeric_limits<unsigned char>::max();
        for (int i = first; i <= last; ++i) {
            unsigned char ch = i;
            bool is_lead = utf8_is_lead(ch);
            //printf("utf8_is_lead(%d) = %s\n", i, (utf8_is_lead_ ? "true":"false"));
            EXPECT_EQ(( uint8_t(ch) >= 0xC2u) and (uint8_t(ch) <= 0xF4u), is_lead) 
            << "with i: 0x" << std::hex << ch;
        }
    }
    
    TEST_F(utilities_test, utf8_is_lead_signed_char) 
    {
        int first = std::numeric_limits<signed char>::min();
        int last = std::numeric_limits<signed char>::max();
        for (int i = first; i <= last; ++i) {
            signed char ch = i;
            bool is_lead = utf8_is_lead(ch);
            //printf("utf8_is_lead(%d) = %s\n", i, (utf8_is_lead_ ? "true":"false"));
            EXPECT_EQ(( uint8_t(ch) >= 0xC2u) and (uint8_t(ch) <= 0xF4u), is_lead) 
            << "with i: 0x" << std::hex << ch;
        }
    }
    
    TEST_F(utilities_test, utf8_is_trail_int) {
        int first =  0;
        int last = 255;
        for (int i = first; i <= last; ++i) {
            int ch = i;
            bool result = utf8_is_trail(ch);
            bool icu_result = U8_IS_TRAIL(ch);
            EXPECT_EQ(icu_result, result) 
            << "with UTF-6 code unit: 0x" << std::hex << ch;
        }
    }
    
    TEST_F(utilities_test, utf8_is_trail_char) {
        int first = std::numeric_limits<char>::min();
        int last = std::numeric_limits<char>::max();
        for (int i = first; i <= last; ++i) {
            char ch = i;
            bool result = utf8_is_trail(ch);
            bool icu_result = U8_IS_TRAIL(ch);
            EXPECT_EQ(icu_result, result) 
            << "with UTF-6 code unit: 0x" << std::hex << ch;
        }
    }
    
    TEST_F(utilities_test, utf8_is_trail_unsigned_char) {
        int first = std::numeric_limits<unsigned char>::min();
        int last = std::numeric_limits<unsigned char>::max();
        for (int i = first; i <= last; ++i) {
            unsigned char ch = i;
            bool result = utf8_is_trail(ch);
            bool icu_result = U8_IS_TRAIL(ch);
            EXPECT_EQ(icu_result, result) 
            << "with UTF-6 code unit: 0x" << std::hex << ch;
        }
    }
    
    TEST_F(utilities_test, utf8_is_trail_signed_char) {
        int first = std::numeric_limits<signed char>::min();
        int last = std::numeric_limits<signed char>::max();
        for (int i = first; i <= last; ++i) {
            signed char ch = i;
            bool result = utf8_is_trail(ch);
            bool icu_result = U8_IS_TRAIL(ch);
            EXPECT_EQ(icu_result, result) 
            << "with UTF-6 code unit: 0x" << std::hex << ch;
        }
    }
    
    
    
    
    TEST_F(utilities_test, utf8_encoded_length) {        
        int failCount = 0;
        for (code_point_t i = 0; i < (kUnicodeCodeSpaceMax + 100); ++i) {
            int result = utf8_encoded_length(i);
            int icu_result = U8_LENGTH(i);
            int expected_result = icu_result;
            failCount += result != expected_result ? 1 : 0;
            EXPECT_EQ(expected_result, result) 
                << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }

    
    TEST_F(utilities_test, utf8_encoded_length_unsafe) {        
        int failCount = 0;
        for (code_point_t i = 0; i < (kUnicodeCodeSpaceMax + 100); ++i) {
            int result = utf8_encoded_length_unsafe(i);
            int icu_result = U8_LENGTH(i);
            int expected_result = isSurrogate(i) ? result : icu_result;
            failCount += result != expected_result ? 1 : 0;
            EXPECT_EQ(expected_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    TEST_F(utilities_test, utf8_num_trails) {   
        int failCount = 0;
        for (code_point_t i = 0; i < 255; ++i) {
            int ref_result = -1;
            if (utf8_is_single(i) or utf8_is_lead(i)) {
                ref_result = test::icu_utf8_count_trail_bytes(static_cast<uint8_t>(i));
            }
            int result = utf8_num_trails(i);
            if (ref_result != result) {
                ++failCount;
                EXPECT_EQ(ref_result, result) 
                << "with UTF-8 code unit: 0x" << std::hex << i << " is single: " << (utf8_is_single(i) ? "yes" : "no  ") 
                <<  "is lead byte: " << (utf8_is_lead(i) ? "yes" : "no") << std::endl;
                if (failCount > 10) {
                    break;
                }
            }
        }
    }
    
    
#pragma mark -
#pragma mark utf16
    
    TEST_F(utilities_test, utf16_is_high_surrogate) 
    {
        int failCount = 0;
        for (utf16_code_unit i = 0; i < std::numeric_limits<utf16_code_unit>::max(); ++i) {
            bool result = utf16_is_high_surrogate(i);
            bool icu_result = U16_IS_SURROGATE(i) and U16_IS_SURROGATE_LEAD(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
                << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_is_low_surrogate) 
    {
        int failCount = 0;
        for (utf16_code_unit i = 0; i < std::numeric_limits<utf16_code_unit>::max(); ++i) {
            int result = utf16_is_low_surrogate(i);
            int icu_result = U16_IS_SURROGATE(i) and U16_IS_SURROGATE_TRAIL(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_is_surrogate) 
    {
        int failCount = 0;
        for (utf16_code_unit i = 0; i < std::numeric_limits<utf16_code_unit>::max(); ++i) {
            int result = utf16_is_surrogate(i);
            int icu_result = U16_IS_SURROGATE(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, uft16_is_single) 
    {
        int failCount = 0;
        for (utf16_code_unit i = 0; i < std::numeric_limits<utf16_code_unit>::max(); ++i) {
            int result = utf16_is_single(i);
            int icu_result = U16_IS_SINGLE(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_is_lead) 
    {
        int failCount = 0;
        for (utf16_code_unit i = 0; i < std::numeric_limits<utf16_code_unit>::max(); ++i) {
            int result = utf16_is_lead(i);
            int icu_result = U16_IS_LEAD(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_is_trail) 
    {
        int failCount = 0;
        for (utf16_code_unit i = 0; i < std::numeric_limits<utf16_code_unit>::max(); ++i) {
            int result = utf16_is_trail(i);
            int icu_result = U16_IS_TRAIL(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_encoded_length) 
    {
        int failCount = 0;
        for (code_point_t i = 0; i < kUnicodeCodeSpaceMax; ++i) {
            int result = utf16_encoded_length(i);
            int icu_result = U16_LENGTH(i);
            int expected_result = U16_IS_SURROGATE(i) ? 0 : icu_result;
            failCount += result != expected_result ? 1 : 0;
            EXPECT_EQ(expected_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    TEST_F(utilities_test, utf16_encoded_length_unsafe) 
    {
        int failCount = 0;
        for (code_point_t i = 0; i < kUnicodeCodeSpaceMax; ++i) {
            int result = utf16_encoded_length_unsafe(i);
            int icu_result = U16_LENGTH(i);
            int expected_result = icu_result;
            failCount += result != expected_result ? 1 : 0;
            EXPECT_EQ(expected_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_surrogate_pair_to_code_point) 
    {
        int failCount = 0;
        for (utf16_code_unit first = 0; first < std::numeric_limits<utf16_code_unit>::max(); ++first) {
            if (utf16_is_high_surrogate(first)) {
                for (utf16_code_unit second = 0; second < std::numeric_limits<utf16_code_unit>::max(); ++second) {
                    if (utf16_is_low_surrogate(second)) {
                        code_point_t result = utf16_surrogate_pair_to_code_point(first, second);
                        code_point_t icu_result = U16_GET_SUPPLEMENTARY(first, second);
                        failCount += result != icu_result ? 1 : 0;
                        EXPECT_EQ(icu_result, result) 
                            << "with high surrogate: 0x" << std::hex << first
                            << ", with low surrogate: 0x" << std::hex << second;
                        if (failCount > 10) {
                            return;
                        }
                    }
                }
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_get_lead) 
    {
        for (code_point_t i = 0x10000; i < 0x10FFFF; ++i) 
        {
            int failCount = 0;
            utf16_code_unit result = utf16_get_lead(i);
            utf16_code_unit icu_result = U16_LEAD(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
                << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
    TEST_F(utilities_test, utf16_get_trail) 
    {
        for (code_point_t i = 0x10000; i < 0x10FFFF; ++i) {
            int failCount = 0;
            utf16_code_unit result = utf16_get_trail(i);
            utf16_code_unit icu_result = U16_TRAIL(i);
            failCount += result != icu_result ? 1 : 0;
            EXPECT_EQ(icu_result, result) 
            << "with code point: 0x" << std::hex << i;
            if (failCount > 10) {
                break;
            }
        }
    }
    
    
}

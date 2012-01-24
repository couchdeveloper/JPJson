//
//  unicode_utilities.hpp
//
//  Created by Andreas Grosam on 5/16/11.
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

#ifndef JSON_UNICODE_UTILITIES_HPP
#define JSON_UNICODE_UTILITIES_HPP


#include "json/config.hpp"
#include <assert.h>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>
#include <boost/array.hpp>
#include <stdint.h>
#include "json/endian/endian.hpp"
#include "json/endian/byte_swap.hpp"

//  
//  A set of types and utility functions supporting Unicode
//  according Unicode Standard Version 6.0
// 
//  For specifications, see 
//  http://www.unicode.org
//  http://en.wikipedia.org/wiki/UTF-8
//  RFC 3629, http://tools.ietf.org/html/rfc3629

namespace json { namespace unicode {
    
    
    using json::internal::host_endianness;
    
  
    enum UNICODE_ENCODING {
        UNICODE_ENCODING_UTF_8 =    1,
        UNICODE_ENCODING_UTF_16BE = 2,
        UNICODE_ENCODING_UTF_16LE = 3,
        UNICODE_ENCODING_UTF_32BE = 4,
        UNICODE_ENCODING_UTF_32LE = 5
    };
    
    
    //
    //  Encoding Tags
    //
    //  Types:
    //
    //  code_unit_type      The underlaying integer type representing an Unicode 
    //                      code unit. It is the minimal bit combination that
    //                      can represent a unit of encoded text for processing
    //                      or interchange.
    //
    //  encode_buffer_type  The minimal buffer required to store one encoded 
    //                      Unicode code point.
    //
    //  endian_tag          The endianess type property of the encoding.
    //
    //  bom_type            A static vector of values of code_unit_type which
    //                      is capable to store a BOM.
    //
        
    struct utf_encoding_tag {};
    
    struct UTF_8_encoding_tag : utf_encoding_tag {
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 1;
        typedef char                code_unit_type;
        static const int            buffer_size = 4;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        typedef json::internal::endian_tag  endian_tag;
        const char*                 name() const { return "UTF-8"; }
        typedef boost::array<code_unit_type,3>      bom_type;
        static const bom_type       bom() { bom_type result = {{0xEF, 0xBB, 0xBF}}; return result; };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
    };
    
    struct UTF_16_encoding_tag : utf_encoding_tag {
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 2;
        typedef uint16_t            code_unit_type;
        static const int            buffer_size = 2;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        const char*                 name() const { return "UTF-16"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{0xFEFFu}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
    }; 
    struct UTF_16BE_encoding_tag : UTF_16_encoding_tag {
        typedef json::internal::big_endian_tag      endian_tag;
        const char*                 name() const { return "UTF-16BE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() {
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
    };
    struct UTF_16LE_encoding_tag : UTF_16_encoding_tag {
        typedef json::internal::little_endian_tag   endian_tag;
        const char*                 name() const { return "UTF-16LE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
    };
    
    struct platform_encoding_tag : utf_encoding_tag {
        static const unsigned int   max_bytes = sizeof(wchar_t);
        static const unsigned int   min_bytes = sizeof(wchar_t);
        typedef wchar_t             code_unit_type;
        static const int            buffer_size = 1;
        typedef wchar_t             encode_buffer_type[buffer_size];
        typedef host_endianness::type endian_tag;
        const char*                 name() const { return "platform"; }
    };

    
    struct UTF_32_encoding_tag : utf_encoding_tag {
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 4;
        typedef uint32_t            code_unit_type;
        static const int            buffer_size = 1;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        const char*                 name() const { return "UTF-32"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{static_cast<code_unit_type>(0xFEFFu)}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
    };
    struct UTF_32BE_encoding_tag : UTF_32_encoding_tag {
        typedef json::internal::big_endian_tag      endian_tag;
        const char*                 name() const { return "UTF-32BE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
    };
    struct UTF_32LE_encoding_tag : UTF_32_encoding_tag {
        typedef json::internal::little_endian_tag   endian_tag;
        const char*                 name() const { return "UTF-32LE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
    };
    
}}    
    
namespace json { namespace unicode {
    
    
    //
    //  Maps json::unicode encoding types to constants
    //
    
    enum Encoding {
        UnicodeEncoding_UTF8        = 1,
        UnicodeEncoding_UTF16BE,
        UnicodeEncoding_UTF16LE,
        UnicodeEncoding_UTF32BE,
        UnicodeEncoding_UTF32LE
    };
    
    
    template <typename EncodingT>
    struct unicode_encoding_traits {};
    
    template <>
    struct unicode_encoding_traits<UTF_8_encoding_tag> {
        static const Encoding value = UnicodeEncoding_UTF8;
    };
    
    template <>
    struct unicode_encoding_traits<UTF_16BE_encoding_tag> {
        static const Encoding value = UnicodeEncoding_UTF16BE;
    };
    
    template <>
    struct unicode_encoding_traits<UTF_16LE_encoding_tag> {
        static const Encoding value = UnicodeEncoding_UTF16LE;
    };
    
    template <>
    struct unicode_encoding_traits<UTF_32BE_encoding_tag> {
        static const Encoding value = UnicodeEncoding_UTF32BE;
    };
    
    template <>
    struct unicode_encoding_traits<UTF_32LE_encoding_tag> {
        static const Encoding value = UnicodeEncoding_UTF32LE;
    };
    

    //
    //
    //
    
    template <int E>
    struct encoding_to_tag {};
        
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF8> {
        typedef UTF_8_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF16BE> {
        typedef UTF_16BE_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF16LE> {
        typedef UTF_16LE_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF32BE> {
        typedef UTF_32BE_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF32LE> {
        typedef UTF_32LE_encoding_tag type;
    };
    
    
    
}}

namespace json { namespace unicode {
#pragma mark -
#pragma mark to_host_endianness
    
    using json::internal::host_endianness;
    using json::internal::little_endian_tag;
    using json::internal::big_endian_tag;
    
    
    // "Upgrades" or converts an UTF-16 or UTF-32 encoding to its corresponding
    // encoding decorated with the endianness of the host.
    
    template <typename EncodingT, typename Enable = void> 
    struct to_host_endianness {
    };
    
    template <typename EncodingT> 
    struct to_host_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_base_of<UTF_16_encoding_tag, EncodingT> >::type
    >  
    {
        typedef typename boost::mpl::if_<
        boost::is_same<host_endianness::type, little_endian_tag>,
            UTF_16LE_encoding_tag,
            UTF_16BE_encoding_tag
        >::type  type;
    };
    
    template <typename EncodingT> 
    struct to_host_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_base_of<UTF_32_encoding_tag, EncodingT> >::type 
    >  
    {
        typedef typename boost::mpl::if_<
            boost::is_same<host_endianness::type, little_endian_tag>,
            UTF_32LE_encoding_tag,
            UTF_32BE_encoding_tag
        >::type  type;
    };
    
    template <typename EncodingT> 
    struct to_host_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_same<UTF_8_encoding_tag, EncodingT> >::type 
    >  
    {
        typedef EncodingT type;
    };
    
}}    
    


namespace json { namespace unicode {
#pragma mark - add_endianness
    
    using json::internal::host_endianness;
    using json::internal::little_endian_tag;
    using json::internal::big_endian_tag;
    
    
    // "Upgrades" an UTF-16 or UTF-32 encoding without endianness
    // to its corresponding encoding decorated with the endianness 
    // of the host.
    // Encodings with endianness will not be changed.
    
    template <typename EncodingT, typename Enable = void> 
    struct add_endianness {
    };
    
    template <typename EncodingT> 
    struct add_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_same<UTF_16_encoding_tag, EncodingT> >::type
    >  
    {
        typedef typename to_host_endianness<EncodingT>::type type;
    };
    
    template <typename EncodingT> 
    struct add_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_same<UTF_32_encoding_tag, EncodingT> >::type 
    >  
    {
        typedef typename to_host_endianness<EncodingT>::type type;
    };
    
    template <typename EncodingT> 
    struct add_endianness<
        EncodingT, 
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                boost::mpl::not_<boost::is_same<UTF_16_encoding_tag, EncodingT> >,
                boost::mpl::not_<boost::is_same<UTF_32_encoding_tag, EncodingT> >
            >
        >::type 
    >  
    {
        typedef EncodingT type;
    };
    
}}    



namespace json { namespace unicode {
#pragma mark - char_type_to_encoding
    
    
    // Return a default encoding in hostendianess from a given char type.
    template <typename CharT>
    struct char_type_to_encoding {};
    
    template <>
    struct char_type_to_encoding<char> {
        typedef UTF_8_encoding_tag type;
    };
    
    template <>
    struct char_type_to_encoding<wchar_t> {
        typedef platform_encoding_tag type;
    };
    
    
#if __cplusplus > 199711L

    template <>
    struct char_type_to_encoding<char16_t> {
        typedef typename to_host_endianness<UTF_16_encoding_tag>::type type;
    };

    template <>
    struct char_type_to_encoding<char32_t> {
        typedef typename to_host_endianness<UTF_32_encoding_tag>::type type;
    };
    
    
#endif
    
}}


namespace json { namespace unicode {    
#pragma mark -
    
    // Determines a default encoding based on the iterator's value_type and
    // the machine endianness:        
    
    template <typename Iterator, typename ValueT = typename boost::iterator_value<Iterator>::type >
    struct iterator_encoding {
        // If the compiler issues an error here, the specialization
        // for the given template parameters does not exist.
        BOOST_STATIC_ASSERT(sizeof(Iterator) == 0); 
    };
    
    
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, char> {
        typedef UTF_8_encoding_tag type;
    };
    template <typename Iterator>
    struct iterator_encoding<Iterator, signed char> {
        typedef UTF_8_encoding_tag type;
    };
    template <typename Iterator>
    struct iterator_encoding<Iterator, unsigned char> {
        typedef UTF_8_encoding_tag type;
    };
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, wchar_t> {
        typedef platform_encoding_tag type;
    };
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, uint16_t> {
        typedef typename boost::mpl::if_c<
        json::internal::host_endianness::is_little_endian, 
        UTF_16LE_encoding_tag, 
        UTF_16BE_encoding_tag
        >::type type;
    };
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, uint32_t> {
        typedef typename boost::mpl::if_c<
        json::internal::host_endianness::is_little_endian, 
        UTF_32LE_encoding_tag, 
        UTF_32BE_encoding_tag
        >::type type;
    };
    



    
    
    template <typename T, typename U>
    struct is_encoding {
        static const bool value = false;
    };
    
    template <>
    struct is_encoding<UTF_8_encoding_tag, UTF_8_encoding_tag> 
    {
        static const bool value = true;
    };
    template <>
    struct is_encoding<UTF_16BE_encoding_tag, UTF_16BE_encoding_tag> 
    {
        static const bool value = true;
    };
    template <>
    struct is_encoding<UTF_32LE_encoding_tag, UTF_32LE_encoding_tag> 
    {
        static const bool value = true;
    };
    template <>
    struct is_encoding<UTF_32BE_encoding_tag, UTF_32BE_encoding_tag> 
    {
        static const bool value = true;
    };
    
    
    
    
    // Stable. Do not change.
    // Several algorithms may be very picky about whether code units
    // are signed or unsigned. We check them here and set it in stone.
    // For UTF-8 all algorithms must work with either signed or usigned 
    // UTF-8 code units.    
    //BOOST_STATIC_ASSERT((boost::is_unsigned<UTF_8_encoding_tag::code_unit_type>::value == true));
    BOOST_STATIC_ASSERT((boost::is_unsigned<UTF_16_encoding_tag::code_unit_type>::value == true));
    BOOST_STATIC_ASSERT((boost::is_unsigned<UTF_32_encoding_tag::code_unit_type>::value == true));
    
    
}}    // namespace json::unicode


namespace json { namespace unicode {
        
    // code_point_t corresponds to a Unicode code point in the Unicode code-
    // space: a range of (unsigned) integers from 0 to 0x10FFFF.
    // (Do not mismatch this type with the UTF-32 "Code Unit" which is as well
    // an unsigend 32-bit integer).
    typedef uint32_t code_point_t;    
    
    
    
    const static code_point_t kReplacementCharacter = 0xFFFD;
    const static code_point_t kUnicodeCodeSpaceMax = 0x10FFFFu;
    
    //const char * const kInvalidCharacterUTF8 = "\xef\xbf\xbd"; // "�"
    //size_t kInvalidCharacterUTF8Length = 3;
        
    
    // Stable. Do not change.
    BOOST_STATIC_ASSERT((boost::is_unsigned<code_point_t>::value == true));    
}}


namespace json { namespace unicode {
    

    
    using json::internal::host_endianness;
    using json::byte_swap;
    
        
    // Returns true if the code point is within range of the
    // Unicode code space [0 .. 0x10FFFF].
    inline bool isCodePoint(code_point_t code_point) {
        return code_point <= 0x10FFFFu;
    }
    
    // Usually, surrogate values are expressed in pairs of 16-bit code units,
    // where the high surrogate comes always first in the stream.
    //    
    // D71: Returns true if the Unicode code point is a high-surrogate code point,
    // an Unicode code point in the range U+D800 to U+DBFF.
    inline bool isHighSurrogate(code_point_t code_point) {
        return (code_point >= 0xD800u and code_point <= 0xDBFFu);  
    }
    // D73: Returns true if the Unicode code point is a Low-surrogate code point,
    // an Unicode code point in the range U+DC00 to U+DFFF.
    inline bool isLowSurrogate(code_point_t code_point) {
        return code_point >= 0xDC00u and code_point <= 0xDFFFu;
    }
    
    // Returns true if the Unicode code point is either a high-surrogate or
    // a low surrogate.
    inline bool isSurrogate(code_point_t code_point) {
        return code_point >= 0xD800u and code_point <= 0xDFFFu;
    }
    

    // A "Noncharacter" is any valid code point that is permanently reserved 
    // for internal use and that should never be interchanged. 
    // Returns true if this is a valid code point and if this is a noncharacter, 
    // otherwise, returns false.
    // see http://unicode.org/reports/tr20/#Noncharacters    
    inline bool isNonCharacter(code_point_t code_point) {
        bool result = 
                (((0x00FFFFu&code_point) >= 0xFFFEu) 
                 or (code_point >= 0xFDD0u and code_point <= 0xFDEFu)
                )
            and code_point <= 0x10FFFFu;
        return result;
    }
    
    
    // Returns true if this is a valid Unicode scalar value (that is, any 
    // Unicode "code point" excluding high and low surrogates). A valid 
    // value implies a valid Unicode code point.
    // D76
    inline bool isUnicodeScalarValue(code_point_t code_point) {
        bool result = not isSurrogate(code_point) and code_point <= 0x10FFFFu;
        return result;
    }
    
    
    // Returns true if the code point is an Unicode character (that is, 
    // an Unicode scalar value excluding noncharacters). 
    inline bool isUnicodeCharacter(code_point_t code_point) {
        bool result = isUnicodeScalarValue(code_point) and not isNonCharacter(code_point);        
        return result;
    }
    
    
    // Returns true if the given code point is defined specifically as
    // a control code. A control point implies, that it is also a valid 
    // code point.
    bool inline isControlCode(code_point_t code_point) {
        bool valid = (code_point <= 0x001Fu) 
            or (code_point >= 0x007Fu and code_point <= 0x009Fu);
        return valid;
    }

    
    
    
}}  // namespace json::internal
    
    
#pragma mark -
#pragma mark UTF-8
namespace json { namespace unicode {
    
    typedef json::unicode::UTF_8_encoding_tag::code_unit_type utf8_code_unit;
    
    //
    // Implementation notes:
    // The algorithm must work with either signed or usigned UTF-8 code units.
    //
    //BOOST_STATIC_ASSERT((boost::is_unsigned<utf8_code_unit>::value == true));
    
    
    // Returns true if the code unit can be encoded into a single byte. That is,
    // in this case the code unit's value is within ASCII [0 .. 0x7F] and can 
    // thus be interpreted directly as ASCII.
    //
    inline bool utf8_is_single(utf8_code_unit cu) {
        return static_cast<uint8_t>(cu) < 0x80u;
    }
    
    // Returns true if the given code unit is a valid lead byte.
    // A "lead byte" is a code unit which is the start byte of a multi byte 
    // sequence consituting an Unicode code point.
    //
    // E.g. a byte with the bit pattern 
    // b110x.xxxx  or   (U+0080 to U+07FF)
    // b1110.xxxx  or   (U+0800 to U+07FF)
    // b1111.0xxx       (U+010000 to U+10FFFF)
    //
    // However, there are bytes which must never be used in a wellformed UTF-8
    // stream: 0xC0 and 0xC1 are (invalid) lead bytes which can only be used to 
    // encode overlong ASCII characters. This is not allowed in UTF-8. The range 
    // [0xF5 to 0xFF] are also not valid lead bytes since it would encode code
    // points larger than 0x10FFFF which are not valid Unicode code points.
    //
    // That is, a valid lead byte is in the range [0xC2 .. 0xF4]. However, the
    // valid lead byte 0xF4 could also be used to encode an Unicode code point
    // which would exceed the valid range!
    // So, a valid lead byte 0xF4 with the matching number of following trail 
    // bytes does not neccessarily constitute a valid UTF-8 sequence since it
    // may encode an Unicode code point greater than 0x10FFFF. Such a UTF-8 
    // sequence is also invalid.
    //
    // A possibly test algorithm, would require to test for the lead byte with 
    // value 0xF4, then check for the following three trail bytes, and finally 
    // convert it to an Unicode code point and test if it is in the valid range. 
    // For all other lead bytes, the Unicode sequence can be checked to be 
    // well-formed without converting to an Unicode code point.
    //
    // The algorithm is strict and takes care of this limitation.
    // 
    inline bool utf8_is_lead(utf8_code_unit cu) {
        // Do not remove any explicit typecasts or any "unsigned" modifiers 
        // applied to variables or constants - unless you really know what 
        // you are doing!!
        bool result = (static_cast<uint8_t>(cu) - 0xC2u) < 0x33u;
        return result;
    }
    
    // Returns true if the given code unit is a trail byte, e.g. a byte with
    // the bit pattern: b10xx.xxxx
    inline bool utf8_is_trail(utf8_code_unit cu) {
        bool result = (static_cast<uint8_t>(cu) & 0xC0u) == 0x80u;
        return result;
    }
    
    // Returns the number of UTF-8 code units required to encode an Unicode 
    // code point.
    // Returns zero if parameter `code_point` is an Unicode surrogate or not a 
    // valid Unicode code point.
    // (A well-formed UTF-8 sequence shall not contain surrogates)
    // Parameter code_point: An Unicode code point.
    inline int utf8_encoded_length(code_point_t code_point) {
        if (code_point <= 0x7Fu) return  1;
        else if (code_point <= 0x7FFu) return  2;
        else if (code_point <= 0xD7FFu) return 3;
        else if (code_point <= 0xDFFFu or code_point > 0x10FFFFu) return 0;
        else if (code_point <= 0xFFFFu) return 3;
        else return 4;
    }
    
    // Returns the number of UTF-8 code units required to encode an Unicode 
    // code point.
    // Parameter code_point shall be a valid Unicode code point.
    // Returns zero for invalid Unicode.  (code_point > U+10FFFF)
    inline int utf8_encoded_length_unsafe(code_point_t code_point) {
        if (code_point <= 0x7Fu) return  1;
        else if (code_point <= 0x7FFu) return  2;
        else if (code_point <= 0xFFFFu) return 3;
        else if (code_point <= 0x10FFFFu) return 4;
        else return 0;
    }
    
    // Returns the number of trail bytes following 'start', which is the start
    // of a multi byte sequence, or which is a single byte.
    // (note: the length of the UTF-8 sequence can be determined by the start 
    // byte of that sequence only).
    //
    // Parameter 'start' shall be a "single byte" (utf8_single(first)== true) or 
    // a "lead byte" (utf8_is_lead(first)==true)) in terms of a valid Unicode
    // UTF-8 seqeunce. 
    //
    // The number of trail bytes is encoded into the start byte. (However, this
    // is in no way a guarantee that the actual sequence will contain that 
    // number of bytes!)
    // 
    // A well-formed UTF-8 Unicode sequence (Unicode, as of to date) may have 1 
    // to 4 bytes. The number of trail bytes is the number of bytes following a 
    // start byte constituting an Unicode code point. If 'start' is a single byte, 
    // the number of trail bytes is zero. If 'start' is a lead byte, the number
    // of trail bytes is [1 .. 3] (in actual Unicode).
    //
    // Please note that a "single byte" is distinct from a "lead byte".
    // Note also, a valid single byte does not at all indicate a valid sequence
    // even though, the number of following trail bytes match. The sequence as
    // a whole may be invalid, if the resulting code point is not a valid Unicode
    // code point (see description for utf8_is_lead()).
    // 
    // 
    // If parameter first is not a single byte or is not a lead byte, the 
    // result is undefined.
    // 
    // Returns [0 .. 3] (number of trail bytes.)
    // (note: returns 0, if parameter first is a single byte)
    
    // Small Lookup Table:
    //
    // Bit-pattern      returns  #trail    start>>3    is_single  is_lead  is_trail  well-formed
    // b0xxx.xxxx           0        0     b0000.xxxx     true      false     false     true
    // b10xx.xxxx          -1       n.a.   b0001.0xxx     false     false     false     false
    // b110x.xxxx           1        1     b0001.10xx     false     true *)   false     ?
    // b1100.0000 (C0)      1       n.a.   b0001.1000     false     false     false     false
    // b1100.0001 (C1)      1       n.a.   b0001.1000     false     false     false     false
    // b1110.xxxx           2        2     b0001.110x     false     true      false     ?
    // b1111.0xxx           3        3     b0001.1110     false     true      false     ?
    // b1111.0100           3        3     b0001.1110     false     true      false     ?
    // b1111.0101 (F5)      3       n.a.   b0001.1110     false     false     false     false
    // b1111.0110 (F6)      3       n.a.   b0001.1110     false     false     false     false
    // b1111.0111 (F7)      3       n.a.   b0001.1110     false     false     false     false
    // b1111.1xxx          -1       n.a.   b0000.1111     false     false     false     false
    //
    //  *) exception: false for: b1100.0000 (0xC0) and b1100.0001 (0xC1)  (overlong ASCII)
    //
    inline int utf8_num_trails_unsafe(utf8_code_unit first) {
        // assertion: utf8_is_lead(first) == true
        const int E = -1;
        static int8_t const table[32] = {
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            E,E,E,E,E,E,E,E,
            1,1,1,1,2,2,3,E            
        };   
        int result = table[static_cast<uint8_t>(first)>>3];
        // Note: the following assertion is for testing the **caller** code!
        // utf8_num_trails_unsafe() will return an undefined result if this assertion is not met.
        assert(result >= 0 and (utf8_is_single(first) or utf8_is_lead(first)) );
        return result;
    }    
    
    
    // The following utf8_num_trails() functions are safe, that is if parameter
    // 'first' is not a valid lead byte or not a single byte the function will 
    // return -1. Otherwise, it returns 0, 1, 2 or 3.
    inline int utf8_num_trails(utf8_code_unit first) {
#if 1
        const int8_t E = -1;
        static int8_t const table[256] = {
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //   0 ..  31
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  32 ..  63
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  64 ..  95
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  96 .. 127
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 128 .. 159
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 160 .. 191
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };   
        return table[static_cast<uint8_t>(first)]; 
        
#elif 0
        uint8_t ch = static_cast<uint8_t>(first);        
        if (ch < 0x80u)
            return 0;
        const int8_t E = -1;
        static int8_t const table[128] = {
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 128 .. 159
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 160 .. 191
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };   
        return table[static_cast<uint8_t>(first)-0x80u];        

#elif 0
        uint8_t ch = static_cast<uint8_t>(first);        
        if (ch < 0x80u)
            return 0;
        if (ch < 0xC0u)
            return -1;
        const int8_t E = -1;
        static int8_t const table[64] = {
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };   
        return table[static_cast<uint8_t>(first)-0xC0u];   
        
#elif 0     
        const int8_t E = -1;        
        static int8_t const table[32] = {
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            E,E,E,E,E,E,E,E,
            1,1,1,1,2,2,3,E            
        };   
        int result = table[static_cast<uint8_t>(first)>>3];
        if (result > 0 and !utf8_is_lead((static_cast<uint8_t>(first))))
            return -1;
        return result;
                
        
#elif 0
        uint8_t ch = static_cast<uint8_t>(first);
        if (ch < 0x80u)
            return 0;
        if (ch < 0xC2u)
            return -1;
        if (ch < 0xE0u)
            return 1;
        if (ch < 0xF0u)
            return 2;
        if (ch < 0xF5)
            return 3;
        return -1;
        
        
#endif        
        
    }
    
    
    // The following utf8_num_trails_no_ctrl() functions are safe, that is if parameter
    // 'first' is not a valid lead byte or not a single byte the function will 
    // return -1. Otherwise, it returns -2 for an ASCII Control Character, otherwise 
    // it returns 0, 1, 2 or 3.
    inline int utf8_num_trails_no_ctrl(utf8_code_unit first) {
#if 1
        const int8_t E = -1;
        const int8_t C = -2;
        static int8_t const table[256] = {
            C,C,C,C,C,C,C,C,  C,C,C,C,C,C,C,C,  C,C,C,C,C,C,C,C,  C,C,C,C,C,C,C,C,   //   0 ..  31
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  32 ..  63
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  64 ..  95
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  96 .. 127
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 128 .. 159
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 160 .. 191
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };   
        return table[static_cast<uint8_t>(first)]; 
        
#elif 0
        uint8_t ch = static_cast<uint8_t>(first); 
        if (ch < 0x20)
            return -2;
        if (ch < 0x80u)
            return 0;
        const int8_t E = -1;
        const int8_t C = -2;
        static int8_t const table[128] = {
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 128 .. 159
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 160 .. 191
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };   
        return table[static_cast<uint8_t>(first)-0x80u];        
        
#elif 0
        uint8_t ch = static_cast<uint8_t>(first);
        if (ch < 0x20)
            return -2;
        if (ch < 0x80u)
            return 0;
        if (ch < 0xC0u)
            return -1;
        const int8_t E = -1;
        static int8_t const table[64] = {
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };   
        return table[static_cast<uint8_t>(first)-0xC0u];   
        
#elif 0        
        const int8_t E = -1;
        const int8_t C = -2;
        static int8_t const table[32] = {
            C,C,C,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            1,1,1,1,2,2,3,E            
        };   
        int result = table[static_cast<uint8_t>(first)>>3];
        if (result > 0 and !utf8_is_lead((static_cast<uint8_t>(first))))
            return -1;
        return result;
        
        
#elif 0
        uint8_t ch = static_cast<uint8_t>(first);
        if (ch < 0x20)
            return -2;
        if (ch < 0x80u)
            return 0;
        if (ch < 0xC2u)
            return -1;
        if (ch < 0xE0u)
            return 1;
        if (ch < 0xF0u)
            return 2;
        if (ch < 0xF5)
            return 3;
        return -1;
        
        
#endif        
        
    }
        
    
    
}}  // namespace json::internal
    
    
#pragma mark -
#pragma mark UTF-16
namespace json { namespace unicode {
    
    
    typedef json::unicode::UTF_16_encoding_tag::code_unit_type utf16_code_unit;
    
    BOOST_STATIC_ASSERT((boost::is_unsigned<utf16_code_unit>::value == true));
    
    
    
    inline bool utf16_is_high_surrogate(utf16_code_unit code_unit) {
        return isHighSurrogate(code_point_t(code_unit));  
    }
    inline bool utf16_is_lead(utf16_code_unit code_unit) {
        return isHighSurrogate(code_point_t(code_unit));  
    }

    inline bool utf16_is_low_surrogate(utf16_code_unit code_unit) {
        return isLowSurrogate(code_point_t(code_unit));
    }
    inline bool utf16_is_trail(utf16_code_unit code_unit) {
        return isLowSurrogate(code_point_t(code_unit));
    }

    // Returns true if the specified code unit is a surrogate (U+D800..U+DFFF)
    inline bool utf16_is_surrogate(utf16_code_unit cu) {
        return (cu & 0xFFFFF800u) == 0xD800u;
    }
    
    
    
    // Returns true if the specified single code unit encodes a 
    // code point (e.g. a BMP).
    inline bool utf16_is_single(utf16_code_unit cu) {
        // return true if the code unit is not a surrogate (U+D800..U+DFFF)
        return not((cu & 0xFFFFF800u) == 0xD800u);  
    }
    
    /*
    // Returns true if the specified code unit is a lead surrogate (U+D800..U+DBFF).
    inline bool utf16_is_lead(utf16_code_unit cu) {        
        return (cu & 0xFFFFFC00u) == 0xD800u;
    }
    
    // Returns true if the specified code unit is a trail surrogate (U+DC00..U+DFFF).
    inline bool utf16_is_trail(utf16_code_unit cu) {
        return (cu & 0xFFFFFC00u) == 0xDC00u;
    }
     */
    
    // Returns the number of UTF-16 code units (1 or 2) required to encode the 
    // unicode code point into UTF-16.
    // Returns zero for invalid Unicode.  (code_point > U+10FFFF)
    inline int utf16_encoded_length_unsafe(code_point_t code_point) {
        if (code_point <= 0xFFFFu) 
            return  1;
        else if (code_point <= 0x10FFFF)
            return  2;
        else 
            return 0;
    }
    
    // Returns the number of UTF-16 code units (1 or 2) required to encode the 
    // unicode code point into UTF-16.
    // For surrogates and invalid Unicode code points, it returns 0.
    inline int utf16_encoded_length(code_point_t code_point) {
        if (isSurrogate(code_point))
            return 0;
        if (code_point <= 0xFFFFu) 
            return  1;
        else if (code_point <= 0x10FFFF)
            return  2;
        else 
            return 0;
    }
    
    
    // Convert a valid surrogate pair into a code point and return the result.
    // If either surrogate is invalid, the result is undefined.
    inline code_point_t 
    utf16_surrogate_pair_to_code_point(utf16_code_unit high, utf16_code_unit low) {
        return (high - 0xD800u) * 0x400u + (low - 0xDC00u) + 0x10000u;
    }
    
    
    
    // Get the lead surrogate for a supplementary code point.
    // code_point must be a valid supplementary Unicode Code Point (U+10000..U+10FFFF).
    // Returns the lead surrogate (U+D800 .. U+DBFF) for the specified code
    // point.
    inline utf16_code_unit
    utf16_get_lead(code_point_t code_point) { 
        return ((code_point >> 10) + 0xD7C0u); 
    }
    
    
    // Get the trail surrogate for a supplementary code point.
    // code_point must be a valid suplementary Unicode Code Point (U+10000..U+10FFFF).
    // Returns the trail surrogate for the specified supplementary code point.
    inline utf16_code_unit
    utf16_get_trail(code_point_t code_point) { 
        return ((code_point & 0x3FFu) | 0xDC00u); 
    }


}}  // namespace json::internal


    
#pragma mark -
#pragma mark UTF-32
namespace json { namespace unicode {
    
    typedef UTF_32_encoding_tag::code_unit_type utf32_code_unit;
    
    BOOST_STATIC_ASSERT((boost::is_unsigned<utf32_code_unit>::value == true));
    BOOST_STATIC_ASSERT((boost::is_same<utf32_code_unit, code_point_t>::value == true));
    
    const utf32_code_unit UTF32_BOM = 0xFeFFu;
    
}}  // namespace json::unicode


#endif // JSON_UNICODE_UTILITIES_HPP

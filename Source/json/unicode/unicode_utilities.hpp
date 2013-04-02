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

#ifndef JSON_UNICODE_UNICODE_UTILITIES_HPP
#define JSON_UNICODE_UNICODE_UTILITIES_HPP

#include "json/config.hpp"
#include "json/endian/endian.hpp"
#include "json/endian/byte_swap.hpp"
#include <type_traits>
#include <cstdint>
#include <cassert>

//  
//  A set of types and utility functions supporting Unicode
//  according Unicode Standard Version 6.0
// 
//  For specifications, see 
//  http://www.unicode.org
//  http://en.wikipedia.org/wiki/UTF-8
//  RFC 3629, http://tools.ietf.org/html/rfc3629


//
// Unicode Utilities
//
namespace json { namespace unicode {
    

    //
    //  Constants for Unicode Encodings
    //
    enum UNICODE_ENCODING {
        UNICODE_ENCODING_UTF_8 =    1,
        UNICODE_ENCODING_UTF_16BE = 2,
        UNICODE_ENCODING_UTF_16LE = 3,
        UNICODE_ENCODING_UTF_32BE = 4,
        UNICODE_ENCODING_UTF_32LE = 5
    };
    

        
    // code_point_t corresponds to an Unicode code point in the Unicode code-
    // space: a range of (unsigned) integers from 0 to 0x10FFFF.
    // (Do not mismatch this type with the UTF-32 "Code Unit" which is as well
    // an unsigend 32-bit integer).
    typedef uint32_t code_point_t; 
    static_assert((std::is_unsigned<code_point_t>::value == true), "");
    
    
    
    static constexpr code_point_t kReplacementCharacter = 0xFFFD;
    static constexpr code_point_t kUnicodeCodeSpaceMax = 0x10FFFFu;
    
    //const char * const kInvalidCharacterUTF8 = "\xef\xbf\xbd"; // "�"
    //size_t kInvalidCharacterUTF8Length = 3;
        
    
    // Stable. Do not change.
    static_assert((std::is_unsigned<code_point_t>::value == true), "");

    
    // Returns true if the code point is within the range of the
    // Unicode code space [0 .. 0x10FFFF].
    inline constexpr bool isCodePoint(code_point_t code_point) {
        return code_point <= 0x10FFFFu;
    }
    
    // Usually, surrogate values are expressed in pairs of 16-bit code units,
    // where the high surrogate comes always first in the stream.
    //    
    // D71: Returns true if the Unicode code point is a high-surrogate code point,
    // an Unicode code point in the range U+D800 to U+DBFF.
    inline constexpr bool isHighSurrogate(code_point_t code_point) {
        //return (code_point >= 0xD800u and code_point <= 0xDBFFu);  
        return (code_point - 0xD800u) <= (0xDBFFu - 0xD800u);
    }
    // D73: Returns true if the Unicode code point is a Low-surrogate code point,
    // an Unicode code point in the range U+DC00 to U+DFFF.
    inline constexpr bool isLowSurrogate(code_point_t code_point) {
        //return code_point >= 0xDC00u and code_point <= 0xDFFFu;
        return (code_point - 0xDC00u) <= (0xDFFFu - 0xDC00u);
    }
    
    // Returns true if the Unicode code point is either a high-surrogate or
    // a low surrogate.
    inline constexpr bool isSurrogate(code_point_t code_point) {
        //return code_point >= 0xD800u and code_point <= 0xDFFFu;
        return (code_point - 0xD800u) <= (0xDFFFu - 0xD800u);
    }
    

    // A "Noncharacter" is any valid code point that is permanently reserved 
    // for internal use and that should never be interchanged. 
    // Returns true if this is a valid code point and if this is a noncharacter, 
    // otherwise, returns false.
    // see http://unicode.org/reports/tr20/#Noncharacters    
    inline constexpr bool isNonCharacter(code_point_t code_point) {
        return
                (((0x00FFFFu&code_point) >= 0xFFFEu) 
                 or (code_point >= 0xFDD0u and code_point <= 0xFDEFu)
                )
            and code_point <= 0x10FFFFu;
    }
    
    
    // Returns true if this is a valid Unicode scalar value (that is, any 
    // Unicode "code point" excluding high and low surrogates). A valid 
    // value implies a valid Unicode code point.
    // D76
    inline constexpr bool isUnicodeScalarValue(code_point_t code_point) {
        return code_point < 0xD800u or ((code_point - 0xE000u) <= (0x10FFFFu - 0xE000u));
        //bool result = not isSurrogate(code_point) and code_point <= 0x10FFFFu;
    }
    
    
    // Returns true if the code point is an Unicode character (that is, 
    // an Unicode scalar value excluding noncharacters). 
    inline constexpr bool isUnicodeCharacter(code_point_t code_point) {
        return isUnicodeScalarValue(code_point) and not isNonCharacter(code_point);
    }
    
    
    // Returns true if the given code point is defined specifically as
    // a control code. A control point implies, that it is also a valid 
    // code point.
    inline constexpr bool inline isControlCode(code_point_t code_point) {
        return (code_point <= 0x001Fu)
            or (code_point >= 0x007Fu and code_point <= 0x009Fu);
    }

    
    
    
}}  // namespace json::internal
    
    
#pragma mark -
#pragma mark UTF-8
namespace json { namespace unicode {
    
    
    //
    // Implementation notes:
    // The algorithm must work with either signed or unsigned UTF-8 code units!
    
    
    // Returns true if the code unit can be encoded into a single byte. That is,
    // in this case the code unit's value is within ASCII [0 .. 0x7F] and can 
    // thus be interpreted directly as ASCII.
    //
    inline constexpr bool utf8_is_single(uint8_t cu) {
        return !(static_cast<uint8_t>(cu) & 0x80u);
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
    inline constexpr bool utf8_is_lead(uint8_t cu) {
        // Do not remove any explicit typecasts or any "unsigned" modifiers 
        // applied to variables or constants - unless you really know what 
        // you are doing!!
        return (static_cast<uint8_t>(cu) - 0xC2u) < 0x33u;
    }
    
    // Returns true if the given code unit is a trail byte, e.g. a byte with
    // the bit pattern: b10xx.xxxx
    inline constexpr bool utf8_is_trail(uint8_t cu) {
        return (static_cast<uint8_t>(cu) & 0xC0u) == 0x80u;
    }
    
    // Returns the number of UTF-8 code units required to encode an Unicode 
    // code point.
    // Returns zero if parameter `code_point` is an Unicode surrogate or not a 
    // valid Unicode code point.
    // (A well-formed UTF-8 sequence shall not contain surrogates)
    // Parameter code_point: An Unicode code point.
    inline constexpr int utf8_encoded_length(code_point_t code_point) {
        return ( (code_point & ~0x007Fu) == 0) ? 1 :
        (code_point <= 0x7FFu) ? 2 : 
        (code_point <= 0xD7FFu) ? 3:
        (code_point <= 0xDFFFu or code_point > 0x10FFFFu) ? 0 :
        (code_point <= 0xFFFFu) ? 3 :
        4;
    }
    
    // Returns the number of UTF-8 code units required to encode an Unicode 
    // code point.
    // Parameter code_point shall be a valid Unicode code point.
    // Returns zero for invalid Unicode.  (code_point > U+10FFFF)
    inline constexpr int utf8_encoded_length_unsafe(code_point_t code_point) {
        return (code_point <= 0x7Fu) ? 1 :
        (code_point <= 0x7FFu) ? 2 :
        (code_point <= 0xFFFFu) ? 3 :
        (code_point <= 0x10FFFFu) ? 4 :
        0;
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
    static constexpr const int __utf8_num_trails_unsafe_table[32] =
    {
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
        -1,-1,-1,-1,-1,-1,-1,-1,
         1, 1, 1, 1, 2, 2, 3,-1
    };
    inline constexpr int utf8_num_trails_unsafe(uint8_t first) {
        // assertion: utf8_is_lead(first) == true
        // Note: the following assertion is for testing the **caller** code!
        //assert(__utf8_num_trails_unsafe_table[static_cast<uint8_t>(first)>>3] >= 0 and (utf8_is_single(first) or utf8_is_lead(first)) );
        return __utf8_num_trails_unsafe_table[static_cast<uint8_t>(first)>>3];
        // utf8_num_trails_unsafe() will return an undefined result if this assertion is not met.
    }
    
    
    // The following utf8_num_trails() functions are safe, that is if parameter
    // 'first' is not a valid lead byte or not a single byte the function will 
    // return -1. Otherwise, it returns 0, 1, 2 or 3.
    namespace detail {
        static constexpr int8_t E{-1};
    }
#if 1
    namespace detail {
        static constexpr int8_t const __utf8_num_trails_table[256] = {
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //   0 ..  31
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  32 ..  63
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  64 ..  95
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  96 .. 127
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 128 .. 159
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 160 .. 191
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };
    }
    
    inline constexpr int utf8_num_trails(uint8_t first) {
        return detail::__utf8_num_trails_table[static_cast<uint8_t>(first)];
    }
#elif 0
    namespace detail {
    
        static constexpr int8_t const __utf8_num_trails_table[128] = {
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 128 .. 159
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 160 .. 191
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };
    }
    
    inline int utf8_num_trails(uint8_t first) {
        return (utf8_is_single(static_cast<uint8_t>(first))) ? 0 :
        detail::__utf8_num_trails_table[static_cast<uint8_t>(first)-0x80u];
    }
#endif        
    
    
    // The following utf8_num_trails_no_ctrl() functions are safe, that is if parameter
    // 'first' is not a valid lead byte or not a single byte the function will 
    // return -1. Otherwise, it returns -2 for an ASCII Control Character, otherwise 
    // it returns 0, 1, 2 or 3.
    namespace detail {
        static constexpr int8_t C{-2};
        static constexpr int8_t const __utf8_num_trails_no_ctrl_table[256] = {
            C,C,C,C,C,C,C,C,  C,C,C,C,C,C,C,C,  C,C,C,C,C,C,C,C,  C,C,C,C,C,C,C,C,   //   0 ..  31
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  32 ..  63
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  64 ..  95
            0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,   //  96 .. 127
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 128 .. 159
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   // 160 .. 191
            E,E,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,  1,1,1,1,1,1,1,1,   // 192 .. 223
            2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,  3,3,3,3,3,E,E,E,  E,E,E,E,E,E,E,E    // 224 .. 255
        };
    }
    
    inline constexpr int utf8_num_trails_no_ctrl(uint8_t first) {
        return detail::__utf8_num_trails_no_ctrl_table[static_cast<uint8_t>(first)];        
    }
        
    
    
}}  // namespace json::internal
    
    
#pragma mark -
#pragma mark UTF-16
namespace json { namespace unicode {
    
    
    inline constexpr bool utf16_is_high_surrogate(uint16_t code_unit) {
        return isHighSurrogate(code_point_t(code_unit));
    }
    inline constexpr bool utf16_is_lead(uint16_t code_unit) {
        return isHighSurrogate(code_point_t(code_unit));  
    }

    inline constexpr bool utf16_is_low_surrogate(uint16_t code_unit) {
        return isLowSurrogate(code_point_t(code_unit));
    }
    inline constexpr bool utf16_is_trail(uint16_t code_unit) {
        return isLowSurrogate(code_point_t(code_unit));
    }

    // Returns true if the specified code unit is a surrogate (U+D800..U+DFFF)
    inline constexpr bool utf16_is_surrogate(uint16_t cu) {
        return (cu & 0xFFFFF800u) == 0xD800u;
    }
    
    
    
    // Returns true if the specified single code unit encodes a 
    // code point (e.g. a BMP).
    inline constexpr bool utf16_is_single(uint16_t cu) {
        // return true if the code unit is not a surrogate (U+D800..U+DFFF)
        return not((cu & 0xFFFFF800u) == 0xD800u);  
    }
    
    /*
    // Returns true if the specified code unit is a lead surrogate (U+D800..U+DBFF).
    inline bool utf16_is_lead(uint16_t cu) {        
        return (cu & 0xFFFFFC00u) == 0xD800u;
    }
    
    // Returns true if the specified code unit is a trail surrogate (U+DC00..U+DFFF).
    inline bool utf16_is_trail(uint16_t cu) {
        return (cu & 0xFFFFFC00u) == 0xDC00u;
    }
     */
    
    // Returns the number of UTF-16 code units (1 or 2) required to encode the 
    // unicode code point into UTF-16.
    // Returns zero for invalid Unicode.  (code_point > U+10FFFF)
    inline constexpr int utf16_encoded_length_unsafe(code_point_t code_point) {
        return (code_point <= 0xFFFFu) ? 1 :
        (code_point <= 0x10FFFFu) ? 2 :
        0;
    }
    
    // Returns the number of UTF-16 code units (1 or 2) required to encode the 
    // unicode code point into UTF-16.
    // For surrogates and invalid Unicode code points, it returns 0.
    inline constexpr int utf16_encoded_length(code_point_t code_point) {
        return (isSurrogate(code_point)) ? 0 :
        (code_point <= 0xFFFFu) ? 1 :
        (code_point <= 0x10FFFFu) ? 2 :
        0;
    }
    
    
    // Convert a valid surrogate pair into a code point and return the result.
    // If either surrogate is invalid, the result is undefined.
    inline constexpr code_point_t
    utf16_surrogate_pair_to_code_point(uint16_t high, uint16_t low) {
        //return (high - 0xD800u) * 0x400u + (low - 0xDC00u) + 0x10000u;
        return ((high - 0xD800u) << 10) + (low - 0xDC00u) + 0x10000u;
    }
    
    
    
    // Get the lead surrogate for a supplementary code point.
    // code_point must be a valid supplementary Unicode Code Point (U+10000..U+10FFFF).
    // Returns the lead surrogate (U+D800 .. U+DBFF) for the specified code
    // point.
    inline constexpr uint16_t
    utf16_get_lead(code_point_t code_point) { 
        return ((code_point >> 10) + 0xD7C0u); 
    }
    
    
    // Get the trail surrogate for a supplementary code point.
    // code_point must be a valid suplementary Unicode Code Point (U+10000..U+10FFFF).
    // Returns the trail surrogate for the specified supplementary code point.
    inline constexpr uint16_t
    utf16_get_trail(code_point_t code_point) { 
        return ((code_point & 0x3FFu) | 0xDC00u); 
    }


}}  // namespace json::internal


    
#pragma mark -
#pragma mark UTF-32
namespace json { namespace unicode {
    
    
    constexpr uint32_t UTF32_BOM = 0xFeFFu;
    
}}  // namespace json::unicode


#endif // JSON_UNICODE_UNICODE_UTILITIES_HPP

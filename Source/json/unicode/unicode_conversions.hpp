//
//  unicode_conversions.hpp
//  
//
//  Created by Andreas Grosam on 7/26/11.
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

#ifndef JSON_UNICODE_UNICODE_CONVERSIONS_HPP
#define JSON_UNICODE_UNICODE_CONVERSIONS_HPP


#include "json/config.hpp"
#include "unicode_utilities.hpp"
#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>


// If defined, conversion functions check for valid Unicode.
// Otherwise, the functions assume wellformed Unicode and omit
// test code which verifies this.
// 



#pragma mark - Error Codes
namespace json { namespace unicode { 
    
    enum ErrorT {
        NO_ERROR = 0,
        E_TRAIL_EXPECTED =          -1,  /* Trail byte expected. */ // ill-formed
        E_INVALID_START_BYTE =      -2,  /* Invalid start byte */  // ill-formed
        E_UNCONVERTABLE_OFFSET =    -3,  /* Unconvertable offset */  // ill-formed (conversion would yield invalid code point)
        E_INVALID_CODE_POINT =      -4,  /* An Unicode code point is invalid, e.g. not a Unicode scalar value */
        E_INVALID_UNICODE =         -5,  /* An Unicode code point is invalid, e.g. not a Unicode scalar value */
        /* the given context (e.g. a Surrogate)*/
        E_NO_CHARACTER =            -6,  /* An Unicode code point is not an Unicode */
                                         /* scalar value or it is a noncharacter. */
        E_NONCHARACTER =            -7,  /* Detected Unicode noncharacter */
        E_PREDICATE_FAILED =        -8,  /* The filter predicate detected invalid character and did not apply replacement */
        E_UNEXPECTED_ENDOFINPUT = -100,  /* Unexpected and of input. */
        E_UNKNOWN_ERROR =        -1000
    };
    
}}    


#pragma mark - Conversion Filter
namespace json { namespace unicode { namespace filter {
    
    
    struct filter_tag {};
    
    struct None : filter_tag
    {        
        bool operator() (code_point_t) const { return false; }
        bool replace() const { return false; }
        code_point_t replacement(code_point_t cp) const { return cp; }
    };

    struct Noncharacter : filter_tag
    {        
        Noncharacter(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            return isNonCharacter(cp); 
        }
        
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
    private:
        code_point_t replacement_;
    };
        
    struct Surrogate : filter_tag
    {        
        Surrogate(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            return isSurrogate(cp); 
        }
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
        
    private:
        code_point_t replacement_;
    };
            
    struct SurrogateOrNoncharacter : filter_tag
    {        
        SurrogateOrNoncharacter(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            return isSurrogate(cp) or isNonCharacter(cp); 
        }
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
        
    private:
        code_point_t replacement_;
    };
    
    
    struct NoncharacterOrNULL : filter_tag
    {        
        NoncharacterOrNULL(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            bool result = cp == 0 or isNonCharacter(cp); 
            return result;
        }
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
        
        void replacement_character(code_point_t replacement) {
            replacement_ = replacement;
        }
        
        code_point_t replacement_character() const {
            return replacement_;
        }
        
    private:
        code_point_t replacement_;
    };
    
    
    
}}}




#pragma mark - Internal Base Templates  convert_one_  
namespace json {  namespace unicode { namespace internal {
    
    template <typename IteratorT, typename EncodingT, class Enable = void>
    struct convert_codepoint_unsafe {        
        BOOST_STATIC_ASSERT_MSG( (boost::is_same<void,Enable>::value), "base template not instantiable");        
    };
    
    
    template <typename IteratorT, typename EncodingT, class Enable = void>
    struct convert_codepoint  {
        BOOST_STATIC_ASSERT_MSG((boost::is_same<void,Enable>::value), "base template not instantiable");        
    };
    
    
}}}    


#pragma mark - I Internal Template Specializations  convert_codepoint_  UTF to Unicode codepoint / Unicode codepoint to UTF
namespace json {  namespace unicode { namespace internal {
    
    // Class Template Specializations
    
    using namespace json::unicode;
    
#pragma mark I 1.a  Struct convert_codepoint_unsafe <UTF-8>
    //
    // struct convert_codepoint_unsafe    UTF-8 -> code point -> UTF-8
    //
    template <typename IteratorT, typename EncodingT>
    struct convert_codepoint_unsafe<IteratorT, EncodingT,
        typename boost::enable_if<
            boost::is_same<UTF_8_encoding_tag, EncodingT>
        >::type
    >
    {
        // Cant't check size of an output iterator!
        //        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
        //                            == sizeof(typename EncodingT::code_unit_type));
        

    private:
        
#if 0 // unused        
        inline int read_trails1(uint8_t ch1, IteratorT& first, IteratorT last, 
                         code_point_t&   code_point) const 
        {
            code_point_t ch2 = static_cast<utf8_code_unit>(*++first);
            code_point = ((ch1 << 6) & 0x7FFu) + ((ch2) & 0x3Fu);
            ++first;
            return 1;
        }
        
        inline int read_trails2(uint8_t ch1, IteratorT& first, IteratorT last, 
                         code_point_t&   code_point) const 
        {
            code_point_t ch2 = static_cast<utf8_code_unit>(*++first);
            code_point_t cp = ((ch1 << 12) & 0xFFFFu) + ((ch2 << 6) & 0xFFFu);
            code_point_t ch3 = static_cast<utf8_code_unit>(*++first);
            cp += ch3 & 0x3Fu;
            code_point = cp;
            ++first;
            return 1;
        }
        
        inline int read_trails3(uint8_t ch1, IteratorT& first, IteratorT last, 
                         code_point_t&   code_point) const 
        {
            code_point_t ch2 = static_cast<utf8_code_unit>(*++first);
            code_point_t cp = ((ch1 << 18) & 0x1FFFFFu) + ((ch2 << 12) & 0x3FFFFu);                
            code_point_t ch3 = static_cast<utf8_code_unit>(*++first);
            cp += (ch3 << 6) & 0xFFFu;
            code_point_t ch4 = static_cast<utf8_code_unit>(*++first);
            cp += ch4 & 0x3Fu; 
            code_point = cp;
            ++first;
            return 1;
        }
        
        // Parameter 'code_point*' is is assigned ch0
        inline int read_trails(IteratorT& first, IteratorT last, code_point_t& code_point) const
        {
            code_point_t ch1 = static_cast<utf8_code_unit>(*first++);
            switch (utf8_num_trails(static_cast<utf8_code_unit>(code_point))) {
                default: {
                    code_point = ((code_point << 6) & 0x7FFu) + ((ch1) & 0x3Fu);
                    return 1;
                    break;
                }
                case 2: {
                    code_point_t cp = ((code_point << 12) & 0xFFFFu) + ((ch1 << 6) & 0xFFFu);
                    code_point_t ch2 = static_cast<utf8_code_unit>(*first++);
                    cp += ch2 & 0x3Fu;
                    code_point = cp;
                    return 1;
                    break;
                }
                case 3: {
                    code_point_t cp = ((code_point << 18) & 0x1FFFFFu) + ((ch1 << 12) & 0x3FFFFu);                
                    code_point_t ch2 = static_cast<utf8_code_unit>(*first++);
                    cp += (ch2 << 6) & 0xFFFu;
                    code_point_t ch3 = static_cast<utf8_code_unit>(*first++);
                    cp += ch3 & 0x3Fu; 
                    code_point = cp;
                    return 1;
                    break;
                }
            }
        }
#endif        
        
        
    public:        
        
#pragma mark UTF-8 to Codepoint
        // Convert the first character contained in the well-formed UTF-8 sequence
        // starting at first to one Unicode code point. Parameter first must point 
        // to the start of a UTF-8 multi byte sequence.
        // On exit first has been advanced by the number of bytes constituting the
        // character.
        // Returns the number of generated code points [1].
        //
        // The result and behavior is undefined if the input sequence is not a 
        // well-formed Unicode.
        // 
        // Unsafe version:
        //  - does not check for iterating past last
        //  - does not check for valid number of trail bytes.
        //  - does not check if first is a valid start byte for UTF-8
        //  - does not check for noncharacters
        //
        //
        // Implementation notes:
        // The algorithm must work with either signed or unsigned UTF-8 code units.
        int operator()(
            IteratorT&       first, 
            IteratorT        last, 
            code_point_t&   code_point) const
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));
            
            assert(first != last);

            code_point = static_cast<uint8_t>(*first++);
            int num_trails = utf8_num_trails_unsafe(code_point);
            assert(num_trails <= 3 and num_trails >= 0);
            if (num_trails == 0) {
                return 1;
            }
            
            assert(first != last);
            code_point_t ch_next = static_cast<uint8_t>(*first++);
            code_point_t cp = code_point;
            switch (num_trails) {
                case 1:
                    cp = ((cp << 6) & 0x7FFu) + ((ch_next) & 0x3Fu);
                    code_point = cp;
                    return 1;
                case 2:
                    cp = ((cp << 12) & 0xFFFFu) + ((ch_next << 6) & 0xFFFu);
                    assert(first != last);
                    ch_next = static_cast<utf8_code_unit>(*first++);
                    cp += ch_next & 0x3Fu;
                    code_point = cp;
                    return 1;
                case 3:
                    cp = ((cp << 18) & 0x1FFFFFu) + ((ch_next << 12) & 0x3FFFFu);                
                    assert(first != last);
                    ch_next = static_cast<utf8_code_unit>(*first++);
                    cp += (ch_next << 6) & 0xFFFu;
                    assert(first != last);
                    ch_next = static_cast<utf8_code_unit>(*first++);
                    cp += ch_next & 0x3Fu; 
                    code_point = cp;
                    return 1;
            }
            
            assert(first == last);
            return 1;
        }
        
#pragma mark Codepoint to UTF-8 
        // Convert one Unicode code point using UTF-8 encoding and copy the 
        // result into the sequence starting at dest.    
        // The output sequence must be capable to hold the number of generated
        // code units.
        //
        // Returns the number of generated code unites.
        //
        // The function omits several checks ensuring valid input parameters. 
        // Invalid Unicode code points (>0+10FFFF) will not be converted and
        // the result equals E_INVALID_CODE_POINT.
        //
        // All other code points (surrogates, noncharacters, etc.) will be con-
        // verted. Note that Sorrugates are not allowed in UTF-8 sequences.
        //
        // Errors:
        //  E_INVALID_CODE_POINT:  The code point is not in the range of valid
        //                         Unicode.
        //
        inline int 
        operator() (    
            code_point_t    code_point,
            IteratorT&      dest) const
        {            
            typedef typename EncodingT::code_unit_type  code_unit_t;                
            
            // encode the Unicode code point to a UTF-8 byte sequence
            
            // note: utf8_encoded_length_unsafe will return zero, if the code 
            // point is not a valid Unicode point (> 0x10FFFF).
            int len = utf8_encoded_length_unsafe(code_point);
            
            switch (len) {
                case 1:
                    *dest++ = static_cast<code_unit_t>(code_point);
                    break;
                case 2:
                    *dest++ = static_cast<code_unit_t>(((code_point >> 6) | 0xC0u));
                    *dest++ = static_cast<code_unit_t>(((code_point & 0x3Fu) | 0x80u));
                    break;
                case 3: 
                    *dest++ = static_cast<code_unit_t>(((code_point >> 12) | 0xE0u));
                    *dest++ = static_cast<code_unit_t>((((code_point >> 6) & 0x3Fu) | 0x80u));
                    *dest++ = static_cast<code_unit_t>(((code_point & 0x3Fu) | 0x80u));
                    break;
                case 4:
                    *dest++ = static_cast<code_unit_t>(((code_point >> 18) | 0xF0u));
                    *dest++ = static_cast<code_unit_t>((((code_point >> 12) & 0x3Fu) | 0x80u));
                    *dest++ = static_cast<code_unit_t>((((code_point >> 6) & 0x3Fu) | 0x80u));
                    *dest++ = static_cast<code_unit_t>(((code_point & 0x3Fu) | 0x80u));
                    break;
                default: 
                    return E_INVALID_CODE_POINT; // code point is not a valid Unicode code point.
            }
            
            return len;        
        }
        
        
    };
    

#pragma mark I 1.b Struct convert_codepoint <UTF-8>
    //
    // struct convert_codepoint           UTF-8 -> code point
    //
    template <typename IteratorT, typename EncodingT>
    struct convert_codepoint<IteratorT, EncodingT,
        typename boost::enable_if<
            boost::is_same<UTF_8_encoding_tag, EncodingT>
        >::type
    >
    {
        // Can't check size of an output iterator!
        //        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
        //                            == sizeof(typename EncodingT::code_unit_type));
        
        
    private:
        
#if 0 /* not used */        
        int read_trails3(uint8_t ch0, IteratorT& first, IteratorT last, 
                         code_point_t&   code_point) const 
        {
            if (first == last) {
                return E_UNEXPECTED_ENDOFINPUT;
            }
            uint8_t ch1 = static_cast<uint8_t>(*first);
            if (not utf8_is_trail(ch1)) {
                return E_TRAIL_EXPECTED;
            }
            if (ch0 == 0xF0u) {
                if (ch1 < 0x90u) {
                    return E_UNCONVERTABLE_OFFSET;
                }
            }
            else if (ch0 == 0xF4 and ch1 > 0x8Fu) {
                return E_UNCONVERTABLE_OFFSET;
            }
            ++first;                            
            code_point_t result = ((ch0 << 18) & 0x1FFFFFu) + ((ch1 << 12) & 0x3FFFFu);                        
            
            if (first == last) {
                return E_UNEXPECTED_ENDOFINPUT;
            }
            uint8_t ch2 = static_cast<uint8_t>(*first);
            if (not utf8_is_trail(ch2)) {
                return E_TRAIL_EXPECTED;
            }
            ++first;
            result += (ch2 << 6) & 0xFFFu;
            
            if (first == last) {
                return E_UNEXPECTED_ENDOFINPUT;
            }
            uint8_t ch3 = static_cast<utf8_code_unit>(*first);
            if (not utf8_is_trail(ch3)) {
                return E_TRAIL_EXPECTED;
            }
            ++first;
            result += ch3 & 0x3Fu; 
            code_point = result;
            return 1;
        }
                
        
        int read_trails2(uint8_t ch0, IteratorT& first, IteratorT last, 
                        code_point_t&   code_point) const 
        {
            if (first == last) {
                return E_UNEXPECTED_ENDOFINPUT;
            }
            uint8_t ch1 = static_cast<uint8_t>(*first);
            if (not utf8_is_trail(ch1)) {
                return E_TRAIL_EXPECTED;
            }
            
            if (ch0 == 0xE0u) {
                if (ch1 < 0xA0u)
                    return E_UNCONVERTABLE_OFFSET;
            }
            else if (ch0 == 0xEDu) {
                if (ch1 > 0x9F) {
                    return E_UNCONVERTABLE_OFFSET;
                }
            }
            code_point_t result = ((ch0 << 12) & 0xFFFFu) + ((ch1 << 6) & 0xFFFu);
            ++first;
            
            if (first == last) {
                return E_UNEXPECTED_ENDOFINPUT;
            }
            uint8_t ch2 = static_cast<uint8_t>(*first);
            if (not utf8_is_trail(ch2)) {
                return E_TRAIL_EXPECTED;
            }
            ++first;
            result += ch2 & 0x3Fu;
            code_point = result;
            return 1;
        }
#endif        
        
        
        // parameter 'code_point' is initialized with ch0
        // parameter 'first' points past the first ch
        inline int read_trails(int32_t num_trails, IteratorT& first, IteratorT last, 
                               code_point_t&   code_point) const 
        {
            if (first == last) {
                return E_UNEXPECTED_ENDOFINPUT;
            }            
            const uint32_t ch1 = static_cast<uint8_t>(*first);
            if (not utf8_is_trail(ch1)) {
                return E_TRAIL_EXPECTED;
            }            
            
            // determine the length of the mb sequence and read n trail bytes 
            //const int num_trails = utf8_num_trails(ch0);
            switch (num_trails)               
            {
                case 1: {
                    // there are no other restrictions for ch1, so
                    code_point = ((code_point << 6) & 0x7FFu) + (ch1 & 0x3Fu); 
                    ++first;  // consume it.
                    return 1;
                    break;
                }
                case 2: {
                    if (code_point == 0xE0u) {
                        if (ch1 < 0xA0u)
                            return E_UNCONVERTABLE_OFFSET;
                    }
                    else if (code_point == 0xEDu) {
                        if (ch1 > 0x9F) {
                            return E_UNCONVERTABLE_OFFSET;
                        }
                    }
                    ++first;
                    code_point = ((code_point << 12) & 0xFFFFu) + ((ch1 << 6) & 0xFFFu);
                    
                    if (first == last) {
                        return E_UNEXPECTED_ENDOFINPUT;
                    }
                    const uint32_t ch2 = static_cast<uint8_t>(*first);
                    if (not utf8_is_trail(ch2)) {
                        return E_TRAIL_EXPECTED;
                    }

                    code_point += ch2 & 0x3Fu;
                    ++first;
                    return 1;
                    break;
                }
                case 3: {
                    if (code_point == 0xF0u) {
                        if (ch1 < 0x90u) {
                            return E_UNCONVERTABLE_OFFSET;
                        }
                    }
                    else if (code_point == 0xF4 and ch1 > 0x8Fu) {
                        return E_UNCONVERTABLE_OFFSET;
                    }
                    
                    ++first;                            
                    code_point = ((code_point << 18) & 0x1FFFFFu) + ((ch1 << 12) & 0x3FFFFu);                        
                    
                    if (first == last) {
                        return E_UNEXPECTED_ENDOFINPUT;
                    }
                    const uint32_t ch2 = static_cast<uint8_t>(*first);
                    if (not utf8_is_trail(ch2)) {
                        return E_TRAIL_EXPECTED;
                    }
                    ++first;
                    code_point += (ch2 << 6) & 0xFFFu;
                    
                    if (first == last) {
                        return E_UNEXPECTED_ENDOFINPUT;
                    }
                    const uint32_t ch3 = static_cast<uint8_t>(*first);
                    if (not utf8_is_trail(ch3)) {
                        return E_TRAIL_EXPECTED;
                    }
                                        
                    code_point += ch3 & 0x3Fu; 
                    ++first;
                    return 1;
                    break;
                }
                default:
                    return E_UNKNOWN_ERROR;
            }
        }
        
        
        
        
    public:
        
#pragma mark UTF-8 to Codepoint
        // Convert the first character contained in the possibly mal-formed UTF-8 
        // sequence starting at first to one Unicode code point. Parameter first 
        // should point to the start of a UTF-8 multi byte sequence.
        // On exit first has been advanced by the number of bytes constituting the
        // character, or in case of an error, to the point where the error occured.
        // Returns the number of generated code points [1, 0], or a negative number
        // indicating an error.
        //
        // Does not fail if the resulting code point is an Unicode noncharacter.
        // 
        // Errors:
        //      E_TRAIL_EXPECTED:           trail byte expected
        //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
        //      E_INVALID_START_BYTE:       invalid start byte
        //      E_INVALID_CODE_POINT:       detected surrogate or code point is invalid Unicode 
        //     
        // Implementation notes:
        // The algorithm must work with either signed or usigned UTF-8 code units.
        //
        // According the "Constraints on Conversion process" in the Unicode
        // Standard v6, this algorithm conforms to the concept of "maximal sub-
        // part".
        
        inline int operator() (
            IteratorT&      first, 
            IteratorT       last, 
            code_point_t&   code_point) const
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));
            
            assert(first != last);            

            code_point = static_cast<uint8_t>(*first);
            int32_t num_trails = utf8_num_trails(static_cast<utf8_code_unit>(code_point));
            if (num_trails == 0) {
                ++first;
                return 1;
            }
            if (num_trails < 0) {
                return E_INVALID_START_BYTE;
            }
            ++first;
            return read_trails(num_trails, first, last, code_point);
            
#if 0            
            // Read trailing bytes:
            if (first == last) {
                return E_UNEXPECTED_ENDOFINPUT;
            }
            code_point_t ch_next = static_cast<uint8_t>(*first); 
            if (not utf8_is_trail(ch_next)) {
                return E_TRAIL_EXPECTED;
            }     
            code_point_t cp = code_point;
            switch (num_trails) {
                case 1:
                    cp = ((cp << 6) & 0x7FFu) + ((ch_next) & 0x3Fu);
                    code_point = cp;
                    ++first;
                    return 1;
                case 2:
                    if (cp == 0xE0u) {
                        if (ch_next < 0xA0u)
                            return E_UNCONVERTABLE_OFFSET;
                    }
                    else if (cp == 0xEDu) {
                        if (ch_next > 0x9F) {
                            return E_UNCONVERTABLE_OFFSET;
                        }
                    }
                    ++first;
                    cp = ((cp << 12) & 0xFFFFu) + ((ch_next << 6) & 0xFFFu);
                    if (first == last) {
                        return E_UNEXPECTED_ENDOFINPUT;
                    }
                    ch_next = static_cast<utf8_code_unit>(*first++);
                    if (not utf8_is_trail(ch_next)) {
                        return E_TRAIL_EXPECTED;
                    }                        
                    cp += ch_next & 0x3Fu;
                    code_point = cp;
                    return 1;
                case 3:
                    if (cp == 0xF0u) {
                        if (ch_next < 0x90u) {
                            return E_UNCONVERTABLE_OFFSET;
                        }
                    }
                    else if (cp == 0xF4 and ch_next > 0x8Fu) {
                        return E_UNCONVERTABLE_OFFSET;
                    }
                    ++first;
                    cp = ((cp << 18) & 0x1FFFFFu) + ((ch_next << 12) & 0x3FFFFu);
                    if (first == last) {
                        return E_UNEXPECTED_ENDOFINPUT;
                    }
                    ch_next = static_cast<utf8_code_unit>(*first++);
                    if (not utf8_is_trail(ch_next)) {
                        return E_TRAIL_EXPECTED;
                    }                        
                    cp += (ch_next << 6) & 0xFFFu;
                    if (first == last) {
                        return E_UNEXPECTED_ENDOFINPUT;
                    }
                    ch_next = static_cast<utf8_code_unit>(*first++);
                    if (not utf8_is_trail(ch_next)) {
                        return E_TRAIL_EXPECTED;
                    }                        
                    cp += ch_next & 0x3Fu; 
                    code_point = cp;
                    return 1;
            }
            
            return E_INVALID_START_BYTE;
#endif            
        }    
        
        
#pragma mark Codepoint to UTF-8 
        // Convert one Unicode code point using UTF-8 encoding and copy the result
        // into the sequence starting at dest.  
        // The output sequence must be capable to hold the number of generated
        // code units.
        // If the conversion was successful, returns the number of generated code
        // unites. Otherwise returns a negative number indicating an error.
        //
        // Surrogates and otherwise invalid Unicode code points will not be 
        // converted, and return an error.
        //
        // IteratorT shall be at least a Forward Iterator type.
        //
        //  Errors:
        //  E_INVALID_CODE_POINT:   The Unicode code point is a surrogate or not
        //                          a valid Unicode code point at all.
        // 
        inline int 
        operator() (    
            code_point_t    code_point, 
            IteratorT&      dest) const
        {
            typedef typename EncodingT::code_unit_type  code_unit_t;          
            
            // encode the Unicode code point to a UTF-8 byte sequence:
            int len = utf8_encoded_length(code_point); // returns 0 if code_point is a surrogate or not a valid Unicode code point
            switch (len) {
                case 1:
                    *dest++ = static_cast<code_unit_t>(code_point);
                    break;
                case 2:
                    *dest++ = static_cast<code_unit_t>(((code_point >> 6) | 0xC0u));
                    *dest++ = static_cast<code_unit_t>(((code_point & 0x3Fu) | 0x80u));
                    break;
                case 3: 
                    *dest++ = static_cast<code_unit_t>(((code_point >> 12) | 0xE0u));
                    *dest++ = static_cast<code_unit_t>((((code_point >> 6) & 0x3Fu) | 0x80u));
                    *dest++ = static_cast<code_unit_t>(((code_point & 0x3Fu) | 0x80u));
                    break;
                case 4:
                    *dest++ = static_cast<code_unit_t>(((code_point >> 18) | 0xF0u));
                    *dest++ = static_cast<code_unit_t>((((code_point >> 12) & 0x3Fu) | 0x80u));
                    *dest++ = static_cast<code_unit_t>((((code_point >> 6) & 0x3Fu) | 0x80u));
                    *dest++ = static_cast<code_unit_t>(((code_point & 0x3Fu) | 0x80u));
                    break;
                default: 
                    return E_INVALID_CODE_POINT;
            }
            
            return len;
        }
        
        
    };
    
    
#pragma mark I 2.a Struct convert_codepoint_unsafe  <UTF-16>
    //
    // struct convert_codepoint_unsafe    UTF-16 -> code point  
    //
    template <typename IteratorT, typename EncodingT>
    struct convert_codepoint_unsafe<IteratorT, EncodingT,
        typename boost::enable_if<
            boost::is_base_of<UTF_16_encoding_tag, EncodingT>
        >::type
    >
    {
        // Cant't check size of an output iterator!
        //        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
        //                            == sizeof(typename EncodingT::code_unit_type));

        // Convert the UTF-16 sequence starting at first to an Unicode code point.
        // Endianness may be explicitly specified, or if not, the Encoding will be
        // "upgraded" to an Encoding including host endianness.
        // Unsafe version:
        //  - does not check for iterating past last
        //  - does not check for valid surrogate pairs.
        //  - does not check if first is a valid start byte for UTF-16
        //  - does not check for noncharacters
        // On success, returns the number of generated code unites. Otherwise
        // returns a negative number.
        inline int
        operator() (
            IteratorT&      first, 
            IteratorT       last, 
            code_point_t&   code_point) const
        {            
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));
            
            
            // Upgrade Encoding to include endianness if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_16_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         from_encoding_t;        
            typedef typename from_encoding_t::endian_tag    from_endian_t;
            typedef typename host_endianness::type          to_endian_t;
            
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            if (utf16_is_single(ch)) {
                ++first;
                code_point = static_cast<code_point_t>(ch);
            }
            else /* assuming utf16_is_lead(ch) */ {            
                utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*++first));
                /* assuming utf16_is_trail(ch2) */
                ++first;
                code_point = utf16_surrogate_pair_to_code_point(ch, ch2);
            }
            
            return 1;        
        }
        
        // Convert one Unicode code point using UTF-16 encoding and copy the 
        // result into the sequence starting at dest.
        // Parameter code_point shall be a valid Unicode scalar value (this 
        // excludes all invalid Unicodes and surrogates).
        //
        // Returns the number of generated code unites 1 or 2 (surrogate pair).
        //
        // The output sequence must be capable to hold the number of generated
        // code units.
        // 
        // If the encoding's endianness does not equal the host endianness a swap 
        // will automatically applied to the resulting code points.
        //
        // The Unicode code point is not checked for validity. If it is not a 
        // valid Unicode scalar value, the result is undefined.
        //
        inline int 
        operator() (    
            code_point_t    code_point, 
            IteratorT&      dest) const
        {
            // Upgrade Target Encoding to include endiannes if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_16_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         to_encoding_t;        
            typedef typename host_endianness::type          from_endian_t;
            typedef typename to_encoding_t::endian_tag      to_endian_t;

            typedef typename to_encoding_t::code_unit_type  code_unit_t;
            
            // encode the unicode character to a UTF-16 code unit, or possibly
            // to a surrogate pair (two code units).
            // utf16_encoded_length_unsafe() will return 1 for surrogates, which
            // are actually invalid and result becomes bogus. It will return -1 
            // for invalid Unicode code points (> U+10FFFF).
            int len = utf16_encoded_length_unsafe(code_point);
            switch (len) {
                case 1: 
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<code_unit_t>(code_point));
                    break;
                case 2:
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(code_point));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(code_point));
                    break;
                default: return E_INVALID_CODE_POINT; // ERROR: bad unicode value
            }
            
            return len;
        }
        
        
    };
    

#pragma mark I 2.b Struct convert_codepoint <UTF-16>
    //
    // struct convert_codepoint           UTF-16 -> code point
    //
    template <typename IteratorT, typename EncodingT>
    struct convert_codepoint<IteratorT, EncodingT,
        typename boost::enable_if<
            boost::is_base_of<UTF_16_encoding_tag, EncodingT>
        >::type
    >
    {
        // Cant't check size of an output iterator!
        //        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
        //                            == sizeof(typename EncodingT::code_unit_type));
        
        // Convert the UTF-16 sequence starting at first to an Unicode code point.
        // Endianness may be explicitly specified, or if not, the Encoding will be
        // "upgraded" to an Encoding including host endianness.
        // On success, returns the number of generated code unites. Otherwise
        // returns a negative number.
        // Errors:
        //      E_TRAIL_EXPECTED:           trail byte expected
        //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
        //      E_INVALID_START_BYTE        invalid start byte
        //
        inline int
        operator() (
            IteratorT&      first, 
            IteratorT       last, 
            code_point_t&   code_point) const
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));
            
            // Upgrade Encoding to include endianness if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_16_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         from_encoding_t;        
            typedef typename from_encoding_t::endian_tag    from_endian_t;
            typedef typename host_endianness::type          to_endian_t;
            
            assert(first != last);
            
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            if (utf16_is_single(ch)) {
                ++first;
                code_point = static_cast<code_point_t>(ch);
                return 1;
            }
            else if (utf16_is_lead(ch)) {
                ++first;
                if (first != last) {
                    utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
                    if (utf16_is_trail(ch2)) {
                        ++first;
                        code_point = utf16_surrogate_pair_to_code_point(ch, ch2);
                    }
                    else {
                        return E_TRAIL_EXPECTED;
                    }
                }
                else {
                    return E_UNEXPECTED_ENDOFINPUT;
                }
            }
            else {
                return E_INVALID_START_BYTE;
            }
            
            return 1;        
        }

        
        // Convert one Unicode code point using UTF-16 encoding and copy the 
        // result into the sequence starting at dest.  
        // The output sequence must be capable to hold the number of generated
        // code units.
        //
        // Returns the number of generated code unites, or a negative mumber
        // indicating an error.
        // 
        // If the encoding's endianness does not equal the host endianness a swap 
        // will automatically applied to the resulting code points.
        //
        // 
        // Errors:
        //  E_INVALID_CODE_POINT:   Surrogate or out of Unicode code space.
        //
        inline int 
        operator() (    
            code_point_t                            code_point, 
            IteratorT&                              dest)
        {
            // Upgrade Target Encoding to include endiannes if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_16_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         to_encoding_t;        
            typedef typename host_endianness::type          from_endian_t;
            typedef typename to_encoding_t::endian_tag      to_endian_t;

            typedef typename to_encoding_t::code_unit_type  code_unit_t;
            
            // encode the unicode character to a UTF-16 code unit, or possibly
            // to a surrogate pair (two code units).
            // utf16_encoded_length() returns zero for surrogates and invalid Unicode
            // code points above U+10FFFF.
            int len = utf16_encoded_length(code_point);
            
            switch (len) {
                case 1: 
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<code_unit_t>(code_point));
                    break;
                case 2:
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(code_point));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(code_point));
                    break;
                default: 
                    return E_INVALID_CODE_POINT;  // surrogate or code_point > U+10FFFF
            }
            
            return len;
        }
        
        
        
    };
    
    
#pragma mark I 3.a  Struct convert_codepoint_unsafe <UTF-32>
    //
    // struct convert_codepoint_unsafe    UTF-32 -> code point
    //
    template <typename IteratorT, typename EncodingT>
    struct convert_codepoint_unsafe<IteratorT, EncodingT,
        typename boost::enable_if<
            boost::is_base_of<UTF_32_encoding_tag, EncodingT>
        >::type
    >
    {
        // Cant't check size of an output iterator!
        //        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
        //                            == sizeof(typename EncodingT::code_unit_type));
        

        // Convert one UTF-32 code unit using Encoding to an Unicode code point 
        // and copy the result into paramerer code_point.
        //
        // If the encoding's endianness does not equal the host endianness a swap 
        // will automatically applied to the input code point.
        //
        // The conversion does not check for errors and always returns 1. The  
        // iterator first will be advanced by one. 
        //
        // This converter melery swaps the bytes if required.
        //
        inline int
        operator() (
            IteratorT&      first, 
            IteratorT       last, 
            code_point_t&   code_point) const
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));            
            
            // Upgrade Encoding to include endianness if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_32_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         from_encoding_t;        
            typedef typename from_encoding_t::endian_tag    from_endian_t;
            typedef typename host_endianness::type          to_endian_t;
            
            code_point = byte_swap<from_endian_t, to_endian_t>(*first++);
            return 1;
        }
        
        
        // TODO: check the validation
        //
        // Convert one Unicode code point using UTF-32 encoding and copy the 
        // result into the sequence starting at dest.
        // Parameter code_point must be a valid Unicode character or can be an
        // Unicode noncharacter.
        // The output sequence must be able to hold the generated code unit.
        // 
        //
        // If the encoding's endianness does not equal the host endianness a swap 
        // will automatically applied to the resulting code point.
        //
        // Returns 1. 
        //
        // The result is undefined if the code_point is not a valid Unicode code
        // point.
        inline int 
        operator() (    
            code_point_t    code_point, 
            IteratorT&      dest) const
        {
            // Upgrade Target Encoding to include endiannes if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_32_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         to_encoding_t;                    
            typedef typename host_endianness::type          from_endian_t;
            typedef typename to_encoding_t::endian_tag      to_endian_t;
            
            *dest++ = byte_swap<from_endian_t, to_endian_t>(code_point);
            return 1;
        }
        

    };
    
    
#pragma mark I 3.b  Struct convert_codepoint <UTF-32>
    //
    // struct convert_codepoint           UTF-32 -> code point
    //
    template <typename IteratorT, typename EncodingT>
    struct convert_codepoint<IteratorT, EncodingT,
        typename boost::enable_if<
            boost::is_base_of<UTF_32_encoding_tag, EncodingT>
        >::type
    >
    {
        // Cant't check size of an output iterator!
        //        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
        //                            == sizeof(typename EncodingT::code_unit_type));
        
        // Convert one UTF-32 code unit using Encoding to an Unicode code point 
        // and copy the result into paramerer code_point.
        //
        // If the encoding's endianness does not equal the host endianness a swap 
        // will automatically applied to the input code point.
        //
        // If the conversion was successful, returns 1 and iterator first will
        // be advanced by one. Otherwise returns a negative number indicating an 
        // error code.
        //
        // This converter merely checks for a valid code point and swaps the bytes 
        // if required.
        //
        // Errors:
        //  E_INVALID_CODE_POINT:   The Unicode code point is not an Unicode scalar
        //                          value, that is out of range of Unicode space.
        //
        //
        inline int
        operator() (
            IteratorT&      first, 
            IteratorT       last, 
            code_point_t&   code_point) const
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));

            // Upgrade Encoding to include endianness if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_32_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         from_encoding_t;        
            
            typedef typename from_encoding_t::endian_tag    from_endian_t;
            typedef typename host_endianness::type          to_endian_t;
            
            code_point = byte_swap<from_endian_t, to_endian_t>(*first);
            if (!isUnicodeScalarValue(code_point)) {
                return E_INVALID_CODE_POINT;
            }
            ++first;
            return 1;
        }
        
        
        // TODO: check the validation
        // 
        // Convert one Unicode code point using UTF-32 encoding and copy the result
        // into the sequence starting at dest.
        // The output sequence must be capable to hold the number of generated
        // code units.
        //
        // If the encoding's endianness does not equal the host endianness a swap 
        // will automatically applied to the resulting code point.
        //
        // If the conversion was successful, returns 1 and iterator dest will be
        // advanced by one. Otherwise returns a negative number indicating an error 
        // code.
        //
        // This converter merely checks for a valid code point and swaps the bytes 
        // if required.
        //
        // Errors:
        //  E_INVALID_CODE_POINT:   The Unicode code point is not an Unicode scalar
        //                          value, that is out of range of Unicode space.
        //
        inline int 
        operator() (    
            code_point_t    code_point, 
            IteratorT&      dest) const
        {
            // Upgrade Target Encoding to include endiannes if required:
            typedef typename boost::mpl::if_<
                boost::is_same<UTF_32_encoding_tag, EncodingT>,
                typename to_host_endianness<EncodingT>::type,
                EncodingT 
            >::type                                         to_encoding_t;                
            typedef typename host_endianness::type          from_endian_t;
            typedef typename to_encoding_t::endian_tag      to_endian_t;
            
            if (!isUnicodeScalarValue(code_point)) {
                return E_INVALID_CODE_POINT;
            }
            *dest++ = byte_swap<from_endian_t, to_endian_t>(code_point);
            return 1;
        }
                
        
    };
    
    
    
}}}   // namespace json::unicode::internal




#pragma mark - UTF to Unicode Code Point Conversions
namespace json { namespace unicode {
    // 
    //  UTF to Unicode Code Point Conversions
    //
    //  Synopsis:
    //
    // A.1
    //  int 
    //  convert_one(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point)
    //
    // A.2
    //  int 
    //  convert_one(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point, Filter filter)
    //
    // A.3
    //  int 
    //  convert_one(Iterator& first, Iterator last, code_point_t& code_point)
    //
    // A.4
    //  int 
    //  convert_one(Iterator& first, Iterator last, code_point_t& code_point,
    //              Filter filter)
    //
    // B.1
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point)
    //
    // B.2
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point, Filter filter)
    //
    // B.3
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point)
    //
    // B.4
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point,
    //                     Filter filter)
    //
    //
    //
    // Converts a sequence of code units constituting one Unicode charater 
    // encoded in encoding to an Unicode code point.
    //
    // The allowed encodings forms of the input sequence are UTF-32, UTF-16
    // and UTF-8, with their respective endianness variants.
    //
    // If the encoding is not specified, an appropriate encoding type from the
    // iterator's value_type will be generated. This encoding type is always in
    // host endianness.
    //
    // The functions takes two iterators, the first iterator points to the start
    // of a minimal well-formed code unit subsequence which maps to a single
    // Unicode scalar value.
    // The second iterator shall point past the end of the sub sequence of code 
    // units constituting the first unicode character, or it may point farther 
    // away.
    //
    // The *_safe versions will detect ill-formed Unicode code sequences. The
    // Unicode conformance clause (C10) requires not to interpret any ill-formed
    // code unit subsequences in a string as characters. Thus, the safe functions
    // will stop advancing the iterator at the point where the ill-formness
    // has been detected. The iterator 'first' may then point to the start of
    // a minimal well-formed subsequence.
    // 
    //
    // Optionally, a Filter operator can be specified. If the filter evaluates
    // true for a certain code point, it will be checked if it shall be replaced.
    // If this is true, the code point will be replaced by the substitution. If
    // this is false, an error E_PREDICATE_FAILED will be returned, and the
    // parameter code_point contains the original code point.
    // Otherwise, the code point will be accepted as is.
    // 
    // Both variants (safe and unsafe versions) expect that the input sequence
    // is not empty (assert(first != last). The safe version will check for an
    // unexpected end of input sequence, though. 
    //
    // The unsafe version ignores parameter last, and will advance parameter 
    // first as far as required. The unsafe version assumes that the input
    // sequence is a well-formed Unicode sequence. If the sequence is in fact
    // mal-formed, ends prematurely, or if the the input sequence is empty, the 
    // behavior is undefined. Calling an unsafe version with ill-formed input
    // is strongly discouraged, since it may cause the application to crash,
    // returns bogus results, or otherwise corrupt the state of the application.
    //
    // When the function returns successfully, zero will be returned, and 
    // iterator 'first' has been advanved by the number of code units consti-
    // tuting a code point. Parameter code_point has been assigned the result of
    // the conversion.
    //
    // When the functions fails, parameter first will be advanced until the
    // input sequence can be detected as ill-formed, and a negative integer will
    // be returned, indicating an error. Input parameter code_point is left un-
    // changed.
    //
    // The safe version returns zero on success. Otherwise an error code.
    // 
    // The unsafe version assums well-formed input, and returns zero. Otherwise
    // (if the input is not well-formed) the behavior is undefined.
    //
    // The algorithm can be used with any iterator type.
    // Optimizations for certain iterator types are possible.
    // The EncodingT sepcifies the Iterator's encoding.
    //
    //
    //  Examples:
    //  
    //  1)  Convert a sequence of UTF-8 code units to Unicode code points and 
    //      store them in an vector.
    //   
    //  std::vector<unit8_t>  input = ...;
    //  std::vector<uint32_t> output;
    //  std::vector<unit8_t>::iterator first = input.begin();
    //  std::vector<unit8_t>::iterator last = input.last();
    //
    //  while (first != last) {
    //      code_point_t cp;
    //      if (convert_one(first, last, cp) > 0)
    //          output.push_back(cp);
    //      else {
    //          throw std::runtime_error("conversion failed");
    //      }
    //  }
    //  
    
    // A.1
    //  int 
    //  convert_one(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point)
    template <typename IteratorT, typename EncodingT> 
    inline int convert_one(IteratorT& first, IteratorT last, EncodingT encoding, 
                           code_point_t& code_point,
                           typename boost::enable_if<
                           boost::is_base_and_derived<utf_encoding_tag, EncodingT>
                           >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint<IteratorT, EncodingT> Converter;        
        return Converter()(first, last, code_point);        
    }
    
    
    // A.2
    //  int 
    //  convert_one(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
    convert_one(IteratorT& first, IteratorT last, EncodingT encoding, 
                code_point_t& code_point, FilterT filter,
                typename boost::enable_if<
                    boost::mpl::and_<
                        boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                        boost::is_base_and_derived<filter::filter_tag, FilterT>,
                        boost::mpl::not_<boost::is_same<filter::None, FilterT> > 
                    >                
                >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint<IteratorT, EncodingT> Converter;        
        int result = Converter()(first, last, code_point);
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }
    
    // A.2a
    //  int 
    //  convert_one(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int convert_one(IteratorT& first, IteratorT last, EncodingT encoding, 
                           code_point_t& code_point, FilterT filter,
                           typename boost::enable_if<
                               boost::mpl::and_<
                                   boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                                   boost::is_same<filter::None, FilterT>
                               >                
                           >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint<IteratorT, EncodingT> Converter;        
        return Converter()(first, last, code_point);        
    }
    

    // A.3
    //  int 
    //  convert_one(Iterator& first, Iterator last, code_point_t& code_point)
    template <typename IteratorT> 
    inline int convert_one(IteratorT& first, IteratorT last, code_point_t& code_point) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint<IteratorT, Encoding> Converter;        
        return Converter()(first, last, code_point);        
    }
    
    
    // A.4
    //  int 
    //  convert_one(Iterator& first, Iterator last, code_point_t& code_point,
    //              Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int convert_one(IteratorT& first, IteratorT last, code_point_t& code_point, 
                           FilterT filter,
                           typename boost::enable_if<
                            boost::mpl::and_<
                                boost::is_base_and_derived<filter::filter_tag, FilterT>,
                                boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                            >
                           >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint<IteratorT, Encoding> Converter;        
        int result = Converter()(first, last, code_point); 
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }
    
    // A.4a
    //  int 
    //  convert_one(Iterator& first, Iterator last, code_point_t& code_point,
    //              Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int convert_one(IteratorT& first, IteratorT last, code_point_t& code_point, 
                           FilterT filter,
                           typename boost::enable_if<
                            boost::is_same<filter::None, FilterT>
                           >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint<IteratorT, Encoding> Converter;        
        return Converter()(first, last, code_point); 
    }
    
    
    // B.1
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point)
    template <typename IteratorT, typename EncodingT> 
    inline int 
    convert_one_unsafe(IteratorT& first, IteratorT last, EncodingT encoding, 
                       code_point_t& code_point,
                       typename boost::enable_if<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT>
                       >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint_unsafe<IteratorT, EncodingT> Converter;        
        return Converter()(first, last, code_point);        
    }
    

    // B.2
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
    convert_one_unsafe(IteratorT& first, IteratorT last, EncodingT encoding, 
                       code_point_t& code_point, FilterT filter,
                       typename boost::enable_if<
                        boost::mpl::and_<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                            boost::is_base_and_derived<filter::filter_tag, FilterT>,
                            boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                        >                
                       >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint_unsafe<IteratorT, EncodingT> Converter;        
        int result = Converter()(first, last, code_point);        
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }
    
    // B.2a
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
    convert_one_unsafe(IteratorT& first, IteratorT last, EncodingT encoding, 
                       code_point_t& code_point, FilterT filter,
                       typename boost::enable_if<
                        boost::mpl::and_<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                            boost::is_same<filter::None, FilterT>
                        >                
                      >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint_unsafe<IteratorT, EncodingT> Converter;        
        return Converter()(first, last, code_point);        
    }
    
    

    // B.3
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point)
    template <typename IteratorT> 
    inline int convert_one_unsafe(IteratorT& first, IteratorT last, code_point_t& code_point) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint_unsafe<IteratorT, Encoding> Converter;        
        return Converter()(first, last, code_point);        
    }
    

    // B.4
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point,
    //                     Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int convert_one_unsafe(IteratorT& first, IteratorT last, 
                           code_point_t& code_point, FilterT filter,
                           typename boost::enable_if<
                                boost::mpl::and_<
                                  boost::is_base_and_derived<filter::filter_tag, FilterT>,
                                  boost::mpl::not_<boost::is_same<filter::None, FilterT> > 
                                >
                           >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint_unsafe<IteratorT, Encoding> Converter;        
        int result = Converter()(first, last, code_point); 
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }

    // B.4a
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point,
    //                     Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int convert_one_unsafe(IteratorT& first, IteratorT last, 
                                  code_point_t& code_point, FilterT filter,
                                  typename boost::enable_if<
                                    boost::is_same<filter::None, FilterT>
                                  >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint_unsafe<IteratorT, Encoding> Converter;        
        return Converter()(first, last, code_point); 
    }
    
    
}}        



#pragma mark - wchar to Unicode Code Point    
namespace json { namespace unicode {
    
    // On success, returns the number of generated code unites. Otherwise
    // returns a negative number.    
    template <typename IteratorT, typename EncodingT>
    inline int
    convert_wchar_unsafe(
                         IteratorT& first, 
                         IteratorT last, 
                         EncodingT encoding,
                         code_point_t& code_point,
                         typename boost::enable_if<boost::is_base_of<platform_encoding_tag, EncodingT> >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_same<typename boost::iterator_value<IteratorT>::type, wchar_t>::value) );
        
        typedef typename EncodingT::endian_tag      from_endian_t;
        typedef typename host_endianness::type      to_endian_t;
        
        assert("not yet implemented"==0);
        
        return -1;                
    }
    

    // On success, returns the number of generated code unites. Otherwise
    // returns a negative number.
    template <typename IteratorT, typename EncodingT>
    inline int
    convert_wchar(
            IteratorT&      first, 
            IteratorT       last, 
            EncodingT       encoding,
            code_point_t&   code_point,
            typename boost::enable_if<boost::is_base_of<platform_encoding_tag, EncodingT> >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_same<typename boost::iterator_value<IteratorT>::type, wchar_t>::value) );
        
        typedef typename EncodingT::endian_tag      from_endian_t;
        typedef typename host_endianness::type      to_endian_t;
        
        assert("not yet implemented"==0);
        
        return E_UNKNOWN_ERROR;                
    }
    
}}



#pragma mark - Unicode Code Point To UTF
namespace json { namespace unicode {
    
    //  Unicode Code Point to UTF Conversions
    //
    //  Synopsis:
    //
    // A.1
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Encoding encoding)
    // 
    // A.2
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    //
    // A.3
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest)
    //
    // A.4
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Filter filter)
    //
    //
    // B.1
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding)
    //
    // B.2
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    //
    // B.3
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest)
    //
    // B.4
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Filter filter)
    //
    //
    // Converts an Unicode code unit to a corresponding sequence of Unicode code 
    // units using Encoding and stores the result into the sequence starting
    // at iterator dest.
    // 
    // Returns the number of generated code units in encoding or a negative
    // integer indicating an error.
    //
    //
    //
    // The allowed encodings forms of the output sequence are UTF-32, UTF-16
    // and UTF-8, with their respective endianness variants.
    //
    // The functions take a parameter code_point which shall be converted, and
    // an iterator specifying the sequence of code units where the result should
    // be copied. 
    //
    // The encoding can be explicitly specified. If it is omitted, an appropria-
    // te encoding type from the iterator's value_type will be generated. This 
    // encoding type is always in host endianness.
    //
    // Optionally, a Filter operator can be specified. If the filter evaluates
    // true for a certain code point, it will be checked if it shall be replaced.
    // If this is true, the code point will be replaced by the replacement. If
    // this is false, an error E_PREDICATE_FAILED will be returned.
    // Otherwise, the code point will be accepted as is.
    // 
    // 
    // The safe version will detect an invalid Unicode code point, and will 
    // return an error in this case. The unsafe versions will not detect invalid
    // code points and in this case the result of the function is undefined.
    //
    // Both versions (safe and unsafe) do not check for a range error of the
    // output sequence. The caller is responsible that the output sequence can
    // hold the result, or otherwise will throw an exception.
    //
    // When the function returns successfully, the number of generated code 
    // units will be returned, and iterator 'dest' has been advanved by the 
    // number of code units respectively.
    //
    // When the functions fails, dest is unchanged and a negative integer will
    // be returned indicating the error.
    //
    // The unsafe version assumes a valid Unicode scalar value, otherwise (if 
    // the input is not valid) the behavior is undefined.
    //
    // The algorithm can be used with any iterator type.
    // The EncodingT sepcifies the Iterator's encoding.
    //
    //
    //  Examples:
    //  
    //  1)  Convert an Unicode code point to UTF-8 and store the result in a 
    //      vector.
    //   
    //  std::vector<unit8_t>  output;
    //  code_point_t cp = ...;
    //  int result = convert_one(cp, std::back_inserter(output));
    //  
    
    // A.1
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Encoding encoding)
    template <typename IteratorT, typename EncodingT> 
    inline int
    convert_one(code_point_t code_point, IteratorT& dest, EncodingT encoding,
                typename boost::enable_if<
                boost::is_base_and_derived<utf_encoding_tag, EncodingT> 
                >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint<IteratorT, EncodingT> Converter;        
        return Converter()(code_point, dest);        
    }
    

    // A.2
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int
    convert_one(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                FilterT filter,
                typename boost::enable_if<
                    boost::mpl::and_<
                        boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                        boost::is_base_and_derived<filter::filter_tag, FilterT>,
                        boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                    >                
                >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint<IteratorT, EncodingT> Converter;        
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter()(code_point, dest);        
    }
    
    // A.2a
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int
    convert_one(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                FilterT filter,
                typename boost::enable_if<
                    boost::mpl::and_<
                        boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                        boost::is_same<filter::None, FilterT>
                    >                
                >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint<IteratorT, EncodingT> Converter;        
        return Converter()(code_point, dest);        
    }
    
    

    
    // A.3
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest)
    template <typename IteratorT> 
    inline int 
    convert_one(code_point_t code_point, IteratorT& dest) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint<IteratorT, Encoding> Converter;        
        return Converter()(code_point, dest);        
    }
    
    
    // A.4
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
    convert_one(code_point_t code_point, IteratorT& dest, FilterT filter,
                typename boost::enable_if<
                    boost::mpl::and_<
                        boost::is_base_and_derived<filter::filter_tag, FilterT>,
                        boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                    >
                >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint<IteratorT, Encoding> Converter;        
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter()(code_point, dest);        
    }
    

    // A.4a
    //  int 
    //  convert_one(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
    convert_one(code_point_t code_point, IteratorT& dest, FilterT filter,
                typename boost::enable_if<
                    boost::is_same<filter::None, FilterT>
                >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint<IteratorT, Encoding> Converter;        
        return Converter()(code_point, dest);        
    }
    
    
    // B.1
    template <typename IteratorT, typename EncodingT> 
    inline int 
    convert_one_unsafe(code_point_t code_point, IteratorT& dest, EncodingT encoding,
                       typename boost::enable_if<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT> 
                       >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint_unsafe<IteratorT, EncodingT> Converter;        
        return Converter()(code_point, dest);        
    }

    
    // B.2
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
    convert_one_unsafe(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                       FilterT filter,
                       typename boost::enable_if<
                        boost::mpl::and_<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                            boost::is_base_and_derived<filter::filter_tag, FilterT>,
                            boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                        >
                       >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint_unsafe<IteratorT, EncodingT> Converter; 
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter()(code_point, dest);        
    }
    
    
    // B.2a
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
    convert_one_unsafe(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                       FilterT filter,
                       typename boost::enable_if<
                        boost::mpl::and_<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                            boost::is_same<filter::None, FilterT>
                        >
                       >::type* dummy = 0) 
    {
        typedef internal::convert_codepoint_unsafe<IteratorT, EncodingT> Converter; 
        return Converter()(code_point, dest);        
    }
    
    
    // B.3
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest)
    template <typename IteratorT> 
    inline int 
    convert_one_unsafe(code_point_t code_point, IteratorT& dest) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint_unsafe<IteratorT, Encoding> Converter;        
        return Converter()(code_point, dest);        
    }
    
    
    // B.4
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
    convert_one_unsafe(code_point_t code_point, IteratorT& dest, FilterT filter,
                       typename boost::enable_if< 
                        boost::mpl::and_<
                            boost::is_base_and_derived<filter::filter_tag, FilterT>,
                            boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                        >
                       >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint_unsafe<IteratorT, Encoding> Converter;   
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter()(code_point, dest);        
    }
    
    
    // B.4a
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
    convert_one_unsafe(code_point_t code_point, IteratorT& dest, FilterT filter,
                       typename boost::enable_if<
                        boost::is_same<filter::None, FilterT>
                       >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::convert_codepoint_unsafe<IteratorT, Encoding> Converter;   
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter()(code_point, dest);        
    }
    
    
    
    
}}  // namespace json::unicode





#pragma mark - Internal  Base Template convert_one_xxx 
namespace json { namespace unicode { namespace internal {

    
    using namespace json::unicode;
    
#pragma mark Base Template convert_one_unsafe 
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT,
        class Enable = void
    >
    struct convert_one_unsafe {
        
        int operator() (
            InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT = FilterT()
        ) const 
        { 
            return E_UNKNOWN_ERROR; 
        }
    };
    
    
#pragma mark Base Template convert_one 
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT,
        class Enable = void
    >
    struct convert_one {
        
        int operator() (
            InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT = FilterT()
        ) const 
        { 
            return E_UNKNOWN_ERROR; 
        }
    };
    
    
}}}
    


#pragma mark - Count 
namespace json { namespace unicode {
    
    
    // Counts the number of required code unites when converting the well-
    // formed UTF-8 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites.
    // If the input sequence is not a well-formed UTF-8 sequence the result and 
    // behavior is undefined.
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count_unsafe(
                 IteratorT&      first, 
                 IteratorT       last, 
                 FromEncodingT   fromEncoding,
                 ToEncodingT     toEncoding,
                 typename boost::enable_if<
                 boost::mpl::and_<
                 boost::is_same<UTF_8_encoding_tag, FromEncodingT>,
                 boost::is_base_of<UTF_16_encoding_tag, ToEncodingT>
                 >
                 >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        std::size_t count = 0;
        while (first != last) {
            int n = utf8_num_trails(*first) + 1;
            switch (n) {
                case 1:
                case 2:
                case 3:
                    ++count;
                case 4:
                    count += 2;
            }
            std::advance(first, n);
        }
        return count;        
    }
    
    
    // Counts the number of required code unites when converting a possibly mal-
    // formed UTF-8 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites if the input sequence is
    // well-formed. Otherwise returns a negative number indicating an error.
    //
    // Performs the following checks:
    //  - checks whether the start of a multi byte sequence is a valid start 
    //    byte.
    //  - checks if the number of trail bytes is correct.
    //  - checks if the range [first, last) constitutes a complete string.
    //  Does NOT check if the input sequence contains an Unicode noncharacter.
    //
    // On exit, if the input sequence is well-formed, first points to last, 
    // otherwise if the input sequence is mal-formed, the parameter first will 
    // be advanced to the point where the error occured.
    //
    // Errors:
    //      E_INVALID_START_BYTE:       not a valid start byte
    //      E_TRAIL_EXPECTED:           expected a trail byte
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected end of string
    //
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count(
          IteratorT&      first, 
          IteratorT       last, 
          FromEncodingT   fromEncoding,
          ToEncodingT     toEncoding,
          int&            error,
          typename boost::enable_if<
          boost::mpl::and_<
          boost::is_same<UTF_8_encoding_tag, FromEncodingT>,
          boost::is_base_of<UTF_16_encoding_tag, ToEncodingT>
          >
          >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        std::size_t count = 0;
        uint8_t ch = *first;
        while (first != last) {
            if (utf8_is_single(ch)) {
                ++count;
                ++first;
            } else if (utf8_is_lead(ch)) {
                int num_trails = utf8_num_trails(ch);
                switch (num_trails) {
                    case 1:
                    case 2:
                        ++count;
                        break;
                    case 3:
                        count += 2;
                        break;
                    default:
                        error = E_UNKNOWN_ERROR;
                        return 0;
                }
                ++first;
                while (first != last and num_trails) {
                    if (not utf8_is_trail(*first)) {
                        error = E_TRAIL_EXPECTED;
                        return 0;
                    } 
                    --num_trails;
                    ++first;
                }
                if (num_trails) {
                    error = E_UNEXPECTED_ENDOFINPUT;
                    return 0;
                }
            }
            else {
                error = E_INVALID_START_BYTE;
                return 0;
            }                
        }
        
        error = 0;
        return count;
    }
    
    
    // Counts the number of required code unites when converting the well-
    // formed UTF-8 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites.
    // If the input sequence is not a well-formed UTF-8 sequence the result and 
    // behavior is undefined.
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count_unsafe(
                 IteratorT&      first, 
                 IteratorT       last, 
                 FromEncodingT   fromEncoding,
                 ToEncodingT     toEncoding,
                 typename boost::enable_if<
                 boost::mpl::and_<
                 boost::is_same<UTF_8_encoding_tag, FromEncodingT>,
                 boost::is_base_of<UTF_32_encoding_tag, ToEncodingT>
                 >
                 >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        std::size_t result = 0;
        while (first != last) {
            int n = utf8_num_trails(*first) + 1;
            ++result;
            std::advance(first, n);
        }
        
        return result;        
    }
    
    
    // Counts the number of required code unites when converting a possibly mal-
    // formed UTF-8 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites if the input sequence is
    // well-formed. Otherwise returns zero and set error to a negative number 
    // indicating an error.
    //  - checks whether the start of a multi byte sequence is a valid start 
    //    byte.
    //  - checks if the number of trail bytes is correct.
    //  - checks if the range [first, last) constitutes a complete string.
    // On exit, if the input sequence is well-formed, first points to last, 
    // otherwise if the input sequence is mal-formed, the parameter first will 
    // be advanced to the point where the error occured.
    //
    // Errors:
    //      E_INVALID_START_BYTE:       not a valid start byte
    //      E_TRAIL_EXPECTED:           expected a trail byte
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected end of string
    //
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count(
          IteratorT&      first, 
          IteratorT       last, 
          FromEncodingT   fromEncoding,
          ToEncodingT     toEncoding,
          int&            error,
          typename boost::enable_if<
          boost::mpl::and_<
          boost::is_same<UTF_8_encoding_tag, FromEncodingT>,
          boost::is_base_of<UTF_32_encoding_tag, ToEncodingT>
          >
          >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        std::size_t result = 0;
        while (first != last) {
            if (is_single(*first)) {
                result += 1;
                ++first;
            }
            else if (is_lead(*first)) {
                int num_trails = utf8_num_trails(*first);
                ++first;
                result += 1;
                while (first != last and num_trails > 0) {
                    if (not is_trail(*first)) {
                        error = E_TRAIL_EXPECTED;
                        return 0;
                    }
                    ++first;
                    --num_trails;
                }
                if (num_trails) {
                    error = E_UNEXPECTED_ENDOFINPUT; // ERROR: unexpected end of string 
                    return 0;
                }
            } else {
                error =  E_INVALID_START_BYTE; // ERROR: not a valid start byte.
                return 0;
            }
        }
        
        error = 0;
        return result;        
    }
    
    
    
    // Counts the number of required code unites when converting the well-
    // formed UTF-16 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites.
    // If the input sequence is not a well-formed UTF-16 sequence the result and 
    // behavior is undefined.
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline long
    count_unsafe(
                 IteratorT&      first, 
                 IteratorT       last, 
                 FromEncodingT   fromEncoding,
                 ToEncodingT     toEncoding,
                 typename boost::enable_if<
                 boost::mpl::and_<
                 boost::is_base_of<UTF_16_encoding_tag, FromEncodingT>,
                 boost::is_same<UTF_8_encoding_tag, ToEncodingT>
                 >
                 >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        std::size_t count = 0;
        while (first != last) {
            utf16_code_unit ch = *first;
            // utf8_encoded_length() returns zero if this is a surrogate,
            // (or if this not a valid Unicode code point). We use this to
            // detect surrogates in the input.
            int n = utf8_encoded_length(ch);  
            ++first;                
            if (n != 0) {
                count += n;
            }
            else {
                // assuming there is a valid surrogate pair
                count += 4;
                ++first;
            }
        }
        
        return count;
    }
    
    
    // Counts the number of required code unites when converting a possibly mal-
    // formed UTF-16 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites if the input sequence is
    // well-formed. Otherwise returns a negative number indicating an error.
    //  - checks whether the start of a multi byte sequence is a valid start 
    //    byte.
    //  - checks if the number of trail bytes is correct.
    //  - checks if the range [first, last) constitutes a complete string.
    // On exit, if the input sequence is well-formed, first points to last, 
    // otherwise if the input sequence is mal-formed, the parameter first will 
    // be advanced to the point where the error occured.
    //
    // Errors:
    //      E_INVALID_START_BYTE:       not a valid start byte
    //      E_TRAIL_EXPECTED:           expected a trail byte
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected end of string
    //
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count(
          IteratorT&      first, 
          IteratorT       last, 
          FromEncodingT   fromEncoding,
          ToEncodingT     toEncoding,
          int&            error,
          typename boost::enable_if<
          boost::mpl::and_<
          boost::is_base_of<UTF_16_encoding_tag, FromEncodingT>,
          boost::is_same<UTF_8_encoding_tag, ToEncodingT>
          >
          >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        std::size_t count = 0;
        while (first != last) {
            utf16_code_unit ch = *first;
            // utf8_encoded_length() returns zero if this is a surrogate,
            // (or if this not a valid Unicode code point). We use this to
            // detect surrogates in the input.
            int n = utf8_encoded_length(ch);
            ++first;                
            if (n != 0) {
                count += n;
            }
            else {
                if (first != last) {
                    utf16_code_unit ch2 = *first;
                    if (utf16_is_lead(ch)) {
                        if (utf16_is_trail(ch2)) {
                            count += 4;
                            ++first;
                        }
                        else {
                            error = E_TRAIL_EXPECTED;
                            return 0;
                        }
                    }
                    else {
                        error = E_INVALID_START_BYTE;
                        return 0;
                    }
                }
                else {
                    error = E_UNEXPECTED_ENDOFINPUT;
                    return 0;
                }
            }
        }
        
        error = 0;
        return count;
    }
  
    
    // Counts the number of required code unites when converting the well-
    // formed UTF-16 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites.
    // If the input sequence is not a well-formed UTF-16 sequence the result and 
    // behavior is undefined.
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count_unsafe(
                 IteratorT&      first, 
                 IteratorT       last, 
                 FromEncodingT   fromEncoding,
                 ToEncodingT     toEncoding,
                 typename boost::enable_if<
                 boost::mpl::and_<
                 boost::is_base_of<UTF_16_encoding_tag, FromEncodingT>,
                 boost::is_base_of<UTF_32_encoding_tag, ToEncodingT>
                 >
                 >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        assert("not yet implemented"==0);
        return 0;        
    }
    
    
    // Counts the number of required code unites when converting a possibly mal-
    // formed UTF-16 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites if the input sequence is
    // well-formed. Otherwise returns a negative number indicating an error.
    //  - checks whether the start of a multi byte sequence is a valid start 
    //    byte.
    //  - checks if the number of trail bytes is correct.
    //  - checks if the range [first, last) constitutes a complete string.
    // On exit, if the input sequence is well-formed, first points to last, 
    // otherwise if the input sequence is mal-formed, the parameter first will 
    // be advanced to the point where the error occured.
    //
    // Errors:
    //      E_INVALID_START_BYTE:       not a valid start byte
    //      E_TRAIL_EXPECTED:           expected a trail byte
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected end of string
    //
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count(
          IteratorT&      first, 
          IteratorT       last, 
          FromEncodingT   fromEncoding,
          ToEncodingT     toEncoding,
          int&            error,
          typename boost::enable_if<
          boost::mpl::and_<
          boost::is_base_of<UTF_16_encoding_tag, FromEncodingT>,
          boost::is_base_of<UTF_32_encoding_tag, ToEncodingT>
          >
          >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        assert("not yet implemented"==0);
        error = -1;
        return 0;        
    }
    
    
    // Counts the number of required code unites when converting the well-
    // formed UTF-32 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites.
    // If the input sequence is not a well-formed UTF-8 sequence the result and 
    // behavior is undefined.
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count_unsafe(
                 IteratorT&      first, 
                 IteratorT       last, 
                 FromEncodingT   fromEncoding,
                 ToEncodingT     toEncoding,
                 typename boost::enable_if<
                 boost::mpl::and_<
                 boost::is_base_of<UTF_32_encoding_tag, FromEncodingT>,
                 boost::is_same<UTF_8_encoding_tag, ToEncodingT>
                 >
                 >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        assert("not yet implemented"==0);
        return 0;        
    }
    
    
    // Counts the number of required code unites when converting a possibly mal-
    // formed UTF-32 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites if the input sequence is
    // well-formed. Otherwise returns a negative number indicating an error.
    //  - checks whether the start of a multi byte sequence is a valid start 
    //    byte.
    //  - checks if the number of trail bytes is correct.
    //  - checks if the range [first, last) constitutes a complete string.
    // On exit, if the input sequence is well-formed, first points to last, 
    // otherwise if the input sequence is mal-formed, the parameter first will 
    // be advanced to the point where the error occured.
    //
    // Errors:
    //      E_INVALID_START_BYTE:       not a valid start byte
    //      E_TRAIL_EXPECTED:           expected a trail byte
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected end of string
    //
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count(
          IteratorT&      first, 
          IteratorT       last, 
          FromEncodingT   fromEncoding,
          ToEncodingT     toEncoding,
          int&            error,
          typename boost::enable_if<
          boost::mpl::and_<
          boost::is_base_of<UTF_32_encoding_tag, FromEncodingT>,
          boost::is_same<UTF_8_encoding_tag, ToEncodingT>
          >
          >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        assert("not yet implemented"==0);
        error = -1;
        return 0;        
    }
    
    
    // Counts the number of required code unites when converting the well-
    // formed UTF-32 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites.
    // If the input sequence is not a well-formed UTF-8 sequence the result and 
    // behavior is undefined.
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count_unsafe(
                 IteratorT&      first, 
                 IteratorT       last, 
                 FromEncodingT   fromEncoding,
                 ToEncodingT     toEncoding,
                 typename boost::enable_if<
                 boost::mpl::and_<
                 boost::is_base_of<UTF_32_encoding_tag, FromEncodingT>,
                 boost::is_base_of<UTF_16_encoding_tag, ToEncodingT>
                 >
                 >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        assert("not yet implemented"==0);
        return 0;        
    }
    
    
    // Counts the number of required code unites when converting a possibly mal-
    // formed UTF-32 sequence [first, last) into a string encoded in toEncoding.
    // Returns the number of required code unites if the input sequence is
    // well-formed. Otherwise returns a negative number indicating an error.
    //  - checks whether the start of a multi byte sequence is a valid start 
    //    byte.
    //  - checks if the number of trail bytes is correct.
    //  - checks if the range [first, last) constitutes a complete string.
    // On exit, if the input sequence is well-formed, first points to last, 
    // otherwise if the input sequence is mal-formed, the parameter first will 
    // be advanced to the point where the error occured.
    //
    // Errors:
    //      E_INVALID_START_BYTE:       not a valid start byte
    //      E_TRAIL_EXPECTED:           expected a trail byte
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected end of string
    //
    template <typename IteratorT, typename FromEncodingT, typename ToEncodingT>
    inline std::size_t
    count(
          IteratorT&      first, 
          IteratorT       last, 
          FromEncodingT   fromEncoding,
          ToEncodingT     toEncoding,
          int&            error,
          typename boost::enable_if<
          boost::mpl::and_<
          boost::is_base_of<UTF_32_encoding_tag, FromEncodingT>,
          boost::is_base_of<UTF_16_encoding_tag, ToEncodingT>
          >
          >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT(
                            sizeof(typename boost::iterator_value<IteratorT>::type) 
                            == sizeof(typename FromEncodingT::code_unit_type));
        
        assert("not yet implemented"==0);
        error = -1;
        return 0;        
    }
    
    
    
}}



#pragma mark - Internal
namespace json { namespace unicode { namespace internal {    
    
    
#pragma mark - UTF-8 to UTF-8  
    
    using namespace json::unicode;
    
    // UTF-8 to UTF-8
    //
    // Convert the first character contained in the well-formed UTF-8 sequence
    // [first, last) to an UTF-8 sequence starting at dest.
    // This function merely applies filter.
    // 
    // (see generic documentation above)
    // 
    // Returns the number of generated code units [1, 2].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid number of trail bytes.
    //  - does not check if first is a valid start byte for UTF-8
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED:     
    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
            boost::is_same<UTF_8_encoding_tag, InEncodingT>,
            boost::is_same<UTF_8_encoding_tag, OutEncodingT>
        >
    >::type>
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        typedef InEncodingT                         from_encoding_t;
        typedef OutEncodingT                        to_encoding_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp;
            int result = json::unicode::convert_one_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t());
            }            
            return result;
        }   
    };
    
    // Specialization for filter type equals filter::None
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,
        typename boost::enable_if<
        boost::mpl::and_<
            boost::is_same<UTF_8_encoding_tag, InEncodingT>,
            boost::is_same<UTF_8_encoding_tag, OutEncodingT>
        >
    >::type>
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef json::unicode::filter::None         NoFilter; 
        typedef InEncodingT                         from_encoding_t;
        typedef OutEncodingT                        to_encoding_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            // assuming start byte
            const int count = utf8_num_trails(static_cast<uint8_t>(*first)) + 1;
            int i = count;
            while (i--) {
                *dest++ = *first++;
            }
            return count;
        }   
    };
    
    
    // UTF-8 to UTF-8
    //
    // Convert the first character contained in the possibly mal-formed UTF-8 
    // sequence [first, last) to an UTF-8 sequence starting at dest.
    // (see generic documentation above)
    //
    // This function merely verifies wellformedness and applies filter.
    //
    // Returns the number of generated code units [1, 2], or a negative number
    // indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_NONCHARACTER:             input character is an Unicode noncharacter
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_same<UTF_8_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_8_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        typedef InEncodingT                             from_encoding_t;
        typedef OutEncodingT                            to_encoding_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            // TODO: possibly not the fastest algorithm. Would need to use a buffer
            // where the input is stored after the first conversion, so that we 
            // can just copy the content to dest.
            code_point_t cp;
            int result = json::unicode::convert_one(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t());
            }
            return result;
        }    
    };    
    
    
#pragma mark - UTF-8 to UTF-16  
    
    using namespace json::unicode;

    // UTF-8 to UTF-16
    //
    // Convert the first character contained in the well-formed UTF-8 sequence
    // [first, last) to an UTF-16 Unicode code unit or to a UTF-16
    // surrogate pair and copy the result to the sequence starting at dest.
    // 
    // (see generic documentation above)
    // 
    // Returns the number of generated code units [1, 2].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid number of trail bytes.
    //  - does not check if first is a valid start byte for UTF-8
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED:     
    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_same<UTF_8_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
            >
        >::type>
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endiannes if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_16_encoding_tag, OutEncodingT>,
            typename to_host_endianness<OutEncodingT>::type,
            OutEncodingT 
        >::type                                         to_encoding_t;        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
    private:        
        int
        slow_path(
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {     
            code_point_t cp;
            int result = json::unicode::convert_one_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                if (cp < 0x10000u) {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(cp));
                    return 1;
                } else {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(cp));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(cp));
                    return 2;
                }
            }
            return result;
        }        
        
    public:        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {     
#if 1                
            if (static_cast<uint8_t>(*first) < 0x80u) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename OutEncodingT::code_unit_type>(static_cast<uint8_t>(*first++)));
                return 1;
            } else {
                return slow_path(first, last, dest, filter);
            }
#else                
            code_point_t cp;
            int result = json::unicode::convert_one_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                if (cp < 0x10000u) {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(cp));
                    return 1;
                } else {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(cp));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(cp));
                    return 2;
                }
            }
            return result;
#endif                
        }   
    };
    
    // UTF-8 to UTF-16
    //
    // Convert the first character contained in the possibly mal-formed UTF-8 
    // sequence [first, last) to an UTF-16 Unicode code unit or to a UTF-16
    // surrogate pair and copy the result to the sequence starting at dest.
    // If the character is an Unicode noncharacter, an error will be returned,
    // and no code unit will be generated.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code units [1, 2], or a negative number
    // indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_NONCHARACTER:             input character is an Unicode noncharacter
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_same<UTF_8_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endiannes if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_16_encoding_tag, OutEncodingT>,
            typename to_host_endianness<OutEncodingT>::type,
            OutEncodingT 
        >::type                                         to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;

        
        
    private: 
        static
        int slow_path(
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT())
        {
            code_point_t cp;
            int result = json::unicode::convert_one(first, last, from_encoding_t(), cp, filter);
            if (__builtin_expect(result > 0, 1)) {
                if (__builtin_expect(cp < 0x10000u, 1)) {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(cp));
                    return 1;
                } else {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(cp));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(cp));
                    return 2;
                }
            }
            return result;
        }
        
        int write(code_point_t      codepoint,
                  OutIteratorT&     dest) const
        {
            if (__builtin_expect(codepoint < 0x10000u, 1)) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(codepoint));
                return 1;
            } else {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(codepoint));
                *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(codepoint));
                return 2;
            }
        }

        
    public:        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                                == sizeof(typename InEncodingT::code_unit_type));
            
#if 1            
            // TODO: filter is not applied to ASCII
            if (static_cast<uint8_t>(*first) < 0x80u) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(static_cast<uint8_t>(*first++)));
                return 1;
            } else {
                code_point_t cp;
                int result = json::unicode::convert_one(first, last, cp, filter);
                if (result) {
                    return write(cp, dest);
                }
                else {
                    return result;
                }
            }
#else
            code_point_t cp;
            int result = json::unicode::convert_one(first, last, cp, filter);
            if (result) {
                return write(cp, dest);
            }
            else {
                return result;
            }            
#endif            
        }    
    };    
    
    
    
#pragma mark - UTF-8 to UTF-32
    
    
    // UTF-8 to UTF-32
    // 
    // Convert the first character contained in the well-formed UTF-8 sequence
    // [first, last) to an Unicode code pointand copy the result to the 
    // sequence starting at dest. The input sequence must not be empty.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code units [0, 1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid number of trail bytes.
    //  - does not check if first is a valid start byte for UTF-8
    //  - does not check for noncharacters
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe< 
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_same<UTF_8_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type
    >
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_32_encoding_tag, OutEncodingT>,
            typename to_host_endianness<OutEncodingT>::type,
            OutEncodingT 
        >::type                                         to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp;        
            int result = json::unicode::convert_one_unsafe(
                    first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(cp);
            }
            return result;
        }    
    };
    
    // UTF-8 to UTF-32
    //
    // Convert the first character contained in the possibly mal-formed UTF-8 
    // sequence [first, last) to an Unicode code point and copy the result 
    // to the sequence starting at dest.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code points [0, 1], or a negative number
    // indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_PREDICATE_FAILED:         filter policy returned error
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_same<UTF_8_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_32_encoding_tag, OutEncodingT>,
            typename to_host_endianness<OutEncodingT>::type,
            OutEncodingT 
        >::type                                         to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp;
            int result = json::unicode::convert_one(
                    first, last, from_encoding_t(), 
                    cp, filter);
            if (result > 0) {
                *dest = byte_swap<from_endian_t, to_endian_t>(cp);
                ++dest;
            }
            return result;            
        }  
    };
    
    
    
    
    
#pragma mark - UTF-16 to UTF-8
    
    // Convert the first character contained in the well-formed UTF-16 sequence
    // [first, last) to a UTF-8 sequence and copy the result to the sequence 
    // starting at dest.
    //
    // Parameter inFirst must point to the start of a UTF-16 sequence.
    // The output sequence must be large enough to hold the generated code 
    // point.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code units [0 .. 4].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe <
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
                boost::is_same<UTF_8_encoding_tag, OutEncodingT>
            >
        >::type    
    >
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_16_encoding_tag, InEncodingT>,
            typename to_host_endianness<InEncodingT>::type,
            InEncodingT 
        >::type                                         from_encoding_t;        
        typedef OutEncodingT                            to_encoding_t;
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            
#if 1
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            ++first;  // consume single or high surrogate
            //int n = utf8_encoded_length(static_cast<code_point_t>(ch));  // Returns zero if this is a surrogate or not a valid Unicode code point
            if (utf16_is_single(ch)) {
                return json::unicode::convert_one_unsafe(static_cast<code_point_t>(ch), dest, to_encoding_t(), filter);
            }
            else {
                // assuming: utf16_is_lead(ch)
                utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first)); 
                ++first;  // consume low surrogate
                code_point_t cp = utf16_surrogate_pair_to_code_point(ch, ch2);
                return json::unicode::convert_one_unsafe(cp, dest, to_encoding_t(), filter);
            }
#else            
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            int n = utf8_encoded_length(ch);  // Returns zero if this is a surrogate or not a valid Unicode code point
            switch (n) {
                case 1: 
                    *dest++ = (uint8_t)(ch);
                    ++first;
                    break;
                case 2:
                    *dest++ = (uint8_t)((ch >> 6) | 0xC0u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                case 3: 
                    *dest++ = (uint8_t)((ch >> 12) | 0xE0u);
                    *dest++ = (uint8_t)(((ch >> 6) & 0x3Fu) | 0x80u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                default: 
                    // assuming: utf16_is_lead(ch)
                    utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*++first)); 
                    code_point_t code_point = utf16_surrogate_pair_to_code_point(ch, ch2);
                    ++first;
                    n = json::unicode::convert_one_unsafe(code_point, dest, UTF_8_encoding_tag());
            }
            
            return n;
#endif            
        }    
    };    
    
    // Convert the first character contained in the possibly mal-formed UTF-16 
    // sequence [first, last) to an UTF-8 sequence and copy the result to the
    // sequence starting at dest.
    //
    // Parameter inFirst should point to the start of a UTF-16 sequence.
    // The output sequence should be large enough to hold the generated code 
    // point.
    //
    // On exit first has been advanced by the number of code units constituting 
    // the character, or in case of an error, it points to the code unit where
    // the error occured.
    //
    // Returns the number of generated code units [0 .. 4], or a negative 
    // number indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //
    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one <
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
                boost::is_same<UTF_8_encoding_tag, OutEncodingT>
            >
        >::type    
    >
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
            boost::is_same<UTF_16_encoding_tag, InEncodingT>,
            typename to_host_endianness<InEncodingT>::type,
            InEncodingT 
        >::type                                         from_encoding_t;   
        typedef OutEncodingT                            to_encoding_t;
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()
                ) const
        {
            assert(first != last);
            
#if 1
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            ++first;  // consume single or high surrogate
            //int n = utf8_encoded_length(static_cast<code_point_t>(ch));  // Returns zero if this is a surrogate or not a valid Unicode code point
            if (utf16_is_single(ch)) {
                return json::unicode::convert_one(static_cast<code_point_t>(ch), dest, to_encoding_t(), filter);
            }
            else if (utf16_is_lead(ch))
            {
                if (first == last) {
                    return E_UNEXPECTED_ENDOFINPUT;
                }
                utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
                if (!utf16_is_trail(ch2)) {
                    return E_TRAIL_EXPECTED;
                }
                ++first;  // consume low surrogate
                code_point_t cp = utf16_surrogate_pair_to_code_point(ch, ch2);
                return json::unicode::convert_one(cp, dest, to_encoding_t(), filter);
            }
            else {
                return E_INVALID_START_BYTE;
            }
#else            
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            int n = utf8_encoded_length(ch);  // Returns zero if this is a surrogate or not a valid Unicode code point
            switch (n) {
                case 1: 
                    *dest++ = (uint8_t)(ch);
                    ++first;
                    break;
                case 2:
                    *dest++ = (uint8_t)((ch >> 6) | 0xC0u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                case 3: 
                    *dest++ = (uint8_t)((ch >> 12) | 0xE0u);
                    *dest++ = (uint8_t)(((ch >> 6) & 0x3Fu) | 0x80u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                default: 
                    if (utf16_is_lead(ch)) {
                        ++first;
                        if (first != last) {
                            utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
                            if (utf16_is_trail(ch2)) {
                                code_point_t code_point = utf16_surrogate_pair_to_code_point(ch, ch2);
                                ++first;
                                n = json::unicode::convert_one(code_point, dest, UTF_8_encoding_tag());
                            } else {
                                return E_TRAIL_EXPECTED;
                            } 
                        }
                        else {
                            return E_UNEXPECTED_ENDOFINPUT;
                        }
                    }
                    else {
                        return E_INVALID_START_BYTE;
                    }
            }
            
            
            // TODO implement filters
            
            return n;
#endif            
        }    
        
    };
    
    
    
    
#pragma mark - UTF-16 to UTF-16
    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;                

        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp;            
            int result = json::unicode::convert_one_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t());
            }
            return result;
        }    
    };
    
    // Specialization for filter type filter::None
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,    
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        typedef typename from_encoding_t::endian_tag            from_endian_t;
        typedef typename to_encoding_t::endian_tag              to_endian_t;
        typedef typename host_endianness::type                  host_endian_t;
        
        typedef json::unicode::filter::None                     NoFilter;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            utf16_code_unit cu = byte_swap<from_endian_t, host_endian_t>(static_cast<utf16_code_unit>(*first));
            int result = utf16_is_single(cu) ? 1 : 2;
            switch (result) {
                case 2: 
                    *dest++ = byte_swap<host_endian_t, to_endian_t>(cu);
                    ++first;
                    cu = byte_swap<from_endian_t, host_endian_t>(static_cast<utf16_code_unit>(*first));
                case 1:
                    *dest++ = byte_swap<host_endian_t, to_endian_t>(cu);
                    ++first;
            }
            return result;
        }    
    };
    

    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        inline int    
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const 
        {
            code_point_t cp;            
            int result = json::unicode::convert_one(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t());
            }
            return result;
        }    
    };    
    
#pragma mark - UTF-16 to UTF-32
    
    // Convert the first character contained in the well-formed UTF-16 sequence
    // [first, last) to a UTF-32 code point and copy the result to the 
    // sequence starting at dest.
    //
    // Parameter inFirst must point to the start of a UTF-16 sequence.
    // The output sequence must be large enough to hold the generated code 
    // point.
    //
    // On exit, first has been advanced by the number of code units constituting 
    // the character.
    //
    // Returns the number of generated code point [0,1].
    // 
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //
    // Errors:
    //   E_PREDICATE_FAILED
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp;            
            int result = json::unicode::convert_one_unsafe(
                first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(cp);
            }
            return result;
        }    
    };    
    
    // Convert the first character contained in the possibly mal-formed UTF-16 
    // sequence [first, last) to a UTF-32 code point and copy the result 
    // to the sequence starting at dest.
    //
    // Parameter inFirst should point to the start of a UTF-16 sequence.
    // The output sequence should be large enough to hold the generated code 
    // point.
    //
    // On exit, parameter first has been advanced by the number of code units 
    // constituting the character, or in case of an error, it points to the code 
    // unit where the error occured.
    //
    // Returns the number of generated code units [0, 1], or a negative 
    // number indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_PREDICATE_FAILED
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
    
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const 
        {
            code_point_t cp;
            int result = json::unicode::convert_one(
                first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                *dest = byte_swap<from_endian_t, to_endian_t>(cp);
                ++dest;
            }
            return result;
        }    
    };    

    
#pragma mark - UTF-32 to UTF-8
    
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-8 and copy the result to the sequence starting at dest.
    //
    // (see generic documentation)
    //
    // Returns the number of generated code point [0 .. 4].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    // 
    //  Errors:
    //    E_PREDICATE_FAILED
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_same<UTF_8_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        // Output iterators do not strictly require a value_type!
        //BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<OutIteratorT>::type)
        //                    == sizeof(typename OutEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef OutEncodingT                                    to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-8 and copy the result to the sequence starting at outFirst.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code point [0 .. 4], or a negative number
    // which indicates an error.
    //
    //  Errors:
    //  E_NONCHARACTER:         The given Unicode code point is a noncharacter.
    //  E_INVALID_CODE_POINT:   The Unicode code point is a surrogate or not
    //                          not a valid Unicode code point at all.
    //  E_PREDICATE_FAILED
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_same<UTF_8_encoding_tag, OutEncodingT>
            >
        >::type
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef OutEncodingT                                    to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        

        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_one(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    
    
#pragma mark - UTF-32 to UTF-16
    
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-16 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code point [0 .. 4].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
                
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t());
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    
    // Convert the first Unicode code point from the sequence [first,last) 
    // to UTF-16 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation)
    //
    // Returns the number of generated code point [0 .. 4], or a negative number
    // which indicates an error.
    //
    //  Errors:
    //  E_PREDICATE_FAILED: 
    //  all errors from convert_one(code_point, iterator, encoding)
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        

        inline int
        operator() (
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_one(cp, dest, to_encoding_t());
            if (result > 0) {
                ++first;                
            }
            return result;
        }    
    };
    
    
#pragma mark - UTF-32 to UTF-32
    
#pragma mark convert_one_unsafe
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code point [1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        typedef typename host_endianness::type          host_endian_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, host_endian_t>(*first));
            int result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    

    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code point [1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Errors:
    //  E_PREDICATE_FAILED
    //
#pragma mark convert_one    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        typename FilterT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        typedef typename host_endianness::type          host_endian_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, host_endian_t>(*first));
            int result = json::unicode::convert_one_unsafe(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    // Specialization for filter type equals filter::None
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code points [1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-32
    //
#pragma mark convert_one_unsafe<filter::None>    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one_unsafe<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag            from_endian_t;
        typedef typename to_encoding_t::endian_tag              to_endian_t;
        typedef typename host_endianness::type                  host_endian_t;
        
        typedef json::unicode::filter::None                     NoFilter;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            *dest++ = static_cast<utf32_code_unit>(byte_swap<from_endian_t, to_endian_t>(*first++));
            return 1;
        }    
    };
    
    // Specialization for filter type equals filter::None
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code points [1].
    //
    // Errors:
    //  E_INVALID_CODE_POINT
#pragma mark convert_one<filter::None>    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one<
        InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
                boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
            >
        >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));

        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag            from_endian_t;
        typedef typename to_encoding_t::endian_tag              to_endian_t;
        typedef typename host_endianness::type                  host_endian_t;
        
        typedef json::unicode::filter::None                     NoFilter;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, host_endian_t>(*first));
            if (isUnicodeScalarValue(cp)) {
                *dest++ = static_cast<code_point_t>(byte_swap<host_endian_t, to_endian_t>(cp));
                ++first;
                return 1;                
            }
            else {
                return E_INVALID_CODE_POINT;
            }
        }    
    };
    

}}}



#pragma mark - Functions convert_one, convert_one_unsafe 
#pragma mark One UTF to One UTF Conversions
namespace json { namespace unicode {
    
#pragma mark Description    
    ////////////////////////////////////////////////////////////////////////////
    //
    //  General synopsis
    //
    //  A.1
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIterator& dest, OutEncoding outEncoding);
    //
    //  A.2
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIterator& dest, OutEncoding outEncoding,
    //              Filter filter);
    //    
    //  A.3
    //  int 
    //  convert_one(InIterator& first, InIterator last, 
    //              OutIteratorT& dest)
    //
    //  A.4
    //  int 
    //  convert_one(InIterator& first, InIterator last,
    //              OutIteratorT& dest,
    //              Filter filter);
    //
    //  A.5
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIteratorT& dest)
    //
    //  A.6
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIteratorT& dest,
    //              Filter filter);
    //
    //  A.7
    //  int 
    //  convert_one(InIterator& first, InIterator last,
    //              OutIteratorT& dest, OutEncoding outEncoding)
    //
    //  A.8
    //  int 
    //  convert_one(InIterator& first, InIterator last,
    //              OutIteratorT& dest, OutEncoding outEncoding,
    //              Filter filter);
    //
    //
    //  B.1
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //                     OutIterator& dest, Encoding outEncoding);
    //
    //  B.2
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //                     OutIterator& dest, OutEncoding outEncoding,
    //                     Filter filter);
    //    
    //  B.3
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last,
    //                     OutIteratorT& dest)
    //
    //  B.4
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last,
    //                     OutIteratorT& dest,
    //                     Filter filter);
    //
    //  B.5
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //                     OutIteratorT& dest)
    //
    //  B.6
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //                     OutIteratorT& dest,
    //                     Filter filter);
    //
    //  B.7
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last,
    //                     OutIteratorT& dest, OutEncoding outEncoding)
    //
    //  B.8
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last,
    //                     OutIteratorT& dest, OutEncoding outEncoding,
    //                     Filter filter);
    //
    //        
    //
    //  Converts the first character constituted by the sequence of code units 
    //  in [first, last) encoded in inEncoding into a sequence of code units 
    //  using outEncoding and copies the result to the sequence starting at dest.
    //
    //  All convert_one functions require that the input sequence is not empty,
    //  thus, initially first must point to a dereferencable value.
    //
    //  Returns the number of generated code units copied to the destination,
    //  or a negative number indicating an error.
    //
    //  On exit, the input iterator first will be incremented by the number of 
    //  code units constituting one character. The output iterator dest will be
    //  incrementend by the number of generated code units. If an error occured,
    //  both iterators will be incremented to the point where the error has been
    //  detected.
    //
    //  Encodings specified without endianness will be "upgraded" to its corres-
    //  ponding encoding including the host endianness. If required, byte-swap-
    //  ping is applied to ensure endianness as sepcified.
    //
    //  The function requires that the destination range is large enough to hold
    //  the number of generated code units. Otherwise, it is assumed an exception 
    //  will be thrown by the iterator, or the behavior is undefined.
    //
    //  The unsafe version will assume a well-formed non-empty input sequence, 
    //  otherwise the result and the behavior is undefined. Unsafe conversion 
    //  functions will almost always return a positive value as its return value, 
    //  though this is no guarantee that the input sequence was well-formed, and
    //  that the result is reliable. More over, when passed a mal-formed input 
    //  sequence it can happen that the program state becomes corrupt and as a 
    //  result the program may crash subsequently.
    //  Unsafe versions do not test for an Unicode noncharacter an pass success-
    //  ful with the corresponding code units generated.
    //  
    //  The safe versions apply a number of checks to test if the input it is 
    //  well-formed. 
    //  The number and kind of tests will be reflected by the possible errors 
    //  which will be returned by the function.
    //  The checked versions fails if the input constitutes a mal-formed Unicode 
    //  sequence and no output will be generated. 
    // 
    //  Filters can be specified to take fine control about which Unicode code
    //  points will be rejected and cause the function to return an error code, 
    //  respectively which Unicode code points will be replaced by a certain 
    //  replacement character which will then be put back to the output iterator.
    //  The default filter does not filter any Unicode code point.
    //
    //  The output iterator will never be checked by the conversion functions.
    //  It is the responsibility of the caller that the requirements are met.
    //
    ////////////////////////////////////////////////////////////////////////////
    
    
#pragma mark convert_one 
    // A.1
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIterator& dest, OutEncoding outEncoding);
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline int 
    convert_one(
        InIteratorT& first, InIteratorT last, InEncodingT inEncoding, 
        OutIteratorT& dest, OutEncodingT outEncoding,
        typename boost::enable_if<
            boost::mpl::and_<
               boost::is_base_and_derived<utf_encoding_tag, InEncodingT>,
               boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>
            >
        >::type* dummy = 0)
    {
        typedef internal::convert_one<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncodingT, 
            filter::None>                    converter_t;

        return converter_t()(first, last, dest);
    }
    
    // A.2
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIterator& dest, OutEncoding outEncoding,
    //              Filter filter);
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT,
        typename FilterT
    >
    inline int 
    convert_one(InIteratorT& first, InIteratorT last, InEncodingT inEncoding, 
                OutIteratorT& dest, OutEncodingT outEncoding,
                FilterT filter,
                typename boost::enable_if<
                boost::mpl::and_<
                boost::is_base_and_derived<utf_encoding_tag, InEncodingT>,
                boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>,
                boost::is_base_and_derived<filter::filter_tag, FilterT>
                >
                >::type* dummy = 0)
    {
        typedef internal::convert_one<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncodingT, 
        FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }

    // A.3
    //  int 
    //  convert_one(InIterator& first, InIterator last, 
    //              OutIteratorT& dest)
    template <
        typename InIteratorT, 
        typename OutIteratorT
    >
    inline int 
    convert_one(
        InIteratorT& first, InIteratorT last, 
        OutIteratorT& dest)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;
        typedef internal::convert_one<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncoding, 
        filter::None>                    converter_t;
        
        return converter_t()(first, last, dest);
    }
    
    // A.4
    //  int 
    //  convert_one(InIterator& first, InIterator last,
    //              OutIteratorT& dest,
    //              Filter filter);
    template <
        typename InIteratorT,
        typename OutIteratorT,
        typename FilterT
    >
    inline int 
    convert_one(InIteratorT& first, InIteratorT last, 
                OutIteratorT& dest,
                FilterT filter,
                typename boost::enable_if<
                    boost::is_base_and_derived<filter::filter_tag, FilterT>
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;        
        typedef internal::convert_one<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncoding, 
        FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }
    
    
    //  A.5
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIteratorT& dest)
    template <
        typename InIteratorT, 
        typename InEncodingT,
        typename OutIteratorT
    >
    inline int 
    convert_one(
                InIteratorT& first, InIteratorT last, InEncodingT inEncoding,
                OutIteratorT& dest)
    {
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;
        typedef internal::convert_one<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncoding, 
        filter::None>                    converter_t;
        
        return converter_t()(first, last, dest);
    }
    
    
    //
    //  A.6
    //  int 
    //  convert_one(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIteratorT& dest,
    //              Filter filter);
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT,
        typename FilterT
    >
    inline int 
    convert_one(InIteratorT& first, InIteratorT last, InEncodingT inEncoding, 
                OutIteratorT& dest, 
                FilterT filter,
                typename boost::enable_if<
                    boost::mpl::and_<
                        boost::is_base_and_derived<utf_encoding_tag, InEncodingT>,
                        boost::is_base_and_derived<filter::filter_tag, FilterT>
                    >
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;        
        typedef internal::convert_one<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncoding, 
            FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }
    
    
    //
    //  A.7
    //  int 
    //  convert_one(InIterator& first, InIterator last,
    //              OutIteratorT& dest, OutEncoding outEncoding)
    template <
        typename InIteratorT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline int 
    convert_one(InIteratorT& first, InIteratorT last,  
                OutIteratorT& dest, OutEncodingT outEncoding,
                typename boost::enable_if<
                    boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;        
        typedef internal::convert_one<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncodingT, 
            filter::None>                    converter_t;
        
        return converter_t()(first, last, dest);
    }
    
    //
    //  A.8
    //  int 
    //  convert_one(InIterator& first, InIterator last,
    //              OutIteratorT& dest, OutEncoding outEncoding,
    //              Filter filter);
    template <
        typename InIteratorT, 
        typename OutIteratorT, typename OutEncodingT,
        typename FilterT
    >
    inline int 
    convert_one(InIteratorT& first, InIteratorT last,  
                OutIteratorT& dest, OutEncodingT outEncoding,
                FilterT filter,
                typename boost::enable_if<
                    boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;        
        typedef internal::convert_one<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncodingT, 
            FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }

    
    
#pragma mark convert_one_unsafe 

    //  B.1
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //                     OutIterator& dest, Encoding outEncoding);
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline int convert_one_unsafe(
        InIteratorT& first, InIteratorT last, InEncodingT inEncoding, 
        OutIteratorT& dest, OutEncodingT outEncoding,
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_and_derived<utf_encoding_tag, InEncodingT>,
                boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>
            >
        >::type* dummy = 0)
    {
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncodingT, 
            filter::None>                    converter_t;
        
        return converter_t()(first, last, dest);
    }
    
    
    //  B.2
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //                     OutIterator& dest, OutEncoding outEncoding,
    //                     Filter filter);
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT,
        typename FilterT
    >
    inline int 
    convert_one_unsafe(
       InIteratorT& first, InIteratorT last, InEncodingT inEncoding, 
       OutIteratorT& dest, OutEncodingT outEncoding, 
       FilterT filter,
       typename boost::enable_if<
           boost::mpl::and_<
               boost::is_base_and_derived<utf_encoding_tag, InEncodingT>,
               boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>,
               boost::is_base_and_derived<filter::filter_tag, FilterT>
           >
        >::type* dummy = 0)
    {
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncodingT, 
            FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }
    
    // B.3
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, 
    //              OutIteratorT& dest)
    template <
    typename InIteratorT, 
    typename OutIteratorT
    >
    inline int 
    convert_one_unsafe(
                InIteratorT& first, InIteratorT last, 
                OutIteratorT& dest)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncoding, 
        filter::None>                    converter_t;
        
        return converter_t()(first, last, dest);
    }
    
    // B.4
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last,
    //              OutIteratorT& dest,
    //              Filter filter);
    template <
        typename InIteratorT,
        typename OutIteratorT,
        typename FilterT
    >
    inline int 
    convert_one_unsafe(InIteratorT& first, InIteratorT last, 
                OutIteratorT& dest,
                FilterT filter,
                typename boost::enable_if<
                       boost::is_base_and_derived<filter::filter_tag, FilterT>
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;        
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncoding, 
        FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }
    
    
    //  B.5
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIteratorT& dest)
    template <
        typename InIteratorT, 
        typename InEncodingT,
        typename OutIteratorT
    >
    inline int 
    convert_one_unsafe(
                InIteratorT& first, InIteratorT last, InEncodingT inEncoding,
                OutIteratorT& dest)
    {
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncoding, 
        filter::None>                    converter_t;
        
        return converter_t()(first, last, dest);
    }
    
    
    //
    //  B.6
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last, InEncoding inEncoding,
    //              OutIteratorT& dest,
    //              Filter filter);
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT,
        typename FilterT
    >
    inline int 
    convert_one_unsafe(InIteratorT& first, InIteratorT last, InEncodingT inEncoding, 
                OutIteratorT& dest, 
                FilterT filter,
                typename boost::enable_if<
                    boost::mpl::and_<
                        boost::is_base_and_derived<utf_encoding_tag, InEncodingT>,
                        boost::is_base_and_derived<filter::filter_tag, FilterT>
                    >
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<OutIteratorT>::type OutEncoding;        
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncodingT, 
            OutIteratorT, OutEncoding, 
        FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }
    
    
    //
    //  B.7
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last,
    //              OutIteratorT& dest, OutEncoding outEncoding)
    template <
        typename InIteratorT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline int 
    convert_one_unsafe(InIteratorT& first, InIteratorT last,  
                OutIteratorT& dest, OutEncodingT outEncoding,
                typename boost::enable_if<
                    boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;        
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncodingT, 
        filter::None>                    converter_t;
        
        return converter_t()(first, last, dest);
    }
    
    //
    //  B.8
    //  int 
    //  convert_one_unsafe(InIterator& first, InIterator last,
    //              OutIteratorT& dest, OutEncoding outEncoding,
    //              Filter filter);
    template <
        typename InIteratorT, 
        typename OutIteratorT, typename OutEncodingT,
        typename FilterT
    >
    inline int 
    convert_one_unsafe(InIteratorT& first, InIteratorT last,  
                OutIteratorT& dest, OutEncodingT outEncoding,
                FilterT filter,
                typename boost::enable_if<
                    boost::is_base_and_derived<utf_encoding_tag, OutEncodingT>
                >::type* dummy = 0)
    {
        typedef typename iterator_encoding<InIteratorT>::type InEncoding;        
        typedef internal::convert_one_unsafe<
            InIteratorT, InEncoding, 
            OutIteratorT, OutEncodingT, 
        FilterT>                    converter_t;
        
        return converter_t()(first, last, dest, filter);
    }
    
    
    
}}



#pragma mark - Convert Sequence to Sequence
namespace json { namespace unicode {
    
    
    // Convert the characters contained in the well-formed UTF sequence 
    // [first, last) encoded in inEncoding to a sequence of code units 
    // encoded in outEncoding and copy the result to the sequence starting at 
    // dest.
    //
    // Parameter first must point to the start of a UTF multi byte sequence
    // or to a Unicode code point.
    // The output sequence must be large enough to hold the generated code 
    // points.
    //
    // On exit, if the function was successful, first will be advanced until 
    // last, and dest will be advanced as much as needed to store the 
    // generated code points. Otherwise, if the the function failed, both
    // iterators will be advanced to the point where the error occured.
    //
    // If the function fails, parameter error will be set accordingly to an 
    // negative number indicating the error - otherwise, it will be set to 0.
    //
    // Returns the number of generated code units. If the function fails, it 
    // returns zero, however the value "0" may also be a valid number of 
    // generated code units IFF the input string is empty. The result state of 
    // the conversion is always indicated by paramerer error, though.
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode. It is generally dangerous to call this function
    // with a mal-formed input.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid number of trail bytes.
    //  - does not check if first is a valid start byte for UTF-8
    //  - does not check for noncharacters
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT>
    inline std::size_t
    convert_unsafe(
                   InIteratorT&     first, 
                   InIteratorT      last, 
                   InEncodingT      inEncoding,
                   OutIteratorT&    dest,
                   OutEncodingT     outEncoding,
                   int&             error)
    {
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one_unsafe(first, last, inEncoding, 
                                            dest, outEncoding);
            if (result > 0)
                count += result;
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    

    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT,
    typename FilterT 
    >
    inline std::size_t
    convert_unsafe(
                   InIteratorT&     first, 
                   InIteratorT      last, 
                   InEncodingT      inEncoding,
                   OutIteratorT&    dest,
                   OutEncodingT     outEncoding,
                   FilterT          filter,
                   int&             error)
    {
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one_unsafe(first, last, inEncoding, 
                                            dest, outEncoding, filter);
            if (result > 0)
                count += result;
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    
    
    // Convert the characters contained in the possibly malformed UTF sequence 
    // [first, last) encoded in inEncoding to a sequence of code units 
    // encoded in outEncoding and copy the result to the sequence starting at 
    // dest.
    //
    // Parameter first should point to the start of a UTF multi byte sequence
    // or to an Unicode code point.
    // The output sequence shall be large enough to hold the generated code 
    // points.
    //
    // On exit, if the function was successful, parameter first will be 
    // advanced until last, and dest will be advanced as much as needed to 
    // store the generated code points. Otherwise, if the the function failed, 
    // both iterators will be advanced to the point where the error occured.
    //
    // If the function fails, parameter error will be set accordingly to an 
    // negative number indicating the error - otherwise, it will be set to 0.
    //
    // Returns the number of generated code units. If the function fails, it 
    // returns zero, however the value "0" may also be a valid number of 
    // generated code units IFF the input string is empty. The result state of 
    // the conversion is always indicated by paramerer error, though.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT>
    inline std::size_t
    convert(
            InIteratorT&     first, 
            InIteratorT      last, 
            InEncodingT      inEncoding,
            OutIteratorT&    dest,
            OutEncodingT     outEncoding,
            int&             error)
    {
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one(first, last, inEncoding, 
                                     dest, outEncoding);
            if (result > 0)
                count += result;
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT,
    typename FilterT>
    inline std::size_t
    convert(
            InIteratorT&     first, 
            InIteratorT      last, 
            InEncodingT      inEncoding,
            OutIteratorT&    dest,
            OutEncodingT     outEncoding,
            FilterT          filter,
            int&             error)
    {
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one(first, last, inEncoding, 
                                     dest, outEncoding, filter);
            if (result > 0)
                count += result;
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    
    
    //
    //  Convert Safe
    //
    // Convert the characters contained in the possibly malformed UTF sequence 
    // [first, last) encoded in inEncoding to a sequence of code units 
    // encoded in outEncoding and copy the result to the sequence starting at 
    // dest.
    //
    // If a malformed Unicode sequence, or an invalid Unicode character will be 
    // detected, the function replaces it with the Unicode replacement 
    // character. There will be no filter applied to the characters, that is, 
    // Unicode noncharacters will be converted.
    //
    // Parameter first should point to the start of a UTF multi byte sequence
    // or to an Unicode code point.
    // The output sequence shall be large enough to hold the generated code 
    // points.
    //
    // On exit, parameter first will be advanced until last, and dest will be 
    // advanced as much as needed to store the generated code points. 
    //
    // Returns the number of generated code units. If the function fails, it 
    // returns zero, however the value "0" may also be a valid number of 
    // generated code units IFF the input string is empty. The result state of 
    // the conversion is always indicated by paramerer error, though.
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT>
    inline std::size_t
    convert_safe(
            InIteratorT&     first, 
            InIteratorT      last, 
            InEncodingT      inEncoding,
            OutIteratorT&    dest,
            OutEncodingT     outEncoding)
    {
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one(first, last, inEncoding, dest, outEncoding);
            if (result > 0)
                count += result;
            else {
                if (first != last)
                    ++first;
                result = convert_one_unsafe(kReplacementCharacter, dest, outEncoding);
                assert(result > 0);
                count += result;                
            }
        }
        return count;
    }    
    
    
    
    
#pragma mark - Unicode Code Point to UTF
    
    
    // Convert the Unicode code points contained in the sequence [first, last) 
    // to a sequence of code units encoded in outEncoding and copy the result 
    // to the sequence starting at dest.
    //
    // The output sequence shall be large enough to hold the generated code 
    // points.
    //
    // On exit, if the function was successful, parameter first will be 
    // advanced until last, and dest will be advanced as much as needed to 
    // store the generated code points. Otherwise, if the the function failed, 
    // both iterators will be advanced to the point where the error occured.
    //
    // If the function fails, parameter error will be set accordingly to an 
    // negative number indicating the error - otherwise, it will be set to 0.
    //
    // Returns the number of generated code units. If the function fails, it 
    // returns zero, however the value "0" may also be a valid number of 
    // generated code units IFF the input string is empty. The result state of 
    // the conversion is always indicated by paramerer error, though.
    //
    // Errors:
    //  E_INVALID_CODE_POINT
    //      
    //
    template <
        typename InIteratorT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline std::size_t
    convert_codepoints(
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            OutEncodingT     outEncoding,
            int&             error)
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(code_point_t));
        
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one(static_cast<code_point_t>(*first),
                                     dest, outEncoding);
            if (result > 0) {
                count += result;
                ++first;
            }
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    
    template <
        typename InIteratorT,
        typename OutIteratorT, typename OutEncodingT,
        typename FilterT
    >
    inline std::size_t
    convert_codepoints(
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            OutEncodingT     outEncoding,
            FilterT          filter,
            int&             error)
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(code_point_t));
        
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one(static_cast<code_point_t>(*first),
                                     dest, outEncoding, filter);
            if (result > 0) {
                count += result;
                ++first;
            }
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    
    
    template <
        typename InIteratorT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline std::size_t
    convert_codepoints_unsafe(
            InIteratorT&     first, 
            InIteratorT      last, 
            OutIteratorT&    dest,
            OutEncodingT     outEncoding,
            int&             error)
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(code_point_t));
        
        
        std::size_t count = 0;
        while (first != last) {
            int result = convert_one_unsafe(static_cast<code_point_t>(*first),
                                     dest, outEncoding);
            if (result > 0) {
                count += result;
                ++first;
            }
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    

    
}}

#endif  // JSON_UNICODE_UNICODE_CONVERSIONS_HPP

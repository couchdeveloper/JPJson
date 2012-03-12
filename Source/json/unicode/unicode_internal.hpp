//
//  unicode_internal.hpp
//  
//
//  Created by Andreas Grosam on 1/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//



#ifndef _unicode_internal_hpp
#define _unicode_internal_hpp

#error This file is deprecated!


#include "json/config.hpp"
#include "unicode_utilities.hpp"
#include "unicode_errors.hpp"
//#include "unicode_filter.hpp"

#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>



#pragma mark - Internal Base Templates  convert_codepoint, convert_codepoint_unsafe
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
    
    
}}} // namespace json::unicode::internal



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
    
    
    
}}}   





#endif

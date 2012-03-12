//
//  parse_string.hpp
//  
//
//  Created by Andreas Grosam on 3/4/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_PARSE_STRING_HPP
#define JSON_PARSER_PARSE_STRING_HPP

#warning Not Yet Finished


#include "unicode/unicode_converter.hpp"
#include "string_buffer.hpp"


namespace json { namespace parser namespace internal {
    
    
    template <
        typename FromEncodingT, 
        typename ToEncodingT,
        int Validation = 0,
        class Enable = void
    >
    struct string_parser {
        
        BOOST_STATIC_ASSERT_MSG(sizeof(Enable) == 0, "Base Template Shall Not Be Instantiated");
        
        template <typename InIteratorT, typename OutIteratorT>
        int 
        parse(InIteratorT& first, InIteratorT last, OutIteratorT& des);
        
    };
    
    
    template <typename OutIterator>
    struct utf8_json_string_actions 
    {         
        typedef typename mb_state<UTF_8_encoding_tag>::buffer_type buffer_type;
        
        utf8_json_string_actions(OutIterator& dest) : dest_(dest), result_(0) {}
        
        bool operator!() const { return result_ != 0; }  // always parse only one character
        
        int result() const { return result_; }
        
        
        void 
        action_single_byte(const buffer_type& buffer) {
            uint32_t c = encoding_traits<UTF_8_encoding_tag>::to_uint(buffer[0]);
            if (c >= 0x20) {
                if (c == '/') {
                    result_ = escape_sequence(first, last, dest);
                } else if (c == '"') {
                    result = -1;  // done
                }
                if (result_ != 0) {
                    // error parsing escape sequence
                    return result; // error state already set
                }
            }
            else if (c == 0) {
                
            }
            
        }
        
        void 
        action_double_byte(const buffer_type& buffer) {
            *dest_++ = buffer[0];
            *dest_++ = buffer[1];
        }
        
        void 
        action_triple_byte(const buffer_type& buffer) {
            *dest_++ = buffer[0];
            *dest_++ = buffer[1];
            *dest_++ = buffer[2];
        }
        
        void 
        action_quad_byte(const buffer_type& buffer) {
            *dest_++ = buffer[0];
            *dest_++ = buffer[1];
            *dest_++ = buffer[2];
            *dest_++ = buffer[3];
        }
        
    private: 
        int result_;
        OutIterator& dest_;            
    };

    template <typename EncodingT, typename OutIterator>
    struct utf16_json_string_actions 
    {         
        typedef mb_state<UTF_16_encoding_tag>::buffer_type buffer_type;
        
        utf16_json_string_actions(OutIterator& dest) : dest_(dest) {}
        
        bool operator!() const { return true; }  // always parse only one character
        
        void action_single_byte(const buffer_type&) {}
        void action_double_byte(const buffer_type&) {}
        void action_triple_byte(const buffer_type&) {}
        void action_surrogate_pair(const buffer_type&) {}
        
    private:        
        OutIterator& dest_;            
    };
    
    

    
    

#pragma mark - string_parser
    //
    // struct string_parser
    //
    template <typename FromEncodingT, typename ToEncodingT, int Validation>
    struct string_parser<FromEncodingT, ToEncodingT, Validation,
    typename boost::enable_if<
        boost::mpl::and_<
        boost::is_same<UTF_8_encoding_tag, FromEncodingT>,
        boost::is_same<code_point_t, ToEncodingT>
        >
    >::type
    > 
    {        
        typedef internal::utf8_parser<
            internal::utf8_validation_traits<Validation>::no_check_input_range, 
            internal::utf8_validation_traits<Validation>::no_check_trails
        >                                                           parser_type;        
    public:
        typedef internal::mb_state<UTF_8_encoding_tag> mb_state_type;
        
    private:   
        mb_state_type state_;
        
    public:
        string_parser() : state_() {}
        string_parser(mb_state_type const& state) : state_(state) {}
        
        mb_state_type state() const { return state_; }
        void clear() { state_.clear(); }
        
        
        /**        
         Convert a sequence of Unicode characters in encoding form UTF-8
         to a corresponding sequence of Unicode code points writing it into
         output iterator 'dest'.
         The converter keeps state information required to parse partial 
         multibyte sequences.
         
         If the converter starts with an initial input sequence, the state of
         the converter shall be cleared. The state is cleared after construction
         or after clear_state() has been called.
         
         If the sequence stops at a partial multibyte sequence - which was 
         valid up to that point, the function returns E_UNEXPECTED_ENDOFINPUT 
         and the conversion state is set such that a subsequent call of 
         convert() can be proceed with the continuing sequence starting at 
         the offset immediately following the last byte. The sequences can be
         divided at any code unit boundary.
         
         Returns zero if conversion was successful and complete. Otherwise
         returns E_UNEXPECTED_ENDOFINPUT indicating a partial multi-byte 
         sequecne at the end, otherwise a negative number indicating any 
         other error.
         
         Errors:
         E_TRAIL_EXPECTED:           trail byte expected or trail byte yields an unconvertable offset.
         E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
         E_INVALID_START_BYTE:       invalid start byte
         E_INVALID_CODE_POINT:       detected surrogate or code point is invalid Unicode 
         
         
         According the "Constraints on Conversion process" in the Unicode
         Standard v6, this algorithm conforms to the concept of "maximal sub-
         part".
         
         Safe version:
         
         - checks for EOF
         - checks for valid UTF input
         
         
         Unsafe version:
         
         The Unicode sequence shall be well-formed, otherwise the result is
         undefined.
         
         - does not check for valid start byte
         - does not check for valid trail byte
         - does not check for valid number of trail bytes.
         - does not ensure that first equals last on exit
         - may crash if input is not well-formed.
         */
        
        template <typename InIteratorT, typename OutIteratorT>
        int parse(InIteratorT& first, InIteratorT last, OutIteratorT& dest)
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                                == sizeof(typename encoding_traits<FromEncodingT>::code_unit_type));

            typedef internal::json_string_actions<OutIteratorT>                 actions_t;            
            typedef converter<FromEncodingT, unicode::code_point_t, Validation, false, true>    converter_t;
            
            converter_t cvt;
            
            assert(first != last);
            assert( unicode::encoding_traits<FromEncodingT>::to_uint(*first) == '"');
                        
            while (first != last) {
                unsigned int c = unicode::encoding_traits<FromEncodingT>::to_uint(*first);
                if (__builtin_expect((c - 0x20u) < 0x60u, 1))  // [0x20 .. 0x7F]  ASCII except control-char and except zero
                {
                    switch (c) {
                        default:
                            ++first;
                            string_buffer_pushback_ASCII(stringBuffer, c);
                            continue;
                        case '\\': // escape sequence
                            ++first;
                            int result = escape_sequence(stringBuffer);
                            if (!result) {
                                // error parsing escape sequence
                                return result; // error state already set
                            }
                            continue;
                        case '"':
                            ++first;
                            return 0; // done
                    }                
                } else if (c > 0x7Fu) {
                    // Reading UTF-8/UTF-16 multi byte sequences or UTF-32.
                    // Convert the UTF character into a code point:
                    // Use a conversions which does not accept surrogates and noncharacters. 
                    // Per default, the filter shall signal errors if it matches characters.
                    //  //Replace invalid characters with Unicode Replacement Character.
                    // The "safe" conversion does not accept surrogates, so we only need to
                    // check for Unicode noncharacters and ASCII control characters:
                    
                    code_point_t cp;
                    code_point_t cp_ptr = &cp;
                    int cvt_result = cvt.convert(first, last, cp_ptr);
                    assert(cvt.state() == 0);
                    if (cvt_result == 0) {
                        // Check for Unicode noncharacters
                    }
                    else {
                        return JP_INVALID_UNICODE_ERROR;
                    }
                } 
                else /*if (c < 0x20u)*/ { // Control char or zero
                    if (c == 0) {
                        ++first;
                        string_buffer_pushback_ASCII(stringBuffer, c);
                        continue;
                    
                    } else {
                        return JP_CONTROL_CHAR_NOT_ALLOWED_ERROR;  // error: control character not allowed
                    }
                }
            
            }  // while 
                        
            
            
            return result;
        }
        
    };  // class string_parser UTF-8 to codepoints
    
    
}}}




namespace json {  namespace parser {
    
    
#define USE_LOOKUP_TABLE
    
    //
    // Read four consequtive digits and return the corresponding value
    // as a uint16_t number. This is part of parsing escaped unicode sequences.
    // Escaped unicode characters are really only reasonable for UTF-8 encoded 
    // input, but any UTF may have them.
    //
    // In case of an error, error state will be set and the result is undefined.
    //
    template <
    typename InputIterator
    , typename SourceEncoding
    , typename SemanticActions
    >
    inline 
    uint16_t 
    parser<InputIterator, SourceEncoding, SemanticActions>::
    hex() 
    {
        assert(state_.error() == JP_NO_ERROR);
        
#if defined (USE_LOOKUP_TABLE)        
        static const int8_t lookupTable[] = {
            //          '0' '1' '2' '3' '4' '5' '6' '7'   '8' '9'
            0,  1,  2,  3,  4,  5,  6,  7,    8,  9, -1, -1, -1, -1, -1, -1, // 48 .. 63
            //              'A' 'B' 'C' 'D' 'E' 'F  
            -1, 10, 11, 12, 13, 14, 15, -1,   -1, -1, -1, -1, -1, -1, -1, -1, // 64 .. 79
            //
            -1, -1, -1, -1, -1, -1, -1, -1,   -1, -1, -1, -1, -1, -1, -1, -1, // 80 .. 64 
            //              'a' 'b' 'c' 'd' 'e' 'f' 
            -1, 10, 11, 12, 13, 14, 15, -1,   -1, -1, -1, -1, -1, -1, -1, -1  // 96 .. 111 
        };
#endif        
        uint16_t uc = 0;        
        // expecting 4 hex digits:
        for (int i = 0; i < 4; ++i, ++p_) {
            if (__builtin_expect(p_ != last_, 1)) 
            {
#if defined (USE_LOOKUP_TABLE)
                uint32_t c = static_cast<uint32_t>(*p_) - 48u;
                // check range:
                int v;
                if ( (c) < (111-48) and (v = lookupTable[c]) >= 0) {
                    uc = (uc << 4) + v;
                }
                else {
                    state_.error() = JP_INVALID_HEX_VALUE_ERROR;
                    sa_.error(state_.error(), state_.error_str());
                    return 0;
                }
#else                
                code_t  c = *p_;
                switch (c) {
                    case '0' ... '9': uc = (uc << 4) + (unsigned int)(c - '0'); break;
                    case 'A' ... 'F': uc = (uc << 4) + ((unsigned int)(c - 'A') + 10U); break;
                    case 'a' ... 'f': uc = (uc << 4) + ((unsigned int)(c - 'a') + 10U); break;
                    default:
                        state_.error() = JP_INVALID_HEX_VALUE_ERROR;
                        sa_.error(state_.error(), state_.error_str());
                        return 0;
                }
#endif                
            }
            else {
                state_.error() = JP_UNEXPECTED_END_ERROR;
                sa_.error(state_.error(), state_.error_str());
                return 0;
            }
        }
        return uc;
    }
    
    
    //
    // Return the unicode code point of an escaped unicode character sequence.
    // Escaped unicode characters are really only reasonable for UTF-8 encoded 
    // input, but any UTF encoding form may have them.
    // Any character may be escaped. If the character is in the Basic
    // Multilingual Plane (U+0000 through U+FFFF), then it may be represented
    // as a six-character sequence: a reverse solidus, followed by the lowercase 
    // letter u, followed by four hexadecimal digits that encode the character's 
    // code point.  The hexadecimal letters A through F can be upper or lower-
    // case.  So, for example, a string containing only a single reverse solidus 
    // character may be represented as "\u005C".
    // 
    // Alternatively, there are two-character sequence escape representations of
    // some popular characters. So, for example, a string containing only a single
    // reverse solidus character may be represented more compactly as "\\".
    // 
    // To escape an extended character that is not in the Basic Multilingual
    // Plane, the character is represented as a twelve-character sequence,
    // encoding the UTF-16 surrogate pair.  So, for example, a string
    // containing only the G clef character (U+1D11E) may be represented as
    // "\uD834\uDD1E".
    //
    // Returns an Unicode scalar value. If the Unicode code point is not a 
    // valid Unicode scalar value, error state will be set and the 
    // Unicode Replacement character will be returned.
    
    template <
    typename InputIterator
    , typename SourceEncoding
    , typename SemanticActions
    >
    inline 
    unicode::code_point_t 
    parser<InputIterator, SourceEncoding, SemanticActions>::
    escaped_unicode() 
    {
        typedef typename json::unicode::UTF_16_encoding_traits::code_unit_type utf16_code_unit;
        
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(*p_ == 'u');
        
        ++p_;
        utf16_code_unit euc1 = hex();
        if (__builtin_expect(state_, 1)) 
        {
            unicode::code_point_t unicode = euc1;
            if (unicode::isCodePoint(unicode)) {
                if (not unicode::isSurrogate(unicode)) {
                    return unicode;
                }
                else {
                    if (unicode::utf16_is_high_surrogate(euc1)) {
                        if (match("\\u", 2)) {
                            utf16_code_unit euc2 = hex();
                            if (state_) {
                                if (unicode::utf16_is_low_surrogate(euc2)) {
                                    unicode = unicode::utf16_surrogate_pair_to_code_point(euc1, euc2);
                                    return unicode;  // success, exit and return unicode
                                } else {
                                    state_.error() = JP_EXPECTED_LOW_SURROGATE_ERROR;
                                }
                            }
                            else {
                                // bad hex quad for unicode. error state is already set.
                                return unicode::kReplacementCharacter;
                            }
                        } else {
                            if (state_.error() != JP_UNEXPECTED_END_ERROR) {
                                state_.error() = JP_EXPECTED_LOW_SURROGATE_ERROR;
                            }
                        }
                    } 
                    else {
                        state_.error() = JP_EXPECTED_HIGH_SURROGATE_ERROR;  // got low surrogate
                    }
                }
            }
            else {
                state_.error() = JP_INVALID_UNICODE_ERROR;
            }
            
            assert(state_.error() != 0);
            sa_.error(state_.error(), state_.error_str());
        }
        else {
            // state already set.
        }
        
        assert(state_.error() != 0);
        return unicode::kReplacementCharacter;
    }
    
    
    
    
    
    //
    // Parse an escape sequence
    //
    // p_ shall point past the initial escape character ('\').
    template <
    typename InputIterator
    , typename SourceEncoding
    , typename SemanticActions
    >
    inline 
    void 
    parser<InputIterator, SourceEncoding, SemanticActions>::
    escape_sequence(string_buffer_base_t& stringBuffer) 
    {
        assert(state_.error() == JP_NO_ERROR);        
        
        if (__builtin_expect(p_ != last_, 1)) 
        {
            code_t c = *p_;
            switch (c) {
                case code_t('"'): string_buffer_pushback_ASCII(stringBuffer, '"');  ++p_; return;
                case code_t('\\'):string_buffer_pushback_ASCII(stringBuffer, '\\'); ++p_; return;
                case code_t('/'): string_buffer_pushback_ASCII(stringBuffer, '/');  ++p_; return;
                case code_t('b'): string_buffer_pushback_ASCII(stringBuffer, '\b'); ++p_; return;
                case code_t('f'): string_buffer_pushback_ASCII(stringBuffer, '\f'); ++p_; return;
                case code_t('n'): string_buffer_pushback_ASCII(stringBuffer, '\n'); ++p_; return;
                case code_t('r'): string_buffer_pushback_ASCII(stringBuffer, '\r'); ++p_; return;
                case code_t('t'): string_buffer_pushback_ASCII(stringBuffer, '\t'); ++p_; return;
                case code_t('u'):  { // escaped unicode
                    unicode::code_point_t unicode = escaped_unicode();
                    if (state_) {
                        int result = string_buffer_pushback_unicode(stringBuffer, unicode);
                        if (result > 0) {
                            return;  // success 
                        } else if (result < 0) {
                            // detected Unicode noncharacter and rejected it,
                            // or U+0000 which is not allowed
                            assert(state_.error() != 0);                            
                        } else  {
                            // could not append unicode to internal string buffer
                            state_.error() = JP_INTERNAL_LOGIC_ERROR;
                        } 
                    } 
                    else {
                        // malformed unicode: not an Unicode scalar value,
                        // or unexpected end.
                        assert(state_.error() != 0);
                        return;
                    }
                    break;
                }
                default:
                    state_.error() = JP_INVALID_ESCAPE_SEQ_ERROR;
                    
            } // switch
        }
        else {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        
        assert(state_.error() != 0);
        sa_.error(state_.error(), state_.error_str());
    }
    
    
    //  parse_string()
    //
    void parse_string(string_buffer_base_t& stringBuffer) 
    {
        assert(state_.error() == JP_NO_ERROR); 
        assert(p_ != last_);
        assert(*p_ == code_t('"'));
        
        stringBuffer.reset();
        
        ++p_;
        uint32_t c;
        while (__builtin_expect(p_ != last_, 1)) 
        {
            // fast path: read ASCII (no control characters)
            c = static_cast<uint32_t>(*p_);
            if (__builtin_expect((c - 0x20u) < 0x60u, 1))  // ASCII except control-char
            {
                switch (c) {
                    default:
                        ++p_;
                        string_buffer_pushback_ASCII(stringBuffer, c);
                        continue;
                    case '\\': // escape sequence
                        ++p_;
                        escape_sequence(stringBuffer);
                        if (!state_) {
                            // error parsing escape sequence
                            return; // error state already set
                        }
                        continue;
                    case '"':
                        ++p_;
                        skip_whitespaces();
                        return; // done
                }
            }
            
            // slow path: reading UTF multi byte sequences and any other UTF encoding form                
            // read the UTF character into a code point:
            // Use a conversions which does not accept surrogates and noncharacters. 
            // Per default, the filter shall signal errors if it matches characters.
            //  //Replace invalid characters with Unicode Replacement Character.
            // The "safe" conversion does not accept surrogates, so we only need to
            // check for Unicode noncharacters and ASCII control characters:
            
            // Need to check for ASCII control chars here
            if ((c - 1u) < (0x20u - 1u)) {
                state_.error() =  JP_CONTROL_CHAR_NOT_ALLOWED_ERROR;  // error: control character not allowed
                break;
            }
            unicode::code_point_t cp;
            int result = unicode::convert_one(p_, last_, SourceEncoding(), cp, unicode_filter_);
            // Possible errors:
            //      E_TRAIL_EXPECTED:           trail byte expected
            //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
            //      E_INVALID_START_BYTE:       invalid start byte
            //      E_INVALID_CODE_POINT:       detected surrogate or code point is invalid Unicode 
            //      E_PREDICATE_FAILED          got a noncharacter or an ASCII control character
            if (__builtin_expect(result > 0, 1)) {
                result = (int)stringBuffer.append_unicode(cp); 
                continue;
            }
            else {
                if (result == unicode::E_UNEXPECTED_ENDOFINPUT) {
                    state_.error() = JP_UNEXPECTED_END_ERROR;
                } else if (result == unicode::E_PREDICATE_FAILED) {
                    // Filtered invalid Unicode code point and did not
                    // replace it. This forces the parser to stop.
                    if (unicode::isNonCharacter(cp))
                        state_.error() = JP_UNICODE_NONCHARACTER_ERROR;
                    else if (cp == 0){
                        state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                    }
                    else if (cp < 0x20u)
                        state_.error() =  JP_CONTROL_CHAR_NOT_ALLOWED_ERROR;  // error: control character not allowed
                    else {
                        state_.error() = JP_UNICODE_REJECTED_BY_FILTER;
                    }
                } else {
                    // E_TRAIL_EXPECTED:
                    // E_INVALID_START_BYTE:
                    // E_INVALID_CODE_POINT:
                    state_.error() = JP_ILLFORMED_UNICODE_SEQUENCE_ERROR;
                }
                break;  // while
            }
        }
        
        if (state_.error() == 0) {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        sa_.error(state_.error(), state_.error_str());
    }
    
    
}}





#endif // JSON_PARSER_PARSE_STRING_HPP

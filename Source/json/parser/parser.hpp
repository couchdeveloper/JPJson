//
//  parser.hpp
//
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

#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP


#include "json/config.hpp"
#include <boost/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>

#include <assert.h>
#include <string.h>
#include <limits>
#include <alloca.h>

#include "parser_errors.hpp"
#include "semantic_actions_base.hpp"
#include "string_buffer.hpp"
#include "string_storage2.hpp"
#include "json/utility/number_builder.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_converter.hpp"
#include "json/endian/endian.hpp"


#if defined (DEBUG)
#include <iostream>
#endif



namespace json {
    
    using unicode::UTF_8_encoding_tag;
    using unicode::UTF_16_encoding_tag;
    using unicode::UTF_16LE_encoding_tag;
    using unicode::UTF_16BE_encoding_tag;
    using unicode::UTF_32_encoding_tag;
    using unicode::UTF_32LE_encoding_tag;
    using unicode::UTF_32BE_encoding_tag;
    using unicode::platform_encoding_tag;
    using unicode::utf_encoding_tag;
    using unicode::is_encoding;
    using unicode::encoding_traits;
    using unicode::add_endianness;
    using json::internal::host_endianness;
    

    using parser_internal::string_buffer;
    using parser_internal::string_storage;
    
    
#pragma mark - Parser Policies    
    
    struct parser_policies {
        
        // Not yet implemented
        
        // May be used to set error handlers for ill-formed Unicode sequences,
        // logging and so on.
    };
    
        
    
#pragma mark - Parser State
    
    class parser_state {
    public:        
        parser_state() 
        : error_(JP_NO_ERROR)
        {}
        void clear()                    { error_ = JP_NO_ERROR; }
        operator bool() const           { return (error_ == JP_NO_ERROR); }
        parser_error_type error() const { return error_; }
        parser_error_type& error()      { return error_; }        
        const char* error_str() const   { return parser_error_str(error_); }        
    private:
        parser_error_type error_;
    };
    
    
#pragma mark -
#pragma mark Class parser
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions = json::semantic_actions_noop<UTF_8_encoding_tag>
    >
    class parser 
    {
    protected:        
        typedef typename SemanticActions::error_t               sa_error_t;        
        
        typedef typename SemanticActions::encoding_t            string_buffer_encoding;        
        typedef string_storage<string_buffer_encoding, SemanticActions> string_storage_t;
        typedef string_buffer<string_storage_t>                 string_buffer_t;
        
        typedef numberbuilder::number_builder<64>               number_builder_t;        
        
        typedef typename boost::iterator_value<InputIterator>::type iterator_value_type;
        
        //
        // Static Assertions:
        //
        
        // SourceEncoding shall be an Unicode encoding scheme:
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, SourceEncoding>::value) );
        
        // The value type of the InputIterator shall match the corresponding code unit type
        // of its encoding:
        BOOST_STATIC_ASSERT( (sizeof(typename boost::iterator_value<InputIterator>::type) 
                              == sizeof(typename encoding_traits<SourceEncoding>::code_unit_type)) );
        
        // Currently, the parser requires that the endianess of its string_buffer
        // encoding matches the platform endianness or is UTF-8 encoding.
        BOOST_STATIC_ASSERT( (boost::is_same<
                              typename encoding_traits<typename add_endianness<string_buffer_encoding>::type>::endian_tag,
                              typename host_endianness::type
                              >::value == true)  );
        
    public:        
        //
        //  Types
        //
        typedef SourceEncoding                          source_encoding_type;
        typedef SemanticActions                         semantic_actions_type;
        typedef parser_state                            state_t;    // Current state of the parser
        typedef typename SemanticActions::result_type   result_t;   // The result of the sematic actions, e.g. a JSON container or AST.
        
        typedef InputIterator iterator;
                                                                            
    public:
        
        //
        //  C-tor
        //
        parser(SemanticActions& sa) 
        : sa_(sa), string_storage_(sa), string_buffer_(string_storage_), unicode_filter_(0)
        {
            configure();
        }

        parser_error_type   parse(InputIterator& first, InputIterator last, bool skipTrailingWhitespaces = true);
        void                reset();
        state_t             state() const   { return state_; }        
        result_t            result() const  { return sa_.result(); }
        result_t            move_result()   { 
            result_t tmp;
            swap(tmp, sa_.result());
            return tmp;
        }
        

    protected:
        
        unsigned int to_uint(iterator_value_type v) const { 
            return unicode::encoding_traits<source_encoding_type>::to_uint(v); 
        }
        
        // Configure the parser from the options set in the semantic actions 
        // instance - and vice versa.
        void configure() 
        {
            string_storage_.enable_partial_strings(true);
            semanticactions::noncharacter_handling_option nch_option = sa_.unicode_noncharacter_handling();
            switch (nch_option) {
                case semanticactions::SignalErrorOnUnicodeNoncharacter:
                    unicode_filter_.replacement_character(0);
                    break;
                case semanticactions::SubstituteUnicodeNoncharacter:                     
                    unicode_filter_.replacement_character(unicode::kReplacementCharacter);
                    break;
                case semanticactions::SkipUnicodeNoncharacters: 
                    assert("feature not yet implemented"==NULL);
                    unicode_filter_.replacement_character(0);
                    break;
            } 
            
            sa_.inputEncoding(encoding_traits<source_encoding_type>::name());
            //semanticactions::non_conformance_flags ncon_flags = sa.extensions();
        }
                
    protected:
        iterator                p_;     
        iterator                last_;  
        SemanticActions&        sa_;
        state_t                 state_;
        number_builder_t        number_builder_;
        unicode::filter::NoncharacterOrNULL unicode_filter_;
        string_storage_t        string_storage_;
        string_buffer_t         string_buffer_;
        
    private:        
        void parse_text();
        void parse_key_value_list();
        void parse_value();
        
        
#pragma mark -
#pragma mark Parse String
        //
        //  parse_string()
        //
        void parse_string() 
        {
            //TEST: TODO: fix
            //((void)printf ("%s:%u: test assertion\n", __FILE__, __LINE__), abort());
            
            assert(state_.error() == JP_NO_ERROR); 
            assert(p_ != last_);
            assert(to_uint(*p_) == '"');            
            assert(string_buffer_.size() == 0);  // check whether we have a new string on stack of the string storage
            
            ++p_;
            while (__builtin_expect(p_ != last_, 1)) 
            {
                // fast path: read ASCII (no control characters, and no ASCII NULL)
                uint32_t c = to_uint(*p_);
                if (__builtin_expect((c - 0x20u) < 0x60u, 1))  // ASCII except control-char, and no ASCII NULL
                {
#if 1                    
                    ++p_;
                    if (c != '\\' and c != '"') {
                        string_buffer_pushback_ASCII(c);  
                        // note: string_buffer_pushback_ASCII() does not check for Unicode NULL
                        continue;
                    } 
                    else if (c == '\\') {
                        escape_sequence();
                        if (!state_) {
                            // error parsing escape sequence
                            return; // error state already set
                        }
                        continue;
                    }
                    else {
                        skip_whitespaces();
                        return; // done
                    }
                        
#else                        
                    ++p_;
                    switch (c) {
                        default:
                            string_buffer_pushback_ASCII(c);  
                                // note: string_buffer_pushback_ASCII() does not check for Unicode NULL
                            continue;
                        case '\\': // escape sequence
                            escape_sequence();
                            if (!state_) {
                                // error parsing escape sequence
                                return; // error state already set
                            }
                            continue;
                        case '"':
                            skip_whitespaces();
                            return; // done
                    }
#endif                    
                }
                
                // slow path: reading UTF-8 multi byte sequences, ASCII NULL, and 
                // characters in any other UTF encoding scheme which are not ASCII.
                // Convert the UTF character into a code point:
                // Use a conversions which does not accept surrogates and noncharacters. 
                // Per default, the filter shall signal errors if it matches characters.
                //  //Replace invalid characters with Unicode Replacement Character.
                // The "safe" conversion does not accept surrogates, so we only need to
                // check for Unicode noncharacters and ASCII control characters:
                
                // Note, no ASCIIs here
                if ( (c >> 7) != 0 or c == 0) // (not ((c - 1u) < (0x20u - 1u))) 
                {
                    // No ASCII control chars - but possibly Unicode NULL
                    // Use a safe, stateless converter which only converts one character:
                    typedef unicode::converter<
                        source_encoding_type, unicode::code_point_t, 
                        unicode::Validation::SAFE, unicode::Stateful::No, unicode::ParseOne::Yes
                    >  converter_t;
                    
                    unicode::code_point_t cp;
                    unicode::code_point_t* cp_ptr = &cp;
                    int result = converter_t().convert(p_, last_, cp_ptr);
                    // Possible results:
                    //      unicode::NO_ERROR
                    //      unicode::E_TRAIL_EXPECTED:           trail byte expected (ill-formed UTF-8)
                    //      unicode::E_INVALID_START_BYTE:       invalid start byte
                    //      unicode::E_UNCONVERTABLE_OFFSET      ill-formed UTF-8
                    //      unicode::E_INVALID_CODE_POINT:       
                    //      unicode::E_INVALID_UNICODE:      
                    //      unicode::E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
                    if (__builtin_expect(result == unicode::NO_ERROR, 1)) {
                        // Note: the code point may still be an Unicode noncharacter, or a Unicode NULL (U+0000).
                        // We check this in string_buffer_pushback_unicode():
                        // Possible return codes:
                        // >0:   success
                        //  0:   string buffer error (possible overflow)
                        // -1:   filter predicate failed (possible Unicode noncharacter) 
                        result = string_buffer_pushback_unicode(cp);
                        if (result == 0)
                            continue;
                        else {
                            // Error state shall be set according the filter rules,
                            // respectively by the string buffer if that failed:
                            assert(state_.error() != JP_NO_ERROR);
                            break;  // or error jump out of while loop                        
                        }
                    }                
                    else {
                        // If we reach here, we got an error during Unicode conversion
                        // Map the unicode error codes to json error codes:
                        switch (result) {
                            case unicode::E_TRAIL_EXPECTED:
                            case unicode::E_INVALID_START_BYTE:
                            case unicode::E_UNCONVERTABLE_OFFSET:
                            case unicode::E_INVALID_CODE_POINT:
                            case unicode::E_INVALID_UNICODE:        state_.error() = JP_ILLFORMED_UNICODE_SEQUENCE_ERROR; break;
                            case unicode::E_NO_CHARACTER:           state_.error() = JP_UNICODE_NONCHARACTER_ERROR; break;
                            case unicode::E_UNEXPECTED_ENDOFINPUT:  state_.error() = JP_UNEXPECTED_END_ERROR; break;
                            default: state_.error() = JP_INVALID_UNICODE_ERROR;                    
                        }
                        
                        break;  // on error jump out of while loop
                    }                
                } 
                else 
                {
                    // Got ASCII Control char
                    state_.error() =  JP_CONTROL_CHAR_NOT_ALLOWED_ERROR;  // error: control character not allowed
                    break; // on error jump out of while loop
                }
            } // while
                
            
            // If we come to here, we got an error, or we got an unexpected EOF/EOS
            if (state_.error() == 0) {
                state_.error() = JP_UNEXPECTED_END_ERROR;
            }
            sa_.error(state_.error(), state_.error_str());
        }
        
        
        
        
#pragma mark -
        void parse_number();
        void skip_whitespaces();
        void parse_object();
        void parse_array();
        bool match(const char* bytes, size_t n);
        
        unicode::code_point_t escaped_unicode();
        uint16_t hex();
        void escape_sequence();
        
        
#pragma mark - String Buffer       
        
        // Pushback an ASCII to a string buffer.
        // Don't use this function to append ASCII NULL, string_buffer_pushback_unicode() in this case.
        void 
        string_buffer_pushback_ASCII(char ch) { 
            assert(ch <= 0x7F);
            // An ASCII can be converted implicitly to any of the encoding forms:
            // UTF-8, UTF-16, and UTF-32.
            string_buffer_.append_ascii(ch); 
        }
        
        //
        // string_buffer_pushback_unicode
        //
        // Append an Unicode code point to the string buffer.
        // Parameter unicode shall be a valid unicode scalar value.
        // Test if the given unicode matches the predicate of the parser's
        // unicode filter, and if this evaluates to true, apply the filter's 
        // replacement policy. 
        // If an invalid unicode will be replaced by a replacement character, the 
        // function will not signal an error.
        // 
        // Returns zero on success, otherwise returns -1 and sets the parser 
        // error state accordingly.
        //        
        // Since paramerer 'unicode' is assumed to be a valid Unicode Scalar Value,
        // and the filter has been applied, a possibly required Unicode conversion  
        // performed while appending to the string buffer will always succeed.
        //
        // May throw an exception if internal buffers could not be allocated.
        // 
        int
        string_buffer_pushback_unicode(unicode::code_point_t unicode) 
        {
            if (unicode_filter_(unicode)) {
                if (unicode_filter_.replace()) {
                    unicode = unicode_filter_.replacement(unicode);
                    assert(unicode != 0);
                } else {
                    // Filtered invalid Unicode code point and did not
                    // replace it. This forces the parser to stop.
                    if (unicode::isNonCharacter(unicode))
                        state_.error() = JP_UNICODE_NONCHARACTER_ERROR;
                    else if (unicode == 0){
                        state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;  // TODO: use better error code
                    }
                    else {
                        state_.error() = JP_UNICODE_REJECTED_BY_FILTER;
                    }
                    sa_.error(state_.error(), state_.error_str());
                    return -1;
                }
            }
            string_buffer_.append_unicode(unicode);  // shall not fail
            return 0;
        }
        
        void throwLogicError(const char* msg); 
        
        
#pragma mark -
        
    private:
        parser(const parser&);  // no copy ctor
        parser();               // no default ctor
        
    };
    
}   // namespace json   


// -----------------------------------------------------------------------------------
#pragma mark -
#pragma mark Parser Implementation
// -----------------------------------------------------------------------------------

namespace json {
    
    // public
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    parser_error_type 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse(InputIterator& first, InputIterator last, bool skipTrailingWhitespaces) 
    {
        p_ = first;
        last_ = last;
        sa_.parse_begin();
        parse_text();
        parser_error_type result = state_.error();
        if (state_.error() == JP_NO_ERROR and skipTrailingWhitespaces and p_ != last_) {
            // Increment past the last significant character:
            ++p_;
            skip_whitespaces();
        }
        first = p_;  // TODO: use statement first = p_;  but this would require a type cast operator defined for iterator_adaptor        
        
        return result;
    }
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    reset() {
        sa_.clear();
        state_.clear();
        string_storage_.clear();
        number_builder_.clear();
    }
    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    skip_whitespaces() 
    {
        assert(state_.error() == JP_NO_ERROR);

        while (p_ != last_) 
        {
            switch (to_uint(*p_)) {
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    ++p_;
                    break;
                default:  // EOS or any other character
                    return;
            }
        }
    }
    
    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_text() 
    {
        assert(state_.error() == JP_NO_ERROR);
        
        if (sa_.is_canceled()) {
            state_.error() = JP_CANCELED;
            sa_.error(state_.error(), state_.error_str());
            return;
        }            
        
        // text => object | array
        skip_whitespaces();
        
        if (p_ == last_) {
            state_.error() = JP_EMPTY_TEXT_ERROR;
            sa_.error(state_.error(), state_.error_str());
            return;
        }

        assert(p_ != last_);  
        unsigned int c = to_uint(*p_);
        switch (c) {
            case '[':
                sa_.begin_array();
                parse_array();
                if (state_) {
                    sa_.end_array();
                    sa_.parse_end();
                }
                break;
                
            case '{':
            {
                sa_.begin_object();
                parse_object();
                if (state_) {
                    bool success = sa_.end_object();
                    if (success) {
                        sa_.parse_end();
                    }
                    else {
                        // The only reason end_object() returns 
                        // false is that a key already exists.
                        // ERROR: insertion failed due to double key error
                        state_.error() = JP_JSON_KEY_EXISTS_ERROR;
                        sa_.error(state_.error(), state_.error_str());
                    }
                }
            }
                break;
                
            default:
                if (c == 0) {
                    state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                } else {
                    state_.error() = JP_EXPECTED_ARRAY_OR_OBJECT_ERROR;
                }
                sa_.error(state_.error(), state_.error_str());
        }        
    }
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_array() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(to_uint(*p_) == '[');
        
        ++p_;        
        // check if the array is empty:
        skip_whitespaces();
        if (p_ != last_) {
            unsigned int c = to_uint(*p_);
            if  (c == ']')
            {
                // end of array  (array is empty)
                return;
            } else if (c == 0) {
                state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
            } else {
                size_t index = 0;
                // parse value_list
                while (1) {
                    sa_.begin_value_at_index(index);
                    parse_value();
                    sa_.end_value_at_index(index);
                    if (state_) {
                        if (p_ != last_) {
                            unsigned int c = to_uint(*p_);
                            if (c == ',') {
                                ++p_;
                                skip_whitespaces();
                                if (p_ != last_) {
                                    ++index;
                                    continue;
                                }
                                else {
                                    state_.error() = JP_UNEXPECTED_END_ERROR;
                                    break;
                                }
                            }
                            else {
                                break; // finished reading values.
                            }
                        } 
                        else {
                            state_.error() = JP_UNEXPECTED_END_ERROR;
                            break;
                        }
                    } else {
                        // error state already set
                        return;
                    }
                }
                    
                if (state_) {
                    if (p_ != last_) {
                        unsigned int c = to_uint(*p_);
                        if (c == ']') {
                            // end of array
                            return;
                        }
                        else {
                            state_.error() = JP_EXPECTED_TOKEN_ARRAY_END_ERROR;
                        }
                    } else {
                        state_.error() = JP_UNEXPECTED_END_ERROR;
                    }
                } 
            }
        }
        else {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        
        if (!state_) {
            sa_.error(state_.error(), state_.error_str());
        }
    }    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_object() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(to_uint(*p_) == '{');
        
        ++p_;
        // check if the object is empty:
        skip_whitespaces();
        if (p_ != last_) {
            unsigned int c = to_uint(*p_);
            if  (c == '}') {
                // end of object  (object is empty)
                return;
            } else if (c == 0) {
                state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
            } else {
                parse_key_value_list();
                if (state_) {
                    // (insertion of key-value pair performed in key_value_list() )
                    // get the '}':
                    if (p_ != last_) {
                        unsigned int c = to_uint(*p_);
                        if (c == '}') {
                            // end of object
                            return;
                        }
                        else if (c == 0) {
                            state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                        } else {
                            state_.error() = JP_EXPECTED_TOKEN_OBJECT_END_ERROR;
                        }
                    }
                    else {
                        state_.error() = JP_UNEXPECTED_END_ERROR;
                    }
                } else {
                    // error parsing key_value_list, state already set
                    return;                    
                }
            }  
        }
        else {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        
        assert(state_.error() != 0);
        sa_.error(state_.error(), state_.error_str());
    }
    
    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_key_value_list() 
    {
        assert(state_.error() == JP_NO_ERROR);
        
        // string ':' value [',' string ':' value]*
        
        
        //bool done = false;
        size_t index = 0;
        while (__builtin_expect(p_ != last_, 1)) {
            unsigned int c = to_uint(*p_);
            if (c == '"') 
            {
                // first, get the key ...
                // prepare the string storage for a key string:
                //string_storage_.stack_push();
                string_storage_.reset();
                //string_storage_.set_mode(string_storage_t::Key);
                parse_string();
                if (state_) {
                    if (p_ != last_) {
                        // ... then, eat the key_value separator ...
                        c = to_uint(*p_);                        
                        if (c == ':') {
                            ++p_;
                            skip_whitespaces();
                            if (p_ != last_)
                            {    
                                // ... finally, get a value and put it onto sa_'s stack
                                sa_.begin_key_value_pair(string_buffer_.buffer(), index);
                                parse_value();  // whitespaces skipped.
                                sa_.end_key_value_pair(/*string_buffer_.buffer()*/typename SemanticActions::const_buffer_t(0,0), index);
                                //string_storage_.stack_pop(); // remove the key from top of the string buffer's stack
                                if (state_) 
                                {
                                    // Note: We populate the object at end_object().
                                    if (p_ != last_) {
                                        c = to_uint(*p_);
                                        if (c == ',') {                                
                                            // not done yet, get the next key value pair
                                            ++p_;
                                            skip_whitespaces();
                                            ++index;
                                            continue; // parse  next key_value pair

                                        } else {
                                            // Seems we are done with no errors.
                                            // note: p() points to start of next token, which shall be '}'
                                            // or it points to last_, which will be detected later.
                                            return; 
                                        }
                                    } else {
                                        state_.error() = JP_UNEXPECTED_END_ERROR;
                                        break; //done = true;
                                    }
                                }
                                else {
                                    // ERROR: expected value. state_ is already set, just leave.
                                    assert(state_.error() != JP_NO_ERROR);
                                    break; //done = true;
                                }
                            }
                            else {
                                state_.error() = JP_UNEXPECTED_END_ERROR;
                                break; //done = true;
                            }
                        }
                        else {
                            // ERROR: expected key-value-separator ':'
                            state_.error() = JP_EXPECTED_TOKEN_KEY_VALUE_SEP_ERROR;
                            break; //done = true;
                        }
                    } 
                    else {
                        state_.error() = JP_UNEXPECTED_END_ERROR;
                        break; //done = true;
                    }
                } 
                else {
                    // ERROR: something went wrong parsing the key string
                    // state is already set.
                    assert(state_.error() != JP_NO_ERROR);
                    break; //done = true;
                }            
            }
            else {
                // there is no start of string '"'
                // handle error: expected string, unexpected EOT
                state_.error() = JP_EXPECTED_STRING_ERROR;
                break; //done = true;
            }
        }  // while (p_ != last_)      
        
        
        if (p_ == last_) {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        assert(state_.error() != 0);
        sa_.error(state_.error(), state_.error_str());
    }
    
    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_value()
    {     
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        
        enum Token {
            s = 0,  // '"'
            O = 1,  // '{'   // a capitalized 'O' (for 'O'bject)
            n = 2,  // [0..9], '-'
            A = 3,  // '['
            t = 4,  // true
            f = 5,  // false
            N = 6,  // null
            E = 7,  // error
        };
        static const uint8_t table[] = {
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,   //  0..31
            E,E,s,E,E,E,E,E,  E,E,E,E,E,n,E,E,  n,n,n,n,n,n,n,n,  n,n,E,E,E,E,E,E,   // 32..63
            E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,E,E,E,E,E,  E,E,E,A,E,E,E,E,   // 64..95
            E,E,E,E,E,E,f,E,  E,E,E,E,E,E,N,E,  E,E,E,E,t,E,E,E,  E,E,E,O,E,E,E,E    // 96..127
        };
        
        unsigned int c = to_uint(*p_);
        Token token = c <= 0x7Fu ? static_cast<Token>(table[c]) : E;
        
        switch (token) {
            case s:
                // Found start of a JSON String
                // Prepare the string storage to hold a data string:
                //string_storage_.stack_push();
                string_storage_.reset();
                //string_storage_.set_mode(string_storage_t::Data);
                parse_string();  // this may send partial strings to the semantic actions object.
                if (state_) {
                    string_storage_.flush();  // send the remaining characters in the string buffer to the semantic actions object.
                } else {
                    // handle error string
                }
                //string_storage_.stack_pop();
                return;
                
            case O: // (This is a capitalized 'O')
                // Found start of a JSON Object
                sa_.begin_object();
                parse_object();
                if (state_) {
                    bool success = sa_.end_object();
                    if (not success) {
                        // The only reason end_object() returns 
                        // false is that a key already exists.
                        // ERROR: insertion failed due to double key error
                        state_.error() = JP_JSON_KEY_EXISTS_ERROR;
                        sa_.error(state_.error(), state_.error_str());
                    }
                    ++p_;
                    skip_whitespaces();
                } else {
                    // handle error object
                }
                return;
                
            case n:
                // Found start of a JSON Number
                number_builder_.clear();
                parse_number();
                if (state_) {
                    // parse_number does not skip whitespaces
                    skip_whitespaces();
                    // note: p() points to the start of next token
                    sa_.value_number(number_builder_.number());
                } else {
                    // handle error number
                    assert(state_.error() != 0);
                }
                return;
                
            case A:
                // Found start of a JSON Array
                sa_.begin_array();
                parse_array();
                if (state_) {
                    sa_.end_array();
                    ++p_;
                    skip_whitespaces();
                }
                return;
                
            case t:
                // Found start of JSON true
                if (match("true", 4)) {
                    // got a "true"
                    // action_value_true
                    sa_.value_boolean(true);
                    skip_whitespaces();
                }
                return;
                
            case f:
                // Found start of JSON false
                if (match("false", 5)) {
                    // action_value_false
                    sa_.value_boolean(false);
                    skip_whitespaces();
                } 
                return;
                
            case N:
                // Found start of JSON null
                if (match("null", 4)) {
                    // action_value_null
                    sa_.value_null();
                    skip_whitespaces();
                }
                return;    
                                
            default:
                state_.error() = JP_EXPECTED_VALUE_ERROR;
                sa_.error(state_.error(), state_.error_str());
        }
    }
    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >
    inline 
    bool 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    match(const char* s, size_t n)
    {
        while (n != 0 and p_ != last_ and *s == to_uint(*p_)) {
            ++s;
            ++p_;
            --n;
        }
        if (n != 0) {
            if (p_ == last_) {
                state_.error() = JP_UNEXPECTED_END_ERROR;
            }
            else {
                state_.error() = JP_EXPECTED_VALUE_ERROR;
            }
            sa_.error(state_.error(), state_.error_str());
            return false;
        } else  {
            return true;
        }
    }
    
#pragma mark - parse string
    
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
//#if !defined (DEBUG)
//    __attribute__((always_inline))
//#endif                    
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
                uint32_t c = to_uint(*p_) - 48u;
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
                unsigned int c = to_uint(*p_);
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
//    #if !defined (DEBUG)
//        __attribute__((always_inline))
//    #endif                    
        parser<InputIterator, SourceEncoding, SemanticActions>::
    escaped_unicode() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(to_uint(*p_) == 'u');
        
        ++p_;
        uint16_t euc1 = hex();
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
                            uint16_t euc2 = hex();
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
//#if !defined (DEBUG)
//    __attribute__((always_inline))
//#endif                
    parser<InputIterator, SourceEncoding, SemanticActions>::
    escape_sequence() 
    {
        assert(state_.error() == JP_NO_ERROR);        
        
        if (__builtin_expect(p_ != last_, 1)) 
        {
            unsigned int c = to_uint(*p_);
            uint8_t ascii;
            switch (c) {
                case '"':   ascii = '"'; break;  // string_buffer_pushback_ASCII(stringBuffer, '"');  ++p_; return;
                case '\\':  ascii = '\\'; break; // string_buffer_pushback_ASCII(stringBuffer, '\\'); ++p_; return;
                case '/':   ascii = '/'; break;  // string_buffer_pushback_ASCII(stringBuffer, '/');  ++p_; return;
                case 'b':   ascii = '\b'; break; // string_buffer_pushback_ASCII(stringBuffer, '\b'); ++p_; return;
                case 'f':   ascii = '\f'; break; // string_buffer_pushback_ASCII(stringBuffer, '\f'); ++p_; return;
                case 'n':   ascii = '\n'; break; // string_buffer_pushback_ASCII(stringBuffer, '\n'); ++p_; return;
                case 'r':   ascii = '\r'; break; // string_buffer_pushback_ASCII(stringBuffer, '\r'); ++p_; return;
                case 't':   ascii = '\t'; break; // string_buffer_pushback_ASCII(stringBuffer, '\t'); ++p_; return;
                case 'u':  { // escaped unicode
                    unicode::code_point_t cp = escaped_unicode();
                    if (state_) {
                        int result = string_buffer_pushback_unicode(cp);
                        if (result == 0) {
                            return;  // success 
                        } else if (result < 0) {
                            assert(state_.error() != JP_NO_ERROR);
                            // Error state shall be set already
                            // detected Unicode noncharacter and rejected it,
                            // or U+0000 which is not allowed
                            return; // with error state set
                        }
                    } 
                    else {
                        // malformed unicode: not an Unicode scalar value,
                        // or unexpected end.
                        assert(state_.error() != 0);
                        return; // with error state set
                    }
                    break;
                }
                default:
                    state_.error() = JP_INVALID_ESCAPE_SEQ_ERROR;
                    sa_.error(state_.error(), state_.error_str());
                    return; // with error state set
                    
            } // switch
            
            string_buffer_pushback_ASCII(ascii);  
            ++p_; 
            return;  // success
        }
        else {
            state_.error() = JP_UNEXPECTED_END_ERROR;
            sa_.error(state_.error(), state_.error_str());
        }
        
        assert(state_.error() != 0);
    }

    
    

#pragma mark -
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
    >    
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_number() 
    {
        assert(p_ != last_);
        
        enum number_state {
            number_state_start,
            number_state_sign,
            number_int_is_zero,
            number_state_int,
            number_state_point,
            number_state_fractional,
            number_state_exponent_start,
            number_state_exponent_sign,
            number_state_exponent,
        };
        
        number_state s = number_state_start;
        bool isNegative = false;
        bool exponentIsNegative = false;        
        while (p_ != last_)
        {
            unsigned int c = to_uint(*p_);
            switch (s) {
                case number_state_start:
                    switch (c) {
                        case '-': 
                            s = number_state_sign; 
                            isNegative = true; 
                            number_builder_.push_sign(isNegative);
                            break;
                        case '0': 
                            s = number_int_is_zero; 
                            number_builder_.push_integer_start(c);
                            break;
                        case '1'...'9': 
                            s = number_state_int; 
                            number_builder_.push_integer_start(c);
                            ++p_;
                            while (p_ != last_ and ( (c = to_uint(*p_)) >= '0' and c <= '9')) {
                                number_builder_.push_digit(c);
                                ++p_;
                            }
                            continue;
                        default: goto Number_done; // error: not a number
                    }
                    break;
                case number_state_sign:
                    switch (c) {
                        case '0':       
                            s = number_int_is_zero; 
                            number_builder_.push_integer_start(c);
                            break;
                        case '1'...'9': 
                            s = number_state_int; 
                            number_builder_.push_integer_start(c);
                            ++p_;
                            while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9') {
                                number_builder_.push_digit(c);
                                ++p_;
                            }
                            continue;
                            //break;
                        default: goto Number_done;  // error: expecting a digit
                    }
                    break;
                case number_int_is_zero:
                    switch (c) {
                        case '.': 
                            s = number_state_point;
                            number_builder_.integer_end();
                            number_builder_.push_decimalPoint();
                            break;
                        case 'e':
                        case 'E': 
                            s = number_state_exponent_start;
                            number_builder_.integer_end();
                            number_builder_.push_exponentIndicator(c); 
                            break;
                        default: goto Number_done; // finished.
                    }
                    break;
                case number_state_int:
                    switch (c) {
                        case '.': 
                            s = number_state_point; 
                            number_builder_.integer_end();
                            number_builder_.push_decimalPoint();
                            break;
                        case 'e':
                        case 'E': 
                            s = number_state_exponent_start; 
                            number_builder_.integer_end();
                            number_builder_.push_exponentIndicator(c); 
                            break;
                        default: goto Number_done; // finished with integer
                    }
                    break;
                case number_state_point:
                    switch (c) {
                        case '0'...'9': 
                            s = number_state_fractional; 
                            number_builder_.push_fractional_start(c);
                            ++p_;
                            while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9') {
                                number_builder_.push_digit(c);
                                ++p_;
                            }
                            continue;
                        default: goto Number_done; // error: expected digit
                    }
                    break;
                case number_state_fractional:
                    switch (c) {
                        case 'e':
                        case 'E': 
                            s = number_state_exponent_start; 
                            number_builder_.fractional_end();
                            number_builder_.push_exponentIndicator(c); 
                            break;
                        default: goto Number_done; // finished with fractional or start exponent
                    }
                    break;
                case number_state_exponent_start:
#if 1                    
                    if (c == '-' or c == '+') {
                        s = number_state_exponent_sign; 
                        number_builder_.push_exponent_start(c);
                        exponentIsNegative = c == '-'; 
                        ++p_;
                        if (p_ == last_) {
                            goto Number_done;  // error
                        }
                        c = to_uint(*p_);
                    } 
                    if (c >= '0' and c <= '9') {
                        s = number_state_exponent; 
                        number_builder_.push_exponent_start(c);
                        ++p_;
                        while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9') {
                            number_builder_.push_digit(c);
                            ++p_;
                        }
                        continue;
                    }
                    else {
                        goto Number_done;  // error
                    }
                    //break;
#else                    
                    switch (c) {
                            
                        case '-': 
                            s = number_state_exponent_sign; 
                            number_builder_.push_exponent_start(c);
                            exponentIsNegative = true; 
                            break;
                        case '+': 
                            s = number_state_exponent_sign; 
                            number_builder_.push_exponent_start(c);
                            break;
                        case '0' ... '9': 
                            s = number_state_exponent; 
                            number_builder_.push_exponent_start(c);
                            ++p_;
                            while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9')
                            {
                                number_builder_.push_digit(c);
                                ++p_;
                            }
                            continue;
                        default: goto Number_done;  // error
                    }
                    break;
#endif                    
                case number_state_exponent_sign:
                /*    
                    switch (c) {
                        case '0' ... '9': 
                            s = number_state_exponent; 
                            number_builder_.push_digit(c); 
                            ++p_;
                            while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9')
                            {
                                number_builder_.push_digit(c);
                                ++p_;
                            }
                            continue;
                        default: goto Number_done;  // finished                            
                    }
                 */
                    break;
                case number_state_exponent:
                    goto Number_done;  // finished
                    //break;
                default:
                    assert(0);
            } //switch
            
            ++p_;
        } // while (p_ != last_)
        
        Number_done:        
        
        switch (s) {
            case number_int_is_zero:    
            case number_state_int:
                number_builder_.integer_end(); 
                skip_whitespaces();
                break;
                
            case number_state_fractional:
                number_builder_.fractional_end(); 
                skip_whitespaces();
                break;
                
            case number_state_exponent:
                number_builder_.exponent_end(); 
                skip_whitespaces();
                break;
                
            default:
                state_.error() = JP_BADNUMBER_ERROR;
                sa_.error(state_.error(), state_.error_str());
        }        
    }
        
    
    template <
        typename InputIterator
      , typename SourceEncoding
      , typename SemanticActions
    >    
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    throwLogicError(const char* msg) {
        throw std::logic_error(msg);
    }
    
    
} // namespace json


#endif // JSON_PARSER_H

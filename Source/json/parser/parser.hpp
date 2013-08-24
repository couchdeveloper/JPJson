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
#include <boost/iterator/iterator_traits.hpp>
#include "parser_errors.hpp"
#include "semantic_actions_base.hpp"
#include "string_buffer.hpp"
#include "number_string_buffer.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_converter.hpp"
#include "json/endian/endian.hpp"
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <limits>
#include <alloca.h>


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
        typedef typename SemanticActions::number_desc_t         number_desc_t;

        typedef parser_internal::string_buffer<
            string_buffer_encoding, SemanticActions
        >                                                       string_buffer_t;
        typedef parser_internal::number_string_buffer<128>      number_string_buffer_t;        
        typedef typename boost::iterator_value<
            InputIterator
        >::type                                                 iterator_value_type;
        
        //
        // Static Assertions:
        //
        
        // SourceEncoding shall be an Unicode encoding scheme:
        static_assert( (std::is_base_of<utf_encoding_tag, SourceEncoding>::value), "" );
        
        // The value type of the InputIterator shall match the corresponding code unit type
        // of its encoding:
        static_assert( (sizeof(typename boost::iterator_value<InputIterator>::type)
                              == sizeof(typename encoding_traits<SourceEncoding>::code_unit_type)), "" );
        
        // Currently, the parser requires that the endianess of its string_buffer
        // encoding matches the platform endianness or is UTF-8 encoding.
        static_assert( (boost::is_same<
                              typename encoding_traits<typename add_endianness<string_buffer_encoding>::type>::endian_tag,
                              typename host_endianness::type
                              >::value == true), "" );
        
    public:        
        //
        //  Types
        //
        typedef SourceEncoding                              source_encoding_type;
        typedef SemanticActions                             semantic_actions_type;
        typedef parser_state                                state_t;    // Current state of the parser
        typedef typename SemanticActions::result_type       result_t;   // The result of the sematic actions, e.g. a JSON container or AST.
        
        
        typedef InputIterator iterator;
                                                                            
    public:
        
        //
        //  C-tor
        //
        parser(SemanticActions& sa) 
        : sa_(sa), string_buffer_(sa), opt_pass_escaped_string_(false)
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
        
        __attribute__((always_inline))
        void next() {
            ++p_;
            ++pos_;
        }
        
        unsigned int to_uint(iterator_value_type v) const { 
            return unicode::encoding_traits<source_encoding_type>::to_uint(v); 
        }
        
        // Configure the parser from the options set in the semantic actions 
        // instance - and vice versa.
        void configure() 
        {
            //string_storage_.enable_partial_strings(true);
            
            nonch_opt_ = sa_.unicode_noncharacter_handling();
            nullch_opt_ = sa_.unicode_nullcharacter_handling();
            
            opt_pass_escaped_string_ = sa_.passEscapdedString();
            sa_.inputEncoding(encoding_traits<source_encoding_type>::name());
            //semanticactions::non_conformance_flags ncon_flags = sa.extensions();
        }
                
    protected:
#pragma mark - Member Variables        
        SemanticActions&        sa_;
        std::size_t             pos_;  // index of character  (not code_unit index)
        iterator                p_;
        iterator                last_;
        semanticactions::noncharacter_handling_option nonch_opt_;
        semanticactions::nullcharacter_handling_option nullch_opt_;
        state_t                 state_;
        string_buffer_t         string_buffer_;
        number_string_buffer_t  number_string_buffer_;
        bool                    opt_pass_escaped_string_;

    private:        
        void parse_text();
        void parse_key_value_list();
        void parse_value();        
        void parse_string(); 
        void parse_number(number_desc_t& number_info);
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
            if (nonch_opt_ != semanticactions::AllowUnicodeNoncharacter) {
                if (__builtin_expect(unicode::isNonCharacter(unicode), 0)) {
                    switch (nonch_opt_) {
                        case semanticactions::AllowUnicodeNoncharacter:
                            break;
                        case semanticactions::SignalErrorOnUnicodeNoncharacter:
                            state_.error() = JP_UNICODE_NONCHARACTER_ERROR;
                            sa_.error(state_.error(), state_.error_str());
                            return -1;
                            break;

                        case semanticactions::SubstituteUnicodeNoncharacter:
                            unicode = unicode::kReplacementCharacter;
                            break;
                            
                        case semanticactions::RemoveUnicodeNoncharacter:
                            break;
                    }
                }
                else {
                    /* do nothing */
                }
            }
            if (__builtin_expect(unicode == 0, 0)) {
                switch (nullch_opt_) {
                    case semanticactions::AllowUnicodeNULLCharacter:
                        break;
                    case semanticactions::SignalErrorOnUnicodeNULLCharacter:
                        state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;  // TODO: use better error code
                        sa_.error(state_.error(), state_.error_str());
                        return -1;
                        break;
                    case semanticactions::SubstituteUnicodeNULLCharacter:
                        unicode = unicode::kReplacementCharacter;
                        break;
                        
                    case semanticactions::RemoveUnicodeNULLCharacter:
                        return 0;
                        break;
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
        pos_ = 0;
        sa_.parse_begin();
        parse_text();
        parser_error_type result = state_.error();
        if (state_.error() == JP_NO_ERROR and skipTrailingWhitespaces and p_ != last_) {
            // Increment past the last significant character:
            next();
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
        string_buffer_.clear();
        number_string_buffer_.clear();
        pos_ = 0;
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
                    next();
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
        
        next();        
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
                                next();
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
        
        next();
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
                // prepare the string buffer for a key string:
                string_buffer_.clear();
                string_buffer_.set_allow_partial_strings(false);
                parse_string();
                if (state_) {
                    if (p_ != last_) {
                        // ... then, eat the key_value separator ...
                        c = to_uint(*p_);                        
                        if (c == ':') {
                            next();
                            skip_whitespaces();
                            if (p_ != last_)
                            {    
                                // ... finally, get a value and put it onto sa_'s stack
                                sa_.begin_key_value_pair(string_buffer_.buffer(), index);
                                parse_value();  // whitespaces skipped.
                                sa_.end_key_value_pair();
                                if (state_) 
                                {
                                    // Note: We populate the object at end_object().
                                    if (p_ != last_) {
                                        c = to_uint(*p_);
                                        if (c == ',') {                                
                                            // not done yet, get the next key value pair
                                            next();
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
        
        number_desc_t number_info;
        
        switch (token) {
            case s:
                // Found start of a JSON String
                // Prepare the string buffer to hold a data string:
                string_buffer_.clear();
                string_buffer_.set_allow_partial_strings(true);
                parse_string();  // this may send partial strings to the semantic actions object.
                if (state_) {
                    string_buffer_.flush();  // send the remaining characters in the string buffer to the semantic actions object.
                } else {
                    // handle error string
                }
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
                    next();
                    skip_whitespaces();
                } else {
                    // handle error object
                }
                return;
                
            case n:
                // Found start of a JSON Number
                number_string_buffer_.reset();
                parse_number(number_info);
                if (state_) {
                    // parse_number does not skip whitespaces
                    skip_whitespaces();
                    // note: p() points to the start of next token
                    sa_.value_number(number_info);
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
                    next();
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
            next();
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
    
    template <
        typename InputIterator
      , typename SourceEncoding
      , typename SemanticActions
    >
    void 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_string() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(to_uint(*p_) == '"');            
        assert(string_buffer_.size() == 0);  // check whether we have a new string on stack of the string storage
        
        next();
        while (__builtin_expect(p_ != last_, 1)) 
        {
            // fast path: read ASCII (no control characters, and no ASCII NULL)
            uint32_t c = to_uint(*p_);
            if (__builtin_expect((c - 0x20u) < 0x60u, 1))  // ASCII except control-char, and no ASCII NULL
            {
                next();
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
            if (__builtin_expect( ((c >> 7) != 0 or c == 0), 1)) // (not ((c - 1u) < (0x20u - 1u)))
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
                    ++pos_;  // increment one character
                    // Note: the code point may still be an Unicode noncharacter, or a Unicode NULL (U+0000).
                    // We check this in string_buffer_pushback_unicode():
                    // Possible return codes:
                    // >0:   success
                    //  0:   string buffer error (possible overflow)
                    // -1:   filter predicate failed (possible Unicode noncharacter) 
                    result = string_buffer_pushback_unicode(cp);
                    if (__builtin_expect(result == 0, 1))
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
//#if !defined (DEBUG)
//    __attribute__((always_inline))
//#endif
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
        for (int i = 0; i < 4; ++i, next()) {
            if (__builtin_expect(p_ != last_, 1)) 
            {
#if defined (USE_LOOKUP_TABLE)
                uint32_t c = to_uint(*p_) - 48u;
                // check range:
                int v;
                if (__builtin_expect( ((c) < (111-48) and (v = lookupTable[c]) >= 0), 1)) {
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
//#if !defined (DEBUG)
//    __attribute__((always_inline))
//#endif
    inline
    unicode::code_point_t 
        parser<InputIterator, SourceEncoding, SemanticActions>::
    escaped_unicode() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(to_uint(*p_) == 'u');
        
        next();
        uint16_t euc1 = hex();
        if (__builtin_expect(state_, 1)) 
        {
            unicode::code_point_t unicode = euc1;
            if (__builtin_expect(unicode::isCodePoint(unicode), 1)) {
                if (__builtin_expect(not unicode::isSurrogate(unicode), 1)) {
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
//#if !defined (DEBUG)
//    __attribute__((always_inline))
//#endif
    void
    parser<InputIterator, SourceEncoding, SemanticActions>::
    escape_sequence()
    {
        assert(state_.error() == JP_NO_ERROR);        
        
        if (__builtin_expect(p_ != last_, 1)) 
        {
            unsigned int c = to_uint(*p_);
            uint8_t ascii;
            switch (c) {
                case '"':   ascii = '"'; break;  // string_buffer_pushback_ASCII(stringBuffer, '"');  next(); return;
                case '\\':  ascii = '\\'; break; // string_buffer_pushback_ASCII(stringBuffer, '\\'); next(); return;
                case '/':   ascii = '/'; break;  // string_buffer_pushback_ASCII(stringBuffer, '/');  next(); return;
                case 'b':   ascii = '\b'; break; // string_buffer_pushback_ASCII(stringBuffer, '\b'); next(); return;
                case 'f':   ascii = '\f'; break; // string_buffer_pushback_ASCII(stringBuffer, '\f'); next(); return;
                case 'n':   ascii = '\n'; break; // string_buffer_pushback_ASCII(stringBuffer, '\n'); next(); return;
                case 'r':   ascii = '\r'; break; // string_buffer_pushback_ASCII(stringBuffer, '\r'); next(); return;
                case 't':   ascii = '\t'; break; // string_buffer_pushback_ASCII(stringBuffer, '\t'); next(); return;
                case 'u':  { // escaped unicode
                    // note: escaped unicode will ignore option opt_pass_escaped_string
                    unicode::code_point_t cp = escaped_unicode();
                    if (__builtin_expect(state_, 1)) {
                        int result = string_buffer_pushback_unicode(cp); // returns zero on success, otherwise -1
                        if (__builtin_expect(result == 0, 1)) {
                            return;  // success 
                        } else {
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
            
            if (opt_pass_escaped_string_) {
                string_buffer_pushback_ASCII('\\');
                string_buffer_pushback_ASCII(c);
            } else {
                string_buffer_pushback_ASCII(ascii);
            }
            next();
            return;  // success
        }
        else {
            state_.error() = JP_UNEXPECTED_END_ERROR;
            sa_.error(state_.error(), state_.error_str());
        }
        
        assert(state_.error() != 0);
    }

    
    

#pragma mark - Parse Number
    
    
#if 0
    template <
    typename InputIterator
    , typename SourceEncoding
    , typename SemanticActions
    >
    inline
    void
    parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_number(number_desc_t& number_info)
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
        
        short digits = 0;
        bool isNegative = false;
        bool isDecimal = false;
        bool hasExponent = false;
        
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
                            number_string_buffer_.append_ascii('-');
                            break;
                        case '0':
                            s = number_int_is_zero;
                            ++digits;
                            number_string_buffer_.append_ascii('0');
                            break;
                        case '1'...'9':
                            s = number_state_int;
                            ++digits;
                            number_string_buffer_.append_ascii(c);
                            ++p_;
                            while (p_ != last_ and ( (c = to_uint(*p_)) >= '0' and c <= '9')) {
                                ++digits;
                                number_string_buffer_.append_ascii(c);
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
                            ++digits;
                            number_string_buffer_.append_ascii('0');
                            break;
                        case '1'...'9':
                            s = number_state_int;
                            ++digits;
                            number_string_buffer_.append_ascii(c);
                            ++p_;
                            while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9') {
                                ++digits;
                                number_string_buffer_.append_ascii(c);
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
                            isDecimal = true;
                            number_string_buffer_.append_ascii('.');
                            break;
                        case 'e':
                        case 'E':
                            s = number_state_exponent_start;
                            hasExponent = true;
                            number_string_buffer_.append_ascii('E');
                            break;
                        default: goto Number_done; // finished.
                    }
                    break;
                case number_state_int:
                    switch (c) {
                        case '.':
                            s = number_state_point;
                            isDecimal = true;
                            number_string_buffer_.append_ascii('.');
                            break;
                        case 'e':
                        case 'E':
                            s = number_state_exponent_start;
                            hasExponent = true;
                            number_string_buffer_.append_ascii('E');
                            break;
                        default: goto Number_done; // finished with integer
                    }
                    break;
                case number_state_point:
                    switch (c) {
                        case '0'...'9':
                            s = number_state_fractional;
                            ++digits;
                            number_string_buffer_.append_ascii(c);
                            ++p_;
                            while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9') {
                                ++digits;
                                number_string_buffer_.append_ascii(c);
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
                            hasExponent = true;
                            number_string_buffer_.append_ascii('E');
                            break;
                        default: goto Number_done; // finished with fractional or start exponent
                    }
                    break;
                case number_state_exponent_start:
#if 1
                    if (c == '-' or c == '+') {
                        s = number_state_exponent_sign;
                        number_string_buffer_.append_ascii(c);
                        exponentIsNegative = c == '-';
                        ++p_;
                        if (p_ == last_) {
                            goto Number_done;  // error
                        }
                        c = to_uint(*p_);
                    }
                    if (c >= '0' and c <= '9') {
                        s = number_state_exponent;
                        number_string_buffer_.append_ascii(c);
                        ++p_;
                        while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9') {
                            number_string_buffer_.append_ascii(c);
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
                            number_string_buffer_.append_ascii('-');
                            exponentIsNegative = true;
                            break;
                        case '+':
                            s = number_state_exponent_sign;
                            number_string_buffer_.append_ascii('+');
                            break;
                        case '0' ... '9':
                            s = number_state_exponent;
                            number_string_buffer_.append_ascii(c);
                            ++p_;
                            while (p_ != last_ and (c = to_uint(*p_)) >= '0' and c <= '9')
                            {
                                number_string_buffer_.append_ascii(c);
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
        
        
        typename number_desc_t::NumberType numberType;
        
        switch (s) {
            case number_int_is_zero:
            case number_state_int:
                numberType = number_desc_t::Integer;
                break;
            case number_state_fractional:
                numberType = number_desc_t::Decimal;
                break;
            case number_state_exponent:
                numberType = number_desc_t::Scientific;
                break;
            default:
                state_.error() = JP_BADNUMBER_ERROR;
                sa_.error(state_.error(), state_.error_str());
                return;
        }
        
        number_string_buffer_.terminate_if();
        
        typename number_string_buffer_t::const_buffer_type str_buffer =  number_string_buffer_.const_buffer();
        typename number_desc_t::const_buffer_type ni_buffer;
        ni_buffer.first = str_buffer.first;
        ni_buffer.second = str_buffer.second;
        
        number_info = number_desc_t(ni_buffer, numberType, digits);
        skip_whitespaces();
    }

#else

    template <
        typename InputIterator
      , typename SourceEncoding
      , typename SemanticActions
    >
    inline
    void
        parser<InputIterator, SourceEncoding, SemanticActions>::
    parse_number(number_desc_t& number_info)
    {
        typedef typename number_desc_t::NumberType number_type_t;
        
        assert(p_ != last_);

        typename number_string_buffer_t::const_buffer_type str_buffer;  // a std::pair<char const*, size_t>
        typename number_desc_t::const_buffer_type ni_buffer;
        number_type_t nt;
        int digits = 0;
        bool is_negative = false;
        
        unsigned int c = to_uint(*p_);
        if (c == '-') {
            number_string_buffer_.append_ascii('-');
            next();
            is_negative = true;
        }            
        if (p_ == last_ or not ((c = to_uint(*p_)) - '0' < 0x0A)) {
            goto BAD_NUMBER;
        }
        
        // one digit ...
        ++digits;
        nt = is_negative ? number_desc_t::Integer : number_desc_t::UnsignedInteger;
        number_string_buffer_.append_ascii(c);
        next();
        if (c != '0' /*not a leading zero*/) {
            while (p_ != last_ and ((c = to_uint(*p_)) - '0' < 0x0A)) {
                ++digits;
                number_string_buffer_.append_ascii(c);
                next();
            }
            if (p_ == last_) {
                goto GOOD_NUMBER;
            }
        } else {
            if (p_ == last_) {
                goto GOOD_NUMBER;
            }
        }
        c = to_uint(*p_);
        if (c == '.')
        {
            number_string_buffer_.append_ascii(c);
            next();
            if (p_ == last_ or not ((c = to_uint(*p_)) - '0' < 0x0A)) {
                goto BAD_NUMBER;
            }
            nt = is_negative ? number_desc_t::Decimal : number_desc_t::UnsignedDecimal;
            // one digit after decimal point
            ++digits;
            number_string_buffer_.append_ascii(c);
            next();
            while (p_ != last_ and ((c = to_uint(*p_)) - '0' < 0x0A))
            {
                ++digits;
                number_string_buffer_.append_ascii(c);
                next();
            }
            if (p_ == last_) {
                goto GOOD_NUMBER;
            }
        }
        if (c == 'e' or c == 'E') {
            number_string_buffer_.append_ascii(c);
            next();
            if (p_ == last_)
                goto BAD_NUMBER;
            c = to_uint(*p_);
            if (c == '-' or c == '+') {
                number_string_buffer_.append_ascii(c);
                next();
            }
            if (p_ == last_ or not ((c = to_uint(*p_)) - '0' < 0x0A)) {
                goto BAD_NUMBER;
            }
            nt = number_desc_t::Scientific;
 
            // one digit after e or E
            number_string_buffer_.append_ascii(c);
            next();
            while (p_ != last_ and ((c = to_uint(*p_)) - '0' < 0x0A))
            {
                number_string_buffer_.append_ascii(c);
                next();
            }
        }
        
    GOOD_NUMBER:
        number_string_buffer_.terminate_if();
        str_buffer =  number_string_buffer_.const_buffer();
        ni_buffer.first = str_buffer.first;
        ni_buffer.second = str_buffer.second;
        number_info = number_desc_t(ni_buffer, nt, digits);
        skip_whitespaces();
        return;

    BAD_NUMBER:
        state_.error() = JP_BADNUMBER_ERROR;
        sa_.error(state_.error(), state_.error_str());
        return;
    }
#endif
    
    
    
#pragma Mark - Errors    
    
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

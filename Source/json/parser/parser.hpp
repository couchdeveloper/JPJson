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


#include <boost/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>

//#include <netinet/in.h>
#include <assert.h>
#include <string.h>
#include <limits>
#include <alloca.h>

#include "parser_errors.hpp"
#include "semantic_actions_base.hpp"
//#incldue "json_path.hpp"
#include "json/utility/string_buffer.hpp"
#include "json/utility/number_builder.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/endian/endian.hpp"
#include "json/endian/byte_swap_iterator.hpp"

#if defined (DEBUG)
#include <iostream>
#endif


#if defined (BOOST_DISABLE_THREADS)
#warning boost threading disabled
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
    

    using internal::string_buffer;
    
    
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
        , typename Policies = parser_policies
    >
    class parser 
    {
    protected:
        
        typedef typename SemanticActions::error_t               sa_error_t;
        typedef SourceEncoding                                  source_encoding;
        typedef typename SemanticActions::encoding_t            string_buffer_encoding;
        typedef typename source_encoding::code_unit_type        code_t;  
        typedef string_buffer<string_buffer_encoding>           string_buffer_t;
        typedef typename string_buffer_t::code_unit_t           string_buffer_char_t;
        typedef numberbuilder::number_builder<64>               number_builder_t;        
        typedef string_buffer<string_buffer_encoding, 128>      key_string_buffer_t;
        typedef typename key_string_buffer_t::code_unit_t       key_string_char_t;
        typedef typename string_buffer_t::base_type             string_buffer_base_t;
        
        
        //
        // Static Assertions
        //
        // The parser class creates the iterator adapters based on InputIterator, 
        // creates a number of deduced types and uses these types throughout this 
        // class. There are a couple of requiments for this Input Iterator:
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, SourceEncoding>::value) );
        BOOST_STATIC_ASSERT( (sizeof(typename boost::iterator_value<InputIterator>::type) 
                              == sizeof(typename SourceEncoding::code_unit_type)) );
        
        // Currently, the parser requires that the endianess of its string_buffer
        // encoding matches the platform endianness or is UTF-8 encoding.
        BOOST_STATIC_ASSERT( (boost::mpl::or_<
                                boost::is_same<UTF_8_encoding_tag, string_buffer_encoding>,
                                boost::is_same<
                                    typename string_buffer_t::from_endian_t, 
                                    typename string_buffer_t::to_endian_t> 
                              >::value ));
        
    public:        
        //
        //  Types
        //
        typedef SemanticActions                         semantic_actions_t;
        typedef parser_state                            state_t;    // Current state of the parser
        typedef typename SemanticActions::result_type   result_t;   // The result of the sematic actions, e.g. a JSON container or AST.
        // Create an iterator adapter type which possibly swaps bytes if required
        // when dereferencing:
        typedef typename internal::byte_swap_iterator<InputIterator, SourceEncoding>    iterator;
                                                                            
    public:
        
        //
        //  C-tor
        //
        parser(SemanticActions& sa) 
        : sa_(sa), unicode_filter_(0)
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
        // Configure the parser from the options set in the semantic actions 
        // instance - and vice versa.
        void configure() 
        {
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
            
            sa_.inputEncoding(source_encoding().name());
            //semanticactions::non_conformance_flags ncon_flags = sa.extensions();
        }
        
        
    protected:
        SemanticActions&        sa_;
        iterator                p_;     // iterator adapter
        iterator                last_;  // iterator adapter
        state_t                 state_;
        string_buffer_t         string_buffer_;
        number_builder_t        number_builder_;
        unicode::filter::NoncharacterOrNULL unicode_filter_;
        
    private:        
        void parse_text();
        void parse_key_value_list();
        void parse_value();
        
        
#pragma mark -
#pragma mark Parse String
        //
        //  parse_string_imp()
        //  Three variants for each of the possible input encoding forms UTF-8, 
        //  UTF-16 and UTF-32 respectively.
        //
        template <typename SourceEncodingT>
        void parse_string_imp(string_buffer_base_t& stringBuffer, 
                              typename boost::enable_if<
                                boost::is_base_of<UTF_8_encoding_tag, SourceEncodingT>
                              >::type* = 0 );
        
        template <typename SourceEncodingT>
        void parse_string_imp(string_buffer_base_t& stringBuffer, 
                              typename boost::enable_if<
                                boost::is_base_of<UTF_16_encoding_tag, SourceEncodingT>  
                              >::type* = 0 );
        
        template <typename SourceEncodingT>
        void parse_string_imp(string_buffer_base_t& stringBuffer, 
                              typename boost::enable_if<
                                boost::is_base_of<UTF_32_encoding_tag, SourceEncodingT>
                              >::type* = 0 );

        // mapps to the corresponding specialization.
        void parse_string(string_buffer_base_t& stringBuffer) {
            this->parse_string_imp<source_encoding>(stringBuffer);
        }
        
        
        // Parse a multi byte UTF-8 sequence and push back the result
        // into the specified string buffer.
        // Prerequisites:
        //  - input source encoding is UTF-8
        //  - the current byte is a UTF-8 lead byte
        // If successful, returns the number of bytes of the multi bytes 
        // sequence [2 .. 4]. Otherwise, returns zero indicating an error.
        // If an error was detected, error state is set.
        // 
        int parse_string_utf8_mb(string_buffer_base_t& stringBuffer);
        
        
        
#pragma mark -
        void parse_number();
        void skip_whitespaces();
        void parse_object();
        void parse_array();
        bool match(const char* bytes, size_t n);
        
        unicode::code_point_t escaped_unicode();
        uint16_t hex();
        void escape_sequence(string_buffer_base_t& stringBuffer);
        
        
#pragma mark - String Buffer       
        
        // Pushback an ASCII to a string buffer.
        void string_buffer_pushback_ASCII(string_buffer_base_t& stringBuffer, char ch) { 
            assert(ch <= 0x7F);
            // An ASCII can be converted implicitly to any of the encoding forms:
            // UTF-8, UTF-16, and UTF-32.
            stringBuffer.append_ascii(ch); 
        }
        
        //
        // string_buffer_pushback_unicode
        //
        // Parameter unicode shall be a valid unicode scalar value.
        // Test if the given unicode matches the predicate of the parser's
        // unicode filter, and if this evaluates to true, apply the filter's 
        // replacement policy. If an invalid unicode will be replaced by a
        // replacement character, the function will not signal an error.
        // 
        // Returns the number of appended code units to the string buffer.
        // In case of an error, returns -1.
        // The function only returns 0 in case of an error while appending
        // the unicode to the string buffer, which in turn only occurs when
        // the string buffer could not be resized.
        // 
        int string_buffer_pushback_unicode(string_buffer_base_t& stringBuffer, 
                                           unicode::code_point_t unicode) 
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
            std::size_t count = stringBuffer.append_unicode(unicode);
            assert(count > 0);
            return (int)count;
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
        , typename Policies
    >
    inline 
    parser_error_type 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse(InputIterator& first, InputIterator last, bool skipTrailingWhitespaces) 
    {
        p_ = iterator(first);
        last_ = iterator(last);
        sa_.parse_begin();
        parse_text();
        parser_error_type result = state_.error();
        if (state_.error() == JP_NO_ERROR and skipTrailingWhitespaces and p_ != last_) {
            // Increment past the last significant character:
            ++p_;
            skip_whitespaces();
        }
        first = p_.base();  // TODO: use statement first = p_;  but this would require a type cast operator defined for iterator_adaptor        
        
        return result;
    }
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    reset() {
        sa_.clear();
        state_.clear();
        string_buffer_.reset();
        number_builder_.clear();
    }
    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    skip_whitespaces() 
    {
        assert(state_.error() == JP_NO_ERROR);

        while (p_ != last_) 
        {
            switch (static_cast<code_t>(*p_)) {
                case code_t(' '):
                case code_t('\t'):
                case code_t('\n'):
                case code_t('\r'):
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
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
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
        code_t c = *p_;
        switch (c) {
            case code_t('['):
                sa_.begin_array();
                parse_array();
                if (state_) {
                    sa_.end_array();
                    sa_.parse_end();
                }
                break;
                
            case code_t('{'):
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
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_array() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(*p_ == '[');
        
        ++p_;        
        // check if the array is empty:
        skip_whitespaces();
        if (p_ != last_) {
            code_t c = *p_;
            if  (c == code_t(']')) {
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
                            code_t c = *p_;
                            if (c == code_t(',')) {
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
                        code_t c = *p_;
                        if (c == code_t(']')) {
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
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_object() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(*p_ == '{');
        
        ++p_;
        // check if the object is empty:
        skip_whitespaces();
        if (p_ != last_) {
            code_t c = *p_;
            if  (c == code_t('}')) {
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
                        code_t c = *p_;
                        if (c == code_t('}')) {
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
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_key_value_list() 
    {
        assert(state_.error() == JP_NO_ERROR);
        
        // string ':' value [',' string ':' value]*
        
        // use a StringBuffer with a small auto buffer for the key string:
        key_string_buffer_t keyStringBuffer;
                
        //bool done = false;
        size_t index = 0;
        while (__builtin_expect(p_ != last_, 1)) {
            code_t c = *p_;
            if (c == code_t('"')) 
            {
                // first, get the key ...
                parse_string(keyStringBuffer);
                if (state_) {
                    // make a string_t and push it onto the stack
                    //keyStringBuffer.terminate_if();
                    sa_.push_key(keyStringBuffer.buffer(), keyStringBuffer.size());
                    if (p_ != last_) {
                        // ... then, eat the key_value separator ...
                        c = *p_;                        
                        if (c == ':') {
                            ++p_;
                            skip_whitespaces();
                            if (p_ != last_)
                            {    
                                // ... finally, get a value and put it onto sa_'s stack
                                sa_.begin_value_with_key(keyStringBuffer.buffer(), keyStringBuffer.size(), index);
                                parse_value();  // whitespaces skipped.
                                sa_.end_value_with_key(keyStringBuffer.buffer(), keyStringBuffer.size(), index);
                                if (state_) 
                                {
                                    // Note: We populate the object at end_object().
                                    if (p_ != last_) {
                                        c = *p_;
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
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_value() 
    {                
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        
        code_t c = *p_;
        switch (c) {
            case code_t('"'):
                parse_string(string_buffer_);
                if (state_) {                    
                    // action_string_end
                    // note: p() points to the start of next token
                    //string_buffer_.terminate_if();
                    sa_.push_string(string_buffer_.buffer(), string_buffer_.size());
                } else {
                    // handle error string
                }
                break;
                
            case code_t('{'):
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
                break;
                
            case code_t('0') ... code_t('9'):
            case code_t('-'):
                number_builder_.clear();
                parse_number();
                if (state_) {
                    // parse_number does not skip whitespaces
                    skip_whitespaces();
                    // note: p() points to the start of next token
                    sa_.push_number(number_builder_.number());
                } else {
                    // handle error number
                    assert(state_.error() != 0);
                }
                break;
                
            case code_t('['):
                sa_.begin_array();
                parse_array();
                if (state_) {
                    sa_.end_array();
                    ++p_;
                    skip_whitespaces();
                }
                break;
                
            case code_t('t'):
                if (match("true", 4)) {
                    // got a "true"
                    // action_value_true
                    sa_.push_boolean(true);
                    skip_whitespaces();
                }
                break;
                
            case code_t('f'):
                if (match("false", 5)) {
                    // action_value_false
                    sa_.push_boolean(false);
                    skip_whitespaces();
                } 
                break;
                
            case code_t('n'):
                if (match("null", 4)) {
                    // action_value_null
                    sa_.push_null();
                    skip_whitespaces();
                }
                break;    
                
            case 0:
                state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                sa_.error(state_.error(), state_.error_str());
                break;
                
            default:
                state_.error() = JP_EXPECTED_VALUE_ERROR;
                sa_.error(state_.error(), state_.error_str());
        }
        /*
        if (state_) {
            skip_whitespaces();
        }
        */
    }
    
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
        , typename Policies
    >
    inline 
    bool 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    match(const char* s, size_t n)
    {
        while (n != 0 and p_ != last_ and code_t(*s) == *p_) {
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
        , typename Policies
    >
    inline 
    uint16_t 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    hex() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        
        uint16_t uc = 0;        
        // expecting 4 hex digits:
        for (int i = 0; i < 4; ++i, ++p_) {
            if (__builtin_expect(p_ != last_, 1)) 
            {
                code_t  c = *p_;
                switch (c) {
                    case '0' ... '9': uc = (uc << 4) + (unsigned int)(c - '0'); break;
                    case 'a' ... 'f': uc = (uc << 4) + ((unsigned int)(c - 'a') + 10U); break;
                    case 'A' ... 'F': uc = (uc << 4) + ((unsigned int)(c - 'A') + 10U); break;
                    default:
                        state_.error() = JP_INVALID_HEX_VALUE_ERROR;
                        sa_.error(state_.error(), state_.error_str());
                        return 0;
                }
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
        , typename Policies
    >
    inline 
    unicode::code_point_t 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    escaped_unicode() 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(*p_ == 'u');
        
        ++p_;
        unicode::utf16_code_unit euc1 = hex();
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
                            unicode::utf16_code_unit euc2 = hex();
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
    // p_ shall point to the initial escape character ('\').
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
        , typename Policies
    >
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    escape_sequence(string_buffer_base_t& stringBuffer) 
    {
        assert(state_.error() == JP_NO_ERROR);        
        assert(p_ != last_);
        assert(*p_ == code_t('\\'));
        
        ++p_;
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
#if 1                        
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
#else
                        {
                        // Got a valid Unicode scalar value.
                        // Check whether this unicode is allowed (e.g., filter
                        // noncharacters or any other Unicode code points not
                        // allowed):
                        if (unicode_filter_(unicode)) {
                            if (unicode_filter_.replace()) {
                                unicode = unicode_filter_.replacement(unicode);
                                assert(unicode != 0);
                            } 
                            else 
                            {
                                // Filtered invalid Unicode code point and did not
                                // replace it. This forces the parser to stop.
                                if (unicode::isNonCharacter(unicode)) {
                                    state_.error() = JP_UNICODE_NONCHARACTER_ERROR;
                                }
                                else if (unicode == 0) {
                                    state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;  // TODO: use better error code
                                }
                                else {
                                    state_.error() = JP_UNICODE_REJECTED_BY_FILTER;
                                }
                                break;
                            }
                        }
                        // Validity of the unicode code point is checked above 
                        // already.
                        // append_unicode() should succeed here. Otherwise this 
                        // is a logic error and we throw an exception, or signal 
                        // the corresponding error.
                        assert(state_.error() == 0);
                        size_t result = stringBuffer.append_unicode(unicode);
                        }
                        if (result > 0) {
                            return;  // success, exit
                        }
                        else {
                            state_.error() = JP_INTERNAL_LOGIC_ERROR;
                        }
#endif                        
                        
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
    
    
    //
    // Pushes a UTF-8 multi-byte sequence from the input onto the string buffer.
    // Returns the number of bytes for multi-byte character.
    // p_ shall point to a UTF-8 lead byte (utf8_is_lead() == true) which must 
    // be asserted by the caller.
    // Returns 0 if this is not a valid mb sequence.
    // If return value is greater than zero, p() points to the start of the next 
    // boundary, otherwise it points to the first occurence of the invalid byte.
    //
    // This function is only enabled for UTF-8 encoded input
    // 
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
        , typename Policies
    >
    inline 
    int 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_string_utf8_mb(string_buffer_base_t& stringBuffer) 
    {
        assert(state_.error() == JP_NO_ERROR);
        assert(p_ != last_);
        assert(unicode::utf8_is_lead(*p_));

        // read the UTF-8 into a code point:
        unicode::code_point_t cp = 0;
        // Use a conversions which does not accept surrogates and noncharacters. 
        // Per default, the filter shall signal errors if it matches characters.
        //  //Replace invalid characters with Unicode Replacement Character.
        // The safe conversions do not accept surrogates, so we only need to
        // check for noncharacters:
        int result = unicode::convert_one(p_, last_, unicode::UTF_8_encoding_tag(), cp, unicode_filter_);
        // Possible errors:
        //      E_TRAIL_EXPECTED:           trail byte expected
        //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
        //      E_INVALID_START_BYTE:       invalid start byte
        //      E_INVALID_CODE_POINT:       detected surrogate or code point is invalid Unicode 
        //      E_PREDICATE_FAILED          got a noncharacter
        if (result > 0) {
            result = (int)stringBuffer.append_unicode(cp); 
            assert(result > 0);
            return result;
        }
        else {
            switch (result) {
                case unicode::E_UNEXPECTED_ENDOFINPUT:
                    state_.error() = JP_UNEXPECTED_END_ERROR;
                    break;
                case unicode::E_PREDICATE_FAILED:
                    // Filtered invalid Unicode code point and did not
                    // replace it. This forces the parser to stop.
                    if (unicode::isNonCharacter(cp))
                        state_.error() = JP_UNICODE_NONCHARACTER_ERROR;
                    else if (cp == 0){
                        state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                    }
                    else {
                        state_.error() = JP_UNICODE_REJECTED_BY_FILTER;
                    }
                    break;
                //case E_TRAIL_EXPECTED:
                //case E_INVALID_START_BYTE:
                //case E_INVALID_CODE_POINT:
                default:
                    state_.error() = JP_ILLFORMED_UNICODE_SEQUENCE_ERROR;
            }
            sa_.error(state_.error(), state_.error_str());
            return  0; // ERROR: UTF-8 sequence invalid                    
        }
    }
    
    
    // Parse a possibly escaped-unicode UTF-8 encdoded string enclosed in 
    // double quotes.
    // This functions is enabled only if source encoding equals UTF-8
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
        , typename Policies
    >
    template <typename EncodingT>
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_string_imp(string_buffer_base_t& stringBuffer, 
                     typename boost::enable_if<
                        boost::is_base_of<UTF_8_encoding_tag, EncodingT> 
                     >::type* ) 
    {
        assert(state_.error() == JP_NO_ERROR); 
        assert(p_ != last_);
        assert(*p_ == code_t('"'));
        
        stringBuffer.reset();

        ++p_;
        while (__builtin_expect(p_ != last_, 1)) 
        {
            code_t c = *p_;
            // if (__builtin_expect( (unsigned(c) - 0x20u) < 0x60u, 1))
            // if (unsigned)c >= 0x20u and (unsigend)c < 0x80u)
            if ( (unsigned(c) - 0x20u) < 0x60u) 
            {
                switch (c) {
                    case '"':
                        ++p_;
                        skip_whitespaces();
                        return; // done
                        
                    case '\\': // escape sequence
                        escape_sequence(stringBuffer);
                        if (!state_) {
                            // error parsing escape sequence
                            return; // error state already set
                        }
                        continue;

                    default:
                        string_buffer_pushback_ASCII(stringBuffer, c);
                        ++p_;
                        continue;
                }
            }
            else if (unicode::utf8_is_lead(c)) {
                // multibyte  sequence: 
                int mb_len = parse_string_utf8_mb(stringBuffer);
                if (mb_len <= 0) {
                    assert(state_.error() != JP_NO_ERROR);
                    return; //  error state already set
                }
                continue;
            }
            else 
            {   
                if (c < 0x20u) {
                    if  (c == 0) {
                        state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                    }
                    else {
                        state_.error() =  JP_CONTROL_CHAR_NOT_ALLOWED_ERROR;  // error: control character not allowed
                    }
                } 
                else  {
                    // possibly trail byte, or invalid Unicode code point. This is mal-formed UTF-8
                    state_.error() = JP_ILLFORMED_UNICODE_SEQUENCE_ERROR;
                }
                break; //done = true;                    
            }
        }  // while (p != last_)
        
        if (p_ == last_) {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        assert(state_.error() != 0);
        sa_.error(state_.error(), state_.error_str());
    }
    
    
    // Parse a possibly escaped-unicode UTF-16 encoded string enclosed in 
    // double quotes.
    // This functions is enabled only if source encoding equals UTF-16.
    // p_ shall point to the initial quote '"'.
    //
    //  TODO: This function is not yet optimized!!
    //  
    template <
    typename InputIterator
      , typename SourceEncoding
      , typename SemanticActions
      , typename Policies
    >
    template <typename EncodingT>
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_string_imp(string_buffer_base_t& stringBuffer, 
                     typename boost::enable_if<
                        boost::is_base_of<UTF_16_encoding_tag, EncodingT>
                     >::type* ) 
    {
        assert(state_.error() == JP_NO_ERROR);        
        assert(p_ != last_);
        assert(*p_ == code_t('"'));
        
        stringBuffer.reset();
        ++p_;
        while (p_ != last_) 
        {
            code_t c = *p_;
            if ((unsigned(c) - 0x20u) < 0x60u)  // ASCII except control-char?
            {
                switch (c) {
                    case '"':
                        ++p_;
                        skip_whitespaces();
                        return; // done
                        
                    case '\\': // escape sequence
                        escape_sequence(stringBuffer);
                        if (!state_) {
                            // error parsing escape sequence
                            return; // error state already set
                        }
                        continue;
                        
                    default:
                        string_buffer_pushback_ASCII(stringBuffer, c);
                        ++p_;
                        continue;
                }
            }
            else if (not unicode::isSurrogate(static_cast<unicode::code_point_t>(c))) 
            {
                //if ( unsigned(c) - 0x20u  < (unicode::CodepointMax - 0x20)) {
                if (c >= 0x20u and unicode::isCodePoint(static_cast<unicode::code_point_t>(c))) {
                    // valid code point and not surrogate -> valid Unicode scalar value
                    // with string_buffer_pushback_unicode() check for noncharacters and perform replacement
                    int result = string_buffer_pushback_unicode(stringBuffer, static_cast<unicode::code_point_t>(c));
                    if (result <= 0) {
                        assert(state_.error() != 0);  // internal error
                        return;  // error state already set
                    }
                    ++p_;
                    continue;
                }
                else { // Control char or U+0000                        
                    if  (c == 0) {
                        state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                    }
                    else if (c < 0x20) {
                        state_.error() =  JP_CONTROL_CHAR_NOT_ALLOWED_ERROR;  // error: control character not allowed
                    }
                    else {
                        state_.error() = JP_INVALID_UNICODE_ERROR;
                    }
                    break;
                }
            }
            else 
            {
                // Read surrogate pair
                if (unicode::utf16_is_high_surrogate(static_cast<unicode::code_point_t>(c))) 
                {
                    ++p_;  // consume the heigh surrogate ...
                    if (p_ != last_) {
                        code_t c2 = *p_; //  ... and read next code unit
                        if (unicode::utf16_is_low_surrogate(static_cast<unicode::code_point_t>(c2))) {
                            unicode::code_point_t unicode = unicode::utf16_surrogate_pair_to_code_point(c, c2);
                            int result = string_buffer_pushback_unicode(stringBuffer, unicode);
                            if (result <= 0) 
                            {
                                assert(state_.error() != 0);
                                return;  // error state already set
                            }
                            ++p_;
                        } else {
                            // illformed UTF-16 code sequence
                            state_.error() = JP_EXPECTED_LOW_SURROGATE_ERROR;
                            break;
                        }
                    }
                    else {
                        //state_.error() = JP_UNEXPECTED_END_ERROR; 
                        break;
                    }
                } 
                else {
                    // illformed UTF-16 code sequence: got a low surrogate
                    state_.error() = JP_EXPECTED_HIGH_SURROGATE_ERROR; 
                    break;
                }
            }
        }  // while 
                
        if (p_ == last_) {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        assert(state_.error() != 0);
        sa_.error(state_.error(), state_.error_str());
        
    }  
    
    
    // Parse a possibly escaped-unicode UTF-32 encoded string enclosed in 
    // double quotes.
    // This functions is enabled only if source encoding equals UTF-32.
    // p_ shall point to the initial quote '"'.
    //  
    template <
    typename InputIterator
    , typename SourceEncoding
    , typename SemanticActions
    , typename Policies
    >
    template <typename EncodingT>
    inline 
    void 
    parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    parse_string_imp(string_buffer_base_t& stringBuffer, 
                     typename boost::enable_if<
                     boost::is_base_of<UTF_32_encoding_tag, EncodingT>
                     >::type* ) 
    {
        assert(state_.error() == JP_NO_ERROR);        
        assert(p_ != last_);
        assert(*p_ == code_t('"'));
        
        stringBuffer.reset();
        ++p_;
        while (__builtin_expect(p_ != last_, 1)) 
        {
            code_t c = *p_;
            if (__builtin_expect((uint32_t(c) - 0x20u) < 0x60u, 1))  // ASCII except control-char?
            {
                switch (c) {
                    default:
                        string_buffer_pushback_ASCII(stringBuffer, c);
                        ++p_;
                        continue;
                    case '"':
                        ++p_;
                        skip_whitespaces();
                        return; // done
                        
                    case '\\': // escape sequence
                        escape_sequence(stringBuffer);
                        if (!state_) {
                            // error parsing escape sequence
                            return; // error state already set
                        }
                        continue;
                }
            }
            else if (c >= 0x20u and unicode::isCodePoint(static_cast<unicode::code_point_t>(c))) {
                /*if  ( unsigned(c) - 0x20u  < (unicode::CodepointMax - 0x20)) { */                    
                // with string_buffer_pushback_unicode() check for noncharacters and perform replacement
                int result = string_buffer_pushback_unicode(stringBuffer, static_cast<unicode::code_point_t>(c));
                if (result <= 0) {
                    assert(state_.error() != 0);  // internal error
                    return;  // error state already set
                }
                ++p_;
                continue;
            }
            else { // Control char or U+0000                        
                if  (c == 0) {
                    state_.error() = JP_UNICODE_NULL_NOT_ALLOWED_ERROR;
                }
                else if (c < 0x20) {
                    state_.error() =  JP_CONTROL_CHAR_NOT_ALLOWED_ERROR;  // error: control character not allowed
                }
                else {
                    state_.error() = JP_INVALID_UNICODE_ERROR;
                }
                break;
            }
        }  // while 
        
        if (p_ == last_) {
            state_.error() = JP_UNEXPECTED_END_ERROR;
        }
        assert(state_.error() != 0);
        sa_.error(state_.error(), state_.error_str());
        
    }  

#pragma mark -
    
    template <
          typename InputIterator
        , typename SourceEncoding
        , typename SemanticActions
        , typename Policies
    >    
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
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
            code_t c = *p_;
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
                            while (p_ != last_ and (*p_ >= '0' and *p_ <= '9')) {
                                number_builder_.push_digit(*p_);
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
                            while (p_ != last_ and (c = *p_) >= '0' and c <= '9') {
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
                            while (p_ != last_ and (c = *p_) >= '0' and c <= '9') {
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
                        c = *p_;
                    } 
                    if (c >= '0' and c <= '9') {
                        s = number_state_exponent; 
                        number_builder_.push_exponent_start(c);
                        ++p_;
                        while (p_ != last_ and (c = *p_) >= '0' and c <= '9') {
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
                            while (p_ != last_ and (c = *p_) >= '0' and c <= '9')
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
                            while (p_ != last_ and (c = *p_) >= '0' and c <= '9')
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
      , typename Policies
    >    
    inline 
    void 
        parser<InputIterator, SourceEncoding, SemanticActions, Policies>::
    throwLogicError(const char* msg) {
        throw std::logic_error(msg);
    }
    
    
} // namespace json


#endif // JSON_PARSER_H

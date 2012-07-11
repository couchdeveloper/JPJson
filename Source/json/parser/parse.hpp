//
//  parse.hpp
//
//
//  Created by Andreas Grosam on 5/23/11.
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

#ifndef JSON_PARSE_HPP
#define JSON_PARSE_HPP


#include "json/config.hpp"
#include <boost/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits.hpp>
#include "parser.hpp"
#include "semantic_actions_base.hpp"
#include "json/unicode/unicode_detect_bom.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "json/utility/flags.hpp"


//#define JSON_PARSE_HPP_LOG_DEBUG

#if defined (JSON_PARSE_HPP_LOG_DEBUG)
#include <pthread.h>
#endif

namespace json { 
    
    //
    // Option flags for the parse functions
    //
    // Parse Option flags can be set in parse functions where a parameter 
    // semantic actions is omitted. Otherwise, parse options can be set 
    // directly through the semantic actions object.
    // 
    
    struct parse_options
    {
        enum E 
        {
            None                        = 0,
            
            // Mutual exclusive
            SignalErrorOnUnicodeNoncharacter    = 1UL << 0,
            SubstituteUnicodeNoncharacters      = 1UL << 1,
            SkipUnicodeNoncharacters            = 1UL << 2,
            
            // Mutual exclusive
            LogLevelDebug                       = 1UL << 3,
            LogLevelWarning                     = 1UL << 4,
            LogLevelError                       = 1UL << 5,
            LogLevelNone                        = 1UL << 6,
            
            // non_conformance_flags (can be ored)
            // not yet implemented!
            AllowComments                       = 1UL << 7,              
            AllowControlCharacters              = 1UL << 8,     
            AllowLeadingPlusInNumbers           = 1UL << 9,
            AllowLeadingZerosInIntegers         = 1UL << 10,

            
            NoncharacterHandling = SignalErrorOnUnicodeNoncharacter | SubstituteUnicodeNoncharacters | SkipUnicodeNoncharacters,
            LogLevel = LogLevelDebug | LogLevelWarning | LogLevelError |  LogLevelNone,
            NonConformanceFlags = AllowComments | AllowControlCharacters | AllowLeadingPlusInNumbers | AllowLeadingZerosInIntegers
        };
        
        typedef E enum_type;
    };    
    
    
    typedef json::utility::flags<parse_options> ParseOptions;
}

UTILITY_DEFINE_FLAG_OPERATORS(json::parse_options);



namespace json { namespace parse_internal {
    
    using json::unicode::encoding_traits;
    
    const char* const JP_JSON_EXTRA_CHARACTERS_AT_END_String = "extra characters at end of json document not allowed";
    
    // Parses multiple json documents unless sa.additionalInputIsError()
    // returns true.
    template <typename Parser, typename SemanticActions, typename Iterator>
    inline
    bool parse_loop(Parser& parser, SemanticActions& sa, Iterator& first, Iterator last) 
    {
        typedef typename SemanticActions::error_t       sa_error_t;
        typedef typename Parser::source_encoding_type   encoding_t;
        
        size_t count = 0;   // count the number of json documents
        bool done = false;  // If true, terminates the parse loop
        bool result;        // Becomes true IFF all documents have been parsed successful
        while (not done) {
            ++count;
#if defined (JSON_PARSE_HPP_LOG_DEBUG)
            printf("[%x] json::parse parse_loop: start parsing document %lu\n",
                   pthread_mach_thread_np(pthread_self()), count);
#endif         
            const bool skipTrailingWhitespaces = false;    
            json::parser_error_type parser_result = parser.parse(first, last, skipTrailingWhitespaces);
            if (parser_result == json::JP_NO_ERROR) {
                // The parser has successfully parsed a JSON document.
                // The parser will never increment to past-the-end position if
                // there was no error (skipTrailingWhitespaces equals false):
                assert(first != last);
                // 'first' now points to the last significant character of the
                // JSON text, that is either a ']' or a '}'.                
                // We only try to advance past the last significant character of 
                // the previous JSON document, if we have to check for spurious 
                // trailing bytes, or of we have to parse multiple JSON documents:
                if (sa.parseMultipleDocuments() or not sa.ignoreSpuriousTrailingBytes()) 
                {
                    ++first;                    
                    // Check if 'first' now possibly points to a sequence of 
                    // out-of-bound characters. We may want to skip them:                
                    if (first == last) {
                        done = true; // terminate parse loop
                        result = true;
                    } else {
                        // Skip out-of-bound characters:
                        // The only "allowed" out-of-bound characters are white-spaces,
                        // Unicode NULL characters (U+0000) and EOF (-1).
                        // An Unicode NULL (U+0000) or an EOF (-1) character will 
                        // terminate the parse loop, regardless if iterator 'first' 
                        // did not yet reach iterator 'last':
                        while (first != last) {
                            uint32_t c = encoding_traits<encoding_t>::to_uint(*first);
                            if (c == uint32_t(' ') 
                                or c == uint32_t('\t') 
                                or c == uint32_t('\n')
                                or c == uint32_t('\r')) {
                                ++first;
                                continue;
                            }
                            else if (c == 0 or c == EOF) {
                                done = true;  // terminate parse loop
                                result = true; // ALL documents have been parsed successful
                                break;
                            }
                            else {
                                //any other character
                                break;
                            }
                        } // while
                    }
                } 
                if (sa.parseMultipleDocuments()) 
                {
                    if (first == last) {
                        // Finished parsing multiple JSON documents with no error,
                        // so, the final result becomes true:
                        done = true;
                        result = true;
                    } 
                    else {
                        if (done) {
                            // The parse loop was terminated through an Unicode NULL 
                            // or EOF. This is not an error, but worth a warning:
                            sa.logger().log(json::utility::LOG_WARNING, 
                                            "parse loop prematurely terminated due to "
                                            "out-of-bound Unicode Null (U+0000) or EOF\n");
                        } else {
                            if (not sa.ignoreSpuriousTrailingBytes()) {
                                // requires a next JSON document, continue parsing
                            }
                            else 
                            {
                                // If there is no further JSON document, stop:
                                uint32_t c = encoding_traits<encoding_t>::to_uint(*first);
                                if (c != '{' and c != '[') {   
                                    done = true;
                                    result = true;
#if defined (DEBUG)                    
                                    sa.logger().log(json::utility::LOG_DEBUG, 
                                                    "INFO: detected out-of-bound characters after valid JSON\n");
#endif                    
                                }
                                else {
                                    // continue parsing
                                }
                            }
                        }
                    }
                }
                else if (not sa.ignoreSpuriousTrailingBytes())
                {
                    // Parsing a single JSON document and not ignoring trailing 
                    // bytes:
                    // We need to check whether there are spurious characters
                    // after the last significant character of the JSON text.                    
                    if (first != last) {
                        if (not done) {
                            // There seems to exist additional input which are not
                            // white spaces or we did not yet receive EOF or U+0000.
                            // Anyway, this is defined bogus:
                            result = false;
                            done = true;
                            sa.error(JP_JSON_EXTRA_CHARACTERS_AT_END, JP_JSON_EXTRA_CHARACTERS_AT_END_String);
                        }
                        else {
                            // The parse loop was terminated through an Unicode
                            // NULL or EOF. This is not an error, but worth a warning:
                            result = true;
                            sa.logger().log(json::utility::LOG_WARNING, 
                                            "detected out-of-bound Unicode Null (U+0000) or EOF after valid JSON\n");
                        }
                    } else {
                        // Finished parsing a single document, and no spurious trailing bytes.
                        result = true;
                        done = true;
                    }
                }
                else {
                    // Finished parsing one JSON document with no error, and ignoring
                    // spurious trailing bytes (if any), so, the final result becomes true:
                    done = true;
                    result = true;
#if defined (DEBUG)                    
                    if (first != last) {
                        sa.logger().log(json::utility::LOG_DEBUG, 
                                        "INFO: detected out-of-bound characters after valid JSON\n");
                    }
#endif                    
                }
            } 
            else 
            {
                // The parser detected an error while parsing a JSON document.
                // This terminates the parse loop and the final result becomes false:
                result = false;
                done = true;
            }
#if defined (JSON_PARSE_HPP_LOG_DEBUG)
            printf("[%x] json::parse parse_loop: finished parsing document (%lu) with result %d (%s)\n",
                   pthread_mach_thread_np(pthread_self()), count, parser_result, json::parser_error_str(parser_result));
#endif            
            
        }
        sa.finished();
        
        return result;        
    }
    
    
    /*
    template <typename ParserT>
    inline void 
    parser_apply_flags(ParserT& parser, json::parse_options::option_flags flags) 
    {        
        if (json::parse_options::NoncharacterHandling & flags) {
            if (json::parse_options::SignalErrorOnNoncharacter & flags) {
                parser.unicode_noncharacter_handling(json::parser_options::SignalErrorOnNoncharacter);
            }
            else if (json::parse_options::ReplaceNoncharacters & flags) {
                parser.unicode_noncharacter_handling(json::parser_options::ReplaceNoncharacters);
            }
            else if (json::parse_options::SkipNoncharacters & flags) {
                parser.unicode_noncharacter_handling(json::parser_options::SkipNoncharacters);
            }
            
        }
    }
     */

    template <typename SemanticActionsT>
    inline void 
    semantic_actions_apply_flags(SemanticActionsT& sa, json::ParseOptions flags) 
    {        
        if (json::ParseOptions::LogLevel & flags) {
            if (json::ParseOptions::LogLevelDebug & flags) {
                sa.log_level(json::semanticactions::LogLevelDebug);
            }
            else if (json::ParseOptions::LogLevelWarning & flags) {            
                sa.log_level(json::semanticactions::LogLevelWarning);
            }
            else if (json::ParseOptions::LogLevelError & flags) {            
                sa.log_level(json::semanticactions::LogLevelError);
            }
            else if (json::ParseOptions::LogLevelNone & flags) {            
                sa.log_level(json::semanticactions::LogLevelNone);
            }
        }
        
        if (json::ParseOptions::NoncharacterHandling & flags) {
            if (json::ParseOptions::SignalErrorOnUnicodeNoncharacter & flags) {
                sa.unicode_noncharacter_handling(json::semanticactions::SignalErrorOnUnicodeNoncharacter);
            }
            else if (json::ParseOptions::SubstituteUnicodeNoncharacters & flags) {
                sa.unicode_noncharacter_handling(json::semanticactions::SubstituteUnicodeNoncharacter);
            }
            else if (json::ParseOptions::SkipUnicodeNoncharacters & flags) {
                sa.unicode_noncharacter_handling(json::semanticactions::SkipUnicodeNoncharacters);
            }
        }
        
        if (json::ParseOptions::NonConformanceFlags & flags) {
            using json::semanticactions::ExtensionOptions;
            ExtensionOptions opts = ExtensionOptions::AllowNone;
            if (json::ParseOptions::AllowComments & flags) {
                opts |= ExtensionOptions::AllowComments;
            }
            if (json::ParseOptions::AllowControlCharacters & flags) {
                opts |= ExtensionOptions::AllowControlCharacters;
            }
            if (json::ParseOptions::AllowLeadingPlusInNumbers & flags) {
                opts |= ExtensionOptions::AllowLeadingPlusInNumbers;
            }
            if (json::ParseOptions::AllowLeadingZerosInIntegers & flags) {
                opts |= ExtensionOptions::AllowLeadingZerosInIntegers;
            }
            sa.extensions(opts);
        }
        
    }
    

}}  // namespace json::parse_internal




namespace json {
    
    //
    //  Detect Encoding
    //
    // Tries to determine the Unicode encoding of the input starting at 
    // first. A BOM shall not be present (you might check with function 
    // json::unicode::detect_bom() whether there is a BOM, in which case 
    // you don't need to call this function when a BOM is present).
    //
    // Return values:
    // 
    //   json::unicode::UNICODE_ENCODING_UTF_8
    //   json::unicode::UNICODE_ENCODING_UTF_16LE
    //   json::unicode::UNICODE_ENCODING_UTF_16BE
    //   json::unicode::UNICODE_ENCODING_UTF_32LE
    //   json::unicode::UNICODE_ENCODING_UTF_32BE
    //
    //  -1:     unexpected EOF
    //  -2:     unknown encoding
    //
    // Note:
    // detect_encoding() requires to read ahead a few bytes in order to deter-
    // mine the encoding. In case of InputIterators, this has the consequences
    // that these iterators cannot be reused, for example for a parser.
    // Usually, this requires to reset the istreambuff, that is using the 
    // member functions pubseekpos() or pupseekoff() in order to reset the get 
    // pointer of the stream buffer to its initial position.
    // However, certain istreambuf implementations may not be able to set the    
    // stream pos at arbitrary positions. In this case, this method cannot be
    // used and other edjucated guesses to determine the encoding may be
    // needed.
    
    template <typename Iterator>    
    inline int 
    detect_encoding(Iterator first, Iterator last) 
    {
        // Assuming the input is Unicode!
        // Assuming first character is ASCII!
        
        // The first character must be an ASCII character, say a "[" (0x5B)
        
        // UTF_32LE:    5B 00 00 00
        // UTF_32BE:    00 00 00 5B
        // UTF_16LE:    5B 00 xx xx
        // UTF_16BE:    00 5B xx xx
        // UTF_8:       5B xx xx xx
        
        uint32_t c = 0xFFFFFF00;
        
        while (first != last) {
            uint32_t ascii;
            if (static_cast<uint8_t>(*first) == 0)
                ascii = 0; // zero byte
            else if (static_cast<uint8_t>(*first) < 0x80)
                ascii = 0x01;  // ascii byte
            else if (*first == EOF)
                break;
            else
                ascii = 0x02; // non-ascii byte, that is a lead or trail byte
            c = c << 8 | ascii;
            switch (c) {
                    // reading first byte
                case 0xFFFF0000:  // first byte was 0
                case 0xFFFF0001:  // first byte was ASCII
                    ++first;
                    continue;
                case 0xFFFF0002:
                    return -2;  // this is bogus
                    
                    // reading second byte
                case 0xFF000000:    // 00 00 
                    ++first;
                    continue;
                case 0xFF000001:    // 00 01
                    return json::unicode::UNICODE_ENCODING_UTF_16BE;
                case 0xFF000100:    // 01 00
                    ++first;
                    continue;
                case 0xFF000101:    // 01 01
                    return json::unicode::UNICODE_ENCODING_UTF_8;
                    
                    // reading third byte:    
                case 0x00000000:  // 00 00 00
                case 0x00010000:  // 01 00 00  
                    ++first;
                    continue;                    
                    //case 0x00000001:  // 00 00 01  bogus
                    //case 0x00000100:  // 00 01 00  na
                    //case 0x00000101:  // 00 01 01  na
                case 0x00010001:  // 01 00 01 
                    return json::unicode::UNICODE_ENCODING_UTF_16LE;
                    
                    // reading fourth byte    
                case 0x01000000:
                    return json::unicode::UNICODE_ENCODING_UTF_32LE;
                case 0x00000001:
                    return json::unicode::UNICODE_ENCODING_UTF_32BE;
                    
                default:
                    return -2;  // could not determine encoding, that is,
                                // assuming the first byte is an ASCII.
            } // switch
        }  // while 
            
        // premature EOF
        return -1;
    }
}


namespace json {
    
    using parse_internal::parse_loop;
    using parse_internal::semantic_actions_apply_flags;    
    using unicode::encoding_traits;
    
    //
    //  bool parse(Iterator& first, Iterator last, ParseOptions flags, int* error = NULL)
    //
    // Validates the text in the range [first last).
    
    // Returns true on success, otherwise returns false and if error is not NULL
    // sets the error state to the variable referenced by error.
    // If an error occured, iterator first points to the element where the error
    // has been detected.
    
    template <typename IteratorT>
    inline bool
    parse(IteratorT& first, IteratorT last, ParseOptions flags = ParseOptions::None, int* error = NULL)
    {
        // No source encoding specified, guess a reasonable default source encoding,
        // based from the iterator type and host endianness:
        typedef typename unicode::iterator_encoding<IteratorT>::type  SourceEncoding;
        
        // No semantic actions sepcified, use a validating parser, use default
        // options, choose no-op semantic actions:
        typedef semantic_actions_noop<SourceEncoding>           SemanticActions;
        typedef typename SemanticActions::error_t                    sa_error_t;
        
        // No policies specified, use default json parser policies:
        //typedef parser_policies                                        Policies;
        
        // Create the parser type:
        typedef parser<IteratorT, SourceEncoding, SemanticActions> parser_t;
        
        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, SourceEncoding>::value) );
        BOOST_STATIC_ASSERT( (sizeof(typename boost::iterator_value<IteratorT>::type) 
                              == sizeof(typename encoding_traits<SourceEncoding>::code_unit_type)) );
        
        SemanticActions sa;
        if (flags) 
            semantic_actions_apply_flags(sa, flags);
        
        parser_t parser(sa);        
        bool result = parse_loop(parser, sa, first, last);
        if (!result and error) {
            *error = sa.error().first;
        }
        return result;
    }

    
    //
    //  bool parse(Iterator& first, Iterator last, SemanticActionsT& sa)
    //
    // Parses the text in the range [first last) with a configuration specified
    // by the semantic actions instance.    
    //
    // The result of parsing will be hold within the semantic actions instance, 
    // as well as any possible error state. The actual result of the parse 
    // process depends entirely on the type of the semantic actions class. 
    // Usually, it contains a representation of the JOSN text as a JSON tree.
    //
    // Parameter first is the start of the seqeunce of Unicode code units. The
    // encoding will be "guessed" upon the size of the iterator's value_type:
    // For 8-bit it will be UTF-8, for 16-bit it will be UTF-16 and for 32-bit
    // it will be UTF-32. The endianess is assumed to be host-endianness.
    // 
    // Parameter sa is the semantic actions instance. Before parsing, the parser
    // will configure itself based on properties set in the semantic actions
    // parameter. The actual "semantic actions" will be performe by the semantic
    // actions instance, hence its name. The "semantic actions" may include to 
    // generate the JSON tree and perform error handling. Or it may just validate
    // the JSON input. The semantic actions may also be capable to parse multiple 
    // JSON documents. What it exactly performs depends entirely on the type of
    // the semantic actions instance.
    //
    // Returns true on success, otherwise returns false.
    // If an error occured, iterator first points to the element where the error
    // has been detected, and further error information can be retrieved from
    // the semantic actions instance.
    //
    // Requirements:
    //
    //  IteratorT shall at least fulfill the Input Iterator concept.
    //
    //  SemanticActionsT shall be a subclass of json::semantic_actions_base.
    
    template <typename IteratorT, typename SemanticActionsT>
    inline bool
    parse(IteratorT& first, IteratorT last, SemanticActionsT& sa)
    {
        // No source encoding specified, guess a reasonable default source encoding,
        // based from the iterator type and host endianness:
        typedef typename unicode::iterator_encoding<IteratorT>::type  SourceEncoding;
        typedef typename SemanticActionsT::error_t sa_error_t;
        
        // No policies specified, use default json parser policies:
        //typedef parser_policies                             Policies;
        
        // Finally, create the parser type:
        typedef parser<IteratorT, SourceEncoding, SemanticActionsT> parser_t;
        
        
        //BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::semantic_actions_base, SemanticActionsT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::utf_encoding_tag, SourceEncoding>::value) );
        BOOST_STATIC_ASSERT( (sizeof(typename boost::iterator_value<IteratorT>::type) 
                              == sizeof(typename encoding_traits<SourceEncoding>::code_unit_type)) );
        
        // TODO: make a static assert which checks the semantic actions. For now, 
        // we simply assume the user's semantic actions class inherits from 
        // json::internal::semantic_actions.
        
        parser_t parser(sa); // create the parser
        bool result = parse_loop(parser, sa, first, last);
        return result;
    }

    
    
    //
    //  bool parse(Iterator& first, Iterator last, EncodingT encoding, 
    //             SemanticActionsT& sa)
    //
    // Parses the text in the range [first last) with a configuration specified
    // by the semantic actions instance.
    //
    // The result of parsing will be hold within the semantic actions instance, 
    // as well as any possible error state. The actual result of the parse 
    // process depends entirely on the type of the semantic actions class. 
    // Usually, it contains a representation of the JOSN text as a JSON tree.
    //
    // Parameter first is the start of the seqeunce of Unicode code units. The
    // encoding is specified in parameter encoding.
    // 
    // Parameter sa is the semantic actions instance. Before parsing, the parser
    // will configure itself based on properties set in the semantic actions
    // parameter. The actual "semantic actions" will be performe by the semantic
    // actions instance, hence its name. The "semantic actions" may include to 
    // generate the JSON tree and perform error handling. Or it may just validate
    // the JSON input. The semantic actions may also be capable to parse multiple 
    // JSON documents. What it exactly performs depends entirely on the type of
    // the semantic actions instance.
    //
    // Returns true on success, otherwise returns false.
    // If an error occured, iterator first points to the element where the error
    // has been detected, and further error information can be retrieved from
    // the semantic actions instance.
    //
    // Requirements:
    //
    //  IteratorT shall at least fulfill the Input Iterator concept.
    //
    //  SemanticActionsT shall be a subclass of json::semantic_actions_base.
    
    
    template <typename IteratorT, typename EncodingT,  typename SemanticActionsT>
    inline bool
    parse(IteratorT& first, IteratorT last, EncodingT encoding, 
          SemanticActionsT& sa)
    {
        typedef typename SemanticActionsT::error_t sa_error_t;
        
        // No policies specified, use default json parser policies:
        //typedef parser_policies                             Policies;
        
        // Finally, create the parser type:
        typedef parser<IteratorT, EncodingT, SemanticActionsT> parser_t;
        
        // TODO: make a static assert which checks the semantic actions. For now, 
        // we simply assume the user's semantic actions class inherits from 
        // json::internal::semantic_actions.
        
        parser_t parser(sa); // create the parser
        bool result = parse_loop(parser, sa, first, last);
        return result;
    }
    
    
    
}


#endif // JSON_PARSE_HPP

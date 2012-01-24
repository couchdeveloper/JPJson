//
//  semantic_actions_base.hpp
//
//
//  Created by Andreas Grosam on 6/16/11.
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

#ifndef JSON_SEMANTIC_ACTIONS_BASE_HPP
#define JSON_SEMANTIC_ACTIONS_BASE_HPP


#include "json/config.hpp"
#include <string>
#include <iostream>
#include <boost/config.hpp>
#include <boost/functional/hash/hash.hpp>

#include "parser_errors.hpp"
#include "json/utility/number_builder.hpp"
#include "json/utility/simple_log.hpp"
#include "json/utility/flags.hpp"

using json::utility::logger;
#if defined (DEBUG)
#define LOG_MAX_LEVEL json::utility::log_debug 
#else
#define LOG_MAX_LEVEL json::utility::log_warning 
#endif    


namespace json { namespace semanticactions {
    
    enum log_option {
        LogLevelDebug,    
        LogLevelWarning,  
        LogLevelError,    
        LogLevelNone    
    };
    
    
    // Parser configuration
    enum noncharacter_handling_option {
        SignalErrorOnUnicodeNoncharacter,
        SubstituteUnicodeNoncharacter,
        SkipUnicodeNoncharacters,
    };
    
    // (Not yet implemented)
    // Extensions
    // Note: setting any of these flags will break JSON conformance.
    
    
}}

namespace json { namespace semanticactions {
    
    struct extension_options {
        enum E {
            AllowNone = 0,
            AllowComments                   = 1UL << 0,              
            AllowControlCharacters          = 1UL << 1,     
            AllowLeadingPlusInNumbers       = 1UL << 2,
            AllowLeadingZerosInIntegers     = 1UL << 3
        };
        typedef E enum_type;
    };
    typedef json::utility::flags<extension_options> ExtensionOptions; 

}}
UTILITY_DEFINE_FLAG_OPERATORS(json::semanticactions::extension_options);  // shall be defined in global namespace


namespace json { 
    
    using namespace semanticactions;
    
    // Requires:
    // Copy Constructable, Assignable
    
    template <typename DerivedT, typename StringBufferEncodingT>
    class semantic_actions_base
    {

        struct error_info : std::pair<int, std::string> 
        {
            typedef std::pair<int, std::string> base;
            
            error_info() : base(0, std::string()) {}
            
            explicit error_info(int code, const char* description)  
            : base(code,description) 
            {}
            
            
            int code() const { return this->first; }
            
            const char* c_str() const { return this->second.c_str(); }
            std::string description() const { return this->second; }
            
            void set(int code, const char* description) {
                this->first = code;
                this->second = description;
            }
            void reset() { 
                this->first = 0;
                this->second.clear();
            }

        };
        
    public:    
        typedef error_info                                  error_t;
        typedef typename StringBufferEncodingT::code_unit_type   char_t;
        typedef StringBufferEncodingT                       encoding_t;  // specifies the string buffer encoding
        
        typedef json::utility::logger<LOG_MAX_LEVEL>        logger_t;
        typedef typename json::numberbuilder::number_t      nb_number_t;
        
        semantic_actions_base()    
        :   nch_option_(SignalErrorOnUnicodeNoncharacter),
            ncon_flags_(ExtensionOptions::AllowNone),
            opt_ignore_spurious_trailing_bytes_(false),
            opt_multiple_documents_(false),
            opt_check_duplicate_key_(true),
            canceled_(false)
        {
        }

        
        DerivedT& derived()             { return static_cast<DerivedT&>(*this); }
        const DerivedT& derived() const { return static_cast<const DerivedT&>(*this); }
        
        void parse_begin()                              { this->derived().parse_begin_imp(); }
        void parse_end()                                { this->derived().parse_end_imp(); }
        void finished()                                 { this->derived().finished_imp(); }
        
        void begin_array()                              { this->derived().begin_array_imp(); }
        void end_array()                                { this->derived().end_array_imp(); }
        
        void begin_object()                             { this->derived().begin_object_imp(); }
        bool end_object()                               { return this->derived().end_object_imp(); }

        void begin_value_at_index(size_t index)         { this->derived().begin_value_at_index_imp(index); }
        void end_value_at_index(size_t index)           { this->derived().end_value_at_index_imp(index); }
        
        void begin_value_with_key(const char_t* s, size_t len, size_t nth) { this->derived().begin_value_with_key_imp(s, len, nth); }
        void end_value_with_key(const char_t* s, size_t len, size_t nth) { this->derived().end_value_with_key_imp(s, len, nth); }
        
        
        void value_string(const char_t* s, std::size_t len) { this->derived().value_string_imp(s, len); }
        void value_number(const nb_number_t& number)     { this->derived().value_number_imp(number); }
        void value_null()                                { this->derived().value_null_imp(); }
        void value_boolean(bool b)                       { this->derived().value_boolean_imp(b); }
        
        
        void clear()                                    { canceled_ = false; inputEncoding_.clear(); this->derived().clear_imp(); }

        void print(std::ostream& os)                    { this->derived().print_imp(os); }
        
        void error(json::parser_error_type code, const char* description)  
        {
            if (canceled_) {
                this->derived().error_imp(static_cast<int>(JP_CANCELED), parser_error_str(JP_CANCELED));
            } else {
                this->derived().error_imp(static_cast<int>(code), description);
            }
            logger_.log(json::utility::LOG_ERROR, "ERROR: json::parser (%d): %s", code, description);
        }
        
        void error(int code, const char* description)  
        {
            assert(code <= 0 or code >= static_cast<int>(json::JP_PARSER_CLIENT));
            if (canceled_) {
                this->derived().error_imp(static_cast<int>(JP_CANCELED), parser_error_str(JP_CANCELED));
            } else {
                this->derived().error_imp(code, description);
            }
            logger_.log(json::utility::LOG_ERROR, "ERROR: json::parser (%d): %s", code, description);
        }
        
        
        const error_t& error() const { 
            return this->derived().error_imp(); 
        }
        
        
        bool    ignoreSpuriousTrailingBytes() const     { return opt_ignore_spurious_trailing_bytes_; }
        void    ignoreSpuriousTrailingBytes(bool set)   { opt_ignore_spurious_trailing_bytes_ = set; }
        
        bool    parseMultipleDocuments() const          { return  opt_multiple_documents_; }
        void    parseMultipleDocuments(bool set)        { opt_multiple_documents_ = set; }
        
        bool    checkDuplicateKey() const               { return opt_check_duplicate_key_; }
        void    checkDuplicateKey(bool set)             { opt_check_duplicate_key_ = set; }
                
        
        
        
        // Log behavior
        void   log_level(log_option option) {
            switch (option) {
                case LogLevelDebug:
                    logger_.log_level(json::utility::LOG_DEBUG);
                    break;
                case LogLevelWarning:
                    logger_.log_level(json::utility::LOG_WARNING);
                    break;
                case LogLevelError:
                    logger_.log_level(json::utility::LOG_ERROR);
                    break;
                case LogLevelNone:
                    logger_.log_level(json::utility::LOG_NONE);
                    break;
            }
        }
        
        log_option   log_level() const {
            switch (logger_.log_level()) {
                case json::utility::LOG_LEVEL_TRACE:
                case json::utility::LOG_LEVEL_DEBUG:
                    return LogLevelDebug;
                case json::utility::LOG_LEVEL_INFO:
                case json::utility::LOG_LEVEL_WARNING:
                    return LogLevelWarning;
                case json::utility::LOG_LEVEL_ERROR:
                case json::utility::LOG_LEVEL_FATAL:
                    return LogLevelError;
                case json::utility::LOG_LEVEL_NONE:
                    return LogLevelNone;
            }
        }
        
        logger_t& logger() { return logger_; }
        
        
        
        noncharacter_handling_option unicode_noncharacter_handling() const {
            return nch_option_;
        }
        
        void unicode_noncharacter_handling(noncharacter_handling_option opt) {
            nch_option_ = opt;
        }
        
        ExtensionOptions extensions() const { return ncon_flags_; }
        void extensions(ExtensionOptions flags) {
            ncon_flags_ = flags;
        }
        
        void cancel() {
            canceled_ = true;
        }
        bool is_canceled() const {
            return canceled_;
        }
        
        // Returns true, if the semantic actions state is valid, aka, there is no error,
        // and it is not canceled, etc.
       bool ok() const { return !this->is_canceled() and this->error().code() == 0; }
        
        
        std::string internalStringBufferEncoding() const {
            return StringBufferEncodingT::name();
        }
        
        std::string inputEncoding() const {
            return inputEncoding_;
        }
        
        void inputEncoding(const std::string& encoding) {
            inputEncoding_ = encoding;
        }
        
        void* result() const { return static_cast<void*>(0); }
        
    protected:
        logger_t                logger_;
        noncharacter_handling_option nch_option_;
        ExtensionOptions        ncon_flags_;
        std::string             inputEncoding_;
        bool                    opt_ignore_spurious_trailing_bytes_;
        bool                    opt_multiple_documents_;
        bool                    opt_check_duplicate_key_;
        bool                    canceled_;
        
        
    private:
        
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const semantic_actions_base& sa) 
        {
            os << "Input encoding: " << sa.inputEncoding() << "\n"

            << "Unicode noncharacter handling: " << 
                ((sa.nch_option_==SignalErrorOnUnicodeNoncharacter) ? "SignalErrorOnUnicodeNoncharacter"
                : ((sa.nch_option_==SubstituteUnicodeNoncharacter) ? "SubstituteUnicodeNoncharacter" : "SkipUnicodeNoncharacters"))
            << "\n"
            << "\nBasic Json Parser Options:"
            << "\n\tCheck For Duplicate Keys: " << (sa.opt_check_duplicate_key_ ? "YES" : "NO")
            << "\n\tParse Multiple Documents: " << (sa.opt_multiple_documents_ ? "YES" : "NO")
            << "\n\tCheck For Spurious Trailing Bytes: " << (sa.opt_ignore_spurious_trailing_bytes_ ? "YES" : "NO")
            << "\n\nExtensions"
            << "\n\tAllow Comments: " << ((sa.ncon_flags_ & semanticactions::extension_options::AllowComments) ? "YES" : "NO")
            << "\n\tAllow Control Characters: " << ((sa.ncon_flags_ & semanticactions::extension_options::AllowControlCharacters) ? "YES" : "NO")
            << "\n\tAllow Leading Plus In Numbers: " << ((sa.ncon_flags_ & semanticactions::extension_options::AllowLeadingPlusInNumbers) ? "YES" : "NO")
            << "\n\tAllow Leading Zeros In Integers: " << ((sa.ncon_flags_ & semanticactions::extension_options::AllowLeadingZerosInIntegers) ? "YES" : "NO")
            
            << "\n\nLog Level: " << (sa.log_level()==semanticactions::LogLevelDebug?"LogLevelDebug"
                                 :sa.log_level()==semanticactions::LogLevelWarning?"LogLevelWarning"
                                 :sa.log_level()==semanticactions::LogLevelError?"LogLevelError"
                                 :"LogLevelNone")
            << std::endl;
            
            if (sa.is_canceled()) {
                os << "State: canceled" << std::endl;
            }
            if (sa.error().code() != 0) {
                os << "Error: code: " << sa.error().code() << ", description: " << sa.error().c_str() << std::endl;
            }
            
            return os;
        }
        
    };
    
} // namespace json


namespace json { 
#pragma mark -
#pragma mark semantic_actions_noop
    //
    //  Test only
    //  
    // Template parameter EncodingT shall be derived from json::utf_encoding_tag.
    // EncodingT shall match the StringBufferEncoding of the parser.
    //    
    template <typename EncodingT>
    class semantic_actions_noop : 
            public semantic_actions_base<semantic_actions_noop<EncodingT>, EncodingT>
    {
    private:   
        typedef semantic_actions_base<semantic_actions_noop, EncodingT> base;
        
        
    public:    
        typedef typename base::error_t                  error_t;
        typedef typename base::char_t                   char_t;
        typedef typename base::encoding_t               encoding_t;
        typedef void                                    result_type;
        typedef typename base::nb_number_t              nb_number_t;
        
    public:    
        semantic_actions_noop(){}
        bool additionalInputIsError_imp() const             { return true; }
        void parse_begin_imp()                              {}
        void parse_end_imp()                                {}
        void finished_imp()                                 {}
        void begin_array_imp()                              {}
        void end_array_imp()                                {}
        void begin_object_imp()                             {}
        bool end_object_imp()                               { return true; }
        void begin_value_at_index_imp(size_t index) {}
        void end_value_at_index_imp(size_t index) {}        
        void begin_value_with_key_imp(const char_t*, size_t len, size_t) {}
        void end_value_with_key_imp(const char_t*, size_t len, size_t) {}
        void value_string_imp(const char_t*, std::size_t)    {}        
        void value_number_imp(const nb_number_t& number)     {}
        void value_boolean_imp(bool b)                       {}
        void value_null_imp()                                {}
        
                
        void clear_imp() 
        {} 
        
        void error_imp(int code, const char* description) {
            error_.set(code, description);
        }
        void error_imp(const error_t& error) {
            error_ = error;
        }        
        const error_t& error_imp() const {
            return error_;
        }
    
        
    private:
        error_t error_ ;   
    };
    
    
    
} // namespace json





#endif // JSON_SEMANTIC_ACTIONS_BASE_HPP

//
//  parser_errors.hpp
//  
//
//  Created by Andreas Grosam on 8/25/11.
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

#ifndef JSON_PARSER_ERRORS_HPP
#define JSON_PARSER_ERRORS_HPP


namespace json {

    // parse errors
    enum parser_error_type {
        JP_NO_ERROR = 0,                        // "no error"
        JP_INTERNAL_LOGIC_ERROR,                // "internal logic error"
        JP_INTERNAL_RUNTIME_ERROR,              // "internal runtime error"
        JP_CANCELED,                            // "operation canceled"
        JP_ENCODING_NOT_SUPPORTED_ERROR,        // "encoding not supported"  - internal logic error
        
        // Syntax errors
        JP_SYNTAX_ERROR,                        // "syntax error" - general error
        JP_EMPTY_TEXT_ERROR,                    // "text is empty"
        JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,      // "control character not allowed in json string"
        JP_UNEXPECTED_END_ERROR,                // "unexpected end of text" (p_ == last_)
        JP_UNICODE_NULL_NOT_ALLOWED_ERROR,      // "encountered U+0000" (An Unicode NULL is not a valid character in this implementation)
        JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,      // "expected array or object"
        JP_EXPECTED_TOKEN_OBJECT_END_ERROR,     // "expected end-of-object '}'"
        JP_EXPECTED_TOKEN_ARRAY_END_ERROR,      // "expected end-of-array ']'"
        JP_EXPECTED_TOKEN_KEY_VALUE_SEP_ERROR,  // "expected key-value-separator ':'"
        JP_INVALID_HEX_VALUE_ERROR,             // "invalid hexadecimal number"
        JP_INVALID_ESCAPE_SEQ_ERROR,            // "invalid escape sequence"
        JP_BADNUMBER_ERROR,                     // "bad number"
        JP_EXPECTED_STRING_ERROR,               // "expected string"
        JP_EXPECTED_NUMBER_ERROR,               // "expected number"
        JP_EXPECTED_VALUE_ERROR,                // "expected value"
        
        // ill-formed Unicode sequence
        JP_INVALID_UNICODE_ERROR,               // "invalid unicode code point" - general error
        JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    // "illformed Unicode sequence" - general error
        JP_EXPECTED_HIGH_SURROGATE_ERROR,       // "expected high surrogate code point"  - illformed Unicode sequence 
        JP_EXPECTED_LOW_SURROGATE_ERROR,        // "expected low surrogate code point" - illformed Unicode sequence
        
        
        // Invalid Unicode code points in JSON text
        JP_UNICODE_NONCHARACTER_ERROR,          // "encountered unicode noncharacter" / Encountered an "Unicode noncharacter" in input - this value is not interchangable.
        JP_UNICODE_REJECTED_BY_FILTER,          // "Unicode code point rejected by filter"
        
        // JSON container errors
        JP_JSON_KEY_EXISTS_ERROR,               // "key exists" - JSON logic error
        
        // Other errors
        JP_JSON_EXTRA_CHARACTERS_AT_END = 100, // "extra characters at end of json document not allowed"
        JP_OUT_OF_BOUND_UNICODE_NULL =    101, // "encountered out-of-bound U+0000 character(s)"

        JP_PARSER_CLIENT =               1000, // client errors start here
        JP_UNEXPECTED_ERROR =            1001, // exceptions cought by the outer levels of clients, description shall be provided by the clients
        
        JP_UNKNOWN_ERROR,                      // "unknown error", usually an unspecified exception caught by outer levels of clients
        JP_ERROR_MAX =                  10000  // user errors start here
    };

    
    inline const char* parser_error_str(parser_error_type error) 
    {
        switch (error) {
            case JP_NO_ERROR:                            return "no error"; break;
            case JP_INTERNAL_LOGIC_ERROR:                return "internal logic error"; break;
            case JP_INTERNAL_RUNTIME_ERROR:              return "internal runtime error"; break;
            case JP_CANCELED:                            return "operation canceled"; break;
            case JP_ENCODING_NOT_SUPPORTED_ERROR:        return "encoding not supported"; break; //  - internal logic error
                
                // Syntax errors
            case JP_SYNTAX_ERROR:                        return "syntax error"; break; // - general error
            case JP_EMPTY_TEXT_ERROR:                    return "text is empty"; break;
            case JP_CONTROL_CHAR_NOT_ALLOWED_ERROR:      return "control character not allowed in json string"; break;
            case JP_UNEXPECTED_END_ERROR:                return "unexpected end of text"; break; // (p_ == last_)
            case JP_UNICODE_NULL_NOT_ALLOWED_ERROR:      return "encountered U+0000"; break; // (An Unicode NULL is not a valid character in this implementation)
            case JP_EXPECTED_ARRAY_OR_OBJECT_ERROR:      return "expected array or object"; break;    
            case JP_EXPECTED_TOKEN_OBJECT_END_ERROR:     return "expected end-of-object '}'"; break;
            case JP_EXPECTED_TOKEN_ARRAY_END_ERROR:      return "expected end-of-array ']'"; break;
            case JP_EXPECTED_TOKEN_KEY_VALUE_SEP_ERROR:  return "expected key-value-separator ':'"; break;
            case JP_INVALID_HEX_VALUE_ERROR:             return "invalid hexadecimal number"; break;
            case JP_INVALID_ESCAPE_SEQ_ERROR:            return "invalid escape sequence"; break;
            case JP_BADNUMBER_ERROR:                     return "bad number"; break;
            case JP_EXPECTED_STRING_ERROR:               return "expected string"; break;
            case JP_EXPECTED_NUMBER_ERROR:               return "expected number"; break;
            case JP_EXPECTED_VALUE_ERROR:                return "expected value"; break;
                
                // ill-formed Unicode sequence
            case JP_INVALID_UNICODE_ERROR:               return "invalid Unicode code point"; break; // - general error
            case JP_ILLFORMED_UNICODE_SEQUENCE_ERROR:    return "illformed Unicode sequence"; break; // - general error
            case JP_EXPECTED_HIGH_SURROGATE_ERROR:       return "expected high surrogate code point"; break; //  - illformed Unicode sequence 
            case JP_EXPECTED_LOW_SURROGATE_ERROR:        return "expected low surrogate code point"; break; // - illformed Unicode sequence
                
                
                // Invalid Unicode code points in JSON text
            case JP_UNICODE_NONCHARACTER_ERROR:          return "encountered Unicode noncharacter"; break; // Encountered an "Unicode noncharacter" in input - this value is not interchangable.
            case JP_UNICODE_REJECTED_BY_FILTER:          return "Unicode code point rejected by filter"; break;
                
                // JSON container errors
            case JP_JSON_KEY_EXISTS_ERROR:               return "key exists"; break; // - JSON logic error
                
                // Other errors (signaled by parse_loop and other parse functions)
            case JP_JSON_EXTRA_CHARACTERS_AT_END:       return "extra characters at end of json document not allowed"; break;
            case JP_OUT_OF_BOUND_UNICODE_NULL:          return "encountered out-of-bound U+0000 character(s)"; break;
                
            case JP_UNKNOWN_ERROR:                       return "unknown error"; break;
                
            default: return "no description available";
        }
    }

}  // namespace json
    
    
#endif  // JSON_PARSER_ERRORS_HPP

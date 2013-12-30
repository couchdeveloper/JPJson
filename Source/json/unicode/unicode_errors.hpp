//
//  unicode_errors.hpp
//  
//
//  Created by Andreas Grosam on 1/27/12.
//  Copyright 2013 Andreas Grosam
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

#ifndef JSON_UNICODE_ERRORS_HPP
#define JSON_UNICODE_ERRORS_HPP

#pragma mark - Error Codes
namespace json { namespace unicode { 
    
    enum ErrorT {
        NO_ERROR = 0,
        E_INVALID_START_BYTE =      -1,  /* Invalid start byte */  // ill-formed
        E_TRAIL_EXPECTED =          -2,  /* Trail byte expected. */ // ill-formed
        E_UNCONVERTABLE_OFFSET =    -3,  /* Unconvertable offset */  // ill-formed (conversion would yield invalid code point)
        E_INVALID_CODE_POINT =      -4,  /* An Unicode code point is invalid, e.g. not in range [0 .. 10FFFF] */
        E_INVALID_UNICODE =         -5,  /* An Unicode code is not a valid Unicode scalar value */
        E_NO_CHARACTER =            -6,  /* An Unicode code point is not an Unicode scalar value or it is a noncharacter. */
        E_UNEXPECTED_ENDOFINPUT = -100,  /* Unexpected and of input. */
        PARSE_ACTION_BREAK  =     -200,  /* A Parse Action forced the parser to return immediately */
        E_UNKNOWN_ERROR =        -1000
    };
    
}}    




#endif // JSON_UNICODE_ERRORS_HPP

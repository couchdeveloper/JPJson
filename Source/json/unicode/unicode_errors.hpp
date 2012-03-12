//
//  unicode_errors.hpp
//  
//
//  Created by Andreas Grosam on 1/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _unicode_errors_hpp
#define _unicode_errors_hpp

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




#endif

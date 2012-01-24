//
//  unicode_detect_encoding.hpp
//  
//
//  Created by Andreas Grosam on 5/18/11.
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

#ifndef JSON_UNICODE_DETECT_ENCODING_HPP
#define JSON_UNICODE_DETECT_ENCODING_HPP


#include "json/config.hpp"
#include <stdint.h>
#include <iterator>
#include "unicode_utilities.hpp"


namespace json { namespace unicode {


    enum UnicodeBOM {
        UNICODE_BOM_UTF_8       =  UNICODE_ENCODING_UTF_8,
        UNICODE_BOM_UTF_16BE    =  UNICODE_ENCODING_UTF_16BE,
        UNICODE_BOM_UTF_16LE    =  UNICODE_ENCODING_UTF_16LE,
        UNICODE_BOM_UTF_32BE    =  UNICODE_ENCODING_UTF_32BE,
        UNICODE_BOM_UTF_32LE    =  UNICODE_ENCODING_UTF_32LE
    };

    // This helps in
    // figuring the enocding form of an arbitrary byte stream. However, if no
    // BOM is provided, the encoding must be "guessed" using domain specific 
    // heuristics.
    //
    
    
    // Synopsis
    // int detect_bom(Iterator& first, iterator last) 
    // 
    //     
    // Iterators shall support the InputIterator concept. 
    // 
    // Tries to find a BOM at the start of the input sequence. 
    //
    // Result:
    //
    // If the return value is greater than zero, the input stream contains a
    // BOM and the value corresponds to one of the Unicode encoding schemes.
    // If the result equals zero, no BOM could be detected.
    // Otherwise, if the result is negative it indicates an error.
    //
    // Upon return first has been advanced so that it points past the end of
    // the BOM, respectively points to the first character or EOF.
    //
    // Results:
    //     >0: success, returns a values of UnicodeBOM
    //     -1: unexpected EOF
    //
    // The size of the iterator's value_type shall be 1.

    template <typename Iterator>
    int detect_bom(Iterator& first, Iterator last) 
    {        
        //  Check if there is a BOM.
        //
        //   00 00 FE FF	UTF-32, big-endian
        //   FF FE 00 00	UTF-32, little-endian
        //   FE FF          UTF-16, big-endian
        //   FF FE          UTF-16, little-endian
        //   EF BB BF       UTF-8
        
        uint32_t c = 0xFFFFFF00;
        while (first != last) {
            c = c << 8 | static_cast<uint8_t>(*first);
            switch (c) {
                // reading data[0]
                case 0xFFFF0000:
                case 0xFFFF00FF:
                case 0xFFFF00FE:
                case 0xFFFF00EF:
                    ++first;
                    continue;
                    
                // reading data[1]
                case 0xFF000000:
                case 0xFF00FFFE:  // possibly UTF-16-LE .. but not yet unambiguous
                case 0xFF00EFBB:
                    ++first;
                    continue;
                case 0xFF00FEFF:
                    ++first;
                    return UNICODE_BOM_UTF_16BE;
                    
                // reading data[2]    
                case 0x000000FE:
                case 0x00FFFE00:  // possibly UTF-32, little-endian - but we need read data[2] to be sure
                    ++first;
                    continue;
                case 0x00EFBBBF: 
                    ++first;
                    return UNICODE_BOM_UTF_8;
                    
                // reading data[3]    
                case 0x0000FEFF:
                    ++first;
                    return UNICODE_BOM_UTF_32BE;
                case 0xFFFE0000:
                    ++first;
                    return UNICODE_BOM_UTF_32LE;
                    
                default:
                    // (reading data[2])
                    // possibly UTF-16-LE ?
                    if ( (c >> 8) == 0x0000FFFE )  {
                        // do not increment first here, it already points to the next
                        return UNICODE_BOM_UTF_16LE;
                    }
                    return 0;  // no BOM
            } // switch
        }  // while 
        
        // We hit EOF
        // We might got a valid UTF-16-LE BOM - but a zero length text. So, check
        // this too:
        if (c == 0xFF00FFFE) {
            return UNICODE_BOM_UTF_16LE; // still ambiguous since we got EOF
        }
        
        return -1;  // unexpected EOF
    }

    
}}


#endif

//
//  NSData+JPJsonDetectEncoding.mm
//  
//
//  Created by Andreas Grosam on 7/20/11.
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

#if __has_feature(objc_arc) 
#error This Objective-C file shall be compiled with ARC disabled.
#endif

#include "json/unicode/unicode_detect_bom.hpp"
#include "json/parser/parse.hpp"
#include <algorithm>
#include <CoreFoundation/CFStringEncodingExt.h>
#import "NSData+JPJsonDetectEncoding.h"


namespace {
    
    
    // Errors:
    // -1:      unexpected EOF
    // -2:      unknown encoding, possibly malformed Unicode
    int detectUnicodeEncoding(const char* first, const char* last)
    {
        // Check if there is a BOM using detect_bom() utility function. Possible 
        // results:
        // positive values in case of success:
        //    json::unicode::UNICODE_BOM_UTF_8    
        //    json::unicode::UNICODE_BOM_UTF_16BE 
        //    json::unicode::UNICODE_BOM_UTF_16LE 
        //    json::unicode::UNICODE_BOM_UTF_32BE 
        //    json::unicode::UNICODE_BOM_UTF_32LE 
        //  zero, if no BOM
        //  and negative values in case of an error:
        //   -1: unexpected EOF
        
        const char* saved_first = first;
        int bom_result = json::unicode::detect_bom(first, last);
        if (bom_result < 0) {
            return bom_result;
        } else if (bom_result == 0) {
            // We need to reset the iterator
            first = saved_first;
        }        
        // Now, the iterator first should point to the first code unit.        
        int encoding = -1;
        if (bom_result > 0) {
            encoding = bom_result;
        }
        else
        {
            // There is no BOM.
            // Call the detect_encoding() utility function.
            // Return values:
            //   json::unicode::UNICODE_ENCODING_UTF_8
            //   json::unicode::UNICODE_ENCODING_UTF_16LE
            //   json::unicode::UNICODE_ENCODING_UTF_16BE
            //   json::unicode::UNICODE_ENCODING_UTF_32LE
            //   json::unicode::UNICODE_ENCODING_UTF_32BE
            //   -1:     unexpected EOF
            //   -2:     unknown encoding
            encoding = json::detect_encoding(first, last);
            if (encoding <= 0) {
                return encoding;
            }
        }
        assert(encoding > 0);
        
        return encoding;
    }
    
    
    
} // namespace



@implementation NSData (JPJsonDetectEncoding)

-(NSStringEncoding) jpj_detectUnicodeNSStringEncoding 
{
    const char* first = static_cast<const char*>([self bytes]);
    const char* last = first + std::min(size_t(4), static_cast<size_t>([self length]));
    int encoding = detectUnicodeEncoding(first, last);
    switch (encoding) {
        case json::unicode::UNICODE_ENCODING_UTF_8:
            return NSUTF8StringEncoding; 
            break;
        case json::unicode::UNICODE_ENCODING_UTF_16LE:
            return NSUTF16LittleEndianStringEncoding;
            break;
        case json::unicode::UNICODE_ENCODING_UTF_16BE:
            return NSUTF16BigEndianStringEncoding;
            break;
        case json::unicode::UNICODE_ENCODING_UTF_32LE:
            return NSUTF32LittleEndianStringEncoding;
            break;
        case json::unicode::UNICODE_ENCODING_UTF_32BE:
            return NSUTF32BigEndianStringEncoding;
            break;

        default:
            return -1;  // Invalid Encoding             
    }
}


@end


// Workaround for linker bug:
@interface DummyNSDataJPJsonDetectEncoding @end
@implementation DummyNSDataJPJsonDetectEncoding @end
//
//  json_parse.h
//
//  Created by Andreas Grosam on 6/17/11.
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

#ifndef JSON_OBJC_JSON_PARSE_H
#define JSON_OBJC_JSON_PARSE_H


#warning deprecated file

#import <Foundation/Foundation.h>


@class JPNSDataBuffers;
@class JPSemanticActions;


#ifdef __cplusplus
extern "C" {
#endif    
    
    
    enum json_parse_options {
        
        None                                    = 0,
        
        // Noncharacter Handling (mutual exclusive)
        SignalErrorOnNoncharacter               = 1UL << 0,
        ReplaceNoncharacters                    = 1UL << 1,
        SkipNoncharacters                       = 1UL << 2,
        
        // Log Levlel (mutual exclusive)
        LogLevelDebug                           = 1UL << 3,
        LogLevelWarning                         = 1UL << 4,
        LogLevelError                           = 1UL << 5,
        LogLevelNone                            = 1UL << 6,
        
        // Numbergenerator Options (mutual exclusive)
        NumberGeneratorGenerateDecimalNumber    = 1UL << 7,
        NumberGeneratorGenerateNumber           = 1UL << 8,
        NumberGeneratorGenerateString           = 1UL << 9,
                
        
        // non_conformance_flags (can be ored)
        // not yet implemented!
        AllowComments                           = 1UL << 10,              
        AllowControlCharacters                  = 1UL << 11,     
        AllowLeadingPlusInNumbers               = 1UL << 12,
        AllowLeadingZerosInIntegers             = 1UL << 13,
        
        
        NoncharacterHandling = SignalErrorOnNoncharacter | ReplaceNoncharacters | SkipNoncharacters,
        LogLevel = LogLevelDebug | LogLevelWarning | LogLevelError |  LogLevelNone,
        NumbergeneratorOptions = NumberGeneratorGenerateDecimalNumber | NumberGeneratorGenerateNumber | NumberGeneratorGenerateString,
        NonConformanceFlags = AllowComments | AllowControlCharacters | AllowLeadingPlusInNumbers | AllowLeadingZerosInIntegers
        
    };
    
    
    // int json_parse_detect_bom(uint8_t const** first_ptr, uint8_t const* last);
    
    //bool json_parse_data(const void* data, size_t len, CFStringEncoding encoding, JPSemanticActions* sa); 
    
    bool json_parse_UTF8(const char*, size_t len, JPSemanticActions* sa);
    
    bool json_parse_string(NSString* jsonString, JPSemanticActions* sa);

    bool json_validate_string(NSString* jsonString, NSError** error);

    bool json_parse_data(void const* first, void const* last, JPSemanticActions* sa);
    bool json_parse_NSData(NSData* data, JPSemanticActions* sa);
    
    
    
    
#ifdef __cplusplus
}
#endif
    
#endif // JSON_OBJECTIVEC_JSON_PARSE_H
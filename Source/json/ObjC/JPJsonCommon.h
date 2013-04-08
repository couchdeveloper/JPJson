//
//  JPJsonCommon.h
//  
//
//  Created by Andreas Grosam on 8/24/11.
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

#ifndef JSON_OBJC_JP_JPJSON_H
#define JSON_OBJC_JP_JPJSON_H


#include <stdint.h>

/// JPUnicodeEncoding 
/// Defines constants for an Unicode encoding scheme

enum {
    JPUnicodeEncoding_Unknown   = 0,
    JPUnicodeEncoding_UTF8      = 1,
    JPUnicodeEncoding_UTF16     = 2,  // (platform endianness)
    JPUnicodeEncoding_UTF16BE   = 3,
    JPUnicodeEncoding_UTF16LE   = 4,
    JPUnicodeEncoding_UTF32     = 5,  // (platform endianness)
    JPUnicodeEncoding_UTF32BE   = 6,
    JPUnicodeEncoding_UTF32LE   = 7
};
typedef uint32_t JPUnicodeEncoding;



// The options available for the "default semantic actions" class:
// JPRepresentationGenerator

enum  {
    
    JPJsonParserIgnoreSpuriousTrailingBytes                 = 1UL << 0,
    JPJsonParserParseMultipleDocuments                      = 1UL << 1,
    JPJsonParserParseMultipleDocumentsAsynchronously        = 1UL << 2,
    
    // Handling of Unicode noncharacters 
    // (Mutual exclusive flags)
    JPJsonParserSignalErrorOnNoncharacter                   = 1UL << 3,
    JPJsonParserSubstituteUnicodeNoncharacter               = 1UL << 4,
    JPJsonParserSkipUnicodeNoncharacter                     = 1UL << 5,
    
    // Log behavior
    // (Mutual exclusive flags)
    JPJsonParserLogLevelDebug                               = 1UL << 6,
    JPJsonParserLogLevelWarning                             = 1UL << 7,
    JPJsonParserLogLevelError                               = 1UL << 8,
    JPJsonParserLogLevelNone                                = 1UL << 9,
    
    // non_conformance_flags (can be ored)
    // (not yet implemented!)
    JPJsonParserAllowComments                               = 1UL << 10,              
    JPJsonParserAllowControlCharacters                      = 1UL << 11,     
    JPJsonParserAllowLeadingPlusInNumbers                   = 1UL << 12,
    JPJsonParserAllowLeadingZerosInIntegers                 = 1UL << 13,
    
    
    
    JPJsonParserCheckForDuplicateKey                        = 1UL << 16,
    JPJsonParserKeepStringCacheOnClear                      = 1UL << 17,
    JPJsonParserCacheDataStrings                            = 1UL << 18,
    JPJsonParserCreateMutableContainers                     = 1UL << 19,
    
    // Number generator options
    // (mutual exclusive flags)
    JPJsonParserNumberGeneratorGenerateAuto                 = 1UL << 20,
    JPJsonParserNumberGeneratorGenerateStrings              = 1UL << 21,
    JPJsonParserGeneratorGenerateDecimals                   = 1UL << 22,
    
    
    
    JPJsonParserNoncharacterHandling = 
    JPJsonParserSignalErrorOnNoncharacter 
    | JPJsonParserSubstituteUnicodeNoncharacter 
    | JPJsonParserSkipUnicodeNoncharacter,
    
    JPJsonParserLogLevel = 
    JPJsonParserLogLevelDebug | 
    JPJsonParserLogLevelWarning | 
    JPJsonParserLogLevelError |  
    JPJsonParserLogLevelNone,
    
    JPJsonParserNonConformanceFlags = 
    JPJsonParserAllowComments | 
    JPJsonParserAllowControlCharacters | 
    JPJsonParserAllowLeadingPlusInNumbers | 
    JPJsonParserAllowLeadingZerosInIntegers,
    
    JPJsonParserNumberGeneratorOptions = 
    JPJsonParserNumberGeneratorGenerateAuto |
    JPJsonParserNumberGeneratorGenerateStrings |
    JPJsonParserGeneratorGenerateDecimals
};
typedef uint32_t JPJsonParserOptions;



//  JPJsonParserIgnoreSpuriousTrailingBytes
//
//  If this options is set, the parser will ignore any additional characters 
//  that occur after the last significant character of a valid JSON document
//  (namely, either a "}" or a "]").
//  Otherwise, if the parser encounters code units which can not be interpreted 
//  as white-space Unicode characters it will issue an error and Unicode NULL (U+0000) 
//  or `EOF` will issue a warning to the console.


//  JPJsonParserParseMultipleDocuments
//
//  If enabled, the parser parses one or more documents from the input until it 
//  receives `EOF`. Otherwise, the parser treats any non white spaces after the
//  first JSON document as an error.
//
//  This option is ignored for synchronous parsers.
//


// JPJsonParserParseMultipleDocumentsAsynchronously
//
//  If this option is set, the parser invokes the `jsonObjectHandlerBlock`
//  asynchronously and immediately processes the next JSON document within the 
//  input data. The JSON container is then retained in the dispatch queue till
//  it is processed by the client. This may tie up a lot of system resources if 
//  the client processes frees the JSON containers slowly.
//  If the flag is `NO`, the parser's thread is blocked until the handler routine 
//  returns.
//
//  It is recommended to leave this flag disabled in systems where system resources 
//  are scarce or if the data input possibly contains many and large JSON documents. 
//  When downloading large data, this helps throttling the consumption of system
//  resources by the underlaying network layer.
 


// JPJsonParserCheckForDuplicateKey     
//
//  Checks whether a key for a JSON object already exists. If it exists, an 
//  "duplicate key error" will be logged to error console and error will 
//  be set.
//                                
//
// JPJsonParserKeepStringCacheOnClear
// 
//  Clears the string cache if the semantic actions is cleared via function 
//  clear().
//
//
// JPJsonParserCacheDataStrings
// 
//  Caches data strings in addition to key strings.
//
// 
// JPJsonParserCreateMutableContainers      
//
//  If this option is set, the semantic actions object creates a JSON representation 
//  with mutable containers. That is, a JSON Array will be represented by a
//  NSMutableArray and a JSON Object will be represented by a NSMutableDictionary.
//

//
// Number Generation Options:
//
// The options for number generation can be used only mutual exclusive.
// 
// JPJsonParserNumberGeneratorGenerateAuto
// 
//  The parser's number generator creates suitable NSNumber objects when it
//  encounters a number in the input text. 
//  // TODO: If a NSNumber is not capable to hold the number, a NSDecimalNumber
//           will be created.
// JPJsonParserNumberGeneratorGenerateAuto equals zero and is the default option.
//
//
// JPJsonParserNumberGeneratorGenerateStrings
//
//  If this option is set, the parser number generator creates a NSString when 
//  it encounters numbers in the input text and initializes it accordingly. 
//  Usually, you wouldn't select this option if you want to output a JSON
//  container into a string or stream as a proper JSON text. In this case it
//  would treat the numbers as JSON strings enclosed in quotes. However, when
//  using a string it preserves the format of the number.
//
//
// JPJsonParserNumberGeneratorGenerateDecimals
// 
//  If this option is set the parser's number generator will always create a 
//  NSDecimalNumber object when encountering a number in the input.
//
//

//
// Unicode Handling
//

// JPJsonParserSignalErrorOnNoncharacter
//
// If this option is set, the parser will signal an error if it encounters
// an Unicode noncharacter within the JSON document.
// This is the default setting.

// JPJsonParserSubstituteUnicodeNoncharacter
//  
//  If this option is set, the parser will substitute Unicode noncharacters
//  encountered within JSON strings in the input text with the Unicode replace-
//  ment character U+FFFD when creating the strings for a JSON container. Other-
//  wise the parser stops parsing the input at the occurrence of an Unicode non-
//  character and signals an error.
//  It is not recommended to enable this option. Substituting an Unicode non-
//  character may modify the meaning of the input source and should be used 
//  with care. Usually, Unicode noncharacters are not allowed in valid Unicode 
//  sequences which is used to transmit data.
//  Occurrences of Unicode noncharacters outside of JSON strings will be always
//  syntax errors and treated as such.

// JPJsonParserSkipUnicodeNoncharacter
//
// If this option is set, the parser will ignore the Unicode noncharacter
// and remove it from the _decoded_ JSON string when generating a representation.
// *Note:* this i not yet implemented






#endif  // JSON_OBJC_JP_JPJSON_H

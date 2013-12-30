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
#include <Foundation/Foundation.h>

/** @name Constants */

/** JPUnicodeEncoding
 Defines constants for an Unicode encoding scheme.
 
 These constants are used to specify the Unicode encoding scheme for a 
 given input respectively the desired output.
 */
typedef NS_ENUM(NSInteger, JPUnicodeEncoding) {
    JPUnicodeEncoding_Unknown   = 0,   /** Unknown Unicode encoding scheme  */
    JPUnicodeEncoding_UTF8      = 1,   /** Unicode UTF-8 */
    JPUnicodeEncoding_UTF16     = 2,   /** Unicode UTF-16 in platform endianness */
    JPUnicodeEncoding_UTF16BE   = 3,   /** Unicode UTF-16 Big Endian */
    JPUnicodeEncoding_UTF16LE   = 4,   /** Unicode UTF-16 Little Endian */
    JPUnicodeEncoding_UTF32     = 5,   /** Unicode UTF-32 in platform endianness */
    JPUnicodeEncoding_UTF32BE   = 6,   /** Unicode UTF-32 Big Endian */
    JPUnicodeEncoding_UTF32LE   = 7    /** Unicode UTF-32 Little Endian */
} ;



/**
 Parser Options
 */
typedef NS_OPTIONS(NSUInteger, JPJsonParserOptions)  {
    
    // Handling of Unicode Noncharacters
    // (Mutual exclusive option)
    // (If none option is set, current setting will be left unchanged)
    JPJsonParserAllowUnicodeNoncharacter                    = 1UL << 0,
    JPJsonParserSignalErrorOnUnicodeNoncharacter            = 2UL << 0,
    JPJsonParserSubstituteUnicodeNoncharacter               = 3UL << 0,
    JPJsonParserRemoveUnicodeNoncharacter                   = 4UL << 0,

    JPJsonParserNoncharacterOptionMask                      = 0x07 << 0, // 4 bits
    
    // Handling of Unicode 'NULL' (U+0000)
    // (Mutual exclusive option)
    // (If none option is set, current setting will be left unchanged)
    JPJsonParserAllowUnicodeNULLCharacter                   = 1UL << 3,
    JPJsonParserSignalErrorOnUnicodeNULLCharacter           = 2UL << 3,
    JPJsonParserSubstituteUnicodeNULLCharacter              = 3UL << 3,
    JPJsonParserRemoveUnicodeNULLCharacter                  = 4UL << 3,

    JPJsonParserNULLCharacterOptionMask                     = 0x07 << 3,  // 4 bits

    // Log behavior
    // (Mutual exclusive flags)
    // (If none option is set, current setting will be left unchanged)
    JPJsonParserLogLevelNone                                = 1UL << 6,
    JPJsonParserLogLevelError                               = 2UL << 6,
    JPJsonParserLogLevelWarning                             = 3UL << 6,
    JPJsonParserLogLevelDebug                               = 4UL << 6,

    JPJsonParserLogLevelMask                                = 0x07 << 6,  // 4 bits
    
    // JSON Documents Stream Options
    JPJsonParserIgnoreSpuriousTrailingBytes                 = 1UL << 12,
    JPJsonParserParseMultipleDocuments                      = 1UL << 13,
    JPJsonParserParseMultipleDocumentsAsynchronously        = 1UL << 14,
    
    
    // Non Conformance Flags
    // (not yet implemented!)
    JPJsonParserAllowComments                               = 1UL << 15,
    JPJsonParserAllowControlCharacters                      = 1UL << 16,
    JPJsonParserAllowLeadingPlusInNumbers                   = 1UL << 17,
    JPJsonParserAllowLeadingZerosInIntegers                 = 1UL << 18,
    
    
    JPJsonParserEncodedStrings                              = 1UL << 19,
    
    
    // JPRepresentationGenerator Option Flags
    JPJsonParserCheckForDuplicateKey                        = 1UL << 18,
    JPJsonParserKeepStringCacheOnClear                      = 1UL << 19,
    JPJsonParserCacheDataStrings                            = 1UL << 20,
    JPJsonParserCreateMutableContainers                     = 1UL << 21,
    
    // Number generator options
    // (mutual exclusive option)
    // (If none option is set, current setting will be left unchanged)
    JPJsonParserNumberGeneratorGenerateAuto                 = 1UL << 22,
    JPJsonParserNumberGeneratorGenerateAutoWithDecimals     = 2UL << 22,
    JPJsonParserNumberGeneratorGenerateStrings              = 3UL << 22,
    JPJsonParserNumberGeneratorGenerateDecimals             = 4UL << 22,
    
    JPJsonParserNumberGeneratorMask                         = 0x07 << 22,  // 4 bits
    
    
    // Miscellaneous
    JPJsonParserGeneratorUseArenaAllocator                  = 1UL << 26,
    
    
    
    JPJsonParserNonConformanceMask =
    JPJsonParserAllowComments | 
    JPJsonParserAllowControlCharacters | 
    JPJsonParserAllowLeadingPlusInNumbers | 
    JPJsonParserAllowLeadingZerosInIntegers,
    
};




//
// Unicode 'NULL' (U+0000) Handling
//

// JPJsonParserAllowUnicodeNULLCharacter
//
// If this options is set, the parser will retain Unicode 'NULL' characters in
// JSON Strings in the output representation.
//

// JPJsonParserSignalErrorOnUnicodeNULLCharacter
//
//  If this option is set, the parser will signal an error if it encounters an
//  Unicode 'NULL' character within a JSON String. Note that a Unicode 'NULL'
//  in a JSON String is a valid character. However, Unicode 'NULL' characters
//  may be problematic in string representations like NSString, and applications.
//  Thus, this option may be set to detect unwanted Unicode 'NULL' characters.
//
//
// JPJsonParserSubstituteUnicodeNULLCharacter
//
//  If this option is set, the parser will substitute Unicode 'NULL' characters
//  encountered within JSON strings in the input text with the Unicode replacement
//  character U+FFFD when creating the string representation.
//
//  It is not recommended to enable this option. Substituting an Unicode 'NULL'
//  character will modify the meaning of the input source and should be used
//  with care.
//
//
// JPJsonParserRemoveUnicodeNULLCharacter
//
//  If this option is set, the parser will not retain the Unicode 'NULL' character
//  in the _decoded_ JSON string when generating a representation.
//
//  It is not recommended to enable this option. Removing an Unicode 'NULL'
//  character will modify the meaning of the input source and should be used
//  with care.
//
//
//
//  If none of the above Unicode 'NULL' handling options is set, the parser will
//  not change the current setting.
//
//  Note: Occurrences of Unicode 'NULL' characters outside of JSON strings will
//  be always syntax errors and treated as such.



//
// Unicode Noncharacter Handling
//

// JPJsonParserAllowUnicodeNoncharacter
//
//  If this option is set, the parser will retain Unicode noncharacters in JSON
//  Strings in the output representation.

//  JPJsonParserSignalErrorOnUnicodeNoncharacter
//
//  If this option is set, the parser will signal an error if it encounters an
//  Unicode noncharacter within a JSON String. Note that a Unicode noncharacter
//  in a JSON String does not make the unicode string illformed. However, Unicode
//  noncharacters are rarely useful in a meaningful JSON String. On the other hand,
//  Unicode noncharacters may be problematic in string representations on the
//  application level.
//
//
// JPJsonParserSubstituteUnicodeNoncharacter
//
//  If this option is set, the parser will substitute Unicode noncharacters
//  encountered within JSON strings in the input text with the Unicode replacement
//  character U+FFFD when creating the string representation.
//
//  It is not recommended to enable this option. Substituting an Unicode noncharacter
//  will modify the meaning of the input source and should be used with care.
//
//
// JPJsonParserRemoveUnicodeNoncharacter
//
//  If this option is set, the parser will not retain the Unicode noncharacter
//  in the _decoded_ JSON string when generating a representation.
//
//  It is not recommended to enable this option. Removing an Unicode noncharacter
//  will modify the meaning of the input source and should be used with care.
//
//
//
//  If none of the above Unicode Noncharacter Handling options is set, the parser
//  will not change the current settings.
//
//  Note: Occurrences of Unicode noncharacters outside of JSON strings will
//  be always syntax errors and treated as such.




//  JPJsonParserIgnoreSpuriousTrailingBytes
//
//  If this options is set, the parser will ignore any additional characters
//  that occur after the last significant character of a valid JSON document
//  (namely, either a "}" or a "]").
//  Otherwise, if the parser encounters code units which can not be interpreted
//  as white-space Unicode characters it will issue an error. If it encounters
//  an Unicode NULL (U+0000) or `EOF` it will issue a warning to the console.


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
//  resources by the underlying network layer.

//
// JPJsonParserEncodedStrings
//
// If enabled, the parser sends properly _encoded_ JSON Strings to the semantic
// actions object in method `-parserFoundKeyValuePairBeginWithKey:length:encoding:index:`
// and method `-parserFoundString:length:hasMore:encoding:`. That is, the string
// is encoded as required by RFC 4627.
//
// Otherwise (the default), the parser sends properly decoded strings to the
// semantic actions object which match the original source string.




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
//  The parser's number generator creates a suitable NSNumber object
//  when it encounters a number in the input text.
//
//  JPJsonParserNumberGeneratorGenerateAuto equals zero and is the default option.
//
//
// JPJsonParserNumberGeneratorGenerateAutoWithDecimals
//
//  The parser's number generator creates a suitable NSNumber or NSDecimal object
//  when it encounters a number in the input text.
//
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
// JPJsonParserGeneratorUseArenaAllocator
//
// If enabled, the receiver uses an _arena allocator_ when it creates _immutable_
// Foundations objects.
//
// Using an arena allocator improves performance, increases memory locality for the
// objects comprising the representation and reduces heap fragmentation. However,
// there is also a caveat:
//
// The memory allocated for the whole representation will eventually be freed only
// until after _ALL_ objects of the representation have been deallocated. Thus, in
// order to avoid possibly large memory areas hanging around unused, this option is
// only useful if the representation is used as a whole and released as a whole without
// having objects elsewhere referencing one or more objects from the representation.









#endif  // JSON_OBJC_JP_JPJSON_H

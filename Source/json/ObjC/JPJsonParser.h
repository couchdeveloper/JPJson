//
//  JPJsonParser.h
//
//  Created by Andreas Grosam on 7/2/11.
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

#ifndef JSON_OBJC_JP_JSON_PARSER_H
#define JSON_OBJC_JP_JSON_PARSER_H


#import "JPJsonCommon.h"   // for JPJsonParserOptions
#import <Foundation/Foundation.h>


@class JPSemanticActionsBase;

//
// Convenience Interface  - Synchronous processing
//

/** @name Overview */

/**
 JPJsonParser provides two _class_ methods for conveniently parse JSON documents
 which return a JSON representation of Foundation objects:
 
 -  `+parseString:options:error:`
 -  `+parseData:options:error:`
 
 The JSON document can be provided either with a `NSString` or with a `NSData` 
 object. The JSON text contained in a `NSData` object shall be encoded in one
 of the _Unicode encoding schemes_: UTF-8, UTF-16, UTF-16LE, UTF-16BE, UTF-32, 
 UTF-32LE or UTF-32BE. A _BOM_ may optionally precede the start of the byte
 stream.

 A number of options can be set in parameter _options_ which are ored flags from
 constants defined in enum "JPJsonParserOptions".
 
 A more powerful class method can be used in conjunction with a 
 _Semantic Actions Object_:
 
 -  `+parseData:semanticActions:`
 
 A _Semantic Actions Object_ acts as a delegate for the _internal JSON parser_. 
 It conforms to the protocol <JPSemanticActionsProtocol> and implements a set of 
 _semantic actions_ to corresponding _parse events_. The parse events will be 
 sent from the internal JSON parser to the semantic actions delegate when it 
 encounters JSON elements in the JSON document.
 
 
 ### JPJsonParserOptions ###

 *Note:* JPJsonParser Options will be only used in the JPJsonParser convenience 
 class methods `+parseString:options:error:` and `+parseData:options:error:`.
 Internally, these options simply set properties of the underlying components 
 (JSON parser and representation generator) of the JPJsonParser facade.
 
 JPJsonParser Options are a set of flags which can be combined with the bit-wise OR
 operator and are accumulated into one JPJsonParserOptions value. A few options can 
 be used only mutually exclusive.
 
 
#### Unicode 'NULL' (U+0000) Handling
 
 This group of options can be used only mutually exclusive. If none of the
 Unicode 'NULL' handling options is set, the parser will not change the current 
 settings.

 Note that a Unicode 'NULL' in a JSON String is a valid character. However, 
 Unicode 'NULL' characters may be problematic in string representations like NSString, 
 and applications.

 
 -  `JPJsonParserAllowUnicodeNULLCharacter`

    If this options is set, the parser will retain Unicode 'NULL' characters in
    JSON Strings in the output representation.
 
 
 -  `JPJsonParserSignalErrorOnUnicodeNULLCharacter`

    If this option is set, the parser will signal an error if it encounters an
    Unicode 'NULL' character within a JSON String.
 
    This is the default option.


 -  `JPJsonParserSubstituteUnicodeNULLCharacter`

    If this option is set, the parser will substitute Unicode 'NULL' characters
    encountered within JSON strings in the input text with the Unicode replacement 
    character U+FFFD when creating the string representation.

    It is not recommended to enable this option. Substituting an Unicode 'NULL'
    character will modify the meaning of the input source and should be used
    with care.


 -  `JPJsonParserRemoveUnicodeNULLCharacter`

    If this option is set, the parser will not retain the Unicode 'NULL' character
    in the _decoded_ JSON string when generating a representation.

    It is not recommended to enable this option. Removing an Unicode 'NULL'
    character will modify the meaning of the input source and should be used
    with care.

 
 *Note:* Occurrences of Unicode 'NULL' characters outside of JSON strings will
 be always syntax errors and treated as such.
 

 
 
#### Unicode Noncharacter Handling
 
 This group of options can be used only mutually exclusive. If none of the
 Unicode Noncharacters handling options is set, the parser will not change the 
 current settings.
 
 Note that a Unicode noncharacter in a JSON String does not make the unicode 
 string illformed. On the other hand, Unicode noncharacters may be problematic in 
 string representations and when used in the application.
 
 
 -  `JPJsonParserAllowUnicodeNoncharacter`

    If this option is set, the parser will retain Unicode noncharacters in JSON
    Strings in the output representation.

 
 -  `JPJsonParserSignalErrorOnUnicodeNoncharacter`

    If this option is set, the parser will signal an error if it encounters an
    Unicode noncharacter within a JSON String.

    This is the default option.


 -  `JPJsonParserSubstituteUnicodeNoncharacter`

    If this option is set, the parser will substitute Unicode noncharacters
    encountered within JSON strings in the input text with the Unicode replacement 
    character U+FFFD when creating the string representation.

    It is not recommended to enable this option. Substituting an Unicode noncharacter
    will modify the meaning of the input source and should be used with care.


 - `JPJsonParserRemoveUnicodeNoncharacter`

    If this option is set, the parser will not retain the Unicode noncharacter
    in the _decoded_ JSON string when generating a representation.

    It is not recommended to enable this option. Removing an Unicode noncharacter
    will modify the meaning of the input source and should be used with care.



 Note: Occurrences of Unicode noncharacters outside of JSON strings will
 be always syntax errors and treated as such.
 
 
 
 #### Parser Logging Options ####

 This group of options can be used only mutually exclusive. If none of the
 Logging options is set, the parser will not change the current settings.

 Generally, the parser will log to the error console.
 
 
 - `JPJsonParserLogLevelNone`
 
    If this option is set, the parser will not log any message to the console.
 
 
 - `JPJsonParserLogLevelError`
    
    If this option is set, the parser will log only errors to the console.
 
    This is the default option.
 
 
 - `JPJsonParserLogLevelWarning`
   
    If this option is set, the parser will log errors and warnings to the console.
 

 - `JPJsonParserLogLevelDebug`
 
    If this option is set, the parser will log debug information in addition to
    warnings and errors.

 
 
 #### Parser Document Stream Options ####
 
 All options are not set by default.
 
 - `JPJsonParserIgnoreSpuriousTrailingBytes`
 
    If this options is set, the parser will ignore any additional characters
    that occur after the last significant character of a valid JSON document
    (namely, either a "}" or a "]").
    Otherwise, if the parser encounters code units which can not be interpreted 
    as white-space Unicode characters it will issue an error, and if it encounters 
    an `Unicode NULL (U+0000)` or `EOF` it will issue a warning to the console.
 
 
 - `JPJsonParserParseMultipleDocuments`
 
    If this option is set, the parser parses one or more documents from the input until it
    receives `EOF`. Otherwise, the parser treats any non white spaces after the
    first JSON document as an error. 

    Default is OFF.

    *Note:* This option is ignored when invoking the JSON parser through one of
    the two convenience methods `+parseString:options:error:` and `+parseData:options:error:`
    since parsing multiple documents require to use a Semantic Actions object, and 
    possibly a asynchronous parser.
 
 
 - `JPJsonParserParseMultipleDocumentsAsynchronously`
 
    If this option is set, the parser invokes the `jsonObjectHandlerBlock`
    asynchronously and immediately processes the next JSON document within the 
    input data. The JSON container is then retained in the dispatch queue till 
    it is processed by the client. This may tie up a lot of system resources if 
    the client processes frees the JSON containers slowly.
    If the flag is `NO`, the parser's thread is blocked until the handler routine 
    returns.
 
    *Note:* This option has no effect when invoking the JSON parser through one of
    the convenience methods `+parseString:options:error:` and `+parseData:options:error:`.

    It is recommended to leave this flag disabled in systems where system resources 
    are scarce or if the data input possibly contains many and large JSON documents. 
    When downloading large data, this helps throttling the consumption of system
    resources by the underlying network layer.
 
 
 #### Representation Generator Options ####
 
 This group of options control the behavior of the representation generator.
 
 All options are not set by default.
 
 
 -  `JPJsonParserCheckForDuplicateKey`
 
    Checks whether a key for a JSON object already exists. If it exists, an
    "duplicate key error" will be logged to error console and error will 
    be set.
 
 
 -  `JPJsonParserCreateMutableContainers`
 
    If this option is set, the semantic actions object creates a JSON representation
    with mutable containers. That is, a JSON Array will be represented by a
    `NSMutableArray` and a JSON Object will be represented by a `NSMutableDictionary`.
 
 
 #### Number Generation Options ####

 This group of options control specifically the behavior of the number generation 
 of the representation generator.

 This group of options can be used only mutually exclusive. If none of the
 Number Generation Options is set, the parser will not change the current settings.
 
 *Note:* Detailed information about mapping of JSON number to a corresponding 
 Foundation class can be found in class <JPRepresentationGenerator>.
 
 
 -  `JPJsonParserNumberGeneratorGenerateAuto`
 
    The parser's number generator creates a `NSNumber` object with a suitable
    underlying numeric C type when it encounters a number in the input text.
    `JPJsonParserNumberGeneratorGenerateAuto` equals zero and is the default option.
 
    This is the default.
 
 
 -  `JPJsonParserNumberGeneratorGenerateAutoWithDecimals`
 
    The parser's number generator creates a `NSNumber` with a suitable underlying
    numeric C type or a `NSDecimalNumber` object when it encounters a number in the 
    input text.

 
 -  `JPJsonParserNumberGeneratorGenerateStrings`
 
    If this option is set, the parser number generator creates a `NSString` when
    it encounters numbers in the input text and initializes it accordingly. 
    Usually, you wouldn't select this option if you want to output a JSON
    container into a string or stream as a proper JSON text. In this case it 
    would treat the numbers as JSON Strings enclosed in quotes. However, when 
    using a string it preserves the format of the number.
 
 
 -  `JPJsonParserNumberGeneratorGenerateDecimals`
 
    If this option is set the parser's number generator will always create a
    `NSDecimalNumber` object when encountering a number in the input.


    
   
 #### Other options ####
 
 All options are not set by default.
 
 -  `JPJsonParserKeepStringCacheOnClear`
 
    Clears the string cache if the semantic actions is receives the message
    `clear`.
 
 
 -  `JPJsonParserCacheDataStrings`
 
    Caches data strings in addition to key strings.
 
 
 
 #### Miscellaneous
 
 All options are not set by default.
 
 -  `JPJsonParserEncodedStrings`
 
    If this option is set, the parser sends properly _encoded_ JSON Strings to the semantic
    actions object in method `-parserFoundKeyValuePairBeginWithKey:length:encoding:index:`
    and method `-parserFoundString:length:hasMore:encoding:`. That is, the string
    is encoded as required by RFC 4627.

    Otherwise (the default), the parser sends properly decoded strings to the
    semantic actions object which match the original source string.
 
 
 
 
 
 ### Using a Semantic Actions object ###
 
 
 Each parser, that is an instance of `JPJsonParser` or and instance of
 <JPAsyncJsonParser>, needs to be associated to a _Semantic Actions_ object. If 
 none is specified when a parser is created, the parser itself creates a default 
 one, which is a `JPRepresentationGenerator`. 
 
 *Info:*  A parser sends _parse events_ to the semantic actions object as they 
 appear in the JSON text. The task of a semantic actions object is to handle 
 these events appropriately.
 
 There are two "built-in" semantic actions classes:
 
 - <JPRepresentationGenerator> and
 - <JPStreamSemanticActions>
 
 *see also:*  <JPSemanticActionsBase> and <JPSemanticActionsProtocol>. 
 
 
 The purpose of `JPRepresentationGenerator` is to create a _representation_ of a 
 JSON document as a hierarchical structure of _Foundation objects_. This class
 is _self-contained_, that means it handles all the parse events and generates 
 the representation itself without the need of a delegate. There is rarely the 
 need to subclass it.
 
 The purpose of a `JPStreamSemanticActions` is to provide a _SAX style_ API. This 
 gives fine grained control over the actions which shall be taken on the various 
 _parse events_. `JPStreamSemanticActions` can be subclassed in order to implement 
 the delegate methods which will be invoked by the parser on "parse events".


 
 
 For detailed information about semantic actions see class <JPSemanticActionsBase>, 
 <JPRepresentationGenerator> and <JPSemanticActionsProtocol>.

 */

@interface JPJsonParser : NSObject 

/** @name Parse a JSON Document from a NSString object */

// Synopsis:
// + (id) parseString:(NSString*)string options:(JPJsonParserOptions)options error:(NSError**)error
//
/**
 Parses the JSON text given in parameter _string_ and returns a representation
 of the JSON text as a Foundation object.
 
 This method internally creates a `JPRepresentationGenerator` instance and configures 
 it according the specified options. Then it starts parsing immediately and returns
 the result. For a detailed description of class  `JPRepresentationGenerator`
 see <JPRepresentationGenerator>.
 
 Using this convenience method, the parser can only parse one JSON document. That 
 is, parameter _string_ should not contain multiple JSON documents.
 
 
 @param string A NSString object containing one JSON text document.
 
 @param options A bit mask specifying various options for parsing. For possible values
 see _JPJsonParserOptions_.

 @param error A pointer to a NSError object. If this is not `NULL`, and if an error
 occurred during parsing the parameter _error_ contains and `NSError` object
 describing the issue.

 @return A Foundation object representing the root object of the representation of
 the JSON text specified in parameter _string_, or `nil` if an error occurred.

*/
+ (id) parseString:(NSString*)string 
           options:(JPJsonParserOptions)options 
             error:(__autoreleasing NSError**)error;


/** @name Parse a JSON Document from a NSData object */

// Synopsis:
// + (id) parseData:(NSData*)data options:(JPJsonParserOptions)options error:(NSError**)error


/**
 Parses the JSON text given in parameter _data_ and returns a representation 
 of the JSON text as a Foundation object.

 The data must contain text in one of the following Unicode encoding schemes:
 UTF-8, UTF-16, UTF-16LE, UTF-16BE, UTF-32, UTF-32LE or UTF-32BE. A BOM
 may optionally precede the start of the byte stream.
 
 This method internally creates a `JPRepresentationGenerator` instance and configures 
 it according the specified options. Then it starts parsing immediately and returns
 the result. For a detailed description of class  `JPRepresentationGenerator`
 see <JPRepresentationGenerator>.
 
 
 Using this method, the parser can only parse one JSON document contained
 within the data object. That is, "data" should not contain multiple JSON 
 documents.
 
 @param data A byte sequence containing JSON text
 
 @param options A bit mask specifying various options for parsing. For possible values see 
 _JPJsonParserOptions_.

 @param error A pointer to a NSError object. If this is not `NULL`, and if an error occurred
 during parsing the parameter error contains and NSError object describing the 
 issue.

 @return A Foundation object representing the JSON text in _data_, or `nil` if an 
 error occurred.

 @see + parseString:options:error:
 
*/
+ (id) parseData:(NSData*)data 
         options:(JPJsonParserOptions)options
           error:(__autoreleasing NSError**)error;


+ (id) parseData:(NSData*)data 
        encoding:(JPUnicodeEncoding) encoding
         options:(JPJsonParserOptions)options
           error:(__autoreleasing NSError**)error;




/** @name Using a Semantic Actions Object */

// Synopsis:
// + (BOOL) parseData:(NSData*)data semanticActions:(JPSemanticActionsBase*)sa;
//
/**
 Parses the JSON text given in parameter _data_ and processes the JSON elements
 according the actual semantic actions object provided in parameter 
 _semanticActions_.
 
 
 The data must contain text in one of the following Unicode encoding schemes:
 UTF-8, UTF-16, UTF-16LE, UTF-16BE, UTF-32, UTF-32LE or UTF-32BE. A BOM may
 optionally precede the start of the byte stream.
 
 The semantic actions object must be properly configured before this method
 will be invoked. The exact behavior of a "semantic actions" class depends 
 on its actual implementation. 
 
 
 
 @param data A byte sequence containing JSON text encoded in one of the valid Unicode 
 encoding scheme.
 
 @param semanticActions A "semantic actions" object which provides the semantic 
 actions for the parser. This parameter must not be `nil`. For detailed information 
 about semantic actions see class <JPSemanticActionsBase>, <JPRepresentationGenerator>,
 <JPStreamSemanticActions> and <JPSemanticActionsProtocol>.

 
 @return Returns `YES`, if the input could be successfully parsed and successfully 
 processed by the semantic actions object.
 Otherwise, if an error occurred returns `NO`. The actual error object can be retrieved from the current semantics actions object.

 @see + parseString:options:error:

 */
+ (BOOL) parseData:(NSData*)data semanticActions:(JPSemanticActionsBase*)semanticActions;



@end


#endif

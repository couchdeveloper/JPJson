#ifndef JSON_OBJC_JP_SEMANTIC_ACTIONS_PROTOCOL_HPP
#define JSON_OBJC_JP_SEMANTIC_ACTIONS_PROTOCOL_HPP 
//
//  JPSemanticActionsProtocol.h
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

#import <Foundation/Foundation.h>


/**
 JPSemanticActionsProtocol defines the interface between the _underlaying 
 JSON parser_ and the semantic actions object. The protocol must be implemented
 by concrete subclasses of JPSemanticActionsBase.
 
 The JSON parser will not perform _semantic actions_ by itself, instead it will
 notify the semantic actions object of certain _parse events_ and provide 
 necessary information for the event. 
  
 Parse events are signaled by the underlaying JSON parser via messages sent to
 the semantic actions object whose signature is prefixed with "parserFound". The 
 semantic actions object is supposed to implement a corresponding "semantic action" 
 appropriate for the event and the current context and current state.
 
 A "begin" message, that is a `parserFoundArrayBegin` or a `parserFoundObjectBegin`
 message, indicates the start of a JSON container (a JSON Array, or a JSON Object).
 An "end" message, that is a `parserFoundArrayEnd` or a `parserFoundObjectEnd`
 message, indicates the end of the container which refers to the container
 signaled with the "begin" message immediately preceding this "end" message.
 
 Additionally, there are two events `parserFoundKeyValuePairBeginWithKey:length:encoding:index:` 
 and `parserFoundKeyValuePairEnd` which are sent when the parser encountered
 a JSON string which is a key in the key-value pair of a JSON Object, and when
 the parser encountered the end of the JSON Value associated to this key.
 
 "begin" and "end" messages are strictly balanced and reflect the recursive, 
 respectively hierarchical, structure of a JSON representation.
 
 
 Finally, the underlaying parser sends messages parserFound<JSON_primitive>, that 
 is `parserFoundString`, `parserFoundNull`, `parserFoundBoolean` and
 `parserFoundNumber` when it encounters a corresponding JSON primitive value. 
 JSON Strings (which include keys) will be unescaped by the parser.
 
 
 Below is a typical flow of the complete messages:
 
    parserFoundArrayBegin
        parserFoundObjectBegin
            parserFoundKeyValuePairBeginWithKey:length:encoding:index:
                parserFoundArrayBegin
                    parserFoundValueBeginAtIndex:
                    parserFoundString:length:hasMore:encoding:
                    parserFoundString:length:hasMore:encoding:
                    parserFoundValueEndAtIndex:

                    parserFoundValueBeginAtIndex:
                    parserFoundString:length:hasMore:encoding:
                    parserFoundValueEndAtIndex:
                    
                    ...
 
                parserFoundArrayEnd
            parserFoundKeyValuePairEnd
 
            parserFoundKeyValuePairBeginWithKey:length:encoding:index:
                parserFoundArrayBegin
                    parserFoundValueBeginAtIndex:
                    parserFoundNumber:length:
                    parserFoundValueEndAtIndex:
                     
                    parserFoundValueBeginAtIndex:
                    parserFoundNumber:length:
                    parserFoundValueEndAtIndex:
                    ..
                parserFoundArrayEnd
            parserFoundKeyValuePairEnd
 
        parserFoundObjectEnd
        parserFoundObjectBegin
            ..
        parserFoundObjectEnd
     parserFoundArrayEnd
 
 
 A `parserFoundString` message may be possibly sent repeatedly, if the string
 is handled in "chunks". The method has a parameter `hasMore` which indicates 
 when the string is eventually complete. A data string will be split into chunks 
 if its size becomes large. The exact size of a chunk is dependent on the
 implementation. Currently, this equals about 2000 code units. Occasionally, 
 this may become larger if the implementation can utilize an already allocated 
 buffer whose size is larger.
 
 
 Implementing actions for "begin", "end" and "found primitive value" messages
 will be usually sufficient to implement a semantic actions class which creates 
 a JSON representation., e.g. with Foundation objects as containers and 
 primitive values.
 
 A concrete implementation would purposefully use an internal stack in order 
 to aid the construction of a JSON representation, e.g. with Foundation 
 objects.
 
 
 
 Basically, a "representation generator" could be implemented as follows:
 
 A "begin" message will be the first message sent from the parser - besides 
 the more informative "parserFoundJsonBegin" event. A valid JSON starts with
 either a JSON Array or a JSON Object.
 
 Now, a "begin" event may not be immediately followed by the corresponding 
 "end" event - rather, since a JSON representation is a hierarchical data 
 structure, "begin" may be followed by one or more other "begin" events. But 
 eventually, the "begin" and "end" become balanced. Nonetheless, the basic 
 principle of building a representation remains simple:
 
 So, on a "begin" message, the generator would create the corresponding
 container (for Foundation, a NSArray or NSDictionary object) and push it 
 onto its internal stack. 
 
 On a "found primitive value" message, the generator would simply construct a 
 corresponding representation (e.g. a Foundation object) for a JSON String (Key), 
 JSON Boolean, and a JSON Null and push them onto the stack as well.
 
 On an "end" message the generator would pop all objects from laying above 
 the current container to the top of the stack and insert them into it. Now, 
 this container will become the top of the stack - which in turn may be an 
 element of another container. 
 
 Note that the corresponding action on an "end" message for JSON Array 
 respectively JSON Object do differ slightly, since a JSON Object expects
 key-value pairs on the stack and a JSON Array expects values in a sequence 
 and must retain the order of elements. 
 
 The process ends with the last message, an "end" message, whose action is
 exactly the same: pop the elements from the stack and insert them into the 
 current container. The current container is, of course, the last remaining
 object residing in the stack, and this becomes the final result.
 
 
 
 
 The four methods
 
 - `-parserFoundValueBeginAtIndex:`,
 - `-parserFoundValueEndAtIndex:`, 
 - `-parserFoundValueBeginWithKey:length:encoding:index` and
 - `-parserFoundValueEndWithKey:length:encoding:index`
 
 are helpful when implementing a streaming API. These additional messages 
 give fine grained control for all relevant parser events.
 For more information see <JPSemanticActionsProtocol> and <JPStreamSemanticActions>.
 
 
 For instance, these messages will be required, reconstructing the JSON text
 from the parser events (possibly converted to another Unicode encoding, with 
 unescaped Unicode characters, removed comments, etc.)
 
 These message are invoked as follows:
 
    parserFoundArrayBegin
        parserFoundValueBeginAtIndex:
        ...
        parserFoundValueEndAtIndex:
        ...
    parserFoundArrayEnd
 
    parserFoundObjectBegin
        parserFoundKeyValuePairBeginWithKey:length:encoding:index:
        ...
        parserFoundKeyValuePairEnd
        ...
    parserFoundObjectEnd
 */



@protocol JPSemanticActionsProtocol <NSObject>

@optional

/**
 Retrieves the (abstract) result of the semantic actions instance. 
 
 The property may return `nil`. 
*/ 
- (id) result;


/**
 Sets the property hasBOM.
 
 The setHasBOM method will be invoked by the JPJsonParser and JPAsyncJsonParser
 when they try to determine the encoding of the input and thereby check for 
 a BOM. The semantic actions instance may use this information if required.
 
 @param value A boolean value.
*/ 
- (void) setHasBOM:(BOOL) value;


/**
 Clears internal cached data.
 
 Method clear shall clear internal data which has been collected during a run.
 That is, it shall reset error information and temporary data like caches.
 @note JPJsonParser and JPAsyncJsonParser do not invoke this method.
*/ 
- (void) clear;


/**
 Terminates a run as soon as possible. 
 
 The error state after a cancel may be undefined, but ideally it should be 
 "canceled". JPAsyncJsonParser will invoke this method when itself receiving 
 the cancel  message.
*/ 
- (void) cancel;


@optional
/** @name Delegate Methods  */
 
// This interface is accessed by the JSON parser, respectively by
// the "semantic actions implementation" class.
 

/** 
 Sent to the delegate when the JSON parser found the start of a JSON document.
 
 Delegates should implement this method as a minimal implementation in order
 to perform appropriate actions when the JSON parser starts to parse a JSON
 document.
*/
- (void) parserFoundJsonBegin;


/** 
 Sent to the delegate when the JSON parser found the end of a JSON document.
*/
- (void) parserFoundJsonEnd;

/** 
 Sent to the delegate when the JSON parser finished parsing the last JSON
 document from its input.

 Delegates should implement this method as a minimal implementation in order
 to perform appropriate actions when the JSON parser finished to parse the
 input which possibly contains one or more JSON documents.
*/
- (void) parserFinished;


/** 
 Sent to the delegate when the JSON parser detected a parse error.

 Delegates should implement this method as a minimal implementation in order
 to perform appropriate actions when the JSON parser detected a parse error.
*/ 
- (void) parserDetectedError;




/** 
 Sent to the delegate when the JSON parser found the start of a JSON Array,
 that is when it encountered the '[' character.
 */
- (void) parserFoundArrayBegin;


/** 
 Sent to the delegate when the JSON parser found the end of a JSON Array,
 that is when it encountered the ']' character.
 */
- (void) parserFoundArrayEnd;


/** 
 Sent to the delegate when the JSON parser found the start of a JSON Object,
 that is when it encountered the '{' character.
 */
- (void) parserFoundObjectBegin;


/** 
 Sent to the delegate when the JSON parser found the end of a JSON Object,
 that is when it encountered the '}' character.
 */
- (bool) parserFoundObjectEnd;



/** 
 Sent to the delegate when the parser found the start of the nth key-value pair
 within a JSON Object.
 
 If property `generateEncodedStrings` equals `NO` (default) the JSON parser will 
 pass the key as a _decoded_ JSON String. If property `generateEncodedStrings`
 equals `YES` the character sequence represents an _encoded_ JSON string as specified
 in RFC 4627.
 
 _Decoding_ a JSON string involves unescaping, possibly replacing certain Unicode
 characters with their replacement character as specified in the semantic actions
 configuration, and possibly converting from the source encoding to the specified
 Unicode encoding form. The resulting string should compare equal to the original
 string.
  
 Subsequently, the delegate will be sent parse events which constitute the value
 associated to the key.
 
 @note The specified Unicode encoding scheme in parameter _encoding_ 
 corresponds to the Library Build Option `JSON_SEMANTIC_ACTIONS_STRING_ENCODING` and
 cannot be selected at runtime.
 
 @param bytes A const void pointer to the start of the sequence of the JSON String.
 @param length The number of bytes of the character sequence.
 @param encoding The _Unicode encoding scheme_ for the character sequence.
 @param index The current index of the key-value pair of the JSON Object.
 */ 
- (void) parserFoundKeyValuePairBeginWithKey:(const void*)bytes
                                      length:(size_t)length
                                    encoding:(NSStringEncoding)encoding
                                       index:(size_t)index;


/** 
 Sent to the delegate when the parser found the end of a key-value pair of a 
 JSON Object.
 
 */ 
- (void) parserFoundKeyValuePairEnd;




/** 
 Sent to the delegate when the parser found the start of a JSON Value 
 belonging to an JSON Array at the specified index.
 
 @param index The index of at which the value is added to the JSON Array.
 */ 
- (void) parserFoundValueBeginAtIndex:(size_t)index;


/** 
 Sent to the delegate when the parser found the end of a JSON Value 
 belonging to an JSON Array at the specified index.
 
 The associated value at this index has been notified by the parser by the 
 corresponding event which has been sent immediately before this message.
 
 @param index The index of at which the value is added to the JSON Array.
 */ 
- (void) parserFoundValueEndAtIndex:(size_t)index;




/**
 Sent to the delegate when the JSON parser found a JSON String as a value.

 If parameter _hasMore_ equals `YES`, the parser found a large string and is 
 delivering the JSON String in several chunks. 
 
 If property `generateEncodedStrings` equals `NO` (default) each chunk has been 
 _decoded_ and should compare equal to the original string. If property `generateEncodedStrings` 
 equals `YES` the character sequence represents an _encoded_ JSON string as specified
 in RFC 4627.
 
 In case of multibyte encodings, it is guaranteed that the  byte sequence will 
 always end at a complete character boundary.
 
 The parser will send as many chunks as necessary to complement the string through 
 sending consecutive messages to the receiver.
  
 _Decoding_ a JSON string involves unescaping, possibly replacing certain Unicode 
 characters with their replacement character as specified in the semantic actions 
 configuration, and possibly converting from the source encoding to the specified 
 Unicode encoding form. The resulting string should compare equal to the original 
 string.
 
 @note The specified Unicode encoding scheme in parameter _encoding_ 
 corresponds to the Library Build Option `JSON_SEMANTIC_ACTIONS_STRING_ENCODING` and
 cannot be selected at runtime.
 
 @param bytes    A void pointer to the start of the possibly partial sequence of the JSON String.
 @param length   The number of bytes of the sequence.
 @param hasMore  A boolean value which equals `YES` in order to indicate that there are more characters available for this JSON string.
 @param encoding The _Unicode encoding scheme_ for the character sequence.
 */ 
- (void) parserFoundString:(const void*)bytes 
                    length:(size_t)length 
                   hasMore:(BOOL)hasMore 
                  encoding:(NSStringEncoding)encoding;



/** 
 Sent to the delegate when the JSON parser found a JSON Number.

 The JSON parser will pass the JSON Number "as is", though converted into ASCII 
 which for number strings is equal to UTF-8 encoding.

 @param numberString A const char pointer to the start of the sequence of the JSON Number encoded in ASCII.
 @param length The number of bytes of the character sequence.
*/
- (void) parserFoundNumber:(const char*)numberString length:(size_t)length; 


/** 
 Sent to the delegate when the JSON parser found a JSON Boolean.
 @param value    Corresponds to the boolean value of the JSON Boolean.
*/  
- (void) parserFoundBoolean:(BOOL)value;


/** 
 Sent to the delegate when the JSON parser found a JSON Null. 
*/
- (void) parserFoundNull;



@end


#endif  // JSON_OBJC_JP_SEMANTIC_ACTIONS_PROTOCOL_HPP

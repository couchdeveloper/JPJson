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
 json parser_ and the semantic actions object. The protocol must be implemented 
 by concrete subclasses of JPSemanticActionsBase.
 
 The json parser will not perform _semantic actions_ by itself, instead it will 
 notify the semantic actions object of certain _parse events_ and provide 
 necessary information for the event. 
  
 Parse events are signaled by the underlaying json parser via messages sent to
 the semantic actions object whose signature begins with "parser". The semantic 
 actions object is supposed to implement a corresponding "semantic action" 
 appropriate for the event and the current context and current state.
 
 A "begin" message, that is a `parserFoundJsonArrayBegin` or a `parserFoundJsonObjectBegin`
 message, indicates the start of a JSON container (a JSON Array, or a JSON Object).
 An "end" message, that is a `parserFoundJsonArrayEnd` or a `parserFoundJsonObjectEnd`
 message, indicates the end of the container which referes to the container
 signaled with the "begin" message immediately preceeding this "end" message.
 
 "begin" and "end" messages are strictly balanced and reflect the recursive, 
 respectively hierarchical, structure of a JSON representation.
 
 
 The underlaying parser sends messages parserFoundJson<JSON_primitive>, that is 
 `parserFoundJsonString`, `parserFoundJsonNull`, `parserFoundJsonBoolean`,
 `parserFoundJsonNumber` and `parserFoundJsonKey` when it encounters a 
 corresponding JSON primitive value. JSON strings (and keys) will be unescaped
 by the parser.
 
 
 A typical flow of messages could be as follows:
 
    parserFoundJsonArrayBegin
        parserFoundJsonObjectBegin
            parserFoundJsonKey
            parserFoundJsonArrayBegin
                parserFoundJsonString
                parserFoundJsonString
                .. 
            parserFoundJsonArrayEnd
            parserFoundJsonKey
            parserFoundJsonArrayBegin
                parserFoundJsonString
                parserFoundJsonString
                ..
            parserFoundJsonArrayEnd
        parserFoundJsonObjectEnd
        parserFoundJsonObjectBegin
            ..
        parserFoundJsonObjectEnd
     parserFoundJsonArrayEnd
 
 
 
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
 repsectively JSON Object do differ slightly, since a JSON Object expects 
 key-value pairs on the stack and a JSON Array expects values in a sequence 
 and must retain the order of elements. 
 
 The process ends with the last message, an "end" message, whose action is
 exactly the same: pop the elements from the stack and insert them into the 
 current container. The current container is, of course, the last remaining
 object residing in the stack, and this becomes the final result.
 
 
 
 
 The four methods
 
 - `-parserFoundJsonValueBeginAtIndex:`,
 - `-parserFoundJsonValueEndAtIndex:`, 
 - `-parserFoundJsonValueBeginWithKey:length:encoding:index` and
 - `-parserFoundJsonValueEndWithKey:length:encoding:index`
 
 are helpful when implementing a streaming API. These additional messages 
 give fine grained control for all relevant parser events.
 For more information see <JPSemanticActionsProtocol> and <JPStreamSemanticActions>.
 
 
 For instance, these messages will be required, reconstructing the JSON text
 from the parser events (possibly converted to another Unicode encoding, with 
 unescaped Unicode characters, removed comments, etc.)
 
 These message are invoked as follows:
 
    parserFoundJsonArrayBegin
        parserFoundJsonValueBeginAtIndex
            ..
        parserFoundJsonValueEndAtIndex
        ..
 
    parserFoundJsonObjectBegin
        parserFoundJsonKey
        parserFoundJsonValueBeginWithKey
            ..
        parserFoundJsonValueEndWithKey
        ..
 */



@protocol JPSemanticActionsProtocol <NSObject>

@optional

/**
 Retrieves the (abstract) result of the semantic actions instance. 
 
 The property may return nil. 
*/ 
- (id) result;


/**
 Sets the property hasBOM.
 
 The setHasBOM method will be invoked by the JPJsonParser and JPAsyncJsonParser
 when they try to determine the encoding of the input and thereby check for 
 a BOM. The semantic actions instance may use this information if requried.
 
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
 
// This interface is accessed by the json parser, respectively by
// the "semantic actions implementation" class.
 

/** 
 Sent to the delegate when the json parser found the start of a JSON document.
 
 Delegates should implement this method as a minimal implemenation in order 
 to perform appropriate actions when the json parser starts to parse a JSON
 document.
*/
- (void) parserFoundJsonBegin;


/** 
 Sent to the delegate when the json parser found the end of a JSON document.
*/
- (void) parserFoundJsonEnd;

/** 
 Sent to the delegate when the json parser finished parsing the last JSON 
 document from its input.

 Delegates should implement this method as a minimal implemenation in order 
 to perform appropriate actions when the json parser finished to parse the 
 input which possibly contains one or more JSON documents.
*/
- (void) parserFinished;


/** 
 Sent to the delegate when the json parser detected a parse error.

 Delegates should implement this method as a minimal implemenation in order 
 to perform appropriate actions when the json parser detected a parse error.
*/ 
- (void) parserDetectedError;


/**
 Sent to the delegate when the json parser found a JSON String.
 
 The json parser will pass the _decoded_ JSON String. _Decoding_ a JSON string 
 involves unescaping, possibly replacing certain Unicode characters with their 
 replacement character as specified in the semantic actions configuration, and 
 possibly converting from the source encoding to the specified Unicode encoding 
 form. 
 
 @warning *Note:* The specified Unicode encoding scheme in parameter `encoding` 
 corresponds to the Library Build Option `JP_JSON_STRING_BUFFER_ENCODING` and
 cannot be selected at runtime.
 
 @param bytes    A void pointer to the start of the sequence of the JSON String.
 @param length   The number of bytes of the character sequence.
 @param encoding The _Unicode encoding scheme_ for the character sequence.
*/ 
- (void) parserFoundJsonString:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding;


/** 
 Sent to the delegate when the json parser found a JSON String which is 
 the "key" of a JSON key-value pair.

 The json parser will pass the _decoded_ JSON String. _Decoding_ a JSON string 
 involves unescaping, possibly replacing certain Unicode characters with their 
 replacement character as specified in the semantic actions configuration, and 
 possibly converting from the source encoding to the specified Unicode encoding 
 form. 
 
 @warning *Note:* The specified Unicode encoding scheme in parameter `encoding` 
 corresponds to the Library Build Option `JP_JSON_STRING_BUFFER_ENCODING` and
 cannot be selected at runtime.

 @param bytes    A const void pointer to the start of the sequence of the JSON String.
 @param length   The number of bytes of the character sequence.
 @param encoding The _Unicode encoding scheme_ for the character sequence.
*/ 
- (void) parserFoundJsonKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding;


/** 
 Sent to the delegate when the json parser found a JSON Number.

 The json parser will pass the JSON Number "as is", though converted into ASCII 
 which for number strings is equal to UTF-8 encoding.

 @param numberString A const char pointer to the start of the sequence of the JSON Number encoded in ASCII.
 @param length The number of bytes of the character sequence.
*/
- (void) parserFoundJsonNumber:(const char*)numberString length:(size_t)length; 


/** 
 Sent to the delegate when the json parser found a JSON Boolean.
 @param value    Corresponds to the boolean value of the JSON Boolean.
*/  
- (void) parserFoundJsonBoolean:(BOOL)value;


/** 
 Sent to the delegate when the json parser found a JSON Null. 
*/
- (void) parserFoundJsonNull;


/** 
 Sent to the delegate when the json parser found the start of a JSON Array,
 that is when it encountered the '[' character.
*/
- (void) parserFoundJsonArrayBegin;


/** 
 Sent to the delegate when the json parser found the end of a JSON Array,
 that is when it encountered the ']' character.
*/
- (void) parserFoundJsonArrayEnd;


/** 
 Sent to the delegate when the json parser found the start of a JSON Object,
 that is when it encountered the '{' character.
*/
- (void) parserFoundJsonObjectBegin;


/** 
 Sent to the delegate when the json parser found the end of a JSON Object,
 that is when it encountered the '}' character.
*/
- (bool) parserFoundJsonObjectEnd;


/** 
 Sent to the delegate when the parser found the start of a JSON Value 
 belonging to an JSON Array at the specified index.

 @param index The index of at which the value is added to the JSON Array.
*/ 
- (void) parserFoundJsonValueBeginAtIndex:(size_t)index;


/** 
 Sent to the delegate when the parser found the end of a JSON Value 
 belonging to an JSON Array at the specified index.

 @param index The index of at which the value is added to the JSON Array.
*/ 
- (void) parserFoundJsonValueEndAtIndex:(size_t)index;


/** 
 Sent to the delegate when the parser found the start of the n'th JSON Value 
 belonging to a JSON Object with the specified key.
 
 The json parser will pass the _decoded_ JSON String. _Decoding_ a JSON string 
 involves unescaping, possibly replacing certain Unicode characters with their 
 replacement character as specified in the semantic actions configuration, and 
 possibly converting from the source encoding to the specified Unicode encoding 
 form. 

 @warning *Note:* The specified Unicode encoding scheme in parameter `encoding` 
 corresponds to the Library Build Option `JP_JSON_STRING_BUFFER_ENCODING` and
 cannot be selected at runtime.

 @param bytes A const void pointer to the start of the sequence of the JSON String.
 @param length The number of bytes of the character sequence.
 @param encoding The _Unicode encoding scheme_ for the character sequence.
 @param index The current index of the key-value pair of the JSON Object.
 */ 
- (void) parserFoundJsonValueBeginWithKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding index:(size_t)index;


/** 
 Sent to the delegate when the parser found the end of the n'th JSON Value 
 belonging to a JSON Object with the specified key.

 The json parser will pass the _decoded_ JSON String. _Decoding_ a JSON string 
 involves unescaping, possibly replacing certain Unicode characters with their 
 replacement character as specified in the semantic actions configuration, and 
 possibly converting from the source encoding to the specified Unicode encoding 
 form. 
 
 @warning *Note:* The specified Unicode encoding scheme in parameter `encoding` 
 corresponds to the Library Build Option `JP_JSON_STRING_BUFFER_ENCODING` and
 cannot be selected at runtime.
  
 @param bytes A const void pointer to the start of the sequence of the JSON String.
 @param length The number of bytes of the character sequence.
 @param encoding The _Unicode encoding scheme_ for the character sequence.
 @param index The current index of the key-value pair of the JSON Object.
*/ 
- (void) parserFoundJsonValueEndWithKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding index:(size_t)index;


@end


#endif  // JSON_OBJC_JP_SEMANTIC_ACTIONS_PROTOCOL_HPP

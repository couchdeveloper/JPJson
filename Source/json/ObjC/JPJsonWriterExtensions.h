//
//  JPJsonWriterExtensions.h
//  
//
//  Created by Andreas Grosam on 5/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>



/**
 JPJsonStreambufferProtocol defines the interface for an stream buffer:
 
 Custom classes which map to a JSON primitive element will use the stream buffer 
 in their implementation of method JPJson_serializeTo:encoding:options:level: in
 order to serialize itself into the stream. (Custom classes for JSON __containers__, 
 like array and object will instead use the predefined class methods 
 serializeObjectAsJSONArray and serializeObjectAsJSONObject).
 */
@protocol JPJsonStreambufferProtocol <NSObject>


/**
 Write a sequence of bytes.
 
 Write up to `length` bytes from the array of bytes pointed to by `buffer` into
 the stream buffer.
 
 @param buffer A pointer to the sequence of bytes to be written.
 
 @param length The number of bytes to write.
 
 @return Returns the actual number of bytes written, or a negative number
 indicating an error.
 */

- (NSInteger) write:(const void*)buffer length:(NSUInteger)length;

@end






/**
 The JPJsonSerializableProtocol must be implemented for *custom* classes,
 which shall be serialized into a JSON document. By default, only the 
 Objective-C classes specified in the mapping can be used to be serialized as
 JSON. 

 When implementing the JPJsonSerializableProtocol for a class, the objects 
 will be serialized into their corresponding JSON element - provided the 
 implementation meets all requirements. For example, a NSDate object could be 
 serialized as a JSON String, while formatting the date into an appropriate 
 date string, or an NSImage could be base64 encoded and then serialized as a 
 JSON string, too.
*/
@protocol JPJsonSerializableProtocol <NSObject>


/**
 An implementation shall serialize itself as a character sequence and write the 
 characters into the stream `buffer`.
 
 @param streambuf A stream buffer into which the characters will be written.
 
 @param encoding The output encoding of the character sequence for the serialized 
 object.
 
 @param options Options that tailor the format of the output.
 
 @param level The current level of the hierarchy of the object structur. This is 
 used only when additional format characters shall be inserted into the JSON text,
 for example when "pretty printing".
 
 
 The character sequence shall be a valid JSON element according RFC 4627, including 
 the appropariate syntax elements.
 
*/ 
- (NSInteger) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>) streambuf
                        encoding:(JPUnicodeEncoding)encoding 
                         options:(JPJsonWriterOptions)options 
                           level:(NSUInteger)level;

@end




/**
 JPJsonWriter (Extension) defines some additional methods which help in serializing
 objects.
 
 */
@interface JPJsonWriter (Extension)

//
///**
// 
// Returns the default number formatter for NSDecimalNumber
// 
// */
//+ NSNumberFormatter* defaultDecimalNumberFormatter;
//


/**
 Serialize an object which shall be represented as a JSON Array. This method already 
 implements the boiler plate code necessary to serialize an object as a JSON Array.
 
 @param object The "array like" object which shall be serialized
 
 @param streambuf A stream buffer into which the characters will be written.
 
 @param encoding The output encoding of the character sequence for the serialized 
 object.
 
 @param options Options that tailor the format of the output.
 
 @param level The current level of the hierarchy of the object structur. This is 
 used only when additional format characters shall be inserted into the JSON text,
 for example when "pretty printing".
 
 @return TODO
 
 Parmeter `object` shall respond to message `count` and shall implement the 
 protocol NSFastEnumeration. 
 */
+ (int) serializeObjectAsJSONArray:(id) object 
                            buffer:(id<JPJsonStreambufferProtocol>) streambuf
                          encoding:(JPUnicodeEncoding) encoding
                           options:(JPJsonWriterOptions) options
                             level:(NSUInteger) level;




/**
 Serialize an object which shall be represented as a JSON Object. This method already 
 implements the boiler plate code necessary to serialize an object as a JSON Array.
 
 @param object The "dictionary like" object which shall be serialized
 
 @param streambuf A stream buffer into which the characters will be written.
 
 @param encoding The output encoding of the character sequence for the serialized 
 object.
 
 @param options Options that tailor the format of the output.
 
 @param level The current level of the hierarchy of the object structur. This is 
 used only when additional format characters shall be inserted into the JSON text,
 for example when "pretty printing".
  
 @return TODO
 
 Parmeter `object` shall respond to message `count` and message objectForKey:
 and shall implement the protocol NSFastEnumeration. 
 */
+ (int) serializeObjectAsJSONObject:(id) object 
                            buffer:(id<JPJsonStreambufferProtocol>) streambuf
                          encoding:(JPUnicodeEncoding) encoding
                           options:(JPJsonWriterOptions) options
                             level:(NSUInteger) level;


@end


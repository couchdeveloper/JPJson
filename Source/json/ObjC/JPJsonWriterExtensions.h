//
//  JPJsonWriterExtensions.h
//  
//
//  Created by Andreas Grosam on 5/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>



/**
 JPJsonStreambufferProtocol defines the interface for a stream-buffer:
 
 Custom classes which map to a JSON primitive element will use the stream-buffer 
 in their implementation of method JPJson_serializeTo:encoding:options:level: in
 order to serialize itself into the stream. (Custom classes for JSON __containers__, 
 like array and object will instead use the predefined class methods 
 `serializeObjectAsJSONArray` and `serializeObjectAsJSONObject`).
 
 Caution: Preliminary. At a later version this protocol may change, supporting
 a higher level abstraction of "stream" instead of "buffer".
 */
@protocol JPJsonStreambufferProtocol <NSObject>



/**
 Flush any pending bytes from internal buffer to the output device.

 @return Returns `YES` on success, otherwise `NO` indicating an error.
 */

- (BOOL) flush;


/**
 Write a sequence of bytes.
 
 Write `length` bytes from the array of bytes pointed to by `buffer` into the
 stream-buffer. This may block the current thread until after the requested
 number of bytes have been written.
 
 @param buffer A pointer to the sequence of bytes to be written.
 
 @param length The number of bytes to write.
 
 @return Returns `YES` on success, otherwise `NO` indicating an error.
 */

- (BOOL) write:(const void*)buffer length:(int)length;



/**
 Close a stream-buffer.
 
 Write any remaining bytes to the output device and close it.

 
 @return Returns `YES` on success, otherwise `NO` indicating an error.
 */
- (BOOL) close;


/**
 @return Returns the error of the last operation, `nil` if there was success.
 */

@property (nonatomic, readonly) NSError* error;

@end






/**
 The `JPJsonSerializableProtocol` must be implemented for *custom classes*,
 which shall be serialized into a JSON document. By default, only the 
 Objective-C classes specified in the mapping can be used to be serialized as
 JSON. 

 When implementing the `JPJsonSerializableProtocol` for a class, the objects
 will be serialized into their corresponding JSON element - provided the 
 implementation meets all requirements. For example, a `NSDate` object could be 
 serialized as a JSON String, while formatting the date into an appropriate 
 date string, or an `NSImage` could be base64 encoded and then serialized as a 
 JSON string, too.
*/
@protocol JPJsonSerializableProtocol <NSObject>


/**
 An implementation shall serialize itself as a character sequence and write the 
 characters into the stream-buffer _streambuf_.
 
 @param streambuf A stream-buffer into which the characters will be written.
 
 @param encoding The output encoding of the character sequence for the serialized 
 object.
 
 @param options Options that tailor the format of the output.
 
 @param level The current level of the hierarchy of the object structure. This is
 used only when additional format characters shall be inserted into the JSON text,
 for example when "pretty printing".
 
 @return Returns zero on success, otherwise an integer value indicating the error.
 
 The character sequence shall be a valid JSON element according RFC 4627, including 
 the appropriate syntax elements.
 
*/ 
- (int) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>) streambuf
                  encoding:(JPUnicodeEncoding)encoding
                   options:(JPJsonWriterOptions)options
                     level:(int)level;

@end




/**
 JPJsonWriter (Extension) defines some additional methods which help in serializing
 objects.
 
 */
@interface JPJsonWriter (Extension)

//
///**
// 
// Returns the default number formatter for `NSDecimalNumber`
// 
// */
//+ NSNumberFormatter* defaultDecimalNumberFormatter;
//


/**
 Serialize an object which shall be represented as a JSON Array. This method already 
 implements the boiler plate code necessary to serialize an object as a JSON Array.
 
 @param object The "array like" object which shall be serialized
 
 @param streambuf A stream-buffer into which the characters will be written.
 
 @param encoding The output encoding of the character sequence for the serialized 
 object.
 
 @param options Options that tailor the format of the output.
 
 @param level The current level of the hierarchy of the object structure. This is
 used only when additional format characters shall be inserted into the JSON text,
 for example when "pretty printing".
 
 @return Returns 0 on success, otherwise -1.
 
 Parmeter `object` shall respond to message `count` and shall implement the
 protocol NSFastEnumeration. 
 */
+ (int) serializeObjectAsJSONArray:(id) object 
                            buffer:(id<JPJsonStreambufferProtocol>) streambuf
                          encoding:(JPUnicodeEncoding) encoding
                           options:(JPJsonWriterOptions) options
                             level:(int) level;




/**
 Serialize an object which shall be represented as a JSON Object. This method already 
 implements the boiler plate code necessary to serialize an object as a JSON Array.
 
 @param object The "dictionary like" object which shall be serialized
 
 @param streambuf A stream-buffer into which the characters will be written.
 
 @param encoding The output encoding of the character sequence for the serialized 
 object.
 
 @param options Options that tailor the format of the output.
 
 @param level The current level of the hierarchy of the object structure. This is
 used only when additional format characters shall be inserted into the JSON text,
 for example when "pretty printing".
  
 @return Returns zero on success, otherwise -1.
 
 Parameter `object` shall respond to message `count` and message objectForKey:
 and shall implement the protocol NSFastEnumeration. 
 */
+ (int) serializeObjectAsJSONObject:(id) object 
                            buffer:(id<JPJsonStreambufferProtocol>) streambuf
                          encoding:(JPUnicodeEncoding) encoding
                           options:(JPJsonWriterOptions) options
                             level:(int) level;


@end



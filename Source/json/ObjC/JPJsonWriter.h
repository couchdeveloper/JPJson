//
//  JPJsonWriter.h
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

#ifndef JSON_OBJC_JP_JSON_WRITER_H
#define JSON_OBJC_JP_JSON_WRITER_H


#import "JPJsonCommon.h"
#import <Foundation/Foundation.h>


/**
 @enum JPJsonWriterOptions
 
 GroupName
 
 JPJsonWriter Options
 */
typedef NS_OPTIONS(NSUInteger, JPJsonWriterOptions)  {
    JPJsonWriterPrettyPrint =               1UL << 0,    /** Output will be more readable */
    JPJsonWriterSortKeys =                  1UL << 1,    /** Keys in JSON Objects will be sorted */
    JPJsonWriterEscapeUnicodeCharacters =   1UL << 2,    /** Non ASCII characters will be Unicode escaped */
    JPJsonWriterWriteBOM =                  1UL << 3,    /** A BOM is prepended */
    JPJsonWriterEscapeSolidus =             1UL << 4     /** The Escape Solidus will be escaped */
};


/**
 A JPJsonWriter class will be used to serialize a Foundation object into
 a JSON text.
 
 
 The following JPJsonWriter Options can be set:
 
 ### JPJsonWriter Options ###

 - `JPJsonWriterPrettyPrint`
  
    If this option is set, a more conveniently readable output is generated 
    through inserting tabs, new lines and indents.


 -  `JPJsonWriterSortKeys`

    If this option is set, JSON objects will be sorted by key.


 -  `JPJsonWriterEscapeUnicodeCharacters`

    If this option is set, the generator escapes Unicode characters
    in JSON strings if they cannot be represented in one code unit. This
    option should only be used when encoding in UTF-8. If this option is set,
    the output effectively becomes ASCII compatible if encoded in UTF-8.
    (Feature not implemented in this version)


- `JPJsonWriterWriteBOM`

    If this option is set, the generator prefixes the output with a BOM
    according the current Unicode encoding.
 
 
- `JPJsonWriterEscapeSolidus`

    If this option is set, the generator will escape the "solidus" (`/`)
    character.
 
*/
@interface JPJsonWriter : NSObject 


// Synopsis:
//
// + (NSData*)dataWithObject:(id)object encoding:(JPUnicodeEncoding)encoding 
//  options:(JPJsonWriterOptions)options error:(NSError**)error


/**
 Serializes the Foundation object with the specified Unicode encoding into a
 NSData object and returns it.

 @param object  A hierarchy of Foundation objects which all implement the
 `JPJsonSerializableProtocol`. The root object shall be a either a `NSDictionary`, 
 `NSArray` or a corresponding subclass or any other object which implements 
 `JPJsonSerializableProtocol and poses as a dictionary or array.
 
 @param encoding One of the constants defined in enum `JPUnicodeEncoding` which 
 specifies the Unicode encoding scheme for the generated output.
 
 *Warning:* Currently, only UTF-8 encoding is supported.

 @param options A bit mask specifying options for serializing. For possible 
 values see "JPJsonWriterOptions".

 @param error A pointer to a `NSError` object. If this is not `NULL`, and if 
 an error occurred during serializing the parameter _error_ contains an `NSError`
 object describing the issue.


 @return A NSData object representing the JSON in parameter _object_ as text,
 or `nil` if an error occurred.
*/

+ (NSData*)dataWithObject:(id)object 
                 encoding:(JPUnicodeEncoding)encoding
                  options:(JPJsonWriterOptions)options 
                    error:(__autoreleasing NSError**)error;


/**
 Serializes the Foundation object as UTF-8 Unicode encoded string into a
 NSData object and returns it.
 
 @param object  A hierarchy of Foundation objects which all implement the
 `JPJsonSerializableProtocol`. The root object shall be a either a `NSDictionary`,
 `NSArray` or a corresponding subclass or any other object which implements
 `JPJsonSerializableProtocol and poses as a dictionary or array.
 
 
 @param options A bit mask specifying options for serializing. For possible
 values see "JPJsonWriterOptions".
 
 @param error A pointer to a `NSError` object. If this is not `NULL`, and if
 an error occurred during serializing the parameter _error_ contains an `NSError`
 object describing the issue.
 
 
 @return A NSData object representing the JSON in parameter _object_ as text,
 or `nil` if an error occurred.
 */

+ (NSData*)dataWithObject:(id)object
                  options:(JPJsonWriterOptions)options
                    error:(__autoreleasing NSError**)error;



/**
 Serializes the Foundation object with the specified Unicode
 encoding and options.
 
 @param object  A hierarchy of Foundation objects which all implement the
 `JPJsonSerializableProtocol`. The root object shall be a either a `NSDictionary`,
 `NSArray` or a corresponding subclass or any other object which implements
 `JPJsonSerializableProtocol and poses as a dictionary or array.

 @param stream An output stream. The stream should be unopened.
 
 @param encoding One of the constants defined in enum `JPUnicodeEncoding` which
 specifies the Unicode encoding scheme for the generated output.
 
 *Warning:* Currently, only UTF-8 encoding is supported.
 
 @param options A bit mask specifying options for serializing. For possible
 values see "JPJsonWriterOptions".
 
 @param error A pointer to a `NSError` object. If this is not `NULL`, and if
 an error occurred during serializing the parameter _error_ contains an `NSError`
 object describing the issue.
 
 
 @return The number of bytes written into the stream, or zero if an error occurred.
 
 
 */

+ (NSUInteger) serializeObject:(id)object
                      toStream:(NSOutputStream*)stream
                      encoding:(JPUnicodeEncoding)encoding
                       options:(JPJsonWriterOptions)options
                         error:(__autoreleasing NSError**)error;



/**
 Serializes the Foundation object as UTF-8 Unicode encoded string into the stream
 and returns the number of bytes written.
 
 @param object  A hierarchy of Foundation objects which all implement the
 `JPJsonSerializableProtocol`. The root object shall be a either a `NSDictionary`,
 `NSArray` or a corresponding subclass or any other object which implements
 `JPJsonSerializableProtocol and poses as a dictionary or array.
 
 @param stream An output stream. The stream must be opened. On return, the 
 stream is closed.
 
 @param options A bit mask specifying options for serializing. For possible
 values see "JPJsonWriterOptions".
 
 @param error A pointer to a `NSError` object. If this is not `NULL`, and if
 an error occurred during serializing the parameter _error_ contains an `NSError`
 object describing the issue.
 
 
 @return The number of bytes written into the stream, or zero if an error occurred.
 
 
 */

+ (NSUInteger) serializeObject:(id)object
                      toStream:(NSOutputStream*)stream
                       options:(JPJsonWriterOptions)options
                         error:(__autoreleasing NSError**)error;



@end


#endif
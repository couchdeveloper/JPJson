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


enum  {
    JPJsonWriterPrettyPrint =               1UL << 0,
    JPJsonWriterSortKeys =                  1UL << 1,
    JPJsonWriterEscapeUnicodeCharacters =   1UL << 2,
    JPJsonWriterWriteBOM =                  1UL << 3,
    JPJsonWriterEscapeSolidus =             1UL << 4
};
typedef NSUInteger JPJsonWriterOptions;





/**
 A JPJsonWriter class will be used to serialize a Foundation object into
 a JSON text.
 
 
 ### JPJsonWriter Options ###

 - `JPJsonWriterPrettyPrint`
  
    If this option is set, a more conveniently readable output is generated 
    through inserting tabs, new lines and indents.


 -  `JPJsonWriterSortKeys`

    If this option is set, JSON objects will be sorted by key.


 -  `JPJsonWriterEscapeUnicodeCharacters`

    If this option is set, the generator escapes Unicode chararacters 
    in JSON strings if they cannot be represented in one code unit. This
    option should only be used when encoding in UTF-8. If this option is set,
    the output effectively becomes ASCII compatible if encoded in UTF-8.
    (Feature not implemented in this version)


- `JPJsonWriterWriteBOM`

    If this option is set, the generator prefixes the output with a BOM
    according the current Unicode encoding.
*/  




@interface JPJsonWriter : NSObject 


// Synopsis:
//
// + (NSData*)dataWithObject:(id)object encoding:(JPUnicodeEncoding)encoding 
//  options:(JPJsonWriterOptions)options error:(NSError**)error


/**
 Serializes the Foundation object with the specified Unicode
 encoding and options.

 @param object  A `NSDictionary` or an `NSArray` or a corresponding mutable 
 subclass which contains a representation of a JSON document.
 
 @param encoding One of the constants defined in enum `JPUnicodeEncoding` which 
 specifies the Unicode encoding scheme for the generated output.
 
 *Warning:* Currently, only UTF-8 encoding is supported.

 @param options A bit mask specifying options for serializing. For possible 
 values see "JPJsonWriterOptions".

 @param error A pointer to a `NSError` object. If this is not `NULL`, and if 
 an error occured during serializing the parameter error contains an `NSError` 
 object describing the issue.


 @return A NSData object representing the JSON in parameter 'object' as text, 
 or `nil` if an error occured.
*/

+ (NSData*)dataWithObject:(id)object 
                 encoding:(JPUnicodeEncoding)encoding
                  options:(JPJsonWriterOptions)options 
                    error:(NSError**)error;


@end


#endif
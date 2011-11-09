//
//  NSData+JPJsonDetectEncoding.h
//  
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
#ifndef JSON_OBJC_JP_JSON_NSDATA_JPJSONDETECTENCODING_H
#define JSON_OBJC_JP_JSON_NSDATA_JPJSONDETECTENCODING_H



#import <Foundation/Foundation.h>

/**
 NSData Category for JPJson 
 */

@interface NSData (JPJsonDetectEncoding)


/**
 Returns a NSStringEncoding constant corresponding to one of the Unicode 
 encoding schemes.
 
 The receiver's content should contain a JSON text. If the Unicode encoding 
 could not be detected, returns -1.
*/ 
-(NSStringEncoding) jpj_detectUnicodeNSStringEncoding;

@end


#endif // JSON_OBJC_JP_JSON_NSDATA_JPJSONDETECTENCODING_H
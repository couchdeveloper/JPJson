//
//  JPSemanticActions.h
//
//  Created by Andreas Grosam on 7/1/11.
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

#ifndef JSON_OBJC_JP_SEMANTIC_ACTIONS_H
#define JSON_OBJC_JP_SEMANTIC_ACTIONS_H


#import "JPSemanticActionsBase.h"
#import "JPJsonCommon.h"
#include <dispatch/dispatch.h>
#import <Foundation/Foundation.h>




/** Number generator options */
enum  {
    JPSemanticActionsNumberGeneratorGenerateAuto = 0,
    JPSemanticActionsNumberGeneratorGenerateNSDecimalNumber =  1,
    JPSemanticActionsNumberGeneratorGenerateNSString  = 2,
};
typedef NSUInteger JPSemanticActionsNumberGeneratorOption;






/** 
 A `JPSemanticActions` class is a delegate of the _internal json parser_ which 
 implements a set of semantic actions in order to generate a JSON representation 
 of Foundation objects from a JSON document.
  
 
 The conversion and mapping of JSON values will be done as
 follows:
 
 - JSON Object      maps to a `NSDictionary`/`NSMutableDictionary`.
 - JSON Array       maps to a `NSArray`/`NSMutableArray`.
 - JSON String      maps to a `NSString`.
 - JSON Number      maps to a `NSDecimalNumber`/`NSNumber`
 - JSON Boolean     maps to a `NSNumber` initialized with a BOOL. 
 - JSON Null        maps to a `NSNull`.

 
 ### I Detailed Mapping of an Integral JSON Number ###

 1. If the number of digits of a JSON *integer number* is equal or
    smaller than the maximal number of digits (in decimal base) that 
    can be represented by a `signed int` without overflow, then a 
    `NSNumber` with underlaying type `signed int` will be generated.

 2. Otherwise, if the number of digits of a JSON *integer number* is
    equal or smaller than the maximal number of digits (in decimal
    base) that can be represented by a `signed long`  without overflow,
    then a `NSNumber` with an underlaying type `signed long` will be 
    generated.

 3. Otherwise, if the number of digits of the JSON *integer number* is
    equal or smaller than the maximal number of digits (in decimal
    base) that can be represented by a `signed long long` without
    overflow, then a `NSNumber` with an underlaying type `signed long long` 
    will be generated.

 4. Otherwise, if the number of digits of the JSON *integer number* is
    equal or smaller than the maximal number of digits (in decimal
    base) that can be represented by a `NSDecimalNumber` without
    loosing precision, then a `NSDecimalNumber` will be generated.

 5. Otherwise, a `NSDecimalNumber` will be generated, and if this is
    successful, a warning will be logged to the error console to 
    indicate the possibly loss of precision while converting a
    JSON Number to a `NSDecimalNumber`.

 6. Otherwise, the conversion fails, and the parser stopps parsing
    with a corresponding runtime error.
    

 ### II Detailed Mapping of a Decimal JSON Number ###
 
 A decimal number is a number with a decimal point but no exponent.
 
 1. If the number of digits of the decimal number is equal or smaller than 
    the maximal number of digits (in decimal base) that can be represented by 
    a `double`, then a `NSNumber` with underlaying type `double` will be 
    generated. 

 2. Otherwise, `NSDecimalNumber` will be generated. If the `NSDecimalNumber`
    cannot represent the decimal value with the same precision as in the
    JSON number, an additional warning will be logged.
    If the decimal value cannot represent the JSON number because it is out of
    range, a range error will be signaled.
 
 
 
 ### II Detailed Mapping of a Scientific JSON Number ###
 
 A number in scientific form has an optional decimal point and an exponent.

 1. A JSON number in scientific format will allways be converted to a NSNumber
    with an underlaying `double` type. If the resulting value is out of range, 
    a range error will be signaled.
 

 @warning *Note:* -objCType for `NSDecimalNumber` returns "d".

 
*/



@interface JPSemanticActions : JPSemanticActionsBase 

/** @name Initialization */

/**
 *Designated Initializer*

 Initializes a `JPSemanticActions` object with the specified dispatch queue where
 handler blocks will be dispatched.
 Parameter handlerQueue may be NULL, in which case no handlers will be
 executed.
 
 @param handlerQueue A dispatch queue or NULL. The receicer will retain the
 dispatch queue.
*/ 
- (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue;

// Initialzes a `JPSemanticActions` object with a private default dispatch queue  
// where handler blocks will be dispatched. The dispatch queue is a serial 
// dispatch queue.
//- (id) init;


/** @name Configuration  */
/** 
 Configures a `JPSemanticActions` instance with the specified options.
 
 @param options A set of ored flags from constants of enum `JPJsonParserOptions`.
 
 This method will be invoked internally from JPJsonParser's convenience methods. 
 The receiver will set only those options which are "understood" by the 
 receiver. "Unknown" flags will be ignored and may be used in subclasses. A 
 subclass shall invoke super.
 
 For possible values for options see <JPJsonParserOptions>.
 */ 
- (void) configureWithOptions:(JPJsonParserOptions)options;



/** @name Properties */

/**
 Sets or returns the option "checkForDuplicateKey".
 
 If this option is set, the receiver checks whether a key for a JSON object  
 already exists. If it exists, a "duplicate key error" will be logged to the  
 console and error state will be set.
 
 Default: NO
*/ 
@property (nonatomic, assign) BOOL checkForDuplicateKey;


/**
 Sets or returns the option "keepStringCacheOnClear".
 
 If this option is set, the receiver does not clear the internal string cache 
 when it receives the clear message.

 Default: NO
*/
@property (nonatomic, assign) BOOL keepStringCacheOnClear;

/**
 Sets or returns the option "cacheDataStrings".
 
 If this option is set, the receiver caches data strings in addition to 
 key strings.

 Default: NO 
 */
@property (nonatomic, assign) BOOL cacheDataStrings;


/**
 Sets or returns the option "createMutableContainers".

 If enabled, the receiver creates its representation using mutable Foundation 
 arrays and mutable Foundation dictionaries.
 
 Default: NO
 */
@property (nonatomic, assign) BOOL createMutableContainers;


/**
 Sets or returns the option "numberGeneratorOption".
  
 Default: JPSemanticActionsNumberGeneratorGenerateAuto
 */
@property (nonatomic, assign) JPSemanticActionsNumberGeneratorOption numberGeneratorOption;


/**
 Returns the input encoding of the current or last parsed JSON document.
 */
@property (nonatomic, readonly) NSString* inputEncoding;


/**
 Returns true if the current input has a BOM.
 */
@property (nonatomic, readonly, assign) BOOL hasBOM;


//@property (nonatomic, assign) NSTimeInterval timeoutInterval;


/** @name Retrieving the Result of the Semantic Actions */
/**
 Returns the result of the receiver, which is a representation of the
 parsed JSON text as Foundation objects, or `nil` in case of an error.
 */
- (id) result;


/** @name Clearing Internal State */
/**
 Clears internal cached data.
 */
- (void) clear;

/** @name Canceling a Running Parser */
/**
 Sets a cancelation request which the underlaying parser is supposed to
 read which in turn causes it to cancel.
 */
- (void) cancel;



@end


#endif


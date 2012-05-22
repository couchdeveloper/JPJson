//
//  JPSemanticActionsBase.h
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

#import "JPSemanticActionsProtocol.h"
#import "JPJsonCommon.h"
#import <Foundation/Foundation.h>


/** Unicode Noncharacter Handling */
enum {
    JPSemanticActionsSignalErrorOnUnicodeNoncharacter,
    JPSemanticActionsSubstituteUnicodeNoncharacter, 
    JPSemanticActionsSkipUnicodeNoncharacters
};
typedef NSUInteger JPSemanticActionsUnicodeNoncharacterHandling;

/** Log behavior */
enum {
    JPSemanticActionsLogLevelDebug,
    JPSemanticActionsLogLevelWarning,
    JPSemanticActionsLogLevelError,
    JPSemanticActionsLogLevelNone
};
typedef NSUInteger JPSemanticActionsLogLevel;


/** non_conformance_flags (can be ored) */
enum {
    JPSemanticActionsAllowNone                     = 0,
    JPSemanticActionsAllowComments                 = 1UL << 0,              
    JPSemanticActionsAllowControlCharacters        = 1UL << 1,     
    JPSemanticActionsAllowLeadingPlusInNumbers     = 1UL << 2,
    JPSemanticActionsAllowLeadingZerosInIntegers   = 1UL << 3
};
typedef NSUInteger JPSemanticActionsNonConformanceOptions;



/** Function pointer type definitiond for handler blocks */
typedef void (^JPSemanticActions_StartJsonHandlerBlockType)(void);
typedef void (^JPSemanticActions_EndJsonHandlerBlockType)(id);
typedef void (^JPSemanticActions_CompletionHandlerBlockType)(void);
typedef void (^JPSemanticActions_ErrorHandlerBlockType)(NSError*);


/**
 Abstract Base Class for concrete Semantic Actions classes.
 
 
 A __Semantic Actions Object__ acts as an internal delegate of a JSON parser. 
 It receives _parse events_ sent from the parser and handles them in corresponding 
 _semantic actions_. The class conforms to the protocol `JPSemanticActionsProtocol` 
 and provides a default implementation for the following optional methods declared 
 in `JPSemanticActionsProtocol`:
 
 - `-parserFoundJsonBegin`
 - `-parserFoundJsonEnd`
 - `-parserFinished`
 - `-parserDetectedError`
   
 `JPSemanticActionsBase` is meant to be subclassed. Subclasses may override the
 methods mentioned above and may define the additional methods declared in
 protocol `JPSemanticActionsProtocol`. 
 
 There are two prominent subclasses: `JPRepresentationGenerator` and
 `JPStreamSemanticActions`. While `JPRepresentationGenerator` shall be used
 as is, `JPStreamSemanticActions` may be subclassed or alternatively used with
 a delegate.
 
 The delegate methods will be invoked when the json parser successfully parsed 
 a certain rule, that is when the parser detects the various JSON elements, or 
 the start and the end of the document. For more information see <JPSemanticActionsProtocol>.
 
 
 
 ### Using a Semantic Actions object ###
 
 Each parser, that is an instance of <JPJsonParser> or and instance of 
 <JPAsyncJsonParser>, needs to be associated to a _Semantic Actions_ object. If 
 none is specified when a parser is created, the parser itself creates a default 
 one, which is a `JPRepresentationGenerator`. 
 
 *Info:*  A parser sends _parse events_ to the semantic actions object as they 
 appear in the JSON text. The task of a semantic actions object is to handle 
 these events appropriately.
 
 All semantic actions classes shall inherit from <JPSemanticActionsBase>. 
 `JPSemanticActionsBase` implements all required common aspects for a semantic 
 actions object.
 
 Semantic actions classes shall conform to the protocol <JPSemanticActionsProtocol>. 
 
 There are two "built-in" semantic actions classes inherting from `JPSemanticActionsBase`:
 
 - <JPRepresentationGenerator> and
 - <JPStreamSemanticActions>
 

 
 ### Setting Up Handler Blocks ###

 An `JPSemanticActions` class provides four Blocks (callbacks) which handle principal 
 events:
 
 - Notifying the start of a JSON text
 - Notifying the end of a JSON text
 - Notifying the completion of a semantic actions task
 - Notifying a possible error
 

 These principal handlers are common to all semantic actions classes. They will
 be implemented using _Blocks_ and the class provides properties to access them.
 
 For a description of each, see section _Handlers and Dispatch Queue_.
 
 
 
 */


@interface JPSemanticActionsBase : NSObject <JPSemanticActionsProtocol>

// -----------------------------------------------------------------------------
/** @name Initialization */
// -----------------------------------------------------------------------------

/**
 *Designated Initializer*

 Initializes a `JPSemanticActionsBase` instance with the specified dispatch queue.
 If parameter `handlerQueue` equals `nil` no dispatch queue will be used and no
 handler blocks will be scheduled.
 
 @param handlerQueue The dispatch queue where handler blocks will be scheduled,
 or `nil`, in which case no handlers will be executed.
*/ 
- (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue;

/** 
 Initializes a `JPSemanticActions` instance with a private default dispatch queue.

 The receiver creates a serial dispatch queue where handler blocks will be
 scheduled.
*/ 
- (id) init;


/** @name Configuration  */
/** 
 Configures a `JPSemanticActionsBase` instance with the specified options.
 
 @param options A set of ored flags from constants of enum `JPJsonParserOptions`.
 
 This method will be invoked internally from JPJsonParser's convenience methods. 
 The receiver will set only those options which are "understood" by the 
 receiver. "Unknown" flags will be ignored and may be used in subclasses. A 
 subclass shall invoke super.
 
 For possible values for options see <JPJsonParserOptions>.
 */ 
- (void) configureWithOptions:(JPJsonParserOptions)options;


// -----------------------------------------------------------------------------
/** @name Parse Options  */
// -----------------------------------------------------------------------------

/**
 Sets or returns the parse option "ignoreSpuriousTrailingBytes".
 
 If this options is set, the parser will not treat any additional characters 
 which may occure after the last significant character of a valid JSON document 
 as an error. Otherwise, any code units which can not be interpreted as 
 white-space Unicode characters, except `Unicode NULL` (U+0000) and an `EOF`,
 following a JSON document will cause an error. `Unicode NULL` and `EOF` will issue 
 a warning.

 Default: `NO` 
*/  
@property (nonatomic, assign) BOOL ignoreSpuriousTrailingBytes;


/**
 Sets or returns the parse option "parseMultipleDocuments".
 
 If this option is set, the parser parses more than one document from the input 
 until it receives an `EOF` in the input data. 

 This flag becomes especially useful if the parser will be fed  from a "stream" 
 of JSON documents - possibly from input downloaded in a single network connection 
 supporting a "Streaming API".
 
 Default: `NO`
*/ 
@property (nonatomic, assign) BOOL parseMultipleDocuments;


/**
 Sets or returns the parse option "parseMultipleDocumentsAsynchronously".
 
 If this option is set, the parser invokes the `jsonObjectHandlerBlock`
 asynchronously and immediately processes the next JSON document within the 
 input data. The json container is then retained in the dispatch queue till 
 it is processed by the client. This may tie up a lot of system resources if 
 the client processess frees the JSON containers slowly.
 If the flag is `NO`, the parser's thread is blocked until the handler routine 
 returns.
 
 It is recommended to leave this flag disabled in systems where system resources 
 are scarce or if the data input possibly contains many and large JSON documents. 
 When downloading large data, this helps throtteling the consumption of system 
 resources by the underlaying network layer.

 Default: `NO`
*/ 
@property (nonatomic, assign) BOOL parseMultipleDocumentsAsynchronously;


/**
 Sets or returns the log level of the semantic actions instance and the
 underlaying json parser. 
 
 Possible values are constants defined in enumeration `JPSemanticActionsLogLevel`:
 
 - `JPSemanticActionsLogLevelDebug`
    Verbose logging.
 - `JPSemanticActionsLogLevelWarning`
    Logs warning messages as well as errors.
 - `JPSemanticActionsLogLevelError`
    Logs only error messages.
 - `JPSemanticActionsLogLevelNone`
    Does not log anything.
 
 Default: `JPSemanticActionsLogLevelError`
*/ 
@property (nonatomic, assign) JPSemanticActionsLogLevel logLevel;


/**
 Sets or returns the policy for handling Unicode noncharacters of the receiver 
 and the underlaying json parser. 
 
 Possible values are constants defined in enumeration `JPSemanticActionsUnicodeNoncharacterHandling`:
 
 - `JPSemanticActionsSignalErrorOnUnicodeNoncharacter`
 - `JPSemanticActionsSubstituteUnicodeNoncharacter`
 - `JPSemanticActionsSkipUnicodeNoncharacters`

 Default: `JPSemanticActionsSignalErrorOnUnicodeNoncharacter`
 */ 
@property (nonatomic, assign) JPSemanticActionsUnicodeNoncharacterHandling  unicodeNoncharacterHandling;


/**
 Sets or returns the strictness of the underlaying json parser.
 
 Possible values are ored flags which are defined in enumeration `JPSemanticActionsNonConformanceOptions`:
 
 - `JPSemanticActionsAllowNone`
 - `JPSemanticActionsAllowComments`
 - `JPSemanticActionsAllowControlCharacters`
 - `JPSemanticActionsAllowLeadingPlusInNumbers`
 - `JPSemanticActionsAllowLeadingZerosInIntegers`
 
 Default: `JPSemanticActionsAllowNone`
*/
@property (nonatomic, assign) JPSemanticActionsNonConformanceOptions nonConformanceOptions;



// -----------------------------------------------------------------------------
/** @name Set and Retrieve Error State */
// -----------------------------------------------------------------------------

/**
 Set error and description.
 
 Allows a client, that is a JPJsonParser and JPAsyncJsonParser instance, to
 set an error code and an associated description which is safed by the 
 semantic actions instance. This is useful in cases where the clients itself 
 detect a error and want the semantic actions instance to safe the error 
 information on behalf of the clients since they lack an interface which let
 users access the error state. 
 
 JPJsonParser and JPAsyncJsonParser set the error state when they detect
 unexpected errors not cought by the semantic actions instance, or when 
 they detect errors during determining the input encoding.
 
 @param error An integer indicating the error code. error should be negative.
 
 @param description A string describing the error.
 */
- (void) setErrorCode:(int)error description:(NSString*)description;


/**  
 Returns the error object from the receiver or `nil` if there is no error.
 */ 
@property (atomic, readonly) NSError* error;



// -----------------------------------------------------------------------------
/** @name Handlers and Dispatch Queue */
// -----------------------------------------------------------------------------


/**
 Returns or sets the dispatch queue where handler blocks will be scheduled.
 
 The dispatch queue will be retained - with function `dispatch_retain()`.
 If parameter queue is set to `NULL`, handler blocks will not be called.
 The disaptch queue must only be modified before a parser associated to 
 the receiver is not yet started. 
 
 @warning Setting a dispatch queue while a parser is started, may result in a crash.
 
 The value shall be a concurrent or a serial dispatch queue or `NULL`.
*/ 
@property (nonatomic) dispatch_queue_t handlerDispatchQueue;



// Synopsis: void (^startJsonHandler)(void)
/**
 Sets or returns the startJson handler block of type `void (^)(void)`.

 This handler block will be called when the json parser has found the start of
 a JSON root element within the input text. 
 If property parseMultipleDocuments equals YES this block will be called 
 each time a the parsers detects the start of a JSON root element.

 Note, that this handler will only be called after the json parser actually has
 been started, which only happens after determining the input encoding, which
 possibly can fail.

 startJsonHandlerBlock can be set to `nil`.
*/ 
@property (copy) JPSemanticActions_StartJsonHandlerBlockType 
startJsonHandlerBlock;


// Synopsis: void (^endJsonHandler)(id result)
/**
 Sets or returns the endJson handler block of type `void (^)(id result)`.
 
 The endJsonHandlerBlock is called when the parser has successully parsed a 
 JSON text and the semantic actions instance was able to create a result (e.g.
 a JSON representation created upon Foundation objects) from this JSON text.
 The result will be passed in parameter result.
 If property parseMultipleDocuments equals YES this block will be called for
 each result which will be created by the semantic actions instance.

 If  property parseMultipleDocumentsAsynchronously returns `NO` (the default) the 
 parser's thread is blocked until the handler finishes. This is a measurement 
 to balance the process of generating result objects (e.g. JSON representations
 as Foundation objects) - which may tie up a large amount of system resources
 - with the actual processing of this result - which may free these resources 
 again.
 Otherwise, if parseMultipleDocumentsAsynchronously returns YES, the parser 
 will immediately continue to parse the input source on its own thread. Be 
 carefully, when considering parseMultipleDocumentsAsynchronously set to YES.

 endJsonHandlerBlock may be set to `nil`.
*/ 
@property (copy) JPSemanticActions_EndJsonHandlerBlockType 
endJsonHandlerBlock;


// Synopsis: void (^completionHandler)(void)
/**
 Sets or returns the completion handler block of type `void (^)(void)`.
 
 If it's not `NULL`, the completionBlock is called when the json parser finished 
 parsing, regardless if there has been a parse error signaled via the error 
 handler block during parsing. The start of the json parser was previously 
 signaled by calling the startJsonHandlerBlock block.
 Note that the error handler may be possibly called _before_ the json parser
 even started parsing, e.g. when JPJsonParser and JPAsyncJsonParser detect an
 error during determining the input encoding. In this case, the json parser
 will not be started, and consequently, startJsonHandlerBlock and
 completionHandlerBlock will not be called.
 completionBlock may be set to `NULL`.
*/ 
@property (copy)  JPSemanticActions_CompletionHandlerBlockType 
completionHandlerBlock;



// Synopsis: void (^errorHandler)(NSError*)
/**
 Sets or retrieves the error handler block of type `void (^)(NSError*)`.

 The errorHandlerBlock will be called when an error has been detected by the
 JPJsonParser/JPAsyncJsonParser during determing the input encoding, by the
 json parser during parsing, or by the semantic actions instance while per-
 forming a semantic action.
 errorHandlerBlock may be set to `NULL` in which case no errors will be reported
 by this means.
*/ 
@property (copy) JPSemanticActions_ErrorHandlerBlockType 
errorHandlerBlock;


// -----------------------------------------------------------------------------
/** @name Retrieving Result  */
// -----------------------------------------------------------------------------

/** 
 Returns `nil`.
 
 This method may be overriden by subclasses in order to return the result 
 of the semantic actions. 
 
 @return An object if a result is available, otherwise `nil`.
 */ 
@property (nonatomic, readonly) id result;



@end

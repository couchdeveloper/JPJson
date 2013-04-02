//
//  JPSemanticActionsBase.m
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

#if __has_feature(objc_arc) 
#error This Objective-C file shall be compiled with ARC disabled.
#endif


#import "JPSemanticActionsBase.h"
#import "JPSemanticActionsBase_private.h"


typedef JPSemanticActions_StartJsonHandlerBlockType   StartJsonHandlerBlockType;
typedef JPSemanticActions_EndJsonHandlerBlockType     EndJsonHandlerBlockType;
typedef JPSemanticActions_CompletionHandlerBlockType  CompletionHandlerBlockType;
typedef JPSemanticActions_ErrorHandlerBlockType       ErrorHandlerBlockType;




@implementation JPSemanticActionsBase
{
    dispatch_queue_t                    handlerDispatchQueue_;  
    StartJsonHandlerBlockType           startJsonHandlerBlock_;
    EndJsonHandlerBlockType             endJsonHandlerBlock_;
    CompletionHandlerBlockType          completionHandlerBlock_;
    ErrorHandlerBlockType               errorHandlerBlock_;
    
    BOOL                                parseMultipleDocumentsAsynchronously_;    
}


@synthesize startJsonHandlerBlock =     startJsonHandlerBlock_;
@synthesize endJsonHandlerBlock =       endJsonHandlerBlock_;
@synthesize completionHandlerBlock =    completionHandlerBlock_;
@synthesize errorHandlerBlock =         errorHandlerBlock_;

@synthesize parseMultipleDocumentsAsynchronously = parseMultipleDocumentsAsynchronously_;


/* This is declared in the header file "JPSemanticActionsBase_private.h":

 @interface JPSemanticActionsBase ()
 @property (readonly, nonatomic) SemanticActionsBase* imp;
 @end
 
 The property imp shall be impemented in the subclass
 */
@dynamic imp;


//
// Designated Initializer
//
- (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue
{
    self = [super init];
    if (self) {
        if (handlerQueue != NULL) {
            handlerDispatchQueue_ = handlerQueue; 
            dispatch_retain(handlerDispatchQueue_);
        }
    }    
    return self;
}


- (id) init {
    dispatch_queue_t handlerDispatchQueue = dispatch_queue_create("com.JPSemanticActions.handler_queue", NULL);
    self = [self initWithHandlerDispatchQueue:handlerDispatchQueue];
    if (handlerDispatchQueue != NULL) {
        dispatch_release(handlerDispatchQueue);
    }
    return self;
}

- (void) dealloc 
{
    if (handlerDispatchQueue_ != NULL) {
        dispatch_release(handlerDispatchQueue_);
    }
    [super dealloc];
}


#pragma mark -


// Private method
// Configures the receiver according the options set as flags in parameter 
// options.
- (void) configureWithOptions:(JPJsonParserOptions)options
{    
    self.ignoreSpuriousTrailingBytes = (options & JPJsonParserIgnoreSpuriousTrailingBytes) != 0;
    self.parseMultipleDocuments = (options & JPJsonParserParseMultipleDocuments) != 0;
    self.parseMultipleDocumentsAsynchronously = (options & JPJsonParserParseMultipleDocumentsAsynchronously) != 0;
    
    if (JPJsonParserNoncharacterHandling & options) {
        if (JPJsonParserSignalErrorOnNoncharacter & options) {
            self.unicodeNoncharacterHandling = JPSemanticActionsSignalErrorOnUnicodeNoncharacter;
        } else if (JPJsonParserSubstituteUnicodeNoncharacter & options) {
            self.unicodeNoncharacterHandling = JPSemanticActionsSubstituteUnicodeNoncharacter;
        } else if (JPJsonParserSkipUnicodeNoncharacter & options) {
            self.unicodeNoncharacterHandling = JPSemanticActionsSkipUnicodeNoncharacters;
        } 
    }
    
    if (JPJsonParserLogLevel && options) {
        if (JPJsonParserLogLevelDebug & options) {
            self.logLevel = JPSemanticActionsLogLevelDebug;
        }
        else if (JPJsonParserLogLevelWarning & options) {
            self.logLevel = JPSemanticActionsLogLevelWarning;
        }
        else if (JPJsonParserLogLevelError & options) {
            self.logLevel = JPSemanticActionsLogLevelError;
        }
        else if (JPJsonParserLogLevelNone & options) {
            self.logLevel = JPSemanticActionsLogLevelNone;
        }        
    }
    
    if (JPJsonParserNonConformanceFlags & options) {
        NSUInteger flags = 0;
        if (JPJsonParserAllowComments & options) {
            flags |= JPSemanticActionsAllowComments;
        }
        else if (JPJsonParserAllowControlCharacters & options) {
            flags |= JPSemanticActionsAllowComments;
        }
        else if (JPJsonParserAllowLeadingPlusInNumbers & options) {
            flags |= JPSemanticActionsAllowControlCharacters;
        }
        else if (JPJsonParserAllowLeadingZerosInIntegers & options) {
            flags |= JPSemanticActionsAllowLeadingZerosInIntegers;
        }
        self.nonConformanceOptions = flags;
    }
    else {
        self.nonConformanceOptions = 0;
    }
}


#pragma mark -

// Creates and returns an autoreleased NSError object from the error state of 
// the internal implementation sa, or nil if there is no error.
- (NSError*) makeError
{
    if (self.imp == 0) {
        return nil;
    }    
    SemanticActionsBase::error_t sa_error = self.imp->error();
    if (sa_error.first == 0) {
        return nil;
    }
    NSString* errStr = [[NSString alloc] initWithUTF8String:sa_error.c_str()];
    NSString* localizedErrStr = NSLocalizedString(errStr, errStr);
    [errStr release];
    NSArray* objectsArray = [[NSArray alloc] initWithObjects: localizedErrStr, nil];
    NSArray* keysArray = [[NSArray alloc] initWithObjects: NSLocalizedDescriptionKey, nil];            
    NSDictionary* userInfoDict = [[NSDictionary alloc] initWithObjects:objectsArray forKeys: keysArray];
    [objectsArray release];
    [keysArray release];        
    NSError* error = [NSError errorWithDomain:@"JPSemanticActions" code:sa_error.first userInfo:userInfoDict];
    [userInfoDict release];
    return error;
}


// property readonly error
- (NSError*) error 
{
    NSError* err = [self makeError];
    return err;
}

- (void) setErrorCode:(int)code description:(NSString*)description
{
    self.imp->error(code, [description UTF8String]);
}



// Returns the result of the semantic actions.
//@property (nonatomic, readonly) id result;
// may be overridden by subclasses
- (id) result {
    return nil;
}

#pragma mark -


// @property (nonatomic) dispatch_queue_t handlerDispatchQueue;
- (dispatch_queue_t) handlerDispatchQueue {
    return handlerDispatchQueue_;
}

- (void) setHandlerDispatchQueue:(dispatch_queue_t) queue
{
    if (queue != handlerDispatchQueue_) {
        if (handlerDispatchQueue_) {
            dispatch_release(handlerDispatchQueue_);
        }
        if (queue) {
            dispatch_retain(queue);
        }
        handlerDispatchQueue_ = queue;
    }
}


#pragma mark - JPSemanticActionsProtocol


-(void) parserFoundJsonBegin 
{
    if (self.handlerDispatchQueue == nil)
        return;
    
    if (self.startJsonHandlerBlock) {
        // Note: the result is retained by the semantic actions object. Unless
        // result is retained by the caller, it is only valid as long as the 
        // semantic actions object sa exists. dispatch_async will retain the
        // variables in the closure - hence, whoever receives the parameter
        // result, will get it retained - even though the semantic actions 
        // object has been deallocated.
        
        if (parseMultipleDocumentsAsynchronously_) {
            dispatch_async(self.handlerDispatchQueue, self.startJsonHandlerBlock);
        }
        else {
            assert (dispatch_get_current_queue() != self.handlerDispatchQueue);
            dispatch_sync(self.handlerDispatchQueue,  self.startJsonHandlerBlock);
            
        }
    }
}


- (void) parserFoundJsonEnd
{
    if (self.handlerDispatchQueue == NULL)
        return;
    
    if (self.endJsonHandlerBlock) {
        // Note: the result is retained by the semantic actions object. Unless
        // result is retained by the caller, it is only valid as long as the 
        // semantic actions object sa exists. dispatch_async will retain the
        // variables in the closure - hence, whoever receives the parameter
        // result, will get it retained - even though the semantic actions 
        // object has been deallocated.
        id result = self.imp->result();
        assert(result);
        if (parseMultipleDocumentsAsynchronously_) {
            dispatch_async(self.handlerDispatchQueue,  ^{
                self.endJsonHandlerBlock(result);
            });
        }
        else {
            assert (dispatch_get_current_queue() != self.handlerDispatchQueue);
            dispatch_sync(self.handlerDispatchQueue,  ^{
                self.endJsonHandlerBlock(result);
            });
        }
    }
}


- (void) parserFinished
{
    if (self.handlerDispatchQueue == NULL)
        return;
    
    if (self.completionHandlerBlock) {
        dispatch_async(self.handlerDispatchQueue, self.completionHandlerBlock);
    }
}


- (void) parserDetectedError
{
    if (self.handlerDispatchQueue == NULL)
        return;
    
    if (self.errorHandlerBlock) {
        NSError* error = [self error];
        assert(error);
        dispatch_async(self.handlerDispatchQueue, ^{ self.errorHandlerBlock(error); } );
    }
}



#pragma mark -

//@property (nonatomic, assign) BOOL ignoreSpuriousTrailingBytes;
- (BOOL) ignoreSpuriousTrailingBytes  {
    return (BOOL)self.imp->ignoreSpuriousTrailingBytes();
}

- (void) setIgnoreSpuriousTrailingBytes:(BOOL)value {
    self.imp->ignoreSpuriousTrailingBytes(static_cast<bool>(value));
}


// @property (nonatomic, assign) BOOL parseMultipleDocuments
- (BOOL) parseMultipleDocuments  {
    return (BOOL)self.imp->parseMultipleDocuments();
}

- (void) setParseMultipleDocuments:(BOOL)value {
    self.imp->parseMultipleDocuments(static_cast<bool>(value));
}


// @property (nonatomic, assign) JPSemanticActionsLogLevel logLevel;
- (void) setLogLevel:(JPSemanticActionsLogLevel) level {
    switch (level) {
        case JPSemanticActionsLogLevelDebug:
            self.imp->log_level(json::semanticactions::LogLevelDebug);
            break;
            
        case JPSemanticActionsLogLevelWarning:
            self.imp->log_level(json::semanticactions::LogLevelWarning);
            break;
            
        case JPSemanticActionsLogLevelError:
            self.imp->log_level(json::semanticactions::LogLevelError);
            break;
            
        case JPSemanticActionsLogLevelNone:
            self.imp->log_level(json::semanticactions::LogLevelNone);
            break;
    }
}

- (JPSemanticActionsLogLevel) logLevel {
    switch (self.imp->log_level()) {
        case json::semanticactions::LogLevelDebug:
            return JPSemanticActionsLogLevelDebug;
        case json::semanticactions::LogLevelWarning:
            return JPSemanticActionsLogLevelWarning;
        case json::semanticactions::LogLevelError:
            return JPSemanticActionsLogLevelError;
        case json::semanticactions::LogLevelNone:
            return JPSemanticActionsLogLevelNone;
    }
    assert("bad log-level"==0);
    return json::semanticactions::LogLevelWarning;
}



- (void) setUnicodeNoncharacterHandling:(JPSemanticActionsUnicodeNoncharacterHandling)opt
{
    switch (opt) {
        case JPSemanticActionsSignalErrorOnUnicodeNoncharacter:
            self.imp->unicode_noncharacter_handling(json::semanticactions::SignalErrorOnUnicodeNoncharacter);
            break;
        case JPSemanticActionsSubstituteUnicodeNoncharacter:
            self.imp->unicode_noncharacter_handling(json::semanticactions::SubstituteUnicodeNoncharacter);
            break;
        case JPSemanticActionsSkipUnicodeNoncharacters: 
            self.imp->unicode_noncharacter_handling(json::semanticactions::SkipUnicodeNoncharacters);
            break;
    }
}

- (JPSemanticActionsUnicodeNoncharacterHandling) unicodeNoncharacterHandling
{
    switch (self.imp->unicode_noncharacter_handling()) {
        case json::semanticactions::SignalErrorOnUnicodeNoncharacter:
            return JPSemanticActionsSignalErrorOnUnicodeNoncharacter;
        case json::semanticactions::SubstituteUnicodeNoncharacter:
            return JPSemanticActionsSubstituteUnicodeNoncharacter;
        case json::semanticactions::SkipUnicodeNoncharacters: 
            return JPSemanticActionsSkipUnicodeNoncharacters;
    }
    assert("bad JPSemanticActionsUnicodeNoncharacterHandling" == 0);
    return json::semanticactions::SignalErrorOnUnicodeNoncharacter;
}



- (void) setNonConformanceOptions:(JPSemanticActionsNonConformanceOptions)opts 
{
    using json::semanticactions::ExtensionOptions;
    
    ExtensionOptions flags = ExtensionOptions::AllowNone;
    if (opts & JPSemanticActionsAllowComments)
        flags |= ExtensionOptions::AllowComments;
    if (opts & JPSemanticActionsAllowControlCharacters)
        flags |= ExtensionOptions::AllowControlCharacters;
    if (opts & JPSemanticActionsAllowLeadingPlusInNumbers)
        flags |= ExtensionOptions::AllowLeadingPlusInNumbers;
    if (opts & JPSemanticActionsAllowLeadingZerosInIntegers)
        flags |= ExtensionOptions::AllowLeadingZerosInIntegers;
    
    self.imp->extensions(flags);
}

- (JPSemanticActionsNonConformanceOptions) nonConformanceOptions 
{
    using json::semanticactions::ExtensionOptions;
    
    ExtensionOptions exts = self.imp->extensions();    
    JPSemanticActionsNonConformanceOptions flags = 0;
    if (exts & ExtensionOptions::AllowComments)
        flags |= JPSemanticActionsAllowComments;
    if (exts & ExtensionOptions::AllowControlCharacters)
        flags |= JPSemanticActionsAllowControlCharacters;
    if (exts & ExtensionOptions::AllowLeadingPlusInNumbers)        
        flags |= JPSemanticActionsAllowLeadingPlusInNumbers;
    if (exts & ExtensionOptions::AllowLeadingZerosInIntegers)
        flags |= JPSemanticActionsAllowLeadingZerosInIntegers;
    
    return flags;
}




- (NSString*) description 
{
    std::stringstream ss;
    self.imp->print(ss);
    NSString* desc = [[NSString alloc] initWithUTF8String:ss.str().c_str()];
    [desc autorelease];
    return desc;
}




@end

//
//  DemoJSONRequestFuture.m
//  DemoApp
//
//  Created by Andreas Grosam on 16.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoJSONRequestFuture.h"
#import "DemoJSONRequest.h"
#include <dispatch/dispatch.h>
#import "DLog.h"
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPRepresentationGenerator.h"
#if defined (DEBUG)
#import "JPJson/JPJsonWriter.h"
#endif


typedef fetchCompletionHandler_BlockType completionHandler_block;

@interface DemoJSONRequestFuture() <DemoJSONRequestDelegate>

@property (nonatomic, strong) DemoJSONRequest* jsonRequest;

@property (nonatomic, readwrite) id value;
@property (nonatomic, readwrite) NSError* error;
@property (nonatomic, readwrite) BOOL isExecuting;   // observable property
@property (nonatomic, readwrite) BOOL isFinished;    // observable property
@property (nonatomic, readwrite) BOOL isCancelled;   // observable property


@end


@implementation DemoJSONRequestFuture  
{
    dispatch_semaphore_t    _finishedSemaphore;
    DemoJSONRequest*        _jsonRequest;
    id                      _value;
    NSError*                _error;
    completionHandler_block _completionHandler;
    
    BOOL                    _isExecuting;
    BOOL                    _isFinished;
    BOOL                    _isCancelled;
}

@synthesize jsonRequest = _jsonRequest;
@synthesize completionHandler = _completionHandler;
@synthesize value = _value;
@synthesize error = _error;
@synthesize isExecuting = _isExecuting;
@synthesize isFinished = _isFinished;
@synthesize isCancelled = _isCancelled;

// Designated Initializer
- (id)initWithJSONRequest:(DemoJSONRequest*)jsonRequest
{
    NSParameterAssert(jsonRequest.delegate == nil);
    self = [super init];
    if (self) {
        if (jsonRequest) {
            _finishedSemaphore = dispatch_semaphore_create(0);
            _jsonRequest = jsonRequest;
            _jsonRequest.delegate = self;
        }
    }
    return self;
}

- (id) initWithError:(NSError*)error {
    _error = error;
    return [self initWithJSONRequest:nil];
}

- (id) init {
    return [self initWithJSONRequest:nil];
}

- (void) dealloc {
    [_jsonRequest cancel];
    if (_finishedSemaphore) {
        dispatch_release(_finishedSemaphore);
    }
}

#pragma mark -

- (id) value {
    if (_finishedSemaphore) {
        dispatch_semaphore_wait(_finishedSemaphore,  DISPATCH_TIME_FOREVER);
        dispatch_semaphore_signal(_finishedSemaphore);
    }
    return _value;
}

- (id) error {
    if (_finishedSemaphore) {
        dispatch_semaphore_wait(_finishedSemaphore,  DISPATCH_TIME_FOREVER);
        dispatch_semaphore_signal(_finishedSemaphore);
    }
    return _error;
}

- (void) cancel {
    [self.jsonRequest cancel];
}


//@property (nonatomic, copy) fetchCompletionHandler_BlockType completionHandler;
- (void) setCompletionHandler:(completionHandler_block)completionHandler
{
    if (completionHandler != _completionHandler) {
        _completionHandler = [completionHandler copy];
    }
}


@end


#pragma mark -



@interface DemoJSONRequestFuture (DemoJSONRequestDelegate)
@end

@implementation DemoJSONRequestFuture (DemoJSONRequestDelegate)

- (void) jsonRequestDidStartConnection:(DemoJSONRequest *)request
{
    _isExecuting = YES;
    NSLog(@"jsonRequestDidStartConnection:");
}

- (void) jsonRequestDidFinishConnection:(DemoJSONRequest *)request
{
    NSLog(@"jsonRequestDidFinishConnection:");
}

- (void) jsonRequestDidStartParsingJSON:(DemoJSONRequest *)request
{
    NSLog(@"jsonRequestDidStartParsingJSON:");
}

- (void) jsonRequest:(DemoJSONRequest *)request didCreateJSONRepresentation:(id)json
{
    NSLog(@"jsonRequest:didCreateJSONRepresentation:");
    NSAssert(json != nil, @"json is nil");
#if defined (DEBUG)
    if (json) {
        NSData* jsonString = [JPJsonWriter dataWithObject:json 
                                                 encoding:JPUnicodeEncoding_UTF8
                                                  options:JPJsonWriterPrettyPrint 
                                                    error:nil];
        NSString* str = [[NSString alloc] initWithBytes:[jsonString bytes] 
                                                 length:[jsonString length] 
                                               encoding:NSUTF8StringEncoding];
        NSLog(@"\n%@", str);
    }
#endif        
    self.value = json;           
}

- (void) jsonRequestFinishedParsing:(DemoJSONRequest*)request
{
    NSLog(@"jsonRequestFinishedParsing:");
}


- (void) jsonRequestDidFinish:(DemoJSONRequest*)request
{
    NSLog(@"jsonRequestDidFinish:");
    self.jsonRequest = nil;
    self.isExecuting = NO;
    if (_completionHandler) {
        _completionHandler(_value, nil);
    }    
    dispatch_semaphore_signal(_finishedSemaphore);
}


- (void) jsonRequest:(DemoJSONRequest *)request didFailWithError:(NSError *)error
{
    NSLog(@"jsonRequest:didFailWithError: %@", error);
    self.jsonRequest = nil;
    self.isExecuting = NO;
    if (error.code == DemoJSONRequestError_OperationCanceled) {
        self.isCancelled = YES;
    }
    self.error = error;
    if (_completionHandler) {
        _completionHandler(nil, error);
    }    
    dispatch_semaphore_signal(_finishedSemaphore);
}

- (void) jsonRequest:(DemoJSONRequest *)request willSendRequestForAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    NSLog(@"jsonRequest:willSendRequestForAuthenticationChallenge: %@", challenge);
}

@end




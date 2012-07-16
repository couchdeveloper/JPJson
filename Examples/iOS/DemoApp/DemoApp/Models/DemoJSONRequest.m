//
//  DemoJSONRequest.m
//  DemoApp
//
//  Created by Andreas Grosam on 11.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoJSONRequest.h"
#import "JPJson/JPJsonParser.h"
#import "DemoNetworkIndicator.h"
#include <libkern/OSAtomic.h>
#include <dispatch/dispatch.h>


#define DEBUG_LOG DEBUG_LOGLEVEL_INFO
#import "DLog.h"

#pragma mark - 

static NSString* defaultErrorDomain = @"DemoJSONRequestError";

typedef enum DemoJSONRequestError_t DemoJSONRequestError;

static NSString* DemoJSONRequestError_ErrorMsg[] = {
    @"No Error.",
    @"Operation canceled.",
    @"Failure to create URL connection.",
    @"Authentication failed.",
    @"Operation aborted due to low memory.",
    @"Unknown error."
};

__attribute__((ns_returns_retained)) 
static NSError* makeError(NSString* domain, NSInteger code, NSString* errString) 
{    
    // setup an error object:
    NSString* msg = errString;
    if (msg == nil && (code  <= DemoJSONRequest_UnknownError && code > 0)) {
        msg = DemoJSONRequestError_ErrorMsg[code];
    }
    NSString* localizedErrStr = msg; //NSLocalizedString(msg, msg);
    NSArray* objectsArray = [[NSArray alloc] initWithObjects: localizedErrStr, nil];
    NSArray* keysArray = [[NSArray alloc] initWithObjects: NSLocalizedDescriptionKey, nil];            
    NSDictionary* userInfoDict = [[NSDictionary alloc] initWithObjects:objectsArray forKeys: keysArray];
    NSError* error = [NSError errorWithDomain:domain code:code userInfo:userInfoDict];
    return error;  
}


enum DemoJSONRequest_State_t 
{
    StateIdle =                                 0U,
    
    ConnectionStateEstablishingConnection =     1U <<  0,    //  1
    ConnectionStateRunning =                    1U <<  1,    //  2
    ConnectionStateCanceledEnd =                1U <<  2,    //  4
    ConnectionStateErrorEnd =                   1U <<  3,    //  8
    ConnectionStateFinishedEnd =                1U <<  4,    // 16
    
    ParserStateRunning =                        1U <<  7,
    ParserStateCanceledEnd =                    1U <<  8,
    ParserStateErrorEnd =                       1U <<  9,
    ParserStateFinishedEnd =                    1U << 10,

    ConnectionStateEndMask =        ConnectionStateCanceledEnd | ConnectionStateErrorEnd | ConnectionStateFinishedEnd,
    ParserStateEndMask =            ParserStateCanceledEnd | ParserStateErrorEnd | ParserStateFinishedEnd,
    StateEndMask =                  ConnectionStateEndMask | ParserStateEndMask,
    ConnectionStateRunningMask =    ConnectionStateEstablishingConnection | ConnectionStateRunning,
    ParserStateRunningMask =        ParserStateRunning,
    StateRunningMask =              ConnectionStateRunningMask | ParserStateRunningMask,
    StateCanceledMask =             ConnectionStateCanceledEnd | ParserStateCanceledEnd
};
typedef enum DemoJSONRequest_State_t DemoJSONRequest_State;



@interface DemoJSONRequest() 

@property (nonatomic, strong) NSURL* baseURL;
@property (nonatomic, strong) NSURLConnection* connection;

@property (nonatomic, readwrite, strong) NSMutableData* responseBuffer;
@property (nonatomic, readwrite, strong) id jsonResult;

- (BOOL) isExecutingOnSecondaryThread;

@end


@implementation DemoJSONRequest 
{
    __strong id<DemoJSONRequestDelegate> _delegate;
    __strong id<DemoJSONAuthenticationDelegate> _authenticationDelegate;
    NSURL*                  _baseURL;
    NSString*               _resourceSpecifier;
    NSURLConnection*        _connection;
    NSMutableURLRequest*    _request;
    NSOperationQueue*       _connectionQueue;
    NSMutableData*          _responseBuffer;
    id                      _jsonResult;
    BOOL                    _requiresJSONResponse;
    
    // _state ivar may be concurrently accessed and modified, from any thread.
    volatile uint32_t       _state;
}


@synthesize delegate = _delegate;
@synthesize authenticationDelegate = _authenticationDelegate;
@synthesize baseURL = _baseURL;
@synthesize resourceSpecifier = _resourceSpecifier;
@synthesize connection = _connection;
@synthesize request = _request;
@synthesize connectionQueue = _connectionQueue;
@synthesize responseBuffer = _responseBuffer;
@synthesize jsonResult = _jsonResult;


// Designated Initializer
- (id) initWithBaseURL:(NSURL*)baseURL 
     resourceSpecifier:(NSString*)resourceSpecifier 
      connectionQueue:(NSOperationQueue*)connectionQueue
              delegate:(id<DemoJSONRequestDelegate>)delegate
{
    self = [super init];
    if (self) {
        self.baseURL = baseURL;
        self.resourceSpecifier = resourceSpecifier;
        self.connectionQueue = connectionQueue;
        self.delegate = delegate;
        _state = StateIdle;
    }
    return self;
}

- (id) initWithURL:(NSURL*)URL 
  connectionQueue:(NSOperationQueue*)connectionQueue
          delegate:(id<DemoJSONRequestDelegate>)delegate
{
    NSParameterAssert(URL != nil);
    NSURL* baseURL = [URL baseURL];
    NSString* resourceSpecifier = [URL resourceSpecifier];
    return [self initWithBaseURL:baseURL 
               resourceSpecifier:resourceSpecifier 
                connectionQueue:connectionQueue
                        delegate:delegate];
}


- (id) init {
    return [self initWithBaseURL:nil resourceSpecifier:nil connectionQueue:nil delegate:nil];
}


#pragma mark - Properties

- (NSOperationQueue*) connectionQueue {
    if (_connectionQueue == nil) {
        _connectionQueue = [NSOperationQueue mainQueue];
    }
    return _connectionQueue;
}

- (NSMutableData*) responseBuffer {
    if (_responseBuffer == nil) {
        _responseBuffer = [[NSMutableData alloc] init];
    }
    return _responseBuffer;
}

- (NSURLRequest*) request
{
    if (_request == nil) 
    {
        NSAssert(_resourceSpecifier != nil, @"_resourceSpecifier is nil");
        NSAssert(_baseURL != nil, @"_baseURL is nil");
        NSURL* url = [self.baseURL URLByAppendingPathComponent:self.resourceSpecifier];    

#if defined (DEBUG)    
        NSTimeInterval request_timeout = 600;
#else    
        NSTimeInterval request_timeout = 60;
#endif    
        // If _request equals nil, it creates a request which is appropriate for
        // loading a JSON document.
        _request = [NSMutableURLRequest requestWithURL:url 
                                           cachePolicy:NSURLRequestUseProtocolCachePolicy/*NSURLRequestReloadIgnoringLocalAndRemoteCacheData */
                                       timeoutInterval:request_timeout];
        [_request setValue:@"application/json" forHTTPHeaderField:@"Accept"];
        [_request setValue:@"gzip,deflate" forHTTPHeaderField:@"Accept-Encoding"];    
    }    
    return _request;
}

- (BOOL) isRunning {
    return ((_state & StateRunningMask) != 0);
}

- (BOOL) isFinished {
    return ((_state & StateEndMask) != 0);
}

- (BOOL) isCancelled {
    return ((_state & StateCanceledMask) != 0);
}



#pragma mark -


- (void) cancel_private 
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    DemoJSONRequest_State old_state = OSAtomicOr32Orig((uint32_t)StateCanceledMask, &_state);
    if ( (old_state & StateCanceledMask) != 0 ) {
        DLogWarn(@"already canceled");
        return;
    }
    if ( old_state == StateIdle || ((old_state & ConnectionStateEndMask) != 0 && (old_state & ParserStateEndMask) != 0)) {
        DLogWarn(@"nothing to cancel");
        return;
    }
    // Since executing on the connection queue -- respectively on the parser's
    // worker queue -- we can ensure that the connection has finished (canceled) 
    // and that the parser has finished (canceled):
    OSAtomicAnd32Barrier((ConnectionStateCanceledEnd|ParserStateCanceledEnd), &_state);  
    dispatch_async(dispatch_get_main_queue(), ^{
        [DemoNetworkIndicator sharedNetworkIndicator].active = NO;
    });
    if ([_delegate respondsToSelector:@selector(jsonRequest:didFailWithError:)]) {
        [_delegate jsonRequest:self didFailWithError:makeError(defaultErrorDomain, DemoJSONRequestError_OperationCanceled, nil)];
    }
    self.delegate = nil;
}


#pragma clang diagnostic push
#pragma clang diagnostic ignored  "-Warc-retain-cycles"
- (void) cancel
{
    // We assume cancel can be send several times to a receiver without any harmful effect!
    [self.connection cancel];      
    // TODO: [self.parser cancel];
    
    // We need to actually wait until cancel is "in effect" through scheduling the method
    // cancel_private to the connection queue where we set the state. When it will
    // be executed, the connection and the parser has been eventually canceled:
    [self.connectionQueue addOperationWithBlock:^{
        [self cancel_private];
    }];
}
#pragma clang diagnostic pop



- (BOOL) isExecutingOnSecondaryThread {
    return ([NSThread currentThread] != [NSThread mainThread]);
}


// Can be invoked from any thread. It will be executed only once.
// Creates a default request, or if property "request" has been set uses the 
// currently set request.
- (void) start {
    [self startWithRequest:self.request];
}


// Can be invoked from any thread. It will be executed only once.
#pragma clang diagnostic push
#pragma clang diagnostic ignored  "-Warc-retain-cycles"
- (void) startWithRequest:(NSURLRequest*)request
{
    if (!OSAtomicCompareAndSwap32(StateIdle, ConnectionStateEstablishingConnection, (volatile int32_t*)&_state)) {
        DLogInfo(@"Starting connection refused since it is not idle");
        return;
    }
    [self.connectionQueue addOperationWithBlock:^{
        [self startConnectionWithRequest:request];
    }];
}
#pragma clang diagnostic pop


- (void) startConnectionWithRequest:(NSURLRequest*)request
{
    // Caution: do not invoke startConnection directly, use startWithRequest:resourceSpecifier: 
    // method. Start connection will be invoked only once!
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    NSAssert(_connection == nil, @"_connection is not nil");
    NSAssert(_state == ConnectionStateEstablishingConnection, @"_state is not 'ConnectionStateEstablishingConnection'");
    
    // Clear the URL cache:
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
    //[[NSURLCache sharedURLCache] setMemoryCapacity:1024*64];   
    [[NSURLCache sharedURLCache] setMemoryCapacity:0];   
    DLogInfo(@"starting connection with request headers: %@", [self.request allHTTPHeaderFields]);
    // Create the URL connection and set its delegate:
    self.request = request;
    self.connection = [[NSURLConnection alloc] initWithRequest:self.request 
                                                      delegate:self 
                                              startImmediately:NO];
    if (self.connection == nil) {
        // Error, most likely due to an invalid URL
        OSAtomicOr32Barrier(ConnectionStateErrorEnd, &_state);
        OSAtomicAnd32Barrier(~ConnectionStateEstablishingConnection, &_state);
        [self handleError:makeError(defaultErrorDomain, DemoJSONRequestError_URLCreateFailed, nil)];
        //[self reset];
        return;
    }
    if (OSAtomicCompareAndSwap32(ConnectionStateEstablishingConnection, ConnectionStateRunning, (volatile int32_t*)&_state))
    {        
        DLogInfo(@"Start downloading URL %@", [_request URL]);   
        dispatch_async(dispatch_get_main_queue(), ^{
            [DemoNetworkIndicator sharedNetworkIndicator].active = YES;
        });
        DLogInfo(@"Connection Queue: %@", self.connectionQueue.name);
#if !defined(BUG_iOS_setDelegateQueue)
        [self.connection setDelegateQueue:self.connectionQueue];
        [self.connection start];
#else
        [self.connection scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        NSAssert([NSThread currentThread] == [NSThread mainThread], @"this workaround requires to schedule the connection on the main queue");
        [self.connection start];
#endif
        if ([self.delegate respondsToSelector:@selector(jsonRequestDidStartConnection:)]) {
            [self.delegate jsonRequestDidStartConnection:self];
        }
    }
    else {
        DLogError(@"state is not StateEstablishingConnection");
    }
}



#pragma mark -
#pragma mark NSURLConnection Delegate

// For demo purposes, disable the cache:
- (NSCachedURLResponse *)connection:(NSURLConnection *)connection 
                  willCacheResponse:(NSCachedURLResponse *)cachedResponse {
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    return nil;
}

- (NSURLRequest *)connection:(NSURLConnection *)connection 
             willSendRequest:(NSURLRequest *)request 
            redirectResponse:(NSURLResponse *)redirectResponse
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    DLogInfo(@"connection will send request: %@\nheaders: %@", request, [request allHTTPHeaderFields]);
    return request;
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    DLogError(@"Connection did fail with error: %@", error);
    OSAtomicOr32Barrier(ConnectionStateErrorEnd, &_state);
    OSAtomicAnd32Barrier(~ConnectionStateRunning, &_state);
    // Reset:
    //[self reset];
    self.responseBuffer = nil;
    if ([error code] == kCFURLErrorNotConnectedToInternet) {
        // if we can identify the error, we can present a more precise message to the user.
        NSDictionary* userInfo = @{NSLocalizedDescriptionKey: NSLocalizedString(@"No Connection Error", nil)};
        NSError* noConnectionError = [NSError errorWithDomain:NSCocoaErrorDomain
                                                         code:kCFURLErrorNotConnectedToInternet
                                                     userInfo:userInfo];
        [self handleError:noConnectionError];
    } else {
        // otherwise handle the error generically
        [self handleError:error];
    }  
    // TODO: reset
    dispatch_async(dispatch_get_main_queue(), ^{
        [DemoNetworkIndicator sharedNetworkIndicator].active = NO;
    });
    self.delegate = nil;
}

- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response 
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    self.responseBuffer = nil;
    // check for HTTP status code for proxy authentication failures
    // anything in the 200 to 299 range is considered successful,
    // also make sure the MIMEType is correct:
    NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse *)response;
    NSInteger statusCode = [httpResponse statusCode];
    DLogInfo(@"Connection did receive response: status code: %d, headers: %@", 
             statusCode, [(NSHTTPURLResponse*)response allHeaderFields]);
    NSString* mimeType = [response MIMEType];
    _requiresJSONResponse = [[_request HTTPMethod] isEqualToString:@"GET"] || [[_request HTTPMethod] isEqualToString:@"POST"];
    if (statusCode < 200 || statusCode > 300 || (_requiresJSONResponse && ![mimeType isEqual:@"application/json"])) 
    {
        OSAtomicOr32Barrier(ConnectionStateErrorEnd, &_state);
        OSAtomicAnd32Barrier(~ConnectionStateRunning, &_state);
        NSDictionary* userInfo = @{NSLocalizedDescriptionKey: NSLocalizedString(@"HTTP Error", nil)};
        NSError* error = [NSError errorWithDomain:@"HTTP" 
                                             code:[httpResponse statusCode] 
                                         userInfo:userInfo];
        [self handleError:error];
    }
}


- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    [self handlePartialResponseData:data];
}


- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    OSAtomicOr32Barrier(ConnectionStateFinishedEnd, &_state);
    OSAtomicAnd32Barrier(~ConnectionStateRunning, &_state);
    
    if (_requiresJSONResponse) {
        if ([_delegate respondsToSelector:@selector(jsonRequestDidStartParsingJSON:)]) {
            [_delegate jsonRequestDidStartParsingJSON:self];
        }    
        [self processResponseData:self.responseBuffer];
    } else {
        if ([_delegate respondsToSelector:@selector(jsonRequestDidFinish:)]) {
            [_delegate jsonRequestDidFinish:self];
        }    
    }
    self.responseBuffer = nil;

    dispatch_async(dispatch_get_main_queue(), ^{
        [DemoNetworkIndicator sharedNetworkIndicator].active = NO;
    });
}


- (void) connection:(NSURLConnection *)connection willSendRequestForAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge 
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    if (([challenge previousFailureCount] == 0))
    {
        NSURLCredential* credential = nil;        
        if ([self.authenticationDelegate respondsToSelector:@selector(jsonRequest:credentialForAuthenticationChallenge:)]) {
            credential = [self.authenticationDelegate jsonRequest:self 
                                credentialForAuthenticationChallenge:challenge];
            [[challenge sender] useCredential:credential forAuthenticationChallenge:challenge]; 
            return;
        }
        else if ([[challenge sender] respondsToSelector:@selector(performDefaultHandlingForAuthenticationChallenge:)]) {
            [[challenge sender] performDefaultHandlingForAuthenticationChallenge:challenge];
            return;
        } 
        else {
            [[challenge sender] continueWithoutCredentialForAuthenticationChallenge:challenge];
        }
    }
    else /* [challenge previousFailureCount] > 0 */
    {
        NSError* error = [challenge error];
        [[challenge sender] cancelAuthenticationChallenge:challenge];            
        if (error == nil) {
            error = makeError(defaultErrorDomain, DemoJSONRequestError_AuthenticationFailed, nil);
        }
        [self handleError:error];
        return;            
    } 
}



#pragma mark - Overridable Methods

- (void) handlePartialResponseData:(NSData*) data
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    [self.responseBuffer appendData:data];
}

- (void) processResponseData:(NSData*) data 
{
    NSAssert(_connectionQueue == [NSOperationQueue currentQueue], @"not executing on the connection queue");
    // We assume only one JSON Document!
    OSAtomicOr32Barrier(ParserStateRunning, &_state);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError* error;
        self.jsonResult = [JPJsonParser parseData:data options:(JPJsonParserOptions)0 error:&error];
        if (self.jsonResult) {
            [self notifyDelegateDidCreateJSONRepresentation:self.jsonResult];
        }
        else {
            OSAtomicOr32Barrier(ParserStateErrorEnd, &_state);
            OSAtomicAnd32Barrier(~ParserStateRunning, &_state);
            if (_connection) {
                [_connection cancel];
            }
            [self handleError:error];
            return;
        }
        OSAtomicOr32Barrier(ParserStateFinishedEnd, &_state);
        OSAtomicAnd32Barrier(~ParserStateRunning, &_state);
        [self notifyDelegateFinishedParsing];
    });
    // Assumption: connection is finished.
    //NSAssert(_state & )    
}



#pragma mark - Private Methods

#pragma clang diagnostic push
#pragma clang diagnostic ignored  "-Warc-retain-cycles"
- (void) notifyDelegateDidCreateJSONRepresentation:(id)json
{
    if ([self.delegate respondsToSelector:@selector(jsonRequest:didCreateJSONRepresentation:)]) {
        [self.connectionQueue addOperationWithBlock:^{
            [self.delegate jsonRequest:self didCreateJSONRepresentation:json];
        }];
    }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored  "-Warc-retain-cycles"
- (void) notifyDelegateFinishedParsing
{
    if ([self.delegate respondsToSelector:@selector(jsonRequestFinishedParsing:)]) {
        [self.connectionQueue addOperationWithBlock:^{
            [self.delegate jsonRequestFinishedParsing:self];
        }];
    }
    if ([self.delegate respondsToSelector:@selector(jsonRequestDidFinish:)]) {
        [self.connectionQueue addOperationWithBlock:^{
            [self.delegate jsonRequestDidFinish:self];
        }];
    }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored  "-Warc-retain-cycles"
- (void) handleError:(NSError*)error
{
    DLogError(@"%@", error);
    if ([self.delegate respondsToSelector:@selector(jsonRequest:didFailWithError:)]) {
        [self.connectionQueue addOperationWithBlock:^{
            [self.delegate jsonRequest:self didFailWithError:error];
        }];
    }    
}
#pragma clang diagnostic pop


#if 0    // not yet implemented

- (void) reset
{
    // We can perform a reset only if the connection and the parser
    // is stopped, and no new start is in progress:
    NSAssert(_connectionQueue == nil, @"_connectionQueue is not nil");
    
    _connection = nil;
    _request = nil;
    _connectionQueue = nil;
    
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    _runLoopDone = NO;
#endif    
#if !defined (WRITE_TO_TEMP_FILE)
    _parser = nil;
#endif    
    
    // stats:
    
    OSAtomicAnd32(0, &_state);        
    
}
#endif    // not yet implemented




@end

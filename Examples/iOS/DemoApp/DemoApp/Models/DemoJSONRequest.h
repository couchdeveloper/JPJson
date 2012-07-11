//
//  DemoJSONRequest.h
//  DemoApp
//
//  Created by Andreas Grosam on 11.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@class DemoJSONRequest;

/**
 DemoJSONRequest Error Codes.
 
 These constants are set in the error's code property.
 
 */
enum DemoJSONRequestError_t {
    DemoJSONRequestError_NoError = 0,
    DemoJSONRequestError_OperationCanceled = 1,
    DemoJSONRequestError_URLCreateFailed = 2,
    DemoJSONRequestError_AuthenticationFailed = 3,
    DemoJSONRequestError_AbortedDueToLowMemory = 4,
    DemoJSONRequestError_LastError,
    DemoJSONRequest_UnknownError = DemoJSONRequestError_LastError
};


@protocol DemoJSONRequestDelegate <NSObject>    
@optional

- (void) jsonRequestDidStartConnection:(DemoJSONRequest *)request;
- (void) jsonRequestDidFinishConnection:(DemoJSONRequest *)request;
- (void) jsonRequestDidStartParsingJSON:(DemoJSONRequest *)request;
- (void) jsonRequest:(DemoJSONRequest *)request didCreateJSONRepresentation:(id)json;
- (void) jsonRequestFinishedParsing:(DemoJSONRequest*)request;
- (void) jsonRequestDidFinish:(DemoJSONRequest*)request;
- (void) jsonRequest:(DemoJSONRequest *)request didFailWithError:(NSError *)error;
@end


@protocol DemoJSONAuthenticationDelegate <NSObject>    
@optional

- (NSURLCredential*) jsonRequest:(DemoJSONRequest *)request 
        credentialForAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge;
@end


/**
 DemoJSONRequest is repsonsible for issuing HTTP requests for sending and
 receiving JSON data, and possibly creating a representation from a JSON
 document, or possibly encoding an object to its corresponding JSON document.
  
 Internally, DemoJSONRequest uses a NSURLConnection to perform asynchronous
 network requests. The NSURLConnection requires a Run Loop and thus a thread,
 where the delegate messages will be scheduled. The thread will be provided
 by an NSOperationQueue. If there is no queue specified through property 
 connectionQueue, DemoJSONRequest uses the main queue.
 
 
 Receiving Data
 --------------
 The NSURLConnection will receive the data in chunks. Depending on its configuration, 
 the DemoJSONRequest instance may 
 
 I) accumulate the data chunks into one data object until the response 
 data is complete and then subsequently start processing it - for example running
 the JSON parser on it to create a JSON representation. Or, it may 
 
 II) immediately process the partial response data. 
 
 Approach I seems quite easy to understand, eventhough there are a few variations
 -- for example that the data can be written to a temporary file.
 
 Obviously, approch II has a few advantages over approach I. It doesn't require
 a temporary buffer or use temporary files. It's possible to load and parse very 
 large input. 
 
 
 Processing Response Data
 ------------------------
 Processing the response data -- in either aproach -- can be performed 
 asynchronously or synchronously. Synchronous vs asynchronous processing has 
 the following consequences:
 
 - In synchronous mode the connection queue's thread will be blocked. Nonetheless, 
 the lower network layers (within the kernel) will still continue to receive data
 and queue it up in its internal buffers - up to a certain limit, which is 
 about 1 MByte.
 
 - In asynchronous mode and if processing is slower than downloading, it requires 
 to queue the data chunks - which in turn may cause memory related issues.
 
 Implementing approach II requires a parser which is capable to parse partial 
 input. Such kind of parser actually exists and would be perfect for approach II. 
 Its implementation however is more complex than a primitive Recursive Decent 
 Parser type. Quite obviously, a recursive decent parser can not be used in 
 approach II. Surprisingly, there is a way for recursive decent parsers as well, 
 letting it appear as if processing is synchronous - but actually downloading and 
 parsing executes simultaneously, even utilizing multiple CPUs.
 
 */


@interface DemoJSONRequest : NSObject <NSURLConnectionDelegate>

// Designated Initializer
- (id) initWithBaseURL:(NSURL*)baseURL 
     resourceSpecifier:(NSString*)resourceSpecifier 
      connectionQueue:(NSOperationQueue*)connectionQueue
              delegate:(id<DemoJSONRequestDelegate>)delegate;

- (id) initWithURL:(NSURL*)URL 
  connectionQueue:(NSOperationQueue*)connectionQueue
          delegate:(id<DemoJSONRequestDelegate>)delegate;


@property (nonatomic, strong) NSOperationQueue* connectionQueue;
@property (nonatomic, strong) id<DemoJSONRequestDelegate> delegate;
@property (nonatomic, strong) NSURLRequest* request;
@property (nonatomic, strong) NSString* resourceSpecifier;


- (void) start;

@property (nonatomic, strong) id<DemoJSONAuthenticationDelegate> authenticationDelegate;
@property (nonatomic, readonly, strong) NSMutableData* responseBuffer;
@property (nonatomic, readonly, strong) id jsonResult;


@property (nonatomic, readonly) BOOL isCancelled;
@property (nonatomic, readonly) BOOL isRunning;
@property (nonatomic, readonly) BOOL isFinished;

- (void) cancel;

// Possible override:
- (void) handlePartialResponseData:(NSData*) data;
- (void) processResponseData:(NSData*) data;


@end

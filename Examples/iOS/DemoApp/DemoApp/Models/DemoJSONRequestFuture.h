//
//  DemoJSONRequestFuture.h
//  DemoApp
//
//  Created by Andreas Grosam on 16.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 Class DemoJSONRequestFuture
 
 Models Future.

 
 A 'future' is a placeholder for the result of an asynchronous message invocation 
 which is "not yet finished". The calling thread will continue to execute code
 as long as the code does not access properties of the future which requires the
 asynchronous method to be finished. Otherwise, the calling thread will block 
 until the result is available.
 
 
 When an DemoJSONRequestFuture will be deallocated and if its fetch
 request is sill executing - the fetch request will be canceled.
 
 Links:
 <http://c2.com/cgi/wiki?FutureValue>
 
 
 Keywords: Future, Asynchronous Request, Active Object
 
 */

@class DemoJSONRequest;

/** Function pointer type definitions for completion handler blocks */
typedef void (^fetchCompletionHandler_BlockType)(id result, NSError* error);

@interface DemoJSONRequestFuture : NSObject  /* DemoFutureProtocol */

// Designated Initializer
- (id)initWithJSONRequest:(DemoJSONRequest*)jsonRequest;

- (id) initWithError:(NSError*)error;



// Non-blocking properies and methods:
@property (nonatomic, readonly) BOOL isCancelled;
@property (nonatomic, readonly) BOOL isExecuting;   // observable property (KVO)
@property (nonatomic, readonly) BOOL isFinished;    // observable property (KVO)
- (void) cancel;


// Properties 'error' and 'value' block until the result has become available.
@property (nonatomic, readonly) id value;
@property (nonatomic, readonly) NSError* error;


// OriginModelFetchRequest specific:
@property (nonatomic, copy) fetchCompletionHandler_BlockType completionHandler;


@end


//
//  DemoOriginModel.h
//  DemoApp
//
//  Created by Andreas Grosam on 16.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "DemoJSONRequestFuture.h"

/**
 Class DemoOriginModel
 
 
 A DemoOriginModel defines the interface to a Webservice API.
 
 "OriginModel" is an abstract base class for an Entity residing on the server
 side, the "Origin".
 
 All methods will execute 
 
 
 
 */


@interface DemoOriginModel : NSObject

/**
 
 A DemoOriginModel executes REST methods over HTTP sent to a remote web service.
 
 
 Internally, OriginModel uses a NSURLConnection to perform asynchronous network
 requests. If there is no connection thread explicitly specified through the 
 public interface through property connectionThread, OriginModel creates its 
 own private thread per instance where it schedules the NSURLConnection. 
 Depending on the configuration, processing of the response data will be either 
 executed synchronously on the connection thread, or synchronously on a 
 different private thread or asynchronously on a different thread.
 
 The receiver may execute several REST methods simultaneously.
 
 Returns a DemoOriginModelFetchRequestFuture object which the caller is required
 to keep alive until the asynchronous task finishes. If the future will be
 destroyed prematurely, the fetch request will be canceled and there is no way to 
 access the result anymore.
 
 */

// Designated Initializer
//
// Intialize an OriginModel whose connection delegate messages will be 
// scheduled on the Run Loop associated to the specified thread. If parameter
// connectionThread equals nil, the main thtread's Run Loop will be used.
- (id)initWithQueue:(NSOperationQueue*)connectionQueue;


// Asynchronously perform a fetch request. 
- (DemoJSONRequestFuture*) asyncFetchObjectsWithBaseURL:(NSURL*)baseURL 
                                             entityName:(NSString*)name
                                              predicate:(NSPredicate*)predicate 
                                      completionHandler:(fetchCompletionHandler_BlockType)completionHandler;


- (DemoJSONRequestFuture*) asyncCreateObject:(id)object
                                 withBaseURL:(NSURL*)baseURL 
                                  entityName:(NSString*)name
                           completionHandler:(fetchCompletionHandler_BlockType)completionHandler;

- (DemoJSONRequestFuture*) asyncReadObject:(id)object
                                 withBaseURL:(NSURL*)baseURL 
                                  entityName:(NSString*)name
                           completionHandler:(fetchCompletionHandler_BlockType)completionHandler;

- (DemoJSONRequestFuture*) asyncUpdateObject:(id)object
                                 withBaseURL:(NSURL*)baseURL 
                                  entityName:(NSString*)name
                           completionHandler:(fetchCompletionHandler_BlockType)completionHandler;

- (DemoJSONRequestFuture*) asyncDeleteObject:(id)object
                                withBaseURL:(NSURL*)baseURL 
                                  entityName:(NSString*)name
                           completionHandler:(fetchCompletionHandler_BlockType)completionHandler;


// Method to override - never invoke it yourself!
- (NSString*) resourceSpeciferWithEntity:(NSString*)entityName 
                               predicate:(NSPredicate*)predicate;

- (NSString*) resourceSpeciferForOriginObject:(id)object 
                                       entity:(NSString*)entityName;

@end

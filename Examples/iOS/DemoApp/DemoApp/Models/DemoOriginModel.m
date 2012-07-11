//
//  DemoOriginModel.m
//  DemoApp
//
//  Created by Andreas Grosam on 16.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoOriginModel.h"
#import "DemoJSONRequestFuture.h"
#import "DemoJSONRequest.h"
#import "JPJson/JPJsonWriter.h"



@interface DemoOriginModel ()

@property (nonatomic, strong) NSOperationQueue* connectionQueue;

@end


@implementation DemoOriginModel  {
    NSOperationQueue* _connectionQueue;
}

@synthesize connectionQueue = _connectionQueue;

- (id)initWithQueue:(NSOperationQueue*)connectionQueue
{
    self = [super init];
    if (self) {
        _connectionQueue = connectionQueue;
    }
    return self;
}

- (id) init {
    return [self initWithQueue:nil];
}


- (NSOperationQueue*) connectionQueue {
    if (_connectionQueue == nil) {
        _connectionQueue = [NSOperationQueue mainQueue];
    }
    return _connectionQueue;
}



- (DemoJSONRequestFuture*) asyncFetchObjectsWithBaseURL:(NSURL*)baseURL 
                                             entityName:(NSString*)name
                                              predicate:(NSPredicate*)predicate 
                                      completionHandler:(fetchCompletionHandler_BlockType)completionHandler;
{
    DemoJSONRequest* readRequest = 
        [[DemoJSONRequest alloc] initWithBaseURL:baseURL
                               resourceSpecifier:[self resourceSpeciferWithEntity:name predicate:predicate]
                                connectionQueue:self.connectionQueue
                                        delegate:nil];
    DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithJSONRequest:readRequest];    
    future.completionHandler = completionHandler;
    [readRequest start];
    return future;
}


- (DemoJSONRequestFuture*) asyncReadObject:(id)object
                               withBaseURL:(NSURL*)baseURL 
                                entityName:(NSString*)name
                         completionHandler:(fetchCompletionHandler_BlockType)completionHandler
{
    NSURL* url = [baseURL URLByAppendingPathComponent:[self resourceSpeciferForOriginObject:object entity:name]];
    NSMutableURLRequest* request = [[NSMutableURLRequest alloc] init];
    [request setURL:url];
    [request setHTTPMethod:@"GET"];
    [request setValue:@"application/json" forHTTPHeaderField:@"Accept"];
    [request setValue:@"gzip,deflate" forHTTPHeaderField:@"Accept-Encoding"];    
    DemoJSONRequest* createRequest = [[DemoJSONRequest alloc] init];
    createRequest.request = request; 
    createRequest.connectionQueue = self.connectionQueue;
    DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithJSONRequest:createRequest];    
    future.completionHandler = completionHandler;
    [createRequest start];
    return future;
}

- (DemoJSONRequestFuture*) asyncCreateObject:(id)object
                                 withBaseURL:(NSURL*)baseURL 
                                  entityName:(NSString*)name
                           completionHandler:(fetchCompletionHandler_BlockType)completionHandler
{
    NSError* error;
    NSData* JSONstring = [JPJsonWriter dataWithObject:object 
                                             encoding:JPUnicodeEncoding_UTF8 
                                              options:(JPJsonWriterOptions)0 
                                                error:&error];
    if (JSONstring == nil) {
        DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithError:error];
        future.completionHandler = completionHandler;
        if (completionHandler) {
            completionHandler(nil, error);
        }
        return future;
    }
    NSURL* url = [baseURL URLByAppendingPathComponent:[self resourceSpeciferWithEntity:name predicate:nil]];
    NSMutableURLRequest* request = [[NSMutableURLRequest alloc] init];
    [request setURL:url];
    [request setHTTPMethod:@"POST"];
    [request setHTTPBody:JSONstring];
    [request setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
    [request setValue:@"application/json" forHTTPHeaderField:@"Accept"];
    [request setValue:@"gzip,deflate" forHTTPHeaderField:@"Accept-Encoding"];    
    DemoJSONRequest* createRequest = [[DemoJSONRequest alloc] init];
    createRequest.connectionQueue = self.connectionQueue;
    createRequest.request = request; 
    DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithJSONRequest:createRequest];    
    future.completionHandler = completionHandler;
    [createRequest start];
    return future;
}

- (DemoJSONRequestFuture*) asyncUpdateObject:(id)object
                                 withBaseURL:(NSURL*)baseURL 
                                  entityName:(NSString*)name
                           completionHandler:(fetchCompletionHandler_BlockType)completionHandler
{
    NSError* error;
    NSData* JSONstring = [JPJsonWriter dataWithObject:object 
                                             encoding:JPUnicodeEncoding_UTF8 
                                              options:(JPJsonWriterOptions)0 
                                                error:&error];
    if (JSONstring == nil) {
        DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithError:error];
        future.completionHandler = completionHandler;
        if (completionHandler) {
            completionHandler(nil, error);
        }
        return future;
    }
    NSURL* url = [baseURL URLByAppendingPathComponent:[self resourceSpeciferForOriginObject:object entity:name]];
    NSMutableURLRequest* request = [[NSMutableURLRequest alloc] init];
    [request setURL:url];
    [request setHTTPMethod:@"PUT"];
    [request setHTTPBody:JSONstring];
    [request setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
    [request setValue:@"application/json" forHTTPHeaderField:@"Accept"];
    [request setValue:@"gzip,deflate" forHTTPHeaderField:@"Accept-Encoding"];    
    DemoJSONRequest* updateRequest = [[DemoJSONRequest alloc] init];
    updateRequest.request = request; 
    updateRequest.connectionQueue = self.connectionQueue;
    DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithJSONRequest:updateRequest];    
    future.completionHandler = completionHandler;
    [updateRequest start];
    return future;
}

- (DemoJSONRequestFuture*) asyncDeleteObject:(id)object
                                 withBaseURL:(NSURL*)baseURL 
                                  entityName:(NSString*)name
                           completionHandler:(fetchCompletionHandler_BlockType)completionHandler
{
    NSURL* url = [baseURL URLByAppendingPathComponent:[self resourceSpeciferForOriginObject:object entity:name]];
    NSMutableURLRequest* request = [[NSMutableURLRequest alloc] init];
    [request setURL:url];
    [request setHTTPMethod:@"DELETE"];
    [request setValue:@"application/json" forHTTPHeaderField:@"Accept"];
    [request setValue:@"gzip,deflate" forHTTPHeaderField:@"Accept-Encoding"];    
    DemoJSONRequest* deleteRequest = [[DemoJSONRequest alloc] init];
    deleteRequest.request = request; 
    deleteRequest.connectionQueue = self.connectionQueue;
    DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithJSONRequest:deleteRequest];    
    future.completionHandler = completionHandler;
    [deleteRequest start];
    return future;
}


-(NSString*) resourceSpeciferWithEntity:(NSString*)entityName predicate:(NSPredicate*)predicate 
{
    NSAssert(predicate == nil, @"predicates not yet implemented");
    
    // Entity names are in singular form - while Rails uses plural form:
    NSString* result = [NSString stringWithFormat:@"%@s",[entityName lowercaseString]];
    return result;
}


- (NSString*) resourceSpeciferForOriginObject:(id)object 
                                       entity:(NSString*)entityName
{
    NSString* originID = [[(NSDictionary*)object objectForKey:@"id"] stringValue];
    NSAssert(originID != nil, @"originID is nil");
    // Entity names are in singular form - while Rails uses plural form:
    NSString* result = [NSString stringWithFormat:@"%@s/%@",[entityName lowercaseString], originID];
    return result;
}


@end

//
//  DemoJSONRequestQueue.m
//  DemoApp
//
//  Created by Andreas Grosam on 16.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoJSONRequestQueue.h"

#import "DemoJSONRequestFuture.h"
#import "DemoJSONRequest.h"


@interface DemoJSONRequestQueue ()

@property (nonatomic, strong) NSThread* connectionThread;

@end


@implementation DemoJSONRequestQueue  {
    NSThread* _connectionThread;
    BOOL _isPrivateTread;
}

@synthesize connectionThread = _connectionThread;

- (id)initWithThread:(NSThread*)connectionThread
{
    self = [super init];
    if (self) {
        _connectionThread = connectionThread;
    }
    return self;
}

- (id) init {
    return [self initWithThread:nil];
}


- (NSThread*) connectionThread {
    if (_connectionThread == nil) {
        NSAssert(0, @"private thread not yet implemented");
    }
    return _connectionThread;
}



- (DemoJSONRequestFuture*) asyncFetchObjectsWithBaseURL:(NSURL*)baseURL 
                                             entityName:(NSString*)name
                                      completionHandler:(fetchCompletionHandler_BlockType)completionHandler;
{
    DemoJSONRequest* jsonRequest = 
    [[DemoJSONRequest alloc] initWithBaseURL:baseURL
                           resourceSpecifier:[self resourceSpeciferWithEntity:name predicate:nil]
                            connectionThread:self.connectionThread
                                    delegate:nil];
    DemoJSONRequestFuture* future = [[DemoJSONRequestFuture alloc] initWithJSONRequest:jsonRequest];    
    future.completionHandler = completionHandler;
    [jsonRequest start];
    return future;
}

-(NSString*) resourceSpeciferWithEntity:(NSString*)entityName predicate:(NSPredicate*)predicate 
{
    NSAssert(predicate == nil, @"predicates not yet implemented");
    
    // Entity names are in singular form - while Rails uses plural form:
    NSString* result = [NSString stringWithFormat:@"%@s",[entityName lowercaseString]];
    return result;
}



@end

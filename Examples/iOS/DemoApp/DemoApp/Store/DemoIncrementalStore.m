//
//  DemoIncrementalStore.m
//  DemoApp
//
//  Created by Andreas Grosam on 17.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoIncrementalStore.h"
#import <CoreData/CoreData.h>


static NSString* defaultErrorDomain = @"DemoIncrementalStore";

enum DemoIncrementalStoreError_t {
    DemoIncrementalStoreError_NoError = 0,
    DemoIncrementalStoreError_NotYetImplemented = 1,
    DemoIncrementalStoreError_LastError,
    DemoIncrementalStoreError_UnknownError = DemoIncrementalStoreError_LastError
};

typedef enum DemoJSONRequestError_t DemoJSONRequestError;

static NSString* DemoIncrementalStoreErroMsg[] = {
    @"No Error.",
    @"Feature not yet implemented.",
    @"Unknown error."
};

__attribute__((ns_returns_retained)) 
static NSError* makeError(NSInteger code, NSString* errString) 
{
    // setup an error object:
    NSString* msg = errString;
    if (msg == nil && (code  <= DemoIncrementalStoreError_UnknownError && code > 0)) {
        msg = DemoIncrementalStoreErroMsg[code];
    }
    NSString* localizedErrStr = msg; //NSLocalizedString(msg, msg);
    NSArray* objectsArray = [[NSArray alloc] initWithObjects: localizedErrStr, nil];
    NSArray* keysArray = [[NSArray alloc] initWithObjects: NSLocalizedDescriptionKey, nil];            
    NSDictionary* userInfoDict = [[NSDictionary alloc] initWithObjects:objectsArray forKeys: keysArray];
    NSError* error = [NSError errorWithDomain:defaultErrorDomain code:code userInfo:userInfoDict];
    return error;  
}


@interface DemoIncrementalStore () 


-(NSDictionary*) fetchMetaDataForApiVersion:(NSString*)version error:(NSError**)error;



@end


@implementation DemoIncrementalStore
{
    NSURL*                  _storeURL;
    NSMutableDictionary*    _cache;
}


+ (NSString*) storeType {
    return @"DemoIncrementalStoreType";
}


-(NSDictionary*) metaDataForAPIVersion:(NSString*)version error:(NSError**)error
{
    // Check if the url is valid, has permissions to read/write and verify that 
    // the scheme is compatible:
    BOOL isReachable = YES; //[_storeURL checkRemoteResourceIsReachableWithAPI:[DemoIncrementalStore apiVersion] options:0 error:&error];    
    if (!isReachable) {
        return nil;
    }
    NSString* identifier = [[_storeURL URLByAppendingPathComponent:[DemoIncrementalStore apiVersion]] absoluteString];
    NSDictionary* result = [[NSDictionary alloc] initWithObjectsAndKeys:
                            [DemoIncrementalStore storeType],   NSStoreTypeKey, 
                            identifier,  NSStoreUUIDKey, 
                            nil];
    return result;
}


- (id)initWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator*)root 
                       configurationName:(NSString*)name 
                                     URL:(NSURL*)url 
                                 options:(NSDictionary *)options
{
    self = [super initWithPersistentStoreCoordinator:root 
                                   configurationName:name 
                                                 URL:url 
                                             options:options];
    if (self) {
        _storeURL = url;
        _cache = [NSMutableDictionary dictionary];
    }
    return self;
}


#pragma mark -

- (BOOL)loadMetadata:(NSError **)error 
{
    NSDictionary* metaData = [self metaDataForAPIVersion:[DemoIncrementalStore apiVersion] 
                                                   error:error];
    if (metaData) {
        [self setMetadata:metaData];
        return YES;
    }
    else {
        return NO;
    }    
}

- (id)executeRequest:(NSPersistentStoreRequest *)request 
         withContext:(NSManagedObjectContext *)context 
               error:(NSError **)error
{
}

- (NSIncrementalStoreNode *)newValuesForObjectWithID:(NSManagedObjectID *)objectID 
                                         withContext:(NSManagedObjectContext *)context 
                                               error:(NSError **)error
{
}


- (id)newValueForRelationship:(NSRelationshipDescription *)relationship 
              forObjectWithID:(NSManagedObjectID *)objectID 
                  withContext:(NSManagedObjectContext *)context 
                        error:(NSError **)error
{
}

- (NSArray *)obtainPermanentIDsForObjects:(NSArray *)array 
                                    error:(NSError **)error
{
}


@end

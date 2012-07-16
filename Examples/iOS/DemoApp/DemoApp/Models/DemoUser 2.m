//
//  DemoUser.m
//  DemoApp
//
//  Created by Andreas Grosam on 11.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#if !__has_feature(objc_arc) 
#error This Objective-C file shall be compiled with ARC enabled.
#endif


#import "DemoUser.h"

#import "JPJson/JPAsyncJsonParser.h"
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPRepresentationGenerator.h"

#import "DemoJSONController.h"

#include <mach/mach_time.h>
#include <unistd.h>
#include <dispatch/dispatch.h>

#if defined (DEBUG)
#import "JPJson/JPJsonWriter.h"
#endif

//#define DEBUG_LOG


#if defined (DEBUG_LOG)
#define DLog(...) NSLog(@"%s %@", __PRETTY_FUNCTION__, [NSString stringWithFormat:__VA_ARGS__])
#else
#define DLog(...) do { } while (0)
#endif



@interface DemoUsers () <DemoJSONControllerDelegate>

@property (nonatomic, strong) NSURL* baseURL;
@property (nonatomic, strong) DemoJSONController* originLoadController;

@property (atomic, strong) NSArray*                originObjects;
@property (nonatomic, strong) NSMutableOrderedSet* createdObjects;
@property (nonatomic, strong) NSMutableOrderedSet* updatedOriginObjects;
@property (nonatomic, strong) NSMutableOrderedSet* updatedObjects;
@property (nonatomic, strong) NSMutableOrderedSet* deletedObjects;

- (void) invalidateObjects;

@end



@implementation DemoUsers  {
    NSArray*                _originObjects;
    
    NSMutableOrderedSet*    _createdObjects;
    NSMutableOrderedSet*    _updatedOriginObjects;
    NSMutableOrderedSet*    _updatedObjects;
    NSMutableOrderedSet*    _deletedObjects;
    NSArray*                _objects;
    
    NSURL*                  _baseURL;
    DemoJSONController*     _originLoadController;
    
    // Observeable states:
    BOOL _isActive;
}

@synthesize baseURL = _baseURL;
@synthesize isActive = _isActive;

@synthesize originObjects = _originObjects;
@synthesize createdObjects = _createdObjects;
@synthesize updatedOriginObjects = _updatedOriginObjects;
@synthesize updatedObjects = _updatedObjects;
@synthesize deletedObjects = _deletedObjects;

@synthesize objects = _objects;

//
// Designated Initializer
//
- (id) initWithBaseURL:(NSURL*)baseURL
{
    self = [super init];
    if (self) {
        self.baseURL = baseURL;
    }
    return self;
}


#pragma mark -

- (NSMutableOrderedSet*) createdObjects {
    if (_createdObjects == nil) {
        _createdObjects = [[NSMutableOrderedSet alloc] init];
    }
    return _createdObjects;
}

- (NSMutableOrderedSet*) updatedOriginObjects {
    if (_updatedOriginObjects == nil) {
        _updatedOriginObjects = [[NSMutableOrderedSet alloc] init];
    }
    return _updatedOriginObjects;
}

- (NSMutableOrderedSet*) updatedObjects {
    if (_updatedObjects == nil) {
        _updatedObjects = [[NSMutableOrderedSet alloc] init];
    }
    return _updatedObjects;
}

- (NSMutableOrderedSet*) deletedObjects {
    if (_deletedObjects == nil) {
        _deletedObjects = [[NSMutableOrderedSet alloc] init];
    }
    return _deletedObjects;
}

- (NSArray*) originObjects {
    NSArray* result;
    @synchronized (self) {
        if (_originObjects == nil) {
            _originObjects = [[NSArray alloc] init];
            [self fetch];
        }
        result = _originObjects;
    }
    return result;
}

- (void) setOriginObjects:(NSArray*)array {
    @synchronized (self) {
        if (_originObjects != array) {
            _originObjects = array;
            [self invalidateObjects];
        }
    }
}


- (NSArray*) objects 
{
    NSArray* result;
    @synchronized (self) {
        if (_objects == nil) {
            [self merge];
        }
        result = _objects;
    }
    return result;
}

- (void) setObjects:(NSArray*) value {
    @synchronized (self) {
        if (_objects != value) {
            _objects = value;
        }
    }    
}



#pragma mark -


- (void) invalidateObjects {
    if (_objects != nil) {
        self.objects = nil;
    }
}

- (void) fetch {
    [self.originLoadController start];
}

- (void) merge 
{
    // For now, we abandon any modificated and deleted origin objects:
    // TODO: implement merge
    self.updatedObjects = nil;
    self.updatedOriginObjects = nil;
    self.deletedObjects = nil;

    NSMutableOrderedSet* objs = [[NSMutableOrderedSet alloc] initWithArray:self.originObjects];
    [objs unionOrderedSet:self.createdObjects];
    [objs minusOrderedSet:self.deletedObjects];
    [objs minusOrderedSet:self.updatedOriginObjects];
    [objs unionOrderedSet:self.updatedObjects];
    self.objects = [[objs array] copy];
}

- (void) push {
    NSAssert(0, @"not yet implemented");
}


- (void) cancelAction
{
    if (_originLoadController) {
        [_originLoadController cancel];
    }
}


//@property (nonatomic, readonly) NSUInteger count;
- (NSUInteger) count
{
    return [self.objects count];
}

- (id) objectAtIndex:(NSUInteger)index {
    id obj = [self.objects objectAtIndex:index];
    return obj;
}

- (void) insertObject:(id)object {
    if (object == nil) {
        return;
    }
    NSAssert([self.createdObjects containsObject:object] == NO, @"Object already created");
    NSAssert([self.deletedObjects containsObject:object] == NO, @"Object deleted");
    NSAssert([self.updatedOriginObjects containsObject:object] == NO, @"Object already exists");
    NSAssert([self.updatedObjects containsObject:object] == NO, @"Object already exists");
    
    [self.createdObjects addObject:object];
    [self invalidateObjects];
}

- (void) deleteObject:(id)object {
    if (object == nil) {
        return;
    }
    if ([self.originObjects containsObject:object]) {
        // Delete an unmodified object
        [self.deletedObjects addObject:object];
    }
    else if ([self.updatedObjects containsObject:object]) {
        // Delete a modified object
        NSUInteger pos = [self.updatedObjects indexOfObject:object];
        [self.updatedObjects removeObject:object];
        [self.updatedOriginObjects removeObjectAtIndex:pos];
        NSAssert([self.updatedOriginObjects count] == [self.updatedObjects count], @"Failed assertion");
    }
    else if ([self.createdObjects containsObject:object])
    {
        [self.createdObjects removeObject:object];
    }
    else {
        NSAssert(0, @"Object does not exist in context");
    }
    [self invalidateObjects];
}

- (void) updateObject:(id)originalObject withModifiedObject:(id)modifiedObject
{
    NSParameterAssert(originalObject != modifiedObject);
    if ( (originalObject == modifiedObject) || (originalObject == nil || modifiedObject == nil)) {
        return;
    }        
    if ([self.originObjects containsObject:originalObject]) {
        // Update an unmodified object:
        NSAssert([self.updatedOriginObjects containsObject:originalObject] == NO, @"Failed assertion");
        [self.updatedOriginObjects addObject:originalObject];
        [self.updatedObjects addObject:modifiedObject];
    }
    else if ([self.updatedObjects containsObject:originalObject]) {
        // Update an already modified object
        NSUInteger pos = [self.updatedObjects indexOfObject:originalObject];
        [self.updatedObjects setObject:modifiedObject atIndex:pos];
    }
    NSAssert([self.updatedOriginObjects count] == [self.updatedObjects count], @"Failed assertion");
    [self invalidateObjects];
}



@synthesize originLoadController = _originLoadController;

- (DemoJSONController*) originLoadController {
    if (_originLoadController == nil) {
       _originLoadController = [[DemoJSONController alloc] 
                                initWithBaseURL:self.baseURL
                                resourceSpecifier:@"users"
                                delegate:self];
    }
    return _originLoadController;
}



@end



@interface DemoUsers (DemoJSONControllerDelegate)
@end

@implementation DemoUsers (DemoJSONControllerDelegate)

- (void) jsonControllerDidStartConnection:(DemoJSONController *)controller
{
    self.isActive = YES;
    NSLog(@"jsonControllerDidStartConnection:");
}

- (void) jsonControllerDidFinishConnection:(DemoJSONController *)controller
{
    NSLog(@"jsonControllerDidFinishConnection:");
}

- (void) jsonControllerDidStartParsingJSON:(DemoJSONController *)controller
{
    NSLog(@"jsonControllerDidStartParsingJSON:");
}

- (void) jsonController:(DemoJSONController *)controller didCreateJSONRepresentation:(id)json
{
    NSLog(@"jsonController:didCreateJSONRepresentation:");
#if defined (DEBUG)
    if (json) {
        NSData* jsonString = [JPJsonWriter dataWithObject:json 
                                                 encoding:JPUnicodeEncoding_UTF8
                                                  options:JPJsonWriterPrettyPrint 
                                                    error:nil];
        NSString* str = [[NSString alloc] initWithBytes:[jsonString bytes] length:[jsonString length] encoding:NSUTF8StringEncoding];
        NSLog(@"\n%@", str);
    }
#endif        
    if (json && [json isKindOfClass:[NSArray class]]) {
        self.originObjects = json;
    }
    else {
        NSLog(@"ERROR: Expected JSON array");
    }       
}

- (void) jsonControllerFinishedParsing:(DemoJSONController*)controller
{
    NSLog(@"jsonControllerFinishedParsing:");
    self.originLoadController = nil;
    self.isActive = NO;
}

- (void) jsonController:(DemoJSONController *)controller didFailWithError:(NSError *)error
{
    NSLog(@"jsonController:didFailWithError: %@", error);
    self.originLoadController = nil;
    self.isActive = NO;
}

- (void) jsonController:(DemoJSONController *)controller 
    willSendRequestForAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge
{
    NSLog(@"jsonController:willSendRequestForAuthenticationChallenge: %@", challenge);
}

@end




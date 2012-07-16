//
//  DemoUser.h
//  DemoApp
//
//  Created by Andreas Grosam on 11.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol RemoteModelProtocol <NSObject>

- (void) fetch;
- (void) merge;
- (void) push;
- (void) cancelAction;
@property (nonatomic) BOOL isActive;


@property (nonatomic, readonly) NSUInteger count;
- (id) objectAtIndex:(NSUInteger) index;
- (void) insertObject:(id)newObject;
- (void) deleteObject:(id)object;
- (void) updateObject:(id)originalObject withModifiedObject:(id)modifiedObject;

@end




@interface DemoUser : NSObject <RemoteModelProtocol>

//
// Designated Initializer
//
- (id) initWithBaseURL:(NSURL*)baseULR;

- (void) fetch;
- (void) merge;
- (void) push;
- (void) cancelAction;
@property (nonatomic) BOOL isActive;


@property (nonatomic, readonly) NSUInteger count;
- (id) objectAtIndex:(NSUInteger) index;
- (void) insertObject:(id)newObject;
- (void) deleteObject:(id)object;
- (void) updateObject:(id)originalObject withModifiedObject:(id)modifiedObject;

@property (nonatomic, strong) NSArray* objects;


@end

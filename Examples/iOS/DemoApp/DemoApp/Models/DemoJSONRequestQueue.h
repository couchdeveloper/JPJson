//
//  DemoJSONRequestQueue.h
//  DemoApp
//
//  Created by Andreas Grosam on 16.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DemoJSONRequestFuture.h"


/**
 
 A DemoJSONRequestQueue executes one or more JSON requests.
 
 
 Returns a DemoJSONRequestFuture object which the caller is required
 to keep alive until the asynchronous task finishes. If the future will be
 destroyed prematurely, the fetch request will be canceled and there is no way to 
 access the result anymore.
 
*/



@interface DemoJSONRequestQueue : NSObject


- (id)initWithThread:(NSThread*)connectionThread;


- (DemoJSONRequestFuture*) asyncFetchObjectsWithBaseURL:(NSURL*)baseURL 
                                             entityName:(NSString*)name
                                      completionHandler:(fetchCompletionHandler_BlockType)completionHandler;

- (NSString*) resourceSpeciferWithEntity:(NSString*)entityName predicate:(NSPredicate*)predicate;


@end

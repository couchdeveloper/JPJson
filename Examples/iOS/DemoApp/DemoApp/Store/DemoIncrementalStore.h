//
//  DemoIncrementalStore.h
//  DemoApp
//
//  Created by Andreas Grosam on 17.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <CoreData/CoreData.h>



@interface DemoIncrementalStore : NSIncrementalStore

+ (NSString*) storeType;
+ (NSString*) apiVersion;


// Designated Initializer
- (id)initWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator*)root 
                       configurationName:(NSString*)name 
                                     URL:(NSURL*)url 
                                 options:(NSDictionary *)options;




@end

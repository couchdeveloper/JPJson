//
//  DemoNetworkIndicator.h
//  DemoApp
//
//  Created by Andreas Grosam on 11.07.12.
//
//

#import <Foundation/Foundation.h>

@interface DemoNetworkIndicator : NSObject

+ (DemoNetworkIndicator*) sharedNetworkIndicator;

@property (nonatomic) BOOL active;

@end

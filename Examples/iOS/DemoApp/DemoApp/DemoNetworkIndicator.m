//
//  DemoNetworkIndicator.m
//  DemoApp
//
//  Created by Andreas Grosam on 11.07.12.
//
//

#import "DemoNetworkIndicator.h"
#include <libkern/OSAtomic.h>
#include <dispatch/dispatch.h>

@implementation DemoNetworkIndicator {
    volatile int32_t       _activations;
}

+ (DemoNetworkIndicator*) sharedNetworkIndicator {
    static DemoNetworkIndicator* sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[DemoNetworkIndicator alloc] init];
    });
    return sharedInstance;
}

//@property (nonatomic) BOOL active;

- (void) setActive:(BOOL)value {
    if (value) {
        int newValue = OSAtomicAdd32Barrier(1, &_activations);
        if (newValue == 1) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
            });
        }
    } else {
        int newValue = OSAtomicAdd32Barrier(-1, &_activations);
        if (newValue == 0) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
            });
        }
    }
}

- (BOOL) active {
    return OSAtomicAdd32Barrier(0, &_activations) > 0;
}


@end

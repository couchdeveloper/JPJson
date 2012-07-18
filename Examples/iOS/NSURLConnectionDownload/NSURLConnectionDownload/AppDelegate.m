//
//  AppDelegate.m
//  NSURLConnectionDownload
//
//  Created by Andreas Grosam on 9/20/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "AppDelegate.h"

#import "NSURLConnectionDownloadViewController.h"
#import "UIDevice-Hardware.h"

static NSString* deviceInformationString()
{
    UIDevice* device = [UIDevice currentDevice];
    
    NSString* infoString = [NSString stringWithFormat:
                            @"\nSystem Name: %@"
                            @"\nSystem Version: %@"
                            @"\nModel: %@"
                            @"\nLocalized Model: %@"
                            @"\nPlatform: %@"
                            @"\nHardware Model: %@"
                            @"\nPlatform Type: %u"
                            @"\nPlatform String: %@"
                            @"\nCPU Frequency: %g MHz"
                            @"\nBUS Frequency: %g MHz"
                            @"\nTotal Memory: %d MByte"
                            @"\nUser Memory: %d MByte"
                            ,
                            device.systemName, device.systemVersion,
                            device.model, device.localizedModel,
                            [device platform], 
                            [device hwmodel],
                            [device platformType], 
                            [device platformString],
                            [device cpuFrequency]*1e-06, 
                            [device busFrequency]*1e-06,
                            (int)([device totalMemory]/(1024*1024)), 
                            (int)([device userMemory]/(1024*1024))
                            ];
    
    return infoString;
}


@implementation AppDelegate

@synthesize window = window_;
@synthesize viewController = viewController_;

- (void)dealloc
{
    [window_ release];
    [viewController_ release];
    [super dealloc];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    // Override point for customization after application launch.
    self.viewController = [[[NSURLConnectionDownloadViewController alloc] 
                            initWithNibName:@"NSURLConnectionDownloadViewController" 
                            bundle:nil] autorelease];
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    NSLog(@"Starting Bench for Device: %@\n", deviceInformationString());
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    /*
     Called when the application is about to terminate.
     Save data if appropriate.
     See also applicationDidEnterBackground:.
     */
}

@end

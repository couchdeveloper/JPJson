//
//  JPBenchAppAppDelegate.m
//  JPBenchApp
//
//  Created by Andreas Grosam on 7/15/11.
//  Copyright 2011 Andreas Grosam
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#import "JPBenchAppAppDelegate.h"

#import "JPBenchAppViewController.h"
#import "UIDevice-Hardware.h"

#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld.h>


static void PrintExecutablePath() {
    uint32_t bufsize = 0;
    _NSGetExecutablePath(NULL, &bufsize);
    char* path = (char*)malloc(bufsize);
    _NSGetExecutablePath(path, &bufsize);                
    char* real_path = realpath(path, NULL);
    free(path);
    NSLog(@"%s", real_path);
    free(real_path);
}


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
                            @"\nPlatform Type: %lu"
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



@implementation JPBenchAppAppDelegate


@synthesize window=_window;

@synthesize viewController=_viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
     
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];    
    PrintExecutablePath();
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

- (void)dealloc
{
    [_window release];
    [_viewController release];
    [super dealloc];
}

@end

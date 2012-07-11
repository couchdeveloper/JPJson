//
//  DemoReachability.h
//  DemoApp
//
//  Created by Andreas Grosam on 11.07.12.
//
//

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>



typedef enum {
    NotReachable = 0,
    ReachableViaWiFi,
    ReachableViaWWAN
} NetworkStatus;

#define kReachabilityChangedNotification @"kNetworkReachabilityChangedNotification"
 
@interface DemoReachability: NSObject
 
//reachabilityWithHostName- Use to check the reachability of a particular host name. 
+ (DemoReachability*) reachabilityWithHostName: (NSString*) hostName;
 
//reachabilityWithAddress- Use to check the reachability of a particular IP address. 
+ (DemoReachability*) reachabilityWithAddress: (const struct sockaddr_in*) hostAddress;
 
//reachabilityForInternetConnection- checks whether the default route is available.  
//  Should be used by applications that do not connect to a particular host
+ (DemoReachability*) reachabilityForInternetConnection;
 
//reachabilityForLocalWiFi- checks whether a local wifi connection is available.
+ (DemoReachability*) reachabilityForLocalWiFi;
 
//Start listening for reachability notifications on the current run loop
- (BOOL) startNotifier;
- (void) stopNotifier;
 
- (NetworkStatus) currentReachabilityStatus;
//WWAN may be available, but not active until a connection has been established.
//WiFi may require a connection for VPN on Demand.
- (BOOL) connectionRequired;
@end

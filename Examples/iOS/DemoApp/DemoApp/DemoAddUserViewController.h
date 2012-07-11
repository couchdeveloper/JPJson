//
//  DemoAddUserViewController.h
//  DemoApp
//
//  Created by Andreas Grosam on 14.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@class DemoAddUserViewController;

@protocol DemoAddUserViewControllerDelegateProtocol <NSObject>
@required
- (void) addUserViewControllerDidCancel:(DemoAddUserViewController*)viewController;
- (void) addUserViewController:(DemoAddUserViewController*)viewController didAcceptWithObject:(id)object;

@optional
- (BOOL) addUserViewController:(DemoAddUserViewController*)viewController isValidObject:(id)object;
- (id) addUserViewControllerProtoypeObject:(DemoAddUserViewController*)viewController;

@end



@interface DemoAddUserViewController : UIViewController

@property (nonatomic, weak) id<DemoAddUserViewControllerDelegateProtocol> delegate;

@end

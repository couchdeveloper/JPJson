//
//  DemoViewController.m
//  DemoApp
//
//  Created by Andreas Grosam on 11.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoViewController.h"

#import "Models/DemoOriginModel.h"
#import "Models/DemoJSONRequestFuture.h"
#import "DemoAddUserViewController.h"
#import "DemoUserTableViewCell.h"
#include <pthread.h>


// The ID of the Segue associated to the DemoAddUserViewController. This ID must
// be set in the corresponding Segue in the MainStoryBoard.
static NSString* DemoAddUserSegueID = @"DemoAddUserSegueID";


@interface DemoViewController () <DemoAddUserViewControllerDelegateProtocol>

@property (nonatomic, copy) NSMutableArray*             users;

@property (weak, nonatomic) IBOutlet UITableView*       tableView;
@property (weak, nonatomic) IBOutlet UIBarButtonItem*   editButton;
@property (weak, nonatomic) IBOutlet UIBarButtonItem*   addButton;
@property (weak, nonatomic) IBOutlet UIBarButtonItem*   cancelButton;
@property (weak, nonatomic) IBOutlet UIBarButtonItem*   pullButton;


@property (nonatomic, strong) DemoOriginModel*          origin;
@property (nonatomic, readonly) NSOperationQueue*               connectionQueue;
@property (nonatomic, strong)        NSURL*             baseURL;
@property (nonatomic, strong)        NSMutableArray*    activeRequests;


@end


/**
 Implementation notes:
 
 The "Model" is represented by the property "users". If users changes, it usually
 requires its view (the table view) to be notified about the change of its content.
 Accesses to property "users" will be executed on a dedicated thread - respectively
 through a dedicated serial dispatch queue. This makes the access to property  
 "users" thread safe.
 
 */


@implementation DemoViewController 
{
    NSMutableArray*         _users;         // our model
    __weak UITableView*     _tableView;     // the view
    __weak UIBarButtonItem* _editButton;
    __weak UIBarButtonItem* _addButton;
    __weak UIBarButtonItem* _cancelButton;
    __weak UIBarButtonItem* _pullButton;
    
    DemoOriginModel*        _origin;
    NSURL*                  _baseURL;
    NSMutableArray*         _activeRequests;
    NSOperationQueue*       _connectionQueue;
    
    // state
    BOOL _isEditMode;  // TODO not yet used
}

#pragma mark - Properties

//@synthesize users = _users;
@synthesize tableView = _tableView;
@synthesize editButton = _editButton;
@synthesize addButton = _addButton;
@synthesize cancelButton = _cancelButton;
@synthesize pullButton = _pullButton;

@synthesize origin = _origin;
@synthesize baseURL = _baseURL;
@synthesize activeRequests = _activeRequests;
@synthesize connectionQueue = _connectionQueue;



#pragma mark - Actions

- (IBAction)editObjects:(id)sender {
    [self toggleEditTableViewAnimated:YES];
}

- (IBAction)addObject:(id)sender {
    [self performSegueWithIdentifier:DemoAddUserSegueID sender:self];
}

- (IBAction)pullObjects:(id)sender {
    [self asyncFetchAndMergeUsers];
}

- (IBAction)cancelAction:(id)sender {
    for (id future in self.activeRequests) {
        [future cancel];
    }
    if (self.tableView.isEditing) {
        [self.tableView setEditing:NO animated:YES];
    }
}


#pragma mark - Properties

- (DemoOriginModel*) origin {
    if (_origin == nil) {
        _origin = [[DemoOriginModel alloc] initWithQueue:self.connectionQueue];
    }
    return _origin;
}


- (NSOperationQueue*) connectionQueue {
    if (_connectionQueue == nil) {        
#if !defined(BUG_iOS_setDelegateQueue)       
        _connectionQueue = [[NSOperationQueue alloc] init];
        _connectionQueue.maxConcurrentOperationCount = 2;
        _connectionQueue.name = @"com.ag.DemoApp.DemoOriginModelQueue";
#else
        _connectionQueue = [NSOperationQueue mainQueue];
#endif
    }
    return _connectionQueue;
}

- (NSMutableArray*) activeRequests {
    if (_activeRequests == nil) {
        _activeRequests = [[NSMutableArray alloc] init];
    }
    return _activeRequests;
}
   
   

- (NSMutableArray*) users {
    NSAssert([NSThread currentThread] == [NSThread mainThread], @"not execuing in main thread");
    if (_users == nil) {
        _users = [NSMutableArray array];
        // TODO: remove loop (for testing)
        //for (int i = 0; i < 3; ++i) {
            [self asyncFetchAndMergeUsers];
        //}
    }
    return _users;
}

- (void) setUsers:(NSArray*)array {
    NSAssert([NSThread currentThread] == [NSThread mainThread], @"not execuing in main thread");
    if (array != _users) {
        _users = [array mutableCopy];
    }
}


#pragma mark -


- (void) asyncFetchAndMergeUsers 
{
    DemoOriginModel* originModel = [[DemoOriginModel alloc] initWithQueue:self.connectionQueue];
    DemoJSONRequestFuture* future = nil;
    future = [originModel asyncFetchObjectsWithBaseURL:self.baseURL 
                                            entityName:@"User"
                                             predicate:nil 
                                     completionHandler:^(id json, NSError* error) {
                                         if (error) {
                                             NSLog(@"ERROR: %@", error);
                                             return;
                                         }
                                         [self mergeUsersWithObjects:json];
                                     }];
}

- (void) mergeUsersWithObjects:(NSArray*)objects
{
    NSAssert(objects != nil, @"objects is nil");
    NSAssert([objects isKindOfClass:[NSArray class]], @"objects is not an array");
    dispatch_async(dispatch_get_main_queue(), ^{
        [self setUsers:(id)objects];
        [self.tableView reloadData];
    });
}


#pragma mark -

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.baseURL = [[NSURL alloc] initWithString:@"http://ag-MacBookPro-2.local:3000"];
    //self.baseURL = [[NSURL alloc] initWithString:@"http://localhost:3000"];
	// Setup KVO
    [self addObserver:self forKeyPath:@"activeRequests" options:0 context:NULL];
    [self addObserver:self forKeyPath:@"users" options:0 context:NULL];
    
    // Setup TableView
    self.tableView.dataSource = self;
    self.tableView.delegate = self;

    // Setup initial state of buttons:
    NSUInteger usersCount = self.users.count; // triggers fetch request if users is not yet set
    BOOL isActive = [self.activeRequests count] > 0;
    self.editButton.enabled = !isActive && (usersCount > 0);
    self.addButton.enabled =!isActive;
    self.cancelButton.enabled = isActive;
    self.pullButton.enabled = !isActive;
    
}

- (void)viewDidUnload
{
    [super viewDidUnload];    
    [self setTableView:nil];
    [self setEditButton:nil];
    [self setAddButton:nil];
    [self setCancelButton:nil];
    [self setPullButton:nil];
    // Release any retained subviews of the main view.
    [self removeObserver:self forKeyPath:@"activeRequests"];
    [self removeObserver:self forKeyPath:@"users"];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}

#pragma mark - Observe Users

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if (object == self) {
        if ([@"activeRequests" isEqualToString:keyPath] || [@"users" isEqualToString:keyPath]) {
            dispatch_async(dispatch_get_main_queue(), ^{
                NSUInteger usersCount = self.users.count;
                BOOL isActive = NO; // [self.activeRequests count] > 0;   TODO: fix it!
                self.editButton.enabled = !isActive && (usersCount > 0);
                self.addButton.enabled =!isActive;
                self.cancelButton.enabled = isActive;
                self.pullButton.enabled = !isActive;
                // TODO: show/hide busy indicator
            });
        }
    }
}


#pragma mark - Edit TableView

- (void) toggleEditTableViewAnimated:(BOOL)animated {
    [self.tableView setEditing:![self.tableView isEditing] animated:animated];
}


#pragma mark - UITableViewDelegate


#pragma mark - UITableViewDataSource

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString* sUserCellIdentifier = @"DemoUserTableViewCell";
    DemoUserTableViewCell* cell = (DemoUserTableViewCell*)[tableView dequeueReusableCellWithIdentifier:sUserCellIdentifier];
    NSAssert(cell, @"cell is nil");
    NSDictionary* user = [self.users objectAtIndex:indexPath.row];
    NSString* name = [user objectForKey:@"name"];
    NSString* email = [user objectForKey:@"email"];
    cell.nameLabel.text = (id)name != (id)[NSNull null] ? name : @"<null>";
    cell.emailLabel.text = (id)email != (id)[NSNull null] ? email : @"<null>";
    id created_at = [user objectForKey:@"created_at"];
    cell.statusLabel.text = (created_at == [NSNull null] || created_at == nil) ? @"N" : @"";
    return cell;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    NSUInteger count = [self.users count];
    return count;
}

// Editing

// Ask which edit style is allowed
- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath {
    return UITableViewCellEditingStyleDelete;
}

// Commit editing
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    switch (editingStyle) {
        case UITableViewCellEditingStyleDelete: {
            NSDictionary* obj = (NSDictionary*)([self.users objectAtIndex:indexPath.row]);
            [self.users removeObjectAtIndex:indexPath.row];
            [self.tableView deleteRowsAtIndexPaths:@[indexPath]
                                  withRowAnimation:UITableViewRowAnimationFade];
            
            // If this is an originObject, delete it on the origin site:
            if ([(NSDictionary*)obj valueForKey:@"created_at"] != nil) {
                // TODO: fix delay
                double delayInSeconds = 2.0;
                dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
                dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                    [self deleteOriginObject:obj];  // this will invoke [tableView reloadData]
                });
            }                        
        }
            break;
            
        case UITableViewCellEditingStyleInsert: {
            NSDictionary* obj = (NSDictionary*)([self.users objectAtIndex:indexPath.row]);
            // Add object to local model:
            [self.users addObject:obj];
            [self.tableView insertRowsAtIndexPaths:@[indexPath]
                                  withRowAnimation:UITableViewRowAnimationFade];
            
            // Insert the object on the origin site:
            [self createOriginObject:obj];            
        }
            break;
             
        case UITableViewCellEditingStyleNone: 
            break;
    }
    return;
}


#pragma mark - Add Users

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:DemoAddUserSegueID]) {
        DemoAddUserViewController* addUserViewController = segue.destinationViewController;
        addUserViewController.delegate = self;
        return;
    }
    else {
        return;
    }
}


// DemoAddUserViewControllerDelegateProtocol

- (void) addUserViewControllerDidCancel:(DemoAddUserViewController*)viewController
{
    [self dismissViewControllerAnimated:YES completion: nil];
}

- (BOOL) addUserViewController:(DemoAddUserViewController*)viewController isValidObject:(id)object
{
    // object shall be a NSDictionary with two properties:
    //  1) "name" : (NSString*)     (required, not empty)
    //  2) "email": (NSString*)     (optional or nil)
    if (object) {
        NSDictionary* user = [object isKindOfClass:[NSDictionary class]] ? object : nil;
        id name;
        if ( (name = [user objectForKey:@"name"]) != nil && [name isKindOfClass:[NSString class]] && [name length] > 0) 
        {
            // name is valid.
        } 
        else {
            // Error - NSDictionary is not a user.
            NSLog(@"ERROR: 'name' is not valid");
            return NO;
        }
        id email;
        if ((email = [user objectForKey:@"email"]) != nil) {
            if ([email isKindOfClass:[NSString class]] && [email length] > 0) {
                // OK
            } else {
                NSLog(@"ERROR: 'email' is not valid");
                return NO;
            }
        }
    }
    else {
        NSLog(@"ERROR: 'user' is not valid or nil");
        return NO;
    }    
    
    return YES;
    
}


- (void) addUserViewController:(DemoAddUserViewController*)viewController didAcceptWithObject:(id)object
{
    NSAssert([NSThread currentThread] == [NSThread mainThread], @"not execuing in main thread");
    // object shall be a newly created object:
    NSAssert([self.users containsObject:object] == NO, @"modified object must be a new object");
    
    [self dismissViewControllerAnimated:YES completion: nil];

    // Add the new object to the local model:
    [self.users addObject:object];
    [self.tableView reloadData];

    // Add the new object to the origin model:
    // TODO: fix delay
    double delayInSeconds = 2.0;    
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [self createOriginObject:object];
    });
}


// Asynchronously create object on the origin site
- (void) createOriginObject:(id)object 
{
    NSAssert([_users containsObject:object] == YES, @"object is not contained in users" );
    // TODO: queue the future, in order to enable cancelation
    // Note: the completion handler will be executed on the connectionQueue.
    //DemoJSONRequestFuture* future = 
    [self.origin asyncCreateObject:object 
                  withBaseURL:self.baseURL 
                   entityName:@"User" 
            completionHandler:^(id result, NSError* error) {
                if (error == nil) {
                    // replace the local object with the origin object:
                    dispatch_async(dispatch_get_main_queue(), ^{
                        NSUInteger idx = [self.users indexOfObject:object];
                        if (idx != NSNotFound) {
                            [self.users replaceObjectAtIndex:idx withObject:result];
                            [self.tableView reloadData];
                        } else {
                            NSLog(@"ERROR: object is not contained in users");
                        }
                    });
                } else {
                    NSLog(@"ERROR: creating origin object failed: %@", error);
                }
            }];
}
     

// Asynchronously fetch object from the origin site
- (void) readOriginObject:(id)object 
{
    NSAssert([_users containsObject:object] == YES, @"object is not contained in users" );
    // TODO: queue the future, in order to enable cancelation
    // Note: the completion handler will be executed on the connectionQueue.
    //DemoJSONRequestFuture* future = 
    [self.origin asyncReadObject:object 
                  withBaseURL:self.baseURL 
                   entityName:@"User" 
            completionHandler:^(id result, NSError* error) {
                if (error == nil) {
                    // replace the old origin object with the newly fetched origin object:
                    dispatch_async(dispatch_get_main_queue(), ^{
                        NSUInteger idx = [self.users indexOfObject:object];
                        if (idx != NSNotFound) {
                            [self.users replaceObjectAtIndex:idx withObject:result];
                            [self.tableView reloadData];
                        } else {
                            NSLog(@"ERROR: object is not contained in users");
                        }
                    });
                } else {
                    NSLog(@"ERROR: reading origin object failed: %@", error);
                }
            }];
}

// Asynchronously update object on the origin site
- (void) updateOriginObject:(id)object 
{
    NSAssert([_users containsObject:object] == YES, @"object is not contained in users" );
    
    // TODO: queue the future, in order to enable cancelation
    // Note: the completion handler will be executed on the connectionQueue.
    //DemoJSONRequestFuture* future = 
    [self.origin asyncUpdateObject:object 
                withBaseURL:self.baseURL 
                 entityName:@"User" 
          completionHandler:^(id result, NSError* error) {
              if (error == nil) {
                  // Read origin object in order to update the local object
                  [self readOriginObject:object];
              } else {
                  NSLog(@"ERROR: updateing origin object failed: %@", error);
              }
          }];
}

// Asynchronously delete object on the origin site
- (void) deleteOriginObject:(id)object 
{    
    // TODO: queue the future, in order to enable cancelation
    // Note: the completion handler will be executed on the connectionQueue.
    //DemoJSONRequestFuture* future = 
    [self.origin asyncDeleteObject:object 
                  withBaseURL:self.baseURL 
                   entityName:@"User" 
            completionHandler:^(id result, NSError* error) {
                if (error == nil) {
                }
                else  {
                    NSLog(@"ERROR: deleting origin object failed: %@", error);
                }
            }];
}




@end

//
//  DemoAddUserViewController.m
//  DemoApp
//
//  Created by Andreas Grosam on 14.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoAddUserViewController.h"

@interface DemoAddUserViewController () <UITextFieldDelegate> 

@property (weak, nonatomic) IBOutlet UITextField *nameTextField;
@property (weak, nonatomic) IBOutlet UITextField *emailTextField;
@property (weak, nonatomic) UITextField *activeTextField;
@property (weak, nonatomic) IBOutlet UIBarButtonItem *saveUserButton;

@end

@implementation DemoAddUserViewController {
    __weak id<DemoAddUserViewControllerDelegateProtocol> _delegate;
}
@synthesize nameTextField = _nameTextField;
@synthesize emailTextField = _emailTextField;
@synthesize activeTextField = _activeTextField;
@synthesize saveUserButton = _saveUserButton;

@synthesize delegate = _delegate;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
    
    if ([self.delegate respondsToSelector:@selector(addUserViewControllerProtoypeObject:)])
    {
        NSDictionary* prototypeUser = [self.delegate addUserViewControllerProtoypeObject:self];
        NSString* name = [prototypeUser objectForKey:@"name"];
        NSString* email = [prototypeUser objectForKey:@"email"];
        
        self.nameTextField.text = name;
        self.emailTextField.text = email;
    }
    
    self.nameTextField.delegate = self;
    self.emailTextField.delegate = self;
}

- (void)viewDidUnload
{
    [self setNameTextField:nil];
    [self setEmailTextField:nil];
    [self setSaveUserButton:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void) viewWillAppear:(BOOL)animated {
    [self registerForKeyboardNotifications];
}

- (void) viewDidDisappear:(BOOL)animated {
    [self unregisterKeyboardNotifications];
}



#pragma mark - IBActions

- (IBAction)saveUser:(id)sender 
{
    NSDictionary* user = [[NSDictionary alloc] initWithObjectsAndKeys:
                          self.nameTextField.text, @"name", 
                          self.emailTextField.text, @"email",
                          nil];
    BOOL isValid = YES;
    if ([self.delegate respondsToSelector:@selector(addUserViewController:isValidObject:)]) {
        isValid = [self.delegate addUserViewController:self isValidObject:user];
    }
    if (isValid) {
        if (self.activeTextField) {
            [self resignFirstResponder];
        }
        [self.delegate addUserViewController:self didAcceptWithObject:user];
    }   
    else {
        // TODO: display error message in a modal view
    }
}

- (IBAction)cancel:(id)sender {
    [self.delegate addUserViewControllerDidCancel:self];
}


#pragma mark - Keyboard Handling

- (void)registerForKeyboardNotifications
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWasShown:)
                                                 name:UIKeyboardDidShowNotification object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillBeHidden:)
                                                 name:UIKeyboardWillHideNotification object:nil];
    
}

- (void)unregisterKeyboardNotifications
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}



// Called when the UIKeyboardDidShowNotification is sent.
- (void)keyboardWasShown:(NSNotification*)aNotification
{
#if 0 // not yet implememented    
    NSDictionary* info = [aNotification userInfo];
    CGSize kbSize = [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;
    
    UIEdgeInsets contentInsets = UIEdgeInsetsMake(0.0, 0.0, kbSize.height, 0.0);
    scrollView.contentInset = contentInsets;
    scrollView.scrollIndicatorInsets = contentInsets;
    
    // If active text field is hidden by keyboard, scroll it so it's visible
    // Your application might not need or want this behavior.
    CGRect aRect = self.view.frame;
    aRect.size.height -= kbSize.height;
    if (!CGRectContainsPoint(aRect, activeField.frame.origin) ) {
        CGPoint scrollPoint = CGPointMake(0.0, activeField.frame.origin.y-kbSize.height);
        [scrollView setContentOffset:scrollPoint animated:YES];
    }
#endif    
}

// Called when the UIKeyboardWillHideNotification is sent
- (void)keyboardWillBeHidden:(NSNotification*)aNotification
{
#if 0 // not yet implemented
    UIEdgeInsets contentInsets = UIEdgeInsetsZero;
    scrollView.contentInset = contentInsets;
    scrollView.scrollIndicatorInsets = contentInsets;
#endif    
}



#pragma mark - UITextFieldDelegate
- (void)textFieldDidBeginEditing:(UITextField *)textField {
    self.activeTextField = textField;
}

- (void)textFieldDidEndEditing:(UITextField *)textField {
    self.activeTextField = nil;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    // If the textField is the last text field, perform "Safe":
    if (textField == self.emailTextField) {   
        [self performSelector:@selector(saveUser:) onThread:[NSThread mainThread] 
                   withObject:textField waitUntilDone:NO];
        return NO;
    }
    else {
        // perform default behavior:
        return YES;
    }
}



@end

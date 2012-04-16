//
//  NSURLConnectionDownloadViewController.m
//  NSURLConnectionDownload
//
//  Created by Andreas Grosam on 9/10/11.
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

#import "NSURLConnectionDownloadViewController.h"

#import "JPJson/JPAsyncJsonParser.h"
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPSemanticActions.h"

#include <mach/mach_time.h>
#include <unistd.h>
#include <dispatch/dispatch.h>


#define WRITE_TO_TEMP_FILE


//#define RUN_INFINITE_LOOP


static NSError* makeError(NSString* domain, NSInteger code, NSString* errString) {
    // setup an error object:
    NSString* localizedErrStr = errString; //NSLocalizedString(errStr, errStr);
    NSArray* objectsArray = [[NSArray alloc] initWithObjects: localizedErrStr, nil];
    NSArray* keysArray = [[NSArray alloc] initWithObjects: NSLocalizedDescriptionKey, nil];            
    NSDictionary* userInfoDict = [[NSDictionary alloc] initWithObjects:objectsArray forKeys: keysArray];
    [objectsArray release];
    [keysArray release];        
    NSError* error = [NSError errorWithDomain:domain code:code userInfo:userInfoDict];
    [userInfoDict release];    
    return error;  //autoreleased
}

static uint64_t absoluteTimeToNanoseconds(uint64_t t) 
{    
    // If this is the first time we've run, get the timebase.
    // We can use denom == 0 to indicate that sTimebaseInfo is 
    // uninitialised because it makes no sense to have a zero 
    // denominator is a fraction.
    static mach_timebase_info_data_t sTimebaseInfo;
    
    if ( sTimebaseInfo.denom == 0 ) {
        (void) mach_timebase_info(&sTimebaseInfo);
    }
    uint64_t elapsedNano = t * sTimebaseInfo.numer / sTimebaseInfo.denom;
    return elapsedNano;
}



#pragma mark - NSURLConnectionDownloadViewController

@interface NSURLConnectionDownloadViewController ()


@property (nonatomic, retain) IBOutlet UIButton*    startDownloadButton;
@property (nonatomic, retain) IBOutlet UIButton*    cancelButton;
@property (nonatomic, retain) IBOutlet UILabel*     messageLabel;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView*  activityIndicatorView;
@property (nonatomic, retain) NSURLConnection*      connection;


#if !defined (WRITE_TO_TEMP_FILE)
@property (nonatomic, retain) JPAsyncJsonParser*    parser;
#else
@property (nonatomic, retain) NSURL*                tempFileURL;
@property (nonatomic, retain) NSFileHandle*         tempFileHandle;
#endif



- (void) startDownloadAndParse;
- (void) startConnectionInSecondaryThread;
- (void) startConnection;
- (void) handleError:(NSError*)error;

@end





@implementation NSURLConnectionDownloadViewController
{
    
    NSURLConnection*    connection_;    
    BOOL                runLoopDone_;
#if !defined (WRITE_TO_TEMP_FILE)
    JPAsyncJsonParser*  parser_;
#else
    NSFileHandle*       tempFileHandle_;
    NSURL*              tempFileURL_;
#endif    
    NSError*            lastError_;
    
    // stats:
    size_t              totalBytesDownloaded_;  // bytes donwloaded
    size_t              con_number_buffers_;    // number of buffer received by the connection
    size_t              numberDocuments_;       // number documents parsed per download
    uint64_t            t_start_;
    uint64_t            t_end_;    
}

static NSString* kDownloadConnectionRunLoopMode = @"MyViewControllerDownloadConnectionRunMode";

@synthesize connection = connection_;
@synthesize startDownloadButton = startDownloadButton_;
@synthesize cancelButton = cancelButton_;
@synthesize messageLabel = messageLabel_;
@synthesize activityIndicatorView = activityIndicatorView_;

#if !defined (WRITE_TO_TEMP_FILE)
@synthesize parser = parser_;
#else
@synthesize tempFileHandle = tempFileHandle_;
@synthesize tempFileURL = tempFileURL_;
#endif

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)dealloc
{
    [connection_ cancel];
    [connection_ release], connection_ = nil;    
    [lastError_ release], lastError_ = nil;

#if !defined (WRITE_TO_TEMP_FILE)    
    [parser_ cancel];
    [parser_ release], parser_ = nil;
#else    
    [tempFileHandle_ closeFile];
    [tempFileHandle_ release], tempFileHandle_ = nil;
    [tempFileURL_ release], tempFileURL_ = nil;
#endif
    [startDownloadButton_ release];
    [cancelButton_ release];
    [messageLabel_ release];
    [activityIndicatorView_ release];
        
    [super dealloc];
}


- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSAssert(startDownloadButton_, @"IBOutlet 'startDownloadButton_' is nil");
    NSAssert(cancelButton_, @"IBOutlet 'cancelButton_' is nil");
    NSAssert(messageLabel_, @"IBOutlet 'messageLable_' is nil");
    NSAssert(activityIndicatorView_, @"IBOutlet 'activityIndicatorView_' is nil");
    
    // Do any additional setup after loading the view from its nib.
    self.activityIndicatorView.hidesWhenStopped = YES;    
    self.messageLabel.text = @"";
    self.cancelButton.enabled = connection_ != nil;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
    self.startDownloadButton = nil;
    self.messageLabel = nil;
    self.activityIndicatorView = nil;
    self.cancelButton = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

#pragma mark -
#pragma mark Actions
- (void) startButtonTapped:(id)sender
{
    [self startDownloadAndParse];
}

- (void) cancelButtonTapped:(id)sender {

    if (connection_) {
        [connection_ cancel];
#if !defined (WRITE_TO_TEMP_FILE)        
        [parser_ cancel];
#endif        
        self.connection = nil;
        runLoopDone_ = YES;
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
        self.startDownloadButton.enabled = YES;
        self.cancelButton.enabled = NO;
    }
}


- (void) setupSemanticActions:(JPSemanticActions*) sa 
{
    sa.startJsonHandlerBlock = ^{ 
        ++numberDocuments_;
        //NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        //NSString* msg = [[NSString alloc] initWithFormat:@"Parser finished parsing document %d", 
        //                 numberDocuments_];
        //NSLog(@"%@", msg);
        //dispatch_async(dispatch_get_main_queue(), ^{
        //    self.messageLabel.text = msg;
        //});
        //[msg release];
        //[pool release];
    };
    sa.endJsonHandlerBlock = ^(id jsonContainer){ 
        dispatch_async(dispatch_get_main_queue(), ^{
            self.messageLabel.text = [NSString stringWithFormat:@"Received JSON document [%d]", numberDocuments_];
        });
    };
    sa.completionHandlerBlock = ^{ 
        t_end_ = mach_absolute_time();
        double secs = absoluteTimeToNanoseconds(t_end_ - t_start_) * 1.0e-9;
        NSString* msg = [[NSString alloc] initWithFormat:@"Parser finished in %g seconds.\n%d Documents parsed.\n%@", 
                         secs, numberDocuments_, 
                         (lastError_)?[NSString stringWithFormat:@"Parsing failed: %@",[lastError_ localizedDescription]]:@""];        
        NSLog(@"%@", msg);
#if !defined (WRITE_TO_TEMP_FILE)        
        self.parser = nil;
#endif        
        dispatch_async(dispatch_get_main_queue(), ^{
            self.messageLabel.text = msg;
            self.startDownloadButton.enabled = YES;
            self.cancelButton.enabled = NO;
            
#if defined (RUN_INFINITE_LOOP)            
            [self startButtonTapped:nil];
#endif            
        });  
        [msg release];
    };
    sa.errorHandlerBlock = ^(NSError* error){ 
        dispatch_async(dispatch_get_main_queue(), ^{
            lastError_ = [error retain];
            NSLog(@"NSURLConnectionDownloadViewController ERROR: %@", error);
        });
    };
    sa.parseMultipleDocuments = YES;
}


- (void) startDownloadAndParse
{
    // Disable the "Start Download" button, until we are ready to
    // download again:
    self.startDownloadButton.enabled = NO;
    
    // We must not invoke several connection attempts:
    if (connection_) {
        return;
    }

#if !defined (WRITE_TO_TEMP_FILE)    
    if (parser_) {
        // we are still parsing ..
        return;
    }
#endif
    
    // Create a parser, scheduled on the global concurrent queue.
    // By setting the handler dispatch queue to NULL, the delegate blocks will 
    // be serialized on the parser's thread.
    
    numberDocuments_ = 0;
    [lastError_ release];
    lastError_ = nil;
    
    
#if !defined (WRITE_TO_TEMP_FILE)        
    //
    //  Setup the parser and the semantic actions:
    //  
    parser_ = [[JPAsyncJsonParser alloc] init];
    [self setupSemanticActions:parser_.semanticActions];

#endif    
    
    
    // The connection's delegates will be scheduled on the thread where  
    // startConnection has been invoked. Since the parser's method parseBuffer:
    // may block until after the parser took this data to process it and possibly
    // until after the semantic actions handler blocks have been finshed (when they
    // are called synchronously with respect to the parser's thread) we shouldn't
    // run on the main thread, since this may take a while.
#if 1    
    // Now, start the NSURLConnection in a secondary thread:
    [NSThread detachNewThreadSelector:@selector(startConnectionInSecondaryThread) 
                             toTarget:self withObject:nil]; 
#else
    [self startConnection];
#endif    
    
}


- (void) startConnection
{
    // Note: startConnection can be performed on secondary threads, thus we need
    // to schedule UIKit methods onto the main thread.
    // 
    
    NSString* urlString = @"http://ag-MacBookPro.local:3000/users";
    NSURL* url = [NSURL URLWithString:urlString];
    
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
    //[[NSURLCache sharedURLCache] setMemoryCapacity:1024*64];   
    [[NSURLCache sharedURLCache] setMemoryCapacity:0];   
    
    NSTimeInterval request_timeout = 600.0;
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url cachePolicy:0 timeoutInterval:request_timeout];
    
    [request setValue:@"gzip,deflate" forHTTPHeaderField:@"Accept-Encoding"];
    NSLog(@"Request header: %@", [request allHTTPHeaderFields]);
    
#if 0 
    // iOS 5:
    //[request setNetworkServiceType:NSURLNetworkServiceTypeBackground];
#endif    
    
    // Create the URL connection and set its delegate:
    NSURLConnection* tmp = [[NSURLConnection alloc] initWithRequest:request 
                                                           delegate:self 
                                                   startImmediately:NO];
    self.connection = tmp;
    [tmp release];
    
    
    
    if (connection_ == nil) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self handleError:makeError(@"NSURLConnectionDownloadViewController", 3, @"Failure to create URL connection.")];
            self.messageLabel.text = @"creating connection failed";
            self.startDownloadButton.enabled = NO;
        });
        
        return;
    }
    
    NSLog(@"Start downloading %@", urlString);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.messageLabel.text = @"start connection request";
        // Start the status bar network activity indicator. We'll turn it off when 
        // the connection finishes or experiences an error.
        [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
        self.startDownloadButton.enabled = NO;
        self.cancelButton.enabled = YES;
    });
    
    
    // Schedule the connection's delegate methods in the current thread's run loop:
    //[connection_ scheduleInRunLoop: [NSRunLoop currentRunLoop] forMode: NSRunLoopCommonModes];
    [connection_ scheduleInRunLoop: [NSRunLoop currentRunLoop] forMode: kDownloadConnectionRunLoopMode];
    
    // Start the download
    [self.connection start];
}


- (void) startConnectionInSecondaryThread
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    [self startConnection];
    
    runLoopDone_ = NO;    
    
    // Enable the run loop:
    // first, add a dummy source in order to prevent the Run loop from exiting
    // immediately when the connection closes (the parser need to run on it as well):
    [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:kDownloadConnectionRunLoopMode];    
    do {
        BOOL processedSource = [[NSRunLoop currentRunLoop] runMode:kDownloadConnectionRunLoopMode beforeDate:[NSDate distantFuture]];
#if defined (XXX_DEBUG)        
        NSLog(@"RunLoop did process source: %@", ( processedSource ? @"YES" : @"NO"));
        if (runLoopDone_) {
            NSLog(@"Exit RunLoop.");
        }
#endif        
    } while (!runLoopDone_);
    
    [pool release];    
}



#pragma mark -
#pragma mark NSURLConnection Delegate

// For demo purposes, disable the cache:
- (NSCachedURLResponse *)connection:(NSURLConnection *)connection willCacheResponse:(NSCachedURLResponse *)cachedResponse {
    return nil;
}


- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    NSLog(@"ERROR: Connection did fail with error: %@", error);
    runLoopDone_ = YES;  // causes the run loop to exit    
    
    
#if defined (WRITE_TO_TEMP_FILE)        
    [tempFileHandle_ closeFile];
    self.tempFileHandle = nil;
#endif        
    
    
    dispatch_async(dispatch_get_main_queue(), 
    ^{
        self.cancelButton.enabled = NO;
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;  
        self.startDownloadButton.enabled = NO;
        if ([error code] == kCFURLErrorNotConnectedToInternet) {
           // if we can identify the error, we can present a more precise message to the user.
           NSDictionary* userInfo =
           [NSDictionary dictionaryWithObject:NSLocalizedString(@"No Connection Error", nil)
                                       forKey:NSLocalizedDescriptionKey];
           NSError* noConnectionError = [NSError errorWithDomain:NSCocoaErrorDomain
                                                            code:kCFURLErrorNotConnectedToInternet
                                                        userInfo:userInfo];
           [self handleError:noConnectionError];
        } else {
           // otherwise handle the error generically
           [self handleError:error];
        }    

        self.connection = nil;
    });
}


- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response 
{
    dispatch_async(dispatch_get_main_queue(), 
    ^{
        // check for HTTP status code for proxy authentication failures
        // anything in the 200 to 299 range is considered successful,
        // also make sure the MIMEType is correct:
        //
        NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse *)response;
        NSInteger statusCode = [httpResponse statusCode];
        NSString* mimeType = [response MIMEType];
        if (statusCode < 200 || statusCode > 300 || ![mimeType isEqual:@"application/json"]) 
        {
            NSDictionary* userInfo = 
            [NSDictionary dictionaryWithObject:NSLocalizedString(@"HTTP Error", nil)
                                        forKey:NSLocalizedDescriptionKey];
            NSError* error = [NSError errorWithDomain:@"HTTP" 
                                                 code:[httpResponse statusCode] 
                                             userInfo:userInfo];
            [self handleError:error];
        }
        
        NSLog(@"Connection did receive response: %@", [(NSHTTPURLResponse*)response allHeaderFields]);
        self.messageLabel.text = @"Connection established. Starting downloading ...";                
        self.cancelButton.enabled = YES;
    });    
    
    totalBytesDownloaded_ = 0;
    con_number_buffers_ = 0;
    t_start_ = mach_absolute_time();    
    
    
#if !defined (WRITE_TO_TEMP_FILE)
    [parser_ start];
#else
    if (tempFileHandle_ == nil) {
        NSError* error = nil;
        NSFileManager* fm = [[NSFileManager alloc] init];
        NSURL* userDocDirURL = [NSURL fileURLWithPath:[@"~/Documents" stringByExpandingTildeInPath] isDirectory:YES];
        
        NSAssert(userDocDirURL != nil, @"userDocDirURL is nil");
        self.tempFileURL = [userDocDirURL URLByAppendingPathComponent:@"file1.txt"];
        NSAssert(tempFileURL_, @"tempFileURL_ is nil");
        BOOL success = [fm createFileAtPath:[self.tempFileURL path] contents:nil attributes:nil];
        NSAssert(success == YES, @"createFileAtPath did fail");
        
        tempFileHandle_ = [[NSFileHandle fileHandleForWritingToURL:self.tempFileURL error:&error] retain];
        [fm release];
        NSAssert(tempFileHandle_ != nil, @"tempFileHandle_ is nil");
    }
    
    [tempFileHandle_ truncateFileAtOffset:0];
#endif    
}


- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{    
    size_t size = [data length];
    totalBytesDownloaded_ += size;
    ++con_number_buffers_;
    //NSLog(@"Connection did receive NSData[%ld](%p), size: %8.ld, total: %8.ld", 
    //      con_number_buffers_, data, size, totalBytesDownloaded_);

#if !defined (WRITE_TO_TEMP_FILE)    
    // Pass the data buffer to the parser
    [parser_ parseBuffer:data];
#else
    assert(tempFileHandle_);
    [tempFileHandle_ writeData:data];
#endif
}


- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{  
    NSString* msg = [[NSString alloc] initWithFormat:@"Connection did finish downloading %lu bytes", totalBytesDownloaded_];
    NSLog(@"%@", msg);
    self.connection = nil;
    runLoopDone_ = YES;    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.messageLabel.text = msg;
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;     
        self.cancelButton.enabled = NO;
    });    
    [msg release];
    
#if !defined (WRITE_TO_TEMP_FILE)        
    // Parse it piece by piece
    [parser_ parseBuffer:nil];    
#else    
    // Start the synchronous parser with that big chunk of NSData buffer on 
    // the global concurrent dispatch queue:
    uint64_t t = mach_absolute_time();
    double secs = absoluteTimeToNanoseconds(t - t_start_) * 1.0e-9;
    unsigned long long pos = [self.tempFileHandle seekToEndOfFile];
    NSLog(@"Finished writing temp file (%llu bytes) in %g s", pos, secs);
    [self.tempFileHandle closeFile];
    self.tempFileHandle = nil;
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfURL:self.tempFileURL
                                                     options:NSDataReadingMappedIfSafe | NSDataReadingUncached
                                                       error:&error];
        
        
        // Create a semantic actions object with default properties - including
        // its own serial dispatch queue where handler blocks will be scheduled.
        JPSemanticActions* sa = [[JPSemanticActions alloc] init];
        [self setupSemanticActions:sa];
        BOOL success = [JPJsonParser parseData:data semanticActions:sa];
        [sa release];
        [data release];
    });
#endif        
}


#pragma -

// Handle errors in the download by showing an alert to the user. 
- (void) handleError:(NSError*)error 
{
    NSString* errorMessage = [error localizedDescription];
    NSLog(@"ERROR: %@", error);
    
    UIAlertView* alertView =
    [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Error Title", nil)
                               message:errorMessage
                              delegate:nil
                     cancelButtonTitle:@"OK"
                     otherButtonTitles:nil];
    [alertView show];
    [alertView release];
}

@end

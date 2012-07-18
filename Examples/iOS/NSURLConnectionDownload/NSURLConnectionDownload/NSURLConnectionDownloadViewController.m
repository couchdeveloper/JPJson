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
#import "JPJson/JPRepresentationGenerator.h"

#include <mach/mach_time.h>
#include <unistd.h>
#include <dispatch/dispatch.h>


//#define DEBUG_LOG

//#define LOOP_INFINITELY

#if defined (DEBUG_LOG)
#define DLog(...) NSLog(@"%s %@", __PRETTY_FUNCTION__, [NSString stringWithFormat:__VA_ARGS__])
#else
#define DLog(...) do { } while (0)
#endif



//#define USE_SYNC_PARSER_WITH_TMP_FILE

// USE_ASYNC_DISPATCH_CONSUMER
// If defined, asynchronously dispatch the producer's data to the consumer (not recommended)
//#define USE_ASYNC_DISPATCH_CONSUMER

#if !defined (USE_ASYNC_PARSER) && !defined (USE_SYNC_PARSER_WITH_TMP_FILE)
    #define USE_ASYNC_PARSER
#endif

#if defined (USE_ASYNC_PARSER) && defined (USE_SYNC_PARSER_WITH_TMP_FILE)
    #error Can only use AsyncParser or SyncParser
#endif

#if defined (USE_ASYNC_PARSER)
    #define START_CONNECTION_IN_SECONDARY_THREAD
#elif defined (USE_SYNC_PARSER_WITH_TMP_FILE)
    #define WRITE_TO_TEMP_FILE
#endif

// Don't set these macros here - use #define USE_ASYNC_PARSER or define USE_SYNC_PARSER_WITH_TMP_FILE on top of the file!
//#define WRITE_TO_TEMP_FILE
//#define START_CONNECTION_IN_SECONDARY_THREAD


//#define LOOP_INFINITIVE

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
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
@property (atomic, retain) NSThread*                connectionThread;
#endif

#if !defined (WRITE_TO_TEMP_FILE)
@property (nonatomic, retain) JPAsyncJsonParser*    parser;
#else
@property (nonatomic, retain) NSURL*                tempFileURL;
@property (nonatomic, retain) NSFileHandle*         tempFileHandle;
#endif



- (void) startDownloadAndParse;
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
- (void) startConnectionInSecondaryThread;
#endif
- (void) startConnection;
- (void) handleError:(NSError*)error;

@end





@implementation NSURLConnectionDownloadViewController
{
    NSURLConnection*    connection_;    
    
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    BOOL                    runLoopDone_;
    NSThread*               connectionThread_;
#endif    
#if !defined (WRITE_TO_TEMP_FILE)
    JPAsyncJsonParser*  parser_;
#else
    NSFileHandle*       tempFileHandle_;
    NSURL*              tempFileURL_;
#endif    
    
#if defined (USE_ASYNC_DISPATCH_CONSUMER)    
    dispatch_queue_t    serial_queue_;
#endif    
    
    NSError*            lastError_;
    BOOL                canceled_;
    
    // stats:
    size_t              totalBytesDownloaded_;  // bytes donwloaded
    size_t              con_number_buffers_;    // number of buffer received by the connection
    size_t              numberDocuments_;       // number documents parsed per download
    
    uint64_t            t_start_;
    uint64_t            t_end_;    
}

static NSString* kDownloadConnectionRunLoopMode = @"MyViewControllerDownloadConnectionRunMode";
static NSString* kTempFileName = @"download.data";

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

#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
@synthesize connectionThread = connectionThread_;
#endif


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
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
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)        
    [connectionThread_ release]; connectionThread_ = nil;
#endif
 
#if defined (USE_ASYNC_DISPATCH_CONSUMER)        
    dispatch_release(serial_queue_);
#endif    
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
    
    // For testing:
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    
    
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
    
    [self.connection cancel];
    
#if !defined (WRITE_TO_TEMP_FILE)    
    [parser_ cancel];
    [parser_ release], parser_ = nil;
#endif    
    
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

#pragma mark - Actions
- (void) startButtonTapped:(id)sender {
    NSLog(@"startButtonTapped");
    [self startDownloadAndParse];
}

- (void) cancelButtonTapped:(id)sender  {    
    NSLog(@"cancelButtonTapped");
    [self cancel];
}



#pragma mark -

// Cancel the connection and the parser.
// As usual, canceling is tricky. The problem is, we need to access shared
// resources.
// Method cancel shall be invoked on the main thread!
- (void) cancel
{
    canceled_ = YES;
    // We need to cancel the connection *synchronously*, otherwise a possible
    // race condition could occur when the delegate methods (running on the 
    // secondary thread) access the file handle which will be closed in method
    // -resetDownloadAndParse running on the main thread.
    [self cancelConnectionWaitUntilDone:YES];
    
#if !defined (WRITE_TO_TEMP_FILE)
    if (self.parser) {
        [self.parser cancel];
        self.parser = nil;
        NSLog(@"JSON parser canceled by user");
    }
#endif                
    
    // Invoking this method requires that the connection and parser delegates do
    // not receive any messages and Blocks must not be called.
    [self resetDownloadAndParse];
    
    self.messageLabel.text = @"Download JSON canceled."; 
}

- (void) setupSemanticActions:(JPRepresentationGenerator*) sa 
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
    
    sa.endJsonHandlerBlock = ^(id jsonContainer) { 
        NSString* msg = [[NSString alloc] initWithFormat:@"Received JSON document [%zd]", numberDocuments_];
        DLog(@"%@", msg);
        dispatch_async(dispatch_get_main_queue(), ^{
            self.messageLabel.text = msg;
        });
        [msg release];
        
    };
    
    sa.completionHandlerBlock = ^{ 
        t_end_ = mach_absolute_time();
        double secs = absoluteTimeToNanoseconds(t_end_ - t_start_) * 1.0e-9;
        NSString* msg = [[NSString alloc] initWithFormat:@"Parser finished. Elapsed time: %g seconds.\n%zd Documents parsed.\n%@", 
                         secs, numberDocuments_, 
                         (lastError_)?[NSString stringWithFormat:@"Parsing failed: %@",[lastError_ localizedDescription]]:@""];        
        DLog(@"%@", msg);
#if !defined (WRITE_TO_TEMP_FILE)        
        self.parser = nil;
#endif    
        [self resetDownloadAndParse];
        dispatch_async(dispatch_get_main_queue(), ^{
            self.messageLabel.text = msg;
        });  
        [msg release];
        
        
#if defined (LOOP_INFINITELY)
        if (sa.error == nil) {
            dispatch_after(dispatch_time( DISPATCH_TIME_NOW,  10ULL*NSEC_PER_SEC), dispatch_get_main_queue(), ^{
                NSLog(@"Repeating download and parse due to option 'LOOP_INFINITIVELY'");
                [self startDownloadAndParse];
            });
        }
#endif        
        
    };

    sa.errorHandlerBlock = ^(NSError* error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            lastError_ = [error retain];
            NSLog(@"NSURLConnectionDownloadViewController ERROR: %@", error);
        });
        
    };
    sa.parseMultipleDocuments = YES;
}


- (void) startDownloadAndParse
{    
#if defined (USE_ASYNC_DISPATCH_CONSUMER)        
    if (serial_queue_ == NULL) {
        serial_queue_ = dispatch_queue_create("com.ag.connectionHandlerQueue", NULL);
    }
#endif    
    
    // Method startDownloadAndParse shall be invoked on the main thread!
    
    // We must not invoke several connection attempts:
    // verify if we can access the shared resources (connection, Run Loop, parser, temp file)
    
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    if (self.connectionThread != nil) {
        NSLog(@"connection attempt refused because a Run Loop for connection is still in use");
        return;
    }
#endif    
        
#if !defined (WRITE_TO_TEMP_FILE)
    if (parser_)  {
        NSLog(@"connection attempt refused because a parser (%@) is still in use", parser_);
        return;
    }
#else        
    if (tempFileHandle_) 
    {    
        NSLog(@"connection attempt refused because temporary file (%@) is still in use", tempFileHandle_);
        return;
    }
#endif        
    
    // Here we assume we can establish a new connection, possibly create 
    // a tempfile and start a parser:    
    NSAssert(connection_ == nil, @"connection_ is not nil");
#if defined (WRITE_TO_TEMP_FILE)    
    NSAssert(tempFileHandle_ == nil, @"tempFileHandle is not nil");
#endif    
#if !defined (WRITE_TO_TEMP_FILE)    
    NSAssert(parser_ == nil, @"parser_ is not nil");
#endif    
    
    self.messageLabel.text = @"Starting JSON download...";         

    
    numberDocuments_ = 0;
    [lastError_ release];
    lastError_ = nil;    
    
#if !defined (WRITE_TO_TEMP_FILE)        
    // Create a parser, scheduled on the global concurrent queue.
    // By setting the handler dispatch queue to NULL, the delegate blocks will 
    // be serialized on the parser's thread.
    //
    //  Setup the parser and the semantic actions:
    //  
    parser_ = [[JPAsyncJsonParser alloc] init];
    [self setupSemanticActions:(JPRepresentationGenerator*)(parser_.semanticActions)];

#endif    
    
    
    // The connection's delegates will be scheduled on the thread where  
    // startConnection has been invoked. Since the parser's method parseBuffer:
    // may block until after the parser took this data to process it and possibly
    // until after the semantic actions handler blocks have been finshed (when they
    // are called synchronously with respect to the parser's thread) we shouldn't
    // run on the main thread, since this may take a while.
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    // Now, start the NSURLConnection in a secondary thread:
    NSThread* tmp = [[NSThread alloc] initWithTarget:self selector:@selector(startConnectionInSecondaryThread) object:nil];
    self.connectionThread = tmp;
    [tmp release];
    [connectionThread_ start];
#else
    [self startConnection];
#endif    
    
}



- (void) startConnection
{
    // Caution: do not invoke startConnection directly, use downloadAndParse method
    
    // Note: startConnection may be possibly executed on secondary threads, thus we need
    // to schedule UIKit methods onto the main thread.

    NSAssert(connection_ == nil, @"connection_ is not nil");
    
    NSString* urlString = @"http://ag-MacBookPro.local:3000/download/download.json";
    NSURL* url = [NSURL URLWithString:urlString];
    
    // Clear the URL cache:
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
    //[[NSURLCache sharedURLCache] setMemoryCapacity:1024*64];   
    [[NSURLCache sharedURLCache] setMemoryCapacity:0];   
    
    // Setup the URL request:
#if defined (DEBUG)    
    NSTimeInterval request_timeout = 30;
#else    
    NSTimeInterval request_timeout = 60;
#endif    
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url 
                                                           cachePolicy:NSURLRequestUseProtocolCachePolicy/*NSURLRequestReloadIgnoringLocalAndRemoteCacheData */
                                                       timeoutInterval:request_timeout];
    [request setValue:@"application/json" forHTTPHeaderField:@"Accept"];
    [request setValue:@"gzip,deflate" forHTTPHeaderField:@"Accept-Encoding"];    
    NSLog(@"starting connection with request headers: %@", [request allHTTPHeaderFields]);
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
        });
        [self resetDownloadAndParse];
        return;
    }
    NSLog(@"Start downloading %@", urlString);
    
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)
    // Start the status bar network activity indicator. We'll turn it off when 
    // the connection finishes or experiences an error.
    dispatch_async(dispatch_get_main_queue(), ^{
        self.messageLabel.text = @"start connection request";
        [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
        [self.activityIndicatorView startAnimating];
        self.startDownloadButton.enabled = NO;
        self.cancelButton.enabled = YES;
    });
#else
    self.messageLabel.text = @"start connection request";
    [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
    [self.activityIndicatorView startAnimating];
    self.startDownloadButton.enabled = NO;
    self.cancelButton.enabled = YES;
#endif    
    
    // Schedule the connection's delegate methods in the current thread's run loop:
    // If we are on the main thread, we use NSRunLoopCommonModes in order to enable
    // the connection's delegates to run not only on the default mode, which would 
    // possibly disable the connection events. Otherwise, we use kDownloadConnectionRunLoopMode
    // since a secondary thread is dedicated to run the connection delegates.
#if !defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    [connection_ scheduleInRunLoop: [NSRunLoop currentRunLoop] forMode: NSRunLoopCommonModes];
#else    
    [connection_ scheduleInRunLoop: [NSRunLoop currentRunLoop] forMode:kDownloadConnectionRunLoopMode];
#endif    
    // Start the download
    [self.connection start];
}


- (void) cancelConnection_private
{
    if (self.connection) {
        NSLog(@"Attempt to cancel the connection ...");
        [self.connection cancel];
        self.connection = nil;
        NSLog(@"... cancel returned.");
    }
    else {
        NSLog(@"No connection to cancel");
    }
}

- (void) cancelConnectionWaitUntilDone:(BOOL)wait 
{
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)  
    [self cancelConnection_private];
    [self stopConnectionRunLoopWaitUntilDone:wait];
#else
    [self cancelConnection_private];
#endif    
}

//#define XXX_DEBUG
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
- (void) startConnectionInSecondaryThread
{
    // Caution: do not invoke startConnectionInSecondaryThread directly, use downloadAndParse method
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    [self startConnection];    
    runLoopDone_ = NO;    
    // Enable the run loop:
    // first, add a dummy source in order to prevent the Run loop from exiting
    // immediately when the connection closes (the parser need to run on it as well):
    [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:kDownloadConnectionRunLoopMode];    
    do {
#if defined (XXX_DEBUG)        
        BOOL processedSource = 
#endif        
        [[NSRunLoop currentRunLoop] runMode:kDownloadConnectionRunLoopMode 
                                 beforeDate:[NSDate dateWithTimeIntervalSinceNow:10]];
#if defined (XXX_DEBUG)        
        NSLog(@"RunLoop did process source: %@", ( processedSource ? @"YES" : @"NO"));
        if (runLoopDone_) {
            NSLog(@"Exit RunLoop.");
        }
#endif        
    } while (!runLoopDone_);
    
    self.connectionThread = nil;
    
#if defined (DEBUG)    
    NSLog(@"Exiting Run Loop for connection");
#endif    
    [pool release];    
}


- (void) stopConnectionRunLoop_private {
    if (self.connection) {
        NSLog(@"cannot stop Run Loop: connection is still active");
        return;
    }
    runLoopDone_ = YES;    
}

// In order to cause the Run Loop to exit, send it a message. The Run Loop
// exits after executing the "event" ‘stopConnectionRunLoop_private‘, This
// happens to set `runLoopDone_` to YES which causes the outer loop to finish
// and the thread eventually terminates.
- (void) stopConnectionRunLoopWaitUntilDone:(BOOL)wait
{    
    if (self.connectionThread == nil) {
        return;
    }
    NSLog(@"stopConnectionRunLoopWaitUntilDone %@ with thread %@", wait?@"YES":@"NO", self.connectionThread);
    // The mode shall match the mode set in the Run Loop!
    [self performSelector:@selector(stopConnectionRunLoop_private) 
                 onThread:connectionThread_
               withObject:nil
            waitUntilDone:wait
                    modes:[NSArray arrayWithObject:kDownloadConnectionRunLoopMode]];
}

#endif    



#pragma mark -
#pragma mark NSURLConnection Delegate

// For demo purposes, disable the cache:
//- (NSCachedURLResponse *)connection:(NSURLConnection *)connection 
//                  willCacheResponse:(NSCachedURLResponse *)cachedResponse {
//    return nil;
//}


- (NSURLRequest *)connection:(NSURLConnection *)connection 
             willSendRequest:(NSURLRequest *)request 
            redirectResponse:(NSURLResponse *)redirectResponse
{
    NSLog(@"connection will send request: %@\nheaders: %@", request, [request allHTTPHeaderFields]);
    return request;
}


- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    NSLog(@"ERROR: Connection did fail with error: %@", error);
    
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)            
    [self stopConnectionRunLoopWaitUntilDone:NO];
#endif    

#if !defined (WRITE_TO_TEMP_FILE)        
    if (parser_) {
        [parser_ cancel];
        self.parser = nil;
        NSLog(@"JSON parser canceled due to a connection error");
    }
#endif    
    [self resetDownloadAndParse];
    
    dispatch_async(dispatch_get_main_queue(), 
    ^{
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
    });
}


- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response 
{
    // check for HTTP status code for proxy authentication failures
    // anything in the 200 to 299 range is considered successful,
    // also make sure the MIMEType is correct:
    NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse *)response;
    NSInteger statusCode = [httpResponse statusCode];
    NSLog(@"Connection did receive response: status code: %d, headers: %@", 
          statusCode, [(NSHTTPURLResponse*)response allHeaderFields]);
    NSString* mimeType = [response MIMEType];
    if (statusCode < 200 || statusCode > 300 || ![mimeType isEqual:@"application/json"]) 
    {
        NSDictionary* userInfo = 
        [NSDictionary dictionaryWithObject:NSLocalizedString(@"HTTP Error", nil)
                                    forKey:NSLocalizedDescriptionKey];
        NSError* error = [NSError errorWithDomain:@"HTTP" 
                                             code:[httpResponse statusCode] 
                                         userInfo:userInfo];
        dispatch_async(dispatch_get_main_queue(), 
                       ^{
                           self.messageLabel.text = @"Connection established. Starting downloading ...";                
                           [self handleError:error];
                       });  
        [self resetDownloadAndParse];
                
        return;
    }
    
    totalBytesDownloaded_ = 0;
    con_number_buffers_ = 0;
    t_start_ = mach_absolute_time();    
    
    
#if !defined (WRITE_TO_TEMP_FILE)
    [parser_ start];
#else
    if (tempFileHandle_ == nil) {
        [self makeTemporaryFileSync];
    }
    else {
        [tempFileHandle_ truncateFileAtOffset:0];
    }
#endif    
}


- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{    
    size_t size = [data length];
    totalBytesDownloaded_ += size;
    ++con_number_buffers_;
    NSString* msg = [[NSString alloc] initWithFormat:@"NSData[%ld](%p), size: %8.ld, total: %8.ld", 
                     con_number_buffers_, data, size, totalBytesDownloaded_];
    NSLog(@"%@", msg);
    [msg release];
    
#if defined (DEBUG)
    NSString* dmsg = [[NSString alloc] initWithFormat:@"Received data with size: %8.ld, total: %8.ld", 
                     con_number_buffers_, data, size, totalBytesDownloaded_];
    
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.messageLabel.text = dmsg;
    });
#else
    self.messageLabel.text = msg;    
#endif    
    [dmsg release];
#endif    
#if !defined (WRITE_TO_TEMP_FILE)    
    // Pass the data buffer to the parser
#if !defined (USE_ASYNC_DISPATCH_CONSUMER)
    [parser_ parseBuffer:data];
#else    
    dispatch_async(serial_queue_, ^{
        [parser_ parseBuffer:data];
    });
#endif    
#else
    assert(tempFileHandle_);
#if !defined (USE_ASYNC_DISPATCH_CONSUMER)
    [tempFileHandle_ writeData:data];
#else    
    dispatch_async(serial_queue_, ^{
        [tempFileHandle_ writeData:data];
    });
#endif    
#endif
}


- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{ 
    uint64_t t = mach_absolute_time();
    double secs = absoluteTimeToNanoseconds(t - t_start_) * 1.0e-9;
    
    NSString* msg = [[NSString alloc] initWithFormat:@"Connection did finish downloading %lu bytes after %.2f sec", 
                     totalBytesDownloaded_, secs];
    NSLog(@"%@", msg);
    
    self.connection = nil;
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    [self stopConnectionRunLoopWaitUntilDone:NO];
#endif  
    
#if defined (START_CONNECTION_IN_SECONDARY_THREAD)    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.messageLabel.text = msg;
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;     
    });    
#else
    self.messageLabel.text = msg;
    [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;     
#endif    
    [msg release];

    
#if !defined (WRITE_TO_TEMP_FILE)        
    // stop the parser
    
#if !defined (USE_ASYNC_DISPATCH_CONSUMER)    
    [parser_ parseBuffer:nil];    
#else    
    dispatch_async(serial_queue_, ^{    
        [parser_ parseBuffer:nil];    
    });
#endif    
#else    
    // We can close and release the file handle
    unsigned long long pos = [self.tempFileHandle seekToEndOfFile];
    [self.tempFileHandle closeFile];
    self.tempFileHandle = nil;
    t = mach_absolute_time();
    secs = absoluteTimeToNanoseconds(t - t_start_) * 1.0e-9;
    DLog(@"Finished writing temp file (%llu bytes) in %g s", pos, secs);
    
    // Start a synchronous JSON parser with a big chunk of NSData buffer which 
    // is memory mapped on the temporay file and execute it on the global 
    // concurrent dispatch queue:
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), 
    ^{
        NSAssert(self.tempFileURL != nil, @"tempFileURL is nil");
        NSLog(@"%@", self.tempFileURL); 
        NSFileManager* fm = [[NSFileManager alloc] init];
        bool canAccessTempFile = [fm fileExistsAtPath:[tempFileURL_ path]];
        [fm release];
        if (!canAccessTempFile) {
            NSLog(@"[NSFileManager] fileExistsAtPath returned NO with path %@", [tempFileURL_ path]);
            abort();
        }
        NSAssert(canAccessTempFile, @"could not access temporary file");
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfURL:self.tempFileURL
                                                     options:NSDataReadingMappedIfSafe | NSDataReadingUncached
                                                       error:&error];
        // Create a semantic actions object with default properties - including
        // its own serial dispatch queue where handler blocks will be scheduled.
        JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] init];
        [self setupSemanticActions:sa];
        BOOL success = [JPJsonParser parseData:data semanticActions:sa];
        if (!success) {
            NSLog(@"JSON Parser Error: %@", sa.error); 
            // Further error handling is performed in sa's handler blocks.
        }            
        [sa release];
        [data release];
        [self resetDownloadAndParse];
        
    });
#endif        
}


#pragma mark - Private Methods


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





- (void) resetDownloadAndParse
{
    // The connection object shall have received either a cancel message
    // or it shall have sent the delegate a connection:didFailWithError:
    // or connectionDidFinishLoading.
    // A JSON parser shall not run.
    
    dispatch_async(dispatch_get_main_queue(), 
    ^{
        self.connection = nil;
        
#if !defined (WRITE_TO_TEMP_FILE)        
        NSAssert(parser_ == nil, @"parser_ is not nil");
#endif   
        
        
#if defined (WRITE_TO_TEMP_FILE)                
        if (tempFileHandle_) {
            [tempFileHandle_ closeFile];
            [tempFileHandle_ release]; tempFileHandle_ = nil;        
        }
        NSFileManager* fm = [[NSFileManager alloc] init];        
        if ([fm fileExistsAtPath:[tempFileURL_ path]]) {
            NSError* error;
            BOOL result = [fm removeItemAtURL:tempFileURL_ error:&error];
            if (!result) {
                NSLog(@"[NSFileManager] removeItemAtURL did fail with error %@", error);
            }
            NSAssert(result, @"could not remove temporary file");
            DLog(@"TempFile removed");
        }
        [fm release];
#endif        
        
        self.startDownloadButton.enabled = YES; 
        self.cancelButton.enabled = NO;
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
        [self.activityIndicatorView stopAnimating];
        
#if defined (LOOP_INFINITIVE)
        if (!canceled_) {
            [self startButtonTapped:nil];
        }
        canceled_ = NO;
#endif        
    });      
}



#pragma mark -
#if defined (WRITE_TO_TEMP_FILE)

- (void) makeTemporaryFileSync
{
    if (tempFileHandle_) {
      [tempFileHandle_ closeFile];
      [tempFileHandle_ release]; tempFileHandle_ = nil;        
    }

    NSFileManager* fm = [[NSFileManager alloc] init];    

    // Get the URL for a temp file
    if (tempFileURL_ == nil) {
      NSURL* tmpDirURL = [[fm URLsForDirectory:NSCachesDirectory inDomains:NSUserDomainMask] lastObject];
      tempFileURL_ = [[tmpDirURL URLByAppendingPathComponent:kTempFileName] retain];
      NSAssert(tempFileURL_ != nil, @"Could not locate temp file");
    }
    // Create the temporary file:
    // If a file already exists at path, this method overwrites the contents of that file
    BOOL success = [fm createFileAtPath:[tempFileURL_ path] contents:nil attributes:nil];
    if (!success) {
        NSLog(@"[NSFileManager] createFileAtPath did fail with path: %@", [tempFileURL_ path]);
    }
    NSAssert(success == YES, @"createFileAtPath did fail");

    // Create a file handle
    NSError* error;
    tempFileHandle_ = [[NSFileHandle fileHandleForWritingToURL:tempFileURL_ error:&error] retain];
    [fm release];
    NSAssert(tempFileHandle_ != nil, @"temporary file handle is nil");
    DLog(@"TempFileHandle created");
}

- (void) removeTemporaryFile 
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (tempFileURL_ == nil) {
            return;
        }
        if (tempFileHandle_) {
            [tempFileHandle_ closeFile];
            [tempFileHandle_ release]; tempFileHandle_ = nil;        
        }
        NSFileManager* fm = [[NSFileManager alloc] init];        
        if ([fm fileExistsAtPath:[tempFileURL_ path]]) {
            NSError* error;
            BOOL result = [fm removeItemAtURL:tempFileURL_ error:&error];
            if (!result) {
                NSLog(@"[NSFileManager] removeItemAtURL did fail with error: %@", error);
            }
            NSAssert(result, @"could not remove temporary file");
        }
        [fm release];
    });      
}
#endif



@end

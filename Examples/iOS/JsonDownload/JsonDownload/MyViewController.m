//
//  MyViewController.m
//  JsonDownload
//
//  Created by Andreas Grosam on 7/6/11.
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

#import "MyViewController.h"
#import "JPJson/JPJsonParser.h"

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#include <dispatch/dispatch.h>

/*
 links
 http://developer.apple.com/library/ios/#qa/qa1712/_index.html
 
 */


#define USE_DUMMY_PARSER


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



#if defined (USE_DUMMY_PARSER)
#pragma mark - DummyParser

typedef void (^DummyParser_CompletionBlockType)(void);
typedef void (^DummyParser_DataHandlerBlockType)(id);
typedef void (^DummyParser_ErrorHandlerBlockType)(NSError*);

@interface DummyParser : NSObject 
{
    JPNSDataBuffers*    buffersQueue_;
    dispatch_queue_t    workerQueue_;    
    dispatch_queue_t    handlerQueue_;    
    size_t              bufferCount_;
    size_t              totalSize_;
    NSMutableArray*     result_;
    
    DummyParser_CompletionBlockType             completionBlock_;
    DummyParser_DataHandlerBlockType            dataHandlerBlock_;
    DummyParser_ErrorHandlerBlockType           errorHandlerBlock_;    
}

-(id) initWithBuffersQueue:(JPNSDataBuffers*)buffersQueue 
       wokerDispatchQueue:(dispatch_queue_t)workerQueue
     handlerDispatchQueue:(dispatch_queue_t)handlerQueue;

@property (copy) DummyParser_CompletionBlockType completionBlock;
@property (copy) DummyParser_DataHandlerBlockType dataHandlerBlock;
@property (copy) DummyParser_ErrorHandlerBlockType errorHandlerBlock;

- (void) start;
- (void) cancel;

@end


@implementation DummyParser 

@synthesize completionBlock = completionBlock_;
@synthesize dataHandlerBlock = dataHandlerBlock_;
@synthesize errorHandlerBlock = errorHandlerBlock_;


-(id) initWithBuffersQueue:(JPNSDataBuffers*)buffersQueue 
        wokerDispatchQueue:(dispatch_queue_t)workerQueue
      handlerDispatchQueue:(dispatch_queue_t)handlerQueue
{
    self = [super init];
    if (self) {
        assert(buffersQueue);
        assert(workerQueue);
        buffersQueue_ = [buffersQueue retain];
        workerQueue_ = workerQueue; dispatch_retain(workerQueue);
        handlerQueue_ = handlerQueue; 
        if (handlerQueue) {
            dispatch_retain(handlerQueue);
        }
    }
    return self;
}

- (void) dealloc {
    [buffersQueue_ release], buffersQueue_ = nil;
    if (workerQueue_) {
        dispatch_release(workerQueue_);
    }
    if (handlerQueue_) {
        dispatch_release(handlerQueue_);
    }
    [completionBlock_ release];
    [dataHandlerBlock_ release];
    [errorHandlerBlock_ release];
    [result_ release];
}


- (void) start
{
    dispatch_async(workerQueue_, 
    ^{
        result_ = [[NSMutableArray alloc] init];
        
        bufferCount_ = 0;
        totalSize_ = 0;
        bool done = false;
        uint32_t c = 0;
        uint32_t n = 0;
        while (!done) 
        {  
            //size_t availl1 = [buffersQueue_ avail];
            NSData* buffer = [buffersQueue_ aquireBuffer];
            if (buffer == nil) {
                NSLog(@"DummyParser: aquire returned nil, retry ...");
                // We do this until the connection closes itself and issues
                // an error.
            }
            else {
                //size_t avail2 = [buffersQueue_ avail];
                size_t size = [buffer length];
                ++bufferCount_;
                totalSize_ += size;
                NSLog(@"DummyParser: processing data buffer[%ld] NSData %p, buffer: %p, size: %ld",                   
                      bufferCount_, buffer, [buffer bytes], size);
                
                // Simulate work:
                uint64_t t0 = mach_absolute_time();
                int x = 0;
                const char* p = [buffer bytes];
                for (int i = 0; i < size; ++i) {
                    x += *p++;
                    ++c;
                    if ((c % 25000) == 0) {
                        // Notify client that we processed a some data:
                        if (dataHandlerBlock_) {
                            if (handlerQueue_) {
                                dispatch_async(handlerQueue_, 
                               ^{
                                   dataHandlerBlock_(result_);
                               });
                            } else {
                                dataHandlerBlock_(result_);
                            }    
                        }
                        [result_ release];
                        result_ = [[NSMutableArray alloc] init];
                    }
                    ++n;
                    if ((n % 20) == 0) {
                        NSString* s = [[NSString alloc] initWithFormat:@"a string %lu", n];
                        [result_ addObject:s];
                        [s release];
                    }
                }
                uint64_t t1 = mach_absolute_time();
                double t = absoluteTimeToNanoseconds(t1 - t0)*1.0e-9;
                NSLog(@"result of processing: %d, time: %gs, %g kB/s", 
                      x, t, (size/1024.0)/t);
                                
                done = size == 0;
                [buffersQueue_ commitBuffer:buffer];            
                if (size == 0) {
                    NSLog(@"DummyParser: received last buffer, finished.");
                }
            }
        }
        
        // Notify client that we processed a some (remaining) data
        if (result_ && dataHandlerBlock_) {
            if (handlerQueue_) {
                dispatch_async(handlerQueue_, 
               ^{
                   dataHandlerBlock_(result_);
               });
            } else {
                dataHandlerBlock_(result_);
            }    
        }
        [result_ release];
        result_ = nil;
        
        // Notify client that we finished:
        if (completionBlock_) {
            if (handlerQueue_) {
                dispatch_async(handlerQueue_, 
               ^{
                   completionBlock_();
               });
            } else {
                completionBlock_();
            }
        }
        
    });
}

- (void) cancel
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
        [buffersQueue_ clear];
        while (![buffersQueue_ produceBuffer:nil]) {
            [buffersQueue_ clear];        
        }
    });
}


@end


#endif // USE_DUMMY_PARSER


#pragma mark - MyViewController

static NSString* kDownloadConnectionRunLoopMode = @"MyViewControllerDownloadConnectionRunMode";

@interface MyViewController ()


@property (nonatomic, retain) IBOutlet UIButton* startDownloadButton;
@property (nonatomic, retain) IBOutlet UILabel*  messageLabel;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView*  activityIndicatorView;

@property (nonatomic, retain) NSURLConnection* connection;
#if defined (USE_DUMMY_PARSER)
@property (nonatomic, retain) DummyParser* parser;
#else
@property (nonatomic, retain) JPAsyncJsonParser* parser;
#endif

- (void) startDownloadAndParse;
- (void) startConnectionInSecondaryThread;
- (void) startConnection;
- (void) handleError:(NSError*)error;
#if defined (USE_DUMMY_PARSER)
- (void) setupDummyParser;
#else
- (void) setupParser;
#endif
@end



@implementation MyViewController {
#if defined (USE_DUMMY_PARSER)    
    DummyParser* parser_;
#else    
    JPAsyncJsonParser*  parser_;    
#endif    
}

@synthesize parser = parser_;
@synthesize bufferQueue = bufferQueue_;
@synthesize connection = connection_;
@synthesize startDownloadButton = startDownloadButton_;
@synthesize messageLabel = messageLabel_;
@synthesize activityIndicatorView = activityIndicatorView_;


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
    [parser_ release], parser_ = nil;
    [connection_ release], connection_ = nil;
    [bufferQueue_ release], bufferQueue_ = nil;
    dispatch_release(parser_handler_queue_);
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
    NSAssert(messageLabel_, @"IBOutlet 'messageLable_' is nil");
    NSAssert(activityIndicatorView_, @"IBOutlet 'activityIndicatorView_' is nil");

    // Do any additional setup after loading the view from its nib.
    self.activityIndicatorView.hidesWhenStopped = YES;    
    self.messageLabel.text = @"";
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
    self.startDownloadButton = nil;
    self.messageLabel = nil;
    self.activityIndicatorView = nil;
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


#if !defined (USE_DUMMY_PARSER)
- (void) setupParser 
{
    // setup the semantic actions:
    JPSemanticActions* sa = [[JPSemanticActions alloc] init];
    sa.parseMultipleDocuments = YES;
    
    // Setup the three handler blocks for the semantic actions object:
    //      1. completionBlock
    //      2. jsonObjectHandlerBlock
    //      3. errorHandlerBlock    
    // The blocks will be scheduled on the queue we specify:
    if (parser_handler_queue_ == NULL) {
        // Create a serial queue where we execute the parser's handler:
        // (We could also use the main queue, but then we would block the main
        // thread if we perform long operations, e.g. when processing the 
        // json container)
        parser_handler_queue_ = dispatch_queue_create("com.JsonDownload.parser_handler", NULL);
    }
    [sa setDispatchQueue:parser_handler_queue_];
    
    // The jsonObjectHandlerBlock is called when the parser has succesully parsed a 
    // Json text and was able to create a corresponding json container which is passed 
    // in argument result.
    // If the option to parse multiple json documents is set in the semantics actions
    // object, this block can be called more than one times. 
    // If parseMultipleDocumentsAsynchronously returns NO (the default) the parser's 
    // thread is blocked until the handler finishes. This is a measurement to throttle
    // the process of generating json containers which may tie up a large amount of
    // system resources. Otherwise, if parseMultipleDocumentsAsynchronously returns 
    // YES, the parser will immediately continue to parse the input source on its
    // own thread. Be carefully, when considering parseMultipleDocumentsAsynchronously
    // set to YES.
    //    typedef void (^JPSemanticActions_JsonObjectHandlerBlockType)(id result);
    sa.jsonObjectHandlerBlock = ^(id json){
        ++numberJsonDocuments_;
        int retained = [json retainCount];
        NSString* msg = [[NSString alloc] initWithFormat:@"Did receive json %p #%d, retainCount: %d", 
                         json, numberJsonDocuments_, retained];
        NSLog(@"%@", msg);
        dispatch_async(dispatch_get_main_queue(), ^{
            self.messageLabel.text = msg;
        });
        [msg release];
        
        // simulate slow processing of json 
        //usleep(1*1000);
    };
    
    // The errorHandlerBlock is called when an error occured during parsing. Depending 
    // on the semantic actions implementation, an error may not necessarily stop the 
    // parser from parsing additional json documents.
    //    typedef void (^JPSemanticActions_ErrorHandlerBlockType)(NSError*);
    sa.errorHandlerBlock = ^(NSError* error){
        NSLog(@"Parser did fail with error %@", error);        
        dispatch_async(dispatch_get_main_queue(), ^{
            [self handleError:error];    
        });
    };
    
    // The completionBlock is called when the parser finished parsing. This block 
    // is always called regardless whether there was a previous error and the 
    // errorHandlerBlock has been called previously.
    // typedef void (^JPSemanticActions_CompletionBlockType)(void);
    sa.completionBlock = ^{
        runLoopDone_ = YES;  // causes the run loop to exit
        self.bufferQueue = nil;
        //        self.dummyParser = nil;
        self.parser = nil;
        uint64_t t_end = mach_absolute_time();
        
        NSString* msg = [[NSString alloc] initWithFormat:@"Parser did finish. %ld document(s) downloaded and parsed in %.3fs", 
                         numberJsonDocuments_, absoluteTimeToNanoseconds(t_end - t_start_)*1.0e-9];
        NSLog(@"%@",msg);
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.activityIndicatorView stopAnimating];
            self.messageLabel.text = msg;
            self.startDownloadButton.enabled = YES;
        });
        [msg release];
    };
    
    
    // Create a buffer queue
    JPNSDataBuffers* bq = [[JPNSDataBuffers alloc] initWithMaxBuffers:4];
    self.bufferQueue = bq;
    [bq release];
    
    // Create the parser:    
    JPAsyncJsonParser*  p = [[JPAsyncJsonParser alloc] initWithBufferQueue:self.bufferQueue
                                                           semanticActions:sa];
    [sa release];
    self.parser = p;
    [p release];
    
}
#endif


#if defined (USE_DUMMY_PARSER)
- (void) setupDummyParser 
{    
    // Create a buffer queue
    JPNSDataBuffers* bq = [[JPNSDataBuffers alloc] initWithMaxBuffers:1];
    self.bufferQueue = bq;
    [bq release];
    
    dispatch_queue_t workerQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    //dispatch_queue_t handlerQueue = dispatch_queue_create("com.agrosam.JsonDownload.DummyParser.handler_queue", NULL); 
    
    // By setting the handler dispatch queue to NULL we effectively serialize the 
    // parser's thread and the handler blocks.
    DummyParser* parser = [[DummyParser alloc] initWithBuffersQueue:self.bufferQueue 
                                                 wokerDispatchQueue:workerQueue
                                               handlerDispatchQueue:NULL];
    //dispatch_release(handlerQueue);
    
    //
    // Handler blocks
    //
    parser.completionBlock = ^{
        runLoopDone_ = YES;
        NSLog(@"Handler: Dummy Parser finished.");
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.activityIndicatorView stopAnimating];
            self.messageLabel.text = @"Dummy Parser finished.";
            self.startDownloadButton.enabled = YES;
            self.bufferQueue = nil;
        });
    };
    
    __block int count = 0;
    parser.dataHandlerBlock = ^(id data){
        ++count;
        NSLog(@"Handler: Dummy Parser did process data[%d] %p", count, data);
        //sleep here to simulate work
        //usleep(1000*1000);
        dispatch_async(dispatch_get_main_queue(), ^{
            self.messageLabel.text = [NSString stringWithFormat:@"Dummy Parser did process data[%d] %p", count, data];
        });
    };
    
    parser.errorHandlerBlock = ^(NSError* error){
        NSLog(@"Handler: Dummy Parser signaled error: %@", error);
        runLoopDone_ = YES;
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.activityIndicatorView stopAnimating];
            [self handleError:error];
            self.startDownloadButton.enabled = YES;
            self.messageLabel.text = [NSString stringWithFormat:@"Dummy Parser signaled error: %@", error];
        });
    };
    
    self.parser = parser;
    [parser release];
}
#endif

#pragma mark -

// This method will be invoked from the "Start Download" button.
//
// 1) startDownload sets up a NSURLConnection ready to run on a secondary thread.
//    This will involve to define several NSURLConnection Delegate methods, which
//    this controller will handle.
// 2) Setup a JPSemanticActions object, a JPAsyncJsonParser and a JPBuffersQueue
//    ready to be used for a parser using the streaming API. This also involves 
//    to specify three handler blocks.
//    
// We want to run the connection delegate methods on a secondary thread because 
// we want to be abe to block them as this is a measurement to limit consumption
// of system resoures. Otherwise, the parser would run very quickly and would 
// generate lots of json containers waiting in a dispatch queue for further proces-
// sing. This however would eat that much resoures and would eventually choke
// the system. Note that an async parser will use its own thread, more specifi-
// cally an async parser will be scheduled on the global concurrent dispatch 
// queue.
//
// So, to avoid this, we do kind of "serialize" the task of parsing and the 
// task to process the json containers. Per default, when the parser calls the
// jsonObjectHandlerBlock it will wait until this block is finished and then
// proceed with its task.
//
// When the processing of the json containers is slow, the parser itself becomes
// slow as well - since both tasks are serialized. Due to this, the buffer queue 
// will be filled by the NSURLConnection quickly until it reaches its limits. 
// When the buffer queue is full - which happens on the NSURLConnection Delegate 
// method - the buffer queue will block the thread, which in turn will throttle
// the NSURLConnection to consume further input.
//
// In order to make this all possibly, we just need to schedule the delegate
// methods of NSURLConnection to a secondary thread. We do not need to configure
// the parser or the semantic actions class in a special way - the default setup
// is already appropriate.
// 

// 
- (void) startDownloadAndParse
{
    // Disable the "Start Download" button, until we are ready to
    // download again:
    self.startDownloadButton.enabled = NO;
    
    // We must not invoke several connection attempts:
    if (connection_)
        return;
    
    // We must not invoke this method if the buffer queue is already in use -
    // that is, the parser is stil running:
    if (bufferQueue_) {
        NSLog(@"We must not invoke startDownload: if the buffer queue is already in use");
        return;
    }

    // Setup the parser
#if defined (USE_DUMMY_PARSER)    
    [self setupDummyParser];
#else
    [self setupParser];
#endif    

    // Start the parser (in its own thread), which will wait for data buffers 
    // becoming available when downloading.
    [self.parser start];
    
    
    // Start the status bar network activity indicator. We'll turn it off when 
    // the connection finishes or experiences an error.
    [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
    
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
    
    // startConnection can be performed on secondary threads, thus we need
    // to schedule UIKit methods onto the main thread.
    
    NSString* urlString = @"http://ag-MacBookPro.local:3000/download/download";
    NSURL* url = [NSURL URLWithString:urlString];
    
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
    //[[NSURLCache sharedURLCache] setMemoryCapacity:1024*64];   
    [[NSURLCache sharedURLCache] setMemoryCapacity:0];   
    
    NSTimeInterval request_timeout = 600.0;
    NSURLRequest* request = [NSURLRequest requestWithURL:url cachePolicy:0 timeoutInterval:request_timeout];
    NSTimeInterval timeout = [request timeoutInterval];
    assert((timeout - request_timeout) == 0);
    
    // Create the URL connection and set its delegate:
    NSURLConnection* tmp = [[NSURLConnection alloc] initWithRequest:request 
                                                           delegate:self 
                                                   startImmediately:NO];
    self.connection = tmp;
    [tmp release];

    if (connection_ == nil) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self handleError:makeError(@"JsonDownload", 3, @"Failure to create URL connection.")];
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
    });
    
    
    // Schedule the connection's delegate methods in the main thread's run loop:
    [connection_ scheduleInRunLoop: [NSRunLoop currentRunLoop] forMode: NSRunLoopCommonModes];
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
    [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];    
    do {
        NSLog(@"Entering RunLoop.");
        [[NSRunLoop currentRunLoop] runMode:kDownloadConnectionRunLoopMode beforeDate:[NSDate distantFuture]];
        if (runLoopDone_) {
            NSLog(@"Exit RunLoop.");
        }
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
    assert(bufferQueue_);
    
    NSLog(@"ERROR: Connection did fail with error: %@", error);
    //runLoopDone_ = YES;  // causes the run loop to exit    
    
    // Perform the following on the main thread:
    dispatch_async(dispatch_get_main_queue(), 
    ^{
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;   
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
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;  
        self.startDownloadButton.enabled = NO;
    });
    
    // By producing a nil buffer, the parser will stop (with an error) eventually.
    while ([self.bufferQueue produceBuffer:nil] == NO) {
    }
}


- (void) connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response 
{
    assert(bufferQueue_);
    
    dispatch_async(dispatch_get_main_queue(), ^{
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
        
        // For the parser:
        [self.activityIndicatorView startAnimating];    
    });    
    
    totalBytesDownloaded_ = 0;
    numberJsonDocuments_ = 0;
    con_number_buffers_ = 0;
    t_start_ = mach_absolute_time();    
}


- (void) connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
#if 0    
    filename = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingPathComponent:save_name];
    NSFileHandle *file1 = [NSFileHandle fileHandleForUpdatingAtPath: filename];
    [file1 writeData: data];
    [file1 closeFile];
#else    
    assert(bufferQueue_);
        
    size_t size = [data length];
    totalBytesDownloaded_ += size;
    ++con_number_buffers_;
    NSLog(@"Connection did receive NSData[%ld](%p), size: %8.ld, total: %8.ld", 
          con_number_buffers_, data, size, totalBytesDownloaded_);
    
    // When "producing" buffers to the bufferQueue we need to consider that 
    // 1) -produceBuffer: may block,
    // 2) the buffer queue might have reached its max capacity. In this case, 
    //    method produceBuffer: will return NO, after the timeout expired. If 
    //    this happens, we need to retry to produce the buffer - until we 
    //    decided to stop and throw an error.
    
    double currentTimeout = 0;
    while ([self.bufferQueue produceBuffer:data] == NO) {
        NSString* msgformat = @"Could not produce buffer to buffer queue till timeout (%gs), possibly since buffer queue is full. %@";
        currentTimeout += self.bufferQueue.timeout;
        if (currentTimeout < 300) {
            NSLog(msgformat, currentTimeout, @"Retry ...");
        }
        else {
            // A possibly reason we reach here is that the parser or its handler
            // is stuck and is blocked on its thread, so the buffer queue is not
            // consumed. We can either wait forever or give up - which possibly
            // makes the whole download invalid.
            
            //runLoopDone_ = YES;  // causes the run loop to exit
            NSLog(msgformat, @"Max timeout reached - giving up.");
            // Cancel the connection, and emit an error alert:
            dispatch_async(dispatch_get_main_queue(), ^{
                self.connection = nil;
                [self handleError:makeError(@"JsonDownload", 2, @"Could not add buffer to buffer queue. Connection closed.")];
            });
            
            break;
        }
    }    
#endif    
}


- (void) connectionDidFinishLoading:(NSURLConnection *)connection
{  
    assert(bufferQueue_);
    
    NSString* msg = [[NSString alloc] initWithFormat:@"Connection did finish downloading"];
    NSLog(@"%@", msg);
    self.connection = nil;
    dispatch_async(dispatch_get_main_queue(), ^{
        self.messageLabel.text = msg;
        [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;     
    });
    [msg release];
    while ([self.bufferQueue produceBuffer:nil] == NO) {
    }
    
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

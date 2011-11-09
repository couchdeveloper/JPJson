//
//  JsonTestAppViewController.m
//  JsonTestApp
//
//  Created by Andreas Grosam on 6/30/11.
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

#import "JsonTestAppViewController.h"
#import "JPJson/JPJsonParser.h"
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>


#include <dispatch/dispatch.h>


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



@interface JsonTestAppViewController ()

@property (nonatomic, retain) JPAsyncJsonParser* parser;
@property (nonatomic, retain) IBOutlet UILabel*  messageLabel;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView*  producerActivityIndicatorView;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView*  parserActivityIndicatorView;
@property (nonatomic, retain) NSMutableArray* documents;



- (void) start;
- (void) finish;
- (void) startParser;
- (void) startLoadingJsonText;
- (void) handleError:(NSError*)error;
- (void) cancel;


//- (void) cancelDownload;

@end



@implementation JsonTestAppViewController

@synthesize parser = parser_;
@synthesize bufferQueue = bufferQueue_;
@synthesize messageLabel = messageLabel_;
@synthesize producerActivityIndicatorView = producerActivityIndicatorView_;
@synthesize parserActivityIndicatorView = parserActivityIndicatorView_;
@synthesize documents = documents_;


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
    [parser_ release], parser_ = nil;
    [bufferQueue_ release], bufferQueue_ = nil;
    [documents_ release], documents_ = nil;
    [messageLabel_ release], messageLabel_ = nil;
    [producerActivityIndicatorView_ release], producerActivityIndicatorView_ = nil;
    [parserActivityIndicatorView_ release], parserActivityIndicatorView_ = nil;
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];

    NSAssert(messageLabel_, @"IBOutlet 'messageLable_' is nil");
    NSAssert(producerActivityIndicatorView_, @"IBOutlet 'producerActivityIndicatorView_' is nil");
    NSAssert(parserActivityIndicatorView_, @"IBOutlet 'parserActivityIndicatorView_' is nil");
    
    // Do any additional setup after loading the view from its nib.
    self.producerActivityIndicatorView.hidesWhenStopped = YES;    
    self.parserActivityIndicatorView.hidesWhenStopped = YES;    
    self.messageLabel.text = @"";
}


- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
    self.messageLabel = nil;
    self.producerActivityIndicatorView = nil;    
    self.parserActivityIndicatorView = nil;    
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
    [self start];
}


- (void) cancelButtonTapped:(id)sender
{
    [self cancel];    
}

- (void) cancel
{
    // First cancel the producer
    // 
    // Canceling a producer thread may be sometimes tricky. 
    // We may run into the following situation:
    // The parser has stopped parsing - e.g. due to an error.
    // The producer thread is blocking in method -produceBuffer: because
    // the buffer queue is full. Unless we erase or consume buffers
    // we can not convince the producer's thread to resume which lets
    // the producer possibly check the "cancel" state and hence exits
    // its function.
    // Therefore the producer should check periodically whether the 
    // cancelation has been set. It achieves this trough setting a 
    // timeout when invoking the produceBuffer method. produceBuffer returns 
    // NO if it could not add the buffer to the queue, indicating a timeout.
    // The producer thread is resumed and it can check the result and
    // the cancelation state. If it is canceled, the producer simply
    // exits. When it exits, the buffer queue will be released, and
    // the controller becomes its sole owner. When the controller 
    // releases the buffer queue it will be dealloced, which in turn 
    // releases all contained data buffers as well.
    
    
    // Set the cancelation state:
    canceledDownload_ = YES;
    // This would suffice to cancel the producer
    
        
    // Cancel the parser:
    [parser_ cancel];    
}


#pragma mark -

- (void) start 
{    
    // We must not invoke this method if the parser is already in use.
    if (parser_) {
        NSLog(@"We must not invoke start if the parser is already in use");
        return;
    }
    
    canceledDownload_ = NO;
    
    countDocuments_ = 0;
    self.documents = nil;
    startTime_ = CFAbsoluteTimeGetCurrent();
    
    // The parser and the loading thread share the same buffer queue.
    [self startParser]; 
    [self startLoadingJsonText];
}

- (void) finish {
    self.bufferQueue = nil;
    self.parser = nil;
    self.documents = nil;
}


// Starts the parser asynchronously using a buffer queue.
- (void) startParser
{        
    NSAssert(parser_ == nil, @"parser_ is not nil");
    
    // Get the buffer queue which will be shared with the producer:
    JPNSDataBuffers* theBufferQueue = self.bufferQueue;
    
    JPAsyncJsonParser* tmp = [[JPAsyncJsonParser alloc] 
                              initWithBufferQueue:theBufferQueue 
                                         delegate:self
                                          options:JPJsonParserParseMultipleDocuments];
    self.parser = tmp;
    [tmp release];
    
    [self.parserActivityIndicatorView startAnimating];
        
    // parse asynchronously:
    [self.parser start];
}


// startLoadingJsonText produces NSData buffers from reading a file containing 
// multiple Json documents. The buffers will be put into the bufferQueue.
// This method runs asynchronously.
- (void) startLoadingJsonText
{
    const size_t BUFFER_SIZE = 1024*4;
    
    const double rate = 1024*400;
    
    useconds_t duration_us = 1e6/rate*BUFFER_SIZE;
    
    
    [self.producerActivityIndicatorView startAnimating];
    
    // get a concurrent dispatch queue where the producer will run:
    dispatch_queue_t concurrent_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    JPNSDataBuffers* buffers = self.bufferQueue;
    
    // Implement a simple producer for the NSData buffer queue.
    // Load the file which contains multiple JSON Documents and produce as 
    // many buffers as requied:
    
    NSString* fileName = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"TestMD.json"];
    
    dispatch_async(concurrent_queue, ^{
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:fileName
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        if (data == nil) {
            dispatch_async(dispatch_get_main_queue(), ^{ 
                [self.producerActivityIndicatorView stopAnimating];
                [self handleError:error]; 
            });
            return;
        }    
        size_t const dataSize = [data length];
        const char* first = (const char*)[data bytes];
        const char* last = first + dataSize;
        while (first < last && !canceledDownload_) {
            
            uint64_t t = mach_absolute_time();
            
            size_t count = MIN( (long)BUFFER_SIZE, last - first);
            NSData* buffer = [[NSData alloc] initWithBytes:first length:count];
            BOOL produced = NO;
            while (!produced && !canceledDownload_) {
                // we got timeout. Retry IFF not canceled:
                produced = [buffers produceBuffer:buffer];                
            }
                
            [buffer release];
            first += count; 
            
#if 0            
            uint64_t dt = mach_absolute_time() - t;
            uint64_t elapsedNanoseconds = absoluteTimeToNanoseconds(dt );
            if (elapsedNanoseconds/1000 < duration_us) {
                useconds_t sleep_us = duration_us - elapsedNanoseconds/1000;
                NSLog(@"throttle input by %u us", sleep_us);
                usleep(sleep_us);
            }
#endif            
            
//#if defined (DEBUG)
            //NSLog(@"produced buffer with size %lu", dataSize);
            //usleep(5000);  // 5ms per 4KByte equals roughly 800kB/s
//#endif                
        }   
        
        // Put a NULL buffer in order to indicate EOF:
        while (!canceledDownload_ && ![buffers produceBuffer:nil]) {
            // we got timeout. Retry IFF not canceled
        }
        
        [data release];
        
        dispatch_async(dispatch_get_main_queue(), ^{ 
            [self.producerActivityIndicatorView stopAnimating];
        });
        
    });
    
}


// Handle errors in the download by showing an alert to the user. 
- (void) handleError:(NSError*)error 
{
    NSString* errorMessage = [error localizedDescription];
    NSLog(@"ERROR: %@", errorMessage);
    
    UIAlertView* alertView =
    [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Error Title", nil)
                               message:errorMessage
                              delegate:nil
                     cancelButtonTitle:@"OK"
                     otherButtonTitles:nil];
    [alertView show];
    [alertView release];
}



#pragma mark -
#pragma mark Properties Implementation
- (JPNSDataBuffers*) bufferQueue {
    if (!bufferQueue_) {
        bufferQueue_ = [[JPNSDataBuffers alloc] initWithMaxBuffers:4];
        
        // The default timeout for a JPNSDataBuffers object for produce and 
        // aquire methods is set in the timeout property. We set it here to 
        // 1 second in order to make a possible cancelation of the producer 
        // more responsive. If the timeout is expired in the produce method, 
        // the producer will simply retry to produce the buffer unless it is 
        // canceled in which case it exits.
        // The timeout used by the parser is not affected by this timout setting.
        bufferQueue_.timeout = 1.0;
    }
    return bufferQueue_;
}


- (NSMutableArray*)  documents {
    if (!documents_) {
        documents_ = [[NSMutableArray alloc] init];
    }
    return documents_;
}



// MyViewController implements the JPJsonParserHandler protocol:
#pragma mark -
#pragma mark JPAsyncJsonParserDelegate
- (void) asyncJsonParser:(JPAsyncJsonParser*)parser didReceiveJson:(id)json
{
    ++countDocuments_;
    [self.documents addObject:json];
    self.messageLabel.text = @"did receive json";
#if defined (DEBUG)    
    NSLog(@"did receive json: %@", json);
#endif    
}


- (void) asyncJsonParser:(JPAsyncJsonParser*)parser didFailWithError:(NSError*)error;
{
    // In the event of an error we must invoke cancel. Otherwise, the producer
    // thread may still be active and possibly blocking. Canceling will eventually
    // cause the producer thread to exit.
    [self cancel];
    
    
    [self.parserActivityIndicatorView stopAnimating];
    [self finish];
    NSLog(@"ERROR: parser did fail with error %@", error);
    [self handleError:error];    
}


- (void) asyncJsonParserFinished:(JPAsyncJsonParser*)parser
{
    [self.parserActivityIndicatorView stopAnimating];
    self.messageLabel.text = [NSString stringWithFormat:@"parser did finish with %ld documents in %.3f sec", 
                              countDocuments_, CFAbsoluteTimeGetCurrent()- startTime_];
    [self finish];
#if defined (DEBUG)    
    NSLog(@"parser did finish");
#endif    
}




@end

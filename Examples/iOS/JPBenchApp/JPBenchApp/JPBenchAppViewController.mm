//
//  JPBenchAppViewController.m
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
#if __has_feature(objc_arc) 
#error This Objective-C file shall be compiled with ARC disabled.
#endif

#import "JPBenchAppViewController.h"
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPJsonWriter.h"
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#include <dispatch/dispatch.h>
#include "utilities/timer.hpp"
#include <utility>
#include <algorithm>

#import "JSONKit/JSONKit.h"


#if !defined (NS_BLOCK_ASSERTIONS)
#warning NS_BLOCK_ASSERTIONS not defined
#endif

#if !defined (NDEBUG)
#warning NDEBUG not defined
#endif


namespace {
    
    template <typename T>
    class MinMaxAvg {
    public:  
        MinMaxAvg() : count_(0), min_(0), max_(0), sum_(0) {};
        T min() const { return min_; }
        T max() const { return max_; }
        double avg() const { return count_ ? sum_/count_ : 0; }
        
        void set(const T& v) {
            if (count_ == 0) {
                sum_ = v;
                min_ = v;
                max_ = v;
            } else {
                sum_ += v;
                min_ = std::min(v, min_);
                max_ = std::max(v, max_);
            }
            ++count_;
        }
        
    private:
        size_t count_;
        T min_;
        T max_;
        T sum_;
    };

    
    
    uint64_t absoluteTimeToNanoseconds(uint64_t t) 
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
    
    
}

typedef     MinMaxAvg<double> MinMaxAvgTime;


@interface JPBenchAppViewController ()

@property (nonatomic, retain) IBOutlet UILabel*  benchJPJsonParserResult1Label;
@property (nonatomic, retain) IBOutlet UILabel*  benchJPJsonParserResult2Label;
@property (nonatomic, retain) IBOutlet UILabel*  benchJSONKitResult1Label;
@property (nonatomic, retain) IBOutlet UILabel*  benchJSONKitResult2Label;



- (void) handleError:(NSError*)error;

- (MinMaxAvgTime) bench_JsonParser1WithN:(int)N;
- (MinMaxAvgTime) bench_JsonParser2WithN:(int)N;

- (MinMaxAvgTime) bench_JSONKit1WithN:(int)N;
- (MinMaxAvgTime) bench_JSONKit2WithN:(int)N;


@end



@implementation JPBenchAppViewController {
    IBOutlet UILabel*       _benchJPJsonParserResult1Label;
    IBOutlet UILabel*       _benchJPJsonParserResult2Label;
    IBOutlet UILabel*       _benchJSONKitResult1Label;
    IBOutlet UILabel*       _benchJSONKitResult2Label;
    BOOL                    _running;
}


static NSString* JsonTestFile = @"Test-UTF8-esc.json";

@synthesize benchJPJsonParserResult1Label = _benchJPJsonParserResult1Label;
@synthesize benchJPJsonParserResult2Label = _benchJPJsonParserResult2Label;
@synthesize benchJSONKitResult1Label = _benchJSONKitResult1Label;
@synthesize benchJSONKitResult2Label = _benchJSONKitResult2Label;


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
    NSAssert(_benchJPJsonParserResult1Label, @"IBOutlet '_benchJPJsonParserResult1Label' is nil");
    NSAssert(_benchJPJsonParserResult2Label, @"IBOutlet '_benchJPJsonParserResult2Label' is nil");
    NSAssert(_benchJSONKitResult1Label, @"IBOutlet '_benchJSONKitResult1Label' is nil");
    NSAssert(_benchJSONKitResult2Label, @"IBOutlet '_benchJSONKitResult2Label' is nil");
    
    self.benchJPJsonParserResult1Label.text = @"";    
    self.benchJPJsonParserResult2Label.text = @"";    
    self.benchJSONKitResult1Label.text = @"";    
    self.benchJSONKitResult2Label.text = @"";    
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
    self.benchJPJsonParserResult1Label = nil;
    self.benchJPJsonParserResult2Label = nil;
    self.benchJSONKitResult1Label = nil;
    self.benchJSONKitResult2Label = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}




#pragma mark - Actions

-(IBAction) startBenchJPJsonParser1:(id)sender
{
    if (_running) {
        NSLog(@"Bench is already running.");
        return;
    }
    MinMaxAvgTime te = [self bench_JsonParser1WithN:100];    
    NSString* msg = [[NSString alloc] initWithFormat:@"%.3f ms", te.min()*1e3];
    self.benchJPJsonParserResult1Label.text = msg;
    [msg release];
}    

-(IBAction) startBenchJPJsonParser2:(id)sender 
{
    if (_running) {
        NSLog(@"Bench is already running.");
        return;
    }
    MinMaxAvgTime te = [self bench_JsonParser2WithN:100];    
    NSString* msg = [[NSString alloc] initWithFormat:@"%.3f ms", te.min()*1e3];
    self.benchJPJsonParserResult2Label.text = msg;
    [msg release];
}
    
-(IBAction) startBenchJSONKitParser1:(id)sender  
{
    if (_running) {
        NSLog(@"Bench is already running.");
        return;
    }
    MinMaxAvgTime te = [self bench_JSONKit1WithN:100];        
    NSString* msg = [[NSString alloc] initWithFormat:@"%.3f ms", te.min()*1e3];
    self.benchJSONKitResult1Label.text = msg;
    [msg release];
}

-(IBAction) startBenchJSONKitParser2:(id)sender  
{
    if (_running) {
        NSLog(@"Bench is already running.");
        return;
    }
    MinMaxAvgTime te = [self bench_JSONKit2WithN:100];        
    NSString* msg = [[NSString alloc] initWithFormat:@"%.3f ms", te.min()*1e3];
    self.benchJSONKitResult2Label.text = msg;
    [msg release];
}



#pragma mark -



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


#pragma mark - Bench

- (MinMaxAvgTime) bench_JsonParser1WithN:(int)N
{
    using namespace utilities;
    
    NSString* fileName = [[[NSBundle mainBundle] resourcePath] 
                          stringByAppendingPathComponent:JsonTestFile];
    __autoreleasing NSError* error;
    NSData* data = [[NSData alloc] initWithContentsOfFile:fileName
                                                  options:NSDataReadingUncached 
                                                    error:&error];
    if (data == nil) {
        NSLog(@"ERROR: could not load file. %@", error);
        NSFileManager* fileManager = [[NSFileManager alloc] init];
        NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
        abort();
    }
    
    
    printf("--------------------------------------------\n");
    printf("Running bench_JsonParser1WithN %d times.\n", N);
    printf("--------------------------------------------\n");    
    printf("Using a NSData with UTF-8 content as input and interface method:\n"
           "+parseData:options:error: (class JPJsonParser)\n"
           "options: 0\n"
           "Automatic detection of input encoding, immutable containers\n");
        
    
    
    MinMaxAvg<double> te;
    timer t = timer();
    
    BOOL gotError = NO;
    for (int i = 0; i < N; ++i) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        t.start();
        // This method creates and destroys the internal semantic actions
        // instance.
        id result = [JPJsonParser parseData:data 
                                    options:(JPJsonParserOptions)0
                                      error:&error];
        [result retain];
        t.stop();
        te.set(t.seconds());
        t.reset();
        [result release];
        [pool release];
        
        if (!result)
            break;
    }
    
    if (gotError) {
        NSLog(@"ERROR: %@", error);
    }
    else {
        NSLog(@"JPJsonParser: elapsed time for parsing:\n"
              "min: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
              te.min()*1e3, te.max()*1e3, te.avg()*1e3);
    }
    
    [data release];
    return te;
}


- (MinMaxAvgTime) bench_JsonParser2WithN:(int)N
{
    using namespace utilities;
    
    NSString* fileName = [[[NSBundle mainBundle] resourcePath] 
                          stringByAppendingPathComponent:JsonTestFile];
    __autoreleasing NSError* error;
    NSData* data = [[NSData alloc] initWithContentsOfFile:fileName
                                                  options:NSDataReadingUncached 
                                                    error:&error];
    if (data == nil) {
        NSLog(@"ERROR: could not load file. %@", error);
        NSFileManager* fileManager = [[NSFileManager alloc] init];
        NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
        abort();
    }
    

    printf("--------------------------------------------\n");
    printf("Running bench_JsonParser2WithN bench %d times.\n", N);
    printf("--------------------------------------------\n");    
    printf("Timing includes destruction of objects, too\n");
    printf("Using a NSData with UTF-8 content as input and interface method:\n"
           "+parseData:options:error: (class JPJsonParser)\n"
           "options: 0\n"
           "Automatic detection of input encoding, immutable containers\n");
    
    MinMaxAvgTime te;
    timer t = timer();
    
    BOOL gotError = NO;
    for (int i = 0; i < N; ++i) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

        t.start();        
        // This method creates and destroys the internal semantic actions
        // instance.
        id result = [JPJsonParser parseData:data 
                                    options:(JPJsonParserOptions)0
                                      error:&error];
        [pool release];
        
        t.stop();
        te.set(t.seconds());
        if (!result)
            break;
    }
    
    if (gotError) {
        NSLog(@"ERROR: %@", error);
    }
    else {
        NSLog(@"JPJsonParser: elapsed time for parsing:\n"
              "min: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
              te.min()*1e3, te.max()*1e3, te.avg()*1e3);
    }
    
    [data release];
    
    return te;
}


- (MinMaxAvgTime) bench_JSONKit1WithN:(int)N
{
    using namespace utilities;
    
    NSString* fileName = [[[NSBundle mainBundle] resourcePath] 
                          stringByAppendingPathComponent:JsonTestFile];
    __autoreleasing NSError* error;
    NSData* data = [[NSData alloc] initWithContentsOfFile:fileName
                                                  options:NSDataReadingUncached 
                                                    error:&error];
    if (data == nil) {
        NSLog(@"ERROR: could not load file. %@", error);
        NSFileManager* fileManager = [[NSFileManager alloc] init];
        NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
        abort();
    }
    
    
    printf("--------------------------------------------\n");
    printf("Running bench_JSONKit1WithN %d times.\n", N);
    printf("--------------------------------------------\n");    
    printf("using a NSData with UTF-8 content as input,\n"
           "interface method: objectWithData: (JSONDecoder),\n"
           "JKParseOptionFlags = 0\n");
    
    MinMaxAvg<double> te;
    timer t = timer();
    
    BOOL gotError = NO;
    for (int i = 0; i < N; ++i) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        t.start();
        JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:(JKParseOptionFlags)JKParseOptionNone];
        id result = [decoder objectWithData:data];
        [result retain];
        [decoder release];
        t.stop();
        te.set(t.seconds());
        t.reset();
        [result release];
        [pool release];
        
        if (!result)
            break;
    }
    
    if (gotError) {
        NSLog(@"ERROR: %@", error);
    }
    else {
        NSLog(@"JSONKit: elapsed time for parsing:\n"
              "min: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
              te.min()*1e3, te.max()*1e3, te.avg()*1e3);
    }
    
    [data release];
    
    return te;
}

-(MinMaxAvgTime) bench_JSONKit2WithN:(int)N
{
    using namespace utilities;
    
    NSString* fileName = [[[NSBundle mainBundle] resourcePath] 
                          stringByAppendingPathComponent:JsonTestFile];
    __autoreleasing NSError* error;
    NSData* data = [[NSData alloc] initWithContentsOfFile:fileName
                                                  options:NSDataReadingUncached 
                                                    error:&error];
    if (data == nil) {
        NSLog(@"ERROR: could not load file. %@", error);
        NSFileManager* fileManager = [[NSFileManager alloc] init];
        NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
        abort();
    }
    
    printf("--------------------------------------------\n");
    printf("Running bench_JSONKit2WithN %d times.\n", N);
    printf("--------------------------------------------\n");  
    printf("Timing includes destruction of objects, too\n");
    printf("using a NSData with UTF-8 content as input,\n"
           "interface method: mutableObjectWithData: (JSONDecoder),\n"
           "JKParseOptionFlags = 0\n");
    
    MinMaxAvg<double> te;
    timer t = timer();
    
    BOOL gotError = NO;
    for (int i = 0; i < N; ++i) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

        t.start();
        JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:(JKParseOptionFlags)JKParseOptionNone];
        id result = [decoder objectWithData:data];
        [decoder release];
        [pool release];

        t.stop();
        te.set(t.seconds());
        if (!result)
            break;
    }
    
    if (gotError) {
        NSLog(@"ERROR: %@", error);
    }
    else {
        NSLog(@"JSONKit: elapsed time for parsing:\n"
              "min: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
              te.min()*1e3, te.max()*1e3, te.avg()*1e3);
    }
    
    [data release];
    return te;
}



@end

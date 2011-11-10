//
//  main.m
//  AGJsonParser
//
//  Created by Andreas Grosam on 4/8/11.
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

#import <Foundation/Foundation.h>
#import "JPJson/JPSemanticActions.h"
#import "JPJson/JPStreamSemanticActions.h"
#import "JPJson/JPAsyncJsonParser.h"
#import "JPJson/JPJsonParser.h"
#import "JPJson/NSData+JPJsonDetectEncoding.h"
#include "utilities/timer.hpp"
#include <algorithm>




#define USE_JSONKit

#if defined (USE_JSONKit)
#include "JSONKit.h"
#endif

#include <dispatch/dispatch.h>




#if defined (BOOST_STRICT_CONFIG)
#warning BOOST_STRICT_CONFIG defined
#endif

#if defined (BOOST_DISABLE_THREADS)
#warning BOOST_DISABLE_THREADS defined
#endif


//------------------------------------------------------------------------------
//
//  Runs benchmark test using the JPJsonLib Interface Methods 
//
//------------------------------------------------------------------------------



namespace {

    // Path to a JSON file which is used globally for all tests.
    NSString* JSON_TEST_FILE = @"Test-UTF8-esc.json";
    
    
    using utilities::timer;
    
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
    
    typedef     MinMaxAvg<double> MinMaxAvgTime;
    
    
#pragma mark -
    
    
    // -----------------------------------------------------------------------------
    //  bench_AGJsonParserString1()
    //  Class JPJsonParser
    //  Using a NSString as input and interface method:
    //  +parseString:options:error:
    // -----------------------------------------------------------------------------
    void bench_AGJsonParserString1(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running AGJsonParser bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Using a NSString as input and interface method:\n"
               "parseString:options:error: (JPJsonParser)\n"
               "options: (JPJsonParserCreateMutableContainers) 0\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE  // 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }    
        
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        NSString* input = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:encoding];
        [data release]; data = nil;
        
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            t.start();
            // This method creates and destroys the internal semantic actions
            // instance.
            id result = [JPJsonParser parseString:input 
                                          options:(JPJsonParserOptions)(0)
                                            error:&error];
            [result retain];
            t.stop();
            te.set(t.seconds());
            t.reset();
#if defined (XX_DEBUG)
            if ((i+1) == N) {
                NSLog(@"%@", result);
            }
#endif        
            [result release];
            [pool release];
            
            if (!result)
                break;
        }
        
        [input release];
        
        if (gotError) {
            NSLog(@"ERROR: %@", error);
        }
        else {
            NSLog(@"AGJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    
    
    
    // -----------------------------------------------------------------------------
    //  bench_AGJsonParser1a()
    //  Class JPJsonParser
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:options:error:
    //  options: JPJsonParserCreateMutableContainers
    // -----------------------------------------------------------------------------
    void bench_AGJsonParser1a(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running AGJsonParser bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Using a NSData with UTF-8 content as input and interface method:\n"
               "parseData:options:error: (JPJsonParser)\n"
               "options: JPJsonParserCreateMutableContainers\n"
               "The input encoding will be detected automatically\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE  // 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
        
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
                                        options:(JPJsonParserOptions)(JPJsonParserCreateMutableContainers)
                                          error:&error];
            [result retain];
            t.stop();
            te.set(t.seconds());
            t.reset();
#if defined (XX_DEBUG)
            if ((i+1) == N) {
                NSLog(@"%@", result);
            }
#endif        
            [result release];
            [pool release];
            
            if (!result)
                break;
        }
        
        if (gotError) {
            NSLog(@"ERROR: %@", error);
        }
        else {
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    
    
    // -----------------------------------------------------------------------------
    //  bench_AGJsonParser1b()
    //  Class JPJsonParser
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:options:error:
    //  options: 0
    //  The input encoding will be detected automatically.
    // -----------------------------------------------------------------------------
    void bench_AGJsonParser1b(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running AGJsonParser bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Class JPJsonParser"
               "Using a NSData with UTF-8 content as input and interface method:\n"
               "+parseData:options:error:\n"
               "options: none\n"
               "The input encoding will be detected automatically\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
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
#if defined (XX_DEBUG)
            if ((i+1) == N) {
                NSLog(@"%@", result);
            }
#endif        
            [result release];
            [pool release];
            
            if (!result)
                break;
        }
        
        if (gotError) {
            NSLog(@"ERROR: %@", error);
        }
        else {
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    
    // -----------------------------------------------------------------------------
    //  bench_AGJsonParser1x()
    //  Class JPJsonParser
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:semanticActions:
    //  semantic actions class: JPStreamSemanticActions
    //  The input encoding will be detected automatically.
    // -----------------------------------------------------------------------------
    void bench_AGJsonParser1x(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running AGJsonParser bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Class JPJsonParser"
               "Using a NSData with UTF-8 content as input and interface method:\n"
               "+parseData:semanticActions:\n"
               "Semantic Actions: JPStreamSemanticActions\n"
               "The input encoding will be detected automatically\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL success = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            t.start();
            // This method creates and destroys the internal semantic actions
            // instance.
            
            JPStreamSemanticActions* sa = [[JPStreamSemanticActions alloc] initWithHandlerDispatchQueue:NULL];        
            success = [JPJsonParser parseData:data semanticActions:sa];
            t.stop();
            te.set(t.seconds());
            t.reset();
            if (!success) {
                error = sa.error;
                break;
            }
            [sa release];
            [pool release];        
        }
        
        if (!success) {
            NSLog(@"ERROR: %@", error);
        }
        else {
            NSLog(@"AGJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    
    // -----------------------------------------------------------------------------
    //  bench_AGJsonParser1d()
    //  Class JPJsonParser
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:options:error:
    //  with options: 0
    //  The input encoding will be detected automatically.
    //  Timing includes destruction of objects
    // -----------------------------------------------------------------------------
    void bench_AGJsonParser1d(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running AGJsonParser bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Timing includes destruction of JSON representation\n");
        printf("using a NSData with UTF-8 content as input and interface method:\n"
               "parseUTF8Data:options:error: (JPJsonParser)\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }

        MinMaxAvg<double> te;
        timer t = timer();
        
        t.start();
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            t.start();
            
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
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
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    
    // -----------------------------------------------------------------------------
    //  bench_AGJsonParser2()
    //  Class JPJsonParser
    //  Timing includes destruction of objects.
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:encoding:options:error:
    //  The input encoding shall be set explicitly
    // -----------------------------------------------------------------------------
    void bench_AGJsonParser2(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running AGJsonParser bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Timing includes destruction of objects, too\n");
        printf("Using a NSData with UTF-8 content as input and interface method:\n"
               "parseData:options:error: (JPJsonParser)\n"
               "options: JPJsonParserCreateMutableContainers\n"
               "The input encoding shall be set explicitly\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE  // 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            t.start();
            
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            // This method creates and destroys the internal semantic actions
            // instance.
            id result = [JPJsonParser parseData:data 
                                        options:(JPJsonParserOptions)(JPJsonParserCreateMutableContainers)
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
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    

    
    // -----------------------------------------------------------------------------
    // bench_AGAsyncJsonParser
    // Asynchronous Parser, using a NSData with UTF-8 content as input.
    // Timing includes destruction of JSON objects, the semantic actions object is 
    // kept alive, giving it and advantage, since key strings are cached.\n"
    // So this test is not directly comparable to the others.
    // -----------------------------------------------------------------------------
    void bench_AGAsyncJsonParser(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running AGJsonParser bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Asynchronous Parser, using a NSData with UTF-8\n"
               "content as input.\n"
               "Timing measures the average time including destruction\n"
               "of JSON objects. The semantic actions object is kept alive,\n"
               "giving it and advantage, since key strings are cached.\n"
               "This test is not directly comparable to the others.\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
        __block BOOL gotError = NO;
        
        timer t = timer();
        
        t.start();
        
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        
        JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
        JPSemanticActions* sa = (JPSemanticActions*)[parser semanticActions];
        sa.parseMultipleDocuments = YES;
        // The handlers should use its own (serial) queue. Handler blocks must not 
        // be scheduled on the global concurrent queue (where the parser runs), if
        // option parseMultipleDocumentsAsynchronously equals YES, otherwise handler 
        // blocks may cause a deadlock. Here, in this example, the handler blocks
        // must also not run in the main queue.    
        // We also want to run our handlers in strict order, otherwise, we don't
        // get the timing right.
        dispatch_queue_t handlerDispatchQueue = dispatch_queue_create("com.test.json.parser.handler", NULL);
        [sa setHandlerDispatchQueue:handlerDispatchQueue];
        dispatch_release(handlerDispatchQueue);
        
        sa.startJsonHandlerBlock = ^{
        };
        sa.endJsonHandlerBlock = ^(id result){
        };
        sa.completionHandlerBlock = ^{
            dispatch_semaphore_signal(sem);
        };
        sa.errorHandlerBlock = ^(NSError* error){
            gotError = YES;
            NSLog(@"ERROR: %@", error);
        };
        
        [parser start];
        
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            for (int i = 0; i < N; ++i) 
            {
                // this will block, until the parser has taken it
                bool delivered = [parser parseBuffer:data]; 
                if (not delivered)
                    break;
            }
            [parser parseBuffer:nil];  // send EOF
        });
        
        // Wait until the parser has been finished
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
        [parser release];
        t.stop();
        
        
        if (!gotError) {
            NSLog(@"JPJsonParser: elapsed time for parsing %d documents:\n"
                  "average: %.3f s\n", N, t.seconds());
        }
        
        /*    
         if (!gotError) {
         NSLog(@"AGJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
         te.min()*1e3, te.max()*1e3, te.avg()*1e3);
         }
         */ 
    }
    
    
    
    
    
#if defined (USE_JSONKit)
    
#pragma mark - JSONKit
    
    
    void bench_JSONKitString1(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JSONKitString1 %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("using a NSString as input,\n"
               "interface method: mutableObjectWithData: (JSONDecoder),\n"
               "JKParseOptionFlags = 0\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
        
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        NSString* input = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:encoding];
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            NSString* input = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:encoding];
            
            t.start();
            JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:(JKParseOptionFlags)JKParseOptionNone]; 
            const char* utf8data = [input UTF8String];
            NSUInteger length = [input lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
            id result = [decoder objectWithUTF8String:(const unsigned char*)(utf8data) length:length error:&error];
            [result retain];
            [decoder release];
            t.stop();
            te.set(t.seconds());
            t.reset();
            [result release];
            [input release];
            [pool release];
            
            if (!result)
                break;
        }
        
        [data release]; data = nil;
        [input release];
        
        if (gotError) {
            NSLog(@"ERROR: %@", error);
        }
        else {
            NSLog(@"JSONKit: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    
    
    
    void bench_JSONKit1(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JSONKit1 %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("using a NSData with UTF-8 content as input,\n"
               "interface method: mutableObjectWithData: (JSONDecoder),\n"
               "JKParseOptionFlags = 0\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            t.start();
            JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:(JKParseOptionFlags)JKParseOptionNone];
            id result = [decoder mutableObjectWithData:data];
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
            NSLog(@"JSONKit: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    void bench_JSONKit2(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JSONKit2 %d times.\n", N);
        printf("--------------------------------------------\n");  
        printf("Timing includes destruction of objects\n");
        printf("using a NSData with UTF-8 content as input,\n"
               "interface method: mutableObjectWithData: (JSONDecoder),\n"
               "JKParseOptionFlags = 0\n");

        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            t.start();
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:(JKParseOptionFlags)JKParseOptionNone];
            id result = [decoder mutableObjectWithData:data];
            [result retain];
            [decoder release];
            [result release];
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
            NSLog(@"JSONKit: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
#endif 
    
    
#pragma mark - NSJSONSerialization
    
    void bench_NSJSONSerialization1(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running NSJSONSerialization bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("using a NSData with UTF-8 content as input,\n"
               "interface method: JSONObjectWithData:options:error:, \n"
               "options: NSJSONReadingMutableContainers\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        error = nil;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            t.start();
            id result = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:&error];
            gotError = result == nil;
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
            NSLog(@"NSJSONSerialization: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    
    void bench_NSJSONSerialization2(const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running NSJSONSerialization bench %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Timing includes destruction of objects, too\n");
        printf("using a NSData with UTF-8 content as input,\n"
               "interface method: JSONObjectWithData:options:error:, \n"
               "options: NSJSONReadingMutableContainers\n");
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:JSON_TEST_FILE 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        error = nil;
        for (int i = 0; i < N; ++i) 
        {
            t.start();
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            id result = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:&error];
            gotError = result == nil;
            [result retain];
            [result release];
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
            NSLog(@"NSJSONSerialization: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
    }
    


} // namespace


#pragma mark -

int main (int argc, const char * argv[])
{
#if defined (DEBUG)
    const int N = 1;
#else
    const int N = 1000;
#endif    

    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        
    //bench_AGJsonParserCharIter(N);
    //bench_AGJsonParserString1(N);
    bench_AGJsonParser1a(N);
/*    
    bench_AGJsonParser1b(N);
    bench_AGJsonParser1x(N);
    bench_AGJsonParser2(N);
    bench_AGAsyncJsonParser(N);
#if defined (USE_JSONKit)    
    bench_JSONKit1(N);
    bench_JSONKit2(N);
    bench_JSONKitString1(N);
#endif    
    bench_NSJSONSerialization1(N);
    bench_NSJSONSerialization2(N);
*/    
    [pool drain];
    return 0;
}


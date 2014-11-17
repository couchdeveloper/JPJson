//
//  main.m
//  JPJsonParser
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



// The bench links against the static library libjson.a.


#import <Foundation/Foundation.h>
#import "JPJson/JPRepresentationGenerator.h"
#import "JPJson/JPStreamSemanticActions.h"
#import "JPJson/JPAsyncJsonParser.h"
#import "JPJson/JPJsonParser.h"
#import "JPJson/NSData+JPJsonDetectEncoding.h"

#include "utilities/timer.hpp"
#include "utilities/MinMaxAvg.hpp"

#include <algorithm>
#include <ctime>
#include <string>

#include <boost/config.hpp>




#define USE_JSONKit

#if defined (USE_JSONKit)
#include "JSONKit.h"
#endif

#include <dispatch/dispatch.h>



//------------------------------------------------------------------------------
//
//  Runs benchmark test using the JPJsonLib Interface Methods 
//
//------------------------------------------------------------------------------


namespace {
    
    const char* NSStringEncodingToCStr(NSStringEncoding encoding) {
        switch (encoding) 
        {
            case NSUTF8StringEncoding: return "NSUTF8StringEncoding";
            case NSUTF16StringEncoding: return "NSUTF16StringEncoding";
            case NSUTF16BigEndianStringEncoding: return "NSUTF16BigEndianStringEncoding";
            case NSUTF16LittleEndianStringEncoding: return "NSUTF16LittleEndianStringEncoding";
            case NSUTF32StringEncoding: return "NSUTF32StringEncoding";
            case NSUTF32BigEndianStringEncoding: return "NSUTF32BigEndianStringEncoding";
            case NSUTF32LittleEndianStringEncoding: return "NSUTF32LittleEndianStringEncoding";
            default: return "invalid encoding";                
        }
    }
    
    
    const char* NSStringEncodingToUnicodeSchemeCStr(NSStringEncoding encoding) {
        switch (encoding) 
        {
            case NSUTF8StringEncoding: return "UTF-8";
            case NSUTF16StringEncoding: return "UTF-16";
            case NSUTF16BigEndianStringEncoding: return "UTF-16BE";
            case NSUTF16LittleEndianStringEncoding: return "UTF-16LE";
            case NSUTF32StringEncoding: return "UTF-32";
            case NSUTF32BigEndianStringEncoding: return "UTF-32BE";
            case NSUTF32LittleEndianStringEncoding: return "UTF-32LE";
            default: return "invalid encoding";                
        }
    }
    
    
    std::string JPJsonParserOptionsToString(JPJsonParserOptions opt)
    {
        if (opt == 0) {
            return "none";
        }
        std::string result;
        if (opt & JPJsonParserSignalErrorOnUnicodeNoncharacter) {
            result += "JPJsonParserSignalErrorOnUnicodeNoncharacter ";
        }
        if (opt & JPJsonParserSignalErrorOnUnicodeNoncharacter) {
            result += "JPJsonParserSignalErrorOnUnicodeNoncharacter ";
        }
        if (opt & JPJsonParserSubstituteUnicodeNoncharacter) {
            result += "JPJsonParserSubstituteUnicodeNoncharacter ";
        }
        if (opt & JPJsonParserRemoveUnicodeNoncharacter) {
            result += "JPJsonParserRemoveUnicodeNoncharacter ";
        }
        if (opt & JPJsonParserSignalErrorOnUnicodeNULLCharacter) {
            result += "JPJsonParserSignalErrorOnUnicodeNULLCharacter ";
        }
        if (opt & JPJsonParserSubstituteUnicodeNULLCharacter) {
            result += "JPJsonParserSubstituteUnicodeNULLCharacter ";
        }
        if (opt & JPJsonParserRemoveUnicodeNULLCharacter) {
            result += "JPJsonParserRemoveUnicodeNULLCharacter ";
        }
        if (opt & JPJsonParserIgnoreSpuriousTrailingBytes) {
            result += "JPJsonParserIgnoreSpuriousTrailingBytes ";
        }
        if (opt & JPJsonParserParseMultipleDocuments) {
            result += "JPJsonParserParseMultipleDocuments ";
        }
        if (opt & JPJsonParserParseMultipleDocumentsAsynchronously) {
            result += "JPJsonParserParseMultipleDocumentsAsynchronously ";
        }
        if (opt & JPJsonParserLogLevelDebug) {
            result += "JPJsonParserLogLevelDebug ";
        }
        if (opt & JPJsonParserLogLevelWarning) {
            result += "JPJsonParserLogLevelWarning ";
        }
        if (opt & JPJsonParserLogLevelError) {
            result += "JPJsonParserLogLevelError ";
        }
        if (opt & JPJsonParserLogLevelNone) {
            result += "JPJsonParserLogLevelNone ";
        }
        if (opt & JPJsonParserAllowComments) {
            result += "JPJsonParserAllowComments ";
        }
        if (opt & JPJsonParserAllowControlCharacters) {
            result += "JPJsonParserAllowControlCharacters ";
        }
        if (opt & JPJsonParserAllowLeadingPlusInNumbers) {
            result += "JPJsonParserAllowLeadingPlusInNumbers ";
        }
        if (opt & JPJsonParserAllowLeadingZerosInIntegers) {
            result += "JPJsonParserAllowLeadingZerosInIntegers ";
        }
        if (opt & JPJsonParserEncodedStrings) {
            result += "JPJsonParserEncodedStrings ";
        }
        if (opt & JPJsonParserCheckForDuplicateKey) {
            result += "JPJsonParserCheckForDuplicateKey ";
        }
        if (opt & JPJsonParserKeepStringCacheOnClear) {
            result += "JPJsonParserKeepStringCacheOnClear ";
        }
        if (opt & JPJsonParserCacheDataStrings) {
            result += "JPJsonParserCacheDataStrings ";
        }
        if (opt & JPJsonParserCreateMutableContainers) {
            result += "JPJsonParserCreateMutableContainers ";
        }
        if (opt & JPJsonParserNumberGeneratorGenerateAuto) {
            result += "JPJsonParserNumberGeneratorGenerateAuto ";
        }
        if (opt & JPJsonParserNumberGeneratorGenerateAutoWithDecimals) {
            result += "JPJsonParserNumberGeneratorGenerateAutoWithDecimals ";
        }
        if (opt & JPJsonParserNumberGeneratorGenerateStrings) {
            result += "JPJsonParserNumberGeneratorGenerateStrings ";
        }
        if (opt & JPJsonParserNumberGeneratorGenerateDecimals) {
            result += "JPJsonParserNumberGeneratorGenerateDecimals ";
        }
        if (opt & JPJsonParserGeneratorUseArenaAllocator) {
            result += "JPJsonParserGeneratorUseArenaAllocator ";
        }
        
        return result;
    }
    
    
    std::string JSONKitParseOptionsToString(JKParseOptionFlags opt) {
        if (opt == 0) {
            return "none";
        }
        std::string result;

        if (JKParseOptionStrict & opt) {
            result += "JKParseOptionStrict ";
        }
        if (JKParseOptionComments & opt) {
            result += "JKParseOptionComments ";
        }
        if (JKParseOptionUnicodeNewlines & opt) {
            result += "JKParseOptionUnicodeNewlines ";
        }
        if (JKParseOptionLooseUnicode & opt) {
            result += "JKParseOptionLooseUnicode ";
        }
        if (JKParseOptionPermitTextAfterValidJSON & opt) {
            result += "JKParseOptionPermitTextAfterValidJSON ";
        }

        return result;
    }
    
    
    
    std::string NSJSONSerializationParseOptionsToString(NSJSONReadingOptions opt) {
        if (opt == 0) {
            return "none";
        }
        std::string result;
        if (NSJSONReadingMutableContainers & opt) {
            result += "NSJSONReadingMutableContainers ";
        }
        if (NSJSONReadingMutableLeaves & opt) {
            result += "NSJSONReadingMutableLeaves ";
        }
        if (NSJSONReadingAllowFragments & opt) {
            result += "NSJSONReadingAllowFragments ";
        }
        if (opt == 0) {
            result = "none";
        }
        
        return result;
    }
    
    
    using utilities::timer;
    using utilities::MinMaxAvg;
    
    typedef     MinMaxAvg<double> MinMaxAvgTime;
    
    
    
    
    
    
    NSData* createDataFromFileInResourceFolder(NSString* path)
    {
        NSString* filePath = [@"Resources" stringByAppendingPathComponent:path];
        __autoreleasing NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:filePath 
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            [fileManager release];
            abort();
        }    
        return data;
    }
    
} // namespace     
    


#pragma mark -
namespace {    
    
    // -----------------------------------------------------------------------------
    //  bench_JPJsonParserWithNSString()
    //  Class JPJsonParser
    //  Using a NSString as input and interface method:
    //  +parseString:options:error:
    // -----------------------------------------------------------------------------
    void bench_JPJsonParserWithNSString(JPJsonParserOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JPJsonParserWithNSString %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Using a NSString as input and interface method:\n"
               "+parseString:options:error: (JPJsonParser)\n"
               "options: %s\n", JPJsonParserOptionsToString(opt).c_str());
        
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToCStr(encoding));
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        
                
        MinMaxAvg<double> te;
        timer t = timer();        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            NSError* error;
            NSString* input = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:encoding];
            if (input == nil) {
                NSLog(@"ERROR: Could not create a NSString from NSData object. Probably encoding error.");
                abort();
            }
            t.start();
            // This method creates and destroys the internal semantic actions
            // instance.
            id result = [JPJsonParser parseString:input 
                                          options:(JPJsonParserOptions)(opt)
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
            [input release];
            
            if (!result) {
                NSLog(@"PJsonParser ERROR: %@", error);
            }
            [pool release];
            if (!result)
                break;
        }
                        
        if (!gotError) {
            printf("JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n",
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }

        [data release];
        [pool release];
    }
    
    
    
    
    // -----------------------------------------------------------------------------
    //  bench_JPJsonParserWithNSData()
    //  Class JPJsonParser
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:options:error:
    // -----------------------------------------------------------------------------
    void bench_JPJsonParserWithNSData(JPJsonParserOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;

        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JPJsonParserWithNSData %d times.\n", N);
        printf("--------------------------------------------\n");    
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("Using a NSData with %s content as input and interface method:\n"
               "+parseData:options:error: (JPJsonParser)\n"
               "options: %s\n"
               "The input encoding will be detected automatically\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), JPJsonParserOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            NSError* error;
            t.start();
            // This method creates and destroys the internal semantic actions
            // instance.
            id result = [JPJsonParser parseData:data 
                                        options:(JPJsonParserOptions)opt
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
            if (!result) {
                NSLog(@"PJsonParser ERROR: %@", error);
            }
            [pool release];
            if (!result)
                break;
        }
        
        if (!gotError) {
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
        [data release];
        [pool release];
    }
    
    
    // -----------------------------------------------------------------------------
    //  bench_JPJsonParserWithNSDataReuseSemanticActionsObject()
    //  Class JPJsonParser
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:options:error:
    // -----------------------------------------------------------------------------
    void bench_JPJsonParserWithNSDataReuseSemanticActionsObject(JPJsonParserOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JPJsonParserWithNSDataReuseSemanticActionsObject %d times.\n", N);
        printf("--------------------------------------------\n");
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("Using a NSData with %s content as input and interface method:\n"
               "+parseData:options:error: (JPJsonParser)\n"
               "options: %s\n"
               "The input encoding will be detected automatically\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), JPJsonParserOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n",
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] initWithHandlerDispatchQueue:NULL];
        [sa configureWithOptions:opt];
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i)
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            t.start();
            gotError = ![JPJsonParser parseData:data semanticActions:sa];
            //id result = sa.result;
            t.stop();
            te.set(t.seconds());
            t.reset();
#if defined (XX_DEBUG)
            if ((i+1) == N) {
                NSLog(@"%@", result);
            }
#endif
            // deallocate result by clearing the sa
            [sa clear];
            if (gotError) {
                NSLog(@"PJsonParser ERROR: %@", sa.error);
            }
            [pool release];
            if (gotError)
                break;
        }
        
        if (!gotError) {
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n",
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        [sa release];
        [data release];
        [pool release];
    }
    
    
    
    
    // -----------------------------------------------------------------------------
    //  bench_JPJsonParserSAXStyle()
    //  Class JPJsonParser
    //  Using a NSData with UTF-8 content as input and interface method:
    //  +parseData:semanticActions:
    //  semantic actions class: JPStreamSemanticActions
    //  The input encoding will be detected automatically.
    // -----------------------------------------------------------------------------
    void bench_JPJsonParserSAXStyle(JPJsonParserOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JPJsonParserSAXStyle %d times.\n", N);
        printf("--------------------------------------------\n");    
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("Class JPJsonParser"
               "Using a NSData with %s content as input and interface method:\n"
               "+parseData:semanticActions:\n"
               "Semantic Actions: JPStreamSemanticActions  (SAX style)\n"
               "options: %s\n"
               "The input encoding will be detected automatically\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), JPJsonParserOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
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
            [sa configureWithOptions:opt];
            success = [JPJsonParser parseData:data semanticActions:sa];
            t.stop();
            te.set(t.seconds());
            t.reset();
            if (!success) {
                NSLog(@"PJsonParser ERROR: %@", sa.error);
            }
            [sa release];
            [pool release];
            if (!success)
                break;
        }
        
        if (success) {
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
        [data release];
        [pool release];
    }
    
    
    // -----------------------------------------------------------------------------
    //  bench_JPJsonParserIncludingDestruction()
    //  Class JPJsonParser
    //  Reading data from file, using interface method:
    //  +parseData:options:error:
    //  with options: 0
    //  The input encoding will be detected automatically.
    //  Timing includes destruction of objects
    // -----------------------------------------------------------------------------
    void bench_JPJsonParserIncludingDestruction(JPJsonParserOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JPJsonParserIncludingDestruction %d times.\n", N);
        printf("--------------------------------------------\n");    
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("Timing includes destruction of JSON representation\n");
        printf("Using a NSData with %s content as input and interface method:\n"
               "+parseData:options:error: (JPJsonParser)\n"
               "options: %s\n"
               "The input encoding will be detected automatically\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), JPJsonParserOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n",
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        t.start();
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            t.start();
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            NSError* error;
            // This method creates and destroys the internal semantic actions
            // instance.
            id result = [JPJsonParser parseData:data 
                                        options:(JPJsonParserOptions)opt
                                          error:&error];
            if (!result) {
                gotError = YES;
                NSLog(@"PJsonParser ERROR: %@", error);
            }
            [pool release];
            
            t.stop();
            te.set(t.seconds());
            
            if (!result)
                break;
        }
        
        if (!gotError) {
            NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
        [data release];
        [pool release];
    }
    
    
    
    // -----------------------------------------------------------------------------
    // bench_JPAsyncJsonParser
    // Asynchronous Parser, using a NSData with UTF-8 content as input.
    // Timing includes destruction of JSON objects, the semantic actions object is 
    // kept alive, giving it and advantage, since key strings are cached.\n"
    // So this test is not directly comparable to the others.
    // -----------------------------------------------------------------------------
    void bench_JPAsyncJsonParser(JPJsonParserOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JPAsyncJsonParser %d times.\n", N);
        printf("--------------------------------------------\n");    
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("Asynchronous Parser with options %s,"
               "\nusing a NSData with %s content as input.\n"
               "Timing measures the average time including destruction\n"
               "of JSON objects. The semantic actions object is kept alive,\n"
               "giving it and advantage, since key strings are cached.\n"
               "This test is not directly comparable to the others.\n",
               JPJsonParserOptionsToString(opt).c_str(),
               NSStringEncodingToUnicodeSchemeCStr(encoding));
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        __block BOOL gotError = NO;
        timer t = timer();
        t.start();
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
        JPRepresentationGenerator* sa = (JPRepresentationGenerator*)[parser semanticActions];
        [sa configureWithOptions:opt];
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
        sa = nil;
        
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
         NSLog(@"JPJsonParser: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
         te.min()*1e3, te.max()*1e3, te.avg()*1e3);
         }
         */
        [data release];
        [pool release];
    }
    
    
    
    
    
#if defined (USE_JSONKit)
    
#pragma mark - JSONKit
    
    
    void bench_JSONKitWithNSString(JKParseOptionFlags opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JSONKitWithNSString %d times.\n", N);
        printf("--------------------------------------------\n");    
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("using a NSString as input,\n"
               "interface method: mutableObjectWithData: (JSONDecoder),\n"
               "options %s\n", JSONKitParseOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            NSError* error;
            NSString* input = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:encoding];
            if (input == nil) {
                NSLog(@"ERROR: Could not create a NSString from NSData object. Probably encoding error.");
                abort();
            }
            
            t.start();
            JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:opt];
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
            
            if (!result) {
                gotError = YES;
                NSLog(@"ERROR: %@", error);
            }
            [pool release];
            if (!result)
                break;
        }
        
        if (!gotError) {
            NSLog(@"JSONKit: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
        [data release];
        [pool release];
    }
    
    
    
    
    void bench_JSONKitWithNSData(JKParseOptionFlags opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JSONKitWithNSData %d times.\n", N);
        printf("--------------------------------------------\n");
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("using a NSData with %s content as input,\n"
               "interface method: objectWithData: (JSONDecoder),\n"
               "options %s\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), JSONKitParseOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            NSError* error = nil;
            t.start();
            JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:opt];
            id result = [decoder objectWithData:data];
            [result retain];
            [decoder release];
            t.stop();
            te.set(t.seconds());
            t.reset();
            [result release];
            
            if (!result) {
                gotError = YES;
                NSLog(@"JSONKit ERROR: %@", error);
            }
            [pool release];
            if (!result)
                break;
        }
        
        if (!gotError) {
            NSLog(@"JSONKit: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n",
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
        [data release];
        [pool release];
    }
    
    void bench_JSONKitIncludingDestruction(JKParseOptionFlags opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_JSONKitIncludingDestruction %d times.\n", N);
        printf("--------------------------------------------\n"); 
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("Timing includes destruction of objects\n");
        printf("using a NSData with %s content as input,\n"
               "interface method: objectWithData: (JSONDecoder),\n"
               "options %s\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), JSONKitParseOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            t.start();
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            JSONDecoder* decoder = [[JSONDecoder alloc] initWithParseOptions:opt];
            id result = [decoder objectWithData:data];
            [result retain];
            [decoder release];
            [result release];
            
            
            if (!result) {
                gotError = YES;
                NSLog(@"JSONKit ERROR:<nil>");
            }
            
            [pool release];
                        
            t.stop();
            te.set(t.seconds());
            
            if (!result)
                break;
        }
        
        if (!gotError) {
            NSLog(@"JSONKit: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
        [data release];
        [pool release];
    }
    
#endif 
    
    
#pragma mark - NSJSONSerialization
    
    void bench_NSJSONSerialization(NSJSONReadingOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_NSJSONSerialization %d times.\n", N);
        printf("--------------------------------------------\n");    
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("using a NSData with %s content as input,\n"
               "interface method: JSONObjectWithData:options:error:, \n"
               "options: %s\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), NSJSONSerializationParseOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n",
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            NSError* error;
            t.start();
            id result = [NSJSONSerialization JSONObjectWithData:data options:opt error:&error];
            gotError = result == nil;
            [result retain];
            t.stop();
            te.set(t.seconds());
            t.reset();
            [result release];
            
            if (gotError) {
                NSLog(@"NSJSONSerialization ERROR: %@", error);
            }
            [pool release];
            
            if (!result)
                break;
        }
        
        
        if (!gotError) {
            NSLog(@"NSJSONSerialization: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n",
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        [data release];
        [pool release];
    }
    

    void bench_NSJSONSerializationIncludingDestruction(NSJSONReadingOptions opt, NSString* file, const int N, bool printInfo = false)
    {
        using namespace utilities;
        
        assert(N > 0);
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running bench_NSJSONSerializationIncludingDestruction %d times.\n", N);
        printf("--------------------------------------------\n");    
        NSData* data = createDataFromFileInResourceFolder(file);
        NSStringEncoding encoding = [data jpj_detectUnicodeNSStringEncoding];
        if (encoding == -1) {
            NSLog(@"ERROR: NSData doesn't contain text encoded in Unicode");
            abort();
        }
        printf("Timing includes destruction of objects, too\n");
        printf("using a NSData with %s content as input,\n"
               "interface method: JSONObjectWithData:options:error:, \n"
               "options: %s\n",
               NSStringEncodingToUnicodeSchemeCStr(encoding), NSJSONSerializationParseOptionsToString(opt).c_str());
        printf("Input file: %s, size: %d, encoding: %s\n", 
               [file UTF8String], (int)[data length], NSStringEncodingToUnicodeSchemeCStr(encoding));
        
        NSError* error;
        MinMaxAvg<double> te;
        timer t = timer();
        
        BOOL gotError = NO;
        error = nil;
        for (int i = 0; i < N; ++i) 
        {
            t.start();
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            id result = [NSJSONSerialization JSONObjectWithData:data options:opt error:&error];
            gotError = result == nil;
            [result retain];
            [result release];
            if (gotError) {
                NSLog(@"NSJSONSerialization ERROR: %@", error);
            }
            [pool release];            
            t.stop();
            te.set(t.seconds());
            if (!result)
                break;
        }
        
        if (!gotError) {
            NSLog(@"NSJSONSerialization: elapsed time for parsing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
        [data release];
        [pool release];
    }
    


} // namespace


#pragma mark -

int main (int argc, const char * argv[])
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
#if defined (DEBUG)
    int N = 1;
#else
    int N = 100;
#endif    
    
    
    time_t time_info;
    std::time(&time_info);
    printf("%s\n%s\n", ctime(&time_info), BOOST_COMPILER);
    NSLog(@"Start Bench");
    

    // -------------------------------------------------------------------------
    // Paths to JSON files which is used for all tests
    // -------------------------------------------------------------------------
    
    NSArray* testFiles = @[
                           @"apache_builds.json",
                           @"github_events.json",
                           @"instruments.json",
                           @"mesh.json",
                           @"mesh.pretty.json",
                           //@"nested.json",
                           @"random.json",
                           @"repeat.json",
                           @"truenull.json",
                           @"twitter_timeline.json",
                           @"update-center.json",
                           @"sample.json",
                           @"Test-UTF8-esc.json",
                           @"Test-UTF8.json"
                           ];
    
    for (id testFile in testFiles) {
//        {
//            NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//            bench_JPJsonParserSAXStyle(JPJsonParserOptions(JPJsonParserAllowUnicodeNoncharacter | JPJsonParserAllowUnicodeNULLCharacter), testFile, N);
//            [pool drain];
//        }
//        {
//            NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//            bench_JPJsonParserWithNSData(JPJsonParserOptions(JPJsonParserAllowUnicodeNoncharacter | JPJsonParserAllowUnicodeNULLCharacter), testFile, N);
//            [pool drain];
//        }
//        {
//            NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//            bench_NSJSONSerialization(NSJSONReadingOptions(0), testFile, N);
//            [pool drain];
//        }
//        {
//            NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
//            bench_JSONKitWithNSData(JKParseOptionFlags(0), testFile, N);
//            [pool drain];
//        }
        {
            NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
            bench_JPAsyncJsonParser(JPJsonParserOptions(JPJsonParserAllowUnicodeNoncharacter | JPJsonParserAllowUnicodeNULLCharacter), testFile, N);
            [pool drain];
        }

    
    }
    
    NSLog(@"\nFinished Bench");
    [pool drain];
    return 0;
}


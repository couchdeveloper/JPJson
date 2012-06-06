//
//  main.m
//  JPJsonWriter
//
//  Created by Andreas Grosam on 5/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

// The bench links against the JPJson Framework.



#import "JPJson/JPJsonWriter.h"
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPJsonWriterExtensions.h"

#include "utilities/timer.hpp"
#include "utilities/MinMaxAvg.hpp"
#include <algorithm>
#include <ctime>
#include <boost/config.hpp>
#include <dispatch/dispatch.h>

#include <string>

#import <Foundation/Foundation.h>


#define USE_JSONKit

#if defined (USE_JSONKit)
#include "JSONKit.h"
#endif




namespace {

    //
    // Path(s) to a JSON file(s) which is shall be used for creating the JSON object.
    //
    NSString* JSON_TEST_FILE = @"Test-UTF8-esc.json";
    //NSString* JSON_TEST_FILE = @"Test-UTF8.json";

}

namespace {
    
    using utilities::timer;
    using utilities::MinMaxAvg;
    
    typedef     MinMaxAvg<double> MinMaxAvgTime;
    
    
    
    std::string NSStringEncodingToString(NSStringEncoding encoding) {
        switch (encoding) 
        {
            case NSUTF8StringEncoding: return "NSUTF8StringEncoding";
            case NSUTF16StringEncoding: return "NSUTF16StringEncoding";
            case NSUTF16BigEndianStringEncoding: return "NSUTF16BigEndianStringEncoding";
            case NSUTF16LittleEndianStringEncoding: return "NSUTF16LittleEndianStringEncoding";
            case NSUTF32StringEncoding: return "NSUTF32StringEncoding";
            case NSUTF32BigEndianStringEncoding: return "NSUTF32BigEndianStringEncoding";
            case NSUTF32LittleEndianStringEncoding: return "NSUTF32LittleEndianStringEncoding";
            default: return "ERROR: invalid NSStringEncoding";                
        }
    }
    
    std::string JPUnicodeEncodingToString(JPUnicodeEncoding encoding) {
        switch (encoding) {
            case JPUnicodeEncoding_Unknown: return "JPUnicodeEncoding_Unknown";
            case JPUnicodeEncoding_UTF8: return "JPUnicodeEncoding_UTF8";
            case JPUnicodeEncoding_UTF16BE: return "JPUnicodeEncoding_UTF16BE";
            case JPUnicodeEncoding_UTF16LE: return "JPUnicodeEncoding_UTF16LE";
            case JPUnicodeEncoding_UTF32BE: return "JPUnicodeEncoding_UTF32BE";
            case JPUnicodeEncoding_UTF32LE: return "JPUnicodeEncoding_UTF32LE";
        }
        return "ERROR: invalid JPUnicodeEncoding";
    }
    
    
    std::string writerOptionsToString(JPJsonWriterOptions flags) 
    {
        std::string result;
        if ((flags & JPJsonWriterPrettyPrint) != 0) {
            result.append("JPJsonWriterPrettyPrint");
        }
        if ((flags & JPJsonWriterSortKeys) != 0) {
            if (!result.empty()) {
                result.append("|");
            }
            result.append("JPJsonWriterSortKeys");
        }
        if ((flags & JPJsonWriterEscapeUnicodeCharacters) != 0) {
            if (!result.empty()) {
                result.append("|");
            }
            result.append("JPJsonWriterEscapeUnicodeCharacters");
        }
        if ((flags & JPJsonWriterWriteBOM) != 0) {
            if (!result.empty()) {
                result.append("|");
            }
            result.append("JPJsonWriterWriteBOM");
        }
        if ((flags & JPJsonWriterEscapeSolidus) != 0) {
            if (!result.empty()) {
                result.append("|");
            }
            result.append("JPJsonWriterEscapeSolidus");
        }
        
        return result;
    }
    
} // namespace     



namespace {
    
    void bench_JPJsonWriter(const int N, JPUnicodeEncoding encoding, JPJsonWriterOptions writerOptions)
    {
        using namespace utilities;
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running JPJsonParser bench 1 %d times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Using file '%s' as input for generating the JSON object:\n"
               "API method: [JPJsonWriter] +dataWithObject:encoding:options:error:\n"
               "Writer Options: %s\n"
               "Output encoding: %s\n",
               [JSON_TEST_FILE UTF8String],
               writerOptionsToString(writerOptions).c_str(),
               JPUnicodeEncodingToString(encoding).c_str());
        
        NSError* error;
        NSData* data = [[NSData alloc] initWithContentsOfFile:[@"Resources" stringByAppendingPathComponent:JSON_TEST_FILE]
                                                      options:NSDataReadingUncached 
                                                        error:&error];
        if (data == nil) {
            NSLog(@"ERROR: could not load file. %@", error);
            NSFileManager* fileManager = [[NSFileManager alloc] init];
            NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
            abort();
        }
        
        id json = [JPJsonParser parseData:data 
                                  options:(JPJsonParserOptions)(0)
                                    error:&error];
        [data release]; data = nil;

        if (json == nil) {
            NSLog(@"ERROR: failed to parse JSON data. %@", error);
            abort();
        }
        
        

        MinMaxAvg<double> te;
        timer t = timer();
        
        bool gotError = false;
        for (int i = 0; i < N; ++i) 
        {
            NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
            t.start();

            NSData* result = [JPJsonWriter dataWithObject:json encoding:encoding options:writerOptions error:&error];
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
            
            if (!result) {
                gotError = true;
                break;
            }
        }
        
                
        if (gotError) {
            NSLog(@"ERROR: %@", error);
        }
        else {
            NSLog(@"JPJsonWriter: elapsed time for serializing:\nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
                  te.min()*1e3, te.max()*1e3, te.avg()*1e3);
        }
        
    }
}


namespace {
    
//    enum  {
//        JPJsonWriterPrettyPrint =               1UL << 0,
//        JPJsonWriterSortKeys =                  1UL << 1,
//        JPJsonWriterEscapeUnicodeCharacters =   1UL << 2,
//        JPJsonWriterWriteBOM =                  1UL << 3,
//        JPJsonWriterEscapeSolidus =             1UL << 4
//    };
//    typedef NSUInteger JPJsonWriterOptions;
    
    void bench_JPJsonWriter1() {
        bench_JPJsonWriter(10000, JPUnicodeEncoding_UTF8, 0);
    }
    
}






int main(int argc, const char * argv[])
{

    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
    bench_JPJsonWriter1();
        
    [pool drain];
    
    return 0;
}


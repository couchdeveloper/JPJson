//
//  JPJsonWriterTest.mm
//  Test
//
//  Created by Andreas Grosam on 7/20/11.
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


#import "JPJson/JPJsonWriter.h"
#import "JPJson/JPJsonParser.h"
#include "gtest/gtest.h"




// for testing
#import <Foundation/Foundation.h>



namespace {
    
    
    class JPJsonWriterTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JPJsonWriterTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JPJsonWriterTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    /*
     JPJsonWriter Options:
     
        JPJsonWriterPrettyPrint
        JPJsonWriterSortKeys
     
     
     
     JPJsonWriter Interface:
     
     + (NSData*)dataWithObject:(id)value 
                       options:(JPJsonWriterOptions)options 
                         error:(NSError**)error;
     
     
     
     */ 
    
    
    TEST_F(JPJsonWriterTest, BasicWriterTest) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSError* error;
        
        // Each JSON string will be parsed in order to get a JSON representation
        NSArray* strings = [NSArray arrayWithObjects:
                            @"{}",
                            @"[]",
                            @"[1]",
                            @"[1,2]",
                            @"[\"string\"]",
                            @"[false,1]",
                            @"[true,1]",
                            @"[null,1]",
                            @"[[[]]]",
                            @"[{}]",
                            @"[{\"key1\":1,\"key2\":2}]",
                            nil];
        
        for (NSString* inString in strings) 
        {
            id json = [JPJsonParser parseString:inString 
                                        options:(JPJsonParserOptions)0
                                          error:&error];
            ASSERT_TRUE(json != nil);
            
            NSData* jsonData = [JPJsonWriter dataWithObject:json
                                                     encoding:JPUnicodeEncoding_UTF8
                                                      options:0 
                                                        error:&error];
            
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes] 
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            
            EXPECT_TRUE([inString isEqualToString:jsonString]);
            [jsonString release];
        }
        
        [pool drain];
    }
    

    TEST_F(JPJsonWriterTest, TestUTF8Strings) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSError* error;
        NSArray* strings = [NSArray arrayWithObjects:
                            @"[\"µöä\"]",
                            @"[\"艾小薇\"]",
                            nil];
        
        for (NSString* inString in strings) 
        {
            id json = [JPJsonParser parseString:inString 
                                        options:(JPJsonParserOptions)JPJsonWriterSortKeys
                                          error:&error];
            ASSERT_TRUE(json != nil);
            
            NSData* jsonData = [JPJsonWriter dataWithObject:json 
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0 
                                                      error:&error];
            
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes] 
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];

            NSLog(@"jsonString:\n%@", jsonString);
            
            EXPECT_TRUE([inString isEqualToString:jsonString]);
        }
        
        [pool drain];
    }
    
    
    TEST_F(JPJsonWriterTest, TestNumbersNoConversion)
    {
        // The given number strings in the input shall be converted to the
        // same string when writing to the output.
        
        // NSDecimal:
        // NSDecimalNumber, an immutable subclass of NSNumber, provides an 
        // object-oriented wrapper for doing base-10 arithmetic. An instance 
        // can represent any number that can be expressed as 
        // mantissa x 10^exponent 
        // where mantissa is a decimal integer up to 38 digits long, and 
        // exponent is an integer from –128 through 127.
            
        
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSError* error;
        NSArray* strings = [NSArray arrayWithObjects:
                            @"[0]",
                            @"[1]",
                            @"[-1]",
                            @"[1,2,3]",
                            @"[1.1,1.2,1.3]",
                            @"[1.123456789012345]",
                            @"[1234567890123456]",
                            @"[9999999999999999]",
                            @"[123456789012345678901234567890]",
                            @"[1.0e+01]",
                            @"[1.0e+03]",
                            @"[1.0e+10]",
                            @"[1.0e+100]",
                            @"[1.0e+245]",
                            @"[1.1e-01]",
                            @"[1.1e-09]",
                            @"[1.1e-10]",
                            @"[1.1e-100]",
                            @"[1e-01]",
                            // Test max precision of NSDecimalNumber
                            @"[0.1234567890123456789012345678]",
                            @"[0.12345678901234567890123456789012345678901234567890]",
                            // Test max range of NSDecimalNumber's exponent
                            @"[0.1234567890123456789012345678e+127]",
                            @"[1.0000000000000000000000000001e-127]",
                            nil];
        
        for (NSString* inString in strings) 
        {
            NSLog(@"JSON input:  \"%@\"", inString);
            
            id json = [JPJsonParser parseString:inString 
                                        options:(JPJsonParserOptions)JPJsonWriterSortKeys
                                          error:&error];
            if (json == nil) {
                NSLog(@"ERROR: %@", error);
            }
            ASSERT_TRUE(json != nil);
            
            // json is an array, with one element which is a NSDecimalNumber:
            NSString* numberString = [[json objectAtIndex:0] stringValue];
            NSLog(@"Number: %@", numberString);
            
            
            NSData* jsonData = [JPJsonWriter dataWithObject:json 
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0 
                                                      error:&error];
            
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes] 
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            
            
            NSLog(@"JSON output: \"%@\"", jsonString);
            EXPECT_EQ( std::string([inString UTF8String]), std::string([jsonString UTF8String]) );
        }
        
        [pool drain];
    }

    
    
}
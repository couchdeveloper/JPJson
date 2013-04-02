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
#include <gtest/gtest.h>
#include <dispatch/dispatch.h>




// for testing
#import <Foundation/Foundation.h>

#if defined (DEBUG)
#define DLog(...) NSLog(@"%s %@", __PRETTY_FUNCTION__, [NSString stringWithFormat:__VA_ARGS__])
#else
#define DLog(...) do { } while (0)
#endif


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
        @autoreleasepool {
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
            }

        }
        
    }
    

    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsBooleanTest)
    {
        @autoreleasepool {
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithBool:NO],
                              [NSNumber numberWithBool:YES],
                              [NSNumber numberWithBool:0],
                              [NSNumber numberWithBool:1],
                              [NSNumber numberWithBool:2],
                              [NSNumber numberWithBool:0xFF],
                              nil];
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            EXPECT_TRUE([@"[false,true,false,true,true,true]" isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    
    
    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsCharTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `char` will be mapped to an integer:
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithChar:NO],
                              [NSNumber numberWithChar:YES],
                              [NSNumber numberWithChar:0],
                              [NSNumber numberWithChar:1],
                              [NSNumber numberWithChar:2],
                              [NSNumber numberWithChar:0xFF],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            EXPECT_TRUE([@"[0,1,0,1,2,-1]" isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    

    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsUnsignedCharTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `unsigned char` will be mapped to an integer:
            
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithUnsignedChar:NO],
                              [NSNumber numberWithUnsignedChar:YES],
                              [NSNumber numberWithUnsignedChar:0],
                              [NSNumber numberWithUnsignedChar:1],
                              [NSNumber numberWithUnsignedChar:2],
                              [NSNumber numberWithUnsignedChar:0xFF],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            EXPECT_TRUE([@"[0,1,0,1,2,255]" isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    
    
    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsShortTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `short` will be mapped to an integer:
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithShort:0],
                              [NSNumber numberWithShort:1],
                              [NSNumber numberWithShort:-1],
                              [NSNumber numberWithShort:SHRT_MAX],
                              [NSNumber numberWithShort:SHRT_MIN],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%d,%d,%d]", 0,1,-1,SHRT_MAX,SHRT_MIN];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    
    
    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsUnsignedShortTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `unsigned short` will be mapped to an integer:
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithUnsignedShort:0],
                              [NSNumber numberWithUnsignedShort:1],
                              [NSNumber numberWithUnsignedShort:USHRT_MAX],
                              nil];
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%u]", 0,1,USHRT_MAX];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }

    
    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsIntTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `int` will be mapped to an integer:
            
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithInt:0],
                              [NSNumber numberWithInt:1],
                              [NSNumber numberWithInt:-1],
                              [NSNumber numberWithInt:INT_MAX],
                              [NSNumber numberWithInt:INT_MIN],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%d,%d,%d]", 0,1,-1,INT_MAX, INT_MIN];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    

    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsUnsignedIntTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `unsigned int` will be mapped to an integer:
            
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithUnsignedInt:0],
                              [NSNumber numberWithUnsignedInt:1],
                              [NSNumber numberWithUnsignedInt:UINT_MAX],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%u]", 0,1,UINT_MAX];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    

    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsLongTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `unsigned int` will be mapped to an integer:
            
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithLong:0],
                              [NSNumber numberWithLong:1],
                              [NSNumber numberWithLong:-1],
                              [NSNumber numberWithLong:LONG_MAX],
                              [NSNumber numberWithLong:LONG_MIN],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%d,%ld,%ld]", 0,1,-1,LONG_MAX,LONG_MIN];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    
    
    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsUnsignedLongTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `unsigned long` will be mapped to an integer:
            
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithUnsignedLong:0],
                              [NSNumber numberWithUnsignedLong:1],
                              [NSNumber numberWithUnsignedLong:ULONG_MAX],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%lu]", 0,1,ULONG_MAX];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    
    
    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsLongLongTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `unsigned long long` will be mapped to an integer:
            
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithLongLong:0],
                              [NSNumber numberWithLongLong:1],
                              [NSNumber numberWithLongLong:-1],
                              [NSNumber numberWithLongLong:LLONG_MAX],
                              [NSNumber numberWithLongLong:LLONG_MIN],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%d,%lld,%lld]", 0,1,-1,LLONG_MAX,LLONG_MIN];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    
    
    TEST_F(JPJsonWriterTest, BasicWriterNSNumberAsUnsignedLongLongTest)
    {
        @autoreleasepool {
            // A NSNumber with an underlaying type `unsigned long long` will be mapped to an integer:
            
            NSArray* array = [[NSArray alloc] initWithObjects:
                              [NSNumber numberWithUnsignedLongLong:0],
                              [NSNumber numberWithUnsignedLongLong:1],
                              [NSNumber numberWithUnsignedLongLong:ULLONG_MAX],
                              nil];
            
            NSError* error;
            NSData* jsonData = [JPJsonWriter dataWithObject:array
                                                   encoding:JPUnicodeEncoding_UTF8
                                                    options:0
                                                      error:&error];
            NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                            length:[jsonData length]
                                                          encoding:NSUTF8StringEncoding];
            NSString* expectedString = [NSString stringWithFormat:@"[%d,%d,%llu]", 0,1,ULLONG_MAX];
            EXPECT_TRUE([expectedString isEqualToString:jsonString]) << "with json string\n" << [jsonString UTF8String];
        }
    }
    
    
    
//    + (NSNumber *)numberWithFloat:(float)value;
//    + (NSNumber *)numberWithDouble:(double)value;
//    + (NSNumber *)numberWithBool:(BOOL)value;
//    + (NSNumber *)numberWithInteger:(NSInteger)value NS_AVAILABLE(10_5, 2_0);
//    + (NSNumber *)numberWithUnsignedInteger:(NSUInteger)value NS_AVAILABLE(10_5, 2_0);
    
    
    
    TEST_F(JPJsonWriterTest, TestUTF8Strings)
    {
        @autoreleasepool {
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
                
                DLog(@"jsonString:\n%@", jsonString);
                
                EXPECT_TRUE([inString isEqualToString:jsonString]);
            }
        }
    }

    
    TEST_F(JPJsonWriterTest, TestUTF8StringsEscaped) 
    {
        @autoreleasepool {
            NSError* error;
            NSArray* strings = [NSArray arrayWithObjects:
                                @"[\"80\u540e\uff0c\u5904\u5973\u5ea7\uff0c\u65e0\u4e3b\u7684\u808b\u9aa8\uff0c\u5b85+\u5fae\u8150\u3002\u5b8c\u7f8e\u63a7\uff0c\u7ea0\u7ed3\u63a7\u3002\u5728\u76f8\u4eb2\u7684\u6253\u51fb\u4e0e\u88ab\u6253\u51fb\u4e2d\u4e0d\u65ad\u6210\u957fing\"]",
                                nil];
            
            for (NSString* inString in strings)
            {
                id json = [JPJsonParser parseString:inString
                                            options:(JPJsonParserOptions)JPJsonWriterSortKeys
                                              error:&error];
                ASSERT_TRUE(json != nil);
                
                NSData* jsonData = [JPJsonWriter dataWithObject:json
                                                       encoding:JPUnicodeEncoding_UTF8
                                                        options:JPJsonWriterEscapeUnicodeCharacters
                                                          error:&error];
                
                BOOL result = [jsonData writeToFile:@"tmp.json" options:0 error:&error];
                ASSERT_TRUE(result);
                
                
                NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                                length:[jsonData length]
                                                              encoding:NSUTF8StringEncoding];
                
                DLog(@"jsonString:\n%@", jsonString);
                
                EXPECT_TRUE([inString isEqualToString:jsonString]);
            }
        }
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
            
        @autoreleasepool {
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
                                @"[1.000000000000000000000000000e-128]",
                                @"[1.000000000000000000000000000e+127]",
                                // @"[1.0000000000000000000000000001e-127]",  would fail!
                                nil];
            
            int idx = 0;
            for (NSString* inString in strings)
            {
                DLog(@"JSON input:  \"%@\"", inString);
                
                id json = [JPJsonParser parseString:inString
                                            options:(JPJsonParserOptions)JPJsonWriterSortKeys
                                              error:&error];
                // Note: The tests above shall not cause the parser to fail
                ASSERT_TRUE(json != nil);
                if (json == nil) {
                    NSLog(@"ERROR: %@", error);
                } else  {
                    // json is an array, with one element which is a NSDecimalNumber:
                    DLog(@"Number: %@", [[json objectAtIndex:0] stringValue]);
                    NSData* jsonData = [JPJsonWriter dataWithObject:json
                                                           encoding:JPUnicodeEncoding_UTF8
                                                            options:0
                                                              error:&error];
                    ASSERT_TRUE(jsonData != nil);
                    if (!jsonData) {
                        NSLog(@"ERROR: %@", error);
                    }
                    
                    NSString* jsonString = [[NSString alloc] initWithBytes:[jsonData bytes]
                                                                    length:[jsonData length]
                                                                  encoding:NSUTF8StringEncoding];
                    ASSERT_TRUE(jsonString != nil);
                    
#if defined (DEBUG)
                    printf("JSON input:   \"%s\"\n", [inString UTF8String]);
                    printf("JSON output:  \"%s\"\n", [jsonString UTF8String]);
#endif                
                    EXPECT_EQ( std::string([inString UTF8String]), std::string([jsonString UTF8String]) );
                }
                ++idx;
            }
        }
    }

    
    
    
    
    
    
    TEST_F(JPJsonWriterTest, TestStreamAPI)
    {
        @autoreleasepool {
            NSDictionary* object = [NSDictionary dictionaryWithObjectsAndKeys:
                                    @"Aaaaaaaaaabbbbbbbbbb", @"first_name",
                                    @"Ccccccccccdddddddddd", @"last_name",
                                    nil];
            
            
            dispatch_semaphore_t sem = dispatch_semaphore_create(0);
            
            const int BufferSize = 4;// For testing, change buffer size
            CFReadStreamRef readStream = NULL;
            CFWriteStreamRef writeStream = NULL;
            CFStreamCreateBoundPair(NULL, &readStream, &writeStream, BufferSize);
            NSInputStream* istream = (__bridge id)readStream;
            NSOutputStream* ostream = (__bridge id)writeStream;
            
            // Invoke the JPJsonWriter asynchronously on its own thread:
            __block NSError* serializerError;
            dispatch_async(dispatch_get_global_queue(0, 0), ^{
                [ostream open];
                NSUInteger count = [JPJsonWriter serializeObject:object
                                                        toStream:ostream
                                                         options:0
                                                           error:&serializerError];
                if (count == 0) {
                    NSLog(@"NSJSONSerialization Error while writing to stream: %@", serializerError);
                }
                [ostream close];
                dispatch_semaphore_signal(sem);
            });
            
            const char* expectedResult = "{\"first_name\":\"Aaaaaaaaaabbbbbbbbbb\",\"last_name\":\"Ccccccccccdddddddddd\"}";
            const int BSIZE = 1024;
            char actualResult[BSIZE] = {};
            int left = BSIZE;
            
            [istream open];
            char* p = &actualResult[0];
            NSInteger len;
            do {
                len = [istream read:(uint8_t*)p maxLength:left];
                if (len) {
                    p += len;
                    left -= len;
                }
            } while (len > 0);
            [istream close];
            
            dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
            dispatch_release(sem);
            
            EXPECT_EQ( std::string(expectedResult), std::string(actualResult) );
        }
    }
    
}
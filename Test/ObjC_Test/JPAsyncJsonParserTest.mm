//
//  JPAsyncJsonParserTest.mm
//  Test
//
//  Created by Andreas Grosam on 9/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "JPJson/JPAsyncJsonParser.h"
#include "JPJson/JPRepresentationGenerator.h"
#include "JPJson/JPJsonWriter.h"
#include <gtest/gtest.h>




// for testing
#import <Foundation/Foundation.h>
#include <dispatch/dispatch.h>


#include <sys/types.h>
#include <unistd.h>


namespace {
    
    class JPAsyncJsonParserTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JPAsyncJsonParserTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JPAsyncJsonParserTest() {
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
     JPJsonParser Options:
     
     JPJsonParserCheckForDuplicateKey
     JPJsonParserKeepStringCacheOnClear
     JPJsonParserCacheDataStrings
     JPJsonParserParseMultipleDocuments
     JPJsonParserParseMultipleDocumentsAsynchronously
     JPJsonParserNumberGeneratorGenerateAuto
     JPJsonParserNumberGeneratorGenerateStrings
     JPJsonParserNumberGeneratorGenerateDecimals
     
     JPJsonParser Interface:
     
     + (id) parseString:(NSString*)string 
     options:(JPJsonParserOptions)options 
     error:(NSError**)error;
     
     + (id) parseUTF8Data:(NSData*)data 
     options:(JPJsonParserOptions)options
     error:(NSError**)error;
     
     
     */ 
    

    //
    //  Test init methods and proper default values.
    //
    TEST_F(JPAsyncJsonParserTest, Init1) 
    {
        @autoreleasepool {
            JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
            EXPECT_TRUE(parser != nil);
            
            JPRepresentationGenerator* sa = (JPRepresentationGenerator*)parser.semanticActions;
            EXPECT_TRUE(sa != nil);
            
            EXPECT_TRUE( sa.startJsonHandlerBlock == nil );
            EXPECT_TRUE( sa.endJsonHandlerBlock == nil );
            EXPECT_TRUE( sa.completionHandlerBlock == nil );
            EXPECT_TRUE( sa.errorHandlerBlock == nil );
            
            EXPECT_EQ(0, parser.bufferQueueSize);
            EXPECT_EQ(0, parser.bufferQueueCapacity);
            
            EXPECT_EQ(NO, [parser isRunning]);
        }
    }
    
    
    
    //
    // Test basic functionality to quickly detect most obvious bugs
    //
    TEST_F(JPAsyncJsonParserTest, BasicParserTestParseString1) 
    {
        @autoreleasepool {
            JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
            
            // Precautions when using a synchronous queue:
            // A synchronous queue transmitts the content via one or more data buffers
            // to the parser. The parser on the other side will consume character by
            // character until it finds the last character terminating the JSON
            // document. At this stage, however, the parser can't know per se if the
            // data stream is actually at its end. If the semantic actions option
            // ignoreSpuriousTrailingBytes is set to FLASE or if parseMultipleDocuments
            // is set to true, the parser will try to receive the next character or
            // possibly receive EOF, which indicates the end of the data stream.
            // An out-of-bound Unicode NULL (U+0000) will also terminate the parse
            // loop with a warning.
            // If there is no out-of-bound Unicode NULL (U+0000) character in the
            // current buffer, or if the previous significant character was the last
            // character in the current buffer, then it will try to receive the
            // next buffer which may cause it to block until a new buffer is avail-
            // able.
            // In order to avoid the parser to wait infiniteley while it tries to
            // find an EOF in the next buffer, the sender of the content shall ex-
            // plicitly send an EOF or alternatively the content shall contain an
            // out-of-bound terminating NULL character (Unicode U+0000) immediately
            // after the last significant character of the JSON text.
            //
            // Alternatively, if the parser is configured to parse only one JSON
            // document, we can set the option ignoreSpuriousTrailingBytes to
            // TRUE, in which case the parser stops immediately after receiving
            // the last significant character of the JSON document (a ']' or a '}').
            // Any trailing bytes are ignored.
            
            // If the parser is configured to parse only a single JSON document,
            // "out-of-bound" characters will be treated as an error - except a
            // trailing zero (U+0000) will merely issue a warning.
            
            bool parseMultipleDocuments = parser.semanticActions.parseMultipleDocuments;
            EXPECT_EQ(false, parseMultipleDocuments);
            parser.semanticActions.ignoreSpuriousTrailingBytes = YES;
            
            [parser start];
            
            // Producer
            
            const char json[] = "[0, 1, 2, 3, true, false, null, \"string1\", \"string2\", \"string3\"]";
            size_t len = strlen(json);
            NSData* buffer = [[NSData alloc] initWithBytes:json length:len];
            bool delivered = [parser parseBuffer:buffer];
            EXPECT_EQ(true, delivered);
            
            while ([parser isRunning]) {
                usleep(1000*100);
            }
            
            JPRepresentationGenerator* sa = (JPRepresentationGenerator*)parser.semanticActions;
            parser = nil;
            
            NSError* err = [sa error];
            EXPECT_EQ(nil, err);
            if (err) {
                NSLog(@"sa error: %@", err);
            }
        }
    }
    
    
    //
    // Test basic functionality to quickly detect most obvious bugs
    //
    TEST_F(JPAsyncJsonParserTest, DISABLED_BasicParserTestParseString2) 
    {
        @autoreleasepool {
            JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
            [parser start];
            
            
            //dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
            //^{         });
            
            // Producer
            bool delivered = false;
            const char json[] = "[[[[}}}}";  // invalid JSON - this gives an error at pos 5
            size_t len = strlen(json);
            NSData* buffer = [[NSData alloc] initWithBytes:json length:len];
            delivered = [parser parseBuffer:buffer];
            EXPECT_EQ(true, delivered);
            
            while ([parser isRunning]) {
                usleep(1000*100);
            }
            
            JPRepresentationGenerator* sa = (JPRepresentationGenerator*)parser.semanticActions;
            NSError* err = [sa error];
            NSLog(@"sa error: %@", err);
        }
    }
    
    
#pragma mark - Specific Tests    
    
    
#pragma mark - Handlers    
    //
    //  Test if the handlers are called 
    //
    TEST_F(JPAsyncJsonParserTest, Handlers) 
    {
        @autoreleasepool {
            dispatch_semaphore_t sem = dispatch_semaphore_create(0);
            
            JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
            JPRepresentationGenerator* sa = (JPRepresentationGenerator*)parser.semanticActions;
            
            __block bool startCalled = false;
            __block bool endCalled = false;
            __block bool completionCalled = false;
            __block bool errorCalled = false;
            
            sa.startJsonHandlerBlock = ^{
                startCalled = true;
            };
            sa.endJsonHandlerBlock = ^(id jsonContainer){
                endCalled = true;
            };
            sa.errorHandlerBlock = ^(NSError* error){
                NSLog(@"TEST ERROR: %@", error);
                errorCalled = true;
            };
            sa.completionHandlerBlock = ^{
                completionCalled = true;
                dispatch_semaphore_signal(sem);
            };
            sa.parseMultipleDocuments = YES;
            
            [parser start];
            
            // Producer
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                // JSON Text, consisting of three valid documents and one syntax error at the end:
                const char json[] = "[0]  [1]  [2] }";
                size_t len = strlen(json);
                
                // When constructing a CFData object with some valid JSON text, we either
                // need to tell the parser the end of string through a zero terminated byte,
                // or we send it a nil buffer. Here, we choose the first method:
                NSData* buffer = [[NSData alloc] initWithBytes:json length:len + 1]; // contains the zero termination
                bool delivered = [parser parseBuffer:buffer];
                EXPECT_EQ(true, delivered);
            });
            
            dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
            
            EXPECT_EQ(true, startCalled);
            EXPECT_EQ(true, endCalled);
            EXPECT_EQ(true, completionCalled);
            EXPECT_EQ(true, errorCalled);
        }
    }
    
    
    
#pragma mark - BOM Detection    
    //
    //  Test whether a BOM and encoding will be detected for all
    //  possible Unicode encodings
    //    
    TEST_F(JPAsyncJsonParserTest, DetectBOM) 
    {
        @autoreleasepool {
            //   00 00 FE FF	UTF-32, big-endian
            //   FF FE 00 00	UTF-32, little-endian
            //   FE FF          UTF-16, big-endian
            //   FF FE          UTF-16, little-endian
            //   EF BB BF       UTF-8
            
            struct test_s {
                const char*     input_;
                size_t          input_len_;
                bool            hasBOM_;
                const char*     encoding_;
                bool            parser_result_;
            };
            
            test_s tests[] = {
                { "[]",                             2,  false,  "UTF-8",    true },
                { "\xEF\xBB\xBF""[]",               5,  true,   "UTF-8",    true },
                { "\x00[\x00]",                     4,  false,  "UTF-16BE", true },
                { "\xFE\xFF""\x00[\x00]",           6,  true,   "UTF-16BE", true },
                { "[\x00]\x00",                     4,  false,  "UTF-16LE", true },
                { "\xFF\xFE""[\x00]\x00",           6,  true,   "UTF-16LE", true },
                { "\x00\x00\x00[\x00\x00\x00]",         8,  false,  "UTF-32BE", true },
                { "\x00\x00\xFE\xFF""\x00\x00\x00[\x00\x00\x00]", 12, true,   "UTF-32BE", true },
                { "[\x00\x00\x00]\x00\x00\x00",             8,  false,  "UTF-32LE", true },
                { "\xFF\xFE\x00\x00""[\x00\x00\x00]\x00\x00\x00", 12, true,   "UTF-32LE", true }
            };
            
            const int count = sizeof(tests)/sizeof(test_s);
            test_s* first = tests;
            test_s* last = tests + count;
            int idx = 0;
            while (first != last)
            {
                dispatch_semaphore_t sem = dispatch_semaphore_create(0);
                
                JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
                JPRepresentationGenerator* sa = (JPRepresentationGenerator*)parser.semanticActions;
                
                __block bool startCalled = false;
                __block bool endCalled = false;
                __block bool completionCalled = false;
                __block bool errorCalled = false;
                __block bool success = false;
                
                sa.startJsonHandlerBlock = ^{
                    startCalled = true;
                };
                sa.endJsonHandlerBlock = ^(id jsonContainer){
                    endCalled = true;
                    success = jsonContainer != nil;
                };
                sa.errorHandlerBlock = ^(NSError* error){
                    NSLog(@"TEST ERROR: %@", error);
                    errorCalled = true;
                };
                sa.completionHandlerBlock = ^{
                    completionCalled = true;
                    dispatch_semaphore_signal(sem);
                };
                
                [parser start];
                
                // Producer
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                    // JSON Text, consisting of three valid documents and one syntax error at the end:
                    const char* json = (*first).input_;
                    size_t len = (*first).input_len_;
                    NSData* buffer = [[NSData alloc] initWithBytes:json length:len];
                    bool delivered = [parser parseBuffer:buffer];
                    EXPECT_EQ(true, delivered);
                    delivered = [parser parseBuffer:nil];
                    EXPECT_EQ(true, delivered);
                });
                
                dispatch_time_t timeout =
#if defined (DEBUG)
                DISPATCH_TIME_FOREVER;
#else
                dispatch_time(DISPATCH_TIME_NOW, 1*NSEC_PER_SEC);
#endif
                dispatch_semaphore_wait(sem, timeout);
                
                EXPECT_EQ(true, startCalled) << "at index " << idx;
                EXPECT_EQ(true, endCalled) << "at index " << idx;
                EXPECT_EQ(true, completionCalled) << "at index " << idx;
                EXPECT_EQ(false, errorCalled) << "at index " << idx;
                
                EXPECT_EQ((*first).hasBOM_, sa.hasBOM) << "at index " << idx;
                EXPECT_EQ(std::string((*first).encoding_), std::string([sa.inputEncoding UTF8String])) << "at index " << idx;
                EXPECT_EQ((*first).parser_result_, success) << "at index " << idx;
                                
                ++first;
                ++idx;
            }
        }
    }
    
    
    
    
    
    
/*    
    TEST_F(JPJsonParserTest, DISABLED_BasicParserTestParseData) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        const char* s = "[\"Sample string\"]";
        NSData* data = [[NSData alloc] initWithBytes:s length:strlen(s)];
        NSError* error;
        
        id result = [JPJsonParser parseData:data 
                                    options:(JPJsonParserOptions)0 
                                      error:&error];
        EXPECT_TRUE( result != nil );
        EXPECT_EQ(1, [result count]);
        
        [pool drain];
    }
    
    TEST_F(JPJsonParserTest, CheckDuplicateKeyError) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        
        NSString* jsonText = @"{\"key1\":1,\"key1\":2}";
        NSError* error;
        
        // 1) Duplicate key errors are ignored:
        //    In case of a duplicate key, the key-value pair cannot be inserted. 
        //    Whether the old value remains, or the value will be overridden by
        //    the subsequent insertion is undefined.
        id result1 = [JPJsonParser parseString:jsonText 
                                       options:(JPJsonParserOptions)0 
                                         error:&error];
        EXPECT_TRUE( result1 != nil );
        EXPECT_EQ(1, [result1 count]);
        
        
        // 2) Duplicate key errors are detected:
        //    In case of a duplicate key error, the result will be nil and if
        //    error is not nil it will be set accordingly.
        id result2 = [JPJsonParser parseString:jsonText 
                                       options:(JPJsonParserOptions)JPJsonParserCheckForDuplicateKey 
                                         error:&error];
        EXPECT_TRUE( result2 == nil );
        EXPECT_TRUE( error != nil );
        NSLog(@"CheckDuplicateKeyError: error message: %@", error);
        
        
        [pool drain];
    }
*/  
    
    
#pragma mark - Cancel
    
    //
    //  Check if we can cancel.
    //    
    TEST_F(JPAsyncJsonParserTest, StartCancel1) 
    {
        @autoreleasepool {
            JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
            
            // Setup SA
            JPRepresentationGenerator* sa = (JPRepresentationGenerator*)(parser.semanticActions);
            sa.startJsonHandlerBlock = ^{
                NSLog(@"startJsonHandlerBlock called");
            };
            
            sa.endJsonHandlerBlock = ^(id jsonContainer){
                NSLog(@"endJsonHandlerBlock called");
            };
            
            sa.completionHandlerBlock = ^{
                NSLog(@"completionHandlerBlock called");
            };
            
            sa.errorHandlerBlock = ^(NSError* error) {
                NSLog(@"errorHandlerBlock called");
            };
            
            
            bool started = [parser start];
            EXPECT_EQ(true, started);
            EXPECT_EQ(YES, [parser isRunning]);
            started = [parser start];
            EXPECT_EQ(false, started);
            EXPECT_EQ(YES, [parser isRunning]);
            [parser cancel];
            int retryCount = 0;
            bool stopped = true;
            while ([parser isRunning]) {
                if (retryCount++ > 100) {
                    stopped = false;
                    break;
                }
                usleep(1000*100);
            }
            EXPECT_EQ(true, stopped);
        }
    }
    
    
    TEST_F(JPAsyncJsonParserTest, StartCancel2) 
    {
        @autoreleasepool {
            JPAsyncJsonParser* parser = [[JPAsyncJsonParser alloc] init];
            [parser start];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                bool delivered = true;
                // Sending buffers of size 1 - pretty unusual case.
                // We need to send a BOM, otherwise the detect_encoding function would
                // be called which needs to look ahead, and we can't reset to the start
                // of the stream anymore for parsing.
                const char json[] = "\xEF\xBB\xBF""[[[[";
                size_t len = strlen(json);
                for (int i = 0; i < len; ++i) {
                    NSData* buffer = [[NSData alloc] initWithBytes:&json[i] length:1];
                    delivered = [parser parseBuffer:buffer];
                    EXPECT_EQ(i <= 6, delivered) << "for i equals " << i << std::endl;
                    if (!delivered)
                        break;
                }
                EXPECT_EQ(true, delivered);
            });
            
            usleep(1000*100);
            [parser cancel];
            int retryCount = 0;
            bool stopped = true;
            while ([parser isRunning]) {
                if (retryCount++ > 10) {
                    stopped = false;
                    break;
                }
                usleep(1000*100);
            }
            
            EXPECT_EQ(true, stopped);
        }
    }
    
    
    
    

#pragma mark - Parse Options 
    // 1. Multiple/Single JSON documents
    
    
#pragma mark - Bogus Input    
    
    // 1. Test an empty text, or EOF
    // 2. Malformed Unicode
    // 3. Check for duplicate key for JSON Objects
    // 4. Various Error messages
    
    
}
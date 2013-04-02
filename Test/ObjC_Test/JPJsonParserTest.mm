//
//  JPJsonParserTest.mm
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

#include "JPJson/JPJsonParser.h"
#include "JPJson/JPJsonWriter.h"
#include "JPJson/JPRepresentationGenerator.h"

#include <gtest/gtest.h>




// for testing
#import <Foundation/Foundation.h>





namespace {
    
    
    class JPJsonParserTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JPJsonParserTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JPJsonParserTest() {
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
     
     
    // Handling of Unicode noncharacters 
    // (Mutual exclusive flags)
    JPJsonParserSignalErrorOnNoncharacter       
    JPJsonParserSubstituteUnicodeNoncharacter   
    JPJsonParserSkipUnicodeNoncharacter         
    
    // Log behavior
    // (Mutual exclusive flags)
    JPJsonParserLogLevelDebug                   
    JPJsonParserLogLevelWarning                 
    JPJsonParserLogLevelError                   
    JPJsonParserLogLevelNone                    
    
    // non_conformance_flags (can be ored)
    // (not yet implemented!)
    JPJsonParserAllowComments                                
    JPJsonParserAllowControlCharacters              
    JPJsonParserAllowLeadingPlusInNumbers       
    JPJsonParserAllowLeadingZerosInIntegers     
    
    JPJsonParserParseMultipleDocuments          

    JPJsonParserCheckForDuplicateKey  
    JPJsonParserKeepStringCacheOnClear          
    JPJsonParserCacheDataStrings                
    JPJsonParserCreateMutableContainers         
    
    // Number generator options
    // (mutual exclusive flags)
    JPJsonParserNumberGeneratorGenerateAuto     
    JPJsonParserNumberGeneratorGenerateStrings  
    JPJsonParserGeneratorGenerateDecimals       
    
    

 
    JPJsonParser convenience interface:

    + (id) parseString:(NSString*)string 
               options:(JPJsonParserOptions)options 
                 error:(NSError**)error;
 
    + (id) parseData:(NSData*)data 
             options:(JPJsonParserOptions)options
               error:(NSError**)error;
 

    
    JPJsonParser advanced interface:

    + (BOOL) parseData:(NSData*)data 
       semanticActions:(JPSemanticActionsBase*)semanticActions;

*/ 
    
    //
    // Test whether the method parseString:options:error
    // will be invoked properly
    //     
    TEST_F(JPJsonParserTest, BasicParserTestParseString) 
    {
        @autoreleasepool {
            NSString* jsonText = @"[\"Sample string\"]";
            NSError* error;
            
            id result = [JPJsonParser parseString:jsonText
                                          options:(JPJsonParserOptions)0
                                            error:&error];
            EXPECT_TRUE( result != nil );
            EXPECT_EQ(1, [result count]);
        }
    }
    

    //
    // Test whether the method parseData:options:error
    // will be invoked properly
    //     
    TEST_F(JPJsonParserTest, BasicParserTestParseData) 
    {
        @autoreleasepool {
            const char* s = "[\"Sample string\"]";
            NSData* data = [[NSData alloc] initWithBytes:s length:strlen(s)];
            NSError* error;
            
            id result = [JPJsonParser parseData:data
                                        options:(JPJsonParserOptions)0
                                          error:&error];
            EXPECT_TRUE( result != nil );
            EXPECT_EQ(1, [result count]);
        }
    }
    
    
    
    
    
    TEST_F(JPJsonParserTest, CheckDuplicateKeyError) 
    {
        @autoreleasepool {
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
            
            error = nil;
            // 2) Duplicate key errors are detected:
            //    In case of a duplicate key error, the result will be nil and if
            //    error is not nil it will be set accordingly.
            id result2 = [JPJsonParser parseString:jsonText
                                           options:(JPJsonParserOptions)JPJsonParserCheckForDuplicateKey
                                             error:&error];
            EXPECT_TRUE( result2 == nil );
            EXPECT_TRUE( error != nil );
            NSLog(@"CheckDuplicateKeyError: error message: %@", error);
        }
    }
    
    
    TEST_F(JPJsonParserTest, ParseJsonTestDocument) 
    { 
        @autoreleasepool {
            NSString* fileName = @"Resources/Test-UTF8-esc.json";
            NSError* error;
            NSData* data = [[NSData alloc] initWithContentsOfFile:fileName
                                                          options:NSDataReadingUncached
                                                            error:&error];
            if (data == nil) {
                NSLog(@"ERROR: %@", error);
            }
            ASSERT_TRUE(data != nil);
            
            id result = [JPJsonParser parseData:data
                                        options:0
                                          error:&error];
            EXPECT_TRUE( result != nil );
            
            
#if defined (DEBUG)
            NSLog(@"\n%@", [result descriptionWithLocale:NULL indent:0]);
#endif        
        }
    }
    
    
    TEST_F(JPJsonParserTest, JsonNumberMapping_Integers) 
    {
        // The conversion and mapping of JSON number will be done as
        // follows:
        // 
        // I)  Integer JSON Number
        //
        // 1)   If the number of digits of the JSON integer number is eqpual or
        //      smaller than the maximal number of digits (in decimal base) that 
        //      can be represented by a signed int without overflow, then a 
        //      NSNumber with underlaying type signed int will be generated.
        //
        // 2)   Otherwise, if the number of digits of the JSON integer number is
        //      equal or smaller than the maximal number of digits (in decimal
        //      base) that can be represented by a signed long  without overflow,
        //      then a NSNumber with an underlaying type signed long will be 
        //      generated.
        //
        // 3)   Otherwise, if the number of digits of the JSON integer number is
        //      equal or smaller than the maximal number of digits (in decimal
        //      base) that can be represented by a signed long long without
        //      overflow, then a NSNumber with an underlaying type signed long
        //      long will be generated.
        //
        // 4)   Otherwise, if the number of digits of the JSON integer number is
        //      equal or smaller than the maximal number of digits (in decimal
        //      base) that can be represented by a NSDecimalNumber without
        //      loosing precision, then a NSDecimalNumber will be generated.
        //
        // 5)   Otherwise, a NSDecimalNumber will be generated, and if this is
        //      successful, a warning will be logged to the error console to 
        //      indicate the possibly loss of precision while converting a
        //      JSON Number to a NSDecimalNumber.
        //
        // 6)   Otherwise, the conversion fails, and the parser stopps parsing
        //      with a corresponding runtime error.
        //
        // Note: -objCType for NSDecimalNumber returns "d".
        
        
        const int maxSignedShortDigits = std::numeric_limits<short>::digits10;
        const int maxSignedIntDigits = std::numeric_limits<int>::digits10;
        const int maxSignedLongDigits = std::numeric_limits<signed long>::digits10;
        const int maxSignedLongLongDigits = std::numeric_limits<signed long long>::digits10;
        const int maxNSDecimalDigits = 38;
        
        std::string s1 = std::string(maxSignedShortDigits, '9');
        std::string s2 = std::string(maxSignedIntDigits, '9');
        std::string s3 = std::string(maxSignedLongDigits, '9');
        std::string s4 = std::string(maxSignedLongLongDigits, '9');
        std::string s5 = std::string(maxNSDecimalDigits, '9');
        
        
        struct test_s {
            NSString* json_format;
            const char* json_arg;
            bool result; 
            int error_code;
            Class type;
            const char* encoding;
        };
        
        
        const test_s tests[] = {
            {@"[%s]", "0",       true, 0, [NSNumber class], @encode(int)},
            {@"[%s]", "-1",      true, 0, [NSNumber class], @encode(int)},
            {@"[%s]", "999999999",      true, 0, [NSNumber class], @encode(int)},
            {@"[%s]", "-999999999",      true, 0, [NSNumber class], @encode(int)},
            {@"[%s]", "9999999991",      true, 0, [NSNumber class], @encode(long)},
            {@"[%s]", "-9999999991",      true, 0, [NSNumber class], @encode(long)},
            {@"[%s]", "999999999999999999",      true, 0, [NSNumber class], @encode(long long)},
            {@"[%s]", "-999999999999999999",      true, 0, [NSNumber class], @encode(long long)},
            {@"[%s]", "9999999999999999991",      true, 0, [NSDecimalNumber class], "d"},
            {@"[%s]", "-9999999999999999991",      true, 0, [NSDecimalNumber class], "d"},
            {@"[%s]", "12345678901234567890123456789012345678",      true, 0, [NSDecimalNumber class], "d"},
            {@"[%s]", "99999999999999999999999999999999999999",      true, 0, [NSDecimalNumber class], "d"},
            {@"[%s]", "-12345678901234567890123456789012345678",      true, 0, [NSDecimalNumber class], "d"},
            {@"[%s]", "-99999999999999999999999999999999999999",      true, 0, [NSDecimalNumber class], "d"},
            {@"[%s]", "999999999999999999999999999999999999991",      true, 0, [NSDecimalNumber class], "d"},
            {@"[%s]", "-999999999999999999999999999999999999991",      true, 0, [NSDecimalNumber class], "d"},
            
        };
        
        
        @autoreleasepool {
            
            const test_s* first = tests;
            const test_s* last = first + sizeof(tests)/sizeof(test_s);
            
            while (first != last)
            {
                NSString* json_text = [NSString stringWithFormat:(*first).json_format, (*first).json_arg];
                //NSLog(@"JSON input:  \"%@\"", json_text);
                NSError* error;
                id json = [JPJsonParser parseString:json_text
                                            options:(JPJsonParserOptions)0
                                              error:&error];
                if ((*first).result) {
                    EXPECT_TRUE(json != nil);
                    
                    if (json) {
                        id number = [json objectAtIndex:0];
                        //NSString* numberString = [number stringValue];
                        //NSLog(@"Number: %@, type: %@, encoding: %s", numberString, [number class], [number objCType]);
                        EXPECT_EQ(std::string([number objCType]),  std::string((*first).encoding));
                    }
                }
                else {
                    EXPECT_TRUE(json == nil);
                    if (json == nil) {
                        NSLog(@"ERROR: %@", error);
                    }
                }
                ++first;
            }
        }
    }


    TEST_F(JPJsonParserTest, JsonNumberMapping_Floats) 
    {
        // The conversion and mapping of JSON number will be done as
        // follows:
        // 
        //
        // II)  Float JSON Numbers
        //
        // 1)   If the number of significant digits of the JSON float number is 
        //      eqpual or smaller than the maximal number of digits (in decimal 
        //      base) that can be represented by a double, then a NSNumber with 
        //      underlaying type double will be generated. If the resulting 
        //      value is out of range, a range error will be signaled.
        // 
        //
        //      Otherwise, if the number of significant digits of the JSON float 
        //      number is greater than the maximal number of digits (in decimal
        //      base) that can be represented by a double and if the exponent is
        //      also greater than 127 or smaller than -127, then a NSNumber with
        //      underlaying type double will be generated and a WARNING will be
        //      logged to the console about loosing precision.
        //
        //      Otherwise, if the number of significant digits of the JSON float 
        //      number is smaller than the maximal number of digits (in decimal
        //      base) that can be represented by a NSDecimalNumber and if the 
        //      exponent is also smaller than 128 or greater than -128, then a 
        //      NSDecimalNumber will be generated.
        //         
        //      Otherwise, (the number of significant digits of the JSON float 
        //      number is greater than the maximal number of digits (in decimal
        //      base) that can be represented by a NSDecimalNumber and the
        //      exponent is within range [-127 .. 127]) a NSDecimalNumber will 
        //      be generates and a WARNING will be logged to the console about 
        //      loosing precision.
        //
        
        // Note: -objCType for NSDecimalNumber returns "d".
        
        
        const int maxDoubleDigits = std::numeric_limits<double>::digits10;
        const int maxNSDecimalDigits = 38;
        
        std::string s1 = std::string(maxDoubleDigits, '9');
        std::string s2 = std::string(maxNSDecimalDigits, '9');
        
        struct test_s {
            NSString* json_format;
            const char* json_arg;
            bool result; 
            int error_code;
            Class type;
            const char* encoding;
        };
        
        const test_s tests[] = {            
            // Floats
            {@"[%s]", "0.0",       true, 0, [NSNumber class], @encode(double)},
            {@"[%s]", "1.0",       true, 0, [NSNumber class], @encode(double)},
            {@"[%s]", "-1.0",      true, 0, [NSNumber class], @encode(double)},
            {@"[%s]", "1e+00",     true, 0, [NSNumber class], @encode(double)},
            {@"[%s]", "-1e+00",    true, 0, [NSNumber class], @encode(double)},
            {@"[%s]", "1e-00",     true, 0, [NSNumber class], @encode(double)},
            {@"[%s]", "-1e-00",    true, 0, [NSNumber class], @encode(double)},
            {@"[0.%s]", s1.c_str(),true, 0, [NSNumber class], @encode(double)},
            {@"[0.%s]", s2.c_str(),true, 0, [NSNumber class], "d"},
            {@"[%s]", "0.1234567890123456789012345678901234567890",true, 0, [NSNumber class], "d"},
            {@"[%s]", "0.1e200",true, 0, [NSNumber class], "d"},
        };
        
        @autoreleasepool {
            const test_s* first = tests;
            const test_s* last = first + sizeof(tests)/sizeof(test_s);
            while (first != last)
            {
                NSString* json_text = [NSString stringWithFormat:(*first).json_format, (*first).json_arg];
                NSLog(@"JSON input:  \"%@\"", json_text);
                NSError* error;
                id json = [JPJsonParser parseString:json_text
                                            options:(JPJsonParserOptions)0
                                              error:&error];
                if ((*first).result) {
                    EXPECT_TRUE(json != nil);
                    if (json) {
                        id number = [json objectAtIndex:0];
                        NSString* numberString = [number stringValue];
                        NSLog(@"Number: %@, type: %@, encoding: %s", numberString, [number class], [number objCType]);
                        EXPECT_EQ(std::string([number objCType]),  std::string((*first).encoding));
                        //EXPECT_EQ((*first).type, [number class] );
                    }
                }
                else {
                    EXPECT_TRUE(json == nil);
                    if (json == nil) {
                        NSLog(@"ERROR: %@", error);
                    }
                }
                ++first;
            }
        }
    }

    
    
#pragma mark - BOM Detection    
    //
    //  Test whether a BOM and encoding will be detected for all
    //  possible Unicode encodings.
    //  Note: there is a more detailed BOM test in unicode_test/unicode_detect_bom_test.cpp
    
    TEST_F(JPJsonParserTest, DetectBOM) 
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
                //          input_                         input_len_   hasBOM_ encding_    parser_result_
                //            { "[]",                                             2,  false,  "UTF-8",    true },
                //            { "\xEF\xBB\xBF""[]",                               5,  true,   "UTF-8",    true },
                //            { "\x00[\x00]",                                     4,  false,  "UTF-16BE", true },
                //            { "\xFE\xFF""\x00[\x00]",                           6,  true,   "UTF-16BE", true },
                //            { "[\x00]\x00",                                     4,  false,  "UTF-16LE", true },
                //            { "\xFF\xFE""[\x00]\x00",                           6,  true,   "UTF-16LE", true },
                { "\x00\x00\x00[\x00\x00\x00]",                     8,  false,  "UTF-32BE", true }
                //            { "\x00\x00\xFE\xFF""\x00\x00\x00[\x00\x00\x00]",   12, true,   "UTF-32BE", true },
                //            { "[\x00\x00\x00]\x00\x00\x00",                     8,  false,  "UTF-32LE", true },
                //            { "\xFF\xFE\x00\x00""[\x00\x00\x00]\x00\x00\x00",   12, true,   "UTF-32LE", true }
            };
            
            const int count = sizeof(tests)/sizeof(test_s);
            test_s* first = tests;
            test_s* last = tests + count;
            int idx = 0;
            while (first != last)
            {
                JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] init];
                
                const char* json = (*first).input_;
                size_t len = (*first).input_len_;
                NSData* buffer = [[NSData alloc] initWithBytes:json length:len];
                BOOL success = [JPJsonParser parseData:buffer semanticActions:sa];
                
                EXPECT_EQ((*first).parser_result_, success) << "at index " << idx;
                EXPECT_EQ((*first).hasBOM_, sa.hasBOM) << "at index " << idx;
                EXPECT_EQ(std::string((*first).encoding_), std::string([sa.inputEncoding UTF8String])) << "at index " << idx;
                if (success) {
                    EXPECT_TRUE(sa.error == nil && sa.result != nil) << "at index " << idx;
                } else
                {
                    EXPECT_TRUE(sa.error != nil && sa.result == nil) << "at index " << idx;
                }
                
                ++first;
                ++idx;
            }
        }
    }
    
    
#if 0    
    TEST_F(JPJsonParserTest, JsonTestSuite)  
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        NSFileManager* fileManager = [[NSFileManager alloc] init];
        
        NSString* fullPath = [[fileManager currentDirectoryPath] stringByAppendingPathComponent:@"Resources/TestJson"];        
        NSArray* extensions = [NSArray arrayWithObject:@"json"];        
        NSArray* fileList = [[fileManager contentsOfDirectoryAtPath:fullPath error:nil] 
                                pathsMatchingExtensions:extensions];
        
        NSLog(@"file list: %@", fileList);

        [pool drain];
    }
#endif
    
    
    TEST_F(JPJsonParserTest, JPJsonParserOptionsUnicodeNoncharactersHandling)  
    {
        @autoreleasepool {
            NSString* jsonText = @"[\"abc\uFFFEdef\"]";
            
            NSError* error = nil;
            id json = [JPJsonParser parseString:jsonText
                                        options:(JPJsonParserOptions)JPJsonParserSignalErrorOnNoncharacter
                                          error:&error];
            EXPECT_EQ(nil, json);
            EXPECT_TRUE(error != nil);
            EXPECT_TRUE([[error description] rangeOfString:@"encountered Unicode noncharacter"].length != 0);
            
            error = nil;
            json = [JPJsonParser parseString:jsonText
                                     options:(JPJsonParserOptions)JPJsonParserSubstituteUnicodeNoncharacter
                                       error:&error];
            EXPECT_TRUE(json != nil);
            EXPECT_TRUE(error == nil);
            NSString* value = [json objectAtIndex:0];
            // Unicode replacement character: U+FFFD
            const NSString* repl_str = @"abc\uFFFDdef";
            EXPECT_TRUE([repl_str isEqualToString:value]);
            
#if 0 // The option `JPJsonParserSkipUnicodeNoncharacter`is not yet implemented
            error = nil;
            json = [JPJsonParser parseString:jsonText
                                     options:(JPJsonParserOptions)JPJsonParserSkipUnicodeNoncharacter
                                       error:&error];
            EXPECT_TRUE(json != nil);
            EXPECT_TRUE(error == nil);
            value = [json objectAtIndex:0];
            // Unicode replacement character: U+FFFD
            const NSString* str = @"abcdef";
            EXPECT_TRUE([str isEqualToString:value]);
#endif        
        }
    }
    

    TEST_F(JPJsonParserTest, JPJsonParserOptionsIgnoreSpuriousTrailingBytes) 
    {
        @autoreleasepool {
            NSString* jsonText = @"[\"abcdef\"] x y z";
            
            NSError* error = nil;
            id json = [JPJsonParser parseString:jsonText
                                        options:(JPJsonParserOptions)0
                                          error:&error];
            EXPECT_EQ(nil, json);
            EXPECT_TRUE(error != nil);
            EXPECT_TRUE([[error description] rangeOfString:@"extra characters at end of json document not allowed"].length != 0);
            
            
            
            error = nil;
            json = [JPJsonParser parseString:jsonText
                                     options:(JPJsonParserOptions)JPJsonParserIgnoreSpuriousTrailingBytes
                                       error:&error];
            EXPECT_TRUE(json != nil);
            EXPECT_TRUE(error == nil);
        }
    }
    
    
    TEST_F(JPJsonParserTest, JPJsonParserOptionsParseMultipleDocuments)  
    {
        @autoreleasepool {
            NSString* s = @"[\"abc\"] \n\t[\"def\"][\"ghi\"]";
            NSData* jsonText = [NSData dataWithBytes:[s UTF8String] length:[s lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
            
            
            JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] init];
            sa.parseMultipleDocuments = YES;
            
            __block int count = 0;
            sa.endJsonHandlerBlock = ^(id result) {
                ++count;
            };
            sa.errorHandlerBlock = ^(NSError* error) {
                NSLog(@"%@", error);
            };
            
            BOOL success = [JPJsonParser parseData:jsonText semanticActions:sa];
            EXPECT_EQ(YES, success);
            EXPECT_EQ(3, count);
        }
    }


    TEST_F(JPJsonParserTest, JPJsonParserOptionsParseMultipleDocuments_ignoreSpuriousTrailingBytes)  
    {
        @autoreleasepool {
            NSString* s = @"[\"abc\"] [\"def\"][\"ghi\"] x y z";
            NSData* jsonText = [NSData dataWithBytes:[s UTF8String] length:[s lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
            
            
            JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] init];
            sa.parseMultipleDocuments = YES;
            sa.ignoreSpuriousTrailingBytes = YES;
            
            __block int count = 0;
            sa.endJsonHandlerBlock = ^(id result) {
                ++count;
            };
            sa.errorHandlerBlock = ^(NSError* error) {
                NSLog(@"%@", error);
            };
            
            BOOL success = [JPJsonParser parseData:jsonText semanticActions:sa];
            EXPECT_EQ(YES, success);
            EXPECT_EQ(3, count);
        }
    }
    
    
    
    TEST_F(JPJsonParserTest, LargeJSONString) 
    {
        @autoreleasepool {
            typedef std::vector<char> json_text_t;
            
            // create the JSON input:
            const size_t Size = 128*1024;
            json_text_t json_text;
            json_text.push_back('[');
            json_text.push_back('\"');
            
            for (int i = 0; i < Size; ++i) {
                json_text.push_back('a');
            }
            json_text.push_back('\"');
            json_text.push_back(']');
            
            NSData* jsonText = [NSData dataWithBytes:&json_text[0] length:json_text.size()];
            
            
            NSError* error = nil;
            id json = [JPJsonParser parseData:jsonText
                                      options:(JPJsonParserOptions)0
                                        error:&error];
            EXPECT_TRUE(json != nil);
            EXPECT_TRUE(error == nil);
            EXPECT_TRUE(([json isKindOfClass:[NSArray class]] == YES));
            
            NSUInteger count = [json count];
            EXPECT_EQ(1U, count);
            
            id json_value = [json objectAtIndex:0];
            EXPECT_TRUE(([json_value isKindOfClass:[NSString class]] == YES));
            
            NSUInteger length = [json_value lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
            EXPECT_EQ(Size, length);
        }
    }
    

}

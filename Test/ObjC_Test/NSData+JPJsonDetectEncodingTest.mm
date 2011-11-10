//
//  NSData+JPJsonDetectEncodingTest.mm
//  Test
//
//  Created by Andreas Grosam on 11/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JPJson/NSData+JPJsonDetectEncoding.h"
#include "gtest/gtest.h"

namespace {
    
    
/*
     Returns a NSStringEncoding constant corresponding to one of the Unicode 
     encoding schemes:
     
    NSUTF8StringEncoding
    NSUTF16BigEndianStringEncoding
    NSUTF16LittleEndianStringEncoding    
    NSUTF32BigEndianStringEncoding
    NSUTF32LittleEndianStringEncoding
 
     The receiver's content should contain a JSON text. If the Unicode encoding 
     could not be detected, returns -1.
     
    -(NSStringEncoding) jpj_detectUnicodeNSStringEncoding;
*/
    
    
    class NSDataJPJsonDetectEncodingTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        NSDataJPJsonDetectEncodingTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~NSDataJPJsonDetectEncodingTest() {
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
    
    
    TEST_F(NSDataJPJsonDetectEncodingTest, DetectUnicodeNSStringEncoding) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        
        //   00 00 FE FF	UTF-32, big-endian
        //   FF FE 00 00	UTF-32, little-endian
        //   FE FF          UTF-16, big-endian
        //   FF FE          UTF-16, little-endian
        //   EF BB BF       UTF-8
        
        
        const size_t SIZE = 16;
        struct test_s {
            const char s_[SIZE];
            NSStringEncoding result_;
        };
        
        
        
        test_s tests[] = {
            // with BOM
            {"\x00\x00\xFE\xFF""xx",        NSUTF32BigEndianStringEncoding}, // 0
            {"\xFF\xFE\x00\x00""xx",        NSUTF32LittleEndianStringEncoding},
            {"\xFE\xFF""xx",                NSUTF16BigEndianStringEncoding},
            {"\xFF\xFE""xx",                NSUTF16LittleEndianStringEncoding},
            {"\xEF\xBB\xBF""xx",            NSUTF8StringEncoding},
            
            {"\x00\x00\xFE\xFF""xx",        NSUTF32BigEndianStringEncoding},   // 5
            {"\xFF\xFE\x00\x00""xx",        NSUTF32LittleEndianStringEncoding},
            {"\xFE\xFF""xx",                NSUTF16BigEndianStringEncoding},
            {"\xFF\xFE""xx",                NSUTF16LittleEndianStringEncoding},
            {"\xEF\xBB\xBF""xx",            NSUTF8StringEncoding},
            
            {"[]",                          NSUTF8StringEncoding},                     // 10
            {"",                            -1},
            
            // Broken BOM, or unknown encoding
            {"\x00\x00\xFE""[]",            -1},                     // 12
            {"\xFF\xFE\x00""[]",            -1},
            {"\xFE""[]",                    -1},
            {"\xFF""[]",                    -1},
            {"\xEF\xBB""[]",                -1},
            
            // No BOM, "[]"
            {"\x5B\x5D",                            NSUTF8StringEncoding},
            {"\x00\x5B\x00\x5D",                    NSUTF16BigEndianStringEncoding},
            {"\x5B\x00\x5D\x00",                    NSUTF16LittleEndianStringEncoding},
            {"\x00\x00\x00\x5B\x00\x00\x00\x5D",    NSUTF32BigEndianStringEncoding},
            {"\x5B\x00\x00\x00\x5D\x00\x00\x00",    NSUTF32LittleEndianStringEncoding}
        };
        

        test_s* first = tests;
        test_s* last = tests + sizeof(tests)/sizeof(test_s);
        
        int idx = 0;
        while (first != last) {
            const char* p = first->s_;
            NSData* data = [[NSData alloc] initWithBytes:p length:SIZE];
            NSStringEncoding result = [data jpj_detectUnicodeNSStringEncoding];
            EXPECT_EQ(first->result_, result) << "at index " << idx;
            [data release];
            ++first;
            ++idx;
        }

        
        [pool drain];
    }
}
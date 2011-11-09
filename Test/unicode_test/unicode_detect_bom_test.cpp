//
//  unicode_detect_bom_test.cpp
//  Test
//
//  Created by Andreas Grosam on 9/22/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "json/unicode/unicode_detect_bom.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <iomanip>
#include <iterator>
#include <algorithm>




namespace {
    
    
    using namespace json::unicode;
    
    
    class unicode_detect_bom_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        unicode_detect_bom_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~unicode_detect_bom_test() {
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
    
    
    
    TEST_F(unicode_detect_bom_test, Test1) {
        
        // 
        //  template <typename Iterator>
        //  int detect_bom(Iterator& first, Iterator last) 
        //
        //  Returns:
        //     >0: success, returns a values of UnicodeBOM
        //  Errors
        //     -1: unexpected EOF
        //
        //   enum UnicodeBOM {
        //       UNICODE_BOM_UTF_8     =  1,
        //       UNICODE_BOM_UTF_16LE  =  2,
        //       UNICODE_BOM_UTF_16BE  =  3,
        //       UNICODE_BOM_UTF_32LE  =  4,
        //       UNICODE_BOM_UTF_32BE  =  5
        //   };

        
        //   00 00 FE FF	UTF-32, big-endian
        //   FF FE 00 00	UTF-32, little-endian
        //   FE FF          UTF-16, big-endian
        //   FF FE          UTF-16, little-endian
        //   EF BB BF       UTF-8
        
        struct test_s {
            const char *s_;
            int len_;
            int result_;
            int distance_;
        };
        
        test_s tests[] = {
            {"\x00\x00\xFE\xFF""abcde", 4+5, UNICODE_BOM_UTF_32BE, 4}, // 0
            {"\xFF\xFE\x00\x00""abcde", 4+5, UNICODE_BOM_UTF_32LE, 4},
            {"\xFE\xFF""abcde",         2+5, UNICODE_BOM_UTF_16BE, 2},
            {"\xFF\xFE""abcde",         2+5, UNICODE_BOM_UTF_16LE, 2},
            {"\xEF\xBB\xBF""abcde",     3+5, UNICODE_BOM_UTF_8, 3},
            
            {"\x00\x00\xFE\xFF""",      4, UNICODE_BOM_UTF_32BE, 4},   // 5
            {"\xFF\xFE\x00\x00""",      4, UNICODE_BOM_UTF_32LE, 4},
            {"\xFE\xFF""",              2, UNICODE_BOM_UTF_16BE, 2},
            {"\xFF\xFE""",              2, UNICODE_BOM_UTF_16LE, 2},
            {"\xEF\xBB\xBF""",          3, UNICODE_BOM_UTF_8, 3},

            {"abcd",                    0+3, 0, 0},                     // 10
            {"",                        0,  -1, 0},
            
            // Broken BOM, or unknown encoding
            {"\x00\x00\xFE""abc",       3+3, 0, 3},                     // 12
            {"\xFF\xFE\x00""abc",       3+3, 0, 3},
            {"\xFE""abc",               1+3, 0, 1},
            {"\xFF""abc",               1+3, 0, 1},
            {"\xEF\xBB""abc",           2+3, 0, 2},

            // EOF before BOM
            {"\x00\x00\xFE",            3, -1, 3},                      // 17
            {"\xFF\xFE\x00",            3, -1, 3},
            {"\xFE",                    1, -1, 1},
            {"\xFF",                    1, -1, 1},
            {"\xEF\xBB",                2, -1, 2}
        };
        
        
        test_s* first = tests;
        test_s* last = tests + sizeof(tests)/sizeof(test_s);
        
        int idx = 0;
        while (first != last) {
            const char* p = first->s_;
            int result = json::unicode::detect_bom(p, (first->s_ + first->len_));
            EXPECT_EQ(first->result_, result) << "at index " << idx;
            EXPECT_EQ(first->distance_, std::distance(first->s_, p)) << "at index " << idx;
            ++first;
            ++idx;
        }
        
    }
}
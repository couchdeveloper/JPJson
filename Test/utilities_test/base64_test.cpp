//
//  base64_test.cpp
//  Test
//
//  Created by Andreas Grosam on 11/15/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "json/utility/base64.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <iomanip>
#include <dispatch/dispatch.h>
#include <vector>
#include <iterator>
#include <string>
#include <cstdlib>


// RFC 4648


namespace {
    
    
    std::string 
    encode(const std::string& in) 
    {
        //std::cout << "encoding: " << in << std::endl;
        std::string result;
        json::utility::encodeBase64(in.begin(), in.end(), std::back_inserter(result));
        return result;
    }
    
    std::string 
    decode(const std::string& in) 
    {
        //std::cout << "decoding: " << in << std::endl;
        std::string result;
        json::utility::decodeBase64(in.begin(), in.end(), std::back_inserter(result));
        return result;
    }
    
        
    class base64_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        base64_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~base64_test() {
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
    
    
#pragma mark -
#pragma mark Unicode Code Point
    
    TEST_F(base64_test, TestLookup) {
        
        using json::utility::base64_detail::lookup;
        
        
        const std::string base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int c = 0; c <= 255; c++) {
            if (c == '=') {
                int v = lookup(c);
                EXPECT_EQ(-2, v) << "character: " << static_cast<char>(c);
            }
            else
            {
                size_t pos = base64chars.find_first_of(static_cast<char>(c));
                if (pos != std::string::npos) {
                    int v  = lookup(c);
                    EXPECT_EQ(pos, v) << "character: " << static_cast<char>(c);
                }
                else {
                    int v = lookup(c);
                    EXPECT_EQ(-1, v) << "character: " << static_cast<char>(c);
                }            
            }
        }
        
    } 
    
    
    TEST_F(base64_test, SimpleEncodeDecode) {
        
        struct test_s {
            const char* a_;
            const char* b_;
        };
        
        
        test_s tests[] = {
            "", "",
            "f", "Zg==",
            "fo", "Zm8=",
            "foo", "Zm9v",
            "foob", "Zm9vYg==",
            "fooba", "Zm9vYmE=",
            "foobar", "Zm9vYmFy"
        };  
        
        int count = sizeof(tests)/sizeof(test_s);
        
        for (int i = 0; i < count; ++i) {
            std::string a = tests[i].a_;
            std::string b = tests[i].b_;
            
            std::string encode_test = encode(a);
            std::string decode_test = decode(b);
            
            EXPECT_EQ(b, encode_test) << "encode: in: '" << a << "', out: '" << b << "'" << std::endl; 
            EXPECT_EQ(a, decode_test) << "decode: in: '" << b << "', out: '" << a << "'" << std::endl; 
        }
    }
    
    
    TEST_F(base64_test, RandomEncodeDecode) {
        
        std::vector<uint8_t> img; 
        std::vector<uint8_t> img2; 
        std::string result;
        
        srand(0);
        
#if defined (DEBUG)
        const size_t K = 100;
#else
        const size_t K = 100000;
#endif        
        
        for (int k = 0; k < K; ++k) {
            
            int size = rand() % 8*1024;
            img.clear();
            for (int i = 0; i <= size; ++i) {
                img.push_back(i);
            }
            
            result.clear();
            img2.clear();
            std::random_shuffle(img.begin(), img.end());
            json::utility::encodeBase64(img.begin(), img.end(), std::back_inserter(result));
            json::utility::decodeBase64(result.begin(), result.end(), std::back_inserter(img2));
            
            bool compare = img == img2;
            EXPECT_TRUE(compare);
            
            if (!compare) {
                break;
            }
        }
    }

}
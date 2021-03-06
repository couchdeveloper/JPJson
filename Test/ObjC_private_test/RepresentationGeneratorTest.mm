//
//  RepresentationGeneratorTest.cpp
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

#include "json/ObjC/RepresentationGenerator.hpp"
#include "json/unicode/unicode_utilities.hpp"   // encoding
#include <gtest/gtest.h>



// for testing
//#import <Foundation/Foundation.h>
#include <string.h>
#include <stdio.h>
#include <stdexcept>
#include "utilities/timer.hpp"
#include <boost/tr1/unordered_map.hpp>
#include <algorithm>





using namespace json;

using json::unicode::UTF_8_encoding_tag;
using json::semantic_actions_base;
using json::objc::RepresentationGenerator;

using utilities::timer;


namespace {
    
    
    // The fixture for testing class json::Boolean:
    
    class RepresentationGeneratorTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        RepresentationGeneratorTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~RepresentationGeneratorTest() {
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
    
    
    
    TEST_F(RepresentationGeneratorTest, DefaultCtor) 
    {
        RepresentationGenerator<UTF_8_encoding_tag> sa; 
        
        EXPECT_TRUE(sa.checkDuplicateKey());
        EXPECT_FALSE(sa.keepStringCacheOnClear());
        EXPECT_FALSE(sa.cacheDataStrings());
        EXPECT_FALSE(sa.ignoreSpuriousTrailingBytes());        
        EXPECT_FALSE(sa.parseMultipleDocuments());
        EXPECT_TRUE(sa.numberGeneratorOption() == json::objc::sa_options::NumberGeneratorGenerateAuto);
    }
    
    TEST_F(RepresentationGeneratorTest, SimpleTest) 
    {
        typedef RepresentationGenerator<UTF_8_encoding_tag> sa_t;
        typedef sa_t::const_buffer_t  const_buffer_t;
        
        @autoreleasepool {
        
        
            sa_t sa; 
                    
            sa.parse_begin();
            sa.begin_array();
            const int J = 10;
            const int N = 20;
            const int K = 10;
            for (int j = 0; j < J; ++j)
            {
                sa.begin_value_at_index(j);            
                sa.begin_object();
                
                for (int i = 0; i < N; ++i) {
                    char buffer[256];
                    std::size_t len = snprintf(buffer, sizeof(buffer), "key#%d", i);
                    sa.begin_key_value_pair(const_buffer_t(buffer, len), i);
                    sa.value_string(const_buffer_t("string", 6));
                    sa.end_key_value_pair();                              
                    //std::cout << sa.json_path() << " = " << std::endl;
                }
                
                sa.begin_key_value_pair(const_buffer_t("list", 4), N);
                sa.begin_array();
                for (int k = 0; k < K; ++k) {
                    sa.begin_value_at_index(k);
                    char buffer[246];
                    std::size_t len = snprintf(buffer, sizeof(buffer), "string#%d", k);
                    sa.value_string(const_buffer_t(buffer, len));
                    sa.end_value_at_index(k);
                    //std::cout << sa.json_path() << " = " << std::endl;
                }
                sa.end_array();
                sa.end_key_value_pair();                              
                
                
                sa.end_object();            
                sa.end_value_at_index(j);
            }
            
            sa.end_array();
            sa.parse_end();
            sa.finished();
            
            id result = sa.result();
            EXPECT_TRUE(result != nil);
            EXPECT_EQ(J, [result count]);
            
            EXPECT_EQ(N+1, sa.cache_miss_count());
            EXPECT_EQ(J*(N+1)-(N+1), sa.cache_hit_count());
            EXPECT_EQ(N+1, sa.cache_size());
        
#if defined (XX_DEBUG)
        NSLog(@"\n%@", result);
#endif        
        
        
        }
    }
    
    
    
}
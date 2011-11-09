//
//  JsonSemanticActionsTest.cpp
//
//  Created by Andreas Grosam on 5/22/11.
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

#include "json/unicode/unicode_utilities.hpp"
#include "json/parser/semantic_actions_test.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <iomanip>


namespace {
    
    using namespace json;
    using unicode::UTF_8_encoding_tag;
    using json::internal::semantic_actions_test;
    
    typedef semantic_actions_test<UTF_8_encoding_tag> semantics_actions_t;
    
    
    // The fixture for testing class JsonParser.
    class JsonSemanticActionsTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonSemanticActionsTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonSemanticActionsTest() {
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
    

    TEST_F(JsonSemanticActionsTest, Constructor)  
    {        
        // The result of a default constructed semantic action is equal the
        // default constructed result_type.
        
        semantics_actions_t::result_type r;                
        semantics_actions_t sa;
        EXPECT_TRUE( r == sa.result() );
    }


} // namespace
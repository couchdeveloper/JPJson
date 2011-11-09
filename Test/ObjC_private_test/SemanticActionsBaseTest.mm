//
//  SemanticActionsBaseTest.mm
//  Test
//
//  Created by Andreas Grosam on 10/26/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "json/ObjC/SemanticActionsBase.hpp"
#include "gtest/gtest.h"


using namespace json;

using json::objc::SemanticActionsBase;
using json::unicode::UTF_8_encoding_tag;


using json::semanticactions::extension_options;
using json::semanticactions::log_option;
using json::semanticactions::log_option;

namespace {
    
    
    // The fixture for testing class json::Boolean:
    
    class SemanticActionsBaseTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        SemanticActionsBaseTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~SemanticActionsBaseTest() {
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
    
    
    
    TEST_F(SemanticActionsBaseTest, DefaultCtor) 
    {
        typedef SemanticActionsBase<UTF_8_encoding_tag> semantic_actions_type; 
        
        semantic_actions_type sa;
        
        EXPECT_TRUE(sa.checkDuplicateKey());
        EXPECT_FALSE(sa.ignoreSpuriousTrailingBytes());        
        EXPECT_FALSE(sa.parseMultipleDocuments());
        
        EXPECT_FALSE(sa.is_canceled());
        EXPECT_EQ(0, sa.error().code());
        EXPECT_EQ(std::string(""), sa.error().description());
        
        EXPECT_TRUE(sa.ok());
        
        //noncharacter_handling_option unicode_noncharacter_handling
        
        
        // log_level
    }
    
    TEST_F(SemanticActionsBaseTest, SimpleTest) 
    {
        typedef SemanticActionsBase<UTF_8_encoding_tag> semantic_actions_type; 
        typedef semantic_actions_type::error_t error_t;

        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
                
        semantic_actions_type sa; 
        sa.error(error_t(-1, "Test error"));
        
        EXPECT_FALSE(sa.ok());
        EXPECT_EQ(-1, sa.error().code());
        EXPECT_EQ(std::string("Test error"), sa.error().description());
        
        sa.cancel();
        EXPECT_TRUE(sa.is_canceled());

        sa.clear();
        EXPECT_FALSE(sa.is_canceled());
        EXPECT_EQ(0, sa.error().code());
        EXPECT_EQ(std::string(""), sa.error().description());
        
        
        sa.cancel();
        EXPECT_TRUE(sa.is_canceled());
        EXPECT_FALSE(sa.ok());

        sa.clear();
        EXPECT_TRUE(sa.ok());
        
        
        std::cout << "description:\n" << sa << std::endl;
        
        [pool drain];
    }
    
    
    
}
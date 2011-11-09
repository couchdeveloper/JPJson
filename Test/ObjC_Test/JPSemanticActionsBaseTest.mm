//
//  JPSemanticActionsBaseTest.mm
//  Test
//
//  Created by Andreas Grosam on 10/23/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JPJson/JPSemanticActionsBase.h"
#include "gtest/gtest.h"



namespace {
    
    
/*
    - (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue;
    - (id) init;
        
    - (void) configureWithOptions:(JPJsonParserOptions)options;
 
    @property (nonatomic, readonly) id result;
    @property (nonatomic, assign) BOOL ignoreSpuriousTrailingBytes;    
    @property (nonatomic, assign) BOOL parseMultipleDocuments;    
    @property (nonatomic, assign) BOOL parseMultipleDocumentsAsynchronously;
    @property (nonatomic, assign) JPSemanticActionsLogLevel logLevel;
    @property (nonatomic, assign) JPSemanticActionsUnicodeNoncharacterHandling  unicodeNoncharacterHandling;
    @property (nonatomic, assign) JPSemanticActionsNonConformanceOptions nonConformanceOptions;
    @property (nonatomic, readonly) dispatch_queue_t handlerDispatchQueue;
 
    - (void) setHandlerDispatchQueue:(dispatch_queue_t) queue;
 
    @property (copy) JPSemanticActions_StartJsonHandlerBlockType  startJsonHandlerBlock; 
    @property (copy) JPSemanticActions_EndJsonHandlerBlockType  endJsonHandlerBlock; 
    @property (copy) JPSemanticActions_CompletionHandlerBlockType  completionHandlerBlock;
    @property (copy) JPSemanticActions_ErrorHandlerBlockType  errorHandlerBlock;
  
    @property (atomic, readonly) NSError* error;
 
 - (void) setErrorCode:(int)error description:(NSString*)description;

 */
    
    class JPSemanticActionsBaseTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JPSemanticActionsBaseTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JPSemanticActionsBaseTest() {
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
    
    
    TEST_F(JPSemanticActionsBaseTest, JPSemanticActionsDefaultProperties) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        JPSemanticActionsBase* sa = [[JPSemanticActionsBase alloc] init];        
        [sa release];
        
        [pool drain];
    }
    
    TEST_F(JPSemanticActionsBaseTest, JPSemanticActionsSetProperties) 
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        JPSemanticActionsBase* sa = [[JPSemanticActionsBase alloc] init];
        
        [sa release];
        [pool drain];
    }
    
    
}
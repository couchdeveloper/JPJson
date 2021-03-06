//
//  JPRepresentationGeneratorTest.mm
//  Test
//
//  Created by Andreas Grosam on 10/23/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "JPJson/JPRepresentationGenerator.h"
#include <gtest/gtest.h>
#import <Foundation/Foundation.h>



namespace {
    
        
    class JPRepresentationGeneratorTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JPRepresentationGeneratorTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JPRepresentationGeneratorTest() {
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
    
    
    
    
    TEST_F(JPRepresentationGeneratorTest, JPSemanticActionsDefaultProperties) 
    {
        @autoreleasepool {
            JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] init];
            EXPECT_TRUE((void*)sa.handlerDispatchQueue != NULL);
            
            sa = [[JPRepresentationGenerator alloc] initWithHandlerDispatchQueue:NULL];
            EXPECT_EQ(nil, sa.handlerDispatchQueue);
            
            EXPECT_EQ(nil, sa.result);
            
#if defined (DEBUG)
            EXPECT_EQ(int(JPSemanticActionsLogLevelDebug), int([sa logLevel]));
#else
            EXPECT_EQ(int(JPSemanticActionsLogLevelError), int([sa logLevel]));
#endif
            
            EXPECT_EQ(int(JPSemanticActionsSignalErrorOnUnicodeNULLCharacter), int([sa unicodeNULLCharacterHandling]));
            EXPECT_EQ(int(JPSemanticActionsSignalErrorOnUnicodeNoncharacter), int([sa unicodeNoncharacterHandling]));

            EXPECT_EQ(int(0), int([sa nonConformanceOptions]));
            
            EXPECT_EQ(NO, sa.ignoreSpuriousTrailingBytes);
            EXPECT_EQ(NO, sa.parseMultipleDocuments);
            EXPECT_EQ(NO, sa.parseMultipleDocumentsAsynchronously);
            
            EXPECT_TRUE((void*)(sa.handlerDispatchQueue) == NULL);
            EXPECT_EQ(NULL, (void*)(sa.startJsonHandlerBlock));
            EXPECT_EQ(NULL, (void*)(sa.endJsonHandlerBlock));
            EXPECT_EQ(NULL, (void*)(sa.completionHandlerBlock));
            EXPECT_EQ(NULL, (void*)(sa.errorHandlerBlock));
            
            
            EXPECT_EQ(YES, sa.checkForDuplicateKey);
            EXPECT_EQ(NO, sa.cacheDataStrings);
            EXPECT_EQ(NO, sa.keepStringCacheOnClear);
            EXPECT_EQ(NO, sa.parseMultipleDocuments);
            EXPECT_EQ(int(JPSemanticActionsNumberGeneratorGenerateAuto), int([sa numberGeneratorOption]));
            
            EXPECT_EQ(NO, sa.generateEncodedStrings);
            
            EXPECT_EQ(nil, sa.error);
        }
    }
    
    TEST_F(JPRepresentationGeneratorTest, JPSemanticActionsSetProperties) 
    {
        @autoreleasepool {
            JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] init];
            
            EXPECT_TRUE((void*)(sa.handlerDispatchQueue) != NULL);
            
            sa.handlerDispatchQueue = NULL;
            EXPECT_TRUE((void*)(sa.handlerDispatchQueue) == NULL);
            
            sa.parseMultipleDocuments = YES;
            EXPECT_EQ(YES, sa.parseMultipleDocuments);
            sa.parseMultipleDocuments = NO;
            EXPECT_EQ(NO, sa.parseMultipleDocuments);
            
            sa.ignoreSpuriousTrailingBytes = YES;
            EXPECT_EQ(YES, sa.ignoreSpuriousTrailingBytes);
            sa.ignoreSpuriousTrailingBytes = NO;
            EXPECT_EQ(NO, sa.ignoreSpuriousTrailingBytes);
            
            sa.parseMultipleDocumentsAsynchronously = YES;
            EXPECT_EQ(YES, sa.parseMultipleDocumentsAsynchronously);
            sa.parseMultipleDocumentsAsynchronously = NO;
            EXPECT_EQ(NO, sa.parseMultipleDocumentsAsynchronously);
            

#if !defined (DEBUG)
            // In Release mode, the max severity is set to warning
            sa.logLevel = JPSemanticActionsLogLevelDebug;
            EXPECT_EQ((int)JPSemanticActionsLogLevelWarning, sa.logLevel);
#else
            sa.logLevel = JPSemanticActionsLogLevelDebug;
            EXPECT_EQ((int)JPSemanticActionsLogLevelDebug, sa.logLevel);
#endif
            sa.logLevel = JPSemanticActionsLogLevelWarning;
            EXPECT_EQ((int)JPSemanticActionsLogLevelWarning, sa.logLevel);
            sa.logLevel = JPSemanticActionsLogLevelError;
            EXPECT_EQ((int)JPSemanticActionsLogLevelError, sa.logLevel);
            sa.logLevel = JPSemanticActionsLogLevelNone;
            EXPECT_EQ((int)JPSemanticActionsLogLevelNone, sa.logLevel);
            
#if !defined (DEBUG)
            // In Release mode, the max severity is set to warning
            sa.logLevel = JPSemanticActionsLogLevelDebug;
            EXPECT_EQ((int)JPSemanticActionsLogLevelWarning, sa.logLevel);
#else
            sa.logLevel = JPSemanticActionsLogLevelDebug;
            EXPECT_EQ((int)JPSemanticActionsLogLevelDebug, sa.logLevel);
#endif
            
            sa.unicodeNoncharacterHandling = JPSemanticActionsSignalErrorOnUnicodeNoncharacter;
            EXPECT_EQ((int)JPSemanticActionsSignalErrorOnUnicodeNoncharacter, sa.unicodeNoncharacterHandling);
            sa.unicodeNoncharacterHandling = JPSemanticActionsSubstituteUnicodeNoncharacter;
            EXPECT_EQ((int)JPSemanticActionsSubstituteUnicodeNoncharacter, sa.unicodeNoncharacterHandling);
            sa.unicodeNoncharacterHandling = JPSemanticActionsRemoveUnicodeNoncharacter;
            EXPECT_EQ((int)JPSemanticActionsRemoveUnicodeNoncharacter, sa.unicodeNoncharacterHandling);
            sa.unicodeNoncharacterHandling = JPSemanticActionsSignalErrorOnUnicodeNoncharacter;
            EXPECT_EQ((int)JPSemanticActionsSignalErrorOnUnicodeNoncharacter, sa.unicodeNoncharacterHandling);

            sa.unicodeNULLCharacterHandling = JPSemanticActionsSignalErrorOnUnicodeNULLCharacter;
            EXPECT_EQ((int)JPSemanticActionsSignalErrorOnUnicodeNoncharacter, sa.unicodeNULLCharacterHandling);
            sa.unicodeNULLCharacterHandling = JPSemanticActionsSubstituteUnicodeNULLCharacter;
            EXPECT_EQ((int)JPSemanticActionsSubstituteUnicodeNULLCharacter, sa.unicodeNULLCharacterHandling);
            sa.unicodeNULLCharacterHandling = JPSemanticActionsRemoveUnicodeNULLCharacter;
            EXPECT_EQ((int)JPSemanticActionsRemoveUnicodeNULLCharacter, sa.unicodeNULLCharacterHandling);
            sa.unicodeNULLCharacterHandling = JPSemanticActionsSignalErrorOnUnicodeNULLCharacter;
            EXPECT_EQ((int)JPSemanticActionsSignalErrorOnUnicodeNULLCharacter, sa.unicodeNULLCharacterHandling);
            
            sa.nonConformanceOptions = JPSemanticActionsAllowComments;
            EXPECT_EQ((int)JPSemanticActionsAllowComments, sa.nonConformanceOptions);
            sa.nonConformanceOptions = JPSemanticActionsAllowNone;
            EXPECT_EQ((int)JPSemanticActionsAllowNone, sa.nonConformanceOptions);
            sa.nonConformanceOptions = JPSemanticActionsAllowComments | JPSemanticActionsAllowControlCharacters;
            EXPECT_EQ((int)JPSemanticActionsAllowComments | JPSemanticActionsAllowControlCharacters, sa.nonConformanceOptions);
            sa.nonConformanceOptions = JPSemanticActionsAllowNone;
            EXPECT_EQ((int)JPSemanticActionsAllowNone, sa.nonConformanceOptions);
            sa.nonConformanceOptions = JPSemanticActionsAllowLeadingPlusInNumbers | JPSemanticActionsAllowLeadingZerosInIntegers;
            EXPECT_EQ((int)JPSemanticActionsAllowLeadingPlusInNumbers | JPSemanticActionsAllowLeadingZerosInIntegers, sa.nonConformanceOptions);
            sa.nonConformanceOptions = JPSemanticActionsAllowNone;
            EXPECT_EQ((int)JPSemanticActionsAllowNone, sa.nonConformanceOptions);
            sa.nonConformanceOptions = JPSemanticActionsAllowComments | JPSemanticActionsAllowControlCharacters | JPSemanticActionsAllowLeadingPlusInNumbers | JPSemanticActionsAllowLeadingZerosInIntegers;
            EXPECT_EQ((int)JPSemanticActionsAllowComments | JPSemanticActionsAllowControlCharacters | JPSemanticActionsAllowLeadingPlusInNumbers | JPSemanticActionsAllowLeadingZerosInIntegers, sa.nonConformanceOptions);
            sa.nonConformanceOptions = JPSemanticActionsAllowNone;
            EXPECT_EQ((int)JPSemanticActionsAllowNone, sa.nonConformanceOptions);
            
            
            
            sa.checkForDuplicateKey = YES;
            EXPECT_EQ(YES, sa.checkForDuplicateKey);
            sa.checkForDuplicateKey = NO;
            EXPECT_EQ(NO, sa.checkForDuplicateKey);
            
            sa.checkForDuplicateKey = YES;
            EXPECT_EQ(YES, sa.checkForDuplicateKey);
            sa.checkForDuplicateKey = NO;
            EXPECT_EQ(NO, sa.checkForDuplicateKey);
            
            sa.cacheDataStrings =YES;
            EXPECT_EQ(YES, sa.cacheDataStrings);
            sa.cacheDataStrings =NO;
            EXPECT_EQ(NO, sa.cacheDataStrings);
            
            sa.keepStringCacheOnClear = YES;
            EXPECT_EQ(YES, sa.keepStringCacheOnClear);
            sa.keepStringCacheOnClear = NO;
            EXPECT_EQ(NO, sa.keepStringCacheOnClear);
            
            
            sa.numberGeneratorOption = JPSemanticActionsNumberGeneratorGenerateNSString;
            EXPECT_EQ((int)JPSemanticActionsNumberGeneratorGenerateNSString, sa.numberGeneratorOption);
            sa.numberGeneratorOption = JPSemanticActionsNumberGeneratorGenerateNSDecimalNumber;
            EXPECT_EQ((int)JPSemanticActionsNumberGeneratorGenerateNSDecimalNumber, sa.numberGeneratorOption);
            sa.numberGeneratorOption = JPSemanticActionsNumberGeneratorGenerateAuto;
            EXPECT_EQ((int)JPSemanticActionsNumberGeneratorGenerateAuto, sa.numberGeneratorOption);            
        }
    }
    
    
    TEST_F(JPRepresentationGeneratorTest, JPSemanticActionsError) 
    {
        @autoreleasepool {
            JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] init];
            
            // If clients detect errors, they may set the error state of the
            // semantic actions instance using method setErrorCode:description:
            // The allowed range of values is as follows:
            //   code <= 0 and code >= json::JP_PARSER_CLIENT
            // Other values are reserved for use by the json parser.
            // A few constants are defined for this purpose:
            //        json::JP_PARSER_CLIENT
            //        json::JP_UNEXPECTED_ERROR
            //        json::JP_UNKNOWN_ERROR,
            //        json::JP_ERROR_MAX
            
            [sa setErrorCode:-1 description:@"test error"];
            NSError* error = sa.error;
            EXPECT_TRUE(error != nil);
            EXPECT_EQ(-1, [error code]);
            EXPECT_EQ(YES, [@"test error" isEqualToString:[error localizedDescription]]);
            
            [sa clear];
            EXPECT_EQ(nil, sa.error);
        }
    }
    
}
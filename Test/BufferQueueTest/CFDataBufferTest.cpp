//
//  CFDataBufferTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 6/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


#include "json/ObjC/CFDataBuffers.hpp"
#include "gtest/gtest.h"

// for testing
#include <dispatch/semaphore.h>
#include <string.h>





using namespace json;

using json::objc::CFDataConstBuffer;



namespace {
    
    
    // The fixture for testing class json::Boolean:
    
    class CFDataBufferTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        CFDataBufferTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~CFDataBufferTest() {
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
    
    TEST_F(CFDataBufferTest, DefaultCtor) 
    {
        CFDataConstBuffer<char> buffer;
        
        EXPECT_EQ( 0, buffer.size() );
        EXPECT_EQ( 0, buffer.ref_count() );
        EXPECT_EQ( 0, buffer.data() );
    }
    
    
    TEST_F(CFDataBufferTest, Ctor1) 
    {
        UInt8 emptyData[] = {0};
        UInt8 someData5[] = {1, 2, 3, 4, 5};
        
        CFDataRef data0 = CFDataCreate(kCFAllocatorDefault, emptyData, 0);
        CFDataConstBuffer<char> buffer0 = data0;        
        CFRelease(data0);
        EXPECT_EQ( 0, buffer0.size() );
        EXPECT_EQ( 0, buffer0.ref_count() );
        EXPECT_EQ( 0, buffer0.data() );

        CFDataRef data1 = CFDataCreate(kCFAllocatorDefault, someData5, 0);
        CFDataConstBuffer<char> buffer1 = data1; 
        CFRelease(data1);
        EXPECT_EQ( 0, buffer1.size() );
        EXPECT_EQ( 0, buffer1.ref_count() );
        EXPECT_EQ( 0, buffer1.data() );
    
        CFDataConstBuffer<char> buffer2 = NULL;        
        EXPECT_EQ( 0, buffer2.size() );
        EXPECT_EQ( 0, buffer2.ref_count() );
        EXPECT_EQ( 0, buffer2.data() );
        
        CFDataRef data3 = CFDataCreate(kCFAllocatorDefault, someData5, 3);
        CFDataConstBuffer<char> buffer3 = data3; 
        CFRelease(data3);
        EXPECT_EQ( 3, buffer3.size() );
        EXPECT_EQ( 1, buffer3.ref_count() );
        EXPECT_TRUE( buffer3.data() != 0);
        EXPECT_TRUE( memcmp(buffer3.data(), someData5, 3) == 0);
    }

    
    TEST_F(CFDataBufferTest, Ctor2) 
    {
        CFDataConstBuffer<char> buffer0 = CFDataConstBuffer<char>( 0, 0);        
        EXPECT_EQ( 0, buffer0.size() );
        EXPECT_EQ( 0, buffer0.ref_count() );
        EXPECT_EQ( 0, buffer0.data() );

        const char* d = "Abc";        
        CFDataConstBuffer<char> buffer1 = CFDataConstBuffer<char>(d, 0);        
        EXPECT_EQ( 0, buffer1.size() );
        EXPECT_EQ( 0, buffer1.ref_count() );
        EXPECT_EQ( 0, buffer1.data() );

        CFDataConstBuffer<char> buffer2 = CFDataConstBuffer<char>(0, 1);        
        EXPECT_EQ( 0, buffer2.size() );
        EXPECT_EQ( 0, buffer2.ref_count() );
        EXPECT_EQ( 0, buffer2.data() );

        CFDataConstBuffer<char> buffer3 = CFDataConstBuffer<char>(d, 1);        
        EXPECT_EQ( 1, buffer3.size() );
        EXPECT_EQ( 1, buffer3.ref_count() );
        EXPECT_TRUE( buffer3.data() != 0);
        EXPECT_TRUE( memcmp(buffer3.data(), d, 1) == 0);
    }

    
    TEST_F(CFDataBufferTest, CopyCtor) 
    {
        CFDataConstBuffer<char> b0;
        CFDataConstBuffer<char> b1("Abc", 3);
        
        CFDataConstBuffer<char> b10 = b0;
        EXPECT_EQ( 0, b10.size() );
        EXPECT_EQ( 0, b10.ref_count() );
        EXPECT_EQ( 0, b10.data() );
        
        CFDataConstBuffer<char> b11 = b1;
        EXPECT_EQ( b1.size(), b11.size() );
        EXPECT_EQ( b1.ref_count(), b11.ref_count() );
        EXPECT_EQ( b1.data(), b11.data() );
        
        CFDataConstBuffer<char> b12 = b1;
        EXPECT_EQ( b1.size(), b12.size() );
        EXPECT_EQ( b1.ref_count(), b12.ref_count() );
        EXPECT_EQ( b1.data(), b12.data() );
    }

    
    TEST_F(CFDataBufferTest, Release) 
    {
        CFDataConstBuffer<char> b0("Abc", 3);
        CFDataConstBuffer<char> b = b0;
        size_t ref_count = b0.ref_count();

        b.release();
        EXPECT_EQ( ref_count-1,  b0.ref_count() );
        EXPECT_EQ( 0, b.size() );
        EXPECT_EQ( 0, b.ref_count() );
        EXPECT_EQ( 0, b.data() );
    }


    TEST_F(CFDataBufferTest, Assignment) 
    {
        CFDataConstBuffer<char> b0("Abc", 3);
        CFDataConstBuffer<char> b;
        
        b = b0;
        EXPECT_EQ( b0.size(), b.size() );
        EXPECT_EQ( b0.data(), b.data() );
    } 
    
    
    TEST_F(CFDataBufferTest, Swap)
    {
        CFDataConstBuffer<char> b0("Abc", 3);
        CFDataConstBuffer<char> b1;
        
        swap(b0, b1);
        
        EXPECT_EQ( 0, b0.size() );
        EXPECT_EQ( 0, b0.data() );
        EXPECT_EQ( 3, b1.size() );
        EXPECT_TRUE( b1.data() != 0 );
    }
    
    
}  //namespace
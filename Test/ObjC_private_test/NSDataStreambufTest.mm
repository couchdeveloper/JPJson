//
//  NSDataStreambufTest.mm
//  Test
//
//  Created by Andreas Grosam on 5/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <gtest/gtest.h>
#include "json/ObjC/NSDataStreambuf.hpp"

#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>
#include <iterator>

// for testing


namespace {
    
    using json::objc::NSDataStreambuf;
    
    
    class NSDataStreambufTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        NSDataStreambufTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~NSDataStreambufTest() {
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
    
    
    //    Public member functions
    //
    //    The common functionality for all stream buffers is provided through the following public member functions:
    //
    //        Locales:
    //        pubimbue        Imbue locale (public member function)
    //        getloc          Get current locale (public member function)
    //
    //        Buffer management and positioning:
    //        pubsetbuf       Set buffer array (public member function)
    //        pubseekoff      Set internal position pointer to relative position (public member function)
    //        pubseekpos      Set internal position pointer to absolute position (public member function )
    //        pubsync         Synchronize stream buffer (public member function )
    //
    //        Input functions (get):
    //        in_avail        Get number of characters available to read (public member function)
    //        snextc          Increase get pointer and return next character (public member function)
    //        sbumpc          Get current character and increase get pointer (public member function)
    //        sgetc           Get current character (public member function)
    //        sgetn           Get sequence of characters (public member function )
    //        sputbackc       Put character back (public member function)
    //        sungetc         Decrease get pointer (public member function)
    //
    //        Output functions (put):
    //        sputc           Store character at current put position and increase put pointer (public member function)
    //        sputn           Write a sequence of characters (public member function)
    //
    //
    //        void data(NSData* __data);
    //        NSData* data() const;
    //        NSMutableData* buffer() const;
    
    
    
    TEST_F(NSDataStreambufTest, Constructor1)
    {
        typedef NSDataStreambuf<char>   stream_buffer_t;
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        stream_buffer_t ios;
        EXPECT_EQ(std::streamsize(0), ios.in_avail());
        EXPECT_TRUE(ios.data() != nil);
        EXPECT_EQ(0, [ios.data() length]);
        
        [pool drain];
    }
    
    TEST_F(NSDataStreambufTest, Constructor2)
    {
        typedef NSDataStreambuf<char>   stream_buffer_t;
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSData* data = [NSData dataWithBytes:"0123456789" length:10];
        
        stream_buffer_t ios(data);
        EXPECT_EQ(std::streamsize(10), ios.in_avail());
        EXPECT_TRUE(ios.data() != nil);
        EXPECT_EQ(10, [ios.data() length]);
        
        [pool drain];
    }
    
    TEST_F(NSDataStreambufTest, Constructor3)
    {
        using namespace std;
        
        typedef NSDataStreambuf<char>   stream_buffer_t;
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSData* data = [NSData dataWithBytes:"0123456789" length:10];
        
        stream_buffer_t ios(data, ios::app|ios::ate|ios::out);
        EXPECT_EQ(0, ios.in_avail());
        EXPECT_TRUE(ios.data() != nil);
        EXPECT_EQ(10, [ios.data() length]);
        
        [pool drain];
    }
    
    
    
    
    
    TEST_F(NSDataStreambufTest, Write)
    {
        typedef NSDataStreambuf<char>   stream_buffer_t;
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        stream_buffer_t ios;
        
        EXPECT_EQ(std::streamsize(0), ios.in_avail());
        EXPECT_TRUE(ios.data() != nil);
        EXPECT_EQ(0, [ios.data() length]);
        
        int const N = 10000;
        int const L = 101;
        for (int i = 0; i < N; ++i) {
            ios.sputn("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n", L);
        }

        NSData* data = ios.data();
        EXPECT_EQ(N*L, [data length]);

#if defined (DEBUG_XX)
        NSString* str = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:NSUTF8StringEncoding];
        NSLog(@"%@", str);
#endif
        
        [pool drain];
    }
    
}
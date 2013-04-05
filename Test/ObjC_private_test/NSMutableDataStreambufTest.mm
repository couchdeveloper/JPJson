//
//  NSMutableDataStreambufTest.cpp
//  Test
//
//  Created by Andreas Grosam on 5/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <gtest/gtest.h>
#include "json/ObjC/NSMutableDataStreambuf.hpp"

#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>
#include <iterator>

// for testing


namespace {
    
    using json::objc::NSMutableDataStreambuf;
    
    
    class NSMutableDataStreambufTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        NSMutableDataStreambufTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~NSMutableDataStreambufTest() {
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
    
    
    
    TEST_F(NSMutableDataStreambufTest, Constructor1) 
    {
        typedef NSMutableDataStreambuf<char>   stream_buffer_t;
        
        @autoreleasepool {
        
            stream_buffer_t ios;
            EXPECT_EQ(std::streamsize(0), ios.in_avail());
            EXPECT_TRUE(ios.data() != nil);
            EXPECT_EQ(0, [ios.data() length]);
        
        }
    }    
    
    TEST_F(NSMutableDataStreambufTest, Constructor2) 
    {
        typedef NSMutableDataStreambuf<char>   stream_buffer_t;
        
        @autoreleasepool {
        
            NSData* data = [NSData dataWithBytes:"0123456789" length:10];
            
            stream_buffer_t ios(data);
            EXPECT_EQ(std::streamsize(10), ios.in_avail());
            EXPECT_TRUE(ios.data() != nil);
            EXPECT_EQ(10, [ios.data() length]);
        
        }
    }    
    
    TEST_F(NSMutableDataStreambufTest, Constructor3) 
    {
        using namespace std;
        
        typedef NSMutableDataStreambuf<char>   stream_buffer_t;
        
        @autoreleasepool {
        
            NSData* data = [NSData dataWithBytes:"0123456789" length:10];
            
            stream_buffer_t ios(data, ios::app|ios::ate|ios::out);
            EXPECT_EQ(std::streamsize(10), ios.in_avail());
            EXPECT_TRUE(ios.data() != nil);
            EXPECT_EQ(10, [ios.data() length]);
        
        }
    }    
    
    
    
    

    TEST_F(NSMutableDataStreambufTest, Write) 
    {
        typedef NSMutableDataStreambuf<char>   stream_buffer_t;
        
        @autoreleasepool {
        
            stream_buffer_t ios;
            
            EXPECT_EQ(std::streamsize(0), ios.in_avail());
            EXPECT_TRUE(ios.data() != nil);
            EXPECT_EQ(0, [ios.data() length]);
        
        //        for (int i = 0; i < 100; ++i) {
        //            ios.sputn("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n", 101);
        //        }
        //        
        //        NSData* data = ios.data();
        //        
        //        NSString* str = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:NSUTF8StringEncoding];
        //        
        //        NSLog(@"%@", str);
        
        
        /*        
         const char* s = "0123456789";
         CFDataBuffer buffer = CFDataBuffer(s, strlen(s)); 
         queue_t queue;
         
         queue.put(buffer, 0);
         EXPECT_EQ(false, queue.empty());
         
         stream_buffer_t sbuffer(queue);
         
         for (int i = 0; i < 10; ++i) {
         char ch = sbuffer.sbumpc();
         EXPECT_EQ(s[i], ch);
         }
         */      
        
        }
    }    
    
    
#if 0    
    
    TEST_F(NSMutableDataStreambufTest, NSMutableDataStreambufWithIstreamIterator) 
    {
        typedef json::objc::CFDataBuffer<char>                  CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>                 queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, char>   stream_buffer_t;
        
        typedef std::pair<queue_t::result_type, queue_t::value_type> result_t;
        
        const char* s = "0123456789";
        CFDataBuffer buffer = CFDataBuffer(s, strlen(s)); 
        queue_t queue;
        queue.put(buffer, 0);
        EXPECT_EQ(false, queue.empty());
        
        stream_buffer_t sbuffer(queue);
        
        std::istreambuf_iterator<char> eos;
        std::istreambuf_iterator<char> iit (&sbuffer); // stdin iterator
        
        
        for (int i = 0; i < 10; ++i) {
            char ch = *iit;
            ++iit;
            EXPECT_EQ(s[i], ch);
        }
        
    }    
    
    TEST_F(NSMutableDataStreambufTest, NSMutableDataStreambufWithIstreamIterator2) 
    {
        typedef json::objc::CFDataBuffer<char>                      CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>                     queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, wchar_t>    stream_buffer_t;
        
        typedef std::pair<queue_t::result_type, queue_t::value_type> result_t;
        
        const wchar_t s[] = L"0123456789";
        char const* data_ptr = reinterpret_cast<char const*>(s);
        
        CFDataBuffer buffer = CFDataBuffer(reinterpret_cast<char const*>(data_ptr), sizeof(s)); 
        queue_t queue;
        queue.put(buffer, 0);
        EXPECT_EQ(false, queue.empty());
        
        stream_buffer_t sbuffer(queue);
        
        std::istreambuf_iterator<wchar_t> eos;
        std::istreambuf_iterator<wchar_t> iit (&sbuffer); // stdin iterator
        
        
        for (int i = 0; i < 10; ++i) {
            wchar_t ch = *iit;
            ++iit;
            EXPECT_EQ(s[i], ch);
        }
        
    }    
    
    
    TEST_F(NSMutableDataStreambufTest, missaligned_buffer) 
    {
        typedef json::objc::CFDataBuffer<char>                      CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>                     queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, wchar_t>    stream_buffer_t;
        
        typedef std::pair<queue_t::result_type, queue_t::value_type> result_t;
        
        const wchar_t s[] = L"0123456789";
        char const* data_ptr = reinterpret_cast<char const*>(s);
        
        CFDataBuffer buffer = CFDataBuffer(data_ptr, sizeof(s)); 
        
        // Increment the data pointer to make it missaligned with a wchar_t:
        buffer.seek(1);        
        
        queue_t queue;
        queue.put(buffer, 0);
        EXPECT_EQ(false, queue.empty());
        
        stream_buffer_t sbuffer(queue);
        
        std::istreambuf_iterator<wchar_t> eos;
        std::istreambuf_iterator<wchar_t> iit (&sbuffer); // stdin iterator
        
        wchar_t  ch;
        EXPECT_ANY_THROW(ch = *iit);
    }    
    
    
    
    
    TEST_F(NSMutableDataStreambufTest, Concurrent) 
    {        
        typedef json::objc::CFDataBuffer<char>                  CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>                 queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, char>   stream_buffer_t;
        
        typedef std::pair<queue_t::result_type, queue_t::value_type> get_result_t;
        typedef queue_t::result_type                                 put_result_t;
        
        
        double timeout = -1;
        
        
        // The shared queue
        queue_t queue;
        queue_t* queue_ptr = &queue;
        
        // stream buffer
        stream_buffer_t sbuffer(queue, timeout);
        stream_buffer_t* sbuffer_ptr = &sbuffer;
        
        
        // Run one consumer and one producer concurrently:
        dispatch_group_t group = dispatch_group_create();
        dispatch_queue_t gc_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        
        
        // Producer:
        const size_t N = 10000;
        __block bool producer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            put_result_t result;
            const char* s = 
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
            "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
            const size_t len = strlen(s);
            for (int i = 0; i <= N; ++i) {
                if (i != N) {
                    CFDataBuffer buffer = CFDataBuffer(s, len); 
                    result = queue_ptr->put(buffer, timeout);
                }
                else {
                    result = queue_ptr->put(NULL, timeout);
                }
                if (result != queue_t::OK) {
                    producer_timeout_occured = true;
                    printf("producer timed out at count = %d\n", i);
                    return;
                }
            }
            
        });
        
        // Consumer
        __block bool consumer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            std::istreambuf_iterator<char> eos;
            std::istreambuf_iterator<char> iit (sbuffer_ptr); // stdin iterator
            int x = 0;
            while (iit != eos) {
                x += *iit;
                ++iit;
            }
            size_t pos = sbuffer_ptr->pubseekoff(0, std::ios_base::cur);
            printf("%d - consumer stopped at count = %lu, timed out: %s\n", 
                   x, pos, (sbuffer_ptr->timeout_occured()?"Yes":"No"));
        });
        
        
        bool completed = (0 == dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 20*NSEC_PER_SEC)));
        dispatch_release(group);
        
        EXPECT_EQ(false, producer_timeout_occured);
        EXPECT_EQ(false, consumer_timeout_occured);
        EXPECT_EQ(true, completed);
    } 
    
#endif    
    
}
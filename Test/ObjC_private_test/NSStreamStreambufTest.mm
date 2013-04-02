//
//  NSStreamStreambufTest.mm
//  Test
//
//  Created by Andreas Grosam on 06.10.12.
//
//

#include <gtest/gtest.h>
#include "json/ObjC/NSStreamStreambuf.hpp"
#include <boost/ref.hpp>
#include <boost/iostreams/device/file.hpp>

#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>
#include <iterator>

// for testing


namespace {
    
    using json::objc::NSInputStreamStreambuf;
    using json::objc::NSOutputStreamStreambuf;
    using json::objc::NSInputStreamStreambuf2;
    using json::objc::NSOutputStreamStreambuf2;
    namespace internal = json::objc::internal;
    
    class NSStreamStreambufTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        NSStreamStreambufTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~NSStreamStreambufTest() {
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
    
    
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambufDefCtor)
    {
        typedef NSInputStreamStreambuf   stream_buffer_t;
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        stream_buffer_t sbuf;
        EXPECT_EQ(std::streamsize(0), sbuf.in_avail());        
        
        [pool drain];
    }
    
    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambufDefCtor)
    {
        typedef NSOutputStreamStreambuf   stream_buffer_t;
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        stream_buffer_t sbuf;
        EXPECT_EQ(std::streamsize(0), sbuf.in_avail());
        
        [pool drain];
    }
    
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf_open)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        const NSUInteger Size = 32000;
        NSMutableData* data = [NSMutableData dataWithLength:Size];
        char* start = static_cast<char*>([data mutableBytes]);
        char* p = start;
        for (int i = 0; i < Size; ++i, ++p) {
            *p = (i % 10) + '0';
        }
        NSInputStream* nsistream = [NSInputStream inputStreamWithData:data];
        [nsistream open];
        internal::NSInputStreamSource iss(nsistream);
        
        NSInputStreamStreambuf issbuf;
        const int ReadBufferSize = 1024;
        const int PushbackBufferSize = 32;
        issbuf.open(boost::ref(iss), ReadBufferSize, PushbackBufferSize);
        
        EXPECT_TRUE(issbuf.is_open());
        
        std::streamsize inavail = issbuf.in_avail();
        //EXPECT_EQ(std::streamsize(ReadBufferSize), inavail);
        
        // peek character at current get pointer
        int ch = issbuf.sgetc();
        EXPECT_EQ(int('0'), ch);
        
        inavail = issbuf.in_avail();
        EXPECT_EQ(std::streamsize(ReadBufferSize), inavail);
        
        char buffer[10];
        std::streamsize n = issbuf.sgetn(buffer, sizeof(buffer));
        EXPECT_EQ(n, sizeof(buffer));
        
        issbuf.close();
        ASSERT_TRUE([nsistream streamStatus] == NSStreamStatusClosed);        
        
        [pool drain];
    }

    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf2_open)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        const NSUInteger Size = 32000;
        NSMutableData* data = [NSMutableData dataWithLength:Size];
        char* start = static_cast<char*>([data mutableBytes]);
        char* p = start;
        for (int i = 0; i < Size; ++i, ++p) {
            *p = (i % 10) + '0';
        }
        NSInputStream* nsistream = [NSInputStream inputStreamWithData:data];
        [nsistream open];
        ASSERT_TRUE([nsistream streamStatus] == NSStreamStatusOpen);
        
        NSInputStreamStreambuf2 issbuf;
        const int ReadBufferSize = 1024;
        const int PushbackBufferSize = 32;
        //issbuf.open(iss, ReadBufferSize, PushbackBufferSize);
        issbuf.open(internal::NSInputStreamSource2(nsistream), ReadBufferSize, PushbackBufferSize);
        
        EXPECT_TRUE(issbuf.is_open());
        
        std::streamsize inavail = issbuf.in_avail();
        //EXPECT_EQ(std::streamsize(ReadBufferSize), inavail);
        
        // peek character at current get pointer
        int ch = issbuf.sgetc();
        EXPECT_EQ(int('0'), ch);
        
        inavail = issbuf.in_avail();
        EXPECT_EQ(std::streamsize(ReadBufferSize), inavail);
        
        char buffer[10];
        std::streamsize n = issbuf.sgetn(buffer, sizeof(buffer));
        EXPECT_EQ(n, sizeof(buffer));
        
        issbuf.close();
        ASSERT_TRUE([nsistream streamStatus] == NSStreamStatusClosed);
        
        [pool drain];
    }
    
    
    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf_open)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSOutputStream* nsostream = [NSOutputStream outputStreamToMemory];
        [nsostream open];
        internal::NSOutputStreamSink oss(nsostream);
        
        NSOutputStreamStreambuf ossbuf;
        ossbuf.open(boost::ref(oss), 1024);
        
        EXPECT_TRUE(ossbuf.is_open());
        
        ossbuf.close();
        ASSERT_TRUE([nsostream streamStatus] == NSStreamStatusClosed);
        
        [pool drain];
    }
    
    
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf2_read1)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        const NSUInteger Size = 32000;
        NSMutableData* data = [NSMutableData dataWithLength:Size];
        char* start = static_cast<char*>([data mutableBytes]);
        char* p = start;
        for (int i = 0; i < Size; ++i, ++p) {
            *p = (i % 10) + '0';
        }
        NSInputStream* nsistream = [NSInputStream inputStreamWithData:data];
        [nsistream open];
        ASSERT_TRUE([nsistream streamStatus] == NSStreamStatusOpen);
        
        const int ReadBufferSize = 1024;
        //char buffer[ReadBufferSize];
        NSInputStreamStreambuf2 issbuf;
        issbuf.open(internal::NSInputStreamSource2(nsistream), ReadBufferSize);
        ASSERT_TRUE(issbuf.is_open());
        
        std::streamsize count = 0;
        int ch;
        while ((ch = issbuf.sbumpc()) != EOF) {
            EXPECT_EQ( (count % 10) + '0', ch);
            ++count;
        }
        EXPECT_EQ(Size, count);
        
        
        issbuf.close();
        ASSERT_TRUE([nsistream streamStatus] == NSStreamStatusClosed);
        
        [pool drain];
    }
    
#if 0
    
    TEST_F(NSStreamStreambufTest, Constructor2)
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
    
    TEST_F(NSStreamStreambufTest, Constructor3)
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
    
    
    
    
    
    TEST_F(NSStreamStreambufTest, Write)
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


#endif
}

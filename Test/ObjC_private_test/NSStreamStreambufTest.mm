//
//  NSStreamStreambufTest.mm
//  Test
//
//  Created by Andreas Grosam on 06.10.12.
//
//

#if !__has_feature(objc_arc)
#error This Objective-C file shall be compiled with ARC enabled.
#endif

#include <gtest/gtest.h>
#include "json/ObjC/NSStreamStreambuf.hpp"
#include <boost/ref.hpp>
#include <boost/iostreams/device/file.hpp>

#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>
#include <iterator>
#include <type_traits>
#include <stdexcept>

// for testing


namespace test {

    template <class SingleSequenceDevice>
    auto
    __has_close_test(SingleSequenceDevice&& d)
    -> decltype(d.close(), std::true_type());
    
    template <class SingleSequenceDevice>
    auto
    __has_close_test(SingleSequenceDevice const& d)
    -> std::false_type;
    
    template <class SingleSequenceDevice>
    struct __has_close
    : std::integral_constant<bool,
        std::is_same<
        decltype(__has_close_test(std::declval<SingleSequenceDevice>())),
            std::true_type
        >::value
    >
    {
    };
    
    
}


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
    
    
    template <typename T>
    using ModeOf = typename boost::iostreams::mode_of<T>::type;

    template <typename T>
    using CategoryOf = typename boost::iostreams::category_of<T>::type;
    
    TEST_F(NSStreamStreambufTest, NSInputStreamSource)
    {
        using json::objc::internal::NSInputStreamSource;
        
        ASSERT_TRUE(std::is_default_constructible<NSInputStreamSource>::value == false);  // default ctor is not meaningful
        ASSERT_TRUE(std::is_copy_constructible<NSInputStreamSource>::value == false);  // not copyable
        ASSERT_TRUE(std::is_move_constructible<NSInputStreamSource>::value == true);  // move constructible
        ASSERT_TRUE(std::is_copy_assignable<NSInputStreamSource>::value == false);  // not copy assignable
        ASSERT_TRUE(std::is_move_assignable<NSInputStreamSource>::value == true);  // move assignable
        ASSERT_TRUE(test::__has_close<NSInputStreamSource>::value == true);  // has close() member function
        
        static_assert( std::is_base_of<boost::iostreams::device_tag, CategoryOf<NSInputStreamSource>>::value, "");
        static_assert( std::is_base_of<boost::iostreams::closable_tag, CategoryOf<NSInputStreamSource>>::value, "");
        static_assert( std::is_base_of<boost::iostreams::input, ModeOf<NSInputStreamSource>>::value, "");
    }
    
    TEST_F(NSStreamStreambufTest, NSInputStreamSource2)
    {
        using json::objc::internal::NSInputStreamSource2;
        using boost::iostreams::category_of;
        
        ASSERT_TRUE(std::is_default_constructible<NSInputStreamSource2>::value == false);  // default ctor is not meaningful
        ASSERT_TRUE(std::is_copy_constructible<NSInputStreamSource2>::value == true);  // copyable
        ASSERT_TRUE(std::is_move_constructible<NSInputStreamSource2>::value == true);  // move constructible
        ASSERT_TRUE(std::is_copy_assignable<NSInputStreamSource2>::value == true);  // copy assignable
        ASSERT_TRUE(std::is_move_assignable<NSInputStreamSource2>::value == true);  // move assignable
        ASSERT_TRUE(test::__has_close<NSInputStreamSource2>::value == true);  // has close() member function
    }
    
    
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambufDefCtor)
    {
        typedef NSInputStreamStreambuf   stream_buffer_t;
        
        @autoreleasepool {
        
            stream_buffer_t sbuf;
            EXPECT_EQ(std::streamsize(0), sbuf.in_avail());        
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf_open)
    {
        @autoreleasepool {
        
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
        
        }
    }

    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf_open_error)
    {
        @autoreleasepool {
        
            const NSUInteger Size = 32000;
            NSMutableData* data = [NSMutableData dataWithLength:Size];
            char* start = static_cast<char*>([data mutableBytes]);
            char* p = start;
            for (int i = 0; i < Size; ++i, ++p) {
                *p = (i % 10) + '0';
            }
            NSInputStream* nsistream = [NSInputStream inputStreamWithData:data];
            // [nsistream open];  NOT open!
            EXPECT_THROW(internal::NSInputStreamSource iss(nsistream), std::invalid_argument);
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf_read)
    {
        @autoreleasepool {
        
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
            
            
            int count = 0;
            std::istreambuf_iterator<char> eos;
            std::istreambuf_iterator<char> iit(&issbuf);
            while (iit != eos) {
                char ch = *iit++;
                EXPECT_EQ(static_cast<char>('0'+ (count%10)), ch );
                ++count;
            }
            EXPECT_EQ(Size, count);
            
            
            issbuf.close();
            ASSERT_TRUE([nsistream streamStatus] == NSStreamStatusClosed);
        
        }
    }
    
    
    
    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambufDefCtor)
    {
        typedef NSOutputStreamStreambuf   stream_buffer_t;
        
        @autoreleasepool {
        
            stream_buffer_t sbuf;
            EXPECT_EQ(std::streamsize(0), sbuf.in_avail());
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf_open)
    {
        @autoreleasepool {
        
            NSOutputStream* nsostream = [NSOutputStream outputStreamToMemory];
            [nsostream open];
            internal::NSOutputStreamSink oss(nsostream);
            
            NSOutputStreamStreambuf ossbuf;
            ossbuf.open(boost::ref(oss), 1024);
            
            EXPECT_TRUE(ossbuf.is_open());
            
            ossbuf.close();
            ASSERT_TRUE([nsostream streamStatus] == NSStreamStatusClosed);
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf_open_error)
    {
        @autoreleasepool {
        
            NSOutputStream* nsostream = [NSOutputStream outputStreamToMemory];
            // [nsostream open]; NOT open!
            EXPECT_THROW(internal::NSOutputStreamSink oss(nsostream), std::invalid_argument);
        
        }
    }

    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf_write)
    {
        @autoreleasepool {
        
            NSOutputStream* nsostream = [NSOutputStream outputStreamToMemory];
            [nsostream open];
            internal::NSOutputStreamSink oss(nsostream);
            
            NSOutputStreamStreambuf ossbuf;
            ossbuf.open(boost::ref(oss), 1024);
            
            int const N = 10000;
            int const L = 101;
            for (int i = 0; i < N; ++i) {
                std::streamsize result =
                ossbuf.sputn("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n", L);
                EXPECT_EQ(static_cast<std::streamsize>(L), result);
            }
            // flush buffers
            ossbuf.pubsync();
            
            NSData* data = [nsostream propertyForKey:NSStreamDataWrittenToMemoryStreamKey];
            EXPECT_EQ(N*L, [data length]);
        
#if defined (DEBUG_XX)
        NSString* str = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:NSUTF8StringEncoding];
        NSLog(@"%@", str);
#endif
        
        }
    }
    
    
    
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf2_open)
    {
        @autoreleasepool {
        
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
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf2_open_error)
    {
        @autoreleasepool {
        
            const NSUInteger Size = 32000;
            NSMutableData* data = [NSMutableData dataWithLength:Size];
            char* start = static_cast<char*>([data mutableBytes]);
            char* p = start;
            for (int i = 0; i < Size; ++i, ++p) {
                *p = (i % 10) + '0';
            }
            NSInputStream* nsistream = [NSInputStream inputStreamWithData:data];
            // [nsistream open];  NOT open!
            EXPECT_THROW(internal::NSInputStreamSource2 iss(nsistream), std::invalid_argument);
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSInputStreamStreambuf2_read)
    {
        @autoreleasepool {
        
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
        
        }
    }
    


    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf2DefCtor)
    {
        typedef NSOutputStreamStreambuf2   stream_buffer_t;
        
        @autoreleasepool {
        
            stream_buffer_t sbuf;
            EXPECT_EQ(std::streamsize(0), sbuf.in_avail());
        
        }
    }

    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf2_open)
    {
        @autoreleasepool {
        
            NSOutputStream* nsostream = [NSOutputStream outputStreamToMemory];
            [nsostream open];
            internal::NSOutputStreamSink2 oss(nsostream);
            
            NSOutputStreamStreambuf2 ossbuf;
            ossbuf.open(oss, 1024);
            
            EXPECT_TRUE(ossbuf.is_open());
            
            ossbuf.close();
            ASSERT_TRUE([nsostream streamStatus] == NSStreamStatusClosed);
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf2_open_error)
    {
        @autoreleasepool {
        
            NSOutputStream* nsostream = [NSOutputStream outputStreamToMemory];
            // [nsostream open]; NOT open!
            EXPECT_THROW(internal::NSOutputStreamSink2 oss(nsostream), std::invalid_argument);
        
        }
    }
    
    TEST_F(NSStreamStreambufTest, NSOutputStreamStreambuf2_write)
    {
        @autoreleasepool {
        
            NSOutputStream* nsostream = [NSOutputStream outputStreamToMemory];
            [nsostream open];
            internal::NSOutputStreamSink2 oss(nsostream);
            
            NSOutputStreamStreambuf2 ossbuf;
            ossbuf.open(oss, 1024);
            
            int const N = 10000;
            int const L = 101;
            for (int i = 0; i < N; ++i) {
                std::streamsize result =
                ossbuf.sputn("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n", L);
                EXPECT_EQ(static_cast<std::streamsize>(L), result);
            }
            
            // flush buffers
            ossbuf.pubsync();
            
            NSData* data = [nsostream propertyForKey:NSStreamDataWrittenToMemoryStreamKey];
            EXPECT_EQ(N*L, [data length]);
        
#if defined (DEBUG_XX)
        NSString* str = [[NSString alloc] initWithBytes:[data bytes] length:[data length] encoding:NSUTF8StringEncoding];
        NSLog(@"%@", str);
#endif
        
        }
    }
    
    
}

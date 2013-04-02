//
//  syncqueue_streambuf_test.cpp
//  Test
//
//  Created by Andreas Grosam on 9/18/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <gtest/gtest.h>
#include "json/utility/syncqueue_streambuf.hpp"

#include "json/utility/synchronous_queue.hpp"
#include "json/objc/CFDataBuffer.hpp"

#include <dispatch/dispatch.h>

#include <iostream>
#include <iomanip>
#include <iterator>

// for testing


namespace {
    
    using json::utility::synchronous_queue;
    using json::utility::basic_syncqueue_streambuf;
        
    
    class syncqueue_streambuf_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        syncqueue_streambuf_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~syncqueue_streambuf_test() {
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
    
    TEST_F(syncqueue_streambuf_test, Constructor) 
    {
        typedef json::objc::CFDataBuffer<char>                  CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>                 queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, char>   stream_buffer_t;
        
        typedef std::pair<queue_t::result_type, queue_t::value_type> result_t;
        
        const char* s = "0123456789";
        CFDataBuffer buffer = CFDataBuffer(s, strlen(s)); 
        queue_t queue;
        
        EXPECT_EQ(queue_t::TIMEOUT_NOT_PICKED, queue.put(buffer, 0));
        EXPECT_EQ(false, queue.empty());
        
        stream_buffer_t sbuffer(queue);
                
        for (int i = 0; i < 10; ++i) {
            char ch = sbuffer.sbumpc();
            EXPECT_EQ(s[i], ch);
        }
    }    

    
    TEST_F(syncqueue_streambuf_test, syncqueue_streambuf_with_istream_iterator) 
    {
        typedef json::objc::CFDataBuffer<char>                  CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>                 queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, char>   stream_buffer_t;
        
        typedef std::pair<queue_t::result_type, queue_t::value_type> result_t;
        
        const char* s = "0123456789";
        queue_t queue;
        queue_t& q = queue;
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        dispatch_async(dispatch_get_global_queue(0, 0), ^{
            CFDataBuffer buffer = CFDataBuffer(s, strlen(s));
            EXPECT_EQ(queue_t::OK, q.put(buffer, 0.2)) << "put() timed out";
            EXPECT_EQ(queue_t::OK, q.put(CFDataBuffer(), 1)) << "put() timed out";
            dispatch_semaphore_signal(sem);
        });
        
        stream_buffer_t sbuffer(queue);
        std::istreambuf_iterator<char> eos;
        std::istreambuf_iterator<char> iit (&sbuffer); // stdin iterator
        
        for (int i = 0; i < 10; ++i) {
            char ch = *iit;
            EXPECT_EQ(s[i], ch);
            ++iit;
        }
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
        dispatch_release(sem);
    }    
    
    
    TEST_F(syncqueue_streambuf_test, syncqueue_streambuf_with_istream_iterator2) 
    {
        typedef json::objc::CFDataBuffer<char>                      CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>                     queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, wchar_t>    stream_buffer_t;
        
        typedef std::pair<queue_t::result_type, queue_t::value_type> result_t;
        
        const wchar_t* s = L"0123456789";
        const size_t s_len = std::char_traits<wchar_t>::length(s);
        char const* data_ptr = reinterpret_cast<char const*>(s);
        
        queue_t queue;
        queue_t& q = queue;
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        dispatch_async(dispatch_get_global_queue(0, 0), ^{
            q.put(CFDataBuffer(reinterpret_cast<char const*>(data_ptr), sizeof(wchar_t)*s_len));
            q.put(CFDataBuffer());
            dispatch_semaphore_signal(sem);
        });
        
        stream_buffer_t sbuffer(queue);
        std::istreambuf_iterator<wchar_t> eos;
        std::istreambuf_iterator<wchar_t> iit (&sbuffer); // stdin iterator
        
        int pos = 0;
        while (iit != eos) {
            ASSERT_TRUE( pos < s_len );
            EXPECT_TRUE(s[pos++] == static_cast<wchar_t>(*iit++));
        }
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
        dispatch_release(sem);
    }    
    
    
    
    TEST_F(syncqueue_streambuf_test, Concurrent) 
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
    
    
#if defined (_LIBCPP_VERSION) && _LIBCPP_VERSION == 1101
    TEST_F(syncqueue_streambuf_test, DISABLED_missaligned_buffer)
#else
    TEST_F(syncqueue_streambuf_test, missaligned_buffer)
#endif
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
    
    
    
}
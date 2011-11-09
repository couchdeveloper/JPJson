//
//  main.cpp
//  sync_queue_iterator_bench
//
//  Created by Andreas Grosam on 9/18/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include "json/utility/syncqueue_streambuf.hpp"
#include "json/utility/synchronous_queue.hpp"
#include "json/utility/istreambuf_iterator.hpp"
#include "json/objc/CFDataBuffer.hpp"
#include <dispatch/dispatch.h>
#include "utilities/timer.hpp"


using json::utility::basic_syncqueue_streambuf;
using json::utility::synchronous_queue;
using utilities::timer;



namespace {
    
    const char* s1 = 
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
    "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
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
    const char* s2 = 
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
    
    // strlen(s1) - strlen(s2) == 1000
    
    const char* s3 = "0";

    
    const size_t N = 100000;
    
    
    void test1(const char* s) 
    {
        std::cout << "--------------------------------------------------------\n";
        std::cout << "     Concurrent syncqueue_streambuf and iterator        \n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::endl;
        
        typedef json::objc::CFDataBuffer<char>              CFDataBuffer;
        typedef synchronous_queue<CFDataBuffer>             queue_t;
        typedef basic_syncqueue_streambuf<CFDataBuffer, char>    stream_buffer_t;
        
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
        __block bool producer_timeout_occured = false;
        dispatch_group_async(group, gc_queue, ^{
            put_result_t result;
            const size_t len = strlen(s);
            for (int i = 0; i <= N; ++i) {
                if (i != N) {
                    result = queue_ptr->put(CFDataBuffer(s, len), timeout);
                }
                else {
                    result = queue_ptr->put(NULL, timeout);
                }
                if (result != queue_t::OK) {
                    //producer_timeout_occured = true;
                    printf("producer timed out at count = %d\n", i);
                    return;
                }
            }
        });
        
        // Consumer
        dispatch_group_async(group, gc_queue, ^{
            const json::utility::istreambuf_iterator<char> eof;  
            json::utility::istreambuf_iterator<char> iit (sbuffer_ptr); // stdin iterator
            int x = 0;
            //int ch;
            //while ((ch = *iit) != EOF) {
            while (iit != eof) {
                x += *iit;
                ++iit;
            }
            size_t pos = sbuffer_ptr->pubseekoff(0, std::ios_base::cur);
            printf("%d - consumer stopped at count = %lu, timed out: %s\n", 
                   x, pos, (sbuffer_ptr->timeout_occured()?"Yes":"No"));
        });
        
        
        timer t;
        t.start();
        long res = dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
        bool completed = res == 0;
        t.stop();
        dispatch_release(group);
        
        std::cout << "producer_timeout_occured: " << producer_timeout_occured << "\n";
        std::cout << "completed: " << completed << std::endl;
        std::cout << "elapsed time: " << std::fixed << std::setprecision(2) << t.seconds() << "s\n\n" << std::endl;
    }
    
    
    void test2(const char* s)
    {
        std::cout << "--------------------------------------------------------\n";
        std::cout << " Serial in one thread char* iterator and CFDataBuffer  \n";
        std::cout << "--------------------------------------------------------\n";
        std::cout << std::endl;
        
        typedef json::objc::CFDataBuffer<char>  CFDataBuffer;
        
        timer t;
        t.start();
        // produce/consume:
        const size_t len = strlen(s);
        size_t pos = 0;
        int x = 0;
        for (int i = 0; i < N; ++i) {
            CFDataBuffer buffer = CFDataBuffer(s, len); 
            const char* first = buffer.data();
            const char* last = first + buffer.size();
            while (first != last) {
                ++pos;
                x += *first;
                ++first;
            }            
        }
        t.stop();
        
        printf("%d - producer and consumer stopped at count = %lu\n", 
               x, pos);
        
        std::cout << "elapsed time: " << std::fixed << std::setprecision(2) << t.seconds() << "s\n\n" << std::endl;

    }
    
}

int main (int argc, const char * argv[])
{

    test1(s1);
    test1(s2);
    test1(s3);
    test2(s1);
    test2(s2);
    test2(s3);
    return 0;
}


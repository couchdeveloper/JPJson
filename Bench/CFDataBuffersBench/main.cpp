//
//  main.cpp
//  CFDataBuffersBench
//
//  Created by Andreas Grosam on 6/27/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include "json/ObjC/CFDataBuffer.hpp"
#include "json/ObjC/semaphore.hpp"
#include "json/ObjC/mutex.hpp"
#include "json/ConstBuffers.hpp"
#include "json/ConstBuffers_iterator.hpp"
#include "utilities/timer.hpp"
#include <iostream>
#include <dispatch/dispatch.h>
#include <assert.h>


#define  USE_FUTURE


using json::objc::gcd::semaphore;
using json::objc::gcd::mutex;
using json::objc::CFDataBuffer;


namespace {
    const size_t DATA_SIZE = 1024*16;
    const size_t BUFFER_SIZE = 1024*16;
    const size_t N = DATA_SIZE / BUFFER_SIZE;
    const size_t C = 4;
}

// Create a buffers instance with capacity N, then produce N buffers with 
// BUFFER_SIZE bytes, fill them and when finished, consume and process them.
// Performs sequencial in one thread. Allocatates all required buffers
// for the duration of the whole operation.
// The performance may suffer due to the massive allocations.
void SimpleProduceConsume1(size_t N, long& result)
{
    using json::ConstBuffers;

    typedef CFDataBuffer<char>          buffer_type;
    typedef ConstBuffers<char, buffer_type, mutex, semaphore>    buffers_type;
    
    // Create a buffers instance with at max N buffers:
    buffers_type buffers(N);
    
    
    size_t k = 0;        
    long sum = 0;        
    UInt8 data[BUFFER_SIZE];
    
    for (int i = 0; i < N; ++i) {
        // fill the buffer:
        for (int j = 0; j < BUFFER_SIZE; ++j, ++k) {
            data[j] = char(k);                                     
        }                                 
        CFDataRef d = CFDataCreate(NULL, data, BUFFER_SIZE);
        buffer_type buffer = d;
        CFRelease(d);
        buffers.produce(buffer);
    }
    
    for (int i = 0; i < N; ++i) {
        buffer_type* bufferPtr = buffers.aquire(3);
        if (bufferPtr) {
            //std::cout << '-' << std::flush;
            size_t len = bufferPtr->size();
            const char* p  = bufferPtr->data();
            for (int j = 0; j < len; ++j) {
                sum += *p;
                ++p;
            }                                 
            buffers.commit(bufferPtr);
        }
        else {
            //std::cout << 'x' << std::flush;
        }
    }
    result = sum;
}

// Create a buffers instance with capacity 1, then produce N times one buffer
// with size BUFFER_SIZE and fill it. Consume and process it immediately and 
// release the buffer. This versions allocates only one buffer per iteration. 
// Performs in one thread. 
// Performance should be fast compared to other approaches, however since it
// uses the CFDataConstBuffers object it invovls a certain overhead due to 
// its thread safe design (which is not used in this case).
void SimpleProduceConsume2(size_t N, long& result)
{
    using json::ConstBuffers;
    
    typedef CFDataBuffer<char>          buffer_type;
    typedef ConstBuffers<char, buffer_type, mutex, semaphore>    buffers_type;
    
    // Create a buffers instance with at max N buffers:
    buffers_type buffers(1);
        
    size_t k = 0;        
    long sum = 0;        
    UInt8 data[BUFFER_SIZE];
    
    for (int i = 0; i < N; ++i) {
        // fill the buffer:
        for (int j = 0; j < BUFFER_SIZE; ++j, ++k) {
            data[j] = char(k);                                     
        }                                 
        CFDataRef d = CFDataCreate(NULL, data, BUFFER_SIZE);
        buffer_type buffer = d;
        CFRelease(d);
        buffers.produce(buffer);
        buffer_type* bufferPtr = buffers.aquire(3);
        if (bufferPtr) {
            //std::cout << '-' << std::flush;
            size_t len = bufferPtr->size();
            const char* p  = bufferPtr->data();
            for (int j = 0; j < len; ++j) {
                sum += *p;
                ++p;
            }                                 
            buffers.commit(bufferPtr);
        }
        else {
            //std::cout << 'x' << std::flush;
        }
    }
    result = sum;
}


// Create a mutable NSData object. For N times create and produce a buffer with 
// size BUFFER_SIZE and fill it. Consume the buffer and append the content to 
// the mutbale NSData object. This may involve to reallocate the mutable buffer 
// and may require to copy the content. When finished process the content of the 
// mutable NSData object. 
// This approach seems to have a sever overhead - but it turns out to perform 
// quite well, possibly due to internal optimizations.
// Performs in one thread. 
void SequencialProduceConsumClassic(size_t N, long& result)
{
    using json::ConstBuffers;
    
    typedef CFDataBuffer<char>          buffer_type;
    typedef ConstBuffers<char, buffer_type, mutex, semaphore>    buffers_type;
    
    typedef std::pair<buffer_type, bool> result_t;
    
    // Create a buffers instance with at max N buffers:
    buffers_type buffers(1);
    
    size_t k = 0;        
    long sum = 0;        
    UInt8 data[BUFFER_SIZE];
    
    CFMutableDataRef buffer = CFDataCreateMutable(NULL, 0);
    
    for (int i = 0; i < N; ++i) {
        // Produce:
        // fill the intermediate buffer:
        for (int j = 0; j < BUFFER_SIZE; ++j, ++k) {
            data[j] = char(k);                                     
        }                                 
        CFDataRef d = CFDataCreate(NULL, data, BUFFER_SIZE);
        assert(d);

        // Consume:
        CFDataAppendBytes(buffer, CFDataGetBytePtr(d), CFDataGetLength(d));
        CFRelease(d);        
    }
    
    // Process:
    unsigned long total = 0;        
    const char* first = reinterpret_cast<const char*>(CFDataGetBytePtr(buffer));
    const char* last = first + CFDataGetLength(buffer);
    while (first != last) {
        sum += *first;
        ++first;
        ++total;
    }
    assert(total == N*BUFFER_SIZE);
    
    CFRelease(buffer);

    result = sum;
}




// Create a buffers instance with capacity C, then produce N buffers with
// BUFFER_SIZE bytes and fill the buffers. Concurrently consume the buffers.
// Performs concurrently on two threads. 

void ConcurrentProduceConsume1(size_t C, size_t N, long& result)
{
    using json::ConstBuffers;
    
    typedef CFDataBuffer<char>          buffer_type;
    typedef ConstBuffers<char, buffer_type, mutex, semaphore>    buffers_type;
    
    
    // Create a buffers instance with at max C buffers:
    buffers_type buffers(C);
    buffers_type* buffersPtr = &buffers;
        
    // Get the global concurrent queue:
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    // Create a group:
    dispatch_group_t group = dispatch_group_create();
    
    dispatch_group_async(group, queue,
    ^{  
        unsigned long k = 0;        
        UInt8 data[BUFFER_SIZE];
        
        for (int i = 0; i < N; ++i) {
            // fill the buffer:
            for (int j = 0; j < BUFFER_SIZE; ++j, ++k) {
                data[j] = char(k);                                     
            }                                 
            CFDataRef d = CFDataCreate(NULL, data, BUFFER_SIZE);
            buffer_type buffer = d;
            CFRelease(d);
            buffersPtr->produce(buffer);
            //std::cout << '+' << std::flush;
        }
    });
    
    
#if defined (USE_FUTURE)        
    __block size_t total = 0;
    __block long sum = 0;
#endif
    
    dispatch_group_async(group, queue,
    ^{
        size_t total_ = 0;
        long sum_ = 0;
        for (int i = 0; i < N; ++i) {
            buffer_type* result = buffersPtr->aquire(2.0);
            if (result) {
                //std::cout << '-' << std::flush;
                size_t len = result->size();
                const char* p  = result->data();
                for (int j = 0; j < len; ++j) {
                    sum_ += *p;
                    ++total_;
                    ++p;
                }                                 
                buffersPtr->commit(result);
            }
            else {
                //std::cout << 'x' << std::flush;
            }
        }
#if defined (USE_FUTURE)        
        sum = sum_;
        total = total_;
#endif    
        //std::cout <<  std::endl;
    });
    
    
    if (dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 1e9*10)))
    {
        std::cout << "ERROR: ConcurrentProduceConsume1 received timeout." << std::endl;
    }
#if defined (USE_FUTURE)        
    if (total != BUFFER_SIZE*N) {
        std::cout << "ERROR: ConcurrentProduceConsume2: could not receive all characters" << std::endl;
    }
    result = sum;
#endif    
    dispatch_release(group);
}


// Create a buffers instance with capacity C, then produce N buffers with 
// BUFFER_SIZE bytes and fill them. Concurrently consume the buffers and 
// iterate over the consumed buffer.
// Performs concurrently on two threads. 

void ConcurrentProduceConsume2(size_t C, size_t N, long& result)
{
    using json::ConstBuffers;
    
    typedef CFDataBuffer<char>          buffer_type;
    typedef ConstBuffers<char, buffer_type, mutex, semaphore>    buffers_type;
    
    // Create a buffers instance with at max C buffers:
    buffers_type buffers(C);
    buffers_type* buffersPtr = &buffers;
    
    
    // Get the global concurrent queue:
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    // Create a group:
    dispatch_group_t group = dispatch_group_create();
    
    dispatch_group_async(group, queue,
    ^{        
        unsigned long k = 0;
        UInt8 data[BUFFER_SIZE];
        for (int i = 0; i < N; ++i) {
            // fill the buffer:
            for (int j = 0; j < BUFFER_SIZE; ++j, ++k) {
                data[j] = char(k);                                     
            }                                 
            CFDataRef d = CFDataCreate(NULL, data, BUFFER_SIZE);
            buffer_type buffer = d;
            CFRelease(d);
            buffersPtr->produce(buffer);
            //std::cout << '-' << std::flush; //TODO         
        }
        
        // put EOF:
        buffersPtr->produce(buffer_type());
        //std::cout << std::endl; // TODO
    });
    
#if defined (USE_FUTURE)        
    __block long sum = 0;
    __block size_t total = 0;
#endif    

    dispatch_group_async(group, queue,
     ^{         
         long sum_ = 0;
         size_t total_ = 0;
         buffer_type* buffer = 0;
         
         try {
            const char* p;
            const char* back;
            
            buffer = buffersPtr->aquire(3);
            if (buffer and buffer->data() and buffer->size() > 0) {
                p = buffer->data();
                back = p + buffer->size() - 1;
            } 
            else {
                // timeout or eof
                p = 0;
            }
                 
            while (p != 0) 
            {   
                ++total_;
                sum_ += *p;
                
                // Increment p. consume a new buffer if required:
                if (p != back) {
                    ++p;
                }
                else
                {
                    buffersPtr->commit(buffer);
                    buffer = 0;
                    buffer = buffersPtr->aquire(3);
                    if (buffer and buffer->data() and buffer->size() > 0) {
                        p = buffer->data();
                        back = p + buffer->size() - 1;
                    }
                    else {
                        // eof or timeout
                        p = 0;
                    }
                }
            }
            if (buffer)
                buffersPtr->commit(buffer);
             
         }
         catch (...) {
             if (buffer)
                 buffersPtr->commit(buffer);
         }
        
#if defined (USE_FUTURE)        
        sum = sum_;
        total = total_;
#endif    
    });
    
    //if (dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 1e9*10)))
    if (dispatch_group_wait(group, DISPATCH_TIME_FOREVER)){
        std::cout << "ERROR: ConcurrentProduceConsume2 received timeout." << std::endl;
    }
#if defined (USE_FUTURE)        
    if (total != BUFFER_SIZE * N) {
        std::cout << "ERROR: ConcurrentProduceConsume2: could not receive all characters" << std::endl;
    }
    result = sum;
#endif    
    
    dispatch_release(group);
}





// Create a buffers instance with capacity C, then produce N buffers with 
// BUFFER_SIZE bytes. Concurrently consume the buffers and process it with 
// iterating over the buffer's content.
// Performs concurrently on two threads. 

void ConcurrentProduceConsumeIter(size_t C, size_t N, long& result)
{
    using json::ConstBuffers;
    using json::ConstBuffers_iterator;
    
    typedef CFDataBuffer<char>          buffer_type;
    typedef ConstBuffers<char, buffer_type, mutex, semaphore>    buffers_type;
    typedef ConstBuffers_iterator<char, buffers_type> iterator;
    
    // Create a buffers instance with at max C buffers:
    buffers_type buffers(C);
    buffers_type* buffersPtr = &buffers;
    
    // Get the global concurrent queue:
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    // Create a group:
    dispatch_group_t group = dispatch_group_create();
    
    
    dispatch_group_async(group, queue,
                         ^{  
                             unsigned long k = 0;
                             UInt8 data[BUFFER_SIZE];
                             for (int i = 0; i < N; ++i) {
                                 // fill the buffer:
                                 for (int j = 0; j < BUFFER_SIZE; ++j, ++k) {
                                     data[j] = char(k);                                     
                                 }                                 
                                 CFDataRef d = CFDataCreate(NULL, data, BUFFER_SIZE);
                                 buffer_type buffer = d;
                                 CFRelease(d);
                                 buffersPtr->produce(buffer);
                                 //std::cout << '-' << std::flush; //TODO         
                             }
                             
                             // put EOF:
                             buffersPtr->produce(buffer_type());
                             //std::cout << std::endl; // TODO
                         });
    
    
#if defined (USE_FUTURE)        
    __block long sum = 0;
    __block size_t total = 0;
#endif    
    dispatch_group_async(group, queue,
                         ^{
                             iterator eof;
                             iterator iter(*buffersPtr);
                             
                             long sum_ = 0;
                             size_t total_ = 0;
                             
                             while (iter != eof)
                             {
                                 //if (iter.avail() == BUFFER_SIZE)
                                 //    std::cout << '+' << std::flush;  // TODO:
                                 sum_ += *iter;
                                 ++iter;
                                 ++total_;
                             }
                             
#if defined (USE_FUTURE)
                             sum = sum_;
                             total = total_;
#endif         
                         });
    
    //if (dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, 1e9*10)))
    if (dispatch_group_wait(group, DISPATCH_TIME_FOREVER)){
        std::cout << "ERROR: ConcurrentProduceConsumeIter received timeout." << std::endl;
    }    
    
#if defined (USE_FUTURE)
    //std::cout <<  std::endl; // TODO;
    if (total != BUFFER_SIZE * N) {
        std::cout << "ERROR: ConcurrentProduceConsume3: could not receive all characters" << std::endl;
    }
    result = sum;
#endif    
    
}



        
int main (int argc, const char * argv[])
{
    using utilities::timer;

    timer t;
#if defined (DEBUG)    
    const int K = 1;
#else
    const int K = 10;
#endif
    size_t c = C;
    
#if defined (USE_FUTURE)
    bool usingFuture = true;
#else    
    bool usingFuture = false;
#endif    
    
    std::cout << "\n**** CFDataBuffers Benchmark ****\n";
    std::cout << "Using  Future: " << (usingFuture ? "Yes" : "No") << std::endl;
    std::cout << "Data size: " << (BUFFER_SIZE * N)/1024 << "KB, " <<
        "Buffer size: " << BUFFER_SIZE << ", N = " << N << ", C = " << C << std::endl;

    
    long result = 0;
    
    
#if 1 
    t.start();
    for (int i = 0; i < K; ++i) {
        result = 0;
        SimpleProduceConsume1(N, result);
    }
    t.stop();
    std::cout << "[SimpleProduceConsume1]: Elapsed time: " << t.seconds()*1e3/K << "ms" << std::endl;
    std::cout << "Result: " << result << std::endl;
    
    t.reset();
    t.start();
    for (int i = 0; i < K; ++i) {
        result = 0;
        SimpleProduceConsume2(N, result);
    }
    t.stop();
    std::cout << "[SimpleProduceConsume2]: Elapsed time: " << t.seconds()*1e3/K << "ms" << std::endl;
    std::cout << "Result: " << result << std::endl;
#endif


#if 1
    t.reset();
    t.start();
    for (int i = 0; i < K; ++i) {
        result = 0;
        SequencialProduceConsumClassic(N, result);
    }
    t.stop();
    std::cout << "[Classic]: Elapsed time: " << t.seconds()*1e3/K << "ms" << std::endl;
    std::cout << "Result: " << result << std::endl;
#endif    
    
    
    
#if 1
    for (int i = 0; i < 1; ++i) {
        t.reset();
        t.start();
        for (int i = 0; i < K; ++i) {
            result = 0;
            ConcurrentProduceConsume1(c, N, result);
        }
        t.stop();
        std::cout << "[ConcurrentProduceConsume1]: Elapsed time: " << t.seconds()*1e3/K << "ms" << std::endl;
        std::cout << "Result: " << result << std::endl;
        c += c;
    }
#endif
    
#if 1 
    
    c = C;
    for (int i = 0; i < 1; ++i) {
        t.reset();
        t.start();
        for (int i = 0; i < K; ++i) {
            result = 0;
            ConcurrentProduceConsume2(c, N, result);
        }
        t.stop();
        std::cout << "[ConcurrentProduceConsume2]: Elapsed time: " << t.seconds()*1e3/K << "ms" << std::endl;
        std::cout << "Result: " << result << std::endl;
        c += c;
    }
#endif
    
#if 1
    c = C;
    for (int i = 0; i < 1; ++i) {
        t.reset();
        t.start();
        for (int i = 0; i < K; ++i) {
            result = 0;
            ConcurrentProduceConsumeIter(c, N, result);
        }
        t.stop();
        std::cout << "[ConcurrentProduceConsumeIter]: Elapsed time: " << t.seconds()*1e3/K << "ms" << std::endl;
        std::cout << "Result: " << result << std::endl;
        c += c;
    }
#endif
    
    
    
    return 0;
}


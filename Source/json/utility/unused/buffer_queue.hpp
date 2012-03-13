#ifndef JSON_BUFFER_QUEUE_HPP
#define JSON_BUFFER_QUEUE_HPP
//
//  buffer_queue.hpp
//
//  Created by Andreas Grosam on 7/1/11.
//  Copyright 2011 Andreas Grosam
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#error Unused

#include <list>
#include <assert.h>
#include <algorithm>
#include "producer_consumer_queue.hpp"



namespace json {
    
    //
    // Class buffer_queue
    //
    // A buffer_queue is a FIFO container holding an ordered sequence of 
    // const buffers of type T, where each buffer may have an individual size.
    // The member function produce() adds an initialized vector to the buffers 
    // instance, while the member functions aquire() returns a pointer to the
    // front buffer of the avialable buffers to the consumer. When finished with 
    // processing the consumer shall call commit() which effectively removes the 
    // buffer from the buffers instance.
    //
    // The producer/aquire/commit functions are thread safe. 
    // A buffers instance can have one or more cunsumers which concurrently
    // process the buffers.
    //
    // It is important to note that a consumer never takes ownership of a buffer.
    // The buffers are invalidated when the buffers list will be destroyed.
    //
    // The member function produce() may block until the number of available 
    // buffers is less than the capacity, effectively limiting the maximum 
    // number of buffers and hence is a means to limit the dynamically allocated 
    // memory during runtime.
    //
    // The member function aquire() may block until after at least one vector 
    // is available or if the specified timeout has been expired.
    //
    // An instance of buffer_queue shall be used in the typical 
    // Consumer / Producer pattern, where the client of aquire()/commit() and 
    // the client of produce() will execute in different threads.
    //
    
    
    template <typename BufferT, typename MutexT, typename SemaphoreT>
    class buffer_queue
    {
    public:
        typedef BufferT                       buffer_type;
        typedef buffer_type*                  buffer_pointer;
        typedef typename SemaphoreT::duration_type duration_type;
        
        static const duration_type TIMEOUT_FOREVER;
        
    public:        
        // Creates a buffers instance with at max capacity buffers.
        // capacity shall be greater or equal 1.
        buffer_queue(size_t capacity = 10) 
        : queue_(capacity) 
        {
            assert(capacity > 0);
        }
        
        bool produce(const buffer_type& buffer, duration_type timeout = buffer_queue::TIMEOUT_FOREVER) {
            return queue_.produce(buffer, timeout);
        }
        
        buffer_pointer aquire(duration_type timeout = buffer_queue::TIMEOUT_FOREVER) {
            return queue_.aquire(timeout);
        }
        
        void commit(buffer_pointer buffer) {
            queue_.commit(buffer);
        }
        
        
        size_t  avail() const {
            return queue_.avail();
        }
        
        size_t  processing() const {
            return queue_.processing();            
        }
        
        bool empty() const {
            return queue_.empty();
        }
        
        void clear() {
            queue_.clear();
        }
        
        void commit() {
            queue_.commit();
        }
        
        void lock() const {
            return queue_.lock();
        }
        
        void unlock() const {
            queue_.unlock();
        }
        
        
    private:
        typedef json::utility::producer_consumer_queue<BufferT, MutexT, SemaphoreT> queue_t;
        
        queue_t queue_;
    };
    
    template <typename B, typename M, typename S>
    typename buffer_queue<B,M,S>::duration_type const buffer_queue<B,M,S>::TIMEOUT_FOREVER = S::wait_forever();

    
    
} // namespace json




#endif // JSON_BUFFER_QUEUE_HPP
#ifndef JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP
#define JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP
//
//  producer_consumer_queue.hpp
//
//  Created by Andreas Grosam on 7/2/11.
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

#if defined (DEBUG)
 #define JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP_LOG_DEBUG
#endif



#if defined (JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP_LOG_DEBUG)
#include <iostream>
#include <pthread.h>
#endif


namespace json { namespace utility {

    // A producer_consumer_queue is a managed concurrent FIFO container containing 
    // elements of type T in a strict ordered sequence. A managed producer con-
    // sumer queue has two internal containers: the FIFO queue where elements
    // are waiting for being processed (avail_queue), and a list which contains 
    // elements which are currently processes by a consumer (the process_list).
    //
    // A prodcuer creates and initialzes an element of type T. Then it calls the 
    // member function produce() which adds the element to the back (that is, as 
    // the newest element) to the internal FIFO container, the "avail_queue".
    //
    // To get an element for processing, a consumer calls the member function
    // aquire() which moves the element from the FIFO container (the 
    // "avail_queue" to its internal "process_list" container. Then it returns
    // a pointer to the element. When the consumer is finished with processing 
    // the element, it shall call the member function 
    // commit(). commit() finally erases the element from the process_list.
    // 
    //
    // The producer/aquire/commit functions are thread safe. 
    // A producer_consumer_queue can have one or more cunsumers which concurrently
    // process the elements.
    //
    //
    // size() returns the sum of the number of elements in the avail_queue and
    // the process_list.
    //
    // The capacity of the producer_consumer_queue is defined as the maximum of 
    // the sum of the number of elements in the avail_queue and the process_list.
    //
    // The member function produce() will block until size is less than the 
    // capacity, effectively limiting the maximum number of elements and hence 
    // is a means to limit the dynamically allocated memory during runtime.
    //
    // The member function aquire() may block until after at least one element 
    // is available or if the specified timeout has been expired.
    //
    //
    //  Requierments:
    //  T shall have a copy ctor and an operator== defined.
    //  operator== () must uniquely identify a buffer.
    //  
    
    // TODO: use scoped locks
    
    template <typename T, typename MutexT, typename SemaphoreT /*, typename CapacityPolicy */>
    class producer_consumer_queue
    {
    public:
        typedef T                                   value_type;
        typedef T*                                  pointer;
        typedef typename SemaphoreT::duration_type  duration_type;     
        
    protected:
        typedef std::list<T>            queue_type;  // avail_queue
        typedef std::list<T>            list_type;   // process_list        
        
        typedef typename queue_type::iterator queue_iterator;
        typedef typename list_type::iterator  list_iterator;
        
    public:        
        // Creates a producer_consumer_queue instance with at max capacity buffers.
        // capacity shall be greater or equal 1.
        producer_consumer_queue(size_t capacity = 10) 
        : avail_(0), capacity_(capacity) 
        {
            assert(capacity > 0);
        }
        
        ~producer_consumer_queue() 
        {
            // The process_list shall be empty by now! Otherwise a crash
            // may happen, if there are external references to the elements
            // in the process_list!
            mutex_.lock();
#if defined (JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP_LOG_DEBUG)
            if (avail_queue_.size()) {
                std::cout << "WARNING: ~producer_consumer_queue: avail queue not empty." << std::endl;
            }
            if (process_list_.size()) {
                std::cout << "WARNING: ~producer_consumer_queue: process list is not empty" << std::endl;
            }            
#endif                
            size_t avail_count = avail_queue_.size();
            while (avail_count and avail_.wait(0)) {
                --avail_count;
                capacity_.signal();
            }
            
            size_t process_count = process_list_.size();
            while (process_count--) {
                capacity_.signal();
            }
            
            mutex_.unlock();
        }
        
        bool produce(const T& value, producer_consumer_queue::duration_type timeout = SemaphoreT::wait_forever()) 
        {
            if (capacity_.wait(timeout)) {
                mutex_.lock();
                avail_queue_.push_back(value);
#if defined (JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP_LOG_DEBUG)
                size_t a = avail_queue_.size();
                size_t p = process_list_.size();
                printf("[%x] producer_consumer_queue: produced: avail %lu, processing: %lu\n",
                       pthread_mach_thread_np(pthread_self()), a, p);
#endif            
                mutex_.unlock();
                avail_.signal();
                
                return true;
            }
#if defined (JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP_LOG_DEBUG)
            printf("[%x] producer_consumer_queue: produce timed out at %g\n", pthread_mach_thread_np(pthread_self()), (double)timeout);
#endif            
            return false;
        }
        
        size_t avail() const 
        {
            size_t count;
            mutex_.lock();
            count = avail_queue_.size();
            mutex_.unlock();
            return count;
        }
        
        size_t processing() const 
        {
            size_t count;
            mutex_.lock();
            count = process_list_.size();
            mutex_.unlock();
            return count;
        }
        
        bool empty() const 
        {
            bool result;
            mutex_.lock();
            result = process_list_.size() == 0 and avail_queue_.size() == 0;
            mutex_.unlock();
            return result;
        }
        
        T* aquire(producer_consumer_queue::duration_type timeout = SemaphoreT::wait_forever()) 
        {
            if (avail_.wait(timeout)) {
                mutex_.lock();
                assert(avail_queue_.begin() != avail_queue_.end());
                process_list_.splice(process_list_.end(), avail_queue_, avail_queue_.begin());
                pointer vp = &process_list_.back();
#if defined (JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP_LOG_DEBUG)
                size_t a = avail_queue_.size();
                size_t p = process_list_.size();
                printf("[%x] producer_consumer_queue: aquired: avail %lu, processing: %lu\n",
                       pthread_mach_thread_np(pthread_self()), a, p);
#endif            
                mutex_.unlock();
                return vp;
            }
#if defined (JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP_LOG_DEBUG)
            printf("[%x] producer_consumer_queue: aquire timed out at %g\n", pthread_mach_thread_np(pthread_self()), (double)timeout);
#endif            
            return 0;
        }
        
        void commit(T* value) 
        {
            assert(value != 0);
            mutex_.lock();
            list_iterator first = process_list_.begin();
            list_iterator last = process_list_.end();
            while (first != last) {
                if ( (*first) == *value ) {
                    process_list_.erase(first);
                    capacity_.signal();
                    break;
                }
                ++first;
            }
            mutex_.unlock();
        }
        
        
        // empties the avail_queue
        void clear() 
        {
            mutex_.lock();
            size_t avail_count = avail_queue_.size();
            while (avail_count and avail_.wait(0)) {
                --avail_count;
                capacity_.signal();
            }
            avail_queue_.clear();
            mutex_.unlock();
        }

        // commits all elements in the process list
        void commit() {
            mutex_.lock();
            size_t process_count = process_list_.size();
            while (process_count--) {
                capacity_.signal();
            }
            process_list_.clear();
            mutex_.unlock();
        }

        void lock() const {
            return mutex_.lock();
        }
        
        void unlock() const {
            mutex_.unlock();
        }
        
        
    protected:
        std::list<value_type>  avail_queue_;
        std::list<value_type>  process_list_;
        mutable MutexT         mutex_;
        SemaphoreT             avail_;
        SemaphoreT             capacity_;
    };
    

    
    
}} // namespace json::utility
    
#endif // JSON_UTILITY_PRODUCER_CONSUMER_QUEUE_HPP
//
//  producer_consumer_queue2.hpp
//
//  Created by Andreas Grosam on 8/30/11.
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

#ifndef JSON_UTILITY_PRODUCER_CONSUMER_QUEUE2_HPP
#define JSON_UTILITY_PRODUCER_CONSUMER_QUEUE2_HPP


#include "atomic.hpp"
#include <assert.h>
#include <list>



#if defined (DEBUG)
#define JSON_UTILITY_PRODUCER_CONSUMER_QUEUE2_HPP_LOG_DEBUG
#endif

#if defined (JSON_UTILITY_PRODUCER_CONSUMER_QUEUE2_HPP_LOG_DEBUG)
#include <iostream>
#include <pthread.h>
#endif


namespace json { namespace utility {
    
    struct queue_category {};
    
    struct muliple_producer_multiple_consumer_category : queue_category {};
    struct single_producer_multiple_consumer_category : queue_category {};
    struct muliple_producer_single_consumer_category : queue_category {};
    struct single_producer_single_consumer_category : queue_category {};


// Implementations shall meet the following requirements:
//
// Thread safe, blocking FIFO queue with support for resource utilization.
//
// The queue shall be capable to handle one producer and one consumer
// concurrently, each running in a different thread.
//
// T shall have copying semantics.

// size_t capacity() const
// 
// Returns the capacity of the queue. capacity determines the maximum of the sum
// of available and processing objects for which the queue is capable to accept
// objects from one or more producers via produce without blocking the producer's 
// thread. capacity does not however limit avail. Virtually unlimited producers
// can be blocked, and hence size can grow beyond capacity.


// bool produce(const T& object)
//
// Add an object 'object' to the queue which shall become available for 
// consuming. When the functions enters, it increments avail (and increments size).
// The function shall block until after size is less than capacity.
// Returns true if the block has become available and can be consumed. Otherwise 
// if it returns false, this may indicate a timeout. If the result is false, it 
// must be guaranteed that object will never be consumed.


// T* aquire()
//
// Returns a pointer to the next available object from the queue. If an object 
// is available, it will be immediately returned. Otherwise, the calling thread 
// will be blocked until one object becomes available, or a timeout occurs. If a 
// timeout occurred, aquire will return NULL.
// If aquire returns an object, processing will be incremented and avail will be
// decremented.


// void commit(const T* object)
//
// commit() shall be send to the receiver from the consumer when it has finished 
// processing the object. On exit, processing and size will be decremented, and
// object will be released by the receiver.
// object shall be previously aquired by a consumer, and commit must be called
// only once for this object, otherwise the behavior is undefined.


// size_t processing() const
//
// Returns the number of objects currently processed by consumers. Note that the
// result may be possibly stale immediately after the method returns.


// size_t avail() const
//
// Returns the number of objects which still need to be processed. Note that the
// result may be possibly stale immediately after the method returns.


// size_t size() const
//
// Returns the sum of the available objects and objects currently processed. 
// Note that the result may be possibly stale immediately after the method 
// returns.


// bool empty() const
//
// Returns true if avail equals zero and processing equals zero. Note that the
// result may be possibly stale immediately after the method returns.


// void clear()
//
// Clears the avail queue.


// void commitAll()
//
// Commits any remaining uncommitted objects.


    template <int Capacity, typename T, 
        typename MutexT, typename SemaphoreT,
        bool MultipleProducer, bool MultipleConsumer>
    class producer_consumer_queue
    {
    };

}}
    

#if 0
namespace json { namespace utility {
    
    // Implementation notes: actually, the queue manages one object internally
    // in a temporary buffer (capacity would be one) in order to avoid
    // context switches when producing, aquiring and committing.
    
    template <
        int Capacity, typename T, 
        typename MutexT, typename SemaphoreT,
        bool bool MultipleProducer, bool MultipleConsumer
    >
    class producer_consumer_queue
    {
        typedef atomic<size_t> atomic_size_t;
    private:
        std::list<T>            avail_queue_;
        std::list<T>            process_list_;
        MutexT                  mutex_;
        SemaphoreT              sem_avail_;    
        SemaphoreT              sem_capacity_;
        atomic_size_t           avail_;
        char                    heap_[sizeof(T)];
        
    public:
        producer_consumer_queue() 
        : avail_p_(NULL), process_p_(NULL), sem_avail_(0), sem_capacity_(Capacity), avail_(0)
        {
        }
        
        ~producer_consumer_queue() {
        }
        
        size_t capacity() const { return Capacity; }
        
        
        // Add an object 'object' to the queue which shall become available for 
        // consuming. When the function enters, it increments avail (and incre-
        // ments size).
        // The function shall block until after size is less or equal than capa-
        // city.
        // Returns true if the object has become available and can be consumed. 
        // Otherwise if it returns false, this may indicate a timeout. If the 
        // result is false, it must be guaranteed that object will never be 
        // consumed.
        bool produce(const T& object, double t = SemaphoreT::wait_forever()) 
        {
            ++avail_;
            bool accepted;
            if (sem_capacity_.wait(t)) {
                mutex_.lock();
                avail_queue_.push_back();
                mutex_.unlock();   
                sem_avail_.signal();
                
                // wait again for capacity to become available:
                if (sem_capacity_.wait(t)) {
                    // ok, capacity is available, release it and go to retrieve the next object:
                    sem_capacity_.signal();
                    accepted = true;
                } 
                else 
                {
                    mutex_.lock();
                    // is my object still there?
                    if (avail_p_ and *avail_p_ == object) {
                        avail_p_->~T();
                        avail_p_ = NULL;
                        --avail_;
                        accepted = false;
                    }                    
                    mutex_.unlock();   
                }
            } else {
                --avail_;
                accepted = false;
            }
            
            return accepted;
        }
        
        
        // Returns a pointer to the next available object from the queue. If an object 
        // is available, it will be immediately returned. Otherwise, the calling thread 
        // will be blocked until one object becomes available, or a timeout occurs. If a 
        // timeout occurred, aquire will return NULL.
        // If aquire returns an object, processing will be incremented and avail will be
        // decremented.
        T* aquire(double t = SemaphoreT::wait_forever()) 
        {
            // wait until buffer avail
            T* result = NULL;
            if (sem_avail_.wait(t)) {
                mutex_.lock();
                --avail_;            
                process_p_ = avail_p_;
                avail_p_ = NULL;
                result = process_p_;
                mutex_.unlock();
            }
            return result;
        }
        
        
        // commit() shall be send to the receiver from the consumer when it has finished 
        // processing the object. On exit, processing and size will be decremented, and
        // object will be released by the receiver.
        // object shall be previously aquired by a consumer, and commit must be called
        // only once for this object, otherwise the behavior is undefined.
        void commit(const T* object) 
        {
            mutex_.lock();
            bool didCommit = true;
            if (object == process_p_) {
                object->~T();
                process_p_ = 0;
            }
            else {
                didCommit = false;
            }
            mutex_.unlock();
            if (didCommit) {
                sem_capacity_.signal();
            }
        }
        
        
        // Returns the number of objects currently processed by consumers. Note that the
        // result may be possibly stale immediately after the method returns.
        size_t processing() const {
            return process_p_ ? 1 : 0;
        }
        
        
        // Returns the number of objects which still need to be processed. Note 
        // that the result may be possibly stale immediately after the method 
        // returns.
        size_t avail() const {
            return avail_;
        }
        
        
        // Returns the sum of the available objects and objects currently processed. 
        // Note that the result may be possibly stale immediately after the method 
        // returns.
        size_t size() const {
            return static_cast<size_t>(avail_) + process_p_ ? 1 : 0;
        }
        
        
        // Returns true if avail equals zero and processing equals zero. Note that the
        // result may be possibly stale immediately after the method returns.
        bool empty() const {
            return (static_cast<size_t>(avail_) == 0 and process_p_ == 0);
        }
        
        
        // Clears the avail queue.
        void clear() {
            T* object;
            while ((object = aquire(0.0)) != NULL) {
                commit(object);
            }
        }
        
        
        // Commits any remaining uncommitted objects.
        bool commitAll() {
            mutex_.lock();
            bool didCommit = true;
            if (process_p_) {
                process_p_->~T();
                process_p_ = NULL;
            }
            else {
                didCommit = false;
            }
            mutex_.unlock();
            if (didCommit) {
                sem_capacity_.signal();
            }
            return didCommit;
        }
        
    };
    
    
}}
#endif


namespace json { namespace utility {
    

    //
    //  Template Specialization for Capacity == 0, MP, MC
    //
    
    // Implementation notes: actually, the queue manages one object internally
    // in a temporary buffer (capacity would be one) in order to avoid
    // context switches when producing, aquiring and committing.
    
    template <typename T, typename MutexT, typename SemaphoreT>
    class producer_consumer_queue<0, T, MutexT, SemaphoreT, true, true>
    {
        typedef atomic<size_t> atomic_size_t;
    private:
        T*                      avail_p_;
        T*                      process_p_;
        MutexT                  mutex_;
        SemaphoreT              sem_avail_;    
        SemaphoreT              sem_capacity_;
        atomic_size_t           avail_;
        char                    heap_[sizeof(T)];
        
    public:
        producer_consumer_queue() 
        : avail_p_(NULL), process_p_(NULL), sem_avail_(0), sem_capacity_(1), avail_(0)
        {
        }
        
        ~producer_consumer_queue() {
        }
        
        size_t capacity() const { return 0; }
        
        
        // Add an object 'object' to the queue which shall become available for 
        // consuming. When the function enters, it increments avail (and incre-
        // ments size).
        // The function shall block until after size is less or equal than capa-
        // city.
        // Returns true if the object has become available and can be consumed. 
        // Otherwise if it returns false, this may indicate a timeout. If the 
        // result is false, it must be guaranteed that object will never be 
        // consumed.
        bool produce(const T& object, double t = SemaphoreT::wait_forever()) 
        {
            //++avail_;
            bool accepted;
            if (sem_capacity_.wait(t)) {
                mutex_.lock();
                assert(avail_p_ == NULL);
                new (heap_) T(object);
                avail_p_ = reinterpret_cast<T*>(heap_);
                mutex_.unlock();   
                sem_avail_.signal();
                
                // wait again for capacity to become available:
                if (sem_capacity_.wait(t)) {
                    // ok, capacity is available, release it and go to retrieve the next object:
                    sem_capacity_.signal();
                    accepted = true;
                } 
                else 
                {
                    mutex_.lock();
                    // is my object still there?
                    if (avail_p_ and *avail_p_ == object) {
                        avail_p_->~T();
                        avail_p_ = NULL;
                        //--avail_;
                        accepted = false;
                    }                    
                    mutex_.unlock();   
                }
            } else {
                //--avail_;
                accepted = false;
            }
            
            return accepted;
        }
        
        
        // Returns a pointer to the next available object from the queue. If an object 
        // is available, it will be immediately returned. Otherwise, the calling thread 
        // will be blocked until one object becomes available, or a timeout occurs. If a 
        // timeout occurred, aquire will return NULL.
        // If aquire returns an object, processing will be incremented and avail will be
        // decremented.
        T* aquire(double t = SemaphoreT::wait_forever()) 
        {
            // wait until buffer avail
            T* result = NULL;
            if (sem_avail_.wait(t)) {
                mutex_.lock();
                --avail_;            
                process_p_ = avail_p_;
                avail_p_ = NULL;
                result = process_p_;
                mutex_.unlock();
            }
            return result;
        }
        
        
        // commit() shall be send to the receiver from the consumer when it has finished 
        // processing the object. On exit, processing and size will be decremented, and
        // object will be released by the receiver.
        // object shall be previously aquired by a consumer, and commit must be called
        // only once for this object, otherwise the behavior is undefined.
        void commit(const T* object) 
        {
            mutex_.lock();
            bool didCommit = true;
            if (object == process_p_) {
                object->~T();
                process_p_ = 0;
            }
            else {
                didCommit = false;
            }
            mutex_.unlock();
            if (didCommit) {
                sem_capacity_.signal();
            }
        }
        
        
        // Returns the number of objects currently processed by consumers. Note that the
        // result may be possibly stale immediately after the method returns.
        size_t processing() const {
            return process_p_ ? 1 : 0;
        }
        
        
        // Returns the number of objects which still need to be processed. Note 
        // that the result may be possibly stale immediately after the method 
        // returns.
        size_t avail() const {
            return avail_;
        }
        
        
        // Returns the sum of the available objects and objects currently processed. 
        // Note that the result may be possibly stale immediately after the method 
        // returns.
        size_t size() const {
            return static_cast<size_t>(avail_) + process_p_ ? 1 : 0;
        }
        
                
        // Returns true if avail equals zero and processing equals zero. Note that the
        // result may be possibly stale immediately after the method returns.
        bool empty() const {
            return (static_cast<size_t>(avail_) == 0 and process_p_ == 0);
        }
        
        
        // Clears the avail queue.
        void clear() {
            T* object;
            while ((object = aquire(0.0)) != NULL) {
                commit(object);
            }
        }
        
        
        // Commits any remaining uncommitted objects.
        bool commitAll() {
            mutex_.lock();
            bool didCommit = true;
            if (process_p_) {
                process_p_->~T();
                process_p_ = NULL;
            }
            else {
                didCommit = false;
            }
            mutex_.unlock();
            if (didCommit) {
                sem_capacity_.signal();
            }
            return didCommit;
        }
        
    };
    
    
    
    

}}
#endif // JSON_UTILITY_PRODUCER_CONSUMER_QUEUE2_HPP

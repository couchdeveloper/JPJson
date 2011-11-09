//
//  synchronous_queue.hpp
//  
//
//  Created by Andreas Grosam on 5/18/11.
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

#ifndef JSON_UTILITY_SYNCHRONOUS_QUEUE_HPP
#define JSON_UTILITY_SYNCHRONOUS_QUEUE_HPP


#include <boost/utility.hpp>
#include <utility>
#include <iostream>
#include <semaphore.h>

#include "json/objc/mutex.hpp"
#include "json/objc/semaphore.hpp"


// TODO: remove json/objc/* dependencies


namespace json { namespace utility {
    
    using json::objc::gcd::mutex;
    using json::objc::gcd::semaphore;
    
    
    //
    //  synchronous_queue   
    //                      
    //                      
    //
    
    //
    //  A synchronous queue is a concurrent transfer channel where producers
    //  wait for consumers and vice versa.
    //  A synchronous queue does not have any internal capacity, not even a 
    //  capacity of one. 
    
    //
    //  Models a "synchronous queue":
    //  
    //  T b = q.get()
    //  std::pair<result_type, T> r = q.get(timeout)
    //  q.put(b)
    //  result_type r = q.put(b, timeout)
    //
    //  Types
    //  value_type
    
    template <typename T>
    class synchronous_queue : boost::noncopyable {
    public:
        
        typedef T value_type;
        
        enum result_type {
            OK = 0,
            TIMEOUT_NOT_DELIVERED = -1,
            TIMEOUT_NOT_PICKED = -2,
            TIMEOUT_NOTHING_OFFERED = -3
        };
        
        
        synchronous_queue() 
        : sync_(0), send_(1), recv_(0), empty_(true)
        {
        }
        
        ~synchronous_queue() {
            if (not empty_) {
#if defined (DEBUG)                
                std::cerr << "WARNING: synchronous_queue buffer not commited while in dtor: " 
                    << *reinterpret_cast<T const*>(storage_) << std::endl; 
#endif                
                reinterpret_cast<T*>(storage_)->~T();
                
            }
        }
        
        void put(const T& v) {
            send_.wait();
            assert(empty_);
            new (storage_) T(v);
            empty_ = false;
            recv_.signal();
            sync_.wait();
        }
        
#if !defined (BOOST_NO_RVALUE_REFERENCES)
        void put(T&& v) {
            send_.wait();
            assert(empty_);
            new (storage_) T(std::move(v));
            empty_ = false;
            recv_.signal();
            sync_.wait();
        }
#endif        
        
        result_type put(const T& v, double timeout) {
            if (send_.wait(timeout)) {
                assert(empty_);
                new (storage_) T(v);
                empty_ = false;
                recv_.signal();
                if (sync_.wait(timeout)) {
                    return OK;
                }
                else {
                    return TIMEOUT_NOT_PICKED;
                }
            }
            else {
                return TIMEOUT_NOT_DELIVERED;
            }        
        }
        
#if !defined (BOOST_NO_RVALUE_REFERENCES)
        result_type put(T&& v, double timeout) {
            if (send_.wait(timeout)) {
                assert(empty_);
                new (storage_) T(std::move(v));
                empty_ = false;
                recv_.signal();
                if (sync_.wait(timeout)) {
                    return OK;
                }
                else {
                    return TIMEOUT_NOT_PICKED;
                }
            }
            else {
                return TIMEOUT_NOT_DELIVERED;
            }        
        }
#endif        
        
        
        
        
        T get() {
            recv_.wait();
            assert(not empty_);
            T& v = *reinterpret_cast<T*>(storage_);
#if !defined (BOOST_NO_RVALUE_REFERENCES)
            T result = std::move(v);            
#else                        
            T result = v;
#endif            
            reinterpret_cast<T*>(storage_)->~T();            
            empty_ = true;
            sync_.signal();
            send_.signal();
            return result;
        }
        
        std::pair<result_type, T> get(double timeout) {
            if (recv_.wait(timeout)) {
                assert(not empty_);
                T& v = *reinterpret_cast<T*>(storage_);
                std::pair<result_type, T> result = 
#if !defined (BOOST_NO_RVALUE_REFERENCES)
                    std::pair<result_type, T>(OK, std::move(v));
#else                        
                    std::pair<result_type, T>(OK, v);
#endif                            
                reinterpret_cast<T*>(storage_)->~T();
                empty_ = true;
                sync_.signal();
                send_.signal();
                return result;
            }
            else {
                return std::pair<result_type, T>(TIMEOUT_NOTHING_OFFERED, T());
            }
        }
        
        
        // Acquire an item. acquire() must be followed a commit() or give().
        T acquire() {
            recv_.wait();
            assert(not empty_);
            return *reinterpret_cast<T*>(storage_);        
        }
        
        std::pair<result_type, T> acquire(double timeout) {
            if (recv_.wait(timeout)) {
                std::pair<result_type, T> result = 
                    std::pair<result_type, T>(OK, *reinterpret_cast<T*>(storage_));
                return result;
            }
            else {
                return std::pair<result_type, T>(TIMEOUT_NOTHING_OFFERED, T());
            }
        }    
        
        // Give back a previously acquired item.
        // The behavior is undefined if no previous call of acquire() occured.
        void give() {
            assert(not empty_);
            if (empty_) {
                return;
            }
            recv_.signal();
        }
        
        // Commit a previously acquired item.
        // The behavior is undefined if no previous call of acquire() occured.
        void commit() {
            assert(not empty_);
            if (empty_) {
                return;
            }
            reinterpret_cast<T*>(storage_)->~T();
            empty_ = true;
            sync_.signal();
            send_.signal();
        }
        
        bool empty() const { return empty_; }
        
        
    private:
        char        storage_[sizeof(T)];
        semaphore   sync_;
        semaphore   send_;
        semaphore   recv_;
        bool        empty_;
    };
    
    
}}  // namespace json::utility


#endif

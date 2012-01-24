//
//  CFDataBuffers.hpp
//
//  Created by Andreas Grosam on 6/23/11.
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

#ifndef JSON_OBJC_CFDATA_BUFFERS_HPP
#define JSON_OBJC_CFDATA_BUFFERS_HPP


#include "json/config.hpp"
#import <CoreFoundation/CoreFoundation.h>
#include "semaphore.hpp"
#include "mutex.hpp"

#include <boost/config.hpp>

#include <list>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iomanip>



//#define JSON_OBJC_CFDATA_BUFFERS_HPP_LOG_DEBUG


#if defined (JSON_OBJC_CFDATA_BUFFERS_HPP_LOG_DEBUG)
#include <iostream>
#endif

namespace json { namespace objc {
    
    // CFDataBuffer implements an immutable buffer whose internal memory 
    // block can be shared among other CFDataBuffer instances.
    //
    // Models the "shared buffer" concept. 
    //
    
    template <typename T>
    class CFDataBuffer {
    public: 
        
        typedef T                   value_type;        
        typedef T*                  pointer;
        typedef T const*            const_pointer;
        
        // Default Ctor
        CFDataBuffer() 
        : data_(NULL), buffer_(NULL), size_(0) 
        {}  
        
        // Construct from an CFData object. The CFData object will be shared.
        // If data's size equals zero, the data object will not be retained.
        CFDataBuffer(CFDataRef data) 
        {
            assert(data == NULL || CFDataGetLength(data) % sizeof(T) == 0);
            if (data == nil or (size_ = CFDataGetLength(data))/sizeof(T) == 0) {
                data_ = NULL;
                size_ = 0;
                buffer_ = 0;
            }
            else 
            {
                data_ = static_cast<CFDataRef>(CFRetain(data));
                buffer_ = reinterpret_cast<T const*>(CFDataGetBytePtr(data));
#if defined (JSON_OBJC_CFDATA_BUFFERS_HPP_LOG_DEBUG) 
                NSLog(@"CFDataBuffer: retained CFData(%p), retain count: %d , buffer: %p, size: %ld", 
                      data_, (int)CFGetRetainCount(data_), buffer_, size_);
#endif                
            }            
        }
        
        // Construct from a pointer to a vector of len elements. This creates
        // a CFData object internally and copies the bytes from the given vector.
        CFDataBuffer(const T* v, size_t len)
        {
            if (v and len > 0) {
                data_ = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(v), sizeof(T)*len);
                size_ = len;
                buffer_ = reinterpret_cast<T const*>(CFDataGetBytePtr(data_));
            } else {
                data_ = NULL;
                buffer_= NULL;
                size_ = 0;                
            }
        }
        
        // Copy constructor.
        // The internal buffer will be shared.
        CFDataBuffer(const CFDataBuffer& other) 
        : data_( static_cast<CFDataRef>(other.data_ != NULL ? CFRetain(other.data_) : NULL) ),
          size_(other.size_),
          buffer_(other.buffer_)
        {        
        }
        
#if !defined (BOOST_NO_RVALUE_REFERENCES)
        // Move constructor
        CFDataBuffer(CFDataBuffer&& other) throw() 
        : data_(other.data_), size_(other.size_), buffer_(other.buffer_)
        { 
            other.buffer_ = 0;
            other.data_ = 0;
            other.size_ = 0;
        }
        
        // Move assignment operator
        CFDataBuffer& operator=(CFDataBuffer&& other) throw() {
            if (this != &other) {
                data_ = other.data_;
                buffer_ = other.buffer_;
                size_ = other.size_;
                other.data_ = 0;
                other.buffer_ = 0;
                other.size_ = 0;
            }
            return *this;
        }        
#endif        
        
        
        ~CFDataBuffer() { 
            if (data_)
                CFRelease(data_);
        }
        
        
        const T*    data() const { return buffer_; }
        size_t      size() const { return size_; }
        
        void        seek(size_t pos) {
            pos = std::min(CFDataGetLength(data_)/sizeof(T), pos);
            buffer_ = reinterpret_cast<T const*>(CFDataGetBytePtr(data_));
            buffer_ += pos;
            size_ -= pos;
        }
        
        void swap(CFDataBuffer& other) {
            std::swap(data_, other.data_);
            std::swap(buffer_, other.buffer_);
            std::swap(size_, other.size_);
        }
        
        CFDataBuffer& operator= (const CFDataBuffer& other) {
            CFDataBuffer tmp(other);
            tmp.swap(*this);
            return *this;
        }
        
        void release() {
            if (data_) {
#if defined (JSON_OBJC_CFDATA_BUFFERS_HPP_LOG_DEBUG) 
                NSLog(@"CFDataBuffer: releasing CFData(%p), retain count: %d , buffer: %p, size: %ld", 
                      data_, (int)CFGetRetainCount(data_), buffer_, size_);
#endif                
                CFRelease(data_);
                data_ = 0;
            }
            buffer_ = 0;
            size_ = 0;
        }
        
        long ref_count() const {
            if (data_)
                return CFGetRetainCount(data_);
            else
                return 0;        
        }
        
        CFDataRef get_CFDataRef() const { return data_; }
        
    private:
        CFDataRef   data_;
        const T*    buffer_;
        size_t      size_;
        
        
        friend inline 
        bool        
        operator== (const CFDataBuffer& lhv, const CFDataBuffer& rhv) {
            return lhv.buffer_ == rhv.buffer_;
        }
        friend inline 
        bool        
        operator!= (const CFDataBuffer& lhv, const CFDataBuffer& rhv) {
            return not (lhv.buffer_ == rhv.buffer_);
        }
        
        friend inline 
        std::ostream&        
        operator<< (std::ostream& os, const CFDataBuffer& buffer) {
            os << "CFDataRef (0x" << std::hex << buffer.data_ 
            << "), buffer (0x" << static_cast<void const*>(buffer.buffer_) <<  "), size: " << std::dec << buffer.size_;
            return os;
        }
        
        
    };
    
    
    template <typename T>
    void swap(CFDataBuffer<T>& lhv, CFDataBuffer<T>& rhv) {
        lhv.swap(rhv);
    }
    
    
}} // namespace json::objc











#endif // JSON_OBJC_CFDATA_BUFFERS_HPP



//
//  NSStreamStreambuf.hpp
//  
//  Created by Andreas Grosam on 05.10.12.
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

#ifndef JSON_OBJC_NSSTREAM_STREAMBUF_HPP
#define JSON_OBJC_NSSTREAM_STREAMBUF_HPP

#include "json/config.hpp"
#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/iostreams/categories.hpp>  // source_tag
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/ref.hpp>
#include <stdexcept>
#include <iosfwd>                          // streamsize

#include <boost/iostreams/device/file.hpp>

#import <Foundation/Foundation.h>


#define NSSTREAMSTREAMBUF_NO_EXCEPTIONS

namespace io = boost::iostreams;

namespace json { namespace objc { namespace internal {
    

    // Note:
    // By definition, boost::stream_buffer requires a device which is
    // CopyConstructable. This is not the case for NSInputStreamSource. Thus,
    // a boost::stream_buffer must be constructed passing a boost::ref.
    
    class NSInputStreamSource : public io::source
    {
    private:
        BOOST_MOVABLE_BUT_NOT_COPYABLE(NSInputStreamSource)
        
    public:
        // c-tor
        explicit NSInputStreamSource(NSInputStream* ns_input_stream)
        : _ns_input_stream(([ns_input_stream streamStatus] == NSStreamStatusOpen) ? ns_input_stream : nil),
          _consumed(0)
        {
            if (_ns_input_stream == nil) throw std::invalid_argument("invalid input stream or nil (stream must be openend)");
            [_ns_input_stream retain];
        }
                
        // d-tor
        ~NSInputStreamSource() {
            [_ns_input_stream release];
        }
        
        // Move ctor
        NSInputStreamSource(BOOST_RV_REF(NSInputStreamSource) other)
        :  _ns_input_stream(other._ns_input_stream),
           _consumed(other._consumed)
        {
            other._ns_input_stream = nil;
        }
        
        // Move assign
        NSInputStreamSource& operator=(BOOST_RV_REF(NSInputStreamSource) other)
        {
            _ns_input_stream = other._ns_input_stream;
            _consumed = other._consumed;
            other._ns_input_stream = nil;
            return *this;
        }
        
        
        
        // Read up to n characters from the underlying data source into the
        // buffer s.
        // Returns: the number of characters read or -1 to indicate EOF.
        std::streamsize read(char* s, std::streamsize n)
        {
            NSInteger amount = [_ns_input_stream read:reinterpret_cast<uint8_t*>(s)
                                            maxLength:static_cast<NSUInteger>(n)];
            if (amount > 0) {
                _consumed += amount;
                return amount;
            }
            else if (amount == 0) {
                return -1; // end of buffer reached - return EOF
            }
            else {
                //throw std::runtime_error("NSInputStream operation read failed");
                return 0;
            }
        }
        
        bool is_open() const {
            NSStreamStatus status = [_ns_input_stream streamStatus];
            return status == NSStreamStatusOpen or status == NSStreamStatusOpening;
        }
        
        void close() {
            [_ns_input_stream close];
        }
        
        std::size_t consumed() const {
            return _consumed;
        }
        
        NSError* error() const {
            return [_ns_input_stream streamError];
        }
        
    private:
        NSInputStream* _ns_input_stream;
        std::size_t _consumed;
    };
    
    
    class NSInputStreamSource2 : public io::source
    {
    private:
        BOOST_COPYABLE_AND_MOVABLE(NSInputStreamSource2)
    public:
        // c-tor
        explicit NSInputStreamSource2(NSInputStream* ns_input_stream)
        : _ns_input_stream(([ns_input_stream streamStatus] == NSStreamStatusOpen) ? ns_input_stream : nil),
          _consumed(0)
        {
            if (_ns_input_stream == nil) throw std::invalid_argument("invalid input stream or nil (stream must be openend)");
            [_ns_input_stream retain];
        }
        
        NSInputStreamSource2(const NSInputStreamSource2& other)
        : _ns_input_stream(other._ns_input_stream),
          _consumed(other._consumed)
        {
            [_ns_input_stream retain];
        }
        
        // d-tor
        ~NSInputStreamSource2() {
            //printf("~NSInputStreamSource2\n");
            [_ns_input_stream release];
        }
        
        // Move ctor
        NSInputStreamSource2(BOOST_RV_REF(NSInputStreamSource2) other)
        :  _ns_input_stream(other._ns_input_stream),
           _consumed(other._consumed)
        {
            other._ns_input_stream = nil;            
        }
        
        // Move assign
        NSInputStreamSource2& operator=(BOOST_RV_REF(NSInputStreamSource2) other)
        {
            _ns_input_stream = other._ns_input_stream;
            _consumed = other._consumed;
            other._ns_input_stream = nil;
            return *this;
        }
        
        // Copy assign
        NSInputStreamSource2& operator=(BOOST_COPY_ASSIGN_REF(NSInputStreamSource2) other)
        {
            if (&other != this) {
                [_ns_input_stream release];
                _ns_input_stream = other._ns_input_stream;
                [_ns_input_stream retain];
                _consumed = other._consumed;
            }
            return *this;
        }
        
        
        // Read up to n characters from the underlying data source into the
        // buffer s.
        // Returns: the number of characters read or -1 to indicate EOF.
        std::streamsize read(char* s, std::streamsize n)
        {
            NSInteger amount = [_ns_input_stream read:reinterpret_cast<uint8_t*>(s)
                                            maxLength:static_cast<NSUInteger>(n)];
            if (amount > 0) {
                _consumed += amount;
                return amount;
            }
            else if (amount == 0) {
                return -1; // end of buffer reached - return EOF
            }
            else {
                //throw std::runtime_error("NSInputStream operation read failed");
                return 0;
            }
        }
        
        bool is_open() const {
            NSStreamStatus status = [_ns_input_stream streamStatus];
            return status == NSStreamStatusOpen or status == NSStreamStatusOpening;
        }
        
        std::size_t consumed() const {
            return _consumed;
        }
        
        void close() {
            [_ns_input_stream close];
        }
        
        NSError* error() const {
            return [_ns_input_stream streamError];
        }
        
    private:
        NSInputStream* _ns_input_stream;
        std::size_t _consumed;
    };
}}}  // json::objc::internal



namespace json { namespace objc { namespace internal {
    
    namespace io = boost::iostreams;
    
    
    class NSOutputStreamSink : public io::sink
    {
    private:
        BOOST_MOVABLE_BUT_NOT_COPYABLE(NSOutputStreamSink)
        
    public:
        // c-tor
        explicit NSOutputStreamSink(NSOutputStream* ns_output_stream)
        : _ns_output_stream(([ns_output_stream streamStatus] == NSStreamStatusOpen) ? ns_output_stream : nil),
          _written(0)
        {
            if (_ns_output_stream == nil) throw std::invalid_argument("invalid input stream or nil (stream must be openend)");
            [_ns_output_stream retain];
        }
        
        // d-tor
        ~NSOutputStreamSink() {
            [_ns_output_stream release];
        }
        
        // Move ctor
        NSOutputStreamSink(BOOST_RV_REF(NSOutputStreamSink) other)
        :  _ns_output_stream(other._ns_output_stream),
           _written(other._written)
        {
            other._ns_output_stream = nil;
        }
        
        // Move assign
        NSOutputStreamSink& operator=(BOOST_RV_REF(NSOutputStreamSink) other)
        {
            _ns_output_stream = other._ns_output_stream;
            _written = other._written;
            other._ns_output_stream = nil;
            return *this;
        }
        
        // Write up to n characters to the underlying data sink into the
        // buffer s.
        // Returns: the number of characters written.
        std::streamsize write(const char* s, std::streamsize n)
        {
            const uint8_t* buffer = reinterpret_cast<const uint8_t*>(s);
            NSInteger amount = [_ns_output_stream write:buffer
                                              maxLength:static_cast<NSUInteger>(n)];
            if (amount >= 0) {
                _written += amount;
                return amount;
            }
            else {
                //throw std::runtime_error("NSOutputStream operation write failed");
                return 0;
            }
        }

        bool is_open() const {
            NSStreamStatus status = [_ns_output_stream streamStatus];
            return status == NSStreamStatusOpen or status == NSStreamStatusOpening;
        }
        
        std::size_t written() const {
            return _written;
        }
        
        void open() {
            [_ns_output_stream open];
        }
        
        void close() {
            [_ns_output_stream close];
        }
        
        NSError* error() const {
            return [_ns_output_stream streamError];
        }
        
    private:
        NSOutputStream* _ns_output_stream;
        std::size_t _written;
    };
    
    
    class NSOutputStreamSink2 : public io::sink
    {
    private:
        BOOST_COPYABLE_AND_MOVABLE(NSOutputStreamSink2)
        
    public:
        // c-tor
        explicit NSOutputStreamSink2(NSOutputStream* ns_output_stream)
        : _ns_output_stream(([ns_output_stream streamStatus] == NSStreamStatusOpen) ? ns_output_stream : nil),
          _written(0)
        {
            if (_ns_output_stream == nil) throw std::invalid_argument("invalid input stream or nil (stream must be open)");
            [_ns_output_stream retain];
        }
        
        NSOutputStreamSink2(const NSOutputStreamSink2& other)
        : _ns_output_stream(([other._ns_output_stream streamStatus] == NSStreamStatusOpen) ? other._ns_output_stream : nil),
          _written(0)
        {
            if (_ns_output_stream == nil) throw std::invalid_argument("invalid input stream or nil (stream must be openend)");
            [_ns_output_stream retain];
        }
        
        // d-tor
        ~NSOutputStreamSink2()
        {
            [_ns_output_stream release];
        }
        
        // Move ctor
        NSOutputStreamSink2(BOOST_RV_REF(NSOutputStreamSink2) other)
        :  _ns_output_stream(other._ns_output_stream),
           _written(other._written)
        {
            other._ns_output_stream = nil;
        }
        
        // Move assign
        NSOutputStreamSink2& operator=(BOOST_RV_REF(NSOutputStreamSink2) other)
        {
            _ns_output_stream = other._ns_output_stream;
            _written = other._written;
            other._ns_output_stream = nil;
            return *this;
        }
        
        // Copy assign
        NSOutputStreamSink2& operator=(BOOST_COPY_ASSIGN_REF(NSOutputStreamSink2) other)
        {
            if (&other != this) {
                [_ns_output_stream release];
                _ns_output_stream = other._ns_output_stream;
                _written = other._written;
                [_ns_output_stream retain];
            }
            return *this;
        }
        
        
        
        // Write up to n characters to the underlying data sink into the
        // buffer s.
        // Returns: the number of characters written.
        std::streamsize write(const char* s, std::streamsize n)
        {
            const uint8_t* p = reinterpret_cast<const uint8_t*>(s);
            NSInteger written;
            NSUInteger remaining = n;
            
            while (1) {
                written = CFWriteStreamWrite((CFWriteStreamRef)_ns_output_stream, p, remaining);
                //written = [_ns_output_stream write:p maxLength:remaining];
                if (written > 0) {
                    p += written;
                    _written += written;
                    remaining -= written;
                    if (remaining) {
                        continue;
                    }
                    break;
                }
            }
            if (remaining == 0) {
                return n;
            }
            if (written == 0) {
                return n-remaining;
            }                
            else {
                //throw std::runtime_error("NSOutputStream operation write failed");
                return 0;
            }
        }
        
        bool is_open() const {
            NSStreamStatus status = [_ns_output_stream streamStatus];
            return status == NSStreamStatusOpen or status == NSStreamStatusOpening;
        }
        
        std::size_t written() const {
            return _written;
        }
        
        void open() {
            [_ns_output_stream open];
        }
        
        void close() {
            [_ns_output_stream close];
        }
        
        NSError* error() const {
            return [_ns_output_stream streamError];
        }
        
    private:
        NSOutputStream* _ns_output_stream;
        std::size_t _written;
    };
    
}}}


namespace json { namespace objc {
    
    typedef io::stream_buffer<boost::reference_wrapper<internal::NSInputStreamSource> >  NSInputStreamStreambuf;
    typedef io::stream_buffer<boost::reference_wrapper<internal::NSOutputStreamSink> >  NSOutputStreamStreambuf;

    typedef io::stream_buffer<internal::NSInputStreamSource2>  NSInputStreamStreambuf2;
    typedef io::stream_buffer<internal::NSOutputStreamSink2>  NSOutputStreamStreambuf2;

}}


#endif  // JSON_OBJC_NSSTREAM_STREAMBUF_HPP

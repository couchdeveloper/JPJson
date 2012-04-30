//
//  number_string_buffer.hpp
//  
//
//  Created by Andreas Grosam on 4/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_INTERNAL_NUMBER_STRING_BUFFER0_HPP
#define JSON_INTERNAL_NUMBER_STRING_BUFFER0_HPP


#include "json/config.hpp"
#include "json/unicode/unicode_traits.hpp"

#include <cstring>
#include <assert.h>

#include <stdexcept>
#include <utility>

#include <boost/utility.hpp>


namespace json { namespace parser_internal {
    
    
    
    
    
    struct string_buffer_overflow_error : public std::runtime_error
    {
        string_buffer_overflow_error() : std::runtime_error("numberbuilder: string buffer too small") 
        {}
    };

    
    
    template <size_t Size>
    class number_string_buffer : boost::noncopyable 
    {
    public:
        typedef char                                    code_unit_t;
        typedef typename std::pair<char*, size_t>       buffer_type;        
        typedef typename std::pair<char const*, size_t> const_buffer_type;        
        
    public:
        number_string_buffer() 
        : end_(buffer_)
        {}
        
        void reset() {
            end_ = buffer_;
        }
        
        void clear() {
            end_ = buffer_;
        }
        
        size_t capazity() const { return Size; }            
        size_t size() const     { return static_cast<size_t>(end_ - buffer_); };
        size_t avail() const    { return static_cast<size_t>(buffer_ + sizeof(buffer_) - end_); };
        
        void append_ascii(char ch)  { 
            assert(ch <= 0x7F);
            if (__builtin_expect(avail() == 0, 0)) {
                number_string_buffer::throw_overflow_error();
            }
            *end_++ = ch;
        }
        
        void terminate_if() {
            if (size() == 0 or *(end_-1) != 0) {
                *end_++ == 0;
            }
        }
        
        const_buffer_type   const_buffer() const    { return const_buffer_type(buffer_, size()); }
        buffer_type         buffer() const          { return buffer_type(buffer_, size()); }
        
        
    private:
        static void throw_overflow_error() {
            throw string_buffer_overflow_error();
        }
        
    private:
        code_unit_t* end_;
        code_unit_t buffer_[Size];
    };
    
    
}}



#endif // JSON_INTERNAL_NUMBER_STRING_BUFFER0_HPP

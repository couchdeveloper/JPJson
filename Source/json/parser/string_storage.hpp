//
//  string_storage.hpp
//  
//
//  Created by Andreas Grosam on 4/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_INTERNAL_STRING_STORAGE0_HPP
#define JSON_PARSER_INTERNAL_STRING_STORAGE0_HPP


#include "json/unicode/unicode_traits.hpp"
#include "json/endian/endian.hpp"


#include <memory>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <limits>


#include <boost/utility.hpp>
#include <boost/type_traits.hpp>




namespace json { namespace parser_internal {
    
    
    //  Models string_storage:
    //
    //  void append(buffer_type const& buffer);
    //  void append(code_unit_type code_unit);
    //  buffer_type buffer() const;
    //  void reset();
    //  size_t size() const;
    //  void extend(size_t size);
    //
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::encoding_traits;
    using json::internal::host_endianness;
    using json::unicode::add_endianness;
    using json::unicode::converter;
    

    template <typename EncodingT>
    class string_storage : boost::noncopyable
    {
    public:        
        typedef EncodingT                                               encoding_type;
        typedef typename encoding_traits<EncodingT>::code_unit_type     code_unit_type;
        typedef typename std::pair<code_unit_type*, std::size_t>        buffer_type;        
        typedef typename std::pair<code_unit_type const*, std::size_t>  const_buffer_type;        
        typedef code_unit_type*                                         storage_pointer;
        
        
    public:
        string_storage() 
        :   storage_start_(0), storage_end_(0), storage_cap_(0)
        {
        }
        
        string_storage(std::size_t initial_storage_capacity) 
        {
            storage_end_ = storage_start_ = (code_unit_type*)malloc(initial_storage_capacity*sizeof(code_unit_type));
            storage_cap_ = storage_start_ + initial_storage_capacity;   
        }
        
        ~string_storage() {
            free(storage_start_);
        }
                
        // Extend the buffer in order to hold additional 'size' code units. The
        // buffer might possibly grow in order to hold at least 'size' code 
        // units, and it might possibly sync the current buffer.
        void extend(size_t size) 
        {
            // If the available code units in the buffer are equal or greater
            // than the requested size do nothing.
            // Otherwise, if the current size of the buffer is greater or equal 
            // than MIN_STRING_SIZE and synching is enabled perform a sync and 
            // grow the buffer if required. Otherwise, if the size of the buffer 
            // is smaller than MIN_STRING_SIZE grow the buffer by at least the 
            // requested size.
            // In any case, guarantee the buffer can hold the requested number 
            // of code units.
            //
            // Synching the buffer writes the current characters from the buffer 
            // to the underlaying Semantic Actions object as a "partial string" 
            // and then restes the current buffer.
            //
            // Synching requires that we take care of the boundaries of a possibly
            // multi byte sequence. 
            // 
            // The current implementation simply does not split any partial strings 
            // by itself, but just takes them as they come in.
            
            if (__builtin_expect(size <= avail(), 1)) {
                return;
            }
            else {
                extend_priv(size);
            }
        }
        
        
        // Append the content of the string buffer 'buffer' to the current string 
        // buffer on top of the stack. A string buffer always starts at the 
        // beginning of a character and ends with a complete character. When
        // synching we must take care of these boundaries, that is we cannot
        // arbitrarily cut a possibly multi byte sequence.
        void append(const_buffer_type const& buffer)
        {
            extend(buffer.second);
            storage_end_ = std::uninitialized_copy(buffer.first, buffer.first + buffer.second, storage_end_);
            assert(storage_end_ <= storage_cap_);
        }
        
        // Append the code unit 'code_unit' to the current string buffer on top of the stack
        void append(code_unit_type code_unit)
        {
            if (__builtin_expect(storage_end_ == storage_cap_, 0)) {
                extend(1);
            }
            *storage_end_++ = code_unit;
        }
        
        // Return the current buffer
        const_buffer_type buffer() const 
        {
            return const_buffer_type(storage_start_, size());
        }
        
        // Return the current buffer on top of the stack
        buffer_type buffer() 
        {
            return buffer_type(storage_start_, size());
        }
        
        // Resets the current buffer, that is its size becomes zero.
        // The original content - that is, content past the new end pointer will
        // not be invalidated, unless it will be overridden or the storage needs
        // to be reallocated.
        void reset() {
            storage_end_ = storage_start_;
        }
        
        // Returns the size of the buffer
        size_t size() const {
            return static_cast<size_t>(storage_end_ - storage_start_);
        }
        
        // Returns a referene to the current end pointer of the buffer
        storage_pointer& dest() {
            return storage_end_;
        }
        
        storage_pointer data() { return storage_start_; }
        const storage_pointer data() const { return storage_start_; }
        
        
        // Reserve a storage size
        void reserve(size_t size) {
            if (capacity() < size) {
                storage_grow(size);
            }
        }
        
        size_t avail() const {
            return static_cast<size_t>(std::distance(storage_end_, storage_cap_));
        }
        
        size_t capacity() const {
            return std::distance(storage_start_, storage_cap_);
        }
        
        
        
    protected:                
        
        void storage_grow(size_t min_size) {
            size_t newcap = capacity();
            if (newcap == 0) {
                newcap = INITIAL_CAPACITY;
            }
            while (min_size > newcap) {
                newcap += std::min(newcap, size_t(MAX_GROW_SIZE));
            }
            if (newcap > capacity()) {
                size_t saved_size = size();
                code_unit_type* new_start = (code_unit_type*)malloc(newcap*sizeof(code_unit_type));
                std::uninitialized_copy(storage_start_, storage_cap_, new_start);
                free(storage_start_);
                storage_start_ = new_start;
                storage_end_ = storage_start_ + saved_size;
                storage_cap_ = storage_start_ + newcap;
            }
        }
        
        
        virtual void sync() = 0; 
        
        
    private:
        
        void extend_priv(size_t sz)
        {
            // Nore, when we reach here, the current buffer is full
            if (this->size() >= MAX_CAPACITY) {
                sync();
                if (sz > avail()) {                    
                    storage_grow(sz + size());
                }
            }
            else {
                storage_grow(sz + size());
            }
        }
        
    private:
        
        static const size_t MAX_GROW_SIZE = 16*1024;
        static const size_t INITIAL_CAPACITY = 4*1024;
        static const size_t MAX_CAPACITY =  32*1024;
        
        code_unit_type*         storage_start_;
        code_unit_type*         storage_end_;
        code_unit_type*         storage_cap_;
        
    };
    
}}



#endif  // JSON_PARSER_INTERNAL_STRING_STORAGE2_HPP

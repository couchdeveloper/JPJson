//
//  string_stack_storage.hpp
//  
//
//  Created by Andreas Grosam on 3/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_INTERNAL_STRING_STACK_STORAGE_HPP
#define JSON_PARSER_INTERNAL_STRING_STACK_STORAGE_HPP


#include "json/unicode/unicode_traits.hpp"
#include "json/endian/endian.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <assert.h>


#include <boost/static_assert.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/or.hpp>


namespace json { namespace parser_internal {
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::encoding_traits;
    using json::internal::host_endianness;
    using json::unicode::add_endianness;
    
    // Due to the recursive decent parser the JSON strings which are keys
    // in dictionaries need to be stacked. This can be accomplished by
    // using 
    // a)  a fixed size buffer with sufficient size allocated on the stack
    // b)  a dynamically allocated buffer per occurance
    // c)  a dynamically allocated buffer shared for all keys.
    //
    // There is a trade of between space and time complexity - where "space"
    // also means bytes allocated on the stack.
    
    
    // The subsequent class implements a string storage for JSON keys, which 
    // attempts to avoid any compromisses regarding space and time complexity.
    // It uses a shared "stack allocator" which is as fast as a fixed size buffer 
    // allocated on the stack but much more space efficent. 
    
    //  Models string_storage:
    //
    //  void append(buffer_type const& buffer);
    //  void append(code_unit_type code_unit);
    //  buffer_type buffer() const;
    //  void reset();
    //  size_t size() const;
    //  void extend(size_t size);
    //
    
    template <typename EncodingT>
    class string_stack_storage : boost::noncopyable
    {
        // Although not strictly required, EncodingT's endianness shall be
        // equal host endianness:
        BOOST_STATIC_ASSERT( (boost::is_same<
                                    typename encoding_traits<typename add_endianness<EncodingT>::type>::endian_tag,
                                    typename host_endianness::type
                              >::value == true)  );
        
    public:
        typedef EncodingT encoding_type;
        typedef typename encoding_traits<EncodingT>::code_unit_type     code_unit_type;
        typedef typename std::pair<code_unit_type*, std::size_t>        buffer_type;        
        typedef typename std::pair<code_unit_type const*, std::size_t>  const_buffer_type;        
        typedef code_unit_type*                                         storage_pointer;
                
    public:
        string_stack_storage() 
        : storage_start_(0), storage_end_(0), storage_cap_(0)
        {
        }

        string_stack_storage(std::size_t initial_storage_capacity)
        {
            storage_end_ = storage_start_ = (code_unit_type*)malloc(initial_storage_capacity*sizeof(code_unit_type));
            storage_cap_ = storage_start_ + initial_storage_capacity;   
            stack_.reserve(initial_storage_capacity/16);
        }
    
        ~string_stack_storage()
        {
            free(storage_start_);
        }
        
        
        void extend(size_t size) {
            if (size > storage_avail()) {
                storage_grow(size + storage_avail());
            }
        }
        
        // Append the content of the string buffer 'buffer' to the current string buffer on top of the stack
        void append(const const_buffer_type& buffer)
        {
            if (storage_avail() < buffer.second) {
                storage_grow(buffer.second + storage_capacity());
            }
            storage_end_ = std::uninitialized_copy(buffer.first, buffer.first + buffer.second, storage_end_);
            assert(storage_end_ <= storage_cap_);
        }

        // Append the code unit 'code_unit' to the current string buffer on top of the stack
        void append(code_unit_type code_unit)
        {
            if (storage_end_ == storage_cap_) {
                storage_grow(1 + storage_capacity());
            }
            *storage_end_++ = code_unit;
        }
        
        // Return the current buffer on top of the stack
        const_buffer_type buffer() const {
            if (stack_size()) {
                code_unit_type* p = storage_start_ + stack_.back();
                size_t size = static_cast<size_t>(std::distance(p, storage_end_));
                return const_buffer_type(p, size);
            }
            else {
                return (const_buffer_type(0,0));
            }
        }
        
        // Return the current buffer on top of the stack
        buffer_type buffer() {
            if (stack_size()) {
                code_unit_type* p = storage_start_ + stack_.back();
                size_t size = static_cast<size_t>(std::distance(p, storage_end_));
                return buffer_type(p, size);
            }
            else {
                return (buffer_type(0,0));
            }
        }
        
        // Resets the current buffer on top of the stack, that is its size becomes 
        // zero.
        // The original content - that is, content past the new end pointer will
        // not be invalidated, unless it will be overridden or the storage needs
        // to be reallocated.
        void reset() 
        {
            storage_end_ = storage_start_ + stack_.back();
        }
        
        // Returns the size of the buffer on top of the stack
        size_t size() const {
            return static_cast<size_t>(std::distance(storage_start_ + stack_.back(), storage_end_));
        }
        
        // Returns a referene to the current end pointer of the buffer on top of the stack.
        storage_pointer& dest() {
            return storage_end_;
        }
        
        // Pop a string buffer
        void stack_pop() {
            storage_end_ = storage_start_ + stack_.back();
            stack_.pop_back();
        }
        
        // Push an empty string buffer
        void stack_push()
        {
            stack_.push_back(storage_size());
        }
        
        // Push a string buffer
        void stack_push(const const_buffer_type& buffer)
        {
            if (storage_avail() < buffer.second) {
                storage_grow(buffer.second + storage_capacity());
            }
            stack_.push_back(storage_size());
            storage_end_ = std::uninitialized_copy(buffer.first, buffer.first + buffer.second, storage_end_);
            assert(storage_end_ <= storage_cap_);
        }
        
        // Return the size of the stack
        size_t stack_size() const { return stack_.size(); }
        
        
        
        // Reserve a storage size for the string buffers
        void storage_reserve(size_t size) {
            if (storage_capacity() < size) {
                storage_grow(size);
            }
        }
        
        size_t storage_avail() const {
            size_t result = static_cast<size_t>(std::distance(storage_end_, storage_cap_));
            return result;
        }
        
        size_t storage_capacity() const {
            return std::distance(storage_start_, storage_cap_);
        }
        
        size_t storage_size() const {
            return std::distance(storage_start_, storage_end_);
        }
        
    protected:        
        typedef int seq_type;
        typedef std::vector<seq_type> seq_stack_type;
        
        
        void storage_grow(size_t min_size) {
            size_t newcap = storage_capacity();
            if (newcap == 0) {
                newcap = INITIAL_CAPACITY;
                stack_.reserve(INITIAL_CAPACITY/16);
            }
            while (min_size > newcap) {
                newcap += std::min(newcap, size_t(MAX_GROW_SIZE));
            }
            if (newcap > storage_capacity()) {
                size_t saved_size = storage_size();
                code_unit_type* new_start = (code_unit_type*)malloc(newcap*sizeof(code_unit_type));
                std::uninitialized_copy(storage_start_, storage_cap_, new_start);
                free(storage_start_);
                storage_start_ = new_start;
                storage_end_ = storage_start_ + saved_size;
                storage_cap_ = storage_start_ + newcap;
            }
        }
        
        
    protected:
        
        static const size_t MAX_GROW_SIZE = 1024;
        static const size_t INITIAL_CAPACITY = 512;
        
        code_unit_type* storage_start_;
        code_unit_type* storage_end_;
        code_unit_type* storage_cap_;
        seq_stack_type stack_;
    };

}}



#endif  // JSON_PARSER_INTERNAL_STRING_STACK_STORAGE_HPP

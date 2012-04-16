//
//  string_chunk_storage.hpp
//  
//
//  Created by Andreas Grosam on 3/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_INTERNAL_STRING_CHUNK_STORAGE_HPP
#define JSON_PARSER_INTERNAL_STRING_CHUNK_STORAGE_HPP


#include "json/unicode/unicode_traits.hpp"
#include "json/unicode/unicode_converter.hpp"
#include "json/endian/byte_swap.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <assert.h>

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/not.hpp>
#include <boost/static_assert.hpp>

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
    
    using json::unicode::encoding_traits;
    using json::byte_swap;
    using json::unicode::UTF_32_encoding_tag;
    using json::internal::host_endianness;
    using json::unicode::add_endianness;
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::converter;
    
    
    
    template <typename EncodingT, typename SemanticActionsT>
    class string_chunk_storage : boost::noncopyable
    {
        // Although not strictly required, EncodingT shall have host endianness:
        BOOST_STATIC_ASSERT( (boost::is_same<
                                    typename encoding_traits<typename add_endianness<EncodingT>::type>::endian_tag,
                                    typename host_endianness::type
                              >::value == true)  );

        // For efficiency reasons, EncodingT shall be either UTF_32_encoding_tag in
        // host endianness, or it shall be equal the SemanticActionsT's encoding:
        // (With the rule above, this also implies, that SemanticActionsT's encoding 
        // must have host endianness as well)
        BOOST_STATIC_ASSERT((boost::mpl::or_<
                              boost::is_same<typename add_endianness<EncodingT>::type, typename add_endianness<UTF_32_encoding_tag>::type>,
                              boost::is_same<EncodingT, typename SemanticActionsT::encoding_t>
                             >::value == true));
        
        
    public:
        typedef EncodingT                                               encoding_type;
        typedef typename encoding_traits<EncodingT>::code_unit_type     code_unit_type;
        typedef typename std::pair<code_unit_type const*, size_t>       const_buffer_type;        
        typedef typename std::pair<code_unit_type*, size_t>             buffer_type;        
        typedef code_unit_type*                                         storage_pointer;
                
        typedef typename SemanticActionsT::encoding_t                   sa_encoding_type;
        
    private:
        string_chunk_storage();  // no default c-tor
        
    public:
        string_chunk_storage(SemanticActionsT& sa, std::size_t bufferSize = kDefaultBufferSize) 
        : sa_(sa)
        {
            storage_end_ = storage_start_ = static_cast<storage_pointer>(malloc(bufferSize*sizeof(code_unit_type)));
            storage_cap_ = storage_start_ + bufferSize;
        }
    
        ~string_chunk_storage() {
            free(storage_start_);
        }
        
        
        void extend(size_t size) {
            if (size > storage_avail()) {
                sync();
                if (size > storage_avail()) {
                    throw std::runtime_error("buffer too big");
                }
            }
        }
        
        // Append the content of the string buffer 'buffer' to the string buffer
        void append(const const_buffer_type& buffer)
        {
            if (storage_avail() < buffer.second) {
                sync();
                if (storage_avail() < buffer.second) {
                    throw std::runtime_error("buffer too big");
                }
            }
            storage_end_ = std::uninitialized_copy(buffer.first, buffer.first + buffer.second, storage_end_);
            assert(storage_end_ <= storage_cap_);
        }
        
        // Append the code unit 'code_unit' to the string buffer
        void append(code_unit_type code_unit)
        {
            if (storage_end_ == storage_cap_) {
                sync();
                assert(storage_avail() > 0);
            }
            *storage_end_++ = code_unit;
        }
        
        // Return the buffer
        const_buffer_type buffer() const {
            return const_buffer_type(storage_start_, size());
        }
        
        // Return the buffer
        buffer_type buffer() {
            return buffer_type(storage_start_, size());
        }
        
        // Returns the size of the string in the buffer in number of code units.
        size_t size() const {
            return static_cast<size_t>(std::distance(storage_start_, storage_end_));
        }
                
        
        // Resets the  buffer, that is its size becomes zero.
        // The original content - that is, content past the new end pointer will
        // not be invalidated, unless it will be overridden or the storage needs
        // to be reallocated.
        void reset() {
            storage_end_ = storage_start_;
        }
        
        // Returns a referene to the current end pointer of the buffer
        storage_pointer& dest() {
            return storage_end_;
        }
        
        
        size_t storage_avail() const {
            return static_cast<size_t>(std::distance(storage_end_, storage_cap_));
        }
        
        size_t storage_capacity() const {
            return std::distance(storage_start_, storage_cap_);
        }
        
        void flush() {
            this->write<encoding_type>(false);
            reset();
        }
        
    protected:
        
        void sync() {
            this->write<encoding_type>(true);
            reset();
        }
        
        
    private:
        
        struct write_policy_direct {};
        struct write_policy_inplace_convert {};
        
        template <typename E1, typename E2, class Enable = void>
        struct write_policy_traits {
            // Shall not be instantiated. Use different encodings to ensure 
            // efficient string buffers.
            BOOST_STATIC_ASSERT(sizeof(Enable)==0);  
        };
        
        template <typename E1, typename E2>
        struct write_policy_traits<E1, E2,
            typename boost::enable_if<
                boost::is_same<E1, E2>
            >::type
        > 
        {
            typedef write_policy_direct type;
        };
        
        
        template <typename E1, typename E2>
        struct write_policy_traits<E1, E2,
            typename boost::enable_if<
                boost::mpl::and_<
                    boost::is_same<typename encoding_traits<E1>::encoding_form, UTF_32_encoding_tag>,
                    boost::mpl::not_<
                        boost::is_same<typename encoding_traits<E2>::encoding_form, UTF_32_encoding_tag>
                    >
                >
            >::type
        > 
        {
            typedef write_policy_inplace_convert type;
        };
        
        

        
        

        // The string buffer's encoding and the semantic actions' encoding are
        // the same:
        template <typename E>
        void 
        write(bool hasMore, 
              typename boost::enable_if<
                 boost::is_same<typename write_policy_traits<E, sa_encoding_type>::type, write_policy_direct>    
                      >::type* dummy = 0) 
        {
            sa_.value_string_write(const_buffer_type(storage_start_, size()), hasMore);
        }
        
        // The string buffer's encoding can be converted "in place" to the 
        // semantic actions' encoding:
        template <typename E>
        void 
        write(bool hasMore,
              typename boost::enable_if<
                 boost::is_same<typename write_policy_traits<E, sa_encoding_type>::type, write_policy_inplace_convert>    
                 >::type* dummy = 0) 
        {
            typedef converter<E, sa_encoding_type, unicode::Validation::UNSAFE, unicode::Stateful::No, unicode::ParseOne::No> cvt_t;
            typedef typename encoding_traits<sa_encoding_type>::code_unit_type sa_char_t;            
            
            code_unit_type const* first = storage_start_;
            code_unit_type const* last = storage_end_;
            sa_char_t* start = reinterpret_cast<sa_char_t*>(static_cast<void*>(storage_start_));
            sa_char_t* dest = start;
#if defined (DEBUG)
            int cvt_result = 
#endif            
            cvt_t().convert(first, last, dest);
            assert(cvt_result == 0);
            size_t length = static_cast<size_t>(std::distance(start, dest));
            sa_.value_string_write(start, length, hasMore);
        }
        
        
    private:
        
        
        
    private:
        SemanticActionsT& sa_;
        storage_pointer storage_start_;
        storage_pointer storage_end_;
        storage_pointer storage_cap_;
        
        
        static const int kDefaultBufferSize = 1024;
    };
    
}}




#endif //JSON_PARSER_INTERNAL_STRING_CHUNK_STORAGE_HPP

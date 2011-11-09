//
//  unicode_iterator.hpp
//  
//
//  Created by Andreas Grosam on 7/25/11.
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

#error not yet implemented

#ifndef JSON_UNICODE_UNICODE_ITERATOR_HPP
#define JSON_UNICODE_UNICODE_ITERATOR_HPP

#include <boost/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include "json/endian/endian.hpp"
#include "json/endian/byte_swap.hpp"
#include "unicode_utilities.hpp"




namespace json { namespace unicode { namespace internal {
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::platform_encoding_tag;
    
    template <typename Iterator, typename FromEncodingT, typename ToEncodingT>
    struct iterator_state {        
    };
    

    template <typename Iterator, typename EncodingT>
    struct iterator_state<Iterator, EncodingT, EncodingT> {};
    
    // Stateful iterators:
    template <typename Iterator>
    class iterator_state<UTF_16_encoding_tag, UTF_8_encoding_tag> {
        typedef typename UTF_8_encoding_tag::code_unit_type value_type;
        typedef typename const value_type*                  const_iterator;
        
    public:    
        iterator_state() : size_(0), index_(-1) {}
            
    private:    
        UTF_8_encoding_tag::encode_buffer_type buffer_;
        short size_;
        short index_;
        Iterator iter_;
    };
    
    template <>
    struct iterator_state<UTF_32_encoding_tag, UTF_8_encoding_tag> {        
        typedef typename UTF_8_encoding_tag::code_unit_type value_type;
        
        UTF_8_encoding_tag::encode_buffer_type buffer_;
    };
    
    template <>
    struct iterator_state<platform_encoding_tag, UTF_8_encoding_tag> {        
        typedef typename UTF_8_encoding_tag::code_unit_type value_type;
        
        UTF_8_encoding_tag::encode_buffer_type buffer_;
    };
    
    template <>
    struct iterator_state<UTF_32_encoding_tag, UTF_16_encoding_tag> {        
        typedef typename UTF_16_encoding_tag::code_unit_type value_type;
        
        UTF_16_encoding_tag::encode_buffer_type buffer_;
    };
    
    template <>
    struct iterator_state<platform_encoding_tag, UTF_16_encoding_tag> {        
        typedef typename UTF_16_encoding_tag::code_unit_type value_type;
        
        UTF_16_encoding_tag::encode_buffer_type buffer_;
    };
    
    
}}}



namespace json { namespace unicode {
    
    
    
    template<
        typename IteratorT 
      , typename FromEncodingT      /* any subclass of json::unicode::utf_encoding_tag */
      , typename ToEncodingT        /* any subclass of json::unicode::utf_encoding_tag */
    >
    class unicode_iterator:
        public boost::iterator_facade<
            unicode_iterator<IteratorT, FromEncodingT, ToEncodingT>  // Derived
          , typename ToEncodingT::code_unit_type                     // Value
          , typename boost::iterator_traversal<IteratorT>::type      // CategoryOrTraversal
          , typename ToEncodingT::code_unit_type                     // Reference
        >
    {
        // make a handy typedef for the base class:
        typedef boost::iterator_facade<
            unicode_iterator<IteratorT, FromEncodingT, ToEncodingT>
                , IteratorT
                , typename ToEncodingT::code_unit_type
                , typename boost::iterator_category<IteratorT>::type
                , typename ToEncodingT::code_unit_type
            >                                       base_class; 
        
        typedef typename FormEncodingT::endian_tag  from_endian_type;
        typedef typename ToEncodingT::endian_tag    to_endian_type;
        typedef typename host_endianness::type      host_endian_type;
        
        typedef internal::iterator_state<from_endian_type, to_endian_type> state_type;
        
    public:
        typedef typename base_class::reference      reference;
        typedef typename base_class::value_type     value_type;
        
        
    public:
        unicode_iterator() {}
        unicode_iterator(IteratorT const& other): base_class(other) {}
        
    private:
        friend class boost::iterator_core_access;
        
        const reference dereference() const 
        {
            if (state_.index_ == -1) {
                
            }
            
            
            return byte_swap<external_endian, internal_endian>(*(this->base_reference()));
        }

        void increment() {
        }
        
        
    private:
        IteratorT   source_iter_;
        state_type  state_;
    };

}}



#endif // JSON_UNICODE_UNICODE_ITERATOR_HPP

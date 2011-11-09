//
//  byte_swap_iterator.hpp
//
//  Created by Andreas Grosam on 5/17/11.
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

#ifndef JSON_BYTE_SWAP_ITERATOR_HPP
#define JSON_BYTE_SWAP_ITERATOR_HPP


#include <boost/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include "endian.hpp"
#include "byte_swap.hpp"
#include "json/unicode/unicode_utilities.hpp"


namespace json { namespace internal {

    // An iterator which swaps the bytes of its value when dereferenced and
    // when byte swapping is required according the machine endianness and
    // the encoding.
    //
    // Note that the reference_type of the iterator is NOT a reference -
    // in fact it is value_type!
    //
    // A decent compiler should be able to optimize a no-swap dereference operation
    // such that it is no slower than the underlaying iterator type. So, no
    // penalty should be payed when using an iterator adapter.
    // 
    // The default template argument for EncodingT is specified such that always
    // a non-swapping iterator will be instantiated. This should be used with
    // care, and the encoding of files shall always be specified.
    
    using json::internal::host_endianness;
 
    template<
        typename IteratorT, 
        typename EncodingT = typename json::unicode::iterator_encoding<IteratorT>::type
    >
    class byte_swap_iterator:
        public boost::iterator_adaptor<
                byte_swap_iterator<IteratorT, EncodingT>,               // Derived
                IteratorT,                                              // Base
                typename boost::iterator_value<IteratorT>::type,        // Value
                typename boost::iterator_category<IteratorT>::type,     // CategoryOrTraversal
                typename boost::iterator_value<IteratorT>::type         // Reference
        >
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, EncodingT>::value) );
        
        // make a handy typedef for the base class:
        typedef boost::iterator_adaptor<
                byte_swap_iterator, 
                IteratorT,
                typename boost::iterator_value<IteratorT>::type,
                typename boost::iterator_category<IteratorT>::type,
                typename boost::iterator_value<IteratorT>::type 
        >   base_class; 
        
        typedef typename EncodingT::endian_tag      external_endian;
        typedef typename host_endianness::type      internal_endian;
        
        BOOST_STATIC_ASSERT( (boost::is_base_of<json::internal::endian_tag, external_endian>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_of<json::internal::endian_tag, internal_endian>::value) );
        

    public:
        typedef typename base_class::reference      reference;
        typedef typename base_class::value_type     value_type;
        
        
    public:
        byte_swap_iterator() {}
        byte_swap_iterator(IteratorT const& other): base_class(other) {}
        
    private:
        friend class boost::iterator_core_access;
        
        const reference dereference() const {
            return byte_swap<external_endian, internal_endian>(*(this->base_reference()));
        }
        
    };
    
    
    //    
    // iterator_selector selects the propper iterator type required internally 
    // for the parser based on the type of source iterator and the specified 
    // encoding scheme. This returns either the source iterator IteratorT or a
    // byte_swap_interator<IteratorT, EncodingT> depending on whether or not 
    // byte swapping is required. 
    //
    template<typename IteratorT, typename EncodingT>
    struct iterator_selector 
    {
        typedef typename EncodingT::endian_tag      external_endian;
        typedef typename host_endianness::type      platform_endian;
        
        typedef typename boost::mpl::if_c< 
            boost::is_same<external_endian, platform_endian>::value or
            boost::is_same<EncodingT, json::unicode::UTF_8_encoding_tag>::value or
            boost::is_same<EncodingT, json::unicode::platform_encoding_tag>::value,
            IteratorT,
            byte_swap_iterator<IteratorT, EncodingT>
        >::type type;    
    };
    

    
    
    // iterator_selector is a helper trait which is used to select the
    // default encoding scheme for a given iterator's value type. It expects
    // an iterator as its template argument. This iterator is assumed to point 
    // to text stored in the machine itself, that is, it does not origin from 
    // an external source (a file or stream), although this is not a strict 
    // requirement. But this way, we can savely determine the endianness.
    // 
    

}}  // namespace json::internal



#endif // JSON_BYTE_SWAP_ITERATOR_HPP

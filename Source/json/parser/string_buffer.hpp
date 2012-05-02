//
//  string_buffer.hpp
//  
//
//  Created by Andreas Grosam on 4/27/12.
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

#ifndef JSON_INTERNAL_STRING_BUFFER0_HPP
#define JSON_INTERNAL_STRING_BUFFER0_HPP

#include "json/config.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "json/unicode/unicode_converter.hpp"
#include "json/unicode/unicode_conversion.hpp"
#include "json/endian/byte_swap.hpp"

#include "string_storage.hpp"

#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>


#include <assert.h>



namespace json { namespace parser_internal {
    
    /**
     Synopsis 
     
     template <typename EncodingT, typename SemanticActionsT>
     class string_buffer
     {
     public:
         typedef typename base::code_unit_type                       code_unit_type;
         typedef typename base::buffer_type                          buffer_type;
         typedef typename base::const_buffer_type                    const_buffer_type;
         
         string_buffer(SemanticActionsT& sa)
         
         // Returns the string as a buffer.
         const_buffer_type buffer() const;
         
         
         // Returns the size of the string (number of code units).
         size_t size() const;
         
         
         // Appends an Unicode code point to the string buffer. Unicode code 
         // points are always in host endianness and are assumed to be valid 
         // unicode scalar values.
         void        
         append_unicode(json::unicode::code_point_t codepoint);
         
         
         // Appends a code unit whose endianness equals the endiannes of the
         // underlaying string storage.
         // It does not check the validity of the code unit nor the validity of 
         // the code unit in the context of the string.
         void 
         append(code_unit_type cu);
         
         // Appends an ASCII character to its internal buffer. The value
         // of ch shall be in the range of valid ASCII characters, that is
         // [0 .. 0x7F]. The function does not check if the character is
         // actually valid.
         void        
         append_ascii(char ch);
         
         
         void clear();
         
         // Causes the remaining bytes from the internal string storage to be send 
         // to the semantic actions object through calling value_string(buffer, false).
         // flush() shall only be called when the parser finished parsing a **data** JSON string.        
         void flush();
         };
     
     */
    
    
    using json::unicode::UTF_32_encoding_tag;
    
    using json::unicode::add_endianness;
    using json::unicode::encoding_traits;
    using json::unicode::host_endianness;
    using json::unicode::encoding_traits;
    
    using json::unicode::converter;
    using json::unicode::Validation;
    using json::unicode::Stateful;
    using json::unicode::ParseOne;
    
    using json::parser_internal::string_storage;
    
    using json::unicode::code_point_t;
    
    using json::byte_swap;
    
    
    template <typename EncodingT, typename SemanticActionsT>
    class  string_buffer : public string_storage<EncodingT>
    {
        // Although not strictly required, EncodingT's endianness shall be
        // equal host endianness:
        BOOST_STATIC_ASSERT( (boost::is_same<
                              typename encoding_traits<typename add_endianness<EncodingT>::type>::endian_tag,
                              typename host_endianness::type
                              >::value == true)  );
        
        // For efficiency reasons, EncodingT shall be either UTF_32_encoding_tag in
        // host endianness, or it shall be equal the SemanticActionsT's encoding:
        // (With the rule above, this also implies, that SemanticActionsT's encoding 
        // must have host endianness as well)
        BOOST_STATIC_ASSERT((boost::mpl::or_<
                             boost::is_same<
                             typename add_endianness<EncodingT>::type, 
                             typename add_endianness<UTF_32_encoding_tag>::type
                             >
                             , boost::is_same<EncodingT, typename SemanticActionsT::encoding_t>
                             >::value == true));
        
    private:
        typedef string_storage<EncodingT>                           base;
    public:
        typedef typename base::code_unit_type                       code_unit_type;
        typedef typename base::buffer_type                          buffer_type;
        typedef typename base::const_buffer_type                    const_buffer_type;
    private:
        typedef typename base::encoding_type                        encoding_type;
        typedef typename add_endianness<encoding_type>::type        to_encoding_t;        
        typedef typename encoding_traits<to_encoding_t>::endian_tag to_endian_t;
        typedef typename host_endianness::type                      host_endian_t;
        typedef typename SemanticActionsT::encoding_t               sa_encoding_type;
        
        
    public:
        string_buffer(SemanticActionsT& sa)
        : sa_(sa), base()
        {
        }
        
        string_buffer(SemanticActionsT& sa, std::size_t initial_storage_capacity)
        : sa_(sa), base(initial_storage_capacity)
        {
        }
        
        // Returns the string as a buffer.
        const_buffer_type buffer() const      { return base::buffer(); }
        
        
        // Returns the size of the string (number of code units).
        size_t size() const { return base::size(); }
        
        
        // Appends an Unicode code point to the string buffer. Unicode code 
        // points are always in host endianness and are assumed to be valid 
        // unicode scalar values.
        void        
        append_unicode(json::unicode::code_point_t codepoint)
        {
            append_unicode_imp<encoding_type>(codepoint);
        }
        
        
        // Appends a code unit whose endianness equals the endiannes of the
        // underlaying string storage.
        // It does not check the validity of the code unit nor the validity of 
        // the code unit in the context of the string.
        void 
        append(code_unit_type cu) {
            base::append(cu);
        }
        
        // Appends an ASCII character to its internal buffer. The value
        // of ch shall be in the range of valid ASCII characters, that is
        // [0 .. 0x7F]. The function does not check if the character is
        // actually valid.
        void        
        append_ascii(char ch)
        {
            assert(ch >= 0 and ch < 0x80);
            base::append(byte_swap<host_endian_t, to_endian_t>(static_cast<code_unit_type>(ch)));
        }
        
        
        void clear() {
            base::reset();
        }
        
        // Causes the remaining bytes from the internal string storage to be send 
        // to the semantic actions object through calling value_string(buffer, false).
        // flush() shall only be called when the parser finished parsing a **data** JSON string.        
        void flush() {
            this->write<encoding_type>(false);
            base::reset();
        }
        
        
        
        
    private:
        
        template <typename E>
        void 
        append_unicode_imp(json::unicode::code_point_t codepoint,
                           typename boost::enable_if<
                           boost::is_same<
                           UTF_32_encoding_tag, 
                           E
                           >    
                           >::type* dummy = 0)
        {
            // code_point_t equals code_unit_t if endianness will be adjusted
            base::append(byte_swap<host_endian_t, to_endian_t>(static_cast<code_unit_type>(codepoint)));
        }
        
        template <typename E>
        void 
        append_unicode_imp(json::unicode::code_point_t codepoint,
                           typename boost::disable_if<
                           boost::is_same<
                           UTF_32_encoding_tag, 
                           E
                           >    
                           >::type* dummy = 0)
        {
            typedef converter<code_point_t, to_encoding_t, Validation::UNSAFE, Stateful::No, ParseOne::Yes> cvt_t;
            base::extend(4/sizeof(code_unit_type));
            json::unicode::code_point_t* first = &codepoint;
#if defined (DEBUG)            
            int result =
#endif            
            cvt_t::convert(first, first+1, base::dest());
            assert(result == 0);
        }
        
        
    private:
        
        // sync() will be called by the underlaying string storage whenever
        // its internal buffer would overflow and needs to be synchronized with
        // the external buffer managed by the semantic actions object. The
        // string storage will reset its internal buffer.
        virtual void sync() {
            this->write<encoding_type>(true);
            this->reset();
        }
        
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
        inline void 
        write(bool hasMore, 
              typename boost::enable_if<
              boost::is_same<typename write_policy_traits<E, sa_encoding_type>::type, write_policy_direct>    
              >::type* dummy = 0) 
        {
            sa_.value_string(base::buffer(), hasMore);
            //            
            //            if (mode_ == Data) {
            //                sa_.value_string(buffer(), hasMore);
            //            }
            //            else {
            //                throw no_partial_keystring_error();
            //            }
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
            
            //            if (mode_ == Key) {
            //                throw no_partial_keystring_error();
            //            }
            
            code_unit_type const* first = base::data();
            code_unit_type const* last = first + base::size();
            sa_char_t* start = reinterpret_cast<sa_char_t*>(static_cast<void*>(base::data()));
            sa_char_t* dest = start;
#if defined (DEBUG)
            int cvt_result = 
#endif            
            cvt_t().convert(first, last, dest);
            assert(cvt_result == 0);
            size_t length = static_cast<size_t>(dest - start);
            sa_.value_string(const_buffer_t(start, length), hasMore);
        }
        
        
    private:
        SemanticActionsT&       sa_;
    };
    
}}  // namespace json::parser_internal



#endif // JSON_INTERNAL_STRING_BUFFER_HPP

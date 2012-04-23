//
//  key_string_buffer.hpp
//  
//
//  Created by Andreas Grosam on 4/13/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_INTERNAL_KEY_STRING_BUFFER_HPP
#define JSON_PARSER_INTERNAL_KEY_STRING_BUFFER_HPP

#include "json/config.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "string_storage.hpp"


#include <assert.h>



namespace json { namespace parser_internal {
    
    /**
     Synopsis 
     
     template <typename StringStorageT>
     class key_string_buffer
     {
     public:
     typedef StringStorageT         string_storage_type;
     typedef     type               size_type;
     typedef     type               code_unit_type;
     typedef     type               buffer_type;
     
     string_buffer(string_storage_type& storage);
     ~string_buffer();
     
     buffer_type buffer() const;
     
     void append_unicode(json::unicode::code_point_t codepoint)
     void append(code_unit_type cu);
     void append_ascii(char ch);
     
     std::pair<const void*, size_t> inplace_encode(Encoding encoding);
     };
     
     */
    
    
    
    using json::unicode::add_endianness;
    using json::unicode::encoding_traits;
    using json::unicode::host_endianness;
    using json::unicode::code_point_t;
        
    
    template <typename StringStorageT>
    class  key_string_buffer {
    public:
        typedef StringStorageT                                      string_storage_type;
        typedef size_t                                              size_type;
        typedef typename string_storage_type::code_unit_type        code_unit_type;
        typedef typename string_storage_type::buffer_type           buffer_type;
        typedef typename string_storage_type::const_buffer_type     const_buffer_type;
        typedef typename string_storage_type::encoding_type         encoding_type;

    private:
        typedef typename add_endianness<encoding_type>::type        to_encoding_t;        
        typedef typename encoding_traits<to_encoding_t>::endian_tag to_endian_t;
        typedef typename host_endianness::type                      host_endian_t;
        
    public:
        key_string_buffer(string_storage_type& storage)
        : string_storage_(storage)
        {
            string_storage_.set_mode(StringStorageT::Key);
        }
        
        ~key_string_buffer() {}
        
        const_buffer_type buffer() const { return string_storage_.buffer(); }
        
        
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
            string_storage_.append(cu);
        }
        
        // Appends an ASCII character to its internal buffer. The value
        // of ch shall be in the range of valid ASCII characters, that is
        // [0 .. 0x7F]. The function does not check if the character is
        // actually valid.
        void        
        append_ascii(char ch)
        {
            assert(ch >= 0 and ch < 0x80);
            string_storage_.append(json::byte_swap<host_endian_t, to_endian_t>(static_cast<code_unit_type>(ch)));
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
            string_storage_.append(json::byte_swap<host_endian_t, to_endian_t>(static_cast<code_unit_type>(codepoint)));
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
            using json::unicode::converter;
            using json::unicode::Validation;
            using json::unicode::Stateful;
            using json::unicode::ParseOne;
                        
            typedef converter<code_point_t, to_encoding_t, Validation::UNSAFE, Stateful::No, ParseOne::Yes> cvt_t;
            
            if (string_storage_.storage_avail() < (4/sizeof(code_unit_type))) {
                string_storage_.extend(4/sizeof(code_unit_type));
            }
            json::unicode::code_point_t* first = &codepoint;
            cvt_t::convert(first, first+1, string_storage_.dest());            
        }
        
        
        
        
    private:
        string_storage_type& string_storage_;
    };
    
}}  // namespace json::parser_internal



#endif // JSON_PARSER_INTERNAL_KEY_STRING_BUFFER_HPP

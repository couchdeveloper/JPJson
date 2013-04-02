//
//  token_traits.hpp
//  
//
//  Created by Andreas Grosam on 30.03.13.
//
//

#ifndef JSON_TOKEN_TRAITS_HPP
#define JSON_TOKEN_TRAITS_HPP

#include "json/unicode/unicode_traits.hpp"
#include "json/endian/byte_swap.hpp"
#include "json/endian/endian.hpp"
#include <array>
#include <type_traits>

namespace json {
    
    using internal::host_endianness;

    using unicode::add_endianness;
    using unicode::encoding_traits;


    

    template <typename Encoding>
    struct token_traits
    {
        typedef typename encoding_traits<Encoding>::code_unit_type char_type;
        
        typedef typename host_endianness::type                      host_endian_t;
        typedef typename encoding_traits<Encoding>::endian_tag      to_endian_t;
        
        static constexpr char_type swap(char_type c) {
            return byte_swap<host_endian_t,to_endian_t>(c);
        }
        
        static constexpr std::array<char_type, 1> object_open_token = {swap(u'{')};
        static constexpr std::array<char_type, 1> object_close_token = {swap(u'}')};
        static constexpr std::array<char_type, 1> array_open_token = {swap(u'[')};
        static constexpr std::array<char_type, 1> array_close_token = {swap(u']')};
        
        static constexpr std::array<char_type, 5> false_token = {swap(u'f'), swap(u'a'), swap(u'l'), swap(u's'), swap(u'e')};
        static constexpr std::array<char_type, 4> true_token = {swap(u't'), swap(u'r'), swap(u'u'), swap(u'e')};
        static constexpr std::array<char_type, 4> null_token = {swap(u'n'), swap(u'u'), swap(u'l'), swap(u'l')};
        
        static constexpr std::array<char_type, 1> quote_token = {swap('"')};
        static constexpr std::array<char_type, 1> colon_token = {swap(':')};
        static constexpr std::array<char_type, 1> comma_token = {swap(',')};
        static constexpr std::array<char_type, 1> newline_token = {swap('\n')};
        static constexpr std::array<char_type, 1> tab_token = {swap('\t')};
        static constexpr std::array<char_type, 1> space_token = {swap(' ')};
        
    };
    
    
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::object_open_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::object_close_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::array_open_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::array_close_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,5> token_traits<Encoding>::false_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,4> token_traits<Encoding>::true_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,4> token_traits<Encoding>::null_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::quote_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::colon_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::comma_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::newline_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::tab_token;
    template <typename Encoding>
    constexpr std::array<typename encoding_traits<Encoding>::code_unit_type,1> token_traits<Encoding>::space_token;
    

}



#endif // JSON_TOKEN_TRAITS_HPP

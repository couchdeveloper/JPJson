//
//  write_value.hpp
//  
//
//  Created by Andreas Grosam on 26.03.13.
//
//

#ifndef _write_value_hpp
#define _write_value_hpp

#include "json/value/value.hpp"
#include "json/utility/number_to_string.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "generate.hpp"
#include "token_traits.hpp"
#include <ios>
#include <algorithm>
#include <cassert>



namespace json {
    
    
    class writer_base
    {
    public:
        typedef unsigned int fmtflags;
        static constexpr fmtflags pretty_print      = 0x0001;
        static constexpr fmtflags escape_solidus    = 0x0002;
        
        writer_base(fmtflags flags = 0) : flags_(flags) {}
        
        unsigned int flags() const { return flags_; }
        
    private:
        fmtflags flags_;
    };
    
    
//    template <typename Value, typename OutputIterator>
//    static OutputIterator write_value(const Value& value, OutputIterator dest, unsigned int fmtflags);


}


namespace json { namespace detail {


    struct to_code_point
    {
        typedef json::unicode::code_point_t code_point_t;
        typedef std::pair<int, code_point_t> result_t;
        
        constexpr to_code_point() noexcept = default;
        
        typedef char const* iterator;
        
        static constexpr code_point_t
        encode(iterator iter)
        {
            return (json::unicode::utf8_encoded_length_unsafe(iter[0]) == 1) ?
                static_cast<code_point_t>(iter[0]) :
            (json::unicode::utf8_encoded_length_unsafe(iter[0]) == 2) ?
                ((static_cast<uint32_t>(iter[0]) << 6) & 0x7FFu) + (iter[1] & 0x3Fu) :
            (json::unicode::utf8_encoded_length_unsafe(iter[0]) == 3) ?
                ((static_cast<uint32_t>(iter[0]) << 12) & 0xFFFFu)
                    + ((static_cast<uint32_t>(iter[1]) << 6) & 0xFFFu)
                    + (static_cast<uint32_t>(iter[2]) & 0x3Fu) :
                ((static_cast<uint32_t>(iter[0]) << 18) & 0x1FFFFFu)
                    + ((static_cast<uint32_t>(iter[1]) << 12) & 0x3FFFFu)
                    + ((static_cast<uint32_t>(iter[2]) << 6) & 0xFFFu)
                    + (iter[3] & 0x3Fu);
        }
        
        
    };
    
    
    constexpr std::pair<int, json::unicode::code_point_t> x;
    
    
}}




namespace json { namespace detail {
    

    /**
     Template parameter `Value` specifies the type of the JSON representation.
     Usually, this is an instantiation of template class json::value.
     
     Template parameter `OutputIterator` and `Encoding` specifies the iterator type 
     of the destination respectively the encoding where the characters are written to.
     `Encoding` shall be one of the unicode encoding schemes or it can be 
     json::unicode::escaped_unicode_t in which case the output sequence becomes
     escaped unicode - and is comprised completely of ASCII characters.
     */
    template <typename Value, typename OutputIterator, typename Encoding = json::unicode::UTF_8_encoding_tag>
    struct writer;
    
    
    
    template <typename CharT>
    struct map_char_type_to_unicode_encoding;
    
    template <>
    struct map_char_type_to_unicode_encoding<char>
    {
        typedef json::unicode::UTF_8_encoding_tag encoding;
    };
    
    template <>
    struct map_char_type_to_unicode_encoding<char16_t>
    {
        typedef json::unicode::UTF_16_encoding_tag encoding;
    };
    
    template <>
    struct map_char_type_to_unicode_encoding<char32_t>
    {
        typedef json::unicode::UTF_32_encoding_tag encoding;
    };
    
    
    template <typename Value, typename OutputIterator, typename Encoding = json::unicode::UTF_8_encoding_tag>
    static OutputIterator _write_value(const Value& value, OutputIterator dest, writer<Value, OutputIterator, Encoding>& w, int level)
    {
        return value.apply_visitor(w, dest, level);
    }
    
    
    
    
    template <typename Value, typename OutputIterator, typename OutEncoding>
    struct writer : writer_base
    {
        static_assert(std::is_same<json::unicode::escaped_unicode_encoding_t, OutEncoding>::value
                      or std::is_base_of<json::unicode::utf_encoding_tag, OutEncoding>::value, "");
        
        
        typedef typename std::conditional<std::is_same<
                json::unicode::escaped_unicode_encoding_t, OutEncoding>::value,
                json::unicode::UTF_8_encoding_tag,
        OutEncoding>::type                              out_encoding_type;
        
        typedef typename encoding_traits<out_encoding_type>::code_unit_type char_type;
        
        using TokenTraits = typename json::token_traits<out_encoding_type>;
        
    public:
        typedef OutputIterator result_type;
        
        typedef typename Value::integral_number_type    IntNumber;
        typedef typename Value::float_number_type       FloatNumber;
        typedef typename Value::null_type               Null;
        typedef typename Value::boolean_type            Boolean;
        typedef typename Value::string_type             String;
        typedef typename Value::array_type              Array;
        typedef typename Value::object_type             Object;
        typedef typename Value::key_type                Key;
        
        typedef typename map_char_type_to_unicode_encoding<typename String::value_type>::encoding string_encoding_type;
        typedef typename map_char_type_to_unicode_encoding<typename Key::value_type>::encoding key_encoding_type;
        
        
        
        
        writer(unsigned int flags) : writer_base(flags) {}
        
        
        OutputIterator operator()(const Null& v, OutputIterator dest, int level)
        {
            return std::copy(TokenTraits::null_token.cbegin(), TokenTraits::null_token.cend(), dest);
        }
        
        OutputIterator operator()(const Boolean& v, OutputIterator dest, int level)
        {
            if (v)
                return std::copy(TokenTraits::true_token.cbegin(), TokenTraits::true_token.cend(), dest);
            else
                return std::copy(TokenTraits::false_token.cbegin(), TokenTraits::false_token.cend(), dest);
        }
        
        OutputIterator operator()(const IntNumber& v, OutputIterator dest, int level)
        {
            return json::utility::write_number(static_cast<long long>(v), dest);
        }
        
        OutputIterator operator()(const FloatNumber& v, OutputIterator dest, int level)
        {
            return json::utility::write_number(static_cast<long double>(v), dest);
        }
        
        OutputIterator operator()(const String& str, OutputIterator dest, int level)
        {
            using json::generator_internal::encode_string;
            
            std::copy(TokenTraits::quote_token.cbegin(), TokenTraits::quote_token.cend(), dest);
            auto first = str.begin();
            auto last = str.end();
            const unsigned int options = (flags()&escape_solidus) ? generator_internal::string_encoder_base::EscapeSolidus : 0;
#if defined (DEBUG)
            int cvt_result =
#endif            
            encode_string(first, last, string_encoding_type(),
                               dest, OutEncoding(),
                               options);
            assert(cvt_result == 0);
            return std::copy(TokenTraits::quote_token.cbegin(), TokenTraits::quote_token.cend(), dest);
        }
        
        OutputIterator operator()(const Array& array, OutputIterator dest, int level)
        {
            OutputIterator result = std::copy(TokenTraits::array_open_token.cbegin(), TokenTraits::array_open_token.cend(), dest);
            ++level;
            std::size_t count = array.size();
            for (Value x : array) {
                if ((flags()&pretty_print) != 0) {
                    result = std::fill_n(result, 1, TokenTraits::newline_token[0]);
                    result = std::fill_n(result, level, TokenTraits::tab_token[0]);
                }
                result = _write_value(x, result, *this, level);
                if (--count > 0) {
                    result = std::copy(TokenTraits::comma_token.cbegin(), TokenTraits::comma_token.cend(), result);
                }
            }
            --level;
            if (array.size() and (flags()&pretty_print) != 0) {
                result = std::fill_n(result, 1, TokenTraits::newline_token[0]);
                result = std::fill_n(result, level, TokenTraits::tab_token[0]);
            }
            return std::copy(TokenTraits::array_close_token.cbegin(), TokenTraits::array_close_token.cend(), result);
        }
        
        OutputIterator operator()(const Object& obj, OutputIterator dest, int level)
        {
            using json::generator_internal::encode_string;
            
            OutputIterator result = std::copy(TokenTraits::object_open_token.cbegin(), TokenTraits::object_open_token.cend(), dest);
            ++level;
            std::size_t count = obj.size();
            for (auto iter : obj) {
                if ((flags()&pretty_print) != 0) {
                    result = std::fill_n(result, 1, TokenTraits::newline_token[0]);
                    result = std::fill_n(result, level, TokenTraits::tab_token[0]);
                }
                std::copy(TokenTraits::quote_token.cbegin(), TokenTraits::quote_token.cend(), dest);
                auto first = iter.first.begin();
                auto last = iter.first.end();
                const unsigned int options = (flags()&escape_solidus) ? generator_internal::string_encoder_base::EscapeSolidus : 0;
#if defined (DEBUG)
                int cvt_result =
#endif
                encode_string(first, last, key_encoding_type(),
                              result, out_encoding_type(),
                              options);
                assert(cvt_result == 0);
                
                if ((flags()&pretty_print) != 0) {
                    result = std::copy(TokenTraits::quote_token.cbegin(), TokenTraits::quote_token.cend(), result);
                    result = std::copy(TokenTraits::space_token.cbegin(), TokenTraits::space_token.cend(), result);
                    result = std::copy(TokenTraits::colon_token.cbegin(), TokenTraits::colon_token.cend(), result);
                    result = std::copy(TokenTraits::space_token.cbegin(), TokenTraits::space_token.cend(), result);
                }
                else {
                    result = std::copy(TokenTraits::quote_token.cbegin(), TokenTraits::quote_token.cend(), result);
                    result = std::copy(TokenTraits::colon_token.cbegin(), TokenTraits::colon_token.cend(), result);
                }
                result = _write_value(iter.second, result, *this, level);
                if (--count > 0) {
                    result = std::copy(TokenTraits::comma_token.cbegin(), TokenTraits::comma_token.cend(), result);
                }
            }
            --level;
            if (obj.size() and (flags()&pretty_print) != 0) {
                result = std::fill_n(result, 1, TokenTraits::newline_token[0]);
                result = std::fill_n(result, level, TokenTraits::tab_token[0]);
            }
            return std::copy(TokenTraits::object_close_token.cbegin(), TokenTraits::object_close_token.cend(), result);
        }
        
    };

    } // namespace detail
    
    template <typename Value, typename OutputIterator, typename Encoding = json::unicode::UTF_8_encoding_tag>
    OutputIterator write_value(const Value& value, OutputIterator dest, unsigned int fmtflags = 0, Encoding encoding = json::unicode::UTF_8_encoding)
    {
        typedef detail::writer<Value, OutputIterator, Encoding> Writer;
        
        Writer w(fmtflags);
        return value.apply_visitor(w, dest, 0);
    }
    
} // namespace json



#endif

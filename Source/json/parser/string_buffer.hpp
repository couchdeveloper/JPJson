//
//  string_buffer.hpp
//  
//
//  Created by Andreas Grosam on 11/19/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_INTERNAL_STRING_BUFFER_HPP
#define JSON_PARSER_INTERNAL_STRING_BUFFER_HPP


#include "json/config.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_conversions.hpp"

namespace json { namespace parser_internal {
    
    
    using json::unicode::Encoding;
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    

    template <size_t Capacity>
    class  string_buffer {
    private:
        typedef json::unicode::code_point_t         value_type;
    public:
        typedef size_t                              size_type;
        typedef std::pair<const void*, size_t>      buffer_type;
        
    public:
        string_buffer()
        : p_(buffer_), end_(buffer_ + Capacity)
        {
        }
        ~string_buffer() {}
        
        size_type   capacity() const    { return Capacity; }
        size_type   size() const        { return p_ - buffer_; }
        
        void        reset()             { p_ = buffer_; }
        
        buffer_type buffer() const      { return buffer_type(buffer_, size()); }
        
        bool        append_unicode(json::unicode::code_point_t codepoint)
        {
            assert(json::unicode::isUnicodeScalarValue(codepoint));
            if (p_ != end_) {
                *p_++ = codepoint;
                return true;
            }
            else {
                return false;
            }
        }

        bool        append_ascii(char ch)
        {
            assert(ch >= 0 and ch < 0x80);            
            if (p_ != end_) {
                *p_++ = static_cast<json::unicode::code_point_t>(ch);
                return true;
            }
            else {
                return false;
            }
        }
        
        buffer_type inplace_encode(Encoding encoding)
        {
            size_t count;
            int error = 0;
            switch (encoding) {
                case json::unicode::UnicodeEncoding_UTF8:
                    count = inplace_encode_imp(json::unicode::encoding_to_tag<json::unicode::UnicodeEncoding_UTF8>::type(), error);
                    break;
                case json::unicode::UnicodeEncoding_UTF16BE:
                    count = inplace_encode_imp(json::unicode::encoding_to_tag<json::unicode::UnicodeEncoding_UTF16BE>::type(), error);
                    count *= 2;
                    break;
                case json::unicode::UnicodeEncoding_UTF16LE:
                    count = inplace_encode_imp(json::unicode::encoding_to_tag<json::unicode::UnicodeEncoding_UTF16LE>::type(), error);
                    count *= 2;
                    break;
                case json::unicode::UnicodeEncoding_UTF32BE:
                    count = inplace_encode_imp(json::unicode::encoding_to_tag<json::unicode::UnicodeEncoding_UTF32BE>::type(), error);
                    count *= 4;
                    break;
                case json::unicode::UnicodeEncoding_UTF32LE:
                    count = inplace_encode_imp(json::unicode::encoding_to_tag<json::unicode::UnicodeEncoding_UTF32LE>::type(), error);
                    count *= 4;
                    break;
            }
            
            buffer_type result(buffer_, count);
            p_ = buffer_;
            return result;
        }
        
        
    private:
        
        template <typename EncodingT>
        size_t inplace_encode_imp(EncodingT encoding, int& error) {
            typedef typename EncodingT::code_unit_type char_t;
            value_type* first = buffer_;
            char_t* dest = reinterpret_cast<char_t*>(first);
            return convert_codepoints_unsafe(first, end_, dest, encoding, error); 
        }
        
        
    private:
        value_type  buffer_[Capacity];
        value_type* p_;
        value_type* end_;
    };

}}  // namespace json::parser_internal



#endif // JSON_PARSER_INTERNAL_STRING_BUFFER_HPP

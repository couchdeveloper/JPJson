//
//  generate.hpp
//  
//
//  Created by Andreas Grosam on 5/23/11.
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

#ifndef JSON_GENERATE_HPP
#define JSON_GENERATE_HPP


#include "json/config.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_converter.hpp"
#include "json/endian/byte_swap.hpp"
#include <algorithm>
#include <cassert>
#include <type_traits>


namespace json { namespace generator_internal {
    
    
    using namespace json::unicode;
    
    using json::byte_swap;
       
    //
    // Copies a BOM for the specified encoding into a range beginning with dest.
    //
    // Returns an iterator to the end of the destination range (which points to 
    // the element following the BOM).
    //
    // OutputIterator shall be appropriate for the encoding.

    template <typename OutputIterator, typename EncodingT>
    inline OutputIterator
    copyBOM(OutputIterator dest, EncodingT encoding)
    {
        static_assert( (std::is_base_of<utf_encoding_tag, EncodingT>::value), "" );
        
        typedef typename encoding_traits<EncodingT>::bom_type bom_type;
        bom_type bom = encoding_traits<EncodingT>::bom();
        return std::copy(bom.begin(), bom.end(), dest);        
    }
    
    
    struct string_encoder_base {
        
        enum EncodingOptions {
            EscapeSolidus   = 1 << 0
        };
        
    };
    
    //
    // Class string_encoder
    //
    // Template parameter InIteratorT and InEncodingT specify the type of the
    // iterator and the type of the encoding for the input string.
    // InEncodingT shall be one of the Unicode encoding tags which is defined in file
    // unicode_traits.hpp.
    //
    // Template parameter OutIterator and OutEncodingT specifies the iterator type
    // of the destination and its encoding. OutEncodingT shall be one of the Unicode
    // encoding tags or it shall equal type json::unicode::escaped_unicode_encoding_t
    // which is defined in file unicode_traits.hpp.
    //
    // If OutEncodingT equals json::unicode::escaped_unicode_t the output sequence
    // becomes effectively ASCII (UTF-8) where unicode characters which are not ASCII
    // are encoded as an Escapded Unicode Sequence as defined in RFC 4627.
    //
    // Synopsis:
    //
    //    inline std::size_t
    //    operator()(InIteratorT&     first,
    //               InIteratorT      last,
    //               InEncodingT      inEncoding,
    //               OutIteratorT&    dest,
    //               OutEncodingT     outEncoding,
    //               EncodingOptions  options) const;
    //
    // The operator escapes and converts an Unicode sequence [first, last)
    // from encoding 'inEncoding' to encoding 'outEncoding' for proper use as
    // a JSON String according RFC and copies the result into an output iterator.
    //
    // Parameter outEncoding may equal constant json::unicode::escaped_unicode_encoding
    // which is defined in unicode_converter.hpp. In this case, the output sequence
    // becomes ASCII where unicode characters which are not ASCII are presented
    // as an Escapded Unicode Sequence as defined in RFC 4627.
    //
    // The options parameter specifies encoding options:
    //
    //  If no flags are set only characters which are required to be escaped 
    //  according RFC 4627 will be escaped by prefixing it with a reverse solidus.
    //
    //  If option EscapeSolidus is set, in addition the solidus character ('/') 
    //  will be esecaped.
    //
    // 
    // Returns: 
    //  Zero on success, otherwise a negative value indicating an error as decribed
    //  by json::unicode::ErrorT.
    //
    // Requirements:
    // The Unicode sequence starting with first shall be valid Unicode. Other-
    // wise the behavior is undefined.
    
    // TODO: implement feature "Escape Non-ASCII" 
    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    class string_encoder : public string_encoder_base
    {
        static_assert(std::is_base_of<json::unicode::utf_encoding_tag, InEncodingT>::value, "");
        static_assert(std::is_same<json::unicode::escaped_unicode_encoding_t, OutEncodingT>::value
                      or std::is_base_of<json::unicode::utf_encoding_tag, OutEncodingT>::value, "");
        
    private:
        typedef typename encoding_traits<UTF_8_encoding_tag>::code_unit_type utf8_char_type;
        typedef typename encoding_traits<UTF_8_encoding_tag>::endian_tag    utf8_endian_t;

        typedef typename std::conditional<std::is_same<
            json::unicode::escaped_unicode_encoding_t, OutEncodingT>::value,
            json::unicode::UTF_8_encoding_tag,
            OutEncodingT>::type
        out_encoding_t;
                
        
        typedef typename encoding_traits<out_encoding_t>::code_unit_type    out_char_type;
        typedef typename add_endianness<out_encoding_t>::type               to_encoding_t;
        typedef typename encoding_traits<to_encoding_t>::endian_tag         to_endian_t;
        
        
        out_char_type swap(out_char_type in) const{
            return byte_swap<utf8_endian_t, to_endian_t>(in);
        }
        
        void write_ascii(utf8_char_type const* first, utf8_char_type const* last, OutIteratorT& dest) const {
            while (first != last) {
                assert(encoding_traits<UTF_8_encoding_tag>::to_uint(*first) <= 0x7Fu);
                *dest++ = swap(encoding_traits<UTF_8_encoding_tag>::to_uint(*first++));
            }
        }
        
        void write_ascii(utf8_char_type ch, OutIteratorT& dest) const {
            *dest++ = swap(encoding_traits<UTF_8_encoding_tag>::to_uint(ch));
        }
        
    public:
        
        
        inline int
        operator()(InIteratorT&     first, 
                   InIteratorT      last, 
                   OutIteratorT&    dest,
                   unsigned int     options) const
        {
            typedef utf8_char_type esc_buffer_type[8];
                        
            esc_buffer_type buffer;
            utf8_char_type* endBuffer = &buffer[0];
            
            while (first != last) 
            {
                unsigned int ch = encoding_traits<InEncodingT>::to_uint(*first); // ch in platform endianness
                if (__builtin_expect(ch < 0x80, 1))  // ASCII character inclusive Unicode NULL and control codes
                {
                    switch (ch)
                    {
                        case '"':
                            buffer[0] = '\\'; buffer[1] = '"'; endBuffer = &buffer[2];  break;
                        case '\\':
                            buffer[0] = '\\'; buffer[1] = '\\'; endBuffer = &buffer[2];  break;
                        case '/':
                            if ((options & EscapeSolidus) != 0) {
                                buffer[0] = '\\'; buffer[1] = '/'; endBuffer = &buffer[2];
                            } else {
                                goto WRITE_ASCII;
                            }
                            break;
                        case '\b':  buffer[0] = '\\'; buffer[1] = 'b'; endBuffer = &buffer[2]; break;
                        case '\f':  buffer[0] = '\\'; buffer[1] = 'f'; endBuffer = &buffer[2]; break;
                        case '\n':  buffer[0] = '\\'; buffer[1] = 'n'; endBuffer = &buffer[2]; break;
                        case '\r':  buffer[0] = '\\'; buffer[1] = 'r'; endBuffer = &buffer[2]; break;
                        case '\t':  buffer[0] = '\\'; buffer[1] = 't'; endBuffer = &buffer[2]; break;
                        default:
                            if (ch < 0x20u) {
                                // escape a control character
                                buffer[0] = '\n';
                                buffer[1] = 'u';
                                buffer[2] = '0';
                                buffer[3] = '0';
                                buffer[4] = ((ch >> 4)+'0');
                                buffer[5] = ((ch & 0x0Fu)+'0');
                                endBuffer = &buffer[6];
                            }
                            else {
                                goto WRITE_ASCII;
                            }
                    } // switch
                    
                    // Copy ASCCI character(s) to dest:
                    // Note, the characters shall be ASCII only which do not require
                    // a special unicode encoding conversion unless possibly swapping bytes:
                WRITE_ASCII_BUFFER:
                    write_ascii(buffer, endBuffer, dest);
                    first++;
                    continue;
                    
                WRITE_ASCII:
                    // unescaped
                    write_ascii(ch, dest);
                    ++first;
                    continue;
                }
                else // ch >= 0xF0
                {
                    // Copy one non-ASCCI character to dest:
                    // This requires a special conversion function:
                    // We assume, we have a complete and wellformed Unicode sequence
                    // as input. Thus, use an unsafe, stateless converter which
                    // converts one character at a time:
                    typedef converter<
                        InEncodingT, OutEncodingT,
                        Validation::UNSAFE, Stateful::No, ParseOne::Yes
                    >  converter_t;
                    
                    int result = converter_t().convert(first, last, dest);
                    if (result != 0) {
                        return result; // note: an unsafe converter possibly never returns an error!
                    }
                    continue;
                }
            } // while
            
            return 0;
        }
    };
    
    
    

    template <
        typename InIteratorT, typename InEncodingT,
        typename OutIteratorT, typename OutEncodingT
    >
    inline int
    encode_string(InIteratorT& first, InIteratorT last, InEncodingT inEncoding,
                  OutIteratorT& dest,  OutEncodingT outEncoding,
                  unsigned int options)
    {
        typedef string_encoder<InIteratorT, InEncodingT, OutIteratorT, OutEncodingT> string_encoder_t;
        return string_encoder_t()(first, last, dest, options);
    }

    
    
    
    
    
    
    
    
    
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT
    >
    inline int
    escape_convert_one_unsafe_slowpath(
                          InIteratorT&     first, 
                          InIteratorT      last, 
                          InEncodingT      inEncoding,
                          OutIteratorT&    dest,
                          OutEncodingT     outEncoding,
                          bool             escapeSolidus)
    {
        typedef typename encoding_traits<UTF_8_encoding_tag>::code_unit_type utf8_char_type;
        typedef typename encoding_traits<OutEncodingT>::code_unit_type      out_char_type;
        
        typedef typename encoding_traits<UTF_8_encoding_tag>::endian_tag    utf8_endian_t;  // this is host endianness
        typedef typename add_endianness<OutEncodingT>::type                 to_encoding_t;
        typedef typename encoding_traits<to_encoding_t>::endian_tag         to_endian_t;
        
        typedef utf8_char_type esc_buffer_type[8];
        
        assert(first != last);
        
        unsigned int ch = encoding_traits<InEncodingT>::to_uint(*first);
        
        esc_buffer_type escapedSequenceBuffer;
        
        bool escaped = true;
        utf8_char_type* startEscaped = &escapedSequenceBuffer[0];
        utf8_char_type* endEscaped = startEscaped;
        switch (ch) {
            case '"':   *endEscaped++ = '\\'; *endEscaped++ = '"'; break;
            case '\\':  *endEscaped++ = '\\'; *endEscaped++ = '\\'; break;
            case '/':   
                if (escapeSolidus) {
                    *endEscaped++ = '\\'; *endEscaped++ = '/';
                } else {
                    escaped = false;
                }
                break;
            case '\b':  *endEscaped++ = '\\'; *endEscaped++ = 'b'; break;
            case '\f':  *endEscaped++ = '\\'; *endEscaped++ = 'f'; break;
            case '\n':  *endEscaped++ = '\\'; *endEscaped++ = 'n'; break;
            case '\r':  *endEscaped++ = '\\'; *endEscaped++ = 'r'; break;
            case '\t':  *endEscaped++ = '\\'; *endEscaped++ = 't'; break;                    
            default:
                if (ch < 0x20u) {
                    // escape a control character
                    *endEscaped++ = '\n';
                    *endEscaped++ = 'u';
                    *endEscaped++ = '0';
                    *endEscaped++ = '0';
                    *endEscaped++ = ((ch >> 4)+'0');
                    *endEscaped++ = ((ch & 0x0Fu)+'0');
                }
                else {
                    // unescaped
                    escaped = false;
                }
        } // switch
        
        if (escaped) {
            // Copy the escaped sequence from internal buffer to dest using encoding 'outEncoding':
            // Note, the character within the internal buffer shall be ASCII only.
            while (startEscaped != endEscaped) {
                assert(encoding_traits<UTF_8_encoding_tag>::to_uint(*startEscaped) <= 0x7Fu);
                out_char_type c = byte_swap<utf8_endian_t, to_endian_t>(
                                                                        static_cast<out_char_type>(encoding_traits<UTF_8_encoding_tag>::to_uint(*startEscaped++)));
                *dest++ = c;
            }
            ++first;
        }
        else {
            // Use an unsafe, stateless converter which only converts one character:
            typedef converter<
            InEncodingT, OutEncodingT, 
            Validation::UNSAFE, Stateful::No, ParseOne::Yes
            >  converter_t;
            
            int result = converter_t().convert(first, last, dest);
            if (result != unicode::NO_ERROR) {
                return result;
            }
        }
        
        return 0;
    }    
    
    //
    //  Deprecated: use class string_encoder.
    //
    
    // Escape and convert a JSON string given as an Unicode sequence [first, last) 
    // from encoding 'inEncoding' to encoding 'outEncoding' and copy the result 
    // into dest.
    //
    // Only characters which are required to be escaped according the JSON
    // specification RFC 4627 will be escaped through prefixing it with a
    // reverse solidus.
    // These are namely the following characters:
    // Control characters (U+0000 through U+001F), quation marks ('"') and 
    // reverse solidus ('\'). The solidus character ('/') will be optionally
    // escaped if parameter escapeSolidus equals true. 
    // The input range [first .. last) shall contain a wellformed Unicode 
    // sequence in the specified Unicode encoding.
    // 
    // Returns unicode::NO_ERROR on success, otherwise a negative number indicating 
    // an error code as decribed in unicode::ErrorT.
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline int
    escape_convert_unsafe(
                          InIteratorT&     first, 
                          InIteratorT      last, 
                          InEncodingT      inEncoding,
                          OutIteratorT&    dest,
                          OutEncodingT     outEncoding,
                          bool             escapeSolidus)
    {
        typedef typename encoding_traits<UTF_8_encoding_tag>::code_unit_type utf8_char_type;
        typedef typename encoding_traits<OutEncodingT>::code_unit_type      out_char_type;
        
        typedef typename encoding_traits<UTF_8_encoding_tag>::endian_tag    utf8_endian_t;  // this is host endianness
        typedef typename add_endianness<OutEncodingT>::type                 to_encoding_t;
        typedef typename encoding_traits<to_encoding_t>::endian_tag         to_endian_t;
        
        typedef utf8_char_type esc_buffer_type[8];
        
        int result = unicode::NO_ERROR;
        while (first != last and result == unicode::NO_ERROR) 
        {
            unsigned int ch = encoding_traits<InEncodingT>::to_uint(*first);
            if (__builtin_expect((ch - 0x20u) < 0x60u, 1) and ch != '"' and ch != '\\' and ch != '/')  {
                ++first;
                *dest++ = byte_swap<utf8_endian_t, to_endian_t>(static_cast<out_char_type>(ch));
                continue;
            }
            result = escape_convert_one_unsafe_slowpath(first, last, inEncoding, dest, outEncoding, escapeSolidus);
        }
        return result;
    }    
    
    
    
    
}} // namespace json::generator_internal



#endif

//
//  unicode_traits.hpp
//  
//  Created by Andreas Grosam on 2/23/12.
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

#ifndef JSON_UNICODE_TRAITS_HPP
#define JSON_UNICODE_TRAITS_HPP

#include "json/config.hpp"
#include "unicode_utilities.hpp"
#include <assert.h>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>
#include <boost/array.hpp>
#include <stdint.h>
#include "json/endian/endian.hpp"
#include "json/endian/byte_swap.hpp"


//  For specifications, see 
//  http://www.unicode.org
//  http://en.wikipedia.org/wiki/UTF-8
//  RFC 3629, http://tools.ietf.org/html/rfc3629



//
//  Encoding Tags
// 
namespace json { namespace unicode {
    
#pragma mark - Encoding Tags        
    
    struct encoding_tag {};
    
    struct utf_encoding_tag :       encoding_tag {};

    struct UTF_8_encoding_tag :     utf_encoding_tag {};
    struct UTF_16_encoding_tag :    utf_encoding_tag {}; 
    struct UTF_16BE_encoding_tag :  UTF_16_encoding_tag {};
    struct UTF_16LE_encoding_tag :  UTF_16_encoding_tag {};
    struct UTF_32_encoding_tag :    utf_encoding_tag {};
    struct UTF_32BE_encoding_tag :  UTF_32_encoding_tag {};
    struct UTF_32LE_encoding_tag :  UTF_32_encoding_tag {};
    
    struct platform_encoding_tag :  encoding_tag {};
    
    
}}    


//
// Unicode Traits
//
namespace json { namespace unicode {
    
#pragma mark - encoding_traits synopsis        
    
    
    using json::internal::host_endianness;
    using json::byte_swap;
    
    
    // Some definitions from the Unicode:
    // 
    //
    // Character Encoding Form. 
    // Mapping from a character set definition to the actual code units used to 
    // represent the data.
    //
    // Character Encoding Scheme. 
    // A character encoding form plus byte serialization. There are seven character 
    // encoding schemes in Unicode:  UTF-8, UTF-16, UTF-16BE, UTF-16LE, UTF-32, 
    // UTF-32BE, and UTF-32LE.
    

    /**
     Synopsis
     
    template <>
    struct encoding_traits<Encoding> 
    {
        // Is true for Unicode encodings, otherwise false
        static const bool is_unicode_encoding = true;
        
        // For an Unicode encdoding scheme 'Encoding', defines the type for the Unicode encoding form
        typedef UTF_16_encoding_tag     encoding_form;
        
        // Returns the corresponding Encoding constant
        static const Encoding       value = UnicodeEncoding;
        
        // Returns true if the encoding may encode a character using one or multiple code units to 
        // encode a character.
        static const bool           is_multi_code_unit  = true;
        
        // Returns the maximum number of bytes required to strore any encoded character
        static const unsigned int   max_bytes = K;
        
        // Returns the minum number of bytes to strore any encoded character
        static const unsigned int   min_bytes = K;
        
        // Defines the type for the code unit 
        typedef uint16_t            code_unit_type;
        
        // Returns the size of a buffer suitable to hold the number of code units for one 
        // encoded character.
        static const int            buffer_size = N;
        
        // Defines the buffer type for an encoded character
        typedef code_unit_type      encode_buffer_type[buffer_size];
        
        // Returns the name of the encoding as a pointer to a c-string
        static const char*          name();
        
        // Defines the type for a BOM
        typedef boost::array<code_unit_type,1>      bom_type;
        
        // Returns the BOM for the encoding
        static const bom_type       bom();
        
        // Returns the size in bytes for the BOM type
        static const int            bom_byte_size;
        
        
        // For a given code unit whose endianness equals the encoding's endianness, 
        // returns an integer type whose endianness equals the host endianness.
        static int  to_int(code_unit_type code_unit);

        // For a given code unit whose endianness equals the encoding's endianness, 
        // returns an unsigned integer type whose endianness equals the host endianness.
        static unsigned to_uint(code_unit_type v);
        
        
        
        // Returns true if the code unit whose endianness equals the encoding's endianness
        // can be identified as a single code unit, otherwise the code unit is possibly 
        // a trailing code unit, a start code unit or an invalid code unit.
        static bool is_single(code_unit_type code_unit);
        
        // Returns true if the code unit whose endianness equals the encoding's endianness 
        // can be identified as a "start code unit", otherwise the code unit is possibly 
        // a trailing, a single code unit or an invalid code unit.
        static bool is_lead(code_unit_type code_unit);
        
        // Returns true if the code unit whose endianness equals the encoding's endianness 
        // can be identified as a trailing code unit, otherwise the code unit is possibly 
        // a start code unit, a single code unit, or an invalid code unit.
        static bool is_trail(code_unit_type code_unit);
        
    };
     
    */
    
    //
    //  Constants for "Unicode encoding forms" and "Unicode encoding schemes"
    //  and "platform encoding".
    //
    enum Encoding {
        UnicodeEncoding_UTF8        = 1,
        UnicodeEncoding_UTF16,
        UnicodeEncoding_UTF16BE,
        UnicodeEncoding_UTF16LE,
        UnicodeEncoding_UTF32,
        UnicodeEncoding_UTF32BE,
        UnicodeEncoding_UTF32LE,
        PlatformEncoding = -1
    };
    
    
    template <typename EncodingT>
    struct encoding_traits {
        //  code_unit_type      The underlaying integer type representing an Unicode 
        //                      code unit. It is the minimal bit combination that
        //                      can represent a unit of encoded text for processing
        //                      or interchange.
        //                      The type of the scalar can be used to determine the
        //                      Encoding. Note that unicode::utilities requires the
        //                      types to be unsigned - whereas the unicode traits
        //                      uses the type 'char' for UTF-8, which is signed but
        //                      distinct from 'signed char'.
        //                      
        // 
        //
        //  encode_buffer_type  The minimal buffer required to store one encoded 
        //                      Unicode code point.
        //
        //  endian_tag          The endianess type property of the encoding.
        //
        //  bom_type            A static vector of values of code_unit_type which
        //                      is capable to store a BOM.
        //
        BOOST_STATIC_ASSERT(sizeof(EncodingT) == 0);   //      
    };
    
#pragma mark - encoding_traits<UTF_8_encoding_tag>
    
    // Note: UTF-8 encoding's endian_tag is defined and equals host endianness, 
    // as opposed to UTF-16 and UTF-32 which do not define an endian_tag. For
    // UTF-16 and UTF-32 endianness must be explicitly added. For UTF-8, adding
    // endianness would not be possible - thus it is fixed.
    // This treatment makes sense in applications where byte swapping is required 
    // and where endianness is deduced from the encoding. If UTF-8's endianness
    // will be deduced as "host endianness" byte swaping works as expected.
    
    template <>
    struct encoding_traits<UTF_8_encoding_tag> {
        static const bool is_unicode_encoding = true;
        typedef UTF_8_encoding_tag     encoding_form;
        static const Encoding       value = UnicodeEncoding_UTF8;
        static const bool           is_multi_code_unit  = true;
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 1;
        typedef char                code_unit_type;
        static const int            buffer_size = 4;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        typedef json::internal::host_endianness::type endian_tag;
        static const char*          name() { return "UTF-8"; }
        typedef boost::array<code_unit_type,3>      bom_type;
        static const bom_type       bom() { bom_type result = {{code_unit_type(0xEF), code_unit_type(0xBB), code_unit_type(0xBF)}}; return result; };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
        
        static int  to_int(code_unit_type cu)        { return static_cast<int>(static_cast<uint8_t>(cu)); }
        static unsigned to_uint(code_unit_type cu)   { return static_cast<unsigned>(static_cast<uint8_t>(cu)); }
        
        static bool is_single(code_unit_type cu)     { return utf8_is_single(cu); }
        static bool is_trail(code_unit_type cu)      { return utf8_is_trail(cu); }
        static bool is_lead(code_unit_type cu)       { return utf8_is_lead(cu); }
    };
    
    typedef encoding_traits<UTF_8_encoding_tag> UTF_8_encoding_traits;
    
    
#pragma mark - encoding_traits<UTF_16_encoding_tag>
    
    template <>
    struct encoding_traits<UTF_16_encoding_tag> {
        static const bool is_unicode_encoding = true;
        typedef UTF_16_encoding_tag     encoding_form;
        static const Encoding       value = UnicodeEncoding_UTF16;
        static const bool           is_multi_code_unit  = true;
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 2;
        typedef uint16_t            code_unit_type;
        static const int            buffer_size = 2;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        static const char*          name() { return "UTF-16"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{code_unit_type(0xFEFFu)}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
        
        static int  to_int(code_unit_type cu)       { return static_cast<int>(static_cast<uint16_t>(cu)); }
        static unsigned to_uint(code_unit_type cu)  { return static_cast<unsigned>(static_cast<uint16_t>(cu)); }

        // Parameter values must be in platform endianness!
        static bool is_single(code_unit_type cu)    { return utf16_is_single(cu); }
        static bool is_trail(code_unit_type cu)     { return utf16_is_trail(cu); }
        static bool is_lead(code_unit_type cu)      { return utf16_is_lead(cu); }        
    };
    
    typedef encoding_traits<UTF_16_encoding_tag> UTF_16_encoding_traits;
    
    
#pragma mark - encoding_traits<UTF_16BE_encoding_tag>
    
    template <>
    struct encoding_traits<UTF_16BE_encoding_tag> {
        static const bool is_unicode_encoding = true;
        typedef UTF_16_encoding_tag     encoding_form;
        static const Encoding       value = UnicodeEncoding_UTF16BE;
        static const bool           is_multi_code_unit  = true;
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 2;
        typedef uint16_t            code_unit_type;
        static const int            buffer_size = 2;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        
        typedef json::internal::big_endian_tag      endian_tag;
        static const char*          name() { return "UTF-16BE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() {
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);         
        
        static int  to_int(code_unit_type cu)       { 
            return static_cast<int>(static_cast<uint16_t>(byte_swap<endian_tag, host_endianness::type>(cu))); 
        }
        static unsigned to_uint(code_unit_type cu)  { 
            return static_cast<unsigned>(static_cast<uint16_t>(byte_swap<endian_tag, host_endianness::type>(cu))); 
        }
        
        static bool is_single(code_unit_type cu)    { return utf16_is_single(to_uint(cu)); }
        static bool is_trail(code_unit_type cu)     { return utf16_is_trail(to_uint(cu)); }
        static bool is_lead(code_unit_type cu)      { return utf16_is_lead(to_uint(cu)); }        
    };
    
    typedef encoding_traits<UTF_16BE_encoding_tag> UTF_16BE_encoding_traits;
    
    
    
#pragma mark - encoding_traits<UTF_16LE_encoding_tag>
    
    template <>
    struct encoding_traits<UTF_16LE_encoding_tag> {
        static const bool is_unicode_encoding = true;
        typedef UTF_16_encoding_tag     encoding_form;
        static const Encoding       value = UnicodeEncoding_UTF16LE;
        static const bool           is_multi_code_unit  = true;
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 2;
        typedef uint16_t            code_unit_type;
        static const int            buffer_size = 2;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        
        typedef json::internal::little_endian_tag   endian_tag;
        static const char*          name() { return "UTF-16LE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
        
        static int  to_int(code_unit_type cu)       { return static_cast<int>(static_cast<uint16_t>(byte_swap<endian_tag, host_endianness::type>(cu))); }
        static unsigned to_uint(code_unit_type cu)  { return static_cast<unsigned>(static_cast<uint16_t>(byte_swap<endian_tag, host_endianness::type>(cu))); }
        
        static bool is_single(code_unit_type cu)    { return utf16_is_single(to_uint(cu)); }
        static bool is_trail(code_unit_type cu)     { return utf16_is_trail(to_uint(cu)); }
        static bool is_lead(code_unit_type cu)      { return utf16_is_lead(to_uint(cu)); }        
    };
    
    typedef encoding_traits<UTF_16LE_encoding_tag> UTF_16LE_encoding_traits;
    
    
#pragma mark - encoding_traits<UTF_32_encoding_tag>
    
    template <>
    struct encoding_traits<UTF_32_encoding_tag> {
        static const bool is_unicode_encoding = true;
        typedef UTF_32_encoding_tag encoding_form;
        static const Encoding       value = UnicodeEncoding_UTF32;
        static const bool           is_multi_code_unit  = false;
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 4;
        typedef uint32_t            code_unit_type;
        static const int            buffer_size = 1;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        static const char*          name() { return "UTF-32"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{static_cast<code_unit_type>(0xFEFFu)}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
        
        static int  to_int(code_unit_type cu)       { return static_cast<int>(static_cast<uint32_t>(cu)); }
        static unsigned to_uint(code_unit_type cu)  { return static_cast<unsigned>(static_cast<uint32_t>(cu)); }
        
        static bool is_single(code_unit_type)       { return true; }
        static bool is_trail(code_unit_type)        { return false; }
        static bool is_lead(code_unit_type)         { return false; }        
    };
    
    typedef encoding_traits<UTF_32_encoding_tag> UTF_32_encoding_traits;
    
    
#pragma mark - encoding_traits<UTF_32BE_encoding_tag>
    
    template <>
    struct encoding_traits<UTF_32BE_encoding_tag> {
        static const bool is_unicode_encoding = true;
        typedef UTF_32_encoding_tag     encoding_form;
        static const Encoding       value = UnicodeEncoding_UTF32BE;
        static const bool           is_multi_code_unit  = false;
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 4;
        typedef uint32_t            code_unit_type;
        static const int            buffer_size = 1;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        
        typedef json::internal::big_endian_tag      endian_tag;
        static const char*          name() { return "UTF-32BE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
        
        static int  to_int(code_unit_type cu)       { 
            return static_cast<int>(static_cast<uint32_t>(byte_swap<endian_tag, host_endianness::type>(cu))); 
        }
        static unsigned to_uint(code_unit_type cu)  { 
            return static_cast<unsigned>(static_cast<uint32_t>(byte_swap<endian_tag, host_endianness::type>(cu))); 
        }
        
        static bool is_single(code_unit_type)       { return true; }
        static bool is_trail(code_unit_type)        { return false; }        
        static bool is_lead(code_unit_type)         { return false; }        
    };
    
    typedef encoding_traits<UTF_32BE_encoding_tag> UTF_32BE_encoding_traits;
    
    
    
#pragma mark - encoding_traits<UTF_32LE_encoding_tag>
    
    template <>
    struct encoding_traits<UTF_32LE_encoding_tag> {
        static const bool is_unicode_encoding = true;
        typedef UTF_32_encoding_tag     encoding_form;
        static const Encoding       value = UnicodeEncoding_UTF32LE;
        static const bool           is_multi_code_unit  = false;
        static const unsigned int   max_bytes = 4;
        static const unsigned int   min_bytes = 4;
        typedef uint32_t            code_unit_type;
        static const int            buffer_size = 1;
        typedef code_unit_type      encode_buffer_type[buffer_size];
        
        typedef json::internal::little_endian_tag   endian_tag;
        static const char*          name() { return "UTF-32LE"; }
        typedef boost::array<code_unit_type,1>      bom_type;
        static const bom_type       bom() { 
            bom_type result = {{byte_swap<host_endianness::type, endian_tag>(static_cast<code_unit_type>(0xFEFFu))}}; 
            return result; 
        };
        static const int            bom_byte_size  = bom_type::static_size*sizeof(code_unit_type);
        
        static int  to_int(code_unit_type cu)       { 
            return static_cast<int>(static_cast<uint32_t>(byte_swap<endian_tag, host_endianness::type>(cu))); 
        }
        static unsigned to_uint(code_unit_type cu)  { 
            return static_cast<unsigned>(static_cast<uint32_t>(byte_swap<endian_tag, host_endianness::type>(cu))); 
        }
        
        static bool is_single(code_unit_type)       { return true; }
        static bool is_trail(code_unit_type)        { return false; }
        static bool is_lead(code_unit_type)         { return false; }        
    };
    
    typedef encoding_traits<UTF_32LE_encoding_tag> UTF_32LE_encoding_traits;
    
    
    
    
#pragma mark - encoding_traits<platform_encoding_tag>
    
    template <>
    struct encoding_traits<platform_encoding_tag> {
        static const bool is_unicode_encoding = false;
        static const Encoding value = PlatformEncoding;
        
        static const bool           is_multi_code_unit  = false;
        
        static const unsigned int   max_bytes = sizeof(wchar_t);
        static const unsigned int   min_bytes = sizeof(wchar_t);
        typedef wchar_t             code_unit_type;
        static const int            buffer_size = 1;
        typedef wchar_t             encode_buffer_type[buffer_size];
        typedef host_endianness::type endian_tag;
        static const char*          name() { return "platform"; }

        static int  to_int(code_unit_type cu)       { return static_cast<int>(static_cast<uint32_t>(cu)); }
        static unsigned to_uint(code_unit_type cu)  { return static_cast<unsigned>(static_cast<uint32_t>(cu)); }
        
        static bool is_single(code_unit_type)       { return true; }
        static bool is_trail(code_unit_type)        { return false; }
        static bool is_lead(code_unit_type)         { return false; }        
    };
    
    typedef encoding_traits<platform_encoding_tag> platform_encoding_traits;
    
    
    
#pragma mark - encoding_traits<code_point_t>
    
    template <>
    struct encoding_traits<code_point_t> {
        static const bool is_unicode_encoding = false;
        static const Encoding value = PlatformEncoding;
        
        static const bool           is_multi_code_unit  = false;
        
        static const unsigned int   max_bytes = sizeof(code_point_t);
        static const unsigned int   min_bytes = sizeof(code_point_t);
        typedef code_point_t        code_unit_type;
        static const int            buffer_size = 1;
        typedef wchar_t             encode_buffer_type[buffer_size];
        typedef host_endianness::type endian_tag;
        static const char*          name() { return "Unicode code point"; }
        
        static int  to_int(code_unit_type cu)       { return static_cast<int>(static_cast<uint32_t>(cu)); }
        static unsigned to_uint(code_unit_type cu)  { return static_cast<unsigned>(static_cast<uint32_t>(cu)); }
        
        static bool is_single(code_unit_type)       { return true; }
        static bool is_trail(code_unit_type)        { return false; }
        static bool is_lead(code_unit_type)         { return false; }        
    };
    
    typedef encoding_traits<platform_encoding_tag> platform_encoding_traits;
    
    
}}


//
// Maps Encoding constants to corresponding type
//
namespace json { namespace unicode {
    
#pragma mark - encoding_to_tag

    
    template <int E>
    struct encoding_to_tag {};
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF8> {
        typedef UTF_8_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF16> {
        typedef UTF_16_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF16BE> {
        typedef UTF_16BE_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF16LE> {
        typedef UTF_16LE_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF32> {
        typedef UTF_32_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF32BE> {
        typedef UTF_32BE_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<UnicodeEncoding_UTF32LE> {
        typedef UTF_32LE_encoding_tag type;
    };
    
    template <>
    struct encoding_to_tag<PlatformEncoding> {
        typedef platform_encoding_tag type;
    };
    
    
    
}}

//
// to_host_endianness
// 
namespace json { namespace unicode {
#pragma mark -
#pragma mark to_host_endianness
    
    // "Upgrades" or converts an UTF-16, UTF-16LE, UTF-16BE, UTF-32, UTF-32LE, 
    // UTF-32BE, encoding to its corresponding encoding scheme whose endianness 
    // equals that of the host. 
    // For UTF-8, platform_encoding and code_poin_t it has no effect.

    using json::internal::host_endianness;
    using json::internal::little_endian_tag;
    using json::internal::big_endian_tag;
    
    
    
    template <typename EncodingT, typename Enable = void> 
    struct to_host_endianness {
        BOOST_STATIC_ASSERT(sizeof(Enable) == 0);
    };
    
    // Specialication for UTF-8
    template <typename EncodingT> 
    struct to_host_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_same<UTF_8_encoding_tag, EncodingT> >::type 
    >  
    {
        typedef EncodingT type;
    };
    
    
    // Specialication for UTF-16, UTF-16BE, UTF-16LE.    
    template <typename EncodingT> 
    struct to_host_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_base_of<UTF_16_encoding_tag, EncodingT> >::type
    >  
    {
        typedef typename boost::mpl::if_<
            boost::is_same<typename host_endianness::type, little_endian_tag>,
            UTF_16LE_encoding_tag,
            UTF_16BE_encoding_tag
        >::type  type;
    };
    
    // Specialization for UTF-32, UTF-32BE, UTF-32LE.    
    template <typename EncodingT> 
    struct to_host_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_base_of<UTF_32_encoding_tag, EncodingT> >::type 
    >  
    {
        typedef typename boost::mpl::if_<
            boost::is_same<typename host_endianness::type, little_endian_tag>,
            UTF_32LE_encoding_tag,
            UTF_32BE_encoding_tag
        >::type  type;
    };
    
    
    // Specialization for platform_encoding_tag
    template <typename EncodingT> 
        struct to_host_endianness<
        EncodingT, 
    typename boost::enable_if<boost::is_same<platform_encoding_tag, EncodingT> >::type 
    >  
    {
        typedef EncodingT type;
    };
    
    // Specialization for code_point_t
    template <typename EncodingT> 
    struct to_host_endianness<
        EncodingT, 
        typename boost::enable_if<boost::is_same<code_point_t, EncodingT> >::type 
    >  
    {
        typedef EncodingT type;
    };
    
}}    


//
// add_endianness
// 
namespace json { namespace unicode {
#pragma mark - add_endianness
    
    using json::internal::host_endianness;
    using json::internal::little_endian_tag;
    using json::internal::big_endian_tag;
    
    
    // "Upgrades" an UTF-16 or UTF-32 Unicode encoding form (without endianness)
    // to its corresponding Unicode encoding scheme whose endianness equals 
    // host endianness.
    // Encodings whose endianness is already specified or is not applicable 
    // will not be changed.
    
    template <typename EncodingT, typename Enable = void> 
    struct add_endianness {
        BOOST_STATIC_ASSERT(sizeof(Enable) == 0);
    };
    
    template <typename EncodingT> 
    struct add_endianness<
        EncodingT, 
        typename boost::enable_if<
            boost::mpl::or_<
                boost::is_same<UTF_16_encoding_tag, EncodingT>,
                boost::is_same<UTF_32_encoding_tag, EncodingT>
            >
        >::type
    >  
    {
        typedef typename to_host_endianness<EncodingT>::type type;
    };
    
    
    template <typename EncodingT> 
    struct add_endianness<
        EncodingT, 
        typename boost::enable_if<
            boost::mpl::or_<
                boost::is_same<UTF_8_encoding_tag, EncodingT>,
                boost::is_same<UTF_16BE_encoding_tag, EncodingT>,
                boost::is_same<UTF_16LE_encoding_tag, EncodingT>,
                boost::is_same<UTF_32BE_encoding_tag, EncodingT>,
                boost::is_same<UTF_32LE_encoding_tag, EncodingT>
            >
        >::type 
    >  
    {
        typedef EncodingT type;
    };
    
    
    template <typename EncodingT> 
    struct add_endianness<
        EncodingT, 
        typename boost::enable_if<
            boost::mpl::or_<
                boost::is_same<platform_encoding_tag, EncodingT>,
                boost::is_same<code_point_t, EncodingT>
            >
        >::type 
    >  
    {
        typedef EncodingT type;
    };
    
    
    
    
}}    


//
//  CharType to encoding
//
namespace json { namespace unicode {
#pragma mark - char_type_to_encoding
    
    
    // Return a default encoding in hostendianess from a given char type.
    template <typename CharT>
    struct char_type_to_encoding {
        BOOST_STATIC_ASSERT(sizeof(CharT) == 0);
    };
    
    template <>
    struct char_type_to_encoding<char> {
        typedef UTF_8_encoding_tag type;
    };
    
    template <>
    struct char_type_to_encoding<wchar_t> {
        typedef platform_encoding_tag type;
    };
    
    
#if __cplusplus > 199711L
    
    template <>
    struct char_type_to_encoding<char16_t> {
        typedef typename to_host_endianness<UTF_16_encoding_tag>::type type;
    };
    
    template <>
    struct char_type_to_encoding<char32_t> {
        typedef typename to_host_endianness<UTF_32_encoding_tag>::type type;
    };
    
    
#endif
    
}}

//
//  IteratorType to encoding
//
namespace json { namespace unicode {    
#pragma mark -
    
    // Determines a default encoding based on the iterator's value_type and
    // the machine endianness:        
    
    template <typename Iterator, typename ValueT = typename boost::iterator_value<Iterator>::type >
    struct iterator_encoding {
        // If the compiler issues an error here, the specialization
        // for the given template parameters does not exist.
        BOOST_STATIC_ASSERT(sizeof(Iterator) == 0); 
    };
    
    
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, char> {
        typedef UTF_8_encoding_tag type;
    };
    template <typename Iterator>
    struct iterator_encoding<Iterator, signed char> {
        typedef UTF_8_encoding_tag type;
    };
    template <typename Iterator>
    struct iterator_encoding<Iterator, unsigned char> {
        typedef UTF_8_encoding_tag type;
    };
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, wchar_t> {
        typedef platform_encoding_tag type;
    };
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, uint16_t> {
        typedef typename boost::mpl::if_c<
        json::internal::host_endianness::is_little_endian, 
        UTF_16LE_encoding_tag, 
        UTF_16BE_encoding_tag
        >::type type;
    };
    
    template <typename Iterator>
    struct iterator_encoding<Iterator, uint32_t> {
        typedef typename boost::mpl::if_c<
        json::internal::host_endianness::is_little_endian, 
        UTF_32LE_encoding_tag, 
        UTF_32BE_encoding_tag
        >::type type;
    };
    
    
    
    
    
    
    template <typename T, typename U>
    struct is_encoding {
        static const bool value = false;
    };
    
    template <>
    struct is_encoding<UTF_8_encoding_tag, UTF_8_encoding_tag> 
    {
        static const bool value = true;
    };
    template <>
    struct is_encoding<UTF_16BE_encoding_tag, UTF_16BE_encoding_tag> 
    {
        static const bool value = true;
    };
    template <>
    struct is_encoding<UTF_32LE_encoding_tag, UTF_32LE_encoding_tag> 
    {
        static const bool value = true;
    };
    template <>
    struct is_encoding<UTF_32BE_encoding_tag, UTF_32BE_encoding_tag> 
    {
        static const bool value = true;
    };
    
    
    
    
    // Stable. Do not change.
    // Several algorithms may be very picky about whether code units
    // are signed or unsigned. We check them here and set it in stone.
    // For UTF-8 all algorithms must work with either signed or usigned 
    // UTF-8 code units.    
    BOOST_STATIC_ASSERT((boost::is_unsigned<UTF_16_encoding_traits::code_unit_type>::value == true));
    BOOST_STATIC_ASSERT((boost::is_unsigned<UTF_32_encoding_traits::code_unit_type>::value == true));
    
    
}}    // namespace json::unicode




#endif  // JSON_UNICODE_TRAITS_HPP

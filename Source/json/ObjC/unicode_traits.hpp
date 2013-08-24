//
//  unicode_traits.hpp
//  
//
//  Created by Andreas Grosam on 7/1/11.
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

#ifndef JSON_OBJC_UNICODE_TRAITS_HPP
#define JSON_OBJC_UNICODE_TRAITS_HPP


#include "json/config.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/endian/endian.hpp"
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>


namespace json {
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;
    using json::unicode::to_host_endianness;
    
    
    
    //
    //  Maps json::unicode encoding types to NSStringEncoding constants
    //
    template <typename EncodingT>
    struct ns_unicode_encoding_traits {};
    
    template <>
    struct ns_unicode_encoding_traits<UTF_8_encoding_tag> {
        static constexpr NSStringEncoding value = NSUTF8StringEncoding;
    };
    
    template <>
    struct ns_unicode_encoding_traits<UTF_16BE_encoding_tag> {
        static constexpr NSStringEncoding value = NSUTF16BigEndianStringEncoding;
    };
    
    template <>
    struct ns_unicode_encoding_traits<UTF_16LE_encoding_tag> {
        static constexpr NSStringEncoding value = NSUTF16LittleEndianStringEncoding;
    };
    
    template <>
    struct ns_unicode_encoding_traits<UTF_32BE_encoding_tag> {
        static constexpr NSStringEncoding value = NSUTF32BigEndianStringEncoding;
    };
    
    template <>
    struct ns_unicode_encoding_traits<UTF_32LE_encoding_tag> {
        static constexpr NSStringEncoding value = NSUTF32LittleEndianStringEncoding;
    };
    

    
    //
    //  Maps json::unicode encoding types to CFStringEncoding constants
    //
    
//    kCFStringEncodingUnicode
//    kTextEncodingUnicodeDefault
    
    template <typename EncodingT>
    struct cf_unicode_encoding_traits {};
    
    template <>
    struct cf_unicode_encoding_traits<UTF_8_encoding_tag> {
        static constexpr CFStringEncoding value = kCFStringEncodingUTF8;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_16BE_encoding_tag> {
        static constexpr CFStringEncoding value = internal::host_endianness::is_big_endian ? kCFStringEncodingUnicode : kCFStringEncodingUTF16BE;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_16LE_encoding_tag> {
        static constexpr CFStringEncoding value = internal::host_endianness::is_little_endian ? kCFStringEncodingUnicode : kCFStringEncodingUTF16LE;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_32BE_encoding_tag> {
        static const CFStringEncoding value = kCFStringEncodingUTF32BE;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_32LE_encoding_tag> {
        static constexpr CFStringEncoding value = kCFStringEncodingUTF32LE;
    };
    
    
    
    
}





#endif

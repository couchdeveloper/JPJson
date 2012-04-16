//
//  main.c
//  CFStringStringBufferBench
//
//  Created by Andreas Grosam on 4/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include <iostream>

#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "json/parser/string_buffer.hpp"
#include "json/parser/string_chunk_storage.hpp"
#include "json/parser/string_buffer.hpp"

#include "utilities/timer.hpp"
#include "utilities/MinMaxAvg.hpp"


namespace bench {
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;
    using json::unicode::to_host_endianness;
    
    
    
    //
    //  Maps json::unicode encoding types to CFStringEncoding constants
    //
    template <typename EncodingT>
    struct cf_unicode_encoding_traits {};
    
    template <>
    struct cf_unicode_encoding_traits<UTF_8_encoding_tag> {
        static const CFStringEncoding value = kCFStringEncodingUTF8;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_16BE_encoding_tag> {
        static const CFStringEncoding value = kCFStringEncodingUTF16BE;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_16LE_encoding_tag> {
        static const CFStringEncoding value = kCFStringEncodingUTF16LE;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_16_encoding_tag> {
        typedef json::unicode::add_endianness<UTF_16_encoding_tag>::type encoding_t;
        static const CFStringEncoding value = cf_unicode_encoding_traits<encoding_t>::value;
    };
    
    
    template <>
    struct cf_unicode_encoding_traits<UTF_32BE_encoding_tag> {
        static const CFStringEncoding value = kCFStringEncodingUTF32BE;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_32LE_encoding_tag> {
        static const CFStringEncoding value = kCFStringEncodingUTF32LE;
    };
    
    template <>
    struct cf_unicode_encoding_traits<UTF_32_encoding_tag> {
        typedef json::unicode::add_endianness<UTF_32_encoding_tag>::type encoding_t;
        static const CFStringEncoding value = cf_unicode_encoding_traits<encoding_t>::value;
    };
    
    
    
}


namespace bench {
    
    using namespace json;
    
    using unicode::encoding_traits;
    
    // A mock for a Semantic Actions class
    template <typename EncodingT>
    class SemanticActions
    {
    public:
        typedef EncodingT                                               encoding_t;
        typedef typename encoding_traits<EncodingT>::code_unit_type     char_t;        
        
        SemanticActions()
        : start_(true), count_(0), small_str_(NULL), large_str_(NULL)
        {}
        
        ~SemanticActions() 
        {
            if (small_str_) {
                CFRelease(small_str_);
            }
            if (large_str_) {
                CFRelease(large_str_);
            }
        }
        
    public:
        void value_string_write(const char_t* s, std::size_t len, bool hasMore) 
        { 
            const CFStringEncoding encoding = bench::cf_unicode_encoding_traits<encoding_t>::value;
            
            if (start_) {
                count_ = 0;
                if (small_str_) {
                    CFRelease(small_str_);
                    small_str_ = NULL;
                }
                if (large_str_) {
                    CFRelease(large_str_);
                    large_str_ = NULL;
                }
            }
                        
            if (start_ and not hasMore) {
                // create a small immutable string:
                assert(small_str_ == NULL);
                assert(large_str_ == NULL);
                small_str_ = CFStringCreateWithBytes(kCFAllocatorDefault, to_bytes(s), sizeof(char_t)*len, encoding, false);
            }
            else if (start_ and hasMore)
            {
                // create a large mutable string:
                assert(small_str_ == NULL);
                assert(large_str_ == NULL);
                CFStringRef tmp = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault, to_bytes(s), sizeof(char_t)*len, 
                                                                encoding,
                                                                false, kCFAllocatorNull);
                large_str_ = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, tmp);
                CFRelease(tmp);
            }
            else if (not start_) {
                // append to the large mutable string:
                assert(small_str_ == NULL);
                assert(large_str_ != NULL);
                CFStringRef tmp = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault, to_bytes(s), sizeof(char_t)*len, 
                                                                encoding,
                                                                false, kCFAllocatorNull);
                CFStringAppend((CFMutableStringRef)(large_str_), tmp);
                CFRelease(tmp);
            }
            
            count_ += len;
            start_ = not hasMore;
        }      
        
        
        CFStringRef str() const { return small_str_ ? small_str_ : (CFStringRef)large_str_ ; }
        
    private:
        template <typename T>
        const UInt8*
        to_bytes(const T* p) {
            return static_cast<UInt8 const*>(static_cast<const void*>(p));
        }
        
    private:
        bool start_;
        CFStringRef        small_str_;  
        CFMutableStringRef large_str_; 
        size_t count_;
    };
    
}



namespace {
    
    using namespace json;
    
    using parser_internal::string_chunk_storage;
    using parser_internal::string_buffer;
    using utilities::timer;
    using utilities::MinMaxAvg;
    
    
    template <typename SB_EncodingT, typename SA_EncodingT>
    void bench1(size_t const N, size_t const S, size_t times, size_t const B, SB_EncodingT, SA_EncodingT) 
    {
        const std::string sb_encoding_name = unicode::encoding_traits<SB_EncodingT>::name();
        const std::string sa_encoding_name = unicode::encoding_traits<SA_EncodingT>::name();
        
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running CFStringStringBuffer bench %lu times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Input characters:          ASCII\n"
               "Input size:                %lu (%lu times)\n"
               "String buffer type:        chunk storage\n"
               "String buffer size:        %lu bytes\n"
               "String buffer encoding:    %s\n"
               "Semantic actions encoding: %s\n", S, times, B, sb_encoding_name.c_str(), sa_encoding_name.c_str());
        
        
        typedef bench::SemanticActions<SA_EncodingT>            sa_t;
        typedef string_chunk_storage<SB_EncodingT, sa_t>       storage_t;    
        typedef string_buffer<storage_t>                        string_buffer_t;
        typedef typename string_buffer_t::buffer_type           buffer_t;
        typedef typename string_buffer_t::code_unit_type        char_t;         
        
        
        sa_t sa;
        storage_t storage(sa, B/sizeof(char_t));
        string_buffer_t string_buffer(storage);
        
        const char* ascii = "01234567890ABCDEF";
        const size_t len = strlen(ascii);
        
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        for (int n = 0; n < N; ++n)
        {
            t.start();
            for (int j = 0; j < times; ++j)
            {
                for (int i = 0; i < S; ++i) {
                    string_buffer.append_ascii(ascii[i%len]);
                }
                storage.flush();            
            }
            t.stop();
            te.set(t.seconds());
            t.reset();
        }
        
        printf("string_buffer: \nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
               te.min()*1e3, te.max()*1e3, te.avg()*1e3);
    }   
    
    
    
    template <typename SB_EncodingT, typename SA_EncodingT>
    void bench2(size_t const N, size_t const S, size_t times, size_t const B, SB_EncodingT, SA_EncodingT) 
    {
        const std::string sb_encoding_name = unicode::encoding_traits<SB_EncodingT>::name();
        const std::string sa_encoding_name = unicode::encoding_traits<SA_EncodingT>::name();
        
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running CFStringStringBuffer bench %lu times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Input characters:          Unicode\n"
               "Input size:                %lu  (%lu times)\n"
               "String buffer type:        chunk storage\n"
               "String buffer size:        %lu bytes\n"
               "String buffer encoding:    %s\n"
               "Semantic actions encoding: %s\n", S, times, B, sb_encoding_name.c_str(), sa_encoding_name.c_str());
        
        
        typedef bench::SemanticActions<SA_EncodingT>            sa_t;
        typedef string_chunk_storage<SB_EncodingT, sa_t>       storage_t;    
        typedef string_buffer<storage_t>                        string_buffer_t;
        typedef typename string_buffer_t::buffer_type           buffer_t;
        typedef typename string_buffer_t::code_unit_type        char_t;         
        
        
        sa_t sa;
        storage_t storage(sa, B/sizeof(char_t));
        string_buffer_t string_buffer(storage);
        
        
        
        const unicode::code_point_t unicodes[] = {
            0x60,
            0x61,
            0x62,
            0x63,
            0x64,
            0x65,
            0x66,
            0x67,
            0x7f0,
            0x7f1,
            0x7f2,
            0xA000,
            0xA001,
            0xA003,
            0x100f0,
        };
        const size_t len = sizeof(unicodes);
        
        
        MinMaxAvg<double> te;
        timer t = timer();
        
        for (int n = 0; n < N; ++n)
        {
            t.start();
            for (int j = 0; j < times; ++j) 
            {
                for (int i = 0; i < S; ++i) {
                    string_buffer.append_unicode(unicodes[i%len]);
                }
                storage.flush();            
            }
            t.stop();
            te.set(t.seconds());
            t.reset();
            
        }
        printf("string_buffer: \nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
               te.min()*1e3, te.max()*1e3, te.avg()*1e3);
    }   
    
    
    
}



int main(int argc, const char * argv[])
{
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    
#if defined (DEBUG)
    const int N = 10;
#else
    const int N = 10;
#endif    
    
    const size_t S = 100000;
    const size_t buffer_size = 8*1024; // (in bytes)
    const size_t times = 1;
    
//    bench1(N, 32, 1000, buffer_size, UTF_32_encoding_tag(), UTF_16_encoding_tag());
//    bench1(N, 32, 1000, buffer_size, UTF_32_encoding_tag(), UTF_8_encoding_tag());
//    bench1(N, 32, 1000, buffer_size, UTF_16_encoding_tag(), UTF_16_encoding_tag());
//    bench1(N, 32, 1000, buffer_size, UTF_8_encoding_tag(), UTF_8_encoding_tag());
//
//    
//    bench2(N, 32, 1000, buffer_size, UTF_32_encoding_tag(), UTF_16_encoding_tag());
//    bench2(N, 32, 1000, buffer_size, UTF_32_encoding_tag(), UTF_8_encoding_tag());
//    bench2(N, 32, 1000, buffer_size, UTF_16_encoding_tag(), UTF_16_encoding_tag());
//    bench2(N, 32, 1000, buffer_size, UTF_8_encoding_tag(), UTF_8_encoding_tag());

    
    
    // Large Strings ASCII
    bench1(N, S, times, buffer_size, UTF_32_encoding_tag(), UTF_16_encoding_tag());
    bench1(N, S, times, buffer_size, UTF_32_encoding_tag(), UTF_8_encoding_tag());
    bench1(N, S, times, buffer_size, UTF_16_encoding_tag(), UTF_16_encoding_tag());
    bench1(N, S, times, buffer_size, UTF_8_encoding_tag(), UTF_8_encoding_tag());
    
    // Large Strings Unicode
    bench2(N, S, times, buffer_size, UTF_32_encoding_tag(), UTF_16_encoding_tag());
    bench2(N, S, times, buffer_size, UTF_32_encoding_tag(), UTF_8_encoding_tag());
    bench2(N, S, times, buffer_size, UTF_16_encoding_tag(), UTF_16_encoding_tag());
    bench2(N, S, times, buffer_size, UTF_8_encoding_tag(), UTF_8_encoding_tag());
    
    
    return 0;
}


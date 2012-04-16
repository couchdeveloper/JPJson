//
//  main.cpp
//  string_buffer_bench
//
//  Created by Andreas Grosam on 4/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "json/parser/string_buffer.hpp"
#include "json/parser/string_chunk_storage.hpp"
#include "json/parser/string_buffer.hpp"

#include "utilities/timer.hpp"
#include "utilities/MinMaxAvg.hpp"


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
        typedef std::vector<char_t>                                     vector_t;
        
        typedef std::pair<char_t*, size_t>                  buffer_t;
        typedef std::pair<char_t const*, size_t>            const_buffer_t;
        
        
        SemanticActions()
        : start_(true), count_(0)
        {}
        
    public:
        void value_string_write(const_buffer_t& buffer, bool hasMore) { 
            if (start_) {
                count_ = 0;
                str_.clear();
            }
            str_.insert(str_.end(), buffer.first, buffer.first + buffer.second);
            count_ += buffer.second;
            start_ = not hasMore;
        }      
        
        
        buffer_t str() const { return str_; }
        
    private:
        bool start_;
        vector_t str_;
        size_t count_;
    };
    
}



namespace {

    using namespace json;
    
    using parser_internal::string_chunk_storage;
    using parser_internal::string_buffer;
    using utilities::timer;
    using utilities::MinMaxAvg;
    

    template <typename SA_EncodingT, typename SB_Encoding_T>
    void bench1(size_t const N, size_t const S, size_t const B, SA_EncodingT, SB_Encoding_T) 
    {
        const std::string sa_encoding_name = unicode::encoding_traits<SA_EncodingT>::name();
        const std::string sb_encoding_name = unicode::encoding_traits<SB_Encoding_T>::name();
        
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running string_buffer bench %lu times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Input characters:          ASCII\n"
               "Input size:                %lu\n"
               "String buffer type:        chunk storage\n"
               "String buffer size:        %lu bytes\n"
               "String buffer encoding:    %s\n"
               "Semantic actions encoding: %s\n", S, B, sb_encoding_name.c_str(), sa_encoding_name.c_str());
        
        
        typedef bench::SemanticActions<SA_EncodingT>            sa_t;
        typedef string_chunk_storage<SB_Encoding_T, sa_t>       storage_t;    
        typedef string_buffer<storage_t>                        string_buffer_t;
        typedef typename string_buffer_t::buffer_type           buffer_t;
        typedef typename string_buffer_t::code_unit_type        char_t;         
        typedef typename sa_t::buffer_t                         char_vector_t;
        
        
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
            for (int i = 0; i < S; ++i) {
                string_buffer.append_ascii(ascii[i%len]);
            }
            storage.flush();            
            t.stop();
            te.set(t.seconds());
            t.reset();
        }
        
        printf("string_buffer: \nmin: %.3f ms, max: %0.3f ms, avg: %0.3f ms\n", 
               te.min()*1e3, te.max()*1e3, te.avg()*1e3);
    }   
        


    template <typename SA_EncodingT, typename SB_Encoding_T>
    void bench2(size_t const N, size_t const S, size_t const B, SA_EncodingT, SB_Encoding_T) 
    {
        const std::string sa_encoding_name = unicode::encoding_traits<SA_EncodingT>::name();
        const std::string sb_encoding_name = unicode::encoding_traits<SB_Encoding_T>::name();
        
        
        
        printf("\n");
        printf("--------------------------------------------\n");
        printf("Running string_buffer bench %lu times.\n", N);
        printf("--------------------------------------------\n");    
        printf("Input characters:          Unicode\n"
               "Input size:                %lu\n"
               "String buffer type:        chunk storage\n"
               "String buffer size:        %lu bytes\n"
               "String buffer encoding:    %s\n"
               "Semantic actions encoding: %s\n", S, B, sb_encoding_name.c_str(), sa_encoding_name.c_str());
        
        
        typedef bench::SemanticActions<SA_EncodingT>            sa_t;
        typedef string_chunk_storage<SB_Encoding_T, sa_t>       storage_t;    
        typedef string_buffer<storage_t>                        string_buffer_t;
        typedef typename string_buffer_t::buffer_type           buffer_t;
        typedef typename string_buffer_t::code_unit_type        char_t;         
        typedef typename sa_t::buffer_t                         char_vector_t;
        
        
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
            for (int i = 0; i < S; ++i) {
                string_buffer.append_unicode(unicodes[i%len]);
            }
            storage.flush();            
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
    const int N = 100;
#else
    const int N = 100;
#endif    
    
    const size_t S = 100000;
    const size_t buffer_size = 1024; // (in bytes)
    
    bench1(N, S, buffer_size, UTF_8_encoding_tag(), UTF_32_encoding_tag());
    bench1(N, S, buffer_size, UTF_8_encoding_tag(), UTF_8_encoding_tag());
    bench1(N, S, buffer_size, UTF_16_encoding_tag(), UTF_16_encoding_tag());
    
    bench2(N, S, buffer_size, UTF_8_encoding_tag(), UTF_32_encoding_tag());
    bench2(N, S, buffer_size, UTF_8_encoding_tag(), UTF_8_encoding_tag());
    bench2(N, S, buffer_size, UTF_16_encoding_tag(), UTF_16_encoding_tag());
    
    
    return 0;
}


//
//  main.cpp
//  unicode_conversion
//
//  Created by Andreas Grosam on 8/16/11.
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

#include <iostream>
#include <vector>
#include <iterator>
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_conversions.hpp"
#include "utilities/timer.hpp"

// ICU
#include <unicode/ustring.h>
#include <unicode/utf.h>
#include <unicode/utf8.h>
#include <unicode/utf16.h>
#include <unicode/ucnv.h>


namespace {
    
    std::vector<char> createSource_UTF8(std::size_t N, bool randomize,
                                      double c0 = 0.6, 
                                      double c1 = 0.25,
                                      double c2 = 0.1,
                                      double c3 = 0.05) 
    {
        using json::unicode::code_point_t;
        using json::unicode::UTF_8_encoding_tag;
        using json::unicode::UTF_32_encoding_tag;
        
        // UTF-8 to code point 
        // Fill a buffer with 10e6 characters with 
        // - 60% single byte, 
        // - 25% two bytes
        // - 10% three bytes,
        // -  5% four bytes
        
        // normalize:
        double C = c0 + c1 + c2 + c3;
        c0 = c0/C;
        c1 = c1/C;
        c2 = c2/C;
        c3 = c3/C;
        
        std::vector<code_point_t> source;
        source.reserve(N);
        
        std::size_t d0 = (N * c0) + 0.5; 
        std::size_t d1 = (N * c1) + 0.5; 
        std::size_t d2 = (N * c2) + 0.5; 
        std::size_t d3 = (N * c3) + 0.5;
        
        for (std::size_t i = 0; i < d0; ++i) {
            source.push_back(static_cast<code_point_t>('A'));
        }
        
        for (std::size_t i = 0; i < d1; ++i) {
            source.push_back(0x00A2u);  // '¢'
        }
        
        for (std::size_t i = 0; i < d2; ++i) {
            source.push_back(0x20ACu); // '€'
        }
        
        for (std::size_t i = 0; i < d3; ++i) {
            source.push_back(0x024B62u); // '𤭢'
        }
        
        if (randomize) {
            std::random_shuffle(source.begin(), source.end());
        }
        
        std::vector<char> result;        
        int error;
        std::vector<code_point_t>::iterator first = source.begin();
        std::back_insert_iterator<std::vector<char> > dest = std::back_inserter(result);
        size_t count = json::unicode::convert(first, source.end(), dest, UTF_8_encoding_tag(), error);        
        assert(error == 0);
        assert(count = result.size());
        
        return result;
    }
    
    
    double bench_utf8_to_codepoint(std::size_t N, bool randomize,
                  double c0 = 0.6, 
                  double c1 = 0.25,
                  double c2 = 0.1,
                  double c3 = 0.05) 
    {
        using json::unicode::code_point_t;
        using json::unicode::convert_one;
        using json::unicode::convert_one_unsafe;
        using utilities::timer;
        
        std::vector<char> source = createSource_UTF8(N, randomize, c0, c1, c2, c3);
        
        std::vector<code_point_t> dest;
        dest.reserve(N+1);
        
        std::vector<char>::iterator first = source.begin();
        std::vector<char>::iterator last = source.end();
        
        timer t0;
        
        t0.start();
        while (first != last) {
            code_point_t code_point;
            int result = convert_one(first, last, code_point);
            if (result > 0) {
                dest.push_back(code_point);
            } else {
                //break;
                throw std::logic_error("conversion failed");
            }
            assert(first <= last);
        }  
        t0.stop();
        
#if defined (DEBUG)        
        std::cout <<  dest.size() << " converted." << std::endl;
#endif        
        return t0.seconds();
        
    }
    
    

    double bench_utf8_to_utf32(std::size_t N, bool randomize,
                                   double c0 = 0.6, 
                                   double c1 = 0.25,
                                   double c2 = 0.1,
                                   double c3 = 0.05) 
    {
        using json::unicode::code_point_t;
        using json::unicode::convert_one;
        using json::unicode::UTF_32_encoding_tag;
        using json::unicode::to_host_endianness;
        using utilities::timer;
        

        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef UTF32_Host_Encoding::code_unit_type code_unit_t;
        
        std::vector<char> source = createSource_UTF8(N, randomize, c0, c1, c2, c3);
        
        typedef std::vector<code_unit_t> target_buffer_t;
        typedef std::back_insert_iterator<target_buffer_t> back_insert_iterator_t;
        target_buffer_t target;
        back_insert_iterator_t back_inserter = back_insert_iterator_t(target);
        target.reserve(N+1);
        
        std::vector<char>::iterator first = source.begin();
        std::vector<char>::iterator last = source.end();
        
        timer t0;
        
        t0.start();
        while (first != last) {
            int result = convert_one(first, last, back_inserter, UTF32_Host_Encoding());
            if (result <= 0) {
                throw std::logic_error("conversion failed");
            }
            assert(first <= last);
        }  
        t0.stop();
        
#if defined (DEBUG)        
        std::cout <<  N << " converted." << std::endl;
#endif        
        return t0.seconds();
        
    }
    
    
    
    double bench_utf8_to_codepoint_ref (std::size_t N, bool randomize,
                  double c0 = 0.6, 
                  double c1 = 0.25,
                  double c2 = 0.1,
                  double c3 = 0.05) 
    {
        using json::unicode::code_point_t;
        using utilities::timer;
        
        std::vector<char> source = createSource_UTF8(N, randomize, c0, c1, c2, c3);
        
        std::vector<code_point_t> dest;
        dest.reserve(N+1);
        
        std::vector<char>::iterator first = source.begin();
        std::vector<char>::iterator last = source.end();
        
        timer t0;
        
        t0.start();
        std::copy(first, last, std::back_inserter(dest));
        t0.stop();
        
#if defined (DEBUG)        
        std::cout <<  dest.size() << " copied." << std::endl;
#endif        
        return t0.seconds();
        
    }
    
    
    double bench_utf8_to_codepoint_ICU(std::size_t N, bool randomize,
                     double c0 = 0.6, 
                     double c1 = 0.25,
                     double c2 = 0.1,
                     double c3 = 0.05) 
    {
        using json::unicode::to_host_endianness;
        using json::unicode::UTF_32_encoding_tag;
        using json::unicode::UTF_16_encoding_tag;
        using utilities::timer;

        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        std::vector<char> source_buffer = createSource_UTF8(N, randomize, c0, c1, c2, c3);
        int32_t source_length = (int)source_buffer.size()/sizeof(uint8_t);
        const char* source = reinterpret_cast<const char*>(&(source_buffer[0]));
        
        int32_t target_size = (int32_t)(N+1)*sizeof(uint32_t);
        char* target = (char*)malloc(target_size);
        
        timer t0;
        
        UErrorCode error = U_ZERO_ERROR;
        t0.start();
        int32_t icu_output_bytes = 
            ucnv_convert(UTF32_Host_Encoding().name() /*to*/, "UTF-8" /*from*/, 
                     target, target_size, 
                     source,  source_length, &error); 
        t0.stop();
        
        assert(error == U_ZERO_ERROR);
#if defined (DEBUG)        
        std::cout <<  icu_output_bytes/sizeof(uint32_t) << " ICU converted." << std::endl;
#endif        
        free(target);
        return t0.seconds();
        
    }
    
    
    
    void bench_utf8_to_codepoint_ref() {
        printf("-------------------------------------------\n");        
        printf(" Copy 1e6 single bytes to Unicode code point\n");
        printf(" for comparison reasons \n");
        printf("-------------------------------------------\n");        
        
        const std::size_t N = 1000*1000;
        double elapsedTime = bench_utf8_to_codepoint_ref(N, false, 1, 0, 0, 0);
        printf("UTF-8 to Unicode code point (copied): elapsed time:  %.3f ms\n", elapsedTime*1000);        
    }
    
    void bench_utf8_to_codepoint_0() {
        printf("-------------------------------------------\n");        
        printf(" Converting 1e6 UTF-8 characters to Unicode\n");
        printf(" 100%% single bytes.\n");
        printf("-------------------------------------------\n");        
        
        const std::size_t N = 1000*1000;
        double elapsedTime = bench_utf8_to_codepoint(N, false, 100, 0, 0, 0);
        printf("json::unicode: elapsed time:  %.3f ms\n", elapsedTime*1000); 
        
        elapsedTime = bench_utf8_to_codepoint_ICU(N, false, 100, 0, 0, 0);        
        printf("ICU: elapsed time:            %.3f ms\n", elapsedTime*1000);         
    }
    
    void bench_utf8_to_codepoint_1() {
        printf("-------------------------------------------\n");        
        printf(" Converting 1e6 UTF-8 characters to Unicode\n");
        printf(" 100%% double bytes.\n");
        printf("-------------------------------------------\n");        
        
        const std::size_t N = 1000*1000;
        double elapsedTime = bench_utf8_to_codepoint(N, false, 0, 100, 0, 0);
        printf("json::unicode: elapsed time:  %.3f ms\n", elapsedTime*1000); 

        elapsedTime = bench_utf8_to_codepoint_ICU(N, false, 0, 100, 0, 0);
        printf("ICU: elapsed time:            %.3f ms\n", elapsedTime*1000);         
    }
    
    void bench_utf8_to_codepoint_2() {
        printf("-------------------------------------------\n");        
        printf(" Converting 1e6 UTF-8 characters to Unicode\n");
        printf(" 100%% triple bytes.\n");
        printf("-------------------------------------------\n");        
        
        const std::size_t N = 1000*1000;
        double elapsedTime = bench_utf8_to_codepoint(N, false, 0, 0, 100, 0);
        printf("json::unicode: elapsed time:  %.3f ms\n", elapsedTime*1000); 
        elapsedTime = bench_utf8_to_codepoint_ICU(N, false, 0, 0, 100, 0);
        printf("ICU: elapsed time:            %.3f ms\n", elapsedTime*1000);         
    }
    
    void bench_utf8_to_codepoint_3() {
        printf("-------------------------------------------\n");        
        printf(" Converting 1e6 UTF-8 characters to Unicode\n");
        printf(" 100%% quad bytes.\n");
        printf("-------------------------------------------\n");        
        
        const std::size_t N = 1000*1000;
        double elapsedTime = bench_utf8_to_codepoint(N, false, 0, 0, 0, 100);
        printf("json::unicode: elapsed time:  %.3f ms\n", elapsedTime*1000); 
        elapsedTime = bench_utf8_to_codepoint_ICU(N, false, 0, 0, 0, 100);
        printf("ICU: elapsed time:            %.3f ms\n", elapsedTime*1000);         
    }
    
    void bench_utf8_to_codepoint_4() {
        printf("-------------------------------------------\n");        
        printf(" Converting 1e6 UTF-8 characters to Unicode\n");
        printf(" 60%% single, 25%% double, 10%% triple and \n");
        printf(" 5%% quad bytes. \n");
        printf("-------------------------------------------\n");        
        
        const std::size_t N = 1000*1000;
        printf("Sequencial partitioned characters:\n");
        double elapsedTime = bench_utf8_to_codepoint(N, false, 60, 25, 10, 5);
        printf("json::unicode: elapsed time:  %.3f ms\n", elapsedTime*1000); 
        elapsedTime = bench_utf8_to_codepoint_ICU(N, false, 60, 25, 10, 5);
        printf("ICU: elapsed time:            %.3f ms\n", elapsedTime*1000); 
        
        printf("Random distributed characters:\n");
        elapsedTime = bench_utf8_to_codepoint(N, true, 60, 25, 10, 5);
        printf("json::unicode: elapsed time:  %.3f ms\n", elapsedTime*1000); 
        elapsedTime = bench_utf8_to_codepoint_ICU(N, true, 60, 25, 10, 5);
        printf("ICU: elapsed time:            %.3f ms\n", elapsedTime*1000); 
    }
    
    void bench_utf8_to_utf32() {
        printf("-------------------------------------------\n");        
        printf(" Converting 1e6 UTF-8 characters to UTF-32\n");
        printf(" 60%% single, 25%% double, 10%% triple and \n");
        printf(" 5%% quad bytes. \n");
        printf("-------------------------------------------\n");        
        
        const std::size_t N = 1000*1000;
        double elapsedTime = bench_utf8_to_utf32(N, false, 60, 25, 10, 5);
        printf("UTF-8 to Unicode code point: elapsed time:  %.3f ms\n", elapsedTime*1000);        
    }
    
    
    
    
    
    
    void bench_UTF8_to_codepoint() 
    {
        std::cout << "=====================================" << std::endl;
        
        bench_utf8_to_codepoint_ref();
        printf("\n\n\n");
        bench_utf8_to_codepoint_ref();
        printf("\n");
        bench_utf8_to_codepoint_0();
        printf("\n");
        bench_utf8_to_codepoint_1();
        printf("\n");
        bench_utf8_to_codepoint_2();
        printf("\n");
        bench_utf8_to_codepoint_3();
        printf("\n");
        bench_utf8_to_codepoint_4();
        printf("\n");
        //bench_utf8_to_utf32();
        //printf("\n");
    }
    
    
    double convert(const std::vector<char>& source, std::vector<json::unicode::code_point_t>& dest) 
    {
        using json::unicode::UTF_32_encoding_tag;
        using json::unicode::to_host_endianness;
        using json::unicode::code_point_t;
        using utilities::timer;
        
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        
        
        std::vector<char>::const_iterator first = source.begin();
        std::vector<char>::const_iterator last = source.end();
        
        timer t0;
        t0.start();
        
        json::unicode::filter::NoncharacterOrNULL filter;
        
        while (first != last) {
            code_point_t code_point;
            int result = json::unicode::convert_one(first, last, code_point, filter);
            if (result > 0) {
                dest.push_back(code_point);
            } else {
                //break;
                throw std::logic_error("conversion failed");
            }
            assert(first <= last);
        }  
        t0.stop();
        
        return t0.seconds();
    }
    
    double convert_ICU(const std::vector<char>& source, char* dest, int32_t dest_size)
    {
        using json::unicode::to_host_endianness;
        using json::unicode::UTF_32_encoding_tag;
        using json::unicode::UTF_16_encoding_tag;
        using utilities::timer;
        
        typedef to_host_endianness<UTF_32_encoding_tag>::type UTF32_Host_Encoding;
        typedef to_host_endianness<UTF_16_encoding_tag>::type UTF16_Host_Encoding;
        
        int32_t source_length = (int32_t)source.size();
        const char* source_buffer = reinterpret_cast<const char*>(&(source[0]));
                
        timer t0;        
        UErrorCode error = U_ZERO_ERROR;
        t0.start();
        int32_t icu_output_bytes = 
        ucnv_convert(UTF32_Host_Encoding().name() /*to*/, "UTF-8" /*from*/, 
                     dest, dest_size, 
                     source_buffer, source_length, &error); 
        t0.stop();
        
        assert(error == U_ZERO_ERROR);
#if defined (DEBUG)        
        std::cout <<  icu_output_bytes/sizeof(uint32_t) << " ICU converted." << std::endl;
#endif        
        return t0.seconds();
    }
    
    void profile() 
    {
        using json::unicode::code_point_t;
        
        const int N = 1000*1000;
        const int K = 100;
        
        std::vector<char> input = createSource_UTF8(N, true, 60, 25, 10, 5);
        std::vector<code_point_t> result;
        
        
        double elapsedTime = 0;
        for (int i = 0; i < K; ++i) {
            elapsedTime += convert(input, result);
            result.clear();
        }
        printf("UTF-8 to Unicode code point: elapsed time:  %.3f ms\n", elapsedTime*(1000.0/K));        
    }
        
    
    void profile_ICU() 
    {
        using json::unicode::code_point_t;
        
        const int N = 1000*1000;
        const int K = 100;
        
        std::vector<char> input = createSource_UTF8(N, true, 60, 25, 10, 5);
        
        const int32_t dest_size = (N+1)*sizeof(code_point_t);
        char* dest = (char*)malloc(dest_size);

        double elapsedTime = 0;
        for (int i = 0; i < K; ++i) {
            elapsedTime += convert_ICU(input, dest, dest_size);
        }
        printf("UTF-8 to Unicode code point ICU: elapsed time:  %.3f ms\n", elapsedTime*(1000.0/K));                
    }
    
}







int main (int argc, const char * argv[])
{
    try {
        //profile();
        bench_UTF8_to_codepoint();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return -1;
    }
        
    return 0;
}


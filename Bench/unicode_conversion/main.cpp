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

#include "json/unicode/unicode_conversion.hpp"
#include "json/unicode/unicode_traits.hpp"

#include "utilities/timer.hpp"
#include "utilities/MinMaxAvg.hpp"

#include <iostream>
#include <vector>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <cassert>
#include <ctime>

// ICU
#include <unicode/ustring.h>
#include <unicode/utf.h>
#include <unicode/utf8.h>
#include <unicode/utf16.h>
#include <unicode/ucnv.h>


#if defined (DEBUG)
//    #define LOGDEBUG
#endif

using namespace json;

#pragma mark Hm
namespace hoehrmann {
    
    using unicode::code_point_t;
    
    const uint8_t UTF8_ACCEPT = 0;
    const uint8_t UTF8_REJECT = 12;
    
/*    
    static const uint8_t utf8d[] = {
        // The first part of the table maps bytes to character classes that
        // to reduce the size of the transition table and create bitmasks.
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
        
        // The second part is a transition table that maps a combination
        // of a state of the automaton and a character class to a state.
        0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
        12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
        12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
        12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
        12,36,12,12,12,12,12,12,12,12,12,12,  
    };
 */
 
    inline uint32_t decode(uint32_t& state, uint32_t& codepoint, uint32_t byte) 
    {
        const uint8_t utf8d[] = {
            // The first part of the table maps bytes to character classes that
            // to reduce the size of the transition table and create bitmasks.
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
            7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
            8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
            
            // The second part is a transition table that maps a combination
            // of a state of the automaton and a character class to a state.
            0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
            12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
            12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
            12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
            12,36,12,12,12,12,12,12,12,12,12,12, 
        };
        
        
        uint32_t type = utf8d[byte];
        if (state != UTF8_ACCEPT) {
            codepoint = (byte & 0x3fu) | (codepoint << 6);
        } 
        else {
            codepoint = (0xff >> type) & (byte);
        }
        
        state = utf8d[256 + state + type];
        return state;
    }
    
    template <typename InputIteratorT>
    inline int utf8_convert_to_codepoint(InputIteratorT& first, InputIteratorT last, code_point_t& codepoint)
    {    
        assert(first != last);
        uint32_t state = 0;
        if (static_cast<uint8_t>(*first) < 0x80) {
            codepoint = static_cast<uint8_t>(*first++);
            return 1;
        }        
        while (first != last) {
            decode(state, codepoint, static_cast<uint8_t>(*first));
            switch (state) {
                default:
                    ++first;
                    continue;
                case UTF8_ACCEPT:
                    ++first;
                    return 1;
                case UTF8_REJECT:
                    return -1;
            }
        }
        return -2; // unexpected EOF
    }
    
    
    
}



namespace {
    
    class dummy_insert_iterator :
    public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    public:
        dummy_insert_iterator() {}
        template <typename T>
        dummy_insert_iterator& operator= (T const&)
        { return *this; }
        dummy_insert_iterator& operator* ()
        { return *this; }
        dummy_insert_iterator& operator++ ()
        { return *this; }
        dummy_insert_iterator operator++ (int)
        { return *this; }
    };

}

#pragma mark - Create Source
namespace {
    
    
    enum BenchFlags {
        BENCH_JP_SAFE =         1 << 0, 
        BENCH_JP_UNSAFE =       1 << 1,
        BENCH_ICU =             1 << 2,
        BENCH_JP =              BENCH_JP_SAFE | BENCH_JP_UNSAFE, 
        BENCH_ALL =             BENCH_ICU | BENCH_JP_SAFE | BENCH_JP_UNSAFE
    };
    
    const int kBenchFlags =  BENCH_JP_SAFE | BENCH_ICU;
    //const int kBenchFlags = BENCH_JP;
    

    
    using unicode::code_point_t;    
    
    enum Distribution {
        D0 = 0,
        D1,
        D2,
        D3,
        D4,
        D5,
        D6
    };
    
    struct distribution {double c0; double c1; double c2; double c3;};
    const distribution Distributions[7] = {
        {1, 0, 0, 0},               // D0
        {0, 1, 0, 0},               // D1
        {0, 0, 1, 0},               // D2
        {0, 0, 0, 1},               // D3
        {0.6, 0.25, 0.1, 0.05},     // D4
        {0.8, 0.15, 0.05, 0},       // D5
        {0.25, 0.25, 0.25, 0.25}    // D6
    }; 
    
    
    // Return a vector of Unicode codepoints with size N with a distribution d.
    // - c0% [1..0x1F]              (converts to single byte)
    // - c1% [0x80..0x7FF]          (converts to two bytes)
    // - c2% [0x800..0xFFFF]        (converts to three bytes)
    // - c3% [0x1000.. 0x10FFFF]    (converts to four bytes)
    std::vector<code_point_t> 
    __attribute__((noinline))    
    create_codepoint_source(std::size_t N, Distribution d, bool randomize = true)
    {
        using unicode::code_point_t;
        using unicode::encoding_traits;
        
        typedef std::vector<code_point_t> result_t;
        
        // normalize:
        double C = Distributions[d].c0 + Distributions[d].c1 + Distributions[d].c2 + Distributions[d].c3;
        double c0 = Distributions[d].c0/C;
        double c1 = Distributions[d].c1/C;
        double c2 = Distributions[d].c2/C;
        double c3 = Distributions[d].c3/C;
        
        std::vector<code_point_t> source;
        source.reserve(N);
        
        std::size_t d0 = 0; 
        std::size_t d1 = 0;
        std::size_t d2 = 0;
        std::size_t d3 = 0;
        
        d0 = static_cast<std::size_t>((N * c0) + 0.5); 
        if (d0 < N) {
            d1 = std::min(std::size_t((N * c1) + 0.5), N-d0);
            if ((d0 + d1) < N) {
                d2 = std::min(std::size_t((N * c2) + 0.5), N-d0-d1);
                if ((d0+d1+d2) < N) {
                    d3 = std::min(std::size_t((N * c3) + 0.5), N-d0-d1-d2);
                }
            }                
        }
        
        
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
        
        // The number of code points shall be N:
        assert(source.size() == N);
        
        if (randomize) {
            std::random_shuffle(source.begin(), source.end());
        }
        
        return source;
    }
    
    
    // Return a vector of code units in encoding 'EncodingT', whose number
    // of code points equals N. The resulting number of code units depends
    // on the distribution and the encoding.
    // - c0% encoded in single byte, 
    // - c1% encoded in two bytes
    // - c2% encoded in three bytes,
    // - c3% encoded in four bytes
    template <typename EncodingT>
    std::vector<typename unicode::encoding_traits<EncodingT>::code_unit_type>
    __attribute__((noinline))    
    create_utf_source(EncodingT encoding,
                  std::size_t N, Distribution d, bool randomize = true)
    {
        using unicode::code_point_t;
        using unicode::add_endianness;
        using unicode::encoding_traits;

        typedef typename add_endianness<EncodingT>::type encoding_type;
        typedef typename encoding_traits<encoding_type>::code_unit_type char_t;
        typedef std::vector<char_t> result_t;
        
        
        std::vector<code_point_t> source = create_codepoint_source(N, d, randomize);
        source.reserve(N);
        
        // Now convert the code points into the specified encoding and return
        // the result. Note: Unicode code points can be seen as Unicode code 
        // units encoded in UTF-32 in host endianness.
        result_t result;        
        std::vector<code_point_t>::iterator first = source.begin();
        std::back_insert_iterator<result_t> dest = std::back_inserter(result);
        // convert code_point to UTF
        unicode::mb_state<unicode::UTF_32_encoding_tag> state;
        int res = unicode::convert(first, source.end(), unicode::UTF_32_encoding_tag(), 
                                  dest, encoding_type(), 
                                  state);     
        if (res < 0) {
            throw std::runtime_error("error while creating UTF source");
        }
        
        return result;
    }
    
    
}



namespace {
    
    typedef     utilities::MinMaxAvg<double> MinMaxAvgTime;
    
}
    
namespace {    

    // Using back_inserter
    template <typename InputEncodingT, typename OutputEncodingT>
    __attribute__((noinline))    
    MinMaxAvgTime 
    bench_ref (InputEncodingT inputEncoding, OutputEncodingT outputEncoding,
               std::size_t N, std::size_t K) 
    {
        using unicode::add_endianness;
        using utilities::timer;
        using unicode::encoding_traits;
        
        typedef typename add_endianness<InputEncodingT>::type input_encoding_t;
        typedef typename add_endianness<OutputEncodingT>::type output_encoding_t;
        
        typedef typename encoding_traits<input_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<output_encoding_t>::code_unit_type output_char_t;
        
        typedef std::vector<input_char_t> input_buffer_t;        
        typedef typename input_buffer_t::const_iterator input_iterator;
        typedef std::vector<output_char_t> output_buffer_t;        
        typedef typename output_buffer_t::iterator output_iterator;
        
        const input_buffer_t source(N, 0);
        
        output_buffer_t dest;
        dest.reserve(N);
        
        
        timer t0;
        MinMaxAvgTime result;
        for (std::size_t i = 0; i < K; ++i) 
        {
            input_iterator first = source.begin();
            input_iterator last = source.end();
            
            t0.start();
            std::copy(first, last, std::back_inserter(dest));
            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            dest.clear();
        }
        
#if defined (LOGDEBUG)        
        std::cout <<  source.size() << " copied." << std::endl;
#endif        
        return result;
    }
    

}



 
namespace {  
    
    
    template <typename InputEncodingT, typename OutputEncodingT>
    MinMaxAvgTime 
    __attribute__((noinline))    
    bench_JP(InputEncodingT inputEncoding, OutputEncodingT outputEncoding,
                    std::size_t N, std::size_t K, Distribution d, bool randomize = true)
    {
        using unicode::add_endianness;
        using unicode::code_point_t;
        using unicode::encoding_traits;
        using utilities::timer;
        
        typedef typename add_endianness<InputEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutputEncodingT>::type     to_encoding_t;        
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        typedef std::vector<input_char_t> input_buffer_t;        
        typedef typename input_buffer_t::iterator input_iterator;
        typedef std::vector<output_char_t> output_buffer_t;        
        typedef typename output_buffer_t::iterator output_iterator;
        
#if defined (LOGDEBUG)        
        std::cout << "source encoding: " << encoding_traits<from_encoding_t>::name() << "\n";
        std::cout << "target encoding: " << encoding_traits<to_encoding_t>::name() << "\n";
#endif        
        
        input_buffer_t source_buffer = create_utf_source(from_encoding_t(), N, d, randomize);
        
        size_t target_buffer_size = (size_t)(N*4/sizeof(output_char_t));
        output_buffer_t target_buffer(target_buffer_size, 0);    

        size_t consumed = 0;
        size_t produced = 0;

        timer t0;
        MinMaxAvgTime result;
        int error = 0;
#if defined (LOGDEBUG)        
        std::cout << "using output buffer with sufficient size" << std::endl;
#endif        
        
        for (std::size_t i = 0; i < K; i++) 
        {
            input_iterator first = source_buffer.begin();
            input_iterator last = source_buffer.end();
            output_iterator dest = target_buffer.begin();
            consumed = 0;
            produced = 0;
            
            t0.start();
            //int state = 0;
            error = unicode::convert(first, last, from_encoding_t(), 
                                     dest, to_encoding_t()/*, state*/);
            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            consumed = (size_t)std::distance(source_buffer.begin(), first);
            produced = (size_t)std::distance(target_buffer.begin(), dest);
            if (produced > target_buffer_size) {
                std::cout << "ERROR: target buffer was too small!" << std::endl;
                abort();
            }
            target_buffer.clear();
        }        
        if (error < 0) {
            std::cout << "Error during conversion: "<< error << std::endl;
        }        
#if defined (LOGDEBUG)        
        std::cout << "source buffer size (code units): " <<  source_buffer.size() <<  std::endl;
        std::cout << "target buffer size (code units): " <<  target_buffer_size <<  std::endl;
        std::cout << "consumed code units from source: " <<  consumed <<  std::endl;
        std::cout << "produced code units into target: " <<  produced <<  std::endl;
#endif        
        return result;
        
    }
    
    template <typename InputEncodingT, typename OutputEncodingT>
    MinMaxAvgTime
    __attribute__((noinline))    
    bench_JP_unsafe(InputEncodingT inputEncoding, OutputEncodingT outputEncoding,
                    std::size_t N, std::size_t K, Distribution d, bool randomize = true)
    {
        using unicode::add_endianness;
        using unicode::encoding_traits;
        using utilities::timer;
        
        typedef typename add_endianness<InputEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutputEncodingT>::type     to_encoding_t;        
            
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        
        typedef std::vector<input_char_t> input_buffer_t;        
        typedef typename input_buffer_t::iterator input_iterator;
        typedef std::vector<output_char_t> output_buffer_t;        
        typedef typename output_buffer_t::iterator output_iterator;
        
#if defined (LOGDEBUG)        
        std::cout << "source encoding: " << encoding_traits<from_encoding_t>::name() << "\n";
        std::cout << "target encoding: " << encoding_traits<to_encoding_t>::name() << "\n";
#endif                
        
        input_buffer_t source_buffer = create_utf_source(from_encoding_t(), N, d, randomize);
        
        size_t target_buffer_size = (size_t)(N*4/sizeof(output_char_t)); 
        output_buffer_t target_buffer(target_buffer_size, 0);
        
        
        timer t0;
        MinMaxAvgTime result;
        size_t consumed = 0;
        size_t produced = 0;
        int error;
#if defined (LOGDEBUG)        
        std::cout << "using output buffer with sufficient size" << std::endl;
#endif        
        for (std::size_t i = 0; i < K; ++i) 
        {
            input_iterator first = source_buffer.begin();
            input_iterator last = source_buffer.end();
            output_iterator dest = target_buffer.begin();
            
            t0.start();
            //int state = 0;
            error = unicode::convert_unsafe(first, last, from_encoding_t(),
                                            dest, to_encoding_t()/*, state*/);
            // error = cvt.convert(first, last, std::back_inserter(target_buffer));
            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            consumed = std::distance(source_buffer.begin(), first);
            produced = std::distance(target_buffer.begin(), dest);
            if (produced > target_buffer_size) {
                std::cout << "ERROR: target buffer was too small!" << std::endl;
                abort();
            }
            target_buffer.clear();
        }
#if defined (LOGDEBUG)        
        std::cout << "source buffer size (code units): " <<  source_buffer.size() <<  std::endl;
        std::cout << "target buffer size (code units): " <<  target_buffer_size <<  std::endl;
        std::cout << "consumed code units from source: " <<  consumed <<  std::endl;
        std::cout << "produced code units into target: " <<  produced <<  std::endl;
#endif        
        return result;
        
    }
    
    
    
#pragma mark - ICU
    
    
    
    
    template <typename InputEncodingT, typename OutputEncodingT>
    MinMaxAvgTime 
    __attribute__((noinline))    
    bench_ICU(InputEncodingT inputEncoding, OutputEncodingT outputEncoding,
              std::size_t N, std::size_t K, Distribution d, bool randomize = true)
    {
        using unicode::add_endianness;
        using unicode::encoding_traits;
        using utilities::timer;
        
        typedef typename add_endianness<InputEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutputEncodingT>::type     to_encoding_t;        
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;

        typedef std::vector<input_char_t> input_buffer_t;
        
        
#if defined (LOGDEBUG)        
        std::cout << "source encoding: " << encoding_traits<from_encoding_t>::name() << "\n";
        std::cout << "target encoding: " << encoding_traits<to_encoding_t>::name() << "\n";
#endif        
        
        input_buffer_t source_buffer = create_utf_source(from_encoding_t(), N, d, randomize);
        int32_t source_length = (int)(source_buffer.size()*sizeof(input_char_t));  // size in bytes
        const char* source = reinterpret_cast<const char*>(&(source_buffer[0]));
        
        // allocate buffer big enough for the worst case maximum target size:
        int32_t target_size = (int32_t)((N+1)*sizeof(uint32_t));
        char* target = (char*)malloc(size_t(target_size));
        
        timer t0;
        MinMaxAvgTime result;        
        UErrorCode error = U_ZERO_ERROR;
#if defined (LOGDEBUG)        
        std::cout << "using output buffer with sufficient size" << std::endl;
#endif        
        
        int32_t icu_output_bytes;
        for (std::size_t i = 0; i < K; ++i) 
        {
            t0.start();
            icu_output_bytes = 
            ucnv_convert_48(encoding_traits<to_encoding_t>::name() /*to*/, 
                            encoding_traits<from_encoding_t>::name() /*from*/, 
                            target, target_size, 
                            source,  source_length, &error); 
            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            if (icu_output_bytes == 0)
            {
                std::cout << "Nothing generated." << std::endl;
            }        
            assert(error == U_ZERO_ERROR);
                
        }
#if defined (LOGDEBUG)        
        std::cout << "source buffer size (code units): " <<  source_buffer.size() <<  std::endl;
        std::cout << "target buffer size (code units): " <<  target_size/sizeof(output_char_t) <<  std::endl;
        std::cout <<  (size_t)icu_output_bytes/sizeof(output_char_t) << " converted." << std::endl;
#endif        
        free(target);
        return result;
        
    }

    
    
#pragma mark - HM
    
#if 0    
    MinMaxAvgTime 
    bench_utf8_to_codepoint_Hm(std::size_t N, std::size_t K, bool randomize,
                                   double c0 = 0.6, 
                                   double c1 = 0.25,
                                   double c2 = 0.1,
                                   double c3 = 0.05) 
    {
        using unicode::code_point_t;
        using utilities::timer;
        
        std::vector<char> source = createSource_UTF8(N, randomize, c0, c1, c2, c3);
        
        std::vector<code_point_t> dest;
        dest.reserve(N+1);
        
        timer t0;
        MinMaxAvgTime result;
        for (int i = 0; i < K; ++i) 
        {
            std::vector<char>::iterator first = source.begin();
            std::vector<char>::iterator last = source.end();
                        
            t0.start();
            while (first != last) {
                code_point_t code_point;
                int result = hoehrmann::utf8_convert_to_codepoint(first, last, code_point);
                if (result > 0) {
                    dest.push_back(code_point);
                } else {
                    //break;
                    throw std::logic_error("conversion failed");
                }
                assert(first <= last);
            }  
            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            dest.clear();
        }
        
#if defined (LOGDEBUG)        
        std::cout <<  dest.size() << " converted." << std::endl;
#endif        
        return result;
        
    }
#endif    
    
}



//
//  JP: UTF to codepoint
//
#pragma mark - JP: UTF to codepoint
namespace {
    
    
    //
    //  Convert an Unicode sequence in encoding 'inputEncoding' whose length
    //  corresponds to N Unicode codepoints whose distribution equals 'd' to
    //  a sequence of Unicode codepoints.
    //  Perform bench K times and return min, max avg.
    //  The input is checked for well-formed Unicode.
    //  The output buffer is not checked for overruns and shall be big enough.
    //
    template <typename InEncodingT>
    MinMaxAvgTime 
    __attribute__((noinline))    
    bench_utf_to_codepoint_JP(InEncodingT inputEncoding,
                              std::size_t N, std::size_t K, Distribution d, bool randomize = true)
    {
        using unicode::add_endianness;
        using unicode::encoding_traits;
        using unicode::code_point_t;
        using utilities::timer;
        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;  
        
        // Note: Unicode code points can be seen as Unicode code units
        // encoded in UTF-8 platform endianness.            
        typedef typename add_endianness<unicode::UTF_32_encoding_tag>::type code_point_encoding_t;
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef std::vector<input_char_t> input_buffer_t;                
        typedef typename input_buffer_t::iterator input_iterator;
        
        typedef std::vector<code_point_t> output_buffer_t;        
        typedef typename output_buffer_t::iterator output_iterator;
        
        input_buffer_t source_buffer = create_utf_source(from_encoding_t(), N, d, randomize);
        size_t target_buffer_size = N;
        output_buffer_t target_buffer(target_buffer_size, 0);
        
        
        timer t0;
        MinMaxAvgTime result;
        size_t count = 0;
#if defined (LOGDEBUG)                
        std::cout << "using output buffer with sufficient size" << std::endl;
#endif        
        
        for (std::size_t i = 0; i < K; ++i) 
        {            
            typename std::vector<input_char_t>::iterator first = source_buffer.begin();
            typename std::vector<input_char_t>::iterator last = source_buffer.end();
            output_iterator dest = target_buffer.begin();
            count = 0;
            
            
            t0.start();
            // Convert UTF to Unicode code points:
//            dummy_insert_iterator d;
            //int state = 0;
            int res = unicode::convert(first, last, from_encoding_t(), dest, code_point_encoding_t()/*, state*/); 
            if (res < 0) {
                throw std::logic_error("conversion failed");
            }
      
            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            target_buffer.clear();
            assert(first == last);
        }
#if defined (LOGDEBUG)        
        std::cout << "source buffer size (code units):"<<  source_buffer.size() <<  std::endl;
        std::cout << "target buffer size (code units):"<<  target_buffer_size <<  std::endl;
        std::cout << "converted code units: "<<  count <<  std::endl;
#endif        
        return result;
    }
    
    
    //
    //  Convert an Unicode sequence in encoding 'inputEncoding' whose length
    //  corresponds to N Unicode codepoints whose distribution equals 'd' to
    //  a sequence of Unicode codepoints.
    //  Perform bench K times and return min, max avg.
    //  The input is not checked for well-formed Unicode, thus the input shall
    //  be well-formed Unicode, otherwise the result is undefined - even a 
    //  crash may occur.
    //  The output buffer is not checked for overruns and shall be big ennough.
    //
    template <typename InEncodingT>
    MinMaxAvgTime 
    __attribute__((noinline))    
    bench_utf_to_codepoint_unsafe_JP(InEncodingT inputEncoding,
                                    std::size_t N, std::size_t K, Distribution d, bool randomize = true)
    {
        using unicode::code_point_t;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using utilities::timer;
        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        // Note: Unicode code points can be seen as Unicode code units
        // encoded in UTF-8 platform endianness.            
        typedef typename add_endianness<unicode::UTF_32_encoding_tag>::type code_point_encoding_t;
        
        
        typedef typename encoding_traits<from_encoding_t>::code_unit_type input_char_t;
        typedef std::vector<input_char_t> input_buffer_t;                
        typedef typename input_buffer_t::iterator input_iterator;
        
        typedef std::vector<code_point_t> output_buffer_t;        
        typedef typename output_buffer_t::iterator output_iterator;
        
        input_buffer_t source_buffer = create_utf_source(from_encoding_t(), N, d, randomize);
        size_t target_buffer_size = N;
        output_buffer_t target_buffer(target_buffer_size, 0);
        
        timer t0;
        MinMaxAvgTime result;
        size_t count = 0;
#if defined (LOGDEBUG)        
        std::cout << "using output buffer with sufficient size" << std::endl;
#endif        
        
        for (std::size_t i = 0; i < K; ++i) 
        {            
            typename std::vector<input_char_t>::iterator first = source_buffer.begin();
            typename std::vector<input_char_t>::iterator last = source_buffer.end();
            output_iterator dest = target_buffer.begin();
            count = 0;
            
            t0.start(); 
            //            dummy_insert_iterator d;
            // Convert UTF to Unicode code points:
#if !defined (NDEBUG)            
            int res = 
#endif            
            unicode::convert_unsafe(first, last, from_encoding_t(), dest, code_point_encoding_t()); 
            assert(res == 0);

            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            target_buffer.clear();
            assert(first == last);
        }
#if defined (LOGDEBUG)        
        std::cout << "source buffer size (code units):"<<  source_buffer.size() <<  std::endl;
        std::cout << "target buffer size (code units):"<<  target_buffer_size <<  std::endl;
        std::cout << "converted code units: "<<  count <<  std::endl;
#endif        
        return result;
    }
    
}    
    

//
//  JP: codepoint to UTF
//
#pragma mark - JP: codepoint to UTF
namespace {
    
    
    //
    //  Convert a sequence of Unicode codepoints of size N whose distribution 
    //  equals 'd' to a sequence of Unicode in encoding form 'outEncoding'.
    //  Perform bench K times and return min, max avg.
    //  The input is checked for valid Unicode codepoints.
    //  The output buffer is not checked for overruns and shall be big ennough.
    //
    template <typename OutEncodingT>
    MinMaxAvgTime 
    __attribute__((noinline))    
    bench_codepoint_to_utf_JP(OutEncodingT outEncoding,
                              std::size_t N, std::size_t K, 
                              Distribution d, bool randomize = true)
    {
        using unicode::code_point_t;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using utilities::timer;
        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;    
        typedef typename encoding_traits<to_encoding_t>::code_unit_type          output_char_t;
        typedef typename std::vector<output_char_t>             target_buffer_t;

        // Note: Unicode code points can be seen as Unicode code units
        // encoded in UTF-8 platform endianness.            
        typedef typename add_endianness<unicode::UTF_32_encoding_tag>::type code_point_encoding_t;
        

        std::vector<code_point_t> source_buffer = create_codepoint_source(N, d, randomize);
        target_buffer_t target_buffer(N*4/sizeof(output_char_t), 0);
        
        timer t0;
        MinMaxAvgTime result;
        size_t target_buffer_size;
#if defined (LOGDEBUG)        
        std::cout << "using output buffer with sufficient size" << std::endl;
#endif        
        size_t total = 0;
        size_t count = 0;
        for (std::size_t i = 0; i < K; ++i) 
        {            
            typename std::vector<code_point_t>::iterator first = source_buffer.begin();
            typename std::vector<code_point_t>::iterator last = source_buffer.end();            
            typename target_buffer_t::iterator dest = target_buffer.begin();
            
            count = 0;
            t0.start();   
            // Convert Unicode code points to UTF
            // Note: Unicode code points can be seen as Unicode code units
            // encoded in UTF-8 platform endianness.            
            int res = convert(first, last, code_point_encoding_t(), dest, to_encoding_t());
            if (res < 0) {
                //break;
                throw std::logic_error("conversion failed");
            }
            
            t0.stop();
            result.set(t0.seconds());
            t0.reset();
            target_buffer_size = target_buffer.size();
            count = std::distance(target_buffer.begin(), dest);
            total += count;
            target_buffer.clear();
            target_buffer.resize(N, 0);
            dest = target_buffer.begin();
            assert(first == last);
        }
        std::cout << "total output size: " << total << std::endl;
#if defined (LOGDEBUG)        
        std::cout << "source buffer size (code units):"<<  source_buffer.size() <<  std::endl;
        std::cout << "target buffer size (code units):"<<  target_buffer_size <<  std::endl;
        std::cout << "converted code units: "<<  count <<  std::endl;
#endif        
        return result;
    }
    
    
    //
    //  Convert a sequence of Unicode codepoints of size N whose distribution 
    //  equals 'd' to a sequence of Unicode in encoding form 'outEncoding'.
    //  Perform bench K times and return min, max avg.
    //  The input is not checked for valid Unicode codepoints - thus the input
    //  shall be valid Unicode codepoints.
    //  The output buffer is not checked for overruns and shall be big ennough.
    //
    template <typename OutEncodingT>
    MinMaxAvgTime 
    __attribute__((noinline))    
    bench_codepoint_to_utf_unsafe_JP(OutEncodingT outEncoding,
                                     std::size_t N, std::size_t K, 
                                     Distribution d, bool randomize = true)
    {
        using unicode::code_point_t;
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using utilities::timer;
        
        typedef typename add_endianness<OutEncodingT>::type             to_encoding_t;    
        typedef typename encoding_traits<to_encoding_t>::code_unit_type output_char_t;
        typedef typename std::vector<output_char_t>                     target_buffer_t;
        // Note: Unicode code points can be seen as Unicode code units
        // encoded in UTF-8 platform endianness.            
        typedef typename add_endianness<unicode::UTF_32_encoding_tag>::type code_point_encoding_t;
        
        
        std::vector<code_point_t> source_buffer = create_codepoint_source(N, d, randomize);
        target_buffer_t target_buffer(N*4/sizeof(output_char_t),0);
                
        timer t0;
        MinMaxAvgTime result;
        size_t target_buffer_size;
#if defined (LOGDEBUG)        
        std::cout << "using output buffer with sufficient size" << std::endl;
#endif        
        size_t total = 0;
        size_t count = 0;
        
        // Convert Unicode code points to UTF
        for (std::size_t i = 0; i < K; ++i) 
        {            
//            std::vector<code_point_t>::iterator first = source_buffer.begin();
//            std::vector<code_point_t>::iterator last = source_buffer.end();            
            const code_point_t* first = &source_buffer[0];
            const code_point_t* last = &source_buffer[source_buffer.size()];
            typename target_buffer_t::iterator dest = target_buffer.begin();
            count = 0;
            t0.start();
#if !defined (NDEBUG)            
            int res = 
#endif            
            convert_unsafe(first, last, code_point_encoding_t(), dest, to_encoding_t());            
            assert(res == 0);

            t0.stop();
            result.set(t0.seconds());
            t0.reset(); 
            target_buffer_size = target_buffer.size();
            count = std::distance(target_buffer.begin(), dest);
            total += count;
            target_buffer.clear();
            target_buffer.resize(N, 0);
            assert(first == last);
        }
        std::cout << "total output size: " << total << std::endl;
#if defined (LOGDEBUG)        
        std::cout << "source buffer size (code units):"<<  source_buffer.size() <<  std::endl;
        std::cout << "target buffer size (code units):"<<  target_buffer_size <<  std::endl;
        std::cout << "converted code units: "<<  count <<  std::endl;
#endif        
        return result;
    }
    
}    



namespace {
    
    void print_result(const std::string& label, const MinMaxAvgTime& time, double scale_for) 
    {
        std::pair<double,std::string> scale;
        if (scale_for == 0) {
            scale_for = time.min();
        }
            
        if (scale_for < 1.0e-6) {
            scale.first = 1.0e9;
            scale.second = "ns";
        } else if (scale_for < 1.0e-3) {
            scale.first = 1.0e6;
            scale.second = "µs";
        } else if (scale_for < 1) {
            scale.first = 1.0e3;
            scale.second = "ms";
        } else {
            scale.first = 1;
            scale.second = "s";
        }
        
//        printf("%-8.8s"
//               "min:%8.3f %s, max:%8.3f %s, avg:%8.3f %s\n", 
//               (label + ":").c_str(),
//               time.min()*scale.first, scale.second.c_str(), 
//               time.max()*scale.first, scale.second.c_str(), 
//               time.avg()*scale.first, scale.second.c_str() );
        
        printf("%-8.8s %8.3f %s\n", 
               (label + ":").c_str(), time.min()*scale.first, scale.second.c_str());
    }
    
    
}    
    
    
namespace {
    
    template <typename InputEncoding>
    void 
    __attribute__((noinline))    
    bench_UTF_to_codepoint(InputEncoding, Distribution d,
                                size_t N = 1000*1000, size_t K = 10) 
    {
        const std::string inEncodingName = unicode::encoding_traits<InputEncoding>::name();
        
        std::stringstream ss;
        if (Distributions[d].c0 > 0)
            ss << Distributions[d].c0*100 << "% single  ";
        if (Distributions[d].c1 > 0)
            ss << Distributions[d].c1*100 << "% double  ";
        if (Distributions[d].c2 > 0)
            ss << Distributions[d].c2*100 << "% triple  ";
        if (Distributions[d].c3 > 0)
            ss << Distributions[d].c3*100 << "% quad";
        
        printf("-------------------------------------------\n");        
        printf(" Converting %s sequence to %lu Unicode codepoints\n", inEncodingName.c_str(), N);
        printf(" %s bytes.\n", ss.str().c_str());
        
        MinMaxAvgTime elapsedTime;
                
        elapsedTime = bench_utf_to_codepoint_JP(InputEncoding(), N, K, d);
        print_result("JP", elapsedTime, elapsedTime.min());

        elapsedTime = bench_utf_to_codepoint_unsafe_JP(InputEncoding(),N, K, d);
        print_result("JP(unsafe)", elapsedTime, elapsedTime.min());
        
        /*
        elapsedTime = bench_ICU(InputEncoding(), outputEncoding, N, K, d);
        print_result("ICU", elapsedTime, elapsedTime.min());
        */
        printf("\n");
    }
    
    
    
    template <typename OutputEncoding>
    void 
    __attribute__((noinline))    
    bench_codepoint_to_UTF(OutputEncoding, Distribution d,
                                size_t N = 1000*1000, size_t K = 10) 
    {
        const std::string encodingName = unicode::encoding_traits<OutputEncoding>::name();
        
        std::stringstream ss;
        if (Distributions[d].c0 > 0)
            ss << Distributions[d].c0*100 << "% single  ";
        if (Distributions[d].c1 > 0)
            ss << Distributions[d].c1*100 << "% double  ";
        if (Distributions[d].c2 > 0)
            ss << Distributions[d].c2*100 << "% triple  ";
        if (Distributions[d].c3 > 0)
            ss << Distributions[d].c3*100 << "% quad";
        
        printf("-------------------------------------------\n");        
        printf(" Converting %lu Unicode codepoints to sequence of %s\n", N, encodingName.c_str());
        printf(" %s bytes.\n", ss.str().c_str());
        
        MinMaxAvgTime elapsedTime;
        
        elapsedTime = bench_codepoint_to_utf_JP(OutputEncoding(),N, K, d);
        print_result("JP", elapsedTime, elapsedTime.min());
        
        elapsedTime = bench_codepoint_to_utf_unsafe_JP(OutputEncoding(), N, K, d);
        print_result("JP(unsafe)", elapsedTime, elapsedTime.min());
        
        /*
        elapsedTime = bench_ICU(inputEncoding, outputEncoding, N, K, d);
        print_result("ICU", elapsedTime, elapsedTime.min());
        */
        printf("\n");
    }
    

}


namespace {
    
    
    
    template <typename InputEncoding, typename OutputEncoding>
    void 
    __attribute__((noinline))    
    bench_UTF_to_UTF_ref(InputEncoding, OutputEncoding,
                              size_t N = 1000*1000, size_t K = 10) 
    {
        const std::string inEncodingName = unicode::encoding_traits<InputEncoding>::name();
        const std::string outEncodingName = unicode::encoding_traits<OutputEncoding>::name();
        
        printf("-------------------------------------------\n");        
        printf(" Copy a buffer of size 1e6 Bytes as %s code_units into a buffer"
               "\n of N %s code_units for comparison reasons\n", inEncodingName.c_str(), outEncodingName.c_str());
        
        MinMaxAvgTime elapsedTime = bench_ref(InputEncoding(), OutputEncoding(), N, K);
        print_result("Reference:  (copied)", elapsedTime, elapsedTime.min());
        printf("\n");
    }
    

    template <typename InputEncoding, typename OutputEncoding>
    void 
    __attribute__((noinline))    
    bench_UTF_to_UTF(InputEncoding, OutputEncoding, Distribution d,
                          size_t N = 1000*1000, size_t K = 10) 
    {
        const std::string inEncodingName = unicode::encoding_traits<InputEncoding>::name();
        const std::string outEncodingName = unicode::encoding_traits<OutputEncoding>::name();
        
        
        std::stringstream ss;
        if (Distributions[d].c0 > 0)
            ss << Distributions[d].c0*100 << "% single  ";
        if (Distributions[d].c1 > 0)
            ss << Distributions[d].c1*100 << "% double  ";
        if (Distributions[d].c2 > 0)
            ss << Distributions[d].c2*100 << "% triple  ";
        if (Distributions[d].c3 > 0)
            ss << Distributions[d].c3*100 << "% quad";

        printf("-------------------------------------------\n");        
        printf(" Converting %lu code units in %s to %s\n", N, inEncodingName.c_str(), outEncodingName.c_str());
        printf(" %s bytes.\n\n", ss.str().c_str());
        
        MinMaxAvgTime elapsedTime;

        if (kBenchFlags & BENCH_JP_SAFE) {
            elapsedTime = bench_JP(InputEncoding(), OutputEncoding(), N, K, d);        
            print_result("JP", elapsedTime, elapsedTime.min());
        }
        
        if (kBenchFlags & BENCH_JP_UNSAFE) {
            elapsedTime = bench_JP_unsafe(InputEncoding(), OutputEncoding(), N, K, d);
            print_result("JP(unsafe)", elapsedTime, elapsedTime.min());
        }
        

        if (kBenchFlags & BENCH_ICU) {
            elapsedTime = bench_ICU(InputEncoding(), OutputEncoding(), N, K, d);
            print_result("ICU", elapsedTime, elapsedTime.min());
        }

        printf("\n");
    }        
}


namespace {

#pragma mark Bench UTF-8 to UTF-8
    
    void bench_utf8_to_utf8_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_8_encoding_tag(), unicode::UTF_8_encoding_tag(), N, K);
    }
    
    void 
    
    bench_utf8_to_utf8(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_8_encoding_tag(), unicode::UTF_8_encoding_tag(), d, N, K);
    }
    

#pragma mark Bench UTF-8 to UTF-16
    
    void bench_utf8_to_utf16_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_8_encoding_tag(), unicode::UTF_16_encoding_tag(), N, K);
    }
    
    void bench_utf8_to_utf16(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_8_encoding_tag(), unicode::UTF_16LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_8_encoding_tag(), unicode::UTF_16BE_encoding_tag(), d, N, K);
    }
    

#pragma mark Bench UTF-8 to UTF-32
    
    void bench_utf8_to_utf32_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_8_encoding_tag(), unicode::UTF_32_encoding_tag(), N, K);
    }
    
    void bench_utf8_to_utf32(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_8_encoding_tag(), unicode::UTF_32LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_8_encoding_tag(), unicode::UTF_32BE_encoding_tag(), d, N, K);
    }
    
} 


namespace {
    
#pragma mark Bench UTF-16 to UTF-8
    
    void bench_utf16_to_utf8_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_16_encoding_tag(), unicode::UTF_8_encoding_tag(), N, K);
    }
    
    void bench_utf16_to_utf8(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_16LE_encoding_tag(), unicode::UTF_8_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_16BE_encoding_tag(), unicode::UTF_8_encoding_tag(), d, N, K);
    }
    
    
#pragma mark Bench UTF-16 to UTF-16
    
    void bench_utf16_to_utf16_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_16_encoding_tag(), unicode::UTF_16_encoding_tag(), N, K);
    }
    
    void bench_utf16_to_utf16(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_16LE_encoding_tag(), unicode::UTF_16LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_16BE_encoding_tag(), unicode::UTF_16LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_16LE_encoding_tag(), unicode::UTF_16BE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_16BE_encoding_tag(), unicode::UTF_16BE_encoding_tag(), d, N, K);
    }
    
    
#pragma mark Bench UTF-16 to UTF-32
    
    void bench_utf16_to_utf32_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_16_encoding_tag(), unicode::UTF_32_encoding_tag(), N, K);
    }
    
    void bench_utf16_to_utf32(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_16LE_encoding_tag(), unicode::UTF_32LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_16BE_encoding_tag(), unicode::UTF_32LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_16LE_encoding_tag(), unicode::UTF_32BE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_16BE_encoding_tag(), unicode::UTF_32BE_encoding_tag(), d, N, K);
    }
    
}

    
namespace {
    
#pragma mark Bench UTF-32 to UTF-8
    
    void bench_utf32_to_utf8_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_32_encoding_tag(), unicode::UTF_8_encoding_tag(), N, K);
    }
    
    void bench_utf32_to_utf8(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_32LE_encoding_tag(), unicode::UTF_8_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_32BE_encoding_tag(), unicode::UTF_8_encoding_tag(), d, N, K);
    }
    
    
#pragma mark Bench UTF-32 to UTF-16
    
    void bench_utf32_to_utf16_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_32_encoding_tag(), unicode::UTF_16_encoding_tag(), N, K);
    }
    
    void bench_utf32_to_utf16(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_32LE_encoding_tag(), unicode::UTF_16LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_32BE_encoding_tag(), unicode::UTF_16LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_32LE_encoding_tag(), unicode::UTF_16BE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_32BE_encoding_tag(), unicode::UTF_16BE_encoding_tag(), d, N, K);
    }
    
    
#pragma mark Bench UTF-32 to UTF-32
    
    void bench_utf32_to_utf32_ref(size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF_ref(unicode::UTF_32_encoding_tag(), unicode::UTF_32_encoding_tag());
    }
    
    void bench_utf32_to_utf32(Distribution d, size_t N = 1000*1000, size_t K = 10) {
        bench_UTF_to_UTF(unicode::UTF_32LE_encoding_tag(), unicode::UTF_32LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_32BE_encoding_tag(), unicode::UTF_32LE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_32LE_encoding_tag(), unicode::UTF_32BE_encoding_tag(), d, N, K);
        bench_UTF_to_UTF(unicode::UTF_32BE_encoding_tag(), unicode::UTF_32BE_encoding_tag(), d, N, K);
    }
    
}

 
namespace {
    

    void bench_UTF8_to_codepoint(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
        bench_UTF_to_codepoint(unicode::UTF_8_encoding_tag(), D0, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_8_encoding_tag(), D1, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_8_encoding_tag(), D2, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_8_encoding_tag(), D3, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_8_encoding_tag(), D4, N, K);
        printf("\n");
    }
    
    void bench_UTF16_to_codepoint(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
        bench_UTF_to_codepoint(unicode::UTF_16_encoding_tag(), D0, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_16_encoding_tag(), D1, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_16_encoding_tag(), D2, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_16_encoding_tag(), D3, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_16_encoding_tag(), D4, N, K);
        printf("\n");
    }
    
    void bench_UTF32_to_codepoint(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
        bench_UTF_to_codepoint(unicode::UTF_32_encoding_tag(), D0, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_32_encoding_tag(), D1, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_32_encoding_tag(), D2, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_32_encoding_tag(), D3, N, K);
        printf("\n");
        bench_UTF_to_codepoint(unicode::UTF_32_encoding_tag(), D4, N, K);
        printf("\n");
    }
    
    
    void bench_UTF_to_codepoint(size_t N = 1000*1000, size_t K = 10) 
    {
        bench_UTF8_to_codepoint(N, K);
        bench_UTF16_to_codepoint(N, K);
        bench_UTF32_to_codepoint(N, K);
    }
    
}


namespace {
    
    void bench_codepoint_to_UTF8(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
        bench_codepoint_to_UTF(unicode::UTF_8_encoding_tag(), D0, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_8_encoding_tag(), D1, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_8_encoding_tag(), D2, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_8_encoding_tag(), D3, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_8_encoding_tag(), D4, N, K);
        printf("\n");
    }
    
    void bench_codepoint_to_UTF16(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
        bench_codepoint_to_UTF(unicode::UTF_16_encoding_tag(), D0, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_16_encoding_tag(), D1, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_16_encoding_tag(), D2, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_16_encoding_tag(), D3, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_16_encoding_tag(), D4, N, K);
        printf("\n");
    }
    
    void bench_codepoint_to_UTF32(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
        bench_codepoint_to_UTF(unicode::UTF_32_encoding_tag(), D0, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_32_encoding_tag(), D1, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_32_encoding_tag(), D2, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_32_encoding_tag(), D3, N, K);
        printf("\n");
        bench_codepoint_to_UTF(unicode::UTF_32_encoding_tag(), D4, N, K);
        printf("\n");
    }
    
    
    void bench_codepoint_to_UTF(size_t N = 1000*1000, size_t K = 10) 
    {
        bench_codepoint_to_UTF8(N, K);
        bench_codepoint_to_UTF16(N, K);
        bench_codepoint_to_UTF32(N, K);
    }
    
    
}


namespace {
    
    void bench_UTF8_to_UTF8(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf8_to_utf8_ref(N, K);
//        printf("\n");
        bench_utf8_to_utf8(D0, N, K);
        printf("\n");
        bench_utf8_to_utf8(D1, N, K);
        printf("\n");
        bench_utf8_to_utf8(D2, N, K);
        printf("\n");
        bench_utf8_to_utf8(D3, N, K);
        printf("\n");
        bench_utf8_to_utf8(D4, N, K);
        printf("\n");  
        bench_utf8_to_utf8(D5, N, K);
        printf("\n");          
    }
    
    void bench_UTF8_to_UTF16(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf8_to_utf16_ref(N, K);
//        printf("\n");
        bench_utf8_to_utf16(D0, N, K);
        printf("\n");
        bench_utf8_to_utf16(D1, N, K);
        printf("\n");
        bench_utf8_to_utf16(D2, N, K);
        printf("\n");
        bench_utf8_to_utf16(D3, N, K);
        printf("\n");
        bench_utf8_to_utf16(D4, N, K);
        printf("\n");        
        bench_utf8_to_utf16(D5, N, K);
        printf("\n");        
    }
    
    void bench_UTF8_to_UTF32(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf8_to_utf32_ref(N, K);
//        printf("\n");
        bench_utf8_to_utf32(D0, N, K);
        printf("\n");
        bench_utf8_to_utf32(D1, N, K);
        printf("\n");
        bench_utf8_to_utf32(D2, N, K);
        printf("\n");
        bench_utf8_to_utf32(D3, N, K);
        printf("\n");
        bench_utf8_to_utf32(D4, N, K);
        printf("\n");        
        bench_utf8_to_utf32(D5, N, K);
        printf("\n");        
    }
    
    void bench_UTF16_to_UTF8(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf16_to_utf8_ref(N, K);
//        printf("\n");
        bench_utf16_to_utf8(D0, N, K);
        printf("\n");
        bench_utf16_to_utf8(D1, N, K);
        printf("\n");
        bench_utf16_to_utf8(D2, N, K);
        printf("\n");
        bench_utf16_to_utf8(D3, N, K);
        printf("\n");
        bench_utf16_to_utf8(D4, N, K);
        printf("\n");        
        bench_utf16_to_utf8(D5, N, K);
        printf("\n");        
    }
    
    void bench_UTF16_to_UTF16(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf16_to_utf16_ref(N, K);
//        printf("\n");
        bench_utf16_to_utf16(D0, N, K);
        printf("\n");
        bench_utf16_to_utf16(D1, N, K);
        printf("\n");
        bench_utf16_to_utf16(D2, N, K);
        printf("\n");
        bench_utf16_to_utf16(D3, N, K);
        printf("\n");
        bench_utf16_to_utf16(D4, N, K);
        printf("\n");        
        bench_utf16_to_utf16(D5, N, K);
        printf("\n");        
    }
    
    void bench_UTF16_to_UTF32(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf16_to_utf32_ref(N, K);
//        printf("\n");
        bench_utf16_to_utf32(D0, N, K);
        printf("\n");
        bench_utf16_to_utf32(D1, N, K);
        printf("\n");
        bench_utf16_to_utf32(D2, N, K);
        printf("\n");
        bench_utf16_to_utf32(D3, N, K);
        printf("\n");
        bench_utf16_to_utf32(D4, N, K);
        printf("\n");        
        bench_utf16_to_utf32(D5, N, K);
        printf("\n");        
    }
    
    void bench_UTF32_to_UTF8(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf32_to_utf8_ref(N, K);
//        printf("\n");
        bench_utf32_to_utf8(D0, N, K);
        printf("\n");
        bench_utf32_to_utf8(D1, N, K);
        printf("\n");
        bench_utf32_to_utf8(D2, N, K);
        printf("\n");
        bench_utf32_to_utf8(D3, N, K);
        printf("\n");
        bench_utf32_to_utf8(D4, N, K);        
        printf("\n");        
        bench_utf32_to_utf8(D5, N, K);
        printf("\n");                
    }
    
    void bench_UTF32_to_UTF16(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf32_to_utf16_ref(N, K);
//        printf("\n");
        bench_utf32_to_utf16(D0, N, K);
        printf("\n");
        bench_utf32_to_utf16(D1, N, K);
        printf("\n");
        bench_utf32_to_utf16(D2, N, K);
        printf("\n");
        bench_utf32_to_utf16(D3, N, K);
        printf("\n");
        bench_utf32_to_utf16(D4, N, K);
        printf("\n");        
        bench_utf32_to_utf16(D5, N, K);
        printf("\n");        
    }
    
    void bench_UTF32_to_UTF32(size_t N = 1000*1000, size_t K = 10) 
    {
        std::cout << "=====================================" << std::endl;
        
//        bench_utf32_to_utf32_ref(N, K);
//        printf("\n");
        bench_utf32_to_utf32(D0, N, K);
        printf("\n");
        bench_utf32_to_utf32(D1, N, K);
        printf("\n");
        bench_utf32_to_utf32(D2, N, K);
        printf("\n");
        bench_utf32_to_utf32(D3, N, K);
        printf("\n");
        bench_utf32_to_utf32(D4, N, K);
        printf("\n");        
        bench_utf32_to_utf32(D5, N, K);
        printf("\n");        
    }
    
    
    
}

namespace {
    
    void bench_UTF_to_UTF8(size_t N = 1000*1000, size_t K = 10) 
    {
        bench_UTF8_to_UTF8(N, K);
        bench_UTF16_to_UTF8(N, K);
        bench_UTF32_to_UTF8(N, K);
    }
    
    void bench_UTF_to_UTF16(size_t N = 1000*1000, size_t K = 10) 
    {
        bench_UTF8_to_UTF16(N, K);
        bench_UTF16_to_UTF16(N, K);
        bench_UTF32_to_UTF16(N, K);
    }
    
    void bench_UTF_to_UTF32(size_t N = 1000*1000, size_t K = 10) 
    {
        bench_UTF8_to_UTF32(N, K);
        bench_UTF16_to_UTF32(N, K);
        bench_UTF32_to_UTF32(N, K);
    }
    
    
    void bench_UTF_to_UTF(size_t N = 1000*1000, size_t K = 10) 
    {
        bench_UTF_to_UTF8(N, K);
        bench_UTF_to_UTF16(N, K);
        bench_UTF_to_UTF32(N, K);
    }

}
    
 


int main (int argc, const char * argv[])
{    
    try {
        system("system_profiler SPHardwareDataType");
        
        
        time_t time_info;    
        std::time(&time_info);
        std::cout << ctime(&time_info) << "\n";
        
        std::cout << BOOST_COMPILER << "\n";
        
                //profile();

        bench_UTF8_to_codepoint(1000*1000, 10);
        bench_UTF16_to_codepoint();
        bench_UTF32_to_codepoint();
        
//        bench_codepoint_to_UTF8();
//        bench_codepoint_to_UTF16();
//        bench_codepoint_to_UTF32();
//        bench_codepoint_to_UTF();
    
        
        //bench_UTF8_to_UTF8(1000*1000, 100);
        //bench_UTF8_to_UTF16();
        //bench_UTF8_to_UTF32();
        
        //bench_UTF16_to_UTF8();
        //bench_UTF16_to_UTF16();
        //bench_UTF16_to_UTF32();        

        bench_UTF_to_UTF(1000, 1000);
        
        std::cout << "finished" << std::endl;
        
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return -1;
    }
    return 0;
}


//
//  endian.hpp
//
//  Created by Andreas Grosam on 5/18/11.
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

#ifndef JSON_ENDIAN_HPP
#define JSON_ENDIAN_HPP


#include "json/config.hpp"
#include <boost/config.hpp>
#include <boost/detail/endian.hpp>
#include <stdint.h>


namespace json {
    namespace internal {
        
        struct endian_tag {};
        struct little_endian_tag : endian_tag {};
        struct big_endian_tag : endian_tag {};
        
        
        // Runtime endianess detection
        struct run_time_host_endianness 
        {
            
            static bool is_little_endian() {
                union {
                    uint16_t i;
                    unsigned char c[sizeof(uint16_t)];
                } u_ = {0x1234U};
                
                return u_.c[0] == 0x34;
            }
            
            static bool is_big_endian() { 
                union {
                    uint16_t i;
                    unsigned char c[sizeof(uint16_t)];
                } u_ = {0x1234U};
                
                return u_.c[0] == 0x12;
            }
        };
        
        
        
        // Compile-time host-endianess
        struct host_endianness {
#if defined(BOOST_LITTLE_ENDIAN)
            typedef little_endian_tag   type;
            static const bool is_little_endian = true;
            static const bool is_big_endian = false;
#elif defined(BOOST_BIG_ENDIAN)
            typedef big_endian_tag      type;
            static const bool is_little_endian = false;
            static const bool is_big_endian = true;
#else
#error "unable to determine system endianness"
#endif
        };
    
        
    }  // namespace internal
} // namespace json








#endif // JSON_ENDIAN_HPP

//
//  decimal_number.hpp
//  
//
//  Created by Andreas Grosam on 22.02.13.
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

#ifndef JSON_DECIMAL_NUMBER_HPP
#define JSON_DECIMAL_NUMBER_HPP

#include "json/config.hpp"
#include "json_traits.hpp"
#include "json/utility/string_to_number.hpp"
#include "json/utility/number_to_string.hpp"
#include "json/utility/json_number.hpp"
#include <iostream>
#include <type_traits>
#include <limits>
#include <cctype>


#pragma mark -
#pragma mark json::decimal_number
namespace json {
    
    //
    // decimal_number
    //
    
    // A decimal number uses an internal implementation which can exactly
    // represent a JSON number. This includes preserving the precision. Since a
    // JSON number is represented as a number string, a decimal number must be
    // able to represent a possibly high precision and huge number. The conversion
    // of the JSON number to a decimal number and back to a JSON number (string)
    // shall yield the original JSON number, including its precision.
    
    template <int Capacity = 46>
    class decimal_number
    {
        static constexpr int DataSize = (Capacity >> 1)+1;

    public:
        
        // default ctor
        decimal_number() noexcept
        {
            *data_ = 0;
        };
        
        // copy ctor
        constexpr decimal_number(decimal_number const& other) noexcept = default;
        
//        constexpr decimal_number(long long coefficient, int exponent) noexcept
//        {}
//        
//        constexpr decimal_number(unsigned long long coefficient, int exponent) noexcept
//        {
//        }

        
        // string conversion ctor
        explicit decimal_number(const char* str)
        : decimal_number(str, strlen(str))
        {
        }
        
        // string conversion ctor
        decimal_number(const char* s, std::size_t len)
        {
            int result = pack(s, static_cast<int>(len));
            assert(result >= 0);
        }
        
        // integral and floating point conversion ctor
        template <typename T,
            typename Enable = typename std::enable_if<
                json::is_numeric<T>::value
            >::type
        >
        decimal_number(T value)
        {
            std::string s = json::utility::number_to_string(value);
            int result = pack(s.data(), static_cast<int>(s.size()));
            assert(result >= 0);
        }
        
        
        // assignement
        decimal_number& operator=(decimal_number const& other) {
            memcpy(other.data_, data_, sizeof(data_));
            return  *this;
        }
        
        
        // value accesss
        float to_float() const
        {
            return 0; // TODO
        }

        double to_double() const
        {
            return 0; // TODO
        }
        
        long double to_long_double() const
        {
            return 0; // TODO
        }
        
        // string conversion
        std::string to_string() const noexcept {
            char buffer[Capacity + 1];
            int len = unpack(buffer, sizeof(buffer));
            if (len > 0) {
                return std::string(buffer, len);
            }
            else {
                return "NaN";
            }
        }
        
        
    private:
        
        int pack(char const* s, int len)
        {
            if ( len > Capacity ) {
                return -1;
            }
            int count = len;
            uint8_t buffer[2*DataSize+1];
            uint8_t* p = buffer;
            while (count--) {
                switch (*s) {
                    case 0: break;
                    case '0': *p++ = 1; break;
                    case '1': *p++ = 2; break;
                    case '2': *p++ = 3; break;
                    case '3': *p++ = 4; break;
                    case '4': *p++ = 5; break;
                    case '5': *p++ = 6; break;
                    case '6': *p++ = 7; break;
                    case '7': *p++ = 8; break;
                    case '8': *p++ = 9; break;
                    case '9': *p++ = 10; break;
                    case '.': *p++ = 11; break;
                    case '-': *p++ = 12; break;
                    case '+': *p++ = 13; break;
                    case 'e': *p++ = 14; break;
                    case 'E': *p++ = 14; break;
                    default: return -1;
                }
                ++s;
            }
            *p = 0;
            p = buffer;
            uint8_t* d = data_;
            while (len--) {
                uint8_t x = (*p++) << 4;
                if (len) {
                    x |= *p++;
                    --len;
                }
                *d++ = x;
            }
            if (d != data_ + DataSize) {
                *d = 0;
            }
            return 0;
        }
        
        
        int unpack(char* buffer, std::size_t len) const
        {
            if ( len < (Capacity+1)) {
                return -1;
            }
            uint8_t const* d = data_;
            char* p = buffer;
            char c = *d >> 4;
            int count = 0;
            while (1) {
                switch (c) {
                    case 0:  *p++ = 0; return count;
                    case 1:  *p++ = '0'; break;
                    case 2:  *p++ = '1'; break;
                    case 3:  *p++ = '2'; break;
                    case 4:  *p++ = '3'; break;
                    case 5:  *p++ = '4'; break;
                    case 6:  *p++ = '5'; break;
                    case 7:  *p++ = '6'; break;
                    case 8:  *p++ = '7'; break;
                    case 9:  *p++ = '8'; break;
                    case 10: *p++ = '9'; break;
                    case 11: *p++ = '.'; break;
                    case 12: *p++ = '-'; break;
                    case 13: *p++ = '+'; break;
                    case 14: *p++ = 'E'; break;
                }
                if (++count & 0x01) {
                    c = *d & 0x0F;
                    ++d;
                }
                else {
                    c = *d >> 4;
                }
            }
        }
        
        
        // friends:
        friend inline
        std::ostream& operator<<(std::ostream& o, const decimal_number& v) {
            o << v.to_string();
            return o;
        }
        
        
        
        
    private:
        uint8_t data_[DataSize];
    };
    
}





namespace json {
    
    
//    template <int N>
//    struct is_json_type<decimal_number<N> > : public std::true_type
//    {
//    };
    
} // namespace json



#endif  // JSON_INTEGRAL_NUMBER_HPP

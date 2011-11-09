#ifndef JOSON_NUMBER_BUILDER_HPP
#define JOSON_NUMBER_BUILDER_HPP


//
//  number_builder.hpp
//
//  Created by Andreas Grosam on 5/16/11.
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

#include <limits>
#include <stdlib.h>
#include <xlocale.h>
#include "json/unicode/unicode_utilities.hpp"
#include "string_buffer.hpp"
#include <boost/static_assert.hpp>
#include <stdexcept>
#include <cerrno>

namespace json { 
    
    namespace numberbuilder {
    
        
        // TODO: make template with CharT
        struct number_parts 
        {
            typedef json::unicode::utf8_code_unit char_t;
            
            number_parts()  {
                clear();
            }
            
            void clear() {
                sign_.first = sign_.second = 
                integer_.first = integer_.second = 
                decimalPoint_.first = decimalPoint_.second = 
                fractional_.first = fractional_.second = 
                exponent_.first = exponent_.second = 0;
            }
            
            std::pair<const char_t*, const char_t*>  sign_;
            std::pair<const char_t*, const char_t*>  integer_;
            std::pair<const char_t*, const char_t*>  decimalPoint_;
            std::pair<const char_t*, const char_t*>  fractional_;
            std::pair<const char_t*, const char_t*>  exponent_;
        };
        
        
        struct normalized_number_t {
            unsigned long long  mantissa_;
            short               exponent_;
            bool                isNegative_;
        };

        struct number_t {
            
            //
            // char_t shall not be templetized. It shall remain an 8-bit wide
            // integer.
            //
            typedef json::unicode::utf8_code_unit char_t;
            
            
            
            
            bool isNegative() const { return parts_.sign_.first != parts_.sign_.second; }
            bool isInteger() const { return parts_.decimalPoint_.first == parts_.decimalPoint_.second and !hasExponent(); }
            bool hasExponent() const { return parts_.exponent_.first != parts_.exponent_.second; }
            
            std::pair<const char_t*, const char_t*>
            exponent() const { return parts_.integer_; }
            
            std::pair<const char_t*, const char_t*>
            integer() const { return parts_.exponent_; }
            
            std::pair<const char_t*, const char_t*>
            fractional() const { return parts_.fractional_; }
            
            int exp() const {
                int exponent = 0;
                const char* p = parts_.exponent_.first;
                if (p != parts_.exponent_.second) {
                    if (*p == '-' or *p == '+') {
                        ++p;
                    }
                    while (p != parts_.exponent_.second) {
                        exponent = exponent*10 + uint8_t((*p)-'0'); 
                        ++p;
                    }
                    if (*(parts_.exponent_.first) == '-') {
                        exponent = -exponent; 
                    }
                }
                return exponent;
            }
            
            
            int digits() const { 
                return int(parts_.integer_.second - parts_.integer_.first 
                    + parts_.fractional_.second - parts_.fractional_.first);
            }
            
            const char* c_str() const           { return reinterpret_cast<const char*>(string_); }
            std::size_t c_str_len() const       { return len_; }
            
            normalized_number_t normalize() const 
            {
                number_parts parts = parts_;
                int integer_digits = int(parts.integer_.second - parts.integer_.first);
                int fractional_digits = int(parts.fractional_.second - parts.fractional_.first);
                int digits = integer_digits + fractional_digits;
                const int stripCount = std::max(0, digits - std::numeric_limits<unsigned long long>::digits10);
                int exponentCorrection = -fractional_digits;
                if (stripCount > 0) 
                {
                    // We need to strip trailing digits until we get an acceptable length
                    // where the mantissa can be constructed without overflowing or underflowing
                    // the ull conversion. This, however, will loose precision.
                    int stripped = std::min(stripCount, fractional_digits);
                    parts.fractional_.second -= stripped;
                    fractional_digits += stripped;
                    exponentCorrection -= stripped;
                    if (stripped < stripCount) {
                        parts.integer_.second -= (stripCount - stripped);
                        integer_digits -= (stripCount - stripped);
                        exponentCorrection += (stripCount - stripped);
                    }
                    digits -= stripCount;
                }    
                // Copy integer and fractional part to a buffer building the mantissa:
                char buffer[std::numeric_limits<unsigned long long>::digits10  + 1];
                memcpy(buffer, parts_.integer_.first, integer_digits);
                memcpy(buffer + integer_digits, parts_.fractional_.first, fractional_digits);
                buffer[digits] = 0;                
                // convert the string to ull (this should not overflow or underflow):
                unsigned long long mantissa = strtoull_l(buffer, NULL, 10, NULL);
                // Convert the exponent to short:
                long exponent = 0;
                bool exponentRangeError = false;
                if ((parts_.exponent_.second - parts_.exponent_.first) > 0) {
                    errno = 0;
                    exponent = strtol_l( reinterpret_cast<const char*>(parts_.exponent_.first), NULL, 10, NULL);
                    if (errno == ERANGE) {
                        exponentRangeError = true;
                    }
                }
                if (exponent > (long(std::numeric_limits<short>::max()) - exponentCorrection)
                    or exponent < (long(std::numeric_limits<short>::min()) - exponentCorrection))
                {
                    exponentRangeError = true;
                }
                if (exponentRangeError) {
                    throw std::range_error("normalized number out of range");
                }
                // correct the exponent:
                exponent += exponentCorrection;
                
                normalized_number_t result;
                result.mantissa_ = mantissa;
                result.exponent_ = exponent;
                result.isNegative_ = parts.sign_.first != parts.sign_.second and *(parts.sign_.first) == '-';
                
                return result;                
            }
            
            const char_t*   string_;
            size_t          len_;
            number_parts    parts_;
        };
    
    } // numberbuilder
    
    namespace internal 
    {

        struct number_s {
            typedef int64_t integer_type;
            typedef long double float_type;
            enum type_t {
                Integer,
                Float
            };
            type_t type;
            union {
                integer_type    i;
                float_type      d;
            } value;
        };
        
        
        
        template <int AutoBufferSize = 64> 
        class number_builder {
        public:
            typedef string_buffer_base<json::unicode::UTF_8_encoding_tag>   string_buffer_type;     
            typedef typename string_buffer_type::code_unit_t   char_type;
            
            typedef number_s result_type;
            
            enum number_type {
                BAD_NUMBER,
                Integer,
                Float,
                Decimal
            };
            
            number_builder() 
            :   buffer_(auto_buffer_, AutoBufferSize),
                type_(BAD_NUMBER)
            {
            } 
            
            // clear the number builder
            void clear() { buffer_.reset(); type_ = BAD_NUMBER; }
            
            // append ASCII character  0x00 <= c <= 0x7F
            void append_ascii(char c) { buffer_.append_ascii(c); }
            
            // set number characteristics
            void set_type(number_builder::number_type type)  {type_ = type;}
            
            const char_type* str() { buffer_.terminate_if(); return buffer_.buffer(); }
            number_builder::number_type type() const { return type_; }
            
            result_type number() { 
                result_type result;
                buffer_.terminate_if();
                if (type_ == Decimal or type_ == Float) {
                    result.type = result_type::Float;
                    result.value.d = strtold(buffer_.buffer(), NULL);
                } else if (type_ == Integer) {
                    result.type = result_type::Integer;
                    result.value.i = atoll(buffer_.buffer());
                }
                else {
                    result.type = result_type::Float;
                    result.value.d = std::numeric_limits<result_type::float_type>::quiet_NaN();
                }
                return result;
            }
            
        private:
            char_type auto_buffer_[AutoBufferSize];
            string_buffer_type buffer_;
            number_type type_;        
        };
        
    } // namespace internal

    
    
    namespace numberbuilder { namespace internal {

        struct number_parts_indieces {
            number_parts_indieces() 
            {
                clear();
            }
            
            void clear() {
                sign_.first = sign_.second = 
                integer_.first = integer_.second = 
                decimalPoint_.first = decimalPoint_.second = 
                fractional_.first = fractional_.second = 
                exponent_.first = exponent_.second = 0;
            }
            
            std::pair<int, int>  sign_;
            std::pair<int, int>  integer_;
            std::pair<int, int>  decimalPoint_;
            std::pair<int, int>  fractional_;
            std::pair<int, int>  exponent_;
        };
        
    }} // namespace numberbuilder::internal
    
    
    namespace numberbuilder {
    
        template <int AutoBufferSize = 64> 
        class number_builder {
        public:
            typedef json::internal::string_buffer_base<json::unicode::UTF_8_encoding_tag>   string_buffer_type;     
            typedef typename string_buffer_type::code_unit_t   char_t;
            
            // The number builder shall consume ASCII characters.
            BOOST_STATIC_ASSERT( sizeof(char_t) == 1 );
                        
            number_builder() 
            :   buffer_(auto_buffer_, AutoBufferSize), index_(0)
            {
            } 
            
            // clear the number builder
            void clear() 
            { 
                buffer_.reset(); 
                parts_.clear(); 
                index_ = 0;
            }
            
            void push_sign(bool negative) {
                parts_.sign_.first = index_;
                parts_.sign_.second = ++index_;
                buffer_.append_ascii(negative ? '-' : '+'); 
            }
            void push_integer_start(char digit) {
                parts_.integer_.first = index_;
                ++index_;
                buffer_.append_ascii(digit);
            }
            void integer_end() {
                parts_.integer_.second = index_;
            }
            void push_decimalPoint() {
                parts_.decimalPoint_.first = index_;
                parts_.decimalPoint_.second = ++index_;
                buffer_.append_ascii('.'); 
            }
            void push_fractional_start(char digit) {
                parts_.fractional_.first = index_;
                ++index_;
                buffer_.append_ascii(digit);
            }
            void fractional_end() {
                parts_.fractional_.second = index_;
            }
            void push_exponentIndicator(char e_or_E) {
                ++index_;
                buffer_.append_ascii(e_or_E);
            }
            void push_exponent_start(char digitOrSign) {
                parts_.exponent_.first = index_;
                ++index_;
                buffer_.append_ascii(digitOrSign);
            }
            void exponent_end() {
                parts_.exponent_.second = index_;
            }
            void push_digit(char digit) {
                ++index_;
                buffer_.append_ascii(digit);
            }
            
            const char_t* str() { buffer_.terminate_if(); return buffer_.buffer(); }
            const char* c_str() { buffer_.terminate_if(); return reinterpret_cast<const char*>(buffer_.buffer()); }
            
                    
            number_t number() 
            { 
                number_t result;
                
                result.string_ = buffer_.buffer();
                result.len_ = buffer_.size();
                buffer_.terminate_if();                

                const char_t* start = buffer_.buffer();
                result.parts_.sign_.first = start + parts_.sign_.first;
                result.parts_.sign_.second = start + parts_.sign_.second;
                result.parts_.integer_.first = start + parts_.integer_.first;
                result.parts_.integer_.second = start + parts_.integer_.second;
                result.parts_.decimalPoint_.first = start + parts_.decimalPoint_.first;
                result.parts_.decimalPoint_.second = start + parts_.decimalPoint_.second;
                result.parts_.fractional_.first = start + parts_.fractional_.first;
                result.parts_.fractional_.second = start + parts_.fractional_.second;
                result.parts_.exponent_.first = start + parts_.exponent_.first;
                result.parts_.exponent_.second = start + parts_.exponent_.second;
                
                return result;
            }
            
        private:
            char_t auto_buffer_[AutoBufferSize];
            string_buffer_type buffer_;
            internal::number_parts_indieces parts_;
            int index_;
        };

    } // namespace numberbuilder        
    
}



#endif // JOSON_NUMBER_BUILDER_HPP

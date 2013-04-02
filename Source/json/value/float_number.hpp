//
//  float_number.hpp
//  
//
//  Created by Andreas Grosam on 26.02.13.
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

#ifndef JSON_UTILITY_FLOAT_NUMBER_HPP
#define JSON_UTILITY_FLOAT_NUMBER_HPP


#include "json/config.hpp"
#include "json_traits.hpp"
#include "json/utility/string_to_number.hpp"
#include "json/utility/number_to_string.hpp"
#include "json/utility/json_number.hpp"
#if defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)
    #include <boost/spirit/include/karma.hpp>
#endif
#include <iostream>
#include <type_traits>
#include <limits>
#include <cctype>


#pragma mark -
#pragma mark json::float_number
namespace json {
    
    
    
    //
    // float_number
    //
    
    // A float number uses a floating point primitive type in order to represent
    // a JSON number. A floating point number can not preserve the precision of
    // the original JSON number, and the back-conversion from the float number
    // to a JSON number may not yield the original JSON number.

#if defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)    
    using boost::spirit::karma::real_policies;
#endif
    
    template <typename FloatT = double,
        typename = typename std::enable_if<std::is_floating_point<FloatT>::value>::type
    >
    class float_number
    {
#if defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)
        // string generator policy
        struct scientific_policy : real_policies<FloatT>
        {
            // we want the numbers always to be in scientific format
            static int floatfield(FloatT) { return real_policies<FloatT>::fmtflags::scientific; }
            static int unsigned precision(FloatT) { return std::numeric_limits<FloatT>::digits10 + 1; }
            static bool trailing_zeros(FloatT) { return false; }
        };
#endif
        
    public:
        
        float_number() noexcept = default;
        
        constexpr float_number(float_number const& other) noexcept = default;
                
        explicit float_number(const char* str)
        : float_number(str, strlen(str))
        {
        }
        
        float_number(const char* s, std::size_t len)
        : value_(json::utility::string_to_number<FloatT>(s, len))
        {
        }
        
//        template <typename T,
//            typename Enable = typename std::enable_if<
//                json::is_numeric<T>::value and !std::is_floating_point<T>::value
//            >::type
//        >
//        explicit constexpr float_number(T const& value) noexcept
//        : value_{value}
//        {
//        }
        
        template <typename T,
            typename Enable = typename std::enable_if<
                std::is_floating_point<T>::value
            >::type
        >
        constexpr float_number(T const& value) noexcept
        : value_{value}
        {
        }
        
        
        // assignment
        float_number& operator=(float_number const& other)
        {
            value_ = other.value_;
            return *this;
        }
        
        template <typename T,
            typename Enable = typename std::enable_if<
                std::is_floating_point<T>::value
            >::type
        >
        float_number& operator=(T const& value)
        {
            value_ = value;
            return *this;
        }
        

        
            
        // type operator
        operator FloatT() const noexcept { return value_; }
            
        
        // value access
        float to_float() const noexcept {
            return static_cast<float>(value_);
        }
        
        double to_double() const noexcept {
            return static_cast<double>(value_);
        }
        
        long double to_long_double() const noexcept {
            return static_cast<long double>(value_);
        }
        
        
        std::string to_string() const noexcept
        {
#if defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)            
            using boost::spirit::karma::real_generator;
            using boost::spirit::karma::generate;
                        
            // float to number string generator:
            // define a real number formatting policy
            real_generator<FloatT, scientific_policy> generator;
            char buffer[64];
            char* p = buffer;
            generate(p, generator, value_);
            return std::string(buffer, p);
#else
            return convert(value_);
#endif
        }
        
        
    private:
        
        // comparison operator
        friend inline
        bool operator==(float_number const& lhv, float_number const& rhv) {
            return lhv.value_ == rhv.value_;
        }
        
        // ostream support:
        friend inline
        std::ostream& operator<<(std::ostream& o, const float_number& v) {
            o << v.to_string();
            return o;
        }
        
        static std::string convert(float value) {
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "%.*e", std::numeric_limits<float>::max_digits10, value);
            assert(len > 0);
            return std::string(buffer, len);
        }
        static std::string convert(double value) {
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "%.*e", std::numeric_limits<double>::max_digits10, value);
            assert(len > 0);
            return std::string(buffer, len);
        }
        
        static std::string convert(long double value) {
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "%.*Le", std::numeric_limits<long double>::max_digits10, value);
            assert(len > 0);
            return std::string(buffer, len);
        }
        
        
    private:
        FloatT value_;
    };
    
}





namespace json {
    
    
//    template <typename T>
//    struct is_json_type<float_number<T> > : public std::true_type
//    {
//    };
    
} // namespace json




#endif // JSON_UTILITY_FLOAT_NUMBER_HPP

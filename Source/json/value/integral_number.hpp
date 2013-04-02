//
//  integral_number.hpp
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

#ifndef JSON_INTEGRAL_NUMBER_HPP
#define JSON_INTEGRAL_NUMBER_HPP

#include "json/config.hpp"
#include "json_traits.hpp"
#include "json/utility/mpl.hpp"
#include "json/utility/string_to_number.hpp"
#include "json/utility/number_to_string.hpp"
#include <iostream>
#include <type_traits>
#include <limits>



#pragma mark -
#pragma mark json::integral_number
namespace json {
    
    namespace mpl = json::utility::mpl;
    
    //
    // integral_number
    //
    
    // An integral number uses an integral primitive type as the internal repre-
    // sentation of a JSON number. The integral number shall be able to exactly
    // represent an integral JSON number.
    
    template <typename T = long,
        typename = typename std::enable_if<
            json::is_numeric<T>::value and std::is_integral<T>::value
        >::type
    >
    class integral_number
    {
        typedef std::integral_constant<int, std::numeric_limits<T>::digits10 + 1> string_size_max;
    
    public:
        
        typedef T value_type;
        
        // default ctor
        integral_number() noexcept = default;
        
        // copy ctor
        constexpr integral_number(integral_number const& other) noexcept
        : value_(other.value_)
        {}
        
        // integral number conversion ctor
        template <typename U,
            typename Enable = typename std::enable_if<
                json::is_numeric<mpl::Unqualified<U>>::value and std::is_integral<mpl::Unqualified<U>>::value
            >::type        
        >
        constexpr integral_number(U const& value) noexcept
        : value_{value}
        {}
        
        
        // string conversion ctor
        explicit integral_number(const char* s, std::size_t len)
        : value_(convert(s, len))
        {            
        }
        
        
        // assignment
        integral_number& operator=(integral_number const& other) {
            value_ = other.value_;
            return *this;
        }
        
        template <typename U,
            typename Enable = typename std::enable_if<
                json::is_numeric<U>::value and std::is_integral<U>::value
            >::type
        >
        integral_number& operator=(T const& value)
        {
            value_ = value;
            return *this;
        }
            

        
        // type operator
        operator T() const noexcept { return value_; }
        
        
        std::string to_string() const noexcept {
            return json::utility::number_to_string(value_);
        }
        
        
    private:
        
        static T convert(char const*s, std::size_t len) {
            return json::utility::string_to_number<T>(s, len);
        }
        
        
        // comparison operator
        friend inline
        bool operator==(integral_number const& lhv, integral_number const& rhv) {
            return lhv.value_ == rhv.value_;
        }
        
        // ostream support:
        friend inline
        std::ostream& operator<<(std::ostream& o, const integral_number& v) {
            o << v.to_string();
            return o;
        }
        
        
    private:
        
        T value_;
    };

}





namespace json {
    
    
//    template <typename T>
//    struct is_json_type<integral_number<T> > : public std::true_type
//    {
//    };
    
} // namespace json



#endif  // JSON_INTEGRAL_NUMBER_HPP

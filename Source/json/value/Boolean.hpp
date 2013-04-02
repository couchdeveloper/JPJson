//
//  Boolean.hpp
//
//  Created by Andreas Grosam on 5/14/11.
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

#ifndef JSON_BOOLEAN_HPP
#define JSON_BOOLEAN_HPP


#include "json/config.hpp"
#include "json_traits.hpp"
#include "json/utility/mpl.hpp"
#include <type_traits>
#include <iostream>


namespace json {
    
#pragma mark -
#pragma mark json::Boolean
    
    namespace mpl = json::utility::mpl;
    
    //
    // Typesafe Boolean
    //
    
    
    class Boolean
    {
        struct bool_value {
            void nonnull() {}
        };
        typedef void (bool_value::*safe_bool)();
        
    public:
        
        Boolean() noexcept = default;
        
        constexpr Boolean(Boolean const& other) noexcept = default;
        
        // Implicit conversions (T => Boolean) are allowed only from type bool
        template <typename T,
                typename = typename std::enable_if<std::is_same<bool, T>::value
            >::type
        >
        constexpr Boolean(T v) noexcept
        : value_(v)
        {
        }
        
        // assignment
        Boolean& operator=(const Boolean& other) noexcept
        {
            value_ = other.value_;
            return *this;
        }
        
        
        // Safe bool operator
        operator safe_bool() const  noexcept {
            return value_ == true ? &bool_value::nonnull : 0;
        }

        // Conversion operator bool
        constexpr explicit operator bool () const noexcept { return value_; }
        
        
        // Not operator
        Boolean operator! () const noexcept { return not value_; }
        
        
        // Comparison Operators (defined inline as friend free functions)
        
        // bool operator== (ConvertibleToBoolean lhv, ConvertibleToBoolean rhv)
        template <typename T, typename U,
            typename = typename std::enable_if<
                std::is_convertible<T, Boolean>::value
                and std::is_convertible<U, Boolean>::value
            >::type
        >
        friend inline
        bool operator== (const T& lhv, const U& rhv) {
            return static_cast<bool>(lhv) == static_cast<bool>(rhv);
        }
        
        // bool operator!= (ConvertibleToBoolean lhv, ConvertibleToBoolean rhv)
        template <typename T, typename U,
            typename = typename std::enable_if<
                std::is_convertible<T, Boolean>::value
                and std::is_convertible<U, Boolean>::value
            >::type
        >
        friend inline
        bool operator!= (const T& lhv, const U& rhv) {
            return static_cast<bool>(lhv) != static_cast<bool>(rhv);
        }
        
        
        
        // ostream support
        friend inline
        std::ostream& operator<< (std::ostream& os, Boolean b) {
            std::ios::fmtflags flags = os.setf(std::ios::boolalpha);
            os << b.value_;
            os.setf(flags);
            return os;
        }

    
    private:
        bool value_;
        
    };
    
    
//    template <>
//    struct is_json_type<Boolean> : std::true_type
//    {
//    };
    
    
}  // namespace json


#endif // JSON_BOOLEAN_HPP

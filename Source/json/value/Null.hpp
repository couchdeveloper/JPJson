//
//  Null.hpp
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

#ifndef JSON_NULL_HPP
#define JSON_NULL_HPP


#include "json/config.hpp"
#include "json_traits.hpp"
#include <iostream>


namespace json {
    
#pragma mark -
#pragma mark json::Null
    
    //
    // Null
    //

    //
    //  The following traits should be defined when used in a boost::variant:
    //      has_nothrow_copy, 
    //      has_nothrow_constructor, 
    //      has_nothrow_default_constructor
    
    class Null {
    public:    
        constexpr Null() noexcept = default;
        constexpr Null(const Null&) noexcept = default;
        Null& operator=(const Null&) noexcept = default;
        
    private:
        // friends
        
        friend inline
        bool operator==(const Null&, const Null&) noexcept { return true; }

        friend inline
        bool operator!=(const Null&, const Null&) noexcept { return false; }
        
        
        friend inline
        std::ostream& operator<<(std::ostream& o, const Null&) {
            o << "null"; return o;
        }
    };
    
    
    static const constexpr Null null = {};  // Constant null can be used to initialize instances of variant Value: Value v = null;
    
    
//    template <> 
//    struct is_json_type<Null> : public std::true_type
//    {
//    };
//    
} // namespace json


#if !defined (USE_JSON_UTILITY_VARIANT)
#include <boost/type_traits.hpp>
namespace boost {
    
    // The following traits must be defined for json::Null when used in a 
    // boost::variant:
    
    template <>
    struct has_nothrow_copy< json::Null > : std::true_type {};
    
    template <>    
    struct has_nothrow_constructor< json::Null >  : std::true_type {};
    
    template <>
    struct has_nothrow_default_constructor< json::Null > : std::true_type {};
}
#endif

#endif // JSON_NULL_HPP

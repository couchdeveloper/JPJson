//
//  json_traits.hpp
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

#ifndef JSON_JSON_TRAITS_HPP
#define JSON_JSON_TRAITS_HPP


#include "json/config.hpp"
#include <type_traits>


namespace json 
{
    
//    // 
//    // json type trait
//    //
//    template <typename T> 
//    struct is_json_type : std::false_type
//    { 
//    };
    

    //
    // is_numeric type trait
    //
    // The is_numeric type trait is used for construction and assignment to
    // instances of type Number.
    // is_numeric trait comprises a C++ arithmetic type excluding the type bool
    // and character types.
    //
    template <typename T>
    struct is_numeric : std::conditional<
            (std::is_same<bool,T>::value
             or std::is_same<char,T>::value
             or std::is_same<unsigned char,T>::value
             or std::is_same<signed char,T>::value
             or std::is_same<char16_t,T>::value
             or std::is_same<char32_t,T>::value),
            std::false_type,
            std::is_arithmetic<T>
        >::type
    {
    };
    
    

}  // namespace json



#endif   // JSON_JSON_TRAITS_HPP

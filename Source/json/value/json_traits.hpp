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


#include <boost/config.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>


namespace json 
{
    
    // 
    // json type trait
    //
    template <typename T> 
    struct is_json_type : public boost::mpl::false_ 
    { 
        static const bool value = false; 
    };
    

    //
    // is_numeric type trait
    //
    // The is_numeric type trait is used for construction and assignment for
    // instances of type Number.
    // is_numeric triat comprises a C++ arithmetic type excluding the type bool.
    //
    template <typename T>
    struct is_numeric : boost::is_arithmetic<T> {};
    
    template <>
    struct is_numeric<bool> {
        static const bool value = false;
    };
    
    

}  // namespace json


#if defined (BOOST_NO_RVALUE_REFERENCES)
namespace json {
    namespace internal
    {
        template <typename T>
        struct move_t {
            explicit move_t(T& value) : source(value) {}
            T& source;
        };
        
    } // namespace internal   
    
    template <typename T>
    internal::move_t<T> 
    move(T& v) {
        return internal::move_t<T>(v);
    }
    
} // namespace json
#endif

#endif   // JSON_JSON_TRAITS_HPP

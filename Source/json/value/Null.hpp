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


#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <iostream>
#include "json_traits.hpp"


namespace json {
    
#pragma mark -
#pragma mark json::Null
    //
    // Null
    //
    // TODO: implement a "null" literal
    //
    //  The following traits should be defined when used in a boost::variant:
    //      has_nothrow_copy, 
    //      has_nothrow_constructor, 
    //      has_nothrow_default_constructor
    
    class Null {
    public:    
        Null() throw() {}
    };
    
    template <typename T>
    inline bool operator==(const T&, const Null&) { return false; }
    
    template <typename T>
    inline bool operator==(const Null&, const T&) { return false; }
    
    inline bool operator==(const Null&, const Null&) { return true; }
    
    inline std::ostream& operator<<(std::ostream& o, const Null&) {
        o << "null"; return o;
    }
    
    static const Null null;  // Constant null can be used to initialize instances of variant Value: Value v = null;
    
    template <> 
    struct is_json_type<Null> : public boost::mpl::true_
    { 
        static const bool value = true; 
    };    
    
} // namespace json

namespace boost {
    

    // The following traits must be defined for json::Null when used in a 
    // boost::variant:
    
    template <>
    struct has_nothrow_copy< json::Null > : mpl::true_ {};
    
    template <>    
    struct has_nothrow_constructor< json::Null >  : mpl::true_ {};
    
    template <>
    struct has_nothrow_default_constructor< json::Null > : mpl::true_ {};
}


#endif // JSON_NULL_HPP

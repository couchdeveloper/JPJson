//
//  string_hasher.hpp
//  json_parser
//
//  Created by Andreas Grosam on 7/16/11.
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

#ifndef JSON_UTILITY_STRING_HASHER_HPP
#define JSON_UTILITY_STRING_HASHER_HPP


#include "json/config.hpp"
#include <boost/config.hpp>
#include <boost/functional/hash.hpp>


namespace json { namespace utility {
    
    //    
    // Hash function for CharT arrays
    //
    static const std::size_t SEED = 0;
    
    template <typename CharT>
    struct string_hasher
    {
        
        // str must be zero terminated
        std::size_t operator()(const CharT* str) const
        {  
            std::size_t seed = SEED;
            for(; *str; ++str)   boost::hash_combine(seed, *str);
            return seed;
        }
        std::size_t operator()(const CharT* first, std::size_t len) const
        {  
            std::size_t seed = SEED;
            for(; len; --len, ++first)   boost::hash_combine(seed, *first);
            return seed;
        }
    };   
    
    
}}  // namespace json::internal


#endif // JSON_UTILITY_STRING_HASHER_HPP
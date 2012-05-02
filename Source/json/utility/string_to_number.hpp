//
//  string_to_number.hpp
//  
//
//  Created by Andreas Grosam on 7/29/11.
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

#ifndef JSON_UTILITY_STRING_TO_NUMBER_HPP
#define JSON_UTILITY_STRING_TO_NUMBER_HPP


#include "json/config.hpp"

/**
 If JSON_UTILITY_STRING_TO_NUMBER_USE_QI is defined, the implemenation uses
 boost::spirit::qi for string to number conversions. This is slightly faster
 that when using std conversion defined in header <xlocale.h>. 
 Otherwise, the implementation uses strtol_l() and strtoll_l() defined in
 header <xlocale.h> in order to convert a string to a number.
 */



#if defined (JSON_UTILITY_STRING_TO_NUMBER_USE_QI)
#if defined(nil) or defined(Nil)
    #error  This header must be included before any Foundation header!
#endif
#endif



#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <xlocale.h>
#include <cerrno>
#include <stdlib.h>  // numeric conversion
#include <boost/static_assert.hpp>
#if defined (JSON_UTILITY_STRING_TO_NUMBER_USE_QI)
#include <boost/spirit/include/qi.hpp>
#endif

namespace json { namespace utility {
    
    
    
    inline void throw_number_conversion_error(const char* msg) {
        throw std::runtime_error(msg);
    }
    

    // T shall be an arithmentic type
    
    template <typename T>
    inline T string_to_number(char const*  str, size_t len) 
    {
        BOOST_STATIC_ASSERT(sizeof(T) != 0);
    }
    
    
    
#if !defined (JSON_UTILITY_STRING_TO_NUMBER_USE_QI)
    template <>
    inline int string_to_number<int>(char const*  str, size_t len) 
    {
        char* endPtr;
        return static_cast<int>(strtol_l(str, &endPtr, 10, NULL));
    }
    
    template <>
    inline long string_to_number<long>(char const*  str, size_t len) 
    {
        char* endPtr;
        return strtol_l(str, &endPtr, 10, NULL);
    }
    
    template <>
    inline long long string_to_number<long long>(char const*  str, size_t len) 
    {
        char* endPtr;
        errno = 0;
        long long result = strtoll_l(str, &endPtr, 10, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: Conversion to long long failed");
        }
        return result;
    }
    
    template <>
    inline double string_to_number<double>(char const*  str, size_t len) 
    {
        char* endPtr;
        errno = 0;
        double result = strtod_l(str, &endPtr, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: floating point value out of range");
        }
        return result;
    }
    
#endif  
    

    
#if defined (JSON_UTILITY_STRING_TO_NUMBER_USE_QI)
    
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    
    template <>
    inline int string_to_number<int>(char const*  str, size_t len) 
    {
        int val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::int_, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to int failed");
        return val;
    }
    
    template <>
    inline long string_to_number<long>(char const*  str, size_t len) 
    {
        long val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::long_, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to int failed");
        return val;
    }
    
    template <>
    inline long long string_to_number<long long>(char const*  str, size_t len) 
    {
        long long val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::long_long, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to int failed");
        return val;
    }
    
    template <>
    inline double string_to_number<double>(char const*  str, size_t len) 
    {
        double val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::double_, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to int failed");
        return val;
    }
    
#endif  
    
 
    
}}




#endif  // JSON_UTILITY_STRING_TO_NUMBER_HPP

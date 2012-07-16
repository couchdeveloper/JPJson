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
#include <limits>
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
    

    // The following conversion function assume that the str is a valid 
    // JSON number string and len is set correctly. The number string shall 
    // not be preceeded with whitespaces or an optional '+' sign.
    // If the conversion result is an *unsigned* integral type there must 
    // be no minus sign.
    
#define JSON_UTILITY_STRING_TO_NUMBER_CHECK_RESULT
    
    
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
        long result = strtol_l(str, &endPtr, 10, NULL);
        if (result > std::numeric_limits<int>::max() or result < std::numeric_limits<int>::min())
            throw_number_conversion_error("ERROR: Conversion to int is out of range");
        return static_cast<int>(result);
    }

    
    template <>
    inline unsigned int string_to_number<unsigned int>(char const*  str, size_t len) 
    {
        char* endPtr;
        unsigned long result = strtoul_l(str, &endPtr, 10, NULL);
        if (result > std::numeric_limits<unsigned int>::max())
            throw_number_conversion_error("ERROR: Conversion to unsigned int is out of range");
        return static_cast<unsigned int>(result);
    }
    
    template <>
    inline long string_to_number<long>(char const*  str, size_t len) 
    {
        char* endPtr;
        errno = 0;
        long result = strtol_l(str, &endPtr, 10, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: Conversion to long is out of range");
        }
        return result;
    }
                                          
    template <>
    inline unsigned long string_to_number<unsigned long>(char const*  str, size_t len) 
    {
        char* endPtr;
        errno = 0;
        unsigned long result = strtoul_l(str, &endPtr, 10, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: Conversion to unsigned long is out of range");
        }
        return result;
    }
    
                                          
    
    template <>
    inline long long string_to_number<long long>(char const*  str, size_t len) 
    {
        char* endPtr;
        errno = 0;
        long long result = strtoll_l(str, &endPtr, 10, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: Conversion to long long is out of range");
        }
        return result;
    }
    
    template <>
    inline unsigned long long string_to_number<unsigned long long>(char const*  str, size_t len) 
    {
        char* endPtr;
        errno = 0;
        unsigned long long result = strtoull_l(str, &endPtr, 10, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: Conversion to unsigned long long is out of range");
        }
        return result;
    }
    

    template <>
    inline float string_to_number<float>(char const*  str, size_t len)
    {
        char* endPtr;
        errno = 0;
        float result = strtof_l(str, &endPtr, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: float value out of range");
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
            throw_number_conversion_error("ERROR: double value out of range");
        }
        return result;
    }
    
    template <>
    inline long double string_to_number<long double>(char const*  str, size_t len)
    {
        char* endPtr;
        errno = 0;
        long double result = strtold_l(str, &endPtr, NULL);
        if (errno == ERANGE) {
            throw_number_conversion_error("ERROR: long double value out of range");
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
    inline unsigned int string_to_number<unsigned int>(char const*  str, size_t len) 
    {
        unsigned int val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::uint_, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to unsigned int failed");
        return val;
    }
    

    template <>
    inline long string_to_number<long>(char const*  str, size_t len) 
    {
        long val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::long_, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to long failed");
        return val;
    }
    
    template <>
    inline unsigned long string_to_number<unsigned long>(char const*  str, size_t len) 
    {
        unsigned long val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::ulong_, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to unsigned long failed");
        return val;
    }
    
    template <>
    inline long long string_to_number<long long>(char const*  str, size_t len) 
    {
        long long val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::long_long, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to long long failed");
        return val;
    }
    
    template <>
    inline unsigned long long string_to_number<unsigned long long>(char const*  str, size_t len) 
    {
        unsigned long long val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::ulong_long, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to unsigned long long failed");
        return val;
    }
    
    template <>
    inline float string_to_number<float>(char const*  str, size_t len)
    {
        float val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::float_, val);
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to float failed");
        return val;
    }
    
    template <>
    inline double string_to_number<double>(char const*  str, size_t len)
    {
        double val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::double_, val);
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to double failed");
        return val;
    }
    
    template <>
    inline long double string_to_number<long double>(char const*  str, size_t len) 
    {
        long double val;
        bool result = boost::spirit::qi::parse(str, str+len, boost::spirit::long_double, val);        
        if (not result)
            throw_number_conversion_error("ERROR: Conversion to long double failed");
        return val;
    }
    
#endif  
    
 
    
}}




#endif  // JSON_UTILITY_STRING_TO_NUMBER_HPP

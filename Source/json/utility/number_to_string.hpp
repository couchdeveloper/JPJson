//
//  number_to_string.hpp
//  
//
//  Created by Andreas Grosam on 22.02.13.
//
//

#ifndef JSON_UTILITY_NUMBER_TO_STRING_HPP
#define JSON_UTILITY_NUMBER_TO_STRING_HPP

#include "json/config.hpp"
#include <type_traits>

/**
 If JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA is defined, the implemenation uses
 boost::spirit::karma for number to string conversions. 
 */



#if defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)
#if defined(nil) or defined(Nil)
#error  This header must be included before any Foundation header!
#endif
#endif



#include <stdexcept>
#include <string>
#if defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)
#include <boost/spirit/include/karma.hpp>
#endif


namespace json { namespace utility {
    
    
    
    // T shall be an arithmentic type
    
//    template <typename T, typename Enable = void>
//    inline std::string number_to_string(T const& number)
//    {
//        static_assert(sizeof(T) != 0, "");
//    }
    
    
    
#if !defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)
    
    template <typename T,
        typename Enable = typename std::enable_if<std::is_arithmetic<T>::value>::type
    >
    inline std::string number_to_string(T const& value)
    {
        return std::to_string(value);
    }
    
    
    template <typename T, typename OutputIterator>
    OutputIterator write_number(T const& value, OutputIterator dest
                                , typename std::enable_if<std::is_arithmetic<T>::value>::type* = 0)
    {
        std::string s = std::to_string(value);
        return std::copy(s.begin(), s.end(), dest);
    }
    
    
#endif
    
    
    
#if defined (JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA)
    
    
    template <typename T
        //typename Enable = typename std::enable_if<std::is_floating_point<T>::value>::type
    >
    struct json_float_number_policy : boost::spirit::karma::real_policies<T>
    {
        // we want the numbers always to be in scientific format
        static constexpr int floatfield(T) { return boost::spirit::karma::real_policies<T>::fmtflags::scientific; }
        
        static constexpr bool trailing_zeros(T) { return false; }
        
        static constexpr unsigned precision(T) { return std::numeric_limits<T>::digits10 + 1; }
    };
    
    
    
    
    template <typename T>
    std::string number_to_string(T const& value, typename std::enable_if<std::is_integral<T>::value>::type* = 0)
    {
        namespace karma = boost::spirit::karma;
        std::string str;
        karma::int_generator<T, 10, false> generator;
        /*bool result =*/ karma::generate(std::back_inserter(str), generator, value);
        return str;
    }
    
    
    template <typename T>
    std::string number_to_string(T const& value, typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
    {
        namespace karma = boost::spirit::karma;
        std::string str;
        /*bool result =*/ karma::generate(std::back_inserter(str), value);
        return str;
    }
    
    

    template <typename T, typename OutputIterator>
    OutputIterator write_number(T const& value, OutputIterator dest
                                , typename std::enable_if<std::is_integral<T>::value>::type* = 0)
    {
        namespace karma = boost::spirit::karma;
        
        OutputIterator out = dest;
        karma::int_generator<T, 10, false> generator;
        /*bool result =*/ karma::generate(out, generator, value);
        return out;
    }
    
    template <typename T, typename OutputIterator, template <typename> class Formatter = json_float_number_policy>
    OutputIterator write_number(T const& value, OutputIterator dest
                                , typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
    {
        namespace karma = boost::spirit::karma;
        
        // define a new generator type based on the formatter
        typedef karma::real_generator<T, Formatter<T> > generator_type;        
        OutputIterator out = dest;
        karma::generate(out, generator_type(), value);
        return out;
    }
    
    
    
//    template <typename T,
//    typename Enable = typename std::enable_if<json::is_decimal<T>::value>::type
//    >
//    char* number_to_string(T const& value, char* buffer, std::size_t buffer_size)
//    {
//        return buffer;  // TODO: implement
//    }
    
#endif
    
    
    
}}




#endif // JSON_UTILITY_NUMBER_TO_STRING_HPP

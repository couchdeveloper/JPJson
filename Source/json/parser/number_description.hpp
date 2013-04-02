//
//  number_description.h
//  
//
//  Created by Andreas Grosam on 01.03.13.
//
//

#ifndef _number_description_h
#define _number_description_h



#include "json/config.hpp"
#include <utility>
#include <cassert>

namespace json {
    
    
    // The JSON parser initializes a number_description instance when parsing
    // a JSON number.
    
    class number_description
    {
    public:
        typedef std::pair<char const*, size_t> const_buffer_type;  // null terminated!
        
        enum NumberType {
            Scientific = 0,     // number using a mantissa and exponet and possibly having a decimal point
            Integer,            // signed number
            UnsignedInteger,    // unsigend number
            Decimal,            // signed number with a decimal point but no exponent
            UnsignedDecimal     // unsigned number with a decimal point but no exponent
        };
        
        number_description() noexcept {}
        
        
        number_description(const_buffer_type const& buffer, NumberType numberType, short precision) noexcept
        :  number_string_(buffer), digits_(precision), type_(numberType)
        {
            assert(number_string_.second >= 0 and number_string_.first[number_string_.second] == 0);  // null terminated
        }
        
        number_description(const number_description& other) noexcept
        :  number_string_(other.number_string_), digits_(other.digits_), type_(other.type_)
        {
            assert(number_string_.second >= 0 and number_string_.first[number_string_.second] == 0);  // null terminated
        }
        
        void assign(const number_description& other) noexcept
        {
            assert(other.number_string_.second >= 0 and other.number_string_.first[other.number_string_.second] == 0);  // null terminated
            number_string_ = other.number_string_;
            digits_ = other.digits_;
            type_ = other.type_;
        }
        
        number_description& operator=(const number_description& other) noexcept
        {
            assign(other);
            return *this;
        }
        
        NumberType number_type() const      { return type_; }
        
        char const* c_str() const           { return number_string_.first; }
        std::size_t c_str_len() const       { return number_string_.second; }
        
        short       digits() const          { return digits_; }
        bool        is_signed() const       { return type_ == Scientific or type_ == Decimal or type_ == Integer; }
        bool        is_unsigned() const     { return type_ == UnsignedInteger or type_ == UnsignedDecimal; }
        bool        is_integer() const      { return type_ == Integer or type_ == UnsignedInteger; }
        bool        is_decimal() const      { return type_ == Decimal or type_ == UnsignedDecimal; }
        bool        is_scientific() const   { return type_ == Scientific; }
        
    private:
        const_buffer_type   number_string_;
        short               digits_;      // number of digits for integer part and fractional part
        NumberType          type_;
    };

    
}  // namespace json

#endif

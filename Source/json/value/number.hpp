//
//  number.hpp
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

#ifndef JSON_NUMBER_HPP
#define JSON_NUMBER_HPP


#include <boost/config.hpp>
#include <boost/utility/enable_if.hpp>
#if !defined (JSON_NO_BOOST_LEXICAL_CAST)
    #include <boost/lexical_cast.hpp>
#endif
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/bool.hpp>
#include <stdexcept>
#include <string.h>
#include <string>
#include <iostream>
#include "json_traits.hpp"
#include <assert.h>


namespace json { namespace internal {

    enum number_type {
        BAD_NUMBER,
        Integer,
        Decimal,
        Float        
    };

    template <typename CharT>
    inline 
    number_type validate_number(const CharT* first, const CharT* last)
    {
        enum number_state {
            number_state_start,
            number_state_sign,
            number_int_is_zero,
            number_state_int,
            number_state_point,
            number_state_fractional,
            number_state_exponent_start,
            number_state_exponent_sign,
            number_state_exponent
        };
        
        number_type numberType = BAD_NUMBER;
        
        number_state s = number_state_start;
        bool isNegative = false;
        bool exponentIsNegative = false;        
        bool done = false;
        while (not done)
        {
            // Note: implicit conversion from CharT to char. It is assumed the 
            // conversion is valid. Valid characters are ASCIIs only.
            // TODO: possibly check errors here
            char ch = *first;
            
            switch (s) {
                case number_state_int:
                    switch (ch) {
                        case '0'...'9': break;
                        case '.': s = number_state_point;  break;
                        case 'e':
                        case 'E': s = number_state_exponent_start; break;
                        default: done = true; // finished with integer
                    }
                    break;
                case number_state_fractional:
                    switch (ch) {
                        case '0'...'9':  break;
                        case 'e':
                        case 'E': s = number_state_exponent_start; break;
                        default: done = true; // finished with fractional or start exponent
                    }
                    break;
                case number_state_exponent:
                    switch (ch) {
                        case '0' ... '9':  break;
                        default: done = true;  // finished
                    }
                    break;
                default:
                case number_state_start:
                    switch (ch) {
                        case '-': s = number_state_sign; isNegative = true;  break;
                        case '0': s = number_int_is_zero; break;
                        case '1'...'9': s = number_state_int; break;
                        default: done = true; // error: not a number
                    }
                    break;
                case number_state_sign:
                    switch (ch) {
                        case '0':       s = number_int_is_zero; break;
                        case '1'...'9': s = number_state_int; break;
                        default: done = true;  // error: expecting a digit
                    }
                    break;
                case number_int_is_zero:
                    switch (ch) {
                        case '.': s = number_state_point; break;
                        case 'e':
                        case 'E': s = number_state_exponent_start; break;
                        default: done = true; // finished.
                    }
                    break;
                case number_state_point:
                    switch (ch) {
                        case '0'...'9': s = number_state_fractional; break;
                        default: done = true; // error: expected digit
                    }
                    break;
                case number_state_exponent_start:
                    switch (ch) {
                        case '-': s = number_state_exponent_sign; exponentIsNegative = true; break;
                        case '+': s = number_state_exponent_sign; break;
                        case '0' ... '9': s = number_state_exponent; break;
                        default: done = true;  // error
                    }
                    break;
                case number_state_exponent_sign:
                    switch (ch) {
                        case '0' ... '9': s = number_state_exponent; break;
                        default: done = true;  // finished                            
                    }
                    break;
                    assert(0);
            } //switch
            
            if (not done and first != last)
                ++first;
            
        } // while (not done)
        
        switch (s) {
            case number_int_is_zero:    
            case number_state_int:
                numberType = Integer; 
                break;
                
            case number_state_fractional:
                numberType = Decimal; 
                break;
                
            case number_state_exponent:
                numberType = Float; 
                break;
                
            default:
                numberType = BAD_NUMBER;
        }
        
        if (first != last)
            numberType = BAD_NUMBER;
        
        return numberType;
    }
    
    /*
     template <typename IntegralT>        
     std::string integer_to_string(IntegralT const& value, 
     typename boost::enable_if<is_integral<IntegralT>, IntegralT>::type* _ = 0) 
     {
     namespace karma = boost::spirit::karma;
     std::string result;
     karma::int_generator<IntegralT, 10, false> generator;
     return karma::generate(std::back_inserter(result), value);            
     }
     
     template <typename FloatT>        
     std::string float_to_string(FloatT const& value, 
     typename boost::enable_if<is_floating_point<FloatT>, FloatT>::type* _ = 0) 
     {
     std::string result;
     namespace karma = boost::spirit::karma;
     return karma::generate(std::back_inserter(result), value);            
     }
     */ 
        
}} // namespace json::internal


#pragma mark -
#pragma mark json::number
namespace json {
    
    namespace mpl = boost::mpl;
    

    //
    // number
    //
    template <typename CharT>
    class number {
    private:        
        static const unsigned int capacity_ = 48; // shall be even
        typedef CharT char_t;
    public:      
        
        typedef char unpacked_type[capacity_ + 1];  // capazity chars plus one char for termination
        
        number() { memset(value_, 0xFF, sizeof(value_)); }
        
        number(const CharT* s) {
            assign(s);
        }
        
        number(const CharT* s, std::size_t len) {
            assign(s, len);
        }
        
#if !defined (JSON_NO_BOOST_LEXICAL_CAST)
        template <typename T>
        number(const T& v,
               typename boost::enable_if<is_numeric<T>, T>::type* _ = 0 )
        {
            // Convert a numeric value into a string using default formatting rules
            // (TODO: this is by far not the fastest method, implement a faster one
            std::string s = boost::lexical_cast<std::string>(v);
            // finally, compress the string
            pack(s.c_str(), s.size());
        }
#endif
        template <typename T>
        number(const T& v, const char* format,
               typename boost::enable_if<is_numeric<T>, T>::type* _ = 0 )
        {
            unpacked_type buffer; 
            int n = ::snprintf (buffer, sizeof(buffer), format, v);
            if (n < 0 or n > (int)(sizeof(buffer))) {
                memset(value_, 0xFF, sizeof(value_));            
            }
            else  {
                pack(buffer, n);
            }
        }
        
#if !defined (JSON_NO_BOOST_LEXICAL_CAST)
  // TODO: remove lexical cast      
        
        template <typename T>
        void assign(const T& v,
                    typename boost::enable_if<is_numeric<T>, T>::type* _ = 0 )
        {
            std::string s = boost::lexical_cast<std::string>(v);
            pack(s.c_str(), s.size());
        }
#endif        
        
        void assign(const number& other) {
            memcpy(value_, other.value_, sizeof(value_));
        }
        
        
        void assign(const CharT* s, size_t len) {
            if (s and len) {
                internal::number_type result = internal::validate_number(s, s+len);
                if (result != internal::BAD_NUMBER) {
                    pack(s, len);
                }
                else {
                    throw std::runtime_error("bad json::number");
                }
            }
            else {
                memset(value_, 0xFF, sizeof(value_));
            }
        }
        
        void assign(const CharT* s) {
            size_t len = 0;
            const CharT* p = s;
            while (*p) {
                ++len;
                ++p;
            }
            assign(s, len);
        }
        
        number& operator=(const number& other) {
            assign(other);
            return *this;
        }
        
        number& operator=(const char* s) {
            assign(s);
            return *this;
        }
        
        template <typename T>
        typename boost::enable_if <
            is_numeric<T>,
            number&
        >::type
        operator=(const T& v) {
            assign(v);
            return *this;
        }
        
#if !defined (JSON_NO_BOOST_LEXICAL_CAST)
        // TODO: remove lexical cast      
        
        template <typename NumericValue>
        typename boost::enable_if <
            is_numeric<NumericValue>,
            NumericValue
        >::type    
        as() const 
        {
            unpacked_type buffer;
            unpack(buffer);
            NumericValue result = boost::lexical_cast<NumericValue>(buffer);
            return result;
        }
#endif
        
        
        size_t capacity() const { return capacity_; }
        
        // template<typename Encoding>
        std::string string() const {
            std::string result;
            if (*value_ == 0xFF)
                result = "NaN";
            else  {
                unpacked_type buffer;
                size_t size = unpack(buffer);
                result.assign(buffer, size);
            }
            return result;
        }
        
        
        
    private:
        bool is_equal(number const&  other) const {
            bool result = (memcmp(value_, other.value_, sizeof(value_)) == 0);
            return result;
        }
        
        typedef unsigned char    packed_type[capacity_ / 2];
        
        void pack(const CharT* s, size_t len);
        size_t unpack(unpacked_type& buffer) const; 
        
        //void pack2(const char* s, size_t len);
        //unsigned char loockup(char c) const;
        
        
    private:        
        typedef unsigned char value_type[capacity_/2];
        value_type value_;
        
        
        
        // The following code defines comparison operators if at least
        // one number type is provided as argument. It seems a little bit verbose, 
        // as it tries to prevent comparison of types which are not safe to 
        // be comapared yet. If such comparison is tried, the compiler would
        // issue an error.
        //
        // Comparison Operators (defined inline as friend free functions)
        
        template <typename U>
        friend inline 
        typename boost::enable_if<
            mpl::and_<
                boost::is_integral<U>
              , mpl::not_< boost::is_same<U, bool> >
            >
            , bool>::type
        operator== (const number& lhv, const U& rhv) {
            return lhv.is_equal(rhv);
        }
        
        template <typename U>
        friend inline 
        typename boost::enable_if<
            mpl::and_<
                boost::is_integral<U>
              , mpl::not_< boost::is_same<U, bool> >
            >
            , bool>::type
        operator== (const U& lhv, const number& rhv) {
            return rhv.is_equal(lhv);
        }
        
        template <typename U>
        friend inline 
        typename boost::enable_if<
            mpl::and_<
                boost::is_integral<U>
              , mpl::not_< boost::is_same<U, bool> >
            >
            , bool>::type
        operator!= (const number& lhv, const U& rhv) {
            return not lhv.is_equal(rhv);
        }
        
        template <typename U>
        friend inline 
        typename boost::enable_if<
                mpl::and_<
                boost::is_integral<U>
              , mpl::not_< boost::is_same<U, bool> >
            >
            , bool>::type
        operator!= (const U& lhv, const number& rhv) {
            return not rhv.is_equal(lhv);
        }
        
        // Defining the operator in the following way, prevents instantiation 
        // for non-number types which are though convertible to number. We currently
        // won't allow floating point numbers to be compared with number, e.g.:
        //      number n = ...;
        //      if (n == 1.0)      // compiler issues error  
        //          ...
        
        template <typename NumberT>
        friend inline 
        typename boost::enable_if<
            boost::is_same<NumberT, number>
          , bool>::type
        operator== (const NumberT& lhv, const NumberT& rhv) {
            return lhv.is_equal(rhv);
        }
        
        template <typename NumberT>
        friend inline 
        typename boost::enable_if<
            boost::is_same<NumberT, number>
          , bool>::type
        operator!= (const NumberT& lhv, const NumberT& rhv) {
            return not lhv.is_equal(rhv);
        }
        
        
    };  // class Number
    
} // namespace json



namespace json {

    //
    //      number implementation
    //
    
    template <typename CharT>
    inline
    void number<CharT>::pack(const CharT* s, size_t len) 
    {
        if (s == 0 or len == 0) {
            memset(value_, 0xFF, sizeof(value_));
            return;
        }
        
        if (len > capacity_)
            throw std::runtime_error("json::number range error");
        

        const CharT* first = s;
        const CharT* last = first + len;
        bool high = false;
        unsigned char buffer = 0xFF;
        unsigned char c = 0xFF;
        unsigned char* p = value_;
        while (first != last) 
        {
            assert( int(char(*first)) == int(*first));            
            
            // convert a CharT to char. It is assumed the conversion is valid. 
            // valid characters are ASCIIs only.
            // TODO: possibly check errors here
            char ch = *first++;  
            
            switch (ch) {
                case '0' ... '9': 
                    c = ch-'0'; 
                    break;
                case '+':
                    continue; 
                    break;
                case '-': c = 0x0C; break;
                case 'e': 
                case 'E': c = 0xE ;  break;
                case '.': c = 0xD;  break;
                default:
                    *value_ = 0xFF;
            };
            
            high = !high;
            if (high) {
                buffer = (c << 4) | 0x0F;
            } 
            else {
                buffer &= 0xF0 | c;
                *p++ = buffer;
            }
        }
        
        if (high) {
            *p++ = buffer;
        } 
        unsigned char* last_v = value_ + sizeof(value_);
        memset(p, 0xFF, last_v - p);
    }
    
    
    template <typename CharT>
    inline 
    size_t number<CharT>::unpack(unpacked_type& buffer) const 
    {
        const char map[16] = "0123456789A+-.e";
        char* op = buffer;
        unsigned char const* ip = value_;
        unsigned int i = 0;
        while (i < sizeof(unpacked_type)) {
            *op = map[(*ip >> 4)];
            if (*op++ == 0x00) 
                break;
            ++i;
            *op = map[0x0F & *ip];
            if (*op++ == 0x00) 
                break;
            ++ip;
            ++i;
        }
        buffer[i] = 0; // terminate
        return i;
    }
    

} // namespace json


namespace json {
    
    
    template <typename CharT>
    inline std::ostream& operator<<(std::ostream& o, const number<CharT>& v) {
        o << v.string(); 
        return o;
    }
    
    
    template <typename CharT> 
    struct is_json_type<number<CharT> > : public boost::mpl::true_
    { 
        static const bool value = true; 
    };
    
} // namespace json




namespace json {
    
    typedef number<uint8_t> Number;
    
}

#endif // JSON_NUMBER_HPP
//
//  unicode_conversion.hpp
//  
//
//  Created by Andreas Grosam on 2/23/12.
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

#ifndef JSON_UNICODE_CONVERSION_HPP
#define JSON_UNICODE_CONVERSION_HPP


#include "json/config.hpp"
#include "unicode_converter.hpp"
#include "unicode_errors.hpp"

#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/assert.hpp>


#define CONVERSION_CONVERT_FORCE_INLINE


//
//  Unicode Conversion Functions
//

namespace json { namespace unicode {
    
    template <typename EncodingT>
    struct mb_state : public internal::mb_state<typename encoding_traits<EncodingT>::encoding_form, true> 
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, EncodingT>::value) );

        typedef internal::mb_state<typename encoding_traits<EncodingT>::encoding_form, true> base;
        
        
        mb_state() : base() {}
        mb_state(mb_state const& other) : base(other) {}
        

        mb_state(base const& other) : base(other) {}        
        mb_state& operator=(base const& other) {
            base::operator=(other);
            return *this;
        }
        
        void clear() { base::clear(); }
        
        bool operator!() const { return base::operator!(); }
        
    };
    
}}



namespace json { namespace unicode {

    enum ConvertOption {
        None = 0,
        ReplaceIllFormed
    };
    
    template <
        typename InIteratorT, typename FromEncodingT, 
        typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
            OutIteratorT& dest, ToEncodingT toEncoding, 
            mb_state<FromEncodingT>& state,
            ConvertOption convertOption = None,
            typename boost::disable_if<
                boost::is_same<
                    typename std::iterator_traits<InIteratorT>::iterator_category, 
                    std::random_access_iterator_tag
                >    
            >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
        converter<FromEncodingT, ToEncodingT, Validation::SAFE> cvt(state);
        int result;
        while ((result = cvt.convert(first, last, dest)) != NO_ERROR 
                and result != E_UNEXPECTED_ENDOFINPUT 
                and convertOption != None) 
        {
            if (result == E_INVALID_START_BYTE) {
                ++first;
            }
            typedef converter<code_point_t, ToEncodingT, Validation::UNSAFE> code_point_cvt_t;
            code_point_t cp = kReplacementCharacter;
            code_point_t* cp_first = &cp;
            code_point_cvt_t().convert(cp_first, cp_first+1, dest);
            cvt.clear();
        }
        state = cvt.state();
        return result;
    }
    
    
    
    template <
        typename InIteratorT, typename FromEncodingT, 
        typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
            OutIteratorT& dest, ToEncodingT toEncoding, 
            ConvertOption convertOption = None,            
            typename boost::disable_if<
                boost::is_same<
                    typename std::iterator_traits<InIteratorT>::iterator_category, 
                    std::random_access_iterator_tag
                >    
            >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
        converter<FromEncodingT, ToEncodingT, Validation::SAFE> cvt;
        
        int result;
        while ((result = cvt.convert(first, last, dest)) != NO_ERROR 
               and convertOption != None) 
        {
            if (result == E_INVALID_START_BYTE) {
                ++first;
            }
            typedef converter<code_point_t, ToEncodingT, Validation::UNSAFE> code_point_cvt_t;
            code_point_t cp = kReplacementCharacter;
            code_point_t* cp_first = &cp;
            code_point_cvt_t().convert(cp_first, cp_first+1, dest);   
            if (result == E_UNEXPECTED_ENDOFINPUT) {
                return NO_ERROR;
            }
            cvt.clear();
        }
        
        return result;
    }
    
    
    
    
    template <
        typename InIteratorT, typename FromEncodingT, 
        typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
            OutIteratorT& dest, ToEncodingT toEncoding, 
            mb_state<FromEncodingT>& state,
            ConvertOption convertOption = None,            
            typename boost::enable_if<
                boost::is_same<
                    typename std::iterator_traits<InIteratorT>::iterator_category, 
                    std::random_access_iterator_tag
                >    
            >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
#if !defined (NO_USE_MINOR_SPEED_OPTIMZATION)  // speed vs code size
        // If we have random access input iterators, we can apply some
        // minor optimizations (don't check for first != last):
        typedef converter<FromEncodingT, ToEncodingT, Validation::NO_CHECK_INPUT_RANGE> cvt1_t;
        const size_t treshhold_length = 24; // should be reasonable size which justifies the additional costs for selecting the optimization.
        int result;
        cvt1_t cvt1(state);
        size_t length = std::distance(first, last);
        if (length > treshhold_length) {
            InIteratorT last1 = last - 3;
            while (last1 != first and encoding_traits<FromEncodingT>::is_trail(*last1))
                --last1;
            
            //result = cvt1.convert(first, last1, dest);
            while ((result = cvt1.convert(first, last1, dest)) != NO_ERROR 
                   and result != E_UNEXPECTED_ENDOFINPUT 
                   and convertOption != None) 
            {
                if (result == E_INVALID_START_BYTE) {
                    ++first;
                }
                typedef converter<code_point_t, ToEncodingT, Validation::UNSAFE> code_point_cvt_t;
                code_point_t cp = kReplacementCharacter;
                code_point_t* cp_first = &cp;
                code_point_cvt_t().convert(cp_first, cp_first+1, dest); 
                cvt1.clear();
                result = NO_ERROR;
            }
            if (result != NO_ERROR)
                return result;
            assert(not (!cvt1.state()));
            state = cvt1.state();
        }
#endif          
        converter<FromEncodingT, ToEncodingT, Validation::SAFE> cvt2(state);
        //result = cvt2.convert(first, last, dest);
        while ((result = cvt2.convert(first, last, dest)) != NO_ERROR 
               and result != E_UNEXPECTED_ENDOFINPUT 
               and convertOption != None) 
        {
            if (result == E_INVALID_START_BYTE) {
                ++first;
            }
            typedef converter<code_point_t, ToEncodingT, Validation::UNSAFE> code_point_cvt_t;
            code_point_t cp = kReplacementCharacter;
            code_point_t* cp_first = &cp;
            code_point_cvt_t().convert(cp_first, cp_first+1, dest); 
            cvt2.clear();
            result = NO_ERROR;
        }
        
        state = cvt2.state();
        return result;
    }
    
    
    
    template <
        typename InIteratorT, typename FromEncodingT, 
        typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
            OutIteratorT& dest, ToEncodingT toEncoding, 
            ConvertOption convertOption = None,            
            typename boost::enable_if<
                boost::is_same<
                typename std::iterator_traits<InIteratorT>::iterator_category, 
                std::random_access_iterator_tag
                >    
            >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
#if !defined (NO_USE_MINOR_SPEED_OPTIMZATION)  // speed vs code size
        // If we have random access input iterators, we can apply some
        // minor optimizations (don't check for first != last):
        const size_t treshhold_length = 24; // should be reasonable size which justifies the additional costs for selecting the optimization.
        int result;
        converter<FromEncodingT, ToEncodingT, Validation::NO_CHECK_INPUT_RANGE> cvt1;
        size_t length = std::distance(first, last);
        if (length > treshhold_length) {
            InIteratorT last1 = last - 3;
            while (last1 != first and encoding_traits<FromEncodingT>::is_trail(*last1))
                --last1;
            //result = cvt1.convert(first, last1, dest);
            while ((result = cvt1.convert(first, last1, dest)) != NO_ERROR 
                   and convertOption != None) 
            {
                if (result == E_INVALID_START_BYTE) {
                    ++first;
                }
                typedef converter<code_point_t, ToEncodingT, Validation::UNSAFE> code_point_cvt_t;
                code_point_t cp = kReplacementCharacter;
                code_point_t* cp_first = &cp;
                code_point_cvt_t().convert(cp_first, cp_first+1, dest);   
                if (result == E_UNEXPECTED_ENDOFINPUT) {
                    return NO_ERROR;
                }
                cvt1.clear();
                result = NO_ERROR;
            }
            if (result != NO_ERROR)
                return result;
        }
        assert(not(!cvt1.state()));
#endif          
        converter<FromEncodingT, ToEncodingT, Validation::SAFE> cvt2;

        //result = cvt2.convert(first, last, dest);
        while ((result = cvt2.convert(first, last, dest)) != NO_ERROR 
               and convertOption != None) 
        {
            if (result == E_INVALID_START_BYTE) {
                ++first;
            }
            typedef converter<code_point_t, ToEncodingT, Validation::UNSAFE> code_point_cvt_t;
            code_point_t cp = kReplacementCharacter;
            code_point_t* cp_first = &cp;
            code_point_cvt_t().convert(cp_first, cp_first+1, dest);   
            if (result == E_UNEXPECTED_ENDOFINPUT) {
                return NO_ERROR;
            }
            cvt2.clear();
            result = NO_ERROR;
        }
        return result;
    }
    
    
    
    template <
        typename InIteratorT, typename FromEncodingT, 
        typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert_unsafe(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
                   OutIteratorT& dest, ToEncodingT toEncoding, 
                   mb_state<FromEncodingT>& state,
                   typename boost::disable_if<
                    boost::is_same<
                        typename std::iterator_traits<InIteratorT>::iterator_category, 
                        std::random_access_iterator_tag
                    >    
                   >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
        converter<FromEncodingT, ToEncodingT, Validation::NO_VALIDATION> cvt(state);
        int result = cvt.convert(first, last, dest);
        state = cvt.state();
        return result;
    }
    
    
    
    template <
    typename InIteratorT, typename FromEncodingT, 
    typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert_unsafe(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
                   OutIteratorT& dest, ToEncodingT toEncoding, 
                   typename boost::disable_if<
                   boost::is_same<
                    typename std::iterator_traits<InIteratorT>::iterator_category, 
                    std::random_access_iterator_tag
                    >    
                   >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
        converter<FromEncodingT, ToEncodingT, Validation::UNSAFE> cvt;
        int result = cvt.convert(first, last, dest);
        return result;
    }
    
    
    
    template <
        typename InIteratorT, typename FromEncodingT, 
        typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert_unsafe(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
                   OutIteratorT& dest, ToEncodingT toEncoding, 
                   mb_state<FromEncodingT>& state,
                   typename boost::enable_if<
                    boost::is_same<
                        typename std::iterator_traits<InIteratorT>::iterator_category, 
                        std::random_access_iterator_tag
                    >    
                   >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
#if !defined (NO_USE_MINOR_SPEED_OPTIMZATION)  // speed vs code size
        // If we have random access input iterators, we can apply some
        // minor optimizations (don't check for first != last):
        const size_t treshhold_length = 24; // should be reasonable size which justifies the additional costs for selecting the optimization.
        int result;
        converter<FromEncodingT, ToEncodingT, Validation::UNSAFE> cvt1(state);
        size_t length = std::distance(first, last);
        if (length > treshhold_length) {
            InIteratorT last1 = last - 3;
            while (last1 != first and encoding_traits<FromEncodingT>::is_trail(*last1))
                --last1;
            result = cvt1.convert(first, last1, dest);
            if (result < 0)
                return result;
            assert(not (!cvt1.state()));
        }
#endif          
        converter<FromEncodingT, ToEncodingT, Validation::NO_VALIDATION> cvt2(state);
        result = cvt2.convert(first, last, dest);
        state = cvt2.state();
        return result;
    }
    
    
    template <
        typename InIteratorT, typename FromEncodingT, 
        typename OutIteratorT, typename ToEncodingT
    >
    inline int
#if defined (CONVERSION_CONVERT_FORCE_INLINE)
    __attribute__((always_inline))
#endif                                
    convert_unsafe(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding, 
                   OutIteratorT& dest, ToEncodingT toEncoding, 
                   typename boost::enable_if<
                    boost::is_same<
                        typename std::iterator_traits<InIteratorT>::iterator_category, 
                        std::random_access_iterator_tag
                    >    
                   >::type* dummy = 0)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, FromEncodingT>::value) );
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, ToEncodingT>::value) );
        
#if !defined (NO_USE_MINOR_SPEED_OPTIMZATION)  // speed vs code size
        // If we have random access input iterators, we can apply some
        // minor optimizations (don't check for first != last):
        const size_t treshhold_length = 24; // should be reasonable size which justifies the additional costs for selecting the optimization.
        int result;
        converter<FromEncodingT, ToEncodingT, Validation::UNSAFE> cvt1;
        size_t length = std::distance(first, last);
        if (length > treshhold_length) {
            InIteratorT last1 = last - 3;
            while (last1 != first and encoding_traits<FromEncodingT>::is_trail(*last1))
                --last1;
            result = cvt1.convert(first, last1, dest);
            if (result < 0)
                return result;
        }
        assert(not(!cvt1.state()));
#endif          
        converter<FromEncodingT, ToEncodingT, Validation::NO_VALIDATION> cvt2;
        return cvt2.convert(first, last, dest);
    }
    
    
}}




#endif  // JSON_UNICODE_CONVERSION_HPP

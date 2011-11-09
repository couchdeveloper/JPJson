//
//  Boolean.hpp
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

#ifndef JSON_BOOLEAN_HPP
#define JSON_BOOLEAN_HPP


#include <boost/config.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/type_traits.hpp>
#include <iostream>
#include "json_traits.hpp"


namespace json {
    
#pragma mark -
#pragma mark json::Boolean
    
    namespace mpl = boost::mpl;
    
    //
    // Typesafe Boolean
    //
    class Boolean 
    {
    private:
        bool value_;
        
        struct bool_value {
            void nonnull() {} 
        };
        typedef void (bool_value::*safe_bool)();
        
    public:
        
        //
        // Constructors
        //
        
        Boolean() throw() : value_(false) {}
        
        Boolean(Boolean const& v)  throw() : value_(v.value_)  {}
        
        // Implicit conversions (T => Boolean) are allowed only from type bool
        template <typename T>
        Boolean(const T& v,
                typename boost::enable_if< boost::is_same<bool, T> >::type* = 0 )
        throw() 
        : value_(v) 
        {}
        
        // Explicit conversions:
        template <typename T>
        /*
        explicit Boolean(const T& v,
                         typename boost::enable_if_c< 
                            not boost::is_same<bool, T>::value
                            and (boost::is_convertible<T,bool>::value 
                                 and not boost::is_convertible<T,int>::value 
                                 and not boost::is_convertible<T, void*>::value)
                         >::type* = 0 
                         ) 
        */
        explicit Boolean(const T& v,
                         typename boost::enable_if< 
                            mpl::and_<
                                mpl::not_<boost::is_same<bool, T> >
                              , mpl::and_<
                                    boost::is_convertible<T,bool>
                                  , mpl::not_<boost::is_convertible<T,int> > 
                                  , mpl::not_<boost::is_convertible<T, void*> >
                                >
                            >
                         >::type* = 0 
                         ) 
        : value_(v)
        {}
        
        //
        // Safe bool operator
        //
        operator safe_bool() const
        {
            return value_ == true? &bool_value::nonnull : 0;
        }
        
        
        //
        // Comparison Operators
        //
        
        Boolean operator! () const { return Boolean(not value_); }
        
        // Comparison Operators (defined inline as friend free functions)
        
        // bool operator== (ConvertibleToBoolean lhv, ConvertibleToBoolean rhv)
        template <typename T, typename U>
        friend inline 
        typename boost::enable_if<
            mpl::and_<
                  boost::is_convertible<T, Boolean>
                , boost::is_convertible<U, Boolean> 
            >
            , bool>::type
        operator== (const T& lhv, const U& rhv) {
            return bool(lhv) == bool(rhv);
        }
        
        // bool operator!= (ConvertibleToBoolean lhv, ConvertibleToBoolean rhv)
        template <typename T, typename U>
        friend inline 
        typename boost::enable_if<
            mpl::and_<
                boost::is_convertible<T, Boolean>
              , boost::is_convertible<U, Boolean>
            >
            , bool>::type
        operator!= (const T& lhv, const U& rhv) {
            return bool(lhv) != bool(rhv);
        }
        
    };
    
    inline std::ostream& operator<<(std::ostream& os, const Boolean& b) {
        os << bool(b);
        return os;
    }
    
    template <> 
    struct is_json_type<Boolean> : public boost::mpl::true_
    { 
        static const bool value = true; 
    };
    
}  // namespace json


#endif // JSON_BOOLEAN_HPP

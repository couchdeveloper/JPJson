//
//  value.hpp
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

#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP


#include "json/config.hpp"
#include <boost/config.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp> 
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/swap.hpp>
#include <boost/variant.hpp>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include <ostream>
#include <limits>
#include "json_traits.hpp"
#include "Null.hpp"
#include "Boolean.hpp"
#include "number2.hpp"
#include "array.hpp"
#include "object.hpp"
#include "string.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_traits.hpp"

#if defined (BOOST_VARIANT_NO_FULL_RECURSIVE_VARIANT_SUPPORT)
#warning BOOST_VARIANT_NO_FULL_RECURSIVE_VARIANT_SUPPORT
#endif


#if !defined (BOOST_NO_RVALUE_REFERENCES)
#include <algorithm>
#endif


#if defined (BOOST_NO_MOVE_SWAP_BY_OVERLOAD)
    // This macro is set in the boost::variant library if it detects an apparen-
    // -tly non-conforming compiler which has a defect in ADL. This detection 
    // however, can be a false positive - which is unfortunately true for LLVM 
    // and effectively for all gccs. I consider this a bug in the boost variant 
    // library. When this macro is defined, the function swap will not find the 
    // type specific overload. This however is crucial in this implementation 
    // when using variants. Otherwise variants would perform very, very badly. 
    // In order to alleviate the problem, you may try to define the boost macro 
    // BOOST_STRICT_CONFIG. This forces to disable all boost workarounds and 
    // will finally apply ADL, even when compiled with LLVM. LLVM has no issues
    // with ADL as well as GCC, however defining BOOST_STRICT_CONFIG is a 
    // global switch to disable all workarounds and assumes a conforing compiler.
    // This may have undesired effects if this is not the case for the compiler.
    #warning BOOST_NO_MOVE_SWAP_BY_OVERLOAD - this may degrade performance significantly!
#endif


// Disable overloaded global relational operators - they do not work well enough
// due to issues with ambiguities.
#define JSON_NO_OVERLOADED_RELATIONAL_OPS

namespace json {
    
    namespace mpl = boost::mpl;
    
#pragma mark -
#pragma mark json::value
    
    
    //
    //  struct value_policies
    // 
    // value policies define the underlaying implementations of the container
    // used in the json container. This one is the default policy.
    //
    struct value_policies 
    {
        typedef std::vector<boost::mpl::_>              array_imp_tt;
        typedef std::map<boost::mpl::_, boost::mpl::_>  object_imp_tt;
        typedef json::string<boost::mpl::_>             string_imp_tt;
        //typedef std::basic_string<boost::mpl::_, boost::mpl::_, boost::mpl::_>        string_imp_tt;
        
        static const bool key_not_exists_throws = false;
    };
    
    //
    //  class value
    //
    template <
        typename PoliciesT = value_policies, 
        typename StringEncondingT = unicode::UTF_8_encoding_tag
    >
    class value
    {
    private:
        typedef typename unicode::encoding_traits<StringEncondingT>::code_unit_type   string_char_t;
        typedef typename PoliciesT::array_imp_tt            array_imp_tt;
        typedef typename PoliciesT::object_imp_tt           object_imp_tt;  
        typedef typename PoliciesT::string_imp_tt           string_imp_tt;  
        
    public:
        typedef PoliciesT                                   policies_t;
        typedef StringEncondingT                            string_encoding_t;
        
        typedef typename mpl::apply<string_imp_tt, StringEncondingT>::type   string_t;
        typedef array<value, array_imp_tt>                  array_t;
        typedef object<string_t, value, object_imp_tt>      object_t;  

        typedef boost::variant<
              Null 
            , Boolean 
            , Number
            , string_t
            , boost::recursive_wrapper<array_t> 
            , boost::recursive_wrapper<object_t>
        > variant_t;        
        
        
    public:
        
        //
        // Constructors
        //
        
        value() : value_(null) {}
        
        template <typename T>
        value(const T& v,
                       typename boost::enable_if<
                            mpl::or_<
                                boost::is_convertible<T, variant_t>
                              , is_json_type<T>
                            >
                       >::type* dummy = 0
                       )             
        : value_(v) {}
        
        
        value(value const & other) throw() : value_(other.value_)
        {
        }
        
        value(const string_char_t* s) : value_(string_t(s)) {}
        
        
        value& operator=(value const& other) throw() {
            if (this != &other) {
                value_ = other.value_;
            }
            return *this;
        }        
        
        
        
#if !defined (BOOST_NO_RVALUE_REFERENCES)
        // Move constructor
        value(value&& other) throw() : value_(std::move(other.value_)) { }
        
        // Move assignment operator
        value& operator=(value&& other) throw() {
            if (this != &other) {
                value_ = std::move(other.value_);
            }
            return *this;
        }        
#else        
        
        // Move constructor
        value(const internal::move_t<value>& other) throw() 
        : value_(other.source.value_) 
        { }
        
        // Move assignment operator
        value& operator=(internal::move_t<value>& other) throw() {
            if (this != &other.source) {
                value_ = other.source.value_;
            }
            return *this;
        }        
#endif    
        
        //
        // Query
        //
        
        template <typename T>
        bool is_type() {
            bool result = boost::get<T>(&value_) != 0;
            return result;
        }
        
        std::string     type_name() const;
        size_t          type_size() const;
        std::ostream&   print(std::ostream& os, bool pretty = false, int indent = 0) const;
        
        bool is_equal(const value& other) const {
            return value_ == other.value_;
        }
        
        
        //
        //  Retrieving concrete type
        //
        
        template <typename JsonT>
        typename boost::enable_if <
            is_json_type<JsonT>,
            const JsonT&>::type    
        as() const { return boost::get<JsonT>(value_); }
        
        template <typename JsonT>
        typename boost::enable_if <
            is_json_type<JsonT>,
            JsonT&>::type    
        as() { return boost::get<JsonT>(value_); }
        
        // Direct cast to a numeric type:
        template <typename NumericT>
        typename boost::enable_if <
            is_numeric<NumericT>,
            NumericT>::type    
        as() const { return boost::get<Number>(value_).as<NumericT>(); }
        
        
        //
        //  Support
        //
        
        value make_default() const;
        
        void swap(value& other) {
            if (value_.which() != other.value_.which()) {
                value tmp = make_default();
                boost::swap(tmp.value_, value_);//  tmp.swap(lhv);
                value_ = other.make_default().value_;
                boost::swap(value_, other.value_); // lhv.swap(rhv);
                other.value_ = tmp.make_default().value_;
                boost::swap(other.value_, tmp.value_); //rhv.swap(tmp);
            }
            else {
                boost::swap(value_, other.value_);
            }
        }
        
        
        
    protected:    
        variant_t value_; 
        
        //    
        // Relational Operators  -  defined as inline friends:    
        //  
        
        // Note: the following two relational operators will be invoked
        // for any argument that is convertible to value, unless there is
        // a better match for another operator defined elsewhere. This of
        // course may involve to construct a temporary - which may have a
        // significant performance penalty, if ctors are expensive - which
        // is possibly the case for these kind of objects.
        friend inline 
        bool        
        operator== (const value& lhv, const value& rhv) {
            return lhv.value_ == rhv.value_;
        }
        friend inline 
        bool        
        operator!= (const value& lhv, const value& rhv) {
            return not (lhv.value_ == rhv.value_);
        }
        
        
        
#if !defined (JSON_NO_OVERLOADED_RELATIONAL_OPS)
        //
        // NOT YET IMPLEMENTED
        //
        // The remaining template functions are merely for oportunities
        // to further optimizations.
        // 
        // Problems can arise due to ambiguity when the compiler has to selected 
        // from several template candidates when resolving a statement involving 
        // relational operators. To aleviate the problem, only a selected set of 
        // template functions has been enabled.
        // 
        template <typename U>
        friend inline 
        typename boost::enable_if<
//            mpl::and_<
//                mpl::not_< boost::is_pointer<U> >
//              , boost::is_convertible<U, value>
//            >
                is_json_type<U>
            , bool>::type 
        operator== (const value& lhv, const U& rhv) {
#if (0)     
            // Note: disabled because the default ctor trick does not
            // work with char[] types.
            if (lhv.value_.which() == value(U()).value_.which()) {
                return lhv.value_ == value(rhv).value_;
            }
            return false;
#else
            return lhv.is_equal(rhv);
#endif            
        }
        
        template <typename U>
        friend inline 
        typename boost::enable_if<
//            mpl::and_<
//                mpl::not_< boost::is_pointer<U> >
//              , boost::is_convertible<U, value>
//            >
                is_json_type<U>
        , bool>::type 
        operator== (const U& lhv, const value& rhv) {
#if 0
            if (rhv.value_.which() == value(U()).value_.which()) {
                return rhv.value_ == value(lhv).value_;
            }
            return false;
#else
            return rhv.is_equal(lhv);
#endif
        }
        
        template <typename U>
        friend inline 
        typename boost::enable_if< is_json_type<U>, bool >::type 
        operator!= (const value& lhv, const U& rhv) {
            return not (lhv == rhv);
        }
        
        template <typename U>
        friend inline 
        typename boost::enable_if< is_json_type<U>, bool >::type 
        operator!= (const U& lhv, const value& rhv) {
            return not (lhv == rhv);
        }
#endif       
        
        // Disambiguate with comparisons involving Null:
        friend inline
        bool operator== (const value& lhv, const Null& rhv) {
            return lhv.is_equal(rhv);
        }
        friend inline
        bool operator== (const Null& lhv, const value& rhv) {
            return rhv.is_equal(lhv);
        }
        friend inline
        bool operator!= (const value& lhv, const Null& rhv) {
            return not lhv.is_equal(rhv);
        }
        friend inline
        bool operator!= (const Null& lhv, const value& rhv) {
            return not rhv.is_equal(lhv);
        }
        
        // swap
        friend inline
        void 
        swap(value& lhv, value& rhv) {
            lhv.swap(rhv);
        }
        
    };  // class value
    
    template <typename P, typename E> 
    struct is_json_type<value<P, E> > : public boost::mpl::true_
    {   
        static const bool value = true; 
    };
    
    
    template <typename P, typename E>
    inline std::ostream& operator<< (std::ostream& os, value<P, E> const& v) {
        v.print(os, false);  // print condensed by default
        return os;
    }
    
    
    
}  // namespace json

namespace json { namespace internal {
#pragma mark -
#pragma mark Namespace json::internal
        
        


    // Create a value which concrete value is default constructed from
    // the given parameter
    template <typename P, typename E>
    struct make_default 
    : public boost::static_visitor<value<P, E> > 
    {
        template <typename T>
        value<P, E> operator() (const T& v) const {
            return value<P>(T());
        }
    };   

    
    template <typename P, typename E>
    struct type_name 
    : public boost::static_visitor<std::string> 
    {
        typedef typename value<P, E>::array_t array_t;
        typedef typename value<P, E>::object_t object_t;
        typedef typename value<P, E>::string_t string_t;
        
        template <typename T>
        std::string operator() (const T& v) const {
            std::string result = typeid(v).name();
            return result;
        }
        
        std::string operator() (const Null&) const {
            return "Null";
        }
        std::string operator() (const Boolean&) const {
            return "Boolean";
        }
        std::string operator() (const Number&) const {
            return "Number";
        }
        std::string operator() (const string_t&) const {
            return "String";
        }
        std::string operator() (const array_t&) const {
            return "Array";
        }
        std::string operator() (const object_t&) const {
            return "Object";
        }
    };
    
    struct type_size
    : public boost::static_visitor<size_t> 
    {
        template <typename T>
        size_t operator() (const T&) const {
            return sizeof(T);
        }
    };
    
    template <typename P, typename E>        
    struct print
    : public boost::static_visitor<std::ostream&> 
    {
        typedef typename value<P, E>::array_t array_t;
        typedef typename value<P, E>::object_t object_t;
        typedef typename value<P, E>::string_t string_t;
        
        static const int tab_size_ = 2;
        std::ostream& os_;
        bool pretty_;
        int indent_;
        size_t count_;
        
        
        print(std::ostream& os, bool pretty = false, int indent = 0) 
        : os_(os), pretty_(pretty), indent_(indent), count_(0) {
        }
        
        void pretty_indent(int inc = 0, bool nl = false) {
            if (pretty_) {
                if (nl and count_ > 0)
                    os_.put('\n');
                int count = (indent_ + inc)*tab_size_;
                while (count--) {
                    os_.put(' ');
                }
            }
        }
        
        void pretty_space() {
            if (pretty_)
                os_.put(' ');
        }
        
        
        template <typename T>
        std::ostream& operator() (const T& v) {
            os_ << v;
            return os_;
        }
        
        std::ostream& operator() (const Null&) {
            os_ << "null";
            return os_;
        }
        
        std::ostream& operator() (const Boolean& v) {
            os_ << (v ? "true" : "false");
            return os_;
        }
        
        std::ostream& operator() (const Number& v) {
            os_ << v.string();
            return os_;
        }
        
        std::ostream& operator() (const string_t& v) {
            os_ << '"' << v << '"';
            return os_;
        }
        
        std::ostream& operator() (const object_t& v) {
            typedef typename object_t::imp_t imp_t;
            typedef typename imp_t::const_iterator iterator_t;
            iterator_t first = v.imp().begin();
            iterator_t last = v.imp().end();
            //pretty_indent(0, true);
            os_ << '{';
            ++count_;
            while (first != last) {
                pretty_indent(1, true);
                os_ << '"' << (*first).first << '"';
                pretty_space();
                os_ <<  ':';
                pretty_space();
                (*first).second.print(os_, pretty_, indent_ + 1);
                ++first;
                if (first != last) {
                    os_ << ',';
                }
            }
            if (not v.imp().empty())
                pretty_indent(0, true);
            os_ << '}' ; 
            return os_;
        }
        
        std::ostream& operator() (array_t const& a) {
            typedef typename array_t::imp_t imp_t;
            typedef typename imp_t::const_iterator iterator_t;
            iterator_t first = a.imp().begin();
            iterator_t last = a.imp().end();
            //pretty_indent(0, true);
            os_ << '[';
            ++count_;                
            while (first != last) {
                pretty_indent(1, true);
                (*first).print(os_, pretty_, indent_ + 1);
                ++first;
                if (first != last) {
                    os_ << ',';
                }
            }
            if (not a.imp().empty())
                pretty_indent(0, true);
            os_ << ']'; 
            return os_;
        }
        
}; //  print visitor
    
    
}} // namespace json::internal




namespace json {

    //
    // Value implementation
    //
    template <typename P, typename E>
    inline value<P, E> 
    value<P, E>::make_default() const
    {
        typedef internal::make_default<P, E> make_default; 
        return boost::apply_visitor(make_default(), value_);
    }
    
    template <typename P, typename E>
    inline std::string 
    value<P, E>::type_name() const  {
        typedef internal::type_name<P, E> type_name; 
        return boost::apply_visitor(type_name(), value_);
    }
    
    template <typename P, typename E>
    inline size_t 
    value<P, E>::type_size() const  {
        return boost::apply_visitor(internal::type_size(), value_);
    }
    
    template <typename P, typename E>
    inline std::ostream& 
    value<P, E>::print(std::ostream& os, bool pretty, int indent) const  {
        typedef internal::print<P, E> print;
        print printer(os, pretty, indent);
        return boost::apply_visitor(printer, value_);
    }

    
}

namespace json {
    
    //
    //  Free Functions
    //
    

}


#endif // JSON_VALUE_HPP

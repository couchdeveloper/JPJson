//
//  array.hpp
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

#ifndef JSON_ARRAY_HPP
#define JSON_ARRAY_HPP


#include "json/config.hpp"
#include <boost/mpl/apply.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/swap.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <type_traits>
#include "json_traits.hpp"


namespace json {
    
#pragma mark -
#pragma mark json::array<T, ArrayT>
    
    namespace mpl = boost::mpl;
    
    // forward declaration for template class array
    template <typename T, typename ArrayT = std::vector<mpl::_> > class array; 
    
    // forward declaration for global stram operator
    template <typename T, typename ArrayT> std::ostream& operator<< (std::ostream&, array<T, ArrayT> const&);
    
    //
    //  array
    //
    template <typename T, typename ArrayT/*= std::vector<mpl::_>*/>
    class array {
    public:
        //
        // Types
        //
        typedef typename mpl::apply<ArrayT, T>::type imp_t;
        typedef T value_t;
        typedef typename imp_t::size_type        size_type;
        typedef typename imp_t::iterator         iterator;   
        typedef typename imp_t::const_iterator   const_iterator;   
        typedef typename imp_t::reference        reference;   
        typedef typename imp_t::const_reference  const_reference;   
        
        //
        // Constructors / Destrucor
        //
        
        // default c-tor
        array() noexcept(std::is_nothrow_default_constructible<imp_t>::value)
        {}
        
        // lvalue copy c-tor
        array(array const& other) noexcept(std::is_nothrow_copy_constructible<imp_t>::value)
        : imp_(other.imp_)
        {
        }
        
        // rvalue move c-tor
        array(array&& other) noexcept(std::is_nothrow_move_constructible<imp_t>::value)
        : imp_(std::move(other.imp_))
        {}
        
        // forwarding c-tor
        template <
            typename... Args,
            typename = typename std::enable_if<
                std::is_constructible<imp_t, Args...>::value
            >::type
        >
        explicit array(Args&&... args) noexcept(std::is_nothrow_constructible<imp_t, Args...>::value)
        : imp_(std::forward<Args>(args)...)
        {
        }
        
        
        
        
        //
        // Assignment
        //
        
        // lvalue copy assignment
        array& operator=(array other) {
            imp_ = other.imp_;
            return *this;
        }
        
        // rvalue move assignment
        array& operator=(array&& other) {
            if (this != &other) {
                imp_ = std::move(other.imp_);
            }
            return *this;
        }        
        
        
        //
        // Capacity
        //
        size_t  size() const                { return imp_.size(); }
        void    reserve(size_t n)           { imp_.reserve(n); }
        void    capacity() const            { return imp_.capacity(); }
        
        
        //
        // Element Access
        //
        value_t& operator[](size_type i)    { return imp_[i]; }
        const value_t& operator[](size_type i) const { return imp_[i]; }
        const_reference at(size_t n) const  { return imp_.at(n); }
        reference at(size_t n)              { return imp_.at(n); }
        reference front()                   { return imp_.front(); }
        const_reference front() const       { return imp_.front(); }
        reference back()                    { return imp_.back(); }
        const_reference back() const        { return imp_.back(); }
        
        
        //
        // Iterators
        //
        iterator begin()                    { return imp_.begin(); }
        const_iterator begin() const        { return imp_.begin(); }
        iterator end()                      { return imp_.end(); }
        const_iterator end() const          { return imp_.end(); }
        
        //
        // Modifiers
        //
        void assign (size_t n, const T& v)  { imp_.assign(n, v); }
        template <class InputIterator>
        void assign ( InputIterator first, InputIterator last ) {   
            imp_.assign(first, last);
        }
        void push_back(T const& v)          { imp_.push_back(v); }
        
        void push_back_move(T& v) { 
            imp_.push_back(T());
            T& x = imp_.back();
            boost::swap(v, x); 
        }
        
        
        void pop_back()                     { imp_.pop_back(); }
        
        iterator insert(iterator pos, const T& v) { return imp_.insert(pos, v); }
        void insert(iterator pos, size_t n, const T& v) { imp_.insert(pos, n, v); }
        template <class InputIterator>
        void insert(iterator position, InputIterator first, InputIterator last) {
            imp_.insert(position, first, last);
        }
        
        iterator erase(iterator position)   { return imp_.erase(position); }
        iterator erase(iterator first, iterator last) { return imp_.erase(first, last); }
        
        void clear()                        { imp_.clear(); }
        
        
        //
        //  Support
        //
        imp_t& imp()                        { return imp_; }
        const imp_t& imp() const            { return imp_; }
        
        
        void swap(array& other) {
            boost::swap(imp_, other.imp_);
        }
        
        
    protected:            
        imp_t imp_;
        
        
        //
        // global functions, friend inline
        //
        
        friend std::ostream& operator<< <> (std::ostream& os, array<T, ArrayT> const& );  
        
        //
        //  Comparisons
        // 
        friend inline 
        bool        
        operator== (const array<T, ArrayT>& lhv, const array<T, ArrayT>& rhv) {
            return lhv.imp_ == rhv.imp_;
        }
        friend inline 
        bool        
        operator!= (const array<T, ArrayT>& lhv, const array<T, ArrayT>& rhv) {
            return lhv.imp_ != rhv.imp_;
        }
        friend inline 
        bool        
        operator< (const array<T, ArrayT>& lhv, const array<T, ArrayT>& rhv) {
            return lhv.imp_ < rhv.imp_;
        }
        friend inline 
        bool        
        operator> (const array<T, ArrayT>& lhv, const array<T, ArrayT>& rhv) {
            return lhv.imp_ > rhv.imp_;
        }
        friend inline 
        bool        
        operator<= (const array<T, ArrayT>& lhv, const array<T, ArrayT>& rhv) {
            return lhv.imp_ <= rhv.imp_;
        }
        friend inline 
        bool        
        operator>= (const array<T, ArrayT>& lhv, const array<T, ArrayT>& rhv) {
            return lhv.imp_ >= rhv.imp_;
        }
        
    };
    
    
    
    template <typename T, typename ArrayT>
    inline std::ostream& operator<< (std::ostream& os, array<T, ArrayT> const& a) {
        typedef typename mpl::apply<ArrayT, T>::type imp_t;
        typedef typename imp_t::const_iterator iterator_t;
        iterator_t first = a.imp_.begin();
        iterator_t last = a.imp_.end();
        os << '['; 
        while (first != last) {
            os << *first++;
            if (first != last) {
                os << ',';
            }
        }
        os << ']'; 
        return os;
    }
    
    template <typename T, typename ArrayT>    
    inline void swap(array<T, ArrayT>& lhv, array<T, ArrayT>& rhv) {
        lhv.swap(rhv);
    } 
    
    
    template <typename T, typename ArrayT> 
    struct is_json_type<array<T, ArrayT> > : public boost::mpl::true_
    { 
        static const bool value = true; 
    };
    
        
}  // namespace json





#endif // JSON_ARRAY_HPP

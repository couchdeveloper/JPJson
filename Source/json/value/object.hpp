//
//  object.hpp
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

#ifndef JSON_OBJECT_HPP
#define JSON_OBJECT_HPP


#include <boost/config.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/swap.hpp>
#include <iostream>
#include <map>
#if !defined (BOOST_NO_RVALUE_REFERENCES)
#include <algorithm>
#endif
#include "json_traits.hpp"


namespace json {    
#pragma mark -
#pragma mark json::object<KeyT, T, MapT>
    
    namespace mpl = boost::mpl;
    
    // forward declaration for template class object
    template <typename KeyT, typename T, typename MapT = std::map<mpl::_, mpl::_> > class object; 
    
    // forward declaration for global stream operator
    template <typename KeyT, typename T, typename MapT> std::ostream& operator<< (std::ostream&, object<KeyT, T, MapT> const&);
    
    template <typename KeyT, typename T, typename MapT /*= std::map<mpl::_, mpl::_>*/ >
    class object
    {
    public:
        
        //
        // Types
        // 
        typedef typename mpl::apply<MapT, KeyT, T>::type imp_t;
        typedef typename imp_t::key_type key_t;        // shall be String
        typedef typename imp_t::mapped_type value_t;   // shall be Value
        typedef typename imp_t::iterator iterator;
        typedef typename imp_t::const_iterator const_iterator;
        typedef typename imp_t::value_type element;
        
        //
        // Constructors
        //        
        object() {}
        object(const object& other) : imp_(other.imp_) {}
#if !defined (BOOST_NO_RVALUE_REFERENCES)
        // Move construtor
        object(object&& other) : imp_(std::move(other.imp_)) { }
#endif    
        
        //
        // Assignment
        //
        object& operator=(object const& other) {
            imp_ = other.imp_;
            return *this;
        }
#if !defined (BOOST_NO_RVALUE_REFERENCES)
        // Move assignment operator
        object& operator=(object&& other) {
            if (this != &other) {
                imp_ = std::move(other.imp_);
            }
            return *this;
        }        
#endif    
        
        //
        // Query accessors
        //        
        size_t size() const { return imp_.size(); }
        
        bool has_key(const key_t& key) const { return imp_.count(key) == 1; }
        
        iterator find(const key_t& key) { return imp_.find(key); }
        const_iterator find(const key_t& key) const { return imp_.find(key); }
        
        
        // Compare operator
        bool operator==(object const& other) const { return imp_ == other.imp_; }
        
        //
        // Iterators
        //
        const_iterator begin() const    { return imp_.begin(); }
        iterator begin()                { return imp_.begin(); }
        const_iterator end() const      { return imp_.end(); }
        iterator end()                  { return imp_.end(); }
        
        //
        // Element Insertion
        //
        std::pair<iterator, bool> 
        insert(const key_t& key, const value_t& value) { return imp_.insert(element(key, value)); }
        
        std::pair<iterator, bool> 
        insert(const element& e) { return imp_.insert(e); }
        
        iterator insert(iterator position, const element& e) { return imp_.insert(position, e); }
        
        template <class InputIterator>
        void insert(InputIterator first, InputIterator last) { imp_.insert(first, last); }
        
        //
        // Element deletion
        //
        void erase(iterator position)   { imp_.erase(position); }
        bool erase(const key_t& key)    { return imp_.erase(key) == 1; }
        
        //
        // Subscription Access 
        //
        //  If key matches the key of an element in the container, the function returns 
        //  a reference to its mapped value.
        //  
        //  Caution:
        //  If key does not match the key of any element in the container, the function 
        //  inserts a new element with that key and returns a reference to its mapped 
        //  value. Notice that this always increases the container size by one, even if no 
        //  mapped value is assigned to the element (the element is constructed 
        //  using its default constructor).        
        value_t& operator[] (const key_t& key) {
            return (*this).imp_[key];
        }
        
        //
        // Support functions
        //
        void swap(object& other) throw() {
            boost::swap(imp_, other.imp_);
        }
        
        //
        // Modifiers
        //
        void clear()  { imp_.clear(); }
        
        
        imp_t& imp()                        { return imp_; }
        const imp_t& imp() const            { return imp_; }
        
    protected:
        imp_t imp_;
        
        // friends:
        friend std::ostream& operator<< <> (std::ostream& os, object<KeyT, T, MapT> const& );
    };
    
    template <typename KeyT, typename T, typename MapT>
    inline std::ostream& operator<<(std::ostream& os, const object<KeyT, T, MapT>& v) {
        typedef typename mpl::apply<KeyT, MapT, T>::type imp_t;
        typedef typename imp_t::const_iterator iterator_t;
        iterator_t first = v.imp_.begin();
        iterator_t last = v.imp_.end();
        os << '{'; 
        while (first != last) {
            os <<(*first).first << ':' << (*first).second;
            ++first;
            if (first != last) {
                os << ',';
            }
        }
        os << '}' ; 
        return os;
    }
    
    template <typename KeyT, typename T, typename MapT>    
    inline void swap(object<KeyT, T, MapT>& lhv, object<KeyT, T, MapT>& rhv) {
        lhv.swap(rhv);
    }
    
    template <typename KeyT, typename T, typename MapT> 
    struct is_json_type< object<KeyT, T, MapT> >  : public boost::mpl::true_
    {   
        static const bool value = true; 
    };
    
    
}  // namespace json



#endif // JSON_OBJECT_HPP

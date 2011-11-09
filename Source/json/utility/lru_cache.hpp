#ifndef JSON_UTILITY_LRU_CACHE_HPP
#define JSON_UTILITY_LRU_CACHE_HPP
//
//  lru_cache.hpp
//
//  Created by Andreas Grosam on 7/14/11.
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


#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/function.hpp>
#include <assert.h>



namespace json { namespace utility {

    // Class providing fixed-size (by number of records) LRU-replacement cache 
    // of a function with signature V f(K) 
    template <typename K, typename V> 
    class lru_cache 
    { 
    public: 
        
        typedef int dummy_type; 
        
        // Bimap with key access on left view, key access 
        // history on right view, and associated value. 
        typedef boost::bimaps::bimap< 
            boost::bimaps::set_of<K>
          , boost::bimaps::list_of<dummy_type>
          , boost::bimaps::with_info<V> 
        > cache_type; 
        
        // Constuctor specifies the cached function and 
        // the maximum number of records to be stored. 
        lru_cache(const boost::function<V(const K&)>& f, 
                  size_t c) 
        :   fn_(f) 
          , capacity_(c) 
        { 
            assert(capacity_ != 0); 
        } 
        
        // Obtain value of the cached function for k 
        V operator()(const K& k) { 
            
            // Attempt to find existing record 
            const typename cache_type::left_iterator it = cache_.left.find(k); 
            
            if (it == cache_.left.end()) { 
                
                // We don't have it: 
                // Evaluate function and create new record 
                const V v = fn_(k); 
                insert(k,v); 
                return v; 
                
            } else { 
                
                // We do have it: 
                // Update the access record view. 
                cache_.right.relocate( 
                                      cache_.right.end(), 
                                      cache_.project_right(it) 
                                      ); 
                
                return it->info; 
            } 
        } 
        
        // Obtain the cached keys, most recently used element 
        // at head, least recently used at tail. 
        // This method is provided purely to support testing. 
        template <typename IT> 
        void 
        get_keys(IT dst) const 
        { 
            typename cache_type::right_const_reverse_iterator src = cache_.right.rbegin(); 
            while (src != cache_.right.rend()) { 
                *dst++ = (*src++).second; 
            } 
        } 
        
    private: 
        
        void insert(const K& k,const V& v) { 
            
            assert(cache_.size()<=capacity_); 
            
            // If necessary, make space 
            if (cache_.size() == capacity_) { 
                // by purging the least-recently-used element 
                cache_.right.erase(cache_.right.begin()); 
            } 
            
            // Create a new record from the key, a dummy and the value 
            cache_.insert( 
                          typename cache_type::value_type( 
                                                          k,0,v 
                                                          ) 
                          ); 
        } 
        
        const boost::function<V(const K&)> fn_; 
        const size_t capacity_; 
        cache_type cache_; 
    };

}}


#endif

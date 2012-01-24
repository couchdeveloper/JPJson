//
//  CFDataCache.hpp
//
//  Created by Andreas Grosam on 7/15/11.
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

#ifndef JSON_OBJC_CFDATA_CACHE_HPP
#define JSON_OBJC_CFDATA_CACHE_HPP


#include "json/config.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <functional>
#include <assert.h>
#include <cstring>
#include <cstdlib>
#include <boost/tr1/unordered_map.hpp>
#include <boost/utility.hpp>
#include "json/utility/string_hasher.hpp"



//      TODO: 
//      Status: development.
//      May be used later to implement a cache with a LRU eviction strategy.


namespace json { namespace objc { 
        
    
    /*
     Special purpose associative container whose mapped type is a Core Foundation 
     object (CFTypeRef) and whose key type is a "string buffer" - a 
     std::pair<const char*, size_t>.
     
     The CF instances are required to be immutable. The key is requiered to be
     unique, that is there is only a one_to_one association allowed.
     
     The semantic actions classes use this cache to minimize creation of 
     CFString instances.
     
     A "string buffer" defines the start of a character sequence and its size,
     but it does not provide the storage for the character sequence itself. It
     must be guaranteed, though, that this character sequence exists for the 
     life-time of the string buffer instance. The character sequence does not 
     need to exist as a separate string object. The string buffer may reference,
     for instance, a substring within another larger sequence. 
     
     When inserting a key-value pair, CFDataCache makes a copy of the string 
     buffer, and hence is required to make a copy of the character sequence, too. 
     The characters are stored internally in an efficient manner.
          
     When inserting a key-value pair, the Core Data object is retained.     
     On destruction, the Core Data objects will be released.

     */
    
    template <typename CharT /*, typename block_allocator*/>
    class CFDataCache : boost::noncopyable
    {
    public:
        typedef CFTypeRef                               mapped_type;
        typedef std::pair<const CharT*, std::size_t>    key_type;
        typedef std::pair<key_type const, mapped_type>  value_type;
        
    private:
        
        struct key_equal_to
        : std::binary_function<key_type, key_type, bool>
        {
            bool operator()(const key_type& lhv, const key_type& rhv ) const
            {
                if (lhv.second == rhv.second)
                    return memcmp(lhv.first, rhv.first, rhv.second) == 0;
                else
                    return false;
            }
        };
        
        struct key_hash
        : std::unary_function<key_type, std::size_t>
        {
            std::size_t operator()(key_type const& x) const
            {
                return json::utility::string_hasher<CharT>()(x.first, x.second);
            }
        };
        
        
        struct block_allocator {
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;
            
            block_allocator(std::size_t capacity = 0) 
            {
            }
            
            void* malloc(const size_type bytes)
            { 
                char * p = new (std::nothrow) char[bytes];
#if defined (DEBUG)                
                //printf("String cache: system allocator: allocated block %p with size %lu\n", p, bytes);
#endif                
                return p; 
            }
            
            void free(void * const block)
            { 
                delete [] static_cast<char*>(block); 
#if defined (DEBUG)                
                //printf("String Cache: system allocator: freed block %p\n", block);
#endif                
            }
            
            void clear() {};
            
        };
        
        
        typedef block_allocator                             pool_t;        
        
        typedef mapped_type                                 map_mapped_t;
        typedef std::pair<const key_type, map_mapped_t>     map_value_t;
        typedef std::allocator<map_value_t>                 map_allocator_type;        
        typedef boost::unordered_map<
              key_type
            , map_mapped_t
            , key_hash
            , key_equal_to
            , map_allocator_type 
        >                                       map_t;        
        
    public:
        
        typedef typename map_t::iterator       iterator;        
        typedef typename map_t::const_iterator const_iterator;
        
        explicit CFDataCache(std::size_t capacity = 1024) 
        : pool_(capacity*16)
        {
            map_.rehash(capacity);
        }
        
        ~CFDataCache() { 
            clear();
        }
        
        iterator        begin()         { return map_.begin(); }
        const_iterator  begin() const   { return map_.begin(); }
        iterator        end()           { return map_.end(); }
        const_iterator  end() const     { return map_.end(); }
        
        size_t  size() const { return map_.size(); }
        
        iterator find(key_type const& key) {
            return map_.find(key);
        }
        
        iterator find(key_type const& key) const {
            return map_.find(key);
        }
                
        std::pair<iterator, bool> 
        insert(const key_type& key, mapped_type v)
        {     
            // TODO: use a faster block allocator
            CharT* p = (CharT*)pool_.malloc(key.second*sizeof(CharT));
            std::copy(key.first, key.first + key.second, p); 
            //std::memcpy(p, key.first, key.second*sizeof(CharT));
            key_type map_key;
            map_key.first = p;
            map_key.second = key.second;
            
            std::pair<iterator, bool> result = 
                    map_.insert(value_type(map_key, v));
            if (result.second) {
                if (v)
                    CFRetain(v);
                return result;
            } 
            else
            {
                return result;
            }
        }
        
        bool erase(const key_type& key);
                
        void clear();
                
        
    private:
        map_t  map_;
        pool_t pool_;
    };    
    

    
}}    


namespace json { namespace objc {
    
        
    
    // Note: Usually, an erase function is not required for a cache (rather,
    // a resize function, which changes the capacity of the cache).
    template <typename CharT>
    inline bool 
    CFDataCache<CharT>::erase(const key_type& key)
    {
        typename map_t::iterator iter = map_.find(key);
        if (iter != map_.end()) {
            key_type map_key = (*iter).first;
            //pool_.free(const_cast<char*>(map_key.first), map_key.second);
            pool_.free(static_cast<void*>(const_cast<CharT*>(map_key.first)));
            mapped_type v = (*iter).second;
            if (v) {
                CFRelease(v);
            }
            map_.erase(iter);
            return true;
        }
        else {
            return false;
        }
    }

    
    template <typename CharT>
    inline void 
    CFDataCache<CharT>::clear() 
    {
#if defined (DEBUG)        
        //printf("clearing string cache. size: %ld\n", size());
#endif        
        typename map_t::iterator first = map_.begin();
        typename map_t::iterator last = map_.end();
        while (first != last) {
            if ((*first).second) {
                CFRelease( (*first).second);
            }
            key_type map_key = (*first).first;
            pool_.free(static_cast<void*>(const_cast<CharT*>(map_key.first)));
            ++first;
        }
        map_.clear();
        pool_.clear();
    }

    
}}   // namespace json::objc



#endif

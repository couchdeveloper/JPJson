#ifndef JSON_FAST_POOL_ALLOCATOR_HPP
#define JSON_FAST_POOL_ALLOCATOR_HPP
//
//  fast_pool_alloc.hpp
//  json_parser
//
//  Created by Andreas Grosam on 7/13/11.
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

#error Unused

#include <boost/pool/pool_alloc.hpp>


// Work around for broken boost::fast_pool_allocator

namespace json {
    
    template <typename T,
        typename UserAllocator = boost::default_user_allocator_new_delete,
        typename Mutex = boost::details::pool::default_mutex,
        unsigned NextSize = 32,
        unsigned MaxSize = 0>
    class fast_pool_allocator : public boost::fast_pool_allocator<T, UserAllocator, Mutex, NextSize, MaxSize>
    {
    private:
        typedef boost::fast_pool_allocator<T, UserAllocator, Mutex, NextSize, MaxSize> base;        
    public:
        
        typedef typename base::value_type            value_type;
        typedef typename base::user_allocator        user_allocator;
        typedef typename base::mutex                 mutex;
        
        typedef typename base::pointer               pointer;
        typedef typename base::const_pointer         const_pointer;
        typedef typename base::reference             reference;
        typedef typename base::const_reference       const_reference;
        typedef typename base::size_type             size_type;
        typedef typename base::difference_type       difference_type;
        
        
        template <typename U>
        struct rebind
        {
            typedef fast_pool_allocator<U, UserAllocator, Mutex, NextSize, MaxSize> other;
        };
        
        fast_pool_allocator() {}
        
        template <typename U>
        fast_pool_allocator(const fast_pool_allocator<U, UserAllocator, Mutex, NextSize, MaxSize> & other)
        : base(other) 
        {
        }
        
        static void deallocate(const pointer ptr, const size_type n)
        {
#ifdef BOOST_NO_PROPER_STL_DEALLOCATE
            if (ptr == 0 || n == 0)
                return;
#endif
            if (n == 1)
                (boost::singleton_pool<boost::fast_pool_allocator_tag, sizeof(T),
                 UserAllocator>::free)(ptr);
            else
                (boost::singleton_pool<boost::fast_pool_allocator_tag, sizeof(T),
                 UserAllocator>::ordered_free)(ptr, n);
        }
        
    };
    
}



#endif // JSON_FAST_POOL_ALLOCATOR_HPP

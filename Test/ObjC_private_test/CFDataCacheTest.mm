//
//  CFDataCacheTest.cpp
//  Test
//
//  Created by Andreas Grosam on 7/18/11.
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

#include "json/ObjC/CFDataCache.hpp"

#include <gtest/gtest.h>


// for testing
#include "utilities/bench.hpp"
#include "json/utility/arena_allocator.hpp"
#include <dispatch/dispatch.h>

#include <stdexcept>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <scoped_allocator>
#include <cstring>
#include <cstdio>

#include <boost/tr1/unordered_map.hpp>

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h> 


using namespace json;
using json::objc::CFDataCache;



namespace test {

#if defined (DEBUG)
    constexpr int N = 10;
#else
    constexpr int N = 10;
#endif
    


    template <typename Derived>
    struct bench_datacache_base : bench_base<Derived, N>
    {
        typedef bench_base<Derived> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        
        bench_datacache_base(std::string title) : title_(title) {}
        
        void prepare_imp()
        {
            std::cout << "\n--- " << title_ << " ---\n";
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        bool result() const { return result_; }
        bool& result() { return result_; }
        
        void teardown_imp() {};
        
        
    private:
        bool result_;
        std::string title_;
    };
    
    
    template <typename Map>
    struct bench_map_create : bench_datacache_base<bench_map_create<Map>>
    {
        typedef bench_datacache_base<bench_map_create<Map>> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        typedef std::map<std::string, CFStringRef> map_t;
        
        bench_map_create(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "function map::emplace" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            char buffer[245];
            map_t map;  // almost zero duration!
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d",  keys_[i]);
                t0 = timer.now();
                map.emplace(buffer, static_cast<CFStringRef>(nullptr));
                result += timer.now() - t0;
            }
            return result;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
    };


    template < template <typename...> class Map>
    struct bench_map_arena_allocator_create : bench_datacache_base<bench_map_arena_allocator_create<Map>>
    {
        typedef json::utility::arena_allocator<char, json::utility::SysArena> allocator_t;
        typedef typename allocator_t::template rebind<char>::other string_allocator_t;
        typedef std::basic_string<char, std::char_traits<char>, string_allocator_t> key_t;
        typedef std::less<key_t> compare_t;
        typedef typename std::scoped_allocator_adaptor<allocator_t> scoped_allocator_t;        
        typedef Map<key_t, CFTypeRef, compare_t, scoped_allocator_t> map_t;
        
        typedef bench_datacache_base<bench_map_arena_allocator_create<Map>> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        
        bench_map_arena_allocator_create(const std::string& title)
        : base(title),
          arena_(4096),
          allocator_(arena_)
        {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "function map::emplace" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            char buffer[245];
            map_t map(allocator_);  // almost zero duration!
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d",  keys_[i]);
                t0 = timer.now();
                map.emplace(key_t(buffer, 16, allocator_), static_cast<CFStringRef>(nullptr));
                result += timer.now() - t0;
            }
            return result;
        }
        
        void teardown_imp()
        {
            arena_.clear();
        }
        
    private:
        json::utility::SysArena arena_;
        allocator_t allocator_;
        int count_;
        std::vector<int> keys_;
    };
    
    
    struct bench_NSMutableDictionary_create : bench_datacache_base<bench_NSMutableDictionary_create>
    {
        typedef bench_datacache_base<bench_NSMutableDictionary_create> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
                
        bench_NSMutableDictionary_create(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "method setObject:forKey:" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            duration creation_duration = duration::zero();
            
            @autoreleasepool {
                char buffer[245];
                t0 = timer.now();
                NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity:count_];
                creation_duration = timer.now() - t0;
                for (int i = 0; i < count_; ++i) {
                    snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                    t0 = timer.now();
                    @autoreleasepool {
                        NSString* key = [[NSString alloc] initWithUTF8String:buffer];
                        [dict setObject:[NSNull null] forKey:key];
                    }
                    result += timer.now() - t0;
                }
            }
            return result + creation_duration;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
        
    };
    

    struct bench_CFMutableDictionary_create : bench_datacache_base<bench_CFMutableDictionary_create>
    {
        typedef bench_datacache_base<bench_CFMutableDictionary_create> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_CFMutableDictionary_create(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "function CFStringCreateWithBytes" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            duration creation_duration = duration::zero();
            
            char buffer[245];
            const UInt8* bytes = reinterpret_cast<const UInt8*>(&buffer[0]);
            t0 = timer.now();
            CFMutableDictionaryRef cfDict = CFDictionaryCreateMutable(
                    kCFAllocatorDefault,
                    0,
                    &kCFTypeDictionaryKeyCallBacks,
                    &kCFTypeDictionaryValueCallBacks);
            creation_duration = timer.now() - t0;
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                t0 = timer.now();
                CFStringRef cfKey = CFStringCreateWithBytes(
                    kCFAllocatorDefault,
                    bytes, 16, kCFStringEncodingUTF8,
                    false);
                CFDictionaryAddValue(cfDict, cfKey, kCFNull);
                CFRelease(cfKey);
                result += timer.now() - t0;
            }
            CFRelease(cfDict);
            return result + creation_duration;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
        
    };
    
    
    
    struct bench_CFDataCache_create : bench_datacache_base<bench_CFDataCache_create>
    {
        typedef bench_datacache_base<bench_CFDataCache_create> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_CFDataCache_create(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "function insert" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();

            typedef CFDataCache<char> cache_t;
            
            cache_t cache(4000);            
            char buffer[245];
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                t0 = timer.now();
                cache.insert(cache_t::key_type(buffer, 16), kCFNull);
                result += timer.now() - t0;
            }
            return result;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
        
    };
    
    

    template <typename Map>
    struct bench_map_lookup : bench_datacache_base<bench_map_lookup<Map>>
    {
        typedef bench_datacache_base<bench_map_lookup<Map>> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        typedef std::map<std::string, CFStringRef> map_t;
        
        bench_map_lookup(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "function map::find" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
            char buffer[245];
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d",  keys_[i]);
                map_.emplace(buffer, static_cast<CFStringRef>(nullptr));
            }
            
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            char buffer[245];
            map_t map;
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d",  keys_[i]);
                t0 = timer.now();
                map_t::iterator iter =  map_.find(buffer);
                result += timer.now() - t0;
                if (iter == map_.end()) {
                    throw std::logic_error("did not find element in map");
                }
                
            }
            return result;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
        map_t map_;
    };
    
    

    
    struct bench_NSMutableDictionary_lookup : bench_datacache_base<bench_NSMutableDictionary_lookup>
    {
        typedef bench_datacache_base<bench_NSMutableDictionary_lookup> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_NSMutableDictionary_lookup(const std::string& title) : base(title), dict_(nil)  {}
        
        void prepare_imp(int count)
        {
            @autoreleasepool {
                count_ = count;
                base::prepare_imp();
                std::cout << "method objectForKey:" << std::endl;
                for (int i = 0; i < count_; ++i) {
                    keys_.push_back(i);
                }
                std::random_shuffle(keys_.begin(), keys_.end());
                char buffer[245];
                dict_ = [[NSMutableDictionary alloc] initWithCapacity:count_];
                for (int i = 0; i < count_; ++i) {
                    snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                    NSString* key = [[NSString alloc] initWithUTF8String:buffer];
                    [dict_ setObject:[NSNull null] forKey:key];
                    key = nil; // try to disassociate key from the autorelease pool
                }
            }
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            @autoreleasepool {
                char buffer[245];
                for (int i = 0; i < count_; ++i) {
                    snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                    t0 = timer.now();
                    id obj;
                    @autoreleasepool {
                        NSString* key = [[NSString alloc] initWithUTF8String:buffer];
                        obj = [dict_ objectForKey:key];
                    }
                    result += timer.now() - t0;
                    if (obj == nil) {
                        throw std::logic_error("did not find object in dictionary");
                    }
                }
            }
            return result;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
        NSMutableDictionary* dict_;
    };
    

    struct bench_CFMutableDictionary_lookup : bench_datacache_base<bench_CFMutableDictionary_lookup>
    {
        typedef bench_datacache_base<bench_CFMutableDictionary_lookup> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_CFMutableDictionary_lookup(const std::string& title) : base(title), cfDict_(NULL)  {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "function CFDictionaryGetValue" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
            char buffer[245];
            cfDict_ = CFDictionaryCreateMutable(
                          kCFAllocatorDefault,
                          0,
                          &kCFTypeDictionaryKeyCallBacks,
                          &kCFTypeDictionaryValueCallBacks);
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                CFStringRef cfKey = CFStringCreateWithBytes(
                                                            kCFAllocatorDefault,
                                                            (const UInt8*)buffer, 16, kCFStringEncodingUTF8,
                                                            false);
                CFDictionaryAddValue(cfDict_, cfKey, kCFNull);
                CFRelease(cfKey);
            }
        }
        
        void teardown_imp()
        {
            CFRelease(cfDict_);
            cfDict_ = NULL;
        }
        
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            char buffer[245];
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                t0 = timer.now();
                CFStringRef cfKey = CFStringCreateWithBytes(
                        kCFAllocatorDefault,
                        (const UInt8*)buffer, 16, kCFStringEncodingUTF8,
                        false);
               
                CFTypeRef element = CFDictionaryGetValue(cfDict_, cfKey);
                result += timer.now() - t0;
                if (element == NULL) {
                    throw std::logic_error("did not find element in CoreFoundation dictionary");
                }
            }
            return result;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
        CFMutableDictionaryRef cfDict_;
    };
    
    
    struct bench_CFDataCache_lookup : bench_datacache_base<bench_CFDataCache_lookup>
    {
        typedef bench_datacache_base<bench_CFDataCache_lookup> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        typedef CFDataCache<char> cache_t;
        
        bench_CFDataCache_lookup(const std::string& title) : base(title), cache_(4000)  {}
        
        void prepare_imp(int count)
        {
            count_ = count;
            base::prepare_imp();
            std::cout << "function find" << std::endl;
            for (int i = 0; i < count_; ++i) {
                keys_.push_back(i);
            }
            std::random_shuffle(keys_.begin(), keys_.end());
            
            char buffer[245];
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                cache_.insert(cache_t::key_type(buffer, 16), kCFNull);
            }
            
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            
            char buffer[245];
            for (int i = 0; i < count_; ++i) {
                snprintf(buffer, sizeof(buffer), "key-%12.d", keys_[i]);
                t0 = timer.now();
                cache_t::iterator iter = cache_.find(cache_t::key_type(buffer, 16));
                result += timer.now() - t0;
                if (iter == cache_.end()) {
                    throw std::logic_error("could not find element in cache");
                }
            }
            return result;
        }
        
    private:
        int count_;
        std::vector<int> keys_;
        cache_t cache_;
    };
    

    
#pragma mark - String Bench
    
    
    template <typename Derived, typename CharT>
    struct bench_string_base : bench_base<Derived, N>
    {
        typedef bench_base<Derived> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        typedef std::basic_string<CharT>    string_t;
        
        
        bench_string_base(std::string title) : title_(title) {}
        
        void prepare_imp(int count, int size = 16)
        {
            std::cout << "\n--- " << title_ << " ---\n";
            
            char t[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
            for (int n = 0; n < count; ++n) {
                std::random_shuffle(t, t+sizeof(t));
                string_t s;
                s.reserve(size);
                for (int i = 0; i < size; ++i) {
                    s.push_back( static_cast<CharT>(t[i%sizeof(t)]) );
                }
                source_strings_.emplace_back(std::move(s));
            }
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        bool result() const { return result_; }
        bool& result() { return result_; }
        
        void teardown_imp() {
            source_strings_.clear();
        };
        
    protected:
        
        std::vector<string_t> source_strings_;
        std::string title_;
        bool result_;
    };
    
    
    struct bench_NSString_createUTF8 : bench_string_base<bench_NSString_createUTF8, char>
    {
        typedef bench_string_base<bench_NSString_createUTF8, char> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_NSString_createUTF8(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count, int size = 16)
        {
            count_ = count;
            base::prepare_imp(count, size);
            std::cout << "method initWithBytes:length:encoding: (" << count << " times),  with length: "
            << size << ", encoding equals NSUTF8StringEncoding" << std::endl;
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            strings_.reserve(count_);
            
            for (int i = 0; i < count_; ++i) {
                const void* bytes = (const void*)(source_strings_[i].data());
                NSUInteger length = source_strings_[i].size();
                t0 = timer.now();
                NSString* string = [[NSString alloc] initWithBytes:bytes length:length encoding:NSUTF8StringEncoding];
                result += timer.now() - t0;
                strings_.emplace_back(string);
            }
            strings_.clear();
            return result;
        }
        
    private:
        int count_;
        std::vector<NSString*> strings_;
    };
    
    
    struct bench_NSString_createUTF16 : bench_string_base<bench_NSString_createUTF16, char16_t>
    {
        typedef bench_string_base<bench_NSString_createUTF16, char16_t> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_NSString_createUTF16(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count, int size = 16)
        {
            count_ = count;
            base::prepare_imp(count, size);
            std::cout << "method initWithBytes:length:encoding: (" << count << " times),  with number of bytes: " << size*2 << ", encoding equals NSUTF16StringEncoding" << std::endl;
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            strings_.reserve(count_);
            
            for (int i = 0; i < count_; ++i) {
                const void* bytes = (const void*)(source_strings_[i].data());
                NSUInteger length = source_strings_[i].size() * 2;
                t0 = timer.now();
                NSString* string = [[NSString alloc] initWithBytes:bytes length:length encoding:NSUTF16StringEncoding];
                result += timer.now() - t0;
                strings_.emplace_back(string);
            }
            strings_.clear();
            return result;
        }
        
    private:
        int count_;
        std::vector<NSString*> strings_;
    };
    
    
    struct bench_CFString_createUTF8 : bench_string_base<bench_CFString_createUTF8, char>
    {
        typedef bench_string_base<bench_CFString_createUTF8, char> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_CFString_createUTF8(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count, int size = 16)
        {
            count_ = count;
            base::prepare_imp(count, size);
            std::cout << "function CFStringCreateWithBytes (" << count << " times),  with size: " << size << ", encoding equals kCFStringEncodingUTF8" << std::endl;
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            strings_.reserve(count_);
            
            for (int i = 0; i < count_; ++i) {
                const UInt8* bytes = (const UInt8*)(source_strings_[i].data());
                CFIndex numBytes = source_strings_[i].size();
                t0 = timer.now();
                CFStringRef cfString = CFStringCreateWithBytes(
                                                            kCFAllocatorDefault,
                                                            bytes, numBytes, kCFStringEncodingUTF8,
                                                            false);
                result += timer.now() - t0;
                strings_.emplace_back(cfString);
            }
            for (CFStringRef cfString : strings_) {
                CFRelease(cfString);
            }
            strings_.clear();
            return result;
        }
        
    private:
        int count_;
        std::vector<CFStringRef> strings_;
    };
    

    struct bench_CFString_createUTF16 : bench_string_base<bench_CFString_createUTF16, char16_t>
    {
        typedef bench_string_base<bench_CFString_createUTF16, char16_t> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_CFString_createUTF16(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count, int size = 16)
        {
            count_ = count;
            base::prepare_imp(count, size);
            std::cout << "function CFStringCreateWithBytes (" << count << " times),  with number of bytes: "
            << size*2 << ", encoding equals kCFStringEncodingUTF16" << std::endl;
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            strings_.reserve(count_);
            
            for (int i = 0; i < count_; ++i) {
                const UInt8* bytes = (const UInt8*)(source_strings_[i].data());
                CFIndex numBytes = source_strings_[i].size() * 2;
                t0 = timer.now();
                CFStringRef cfString = CFStringCreateWithBytes(
                                                               kCFAllocatorDefault,
                                                               bytes, numBytes, kCFStringEncodingUTF16,
                                                               false);
                result += timer.now() - t0;
                strings_.emplace_back(cfString);
            }
            for (CFStringRef cfString : strings_) {
                CFRelease(cfString);
            }
            strings_.clear();
            return result;
        }
        
    private:
        int count_;
        std::vector<CFStringRef> strings_;
    };
    

    
    
    struct bench_std_string_createUTF8 : bench_string_base<bench_std_string_createUTF8, char>
    {
        typedef bench_string_base<bench_std_string_createUTF8, char> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_std_string_createUTF8(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count, int size = 16)
        {
            count_ = count;
            base::prepare_imp(count, size);
            std::cout << "function basic_string(const CharT*, size_type count) (" << count << " times),  with size: "
            << size << ", encoding equals UTF-8" << std::endl;
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            strings_.reserve(count_);
            
            for (int i = 0; i < count_; ++i) {
                const char* s = source_strings_[i].data();
                size_t size = source_strings_[i].size();
                t0 = timer.now();
                std::string string(s, size);
                result += timer.now() - t0;
                strings_.emplace_back(std::move(string));
            }
            strings_.clear();
            return result;
        }
        
    private:
        int count_;
        std::vector<std::string> strings_;
    };

    
    struct bench_std_string_createUTF16 : bench_string_base<bench_std_string_createUTF16, char16_t>
    {
        typedef bench_string_base<bench_std_string_createUTF16, char16_t> base;
        typedef typename base::timer timer;
        typedef typename base::time time;
        typedef typename base::duration duration;
        
        bench_std_string_createUTF16(const std::string& title) : base(title)  {}
        
        void prepare_imp(int count, int size = 16)
        {
            count_ = count;
            base::prepare_imp(count, size);
            std::cout << "function basic_string(const CharT*, size_type count) (" << count << " times),  with size: "
            << size << ", encoding equals UTF-16" << std::endl;
        }
        
        duration bench_imp()
        {
            timer timer;
            time t0;
            duration result = duration::zero();
            
            strings_.reserve(count_);
            
            for (int i = 0; i < count_; ++i) {
                const char16_t* s = source_strings_[i].data();
                size_t size = source_strings_[i].size();
                t0 = timer.now();
                std::u16string string(s, size);
                result += timer.now() - t0;
                strings_.emplace_back(std::move(string));
            }
            strings_.clear();
            return result;
        }
        
    private:
        int count_;
        std::vector<std::u16string> strings_;
    };
    
}


namespace {
    
    class CFDataCacheTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        CFDataCacheTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~CFDataCacheTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    /*        
     
     typedef typename map_t::iterator       iterator;        
     typedef typename map_t::const_iterator const_iterator;
     
     CFDataCache(std::size_t capacity = 300);
     
     ~CFDataCache();
     size_t  size() const;
     mapped_type find(const key_type& key);     
     const mapped_type find(const key_type& key) const;     
     bool insert(const key_type& key, mapped_type v);     
     void clear();
     
     */    
    
    
    TEST_F(CFDataCacheTest, DefaultCtor) 
    {
        CFDataCache<char> cache; 
        EXPECT_EQ(0, cache.size());
        
        typedef CFDataCache<char>::iterator iterator;
        
        iterator first = cache.begin();
        iterator last = cache.end();                
        EXPECT_TRUE(first == last);        
        EXPECT_EQ(0, cache.size());        
    }
    
    TEST_F(CFDataCacheTest, Insertion) 
    {
        CFDataCache<char> cache(1000*2); 
        
        typedef CFDataCache<char>::key_type key_t;
        typedef CFDataCache<char>::iterator iterator;
        typedef CFDataCache<char>::value_type value_t;
        
        char buffer[245];
        const int N = 1000;
        
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%d", i);
            std::pair<iterator, bool> result = cache.insert(key_t(buffer, strlen(buffer)), kCFNull);
            iterator iter = result.first;
            value_t value = *iter;
            EXPECT_TRUE(buffer != value.first.first);
            EXPECT_EQ(strlen(buffer), value.first.second);
            EXPECT_TRUE(memcmp(buffer, value.first.first, value.first.second) == 0);
            EXPECT_EQ(true, result.second);
        }
        
        EXPECT_EQ(N, cache.size());
    }
    
    TEST_F(CFDataCacheTest, Find) 
    {
        typedef CFDataCache<char> cache_t;
        typedef cache_t::key_type key_t;
        typedef cache_t::mapped_type mapped_t;
        typedef cache_t::iterator iterator;
        CFStringRef data = CFSTR("Test");
        
        cache_t cache;
        
        EXPECT_TRUE(cache.find(key_t("aaa", 3)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("aa", 2)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("a", 1)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("", 0)) == cache.end());
        
        cache.insert(key_t("aaa", 3), data);
        cache.insert(key_t("aa", 2), data);
        cache.insert(key_t("a", 1), data);
        cache.insert(key_t("", 0), data);
        
        EXPECT_TRUE(cache.find(key_t("aaa", 3)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("aa", 2)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("a", 1)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("", 0)) != cache.end());
        
        EXPECT_TRUE(cache.find(key_t("bbb", 3)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("bb", 2)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("b", 1)) == cache.end());
    }
    
    TEST_F(CFDataCacheTest, Clear) 
    {
        CFDataCache<char> cache;
        typedef CFDataCache<char>::key_type key_t;
        CFStringRef data = CFSTR("Test");
        
        cache.insert(key_t("aaa", 3), data);
        cache.insert(key_t("aa", 2), data);
        cache.insert(key_t("a", 1), data);
        cache.insert(key_t("", 0), data);
        
        EXPECT_TRUE(cache.find(key_t("aaa", 3)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("aa", 2)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("a", 1)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("", 0)) != cache.end());
        EXPECT_EQ(4, cache.size());
        
        cache.clear();
        EXPECT_EQ(0, cache.size());        
        EXPECT_TRUE(cache.find(key_t("bbb", 3)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("bb", 2)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("b", 1)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("", 0)) == cache.end());
    }
    
    TEST_F(CFDataCacheTest, Erase) 
    {
        CFDataCache<char> cache;
        typedef CFDataCache<char>::key_type key_t;
        CFStringRef data = CFSTR("Test");
        
        cache.insert(key_t("aaa", 3), data);
        cache.insert(key_t("aa", 2), data);
        cache.insert(key_t("a", 1), data);
        cache.insert(key_t("", 0), data);
        
        EXPECT_TRUE(cache.erase(key_t("aaa", 3)));
        EXPECT_TRUE(cache.find(key_t("aaa", 3)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("aa", 2)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("a", 1)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("", 0)) != cache.end());

        EXPECT_TRUE(cache.erase(key_t("aa", 2)));
        EXPECT_TRUE(cache.find(key_t("aa", 2)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("a", 1)) != cache.end());
        EXPECT_TRUE(cache.find(key_t("", 0)) != cache.end());

        EXPECT_TRUE(cache.erase(key_t("a", 1)));
        EXPECT_TRUE(cache.find(key_t("a", 1)) == cache.end());
        EXPECT_TRUE(cache.find(key_t("", 0)) != cache.end());
    
        EXPECT_TRUE(cache.erase(key_t("", 0)));
        EXPECT_TRUE(cache.find(key_t("", 0)) == cache.end());
    }
    
    
#if 0 and defined (DEBUG)
    TEST_F(CFDataCacheTest, DISABLED_BenchStringCreation)
#else
    TEST_F(CFDataCacheTest, BenchStringCreation)
#endif
    {
        std::cout << "\n== Bench Create CFString with UTF-8 source string ==" << std::endl;
        test::bench_CFString_createUTF8 b1("CFStringRef");
        
        b1.run(1000, 4);
        b1.run(1000, 16);
        b1.run(1000, 22);
        b1.run(1000, 32);
        b1.run(1000, 78);
        b1.run(1000, 128);
        
        std::cout << "\n\n== Bench Create NSString with UTF-8 source string==" << std::endl;
        test::bench_NSString_createUTF8 b2("NSString");
        
        b2.run(1000, 4);
        b2.run(1000, 16);
        b2.run(1000, 22);
        b2.run(1000, 32);
        b2.run(1000, 78);
        b2.run(1000, 128);
        
        std::cout << "\n== Bench Create CFString with UTF-16 source string ==" << std::endl;
        test::bench_CFString_createUTF16 b3("CFStringRef");
        
        b3.run(1000, 4);
        b3.run(1000, 16);
        b3.run(1000, 22);
        b3.run(1000, 32);
        b3.run(1000, 78);
        b3.run(1000, 128);
        
        std::cout << "\n\n== Bench Create NSString with UTF-16 source string==" << std::endl;
        test::bench_NSString_createUTF16 b4("NSString");
        
        b4.run(1000, 4);
        b4.run(1000, 16);
        b4.run(1000, 22);
        b4.run(1000, 32);
        b4.run(1000, 78);
        b4.run(1000, 128);
        
        
        
        std::cout << "\n== Bench Create std::string with UTF-8 source string ==" << std::endl;
        test::bench_std_string_createUTF8 b5("std::string");
        
        b5.run(1000, 4);
        b5.run(1000, 16);
        b5.run(1000, 22);
        b5.run(1000, 32);
        b5.run(1000, 78);
        b5.run(1000, 128);
        
        std::cout << "\n\n== Bench Create std::u16string with UTF-16 source string==" << std::endl;
        test::bench_std_string_createUTF16 b6("std::u16string");
        
        b6.run(1000, 4);
        b6.run(1000, 16);
        b6.run(1000, 22);
        b6.run(1000, 32);
        b6.run(1000, 128);
        
    }
    
    
#if 0 and defined (DEBUG)
    TEST_F(CFDataCacheTest, DISABLED_BenchCreate)
#else
    TEST_F(CFDataCacheTest, BenchCreate)
#endif
    {
        typedef std::map<std::string, CFStringRef> std_map_t;
        typedef std::unordered_map<std::string, CFStringRef> std_unordered_map_t;
        typedef boost::unordered_map<std::string, CFStringRef> boost_unordered_map_t;

        test::bench_map_create<std_map_t> b1("std::map<std::string, CFStringRef>");
        test::bench_map_create<std_unordered_map_t> b2("std::unordered_map<std::string, CFStringRef>");
        test::bench_map_create<boost_unordered_map_t> b3("boost::unordered_map<std::string, CFStringRef>");
        test::bench_NSMutableDictionary_create b4("NSMutableDictionary");
        test::bench_CFMutableDictionary_create b5("CFMutableDictionary");
        test::bench_CFDataCache_create b6("CFDataCache");
        test::bench_map_arena_allocator_create<std::map> b10("std:map with arena allocator");
        
        
        std::cout << "\n== Number of elements: 10 ==" << std::endl;
        b1.run(10);
        b2.run(10);
        b3.run(10);
        b4.run(10);
        b5.run(10);
        b6.run(10);
        b10.run(10);
        
        std::cout << "\n\n== Number of elements: 100 ==" << std::endl;
        b1.run(100);
        b2.run(100);
        b3.run(100);
        b4.run(100);
        b5.run(100);
        b6.run(100);
        b10.run(100);
        
        std::cout << "\n\n== Number of elements: 1000 ==" << std::endl;
        b1.run(1000);
        b2.run(1000);
        b3.run(1000);
        b4.run(1000);
        b5.run(1000);
        b6.run(1000);
        b10.run(1000);
        
        std::cout << "\n\n== Number of elements: 10000 ==" << std::endl;
        b1.run(10000);
        b2.run(10000);
        b3.run(10000);
        b4.run(10000);
        b5.run(10000);
        b6.run(10000);
        b10.run(10000);
        
        std::cout << "\n\n== Number of elements: 100000 ==" << std::endl;
        b1.run(100000);
        b2.run(100000);
        b3.run(100000);
        b4.run(100000);
        b5.run(100000);
        b6.run(100000);
        b10.run(100000);
        
    }

        
#if 0 and defined (DEBUG)
    TEST_F(CFDataCacheTest, DISABLED_BenchLookup)
#else
    TEST_F(CFDataCacheTest, BenchLookup)
#endif
    {
        
        typedef std::map<std::string, CFStringRef> std_map_t;
        typedef std::unordered_map<std::string, CFStringRef> std_unordered_map_t;
        typedef boost::unordered_map<std::string, CFStringRef> boost_unordered_map_t;
        
        test::bench_map_lookup<std_map_t> b1("std::map<std::string, CFStringRef>");
        test::bench_map_lookup<std_unordered_map_t> b2("std::unordered_map<std::string, CFStringRef>");
        test::bench_map_lookup<boost_unordered_map_t> b3("boost::unordered_map<std::string, CFStringRef>");
        test::bench_NSMutableDictionary_lookup b4("NSMutableDictionary");
        test::bench_NSMutableDictionary_lookup b5("CFMutableDictionary");
        test::bench_CFDataCache_lookup b6("CFDataCache");
        
        
        std::cout << "\n== Accessing all elements: 10 ==" << std::endl;
        b1.run(10);
        b2.run(10);
        b3.run(10);
        b4.run(10);
        b5.run(10);
        b6.run(10);
        
        std::cout << "\n\n== Accessing all elements: 100 ==" << std::endl;
        b1.run(100);
        b2.run(100);
        b3.run(100);
        b4.run(100);
        b5.run(100);
        b6.run(100);
        
        std::cout << "\n\n== Accessing all elements: 1000 ==" << std::endl;
        b1.run(1000);
        b2.run(1000);
        b3.run(1000);
        b4.run(1000);
        b5.run(1000);
        b6.run(1000);
        
        std::cout << "\n\n== Accessing all elements: 10000 ==" << std::endl;
        b1.run(10000);
        b2.run(10000);
        b3.run(10000);
        b4.run(10000);
        b5.run(10000);
        b6.run(10000);
        
        std::cout << "\n\n== Accessing all elements: 100000 ==" << std::endl;
        b1.run(100000);
        b2.run(100000);
        b3.run(100000);
        b4.run(100000);
        b5.run(100000);
        b6.run(100000);
        
        
    }
    
    
//    TEST_F(CFDataCacheTest, BenchErase)
//    {
//#if defined (DEBUG)
//        std::cout << "Performance Tests will be skipped." << std::endl;
//        return;
//#endif        
//        
//        const int N = 1000;
//
//        std::vector<int> keys;
//        keys.reserve(N);
//        for (int i = 0; i < N; ++i)
//            keys.push_back(i);
//        std::random_shuffle(keys.begin(), keys.end());
//        
//        // using std::map:
//        typedef std::map<std::string, CFStringRef> map_t;
//        char buffer[245];
//        timer t1;
//        map_t map;
//        for (int i = 0; i < N; ++i) {
//            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
//            map.insert(map_t::value_type(buffer, NULL)); 
//        }
//        for (int i = 0; i < N; ++i) {
//            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
//            t1.start();
//            std::size_t n = map.erase(buffer); 
//            t1.pause();
//            ASSERT_EQ( 1, n );
//        }
//        
//        
//        // using boost::unordered_map
//        typedef boost::unordered_map<std::string, CFStringRef> unordered_map_t;
//        timer t2;
//        unordered_map_t umap;
//        umap.rehash(N);
//        for (int i = 0; i < N; ++i) {
//            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
//            umap.insert(unordered_map_t::value_type(buffer, NULL));
//        }
//        for (int i = 0; i < N; ++i) {
//            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
//            t2.start();
//            std::size_t n = umap.erase(buffer);
//            t2.pause();
//            ASSERT_TRUE( n == 1);
//        }
//        
//        // using NSMutableDictionary:
//        timer t3;
//        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
//        NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity:N];
//        for (int i = 0; i < N; ++i) {
//            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
//            NSString* key = [[NSString alloc] initWithUTF8String:buffer];
//            [dict setObject:[NSNull null] forKey:key];            
//        }
//        for (int i = 0; i < N; ++i) {
//            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
//            NSUInteger count = [dict count];
//            t3.start();
//            NSString* key = [[NSString alloc] initWithUTF8String:buffer];            
//            [dict removeObjectForKey:key];
//            t3.pause();
//            ASSERT_TRUE( [dict count] == count - 1);
//        }
//        [dict release];
//        [pool release];        
//        
//        
//        // using cache:
//        typedef CFDataCache<char> cache_t;
//        timer t0;
//        cache_t cache(4000);
//        for (int i = 0; i < N; ++i) {
//            CFStringRef s = CFSTR("Test");
//            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
//            cache.insert(cache_t::key_type(buffer, 16), s);
//        }
//        for (int i = 0; i < N; ++i) {
//            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
//            t0.start();
//            bool deleted = cache.erase(cache_t::key_type(buffer, 16));
//            t0.pause();
//            ASSERT_TRUE(deleted);
//        }
//        
//        
//        printf("CFDataCacheTest Bench: Erasing %d elements in random order"
//               "\n\tstd::map:             %8.2f us"
//               "\n\tboost::unordered_map: %8.2f us"
//               "\n\tNSMutableDictionary:  %8.2f us"
//               "\n\tcache:                %8.2f us\n",
//               N,
//               t1.seconds()*1e6, t2.seconds()*1e6, t3.seconds()*1.e6, t0.seconds()*1.e6);
//        
//        EXPECT_LT( t0.seconds(), t1.seconds() );
//        EXPECT_LT( t0.seconds(), t2.seconds() );
//        EXPECT_LT( t0.seconds(), t3.seconds() );
//    }
    
    
}  //namespace
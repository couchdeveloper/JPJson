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

#include "gtest/gtest.h"



// mutex and semaphores
//#include "json/ObjC/mutex.hpp"
//#include "json/ObjC/semaphore.hpp"


// for testing
#import <Foundation/Foundation.h>
#include <dispatch/dispatch.h>
#include <string.h>
#include <stdio.h>
#include <stdexcept>
#include "utilities/timer.hpp"
#include <map>
#include <boost/tr1/unordered_map.hpp>
#include <algorithm>





using namespace json;

using json::objc::CFDataCache;
using utilities::timer;


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
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            std::pair<iterator, bool> result = cache.insert(key_t(buffer, strlen(buffer)), NULL);
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
    
    
    TEST_F(CFDataCacheTest, BenchCreate) 
    {
#if defined (DEBUG)
        std::cout << "Performance Tests will be skipped." << std::endl;
        return;
#endif        
        // using std::map:        
        typedef std::map<std::string, CFStringRef> map_t;
        
        char buffer[245];
        const int N = 1000;
        timer t1;
        map_t map;
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t1.start();
            map.insert(map_t::value_type(buffer, NULL)); 
            t1.pause();
        }
        
        // using boost::unordered_map
        typedef boost::unordered_map<std::string, CFStringRef> unordered_map_t;
        timer t2;
        unordered_map_t umap;
        umap.rehash(N);
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t2.start();
            umap.insert(unordered_map_t::value_type(buffer, NULL));
            t2.pause();
        }
                
        // using NSMutableDictionary:
        timer t3;
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity:N];
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t3.start();
            NSString* key = [[NSString alloc] initWithUTF8String:buffer];
            [dict setObject:[NSNull null] forKey:key];            
            t3.pause();
        }
        [dict release];
        [pool release];        
        

        // using cache:
        typedef CFDataCache<char> cache_t;
        timer t0;
        cache_t cache(4000);
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t0.start();
            cache.insert(cache_t::key_type(buffer, 16), NULL);
            t0.pause();
        }
        
        
        printf("CFDataCacheTest Bench: Creation of %d elements"
               "\n\tstd::map:             %8.2f us"
               "\n\tboost::unordered_map: %8.2f us"
               "\n\tNSMutableDictionary:  %8.2f us"
               "\n\tcache:                %8.2f us\n",
               N,
               t1.seconds()*1e6, t2.seconds()*1e6, t3.seconds()*1.e6, t0.seconds()*1.e6);
        
        EXPECT_LT( t0.seconds(), t1.seconds() );
        EXPECT_LT( t0.seconds(), t2.seconds() );
        EXPECT_LT( t0.seconds(), t3.seconds() );
    }
    

    TEST_F(CFDataCacheTest, BenchLookup) 
    {
#if defined (DEBUG)
        std::cout << "Performance Tests will be skipped." << std::endl;
        return;
#endif        
        
        // using std::map:
        typedef std::map<std::string, CFStringRef> map_t;
        
        char buffer[245];
        const int N = 1000;
        timer t1;
        map_t map;
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            map.insert(map_t::value_type(buffer, NULL)); 
        }
        
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t1.start();
            map_t::iterator iter =  map.find(buffer); 
            t1.pause();
            ASSERT_TRUE( iter != map.end());
        }
        
        
        // using boost::unordered_map
        typedef boost::unordered_map<std::string, CFStringRef> unordered_map_t;
        timer t2;
        unordered_map_t umap;
        umap.rehash(N);
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            umap.insert(unordered_map_t::value_type(buffer, NULL));
        }
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t2.start();
            unordered_map_t::iterator iter = umap.find(buffer);
            t2.pause();
            ASSERT_TRUE( iter != umap.end());
        }
        
        // using NSMutableDictionary:
        timer t3;
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity:N];
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            NSString* key = [[NSString alloc] initWithUTF8String:buffer];
            [dict setObject:[NSNull null] forKey:key];            
        }
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t3.start();
            NSString* key = [[NSString alloc] initWithUTF8String:buffer];            
            id result = [dict objectForKey:key];
            t3.pause();
            ASSERT_TRUE(result != nil);
        }
        [dict release];
        [pool release];        
        
        
        // using cache:
        typedef CFDataCache<char> cache_t;
        timer t0;
        cache_t cache(4000);
        for (int i = 0; i < N; ++i) {
            CFStringRef s = CFSTR("Test");
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            cache.insert(cache_t::key_type(buffer, 16), s);
        }
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            t0.start();
             cache_t::iterator iter = cache.find(cache_t::key_type(buffer, 16));
            t0.pause();
            ASSERT_TRUE(iter != cache.end());
        }
        
        
        printf("CFDataCacheTest Bench: Lookup of %d elements"
               "\n\tstd::map:             %8.2f us"
               "\n\tboost::unordered_map: %8.2f us"
               "\n\tNSMutableDictionary:  %8.2f us"
               "\n\tcache:                %8.2f us\n",
               N,
               t1.seconds()*1e6, t2.seconds()*1e6, t3.seconds()*1.e6, t0.seconds()*1.e6);
        
        EXPECT_LT( t0.seconds(), t1.seconds() );
        EXPECT_LT( t0.seconds(), t2.seconds() );
        EXPECT_LT( t0.seconds(), t3.seconds() );
    }
    
    
    TEST_F(CFDataCacheTest, BenchErase) 
    {
#if defined (DEBUG)
        std::cout << "Performance Tests will be skipped." << std::endl;
        return;
#endif        
        
        const int N = 1000;

        std::vector<int> keys;
        keys.reserve(N);
        for (int i = 0; i < N; ++i)
            keys.push_back(i);
        std::random_shuffle(keys.begin(), keys.end());
        
        // using std::map:
        typedef std::map<std::string, CFStringRef> map_t;
        char buffer[245];
        timer t1;
        map_t map;
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            map.insert(map_t::value_type(buffer, NULL)); 
        }
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
            t1.start();
            std::size_t n = map.erase(buffer); 
            t1.pause();
            ASSERT_EQ( 1, n );
        }
        
        
        // using boost::unordered_map
        typedef boost::unordered_map<std::string, CFStringRef> unordered_map_t;
        timer t2;
        unordered_map_t umap;
        umap.rehash(N);
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            umap.insert(unordered_map_t::value_type(buffer, NULL));
        }
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
            t2.start();
            std::size_t n = umap.erase(buffer);
            t2.pause();
            ASSERT_TRUE( n == 1);
        }
        
        // using NSMutableDictionary:
        timer t3;
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity:N];
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            NSString* key = [[NSString alloc] initWithUTF8String:buffer];
            [dict setObject:[NSNull null] forKey:key];            
        }
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
            NSUInteger count = [dict count];
            t3.start();
            NSString* key = [[NSString alloc] initWithUTF8String:buffer];            
            [dict removeObjectForKey:key];
            t3.pause();
            ASSERT_TRUE( [dict count] == count - 1);
        }
        [dict release];
        [pool release];        
        
        
        // using cache:
        typedef CFDataCache<char> cache_t;
        timer t0;
        cache_t cache(4000);
        for (int i = 0; i < N; ++i) {
            CFStringRef s = CFSTR("Test");
            snprintf(buffer, sizeof(buffer), "key-%12.d", i);
            cache.insert(cache_t::key_type(buffer, 16), s);
        }
        for (int i = 0; i < N; ++i) {
            snprintf(buffer, sizeof(buffer), "key-%12.d", keys[i]);
            t0.start();
            bool deleted = cache.erase(cache_t::key_type(buffer, 16));
            t0.pause();
            ASSERT_TRUE(deleted);
        }
        
        
        printf("CFDataCacheTest Bench: Erasing %d elements in random order"
               "\n\tstd::map:             %8.2f us"
               "\n\tboost::unordered_map: %8.2f us"
               "\n\tNSMutableDictionary:  %8.2f us"
               "\n\tcache:                %8.2f us\n",
               N,
               t1.seconds()*1e6, t2.seconds()*1e6, t3.seconds()*1.e6, t0.seconds()*1.e6);
        
        EXPECT_LT( t0.seconds(), t1.seconds() );
        EXPECT_LT( t0.seconds(), t2.seconds() );
        EXPECT_LT( t0.seconds(), t3.seconds() );
    }
    
    
}  //namespace
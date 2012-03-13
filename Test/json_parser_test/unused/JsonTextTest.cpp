//
//  JsonTextTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/23/11.
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
// This test is no longer needed since the byte swap iterator became obsolete.

#include "json/byte_swap_iterator.hpp"
#include "json/unicode_utilities.hpp"
#include "json/endian.hpp"
#include "json/byte_swap.hpp"
#include "gtest/gtest.h"
#include <wchar.h>
#include <iostream>
#include <iomanip>
#include "utilities/timer.hpp"
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_traits.hpp>



//
//  Objective:  test the byte_swap_iterator
//

namespace {
    
    using namespace json;
    using json::internal::big_endian_tag;
    using json::internal::little_endian_tag;
    using json::internal::byte_swap_iterator;
    using json::internal::iterator_selector;
    using json::internal::host_endianness;
    using json::internal::byte_swap_iterator;
    
    using json::unicode::iterator_encoding;
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    using json::unicode::platform_encoding_tag;
    
    
    // The fixture for testing byte_swap_iterator
    class JsonTextTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonTextTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonTextTest() {
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
    
    
    //
    // Default Iterator Adapter Type
    //  
    template <typename Iterator>
    struct default_byte_swap_iterator {
        typedef typename iterator_encoding<Iterator>::type  SourceEncoding;
        typedef byte_swap_iterator<Iterator, SourceEncoding> type;
    };
    
    template <typename Iterator>
    typename default_byte_swap_iterator<Iterator>::type    
    make_byte_swap_iterator(Iterator iter) {
        typedef typename default_byte_swap_iterator<Iterator>::type result_t;
        return result_t(iter);
    }
    
    //
    // Demonstrating the use of the iterator adapter in a "parse" function:
    //
    template <typename IteratorT, typename EncodingT>
    inline bool foo(IteratorT& first, IteratorT last, EncodingT encoding) 
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::utf_encoding_tag, EncodingT>::value) );
        BOOST_STATIC_ASSERT( (sizeof(typename boost::iterator_value<IteratorT>::type) == sizeof(typename EncodingT::code_unit_type)) );

        // Create an iterator adapter which swaps bytes when dereferencing, but only
        // if swapping is actually required.
        typedef internal::byte_swap_iterator<IteratorT, EncodingT> iterator_adapter;
                
        iterator_adapter first_(first);
        iterator_adapter last_(last);
        
        if (first_ != last_) {
            ++first_;
            first = first_.base();  // assign the output parameter 'first' the current iterator.
            return true;
        } else {
            return false;
        }
    }

    
    
    
    TEST_F(JsonTextTest, ByteSwapIterator_UTF_8) 
    {
        const char* s = "0102030405060708";
        size_t len = strlen(s);
        byte_swap_iterator<const char*, UTF_8_encoding_tag> iter;
        iter = s;
        for (int i = 0; i< len; ++i)
        {
            EXPECT_EQ( s[i],  *iter);
            if (s[i] != *iter)
                break;
            ++iter;
        }
    }
    
    TEST_F(JsonTextTest, ByteSwapIterator_UTF_16LE) 
    {
        const char* s = "0102030405060708";
        size_t len = strlen(s) / 2;
        const uint16_t* p = (const uint16_t*)s;
        
        byte_swap_iterator<const uint16_t*, UTF_16LE_encoding_tag> iter;
        const bool isHostIsLittleEndian = host_endianness::is_little_endian;        
        iter = p;
        
        for (int i = 0; i < len; ++i)
        {
            uint16_t c = *iter;
            if (!isHostIsLittleEndian)
                c = json::byte_swap(c);
            
            EXPECT_EQ( p[i],  c);
            if (s[i] != c)
                break;
            ++iter;
        }
    }
    
    TEST_F(JsonTextTest, ByteSwapIterator_UTF_16BE) 
    {
        const char* s = "0102030405060708";
        size_t len = strlen(s) / 2;
        const uint16_t* p = (const uint16_t*)s;
        
        byte_swap_iterator<const uint16_t*, UTF_16BE_encoding_tag> iter;
        const bool isHostIsBigEndian = host_endianness::is_big_endian;        
        iter = p;
        
        for (int i = 0; i < len; ++i)
        {
            uint16_t c = *iter;
            if (!isHostIsBigEndian)
                c = json::byte_swap(c);
            
            EXPECT_EQ( p[i],  c);
            if (s[i] != c)
                break;
            ++iter;
        }
    }
    
    TEST_F(JsonTextTest, ByteSwapIterator_InternalEncoding) 
    {
        // platform encoding should never swap bytes - no matter of
        // the type of the value.
        
        const wchar_t* ws = L"0102030405060708";
        size_t len = wcslen(ws);
        
        byte_swap_iterator<const wchar_t*, platform_encoding_tag> iter;
        iter = ws;
        
        for (int i = 0; i < len; ++i)
        {
            wchar_t c = *iter;
            EXPECT_EQ( ws[i],  c);
            if (ws[i] != c)
                break;
            ++iter;
        }
    }
    
    TEST_F(JsonTextTest, ByteSwapIterator_Iterating) 
    {
        // swapping iterator, swap applied:
        const wchar_t* ws = L"0102030405060708";
        typedef const wchar_t* iterator_t;
        size_t len = wcslen(ws);
        iterator_t first = ws;
        iterator_t last = ws + len;
       
        // 1) Test whether a swapping iterator modifies the in/out parameter 'first': 
        // Deduce an encoding which will force to use a swapping iterator adapter:
        typedef boost::mpl::if_c<
            host_endianness::is_big_endian, 
            UTF_32LE_encoding_tag, 
            UTF_32BE_encoding_tag
        >::type encoding_t;

        foo(first, last, encoding_t());
        EXPECT_TRUE(  (ws + 1) == first );
        
        // 2) Test whether a non-swapping iterator modifies the in/out parameter 'first':
        // Use "internal_encoding_tag" as encoding which shall never create a swapping iterator adapter:
        first = ws;
        foo(first, last, platform_encoding_tag());
        EXPECT_TRUE(  (ws + 1) == first );        
    } 
    
    TEST_F(JsonTextTest, BenchByteSwapIterator1) 
    {
#if !defined (NDEBUG)
        std::cout << "WARNING: BenchByteSwapIterator1 skipped: no release build, performance result may be misleading!\n" ;
#else      
        
        using utilities::timer;
        
        const size_t N = 10000;        
        typedef uint16_t vector_t[N];
        typedef uint16_t const* iterator_t;

        vector_t v;
        iterator_t first = v;
        
        for (int i = 0; i < N; ++i)
            v[i] = i;
        
        // non-swapping iterator
        volatile int sum = 0;        
        timer t;

        t.start();
        iterator_t iter1;
        for (int i = 0; i < 1000; ++i) {
            iter1 = first;
            for (int j = 0; j < N; ++j)
            {
                sum = *iter1++;
            }            
        }        
        t.stop();
        double noconv = t.seconds();
        printf("BenchByteSwapIterator1: elapsed time for non-swappping iterator:    %12.3f µs\n", noconv*1e6);
        
        // swapping iterator, no swap applied:
        sum = 0;
        t.reset();
        t.start();
        byte_swap_iterator<iterator_t, platform_encoding_tag> iter2;
        for (int i = 0; i < 1000; ++i) {
            iter2 = first;
            for (int j = 0; j < N; ++j)
                sum = *iter2++;
        }        
        t.stop();
        
        printf("BenchByteSwapIterator1: elapsed time for swap-iterator (no-conv):   %12.3f µs\n", t.seconds()*1e6);
        double degradation = t.seconds() / noconv - 1.0;
        
        // Accepted degradion is 0.0
        EXPECT_LT( degradation, 0.1 );
        
        
        // swapping iterator, swap applied:
        typedef boost::mpl::if_c<
            host_endianness::is_big_endian, 
            byte_swap_iterator<iterator_t, UTF_16LE_encoding_tag>, 
            byte_swap_iterator<iterator_t, UTF_16BE_encoding_tag>
        >::type swap_iter_t;
        
        sum = 0;
        t.reset();
        t.start();
        swap_iter_t iter3;
        for (int i = 0; i < 1000; ++i) {
            iter3 = first;
            for (int j = 0; j < N; ++j)
                sum = *iter3++;
        }        
        t.stop();
        
        printf("BenchByteSwapIterator1: elapsed time for swap-iterator (swapping):  %12.3f µs\n", t.seconds()*1e6);
        degradation = t.seconds() / noconv - 1.0;
        
        // Accepted degradion is 1.0
        EXPECT_LT( degradation, 1.0 );
        
#endif        
    }
    
    TEST_F(JsonTextTest, SelectByteSwapIterator) 
    {
        // Note: using the iterator_selector is not fool-proof. Well, it's internal anyway.
        
        EXPECT_TRUE( (boost::is_same<const char*,       
                      iterator_selector<const char*, UTF_8_encoding_tag>::type>::value) );        
        
        EXPECT_TRUE( (boost::is_same<const uint16_t*,   
                      iterator_selector<const uint16_t*, platform_encoding_tag>::type>::value) );

        EXPECT_TRUE( (boost::is_same<const uint32_t*,   
                      iterator_selector<const uint32_t*, platform_encoding_tag>::type>::value) );
        
        EXPECT_TRUE( (boost::is_same<const wchar_t*,   
                      iterator_selector<const wchar_t*, platform_encoding_tag>::type>::value) );
        
        
        
        // swapping iterator
        typedef boost::mpl::if_c<
            host_endianness::is_big_endian, 
            byte_swap_iterator<const uint16_t*, UTF_16LE_encoding_tag>, 
            byte_swap_iterator<const uint16_t*, UTF_16BE_encoding_tag>
        >::type swapping_iterator_t;
        
        typedef boost::mpl::if_c<
        host_endianness::is_big_endian, 
        iterator_selector<const uint16_t*, UTF_16LE_encoding_tag>::type, 
        iterator_selector<const uint16_t*, UTF_16BE_encoding_tag>::type
        >::type test_iterator_1;
        
        EXPECT_TRUE( (boost::is_same<test_iterator_1, swapping_iterator_t>::value) );
        
        
        // none-swapping iterator
        typedef boost::mpl::if_c<
            host_endianness::is_big_endian, 
            iterator_selector<const uint16_t*, UTF_16BE_encoding_tag>::type, 
            iterator_selector<const uint16_t*, UTF_16LE_encoding_tag>::type
        >::type test_iterator_2;
        
        EXPECT_TRUE( (boost::is_same<test_iterator_2, const uint16_t*>::value) );
        
    }

    
    
    
}  // namespace

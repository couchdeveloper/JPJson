//
//  IteratorTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/25/11.
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


#include "json/endian/byte_swap_iterator.hpp"
#include <gtest/gtest.h>

#include "json/unicode/unicode_utilities.hpp"
#include "json/endian/endian.hpp"
#include "utilities/timer.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <wchar.h>
#include <iostream>
#include <iomanip>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/iterator/iterator_traits.hpp>



using testing::Types;

using namespace json;

namespace test {


    //
    //  This class solely demonstrates the use of an iterator adapter
    //  in the parser class. The test::parser class copies the json::parser
    //  parts relevant for the iterator adapter.
    //
    
    template <
          typename InputIterator
        , typename SourceEncoding
    >
    class parser 
    {
    public:
        // Create an iterator adapter type which possibly swaps bytes if required
        // when dereferencing:
        typedef typename internal::byte_swap_iterator<InputIterator, SourceEncoding>    iterator;
        
        typedef SourceEncoding                                  source_encoding;
        typedef typename source_encoding::code_unit_type        code_t;
        
        // Assert:
        // InputIterator shall be the source input iterator. The member function
        // parse() creates the iterator adapters and uses this iterators throught 
        // this class.
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, SourceEncoding>::value) );
        BOOST_STATIC_ASSERT( (sizeof(typename boost::iterator_value<InputIterator>::type) == sizeof(typename SourceEncoding::code_unit_type)) );
        
    public:
        parser() {}
        parser(InputIterator& first, InputIterator last) 
            : p_(first), last_(last)
        {}
        
        code_t  ch() const      { return p_ != last_ ? *p_ : 0; }
        code_t  inc_ch()        { return p_ != last_ ? *(++p_) : 0; }
        code_t  ch_inc()        { return p_ != last_ ? *(p_++) : 0; }
        void    inc()           { if (p_ != last_) ++p_; }
        bool    is_eot() const  { return p_ == last_; }
        

        bool parse(InputIterator& first, InputIterator last) 
        {
            p_ = first;
            last_ = last;
                        
            while (!is_eot()) {
                code_t code = ch();
                if (code == 0)
                    break;
                inc();
            }
            
            first = p_.base();
            
            return true;
        }
        
        
    
    public:
        iterator            p_;     // iterator adapter
        iterator            last_;  // iterator adapter
    };

    template <typename IteratorT>
    inline bool
    parse(IteratorT& first, IteratorT last)
    {
        typedef typename json::unicode::iterator_encoding<IteratorT>::type  SourceEncoding;
        typedef parser<IteratorT, SourceEncoding> parser_t;
        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, SourceEncoding>::value) );
        BOOST_STATIC_ASSERT( (sizeof(typename boost::iterator_value<IteratorT>::type) == sizeof(typename SourceEncoding::code_unit_type)) );
        
        parser_t parser; // create the parser
        bool result = parser.parse(first, last);
        if (result and first == last) {
            return true;
        } else {
            return false;
        }
    }
    
}

namespace {
    
    
    //
    // Demonstrating the use of the iterator adapter in a "parse" function:
    //
    template <typename IteratorT, typename EncodingT>
    inline bool foo(IteratorT& first, IteratorT last, EncodingT encoding) 
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, EncodingT>::value) );
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
    

}



template <class T>
class IteratorTest1 : public testing::Test {
};

// The list of Input Iterators we support
typedef Types<char*, unsigned char*, signed char*, wchar_t*> InputIterator_Types;



TYPED_TEST_CASE(IteratorTest1, InputIterator_Types);
TYPED_TEST(IteratorTest1, VerifyNonSwappingIteratorCreation) 
{
    typedef TypeParam SourceIterator;
    
    // Get the iterator adapter type based on the source iterator type (ParamType) 
    // and the source encoding:
    // The following shall create always non-swapping iterators:
    // When the encoding of the iterator (the second template paramerter) is
    // omitted, the iterator becomes always an non-swapping iterator.
    
    typedef typename internal::byte_swap_iterator<SourceIterator> iterator;
    
    // Get the value type of the source iterator:
    typedef typename boost::iterator_value<SourceIterator>::type value_t;
    
    // 
    const char buffer[] = "ABCD1234";
    value_t v0;
    memcpy(&v0, buffer, sizeof(value_t));
    SourceIterator p = &v0;
    iterator iter = p;
    value_t v1 = *iter;
    
    EXPECT_EQ(v0, v1);
    
}



template <class T>
class IteratorTest2 : public testing::Test {
};
typedef Types<char, wchar_t> StdStringCharT_Types;

TYPED_TEST_CASE(IteratorTest2, StdStringCharT_Types);

TYPED_TEST(IteratorTest2, VerifyNonSwappingIteratorCreation) 
{
    typedef std::basic_string<TypeParam> string_t;
    typedef typename string_t::iterator SourceIterator;
    
    // Get the iterator adapter type based on the source iterator type (ParamType) 
    // and the source encoding:
    // The following shall create always non-swapping iterators:
    // When the encoding of the iterator (the second template paramerter) is
    // omitted, the iterator becomes always an non-swapping iterator.
    
    typedef typename internal::byte_swap_iterator<SourceIterator> iterator;
    
    // Get the value type of the source iterator:
    typedef typename boost::iterator_value<SourceIterator>::type value_t;
    
    
    const char buffer[] = "ABCD1234";
    value_t v0;
    memcpy(&v0, buffer, sizeof(value_t));
    string_t s;
    s.append(1, v0);
    
    SourceIterator p =  s.begin();
    iterator iter = p;
    value_t v1 = *iter;
    
    EXPECT_EQ(v0, v1);
    
}


template <class T>
class IteratorTest3 : public testing::Test {
};
typedef Types<char, wchar_t> StdStream_Types;

TYPED_TEST_CASE(IteratorTest3, StdStream_Types);

TYPED_TEST(IteratorTest3, VerifyNonSwappingIteratorCreation) 
{
    typedef std::basic_stringstream<TypeParam> stream_t;
    //typedef std::stringstream stream_t;
    
    typedef std::istream_iterator<TypeParam, TypeParam> SourceIterator;
    
    // Get the iterator adapter type based on the source iterator type (ParamType) 
    // and the source encoding:
    // The following shall create always non-swapping iterators:
    // When the encoding of the iterator (the second template parameter) is
    // omitted, the iterator becomes always an non-swapping iterator.    
    typedef typename internal::byte_swap_iterator<SourceIterator> iterator;
    
    // Get the value type of the source iterator:
    typedef typename boost::iterator_value<SourceIterator>::type value_t;
    
    const char buffer[] = "ABCD1234";
    value_t v0;
    memcpy(&v0, buffer, sizeof(value_t));

    stream_t ss;
    ss.put(v0);
    
    std::istream_iterator<TypeParam, TypeParam> eos;
    std::istream_iterator<TypeParam, TypeParam> p(ss);
    
    iterator iter = p;
    
    value_t v1 = *iter;
    
    EXPECT_EQ(v0, v1);
    
}


/*
 // Deduce an encoding which will force to use a swapping iterator adapter:
 typedef boost::mpl::if_c<
 host_endianness::is_big_endian, 
 UTF_32LE_encoding_tag, 
 UTF_32BE_encoding_tag
 >::type encoding_t;
*/


namespace {
    
    using namespace json;
    using json::internal::big_endian_tag;
    using json::internal::little_endian_tag;
    using json::internal::byte_swap_iterator;
    using json::internal::iterator_selector;
    using json::internal::host_endianness;
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    using json::unicode::platform_encoding_tag;
    
    
    // The fixture for testing byte_swap_iterator
    class IteratorTestA : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        IteratorTestA() {
            // You can do set-up work for each test here.
            
#if defined (DEBUG)
            std::cout << "WARNING: DEBUG mode: Benchmark test will be skipped" << std::endl ;
#endif      
            
        }
        
        virtual ~IteratorTestA() {
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


    TEST_F(IteratorTestA, ByteSwapIterator_UTF_8) 
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
        
        const std::string str(s, len);
        typedef std::string::const_iterator iter_t;

        byte_swap_iterator<iter_t, UTF_8_encoding_tag> iter2;
        iter2 = str.begin();
        for (int i = 0; i< len; ++i)
        {
            EXPECT_EQ( s[i],  *iter2);
            if (s[i] != *iter2)
                break;
            ++iter2;
        }
        
        
        iter2 = str.begin();
        for (int i = 0; i< len; ++i)
        {
            EXPECT_EQ( s[i],  *iter2);
            if (s[i] != *iter2)
                break;
            iter2++;
        }
    }

    
    TEST_F(IteratorTestA, ByteSwapIterator_UTF_16LE) 
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

    TEST_F(IteratorTestA, ByteSwapIterator_UTF_16BE) 
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

    TEST_F(IteratorTestA, ByteSwapIterator_InternalEncoding) 
    {
        // Internal encoding should never swap bytes - no matter of
        // the type of the value.
        
        const wchar_t* ws = L"0102030405060708";
        size_t len = wcslen(ws);
        
        byte_swap_iterator<const wchar_t*, platform_encoding_tag> iter;
      
        // pre increment
        iter = ws;        
        for (int i = 0; i < len; ++i)
        {
            wchar_t c = *iter;
            ++iter;
            EXPECT_EQ( ws[i],  c);
            if (ws[i] != c)
                break;
        }
        
        // post increment
        iter = ws;
        for (int i = 0; i < len; ++i)
        {
            wchar_t c = *iter;
            EXPECT_EQ( ws[i],  c);
            if (ws[i] != c)
                break;
            iter++;
        }
        
        // post increment
        iter = ws;
        for (int i = 0; i < len; ++i)
        {
            wchar_t c = *iter++;
            EXPECT_EQ( ws[i],  c);
            if (ws[i] != c)
                break;
        }
        
        
    }

    TEST_F(IteratorTestA, ByteSwapIterator_Iterating) 
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
        // Use "platform_encoding_tag" as encoding which shall never create a swapping iterator adapter:
        first = ws;
        foo(first, last, platform_encoding_tag());
        EXPECT_TRUE(  (ws + 1) == first );        
    } 

    
#if defined (DEBUG)    
    TEST_F(IteratorTestA, DISABLED_BenchByteSwapIterator1) 
#else
    TEST_F(IteratorTestA, BenchByteSwapIterator1) 
#endif    
    {
        
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
        
    }

    TEST_F(IteratorTestA, SelectByteSwapIterator) 
    {
        
        // A source iterator whose value type equals char or char8_t and a 
        // UTF-8 encoding scheme shall select a non-swapping internal iterator,
        // i.e., shall return the given iterator itself:
        EXPECT_TRUE( (boost::is_same<const char*,       
                      iterator_selector<const char*, UTF_8_encoding_tag>::type>::value) );        
#if Cpp0x        
        EXPECT_TRUE( (boost::is_same<const char8_t*,   
                      iterator_selector<const uint8_t*, UTF_8_encoding_tag>::type>::value) );
#endif                

        // A source iterator whose value type equals uint16_t or char16_t and a
        // platform encoding shall return the given iterator itself:
        EXPECT_TRUE( (boost::is_same<const uint16_t*,   
                      iterator_selector<const uint16_t*, platform_encoding_tag>::type>::value) );
#if Cpp0x        
        EXPECT_TRUE( (boost::is_same<const char16_t*,   
                      iterator_selector<const char16_t*, platform_encoding_tag>::type>::value) );
#endif        
                
        // A source iterator whose value type equals uint32_t or char32_t and a
        // platform encdoding shall return the iterator itself:
        EXPECT_TRUE( (boost::is_same<const uint32_t*,   
                      iterator_selector<const uint32_t*, platform_encoding_tag>::type>::value) );
        
        EXPECT_TRUE( (boost::is_same<const wchar_t*,   
                      iterator_selector<const wchar_t*, platform_encoding_tag>::type>::value) );
#if Cpp0x        
        EXPECT_TRUE( (boost::is_same<const char23_t*,   
                      iterator_selector<const char32_t*, platform_encoding_tag>::type>::value) );
#endif        
        
        
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
    
    TEST_F(IteratorTestA, ParserIterator) 
    {
        const std::string s = "abcdefgh123456";
        std::vector<char> v = std::vector<char>(s.begin(), s.end());
        
        typedef std::vector<char>::const_iterator iter_t;
        iter_t first = v.begin();
        iter_t last = v.end();
        
        typedef test::parser<iter_t, UTF_8_encoding_tag> parser_t;
        typedef parser_t::code_t code_t;
        
        // increment
        parser_t parser1(first, last);
        for (int i = 0; i < s.size(); ++i)
        {
            code_t c = parser1.ch();
            parser1.inc();
            EXPECT_EQ( s[i],  c);
            if (s[i] != c)
                break;
        }
        
        EXPECT_TRUE (parser1.p_ == parser1.last_);
        EXPECT_FALSE (parser1.p_ != parser1.last_);
        
        // pre increment
        first = v.begin();
        parser_t parser2(first, last);
        for (int i = 0; i < s.size() - 1; ++i)
        {
            code_t c = parser2.inc_ch();
            EXPECT_EQ( s[i+1],  c);
            if (s[i+1] != c)
                break;
        }
        
        // post increment
        first = v.begin();
        parser_t parser3(first, last);
        for (int i = 0; i < s.size(); ++i)
        {
            code_t c = parser3.ch_inc();
            EXPECT_EQ( s[i],  c);
            if (s[i] != c)
                break;
        }
    }
    
    
    TEST_F(IteratorTestA, ParserIterator2) 
    {
        const char* s = "abcdefgh123456";
        typedef const char* InputIterator;
        typedef test::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        typedef parser_t::iterator iterator;  // iterator adapter
        
        InputIterator first = s;
        InputIterator last = s + strlen(s);
        
        iterator it1 = first;
        iterator it2 = last;
        
        while (it1 != it2)
        {
            ++it1;
        }

        EXPECT_TRUE (it1 == it2);
        
        parser_t p;
        bool result = p.parse(first, last);
        EXPECT_TRUE (result == true);
    }
    
    
    TEST_F(IteratorTestA, IteratorAdapters) 
    {
        typedef std::string::const_iterator  iterator1;
        typedef test::parser<iterator1, UTF_8_encoding_tag> parser_t1;
        std::string s1 = "abc";
        iterator1 first1 = s1.begin();
        iterator1 last1 = s1.end();
        parser_t1 p1(first1, last1);
        char c1 = p1.ch();
        EXPECT_TRUE ( c1 == s1[0] );
        
        typedef std::string::iterator  iterator2;
        typedef test::parser<iterator2, UTF_8_encoding_tag> parser_t2;
        std::string s2 = "abc";
        iterator2 first2 = s2.begin();
        iterator2 last2 = s2.end();
        parser_t2 p2(first2, last2);
        char c2 = p2.ch();
        EXPECT_TRUE ( c2 == s2[0] );
    }
    
    
    // Test the test code
    TEST_F(IteratorTestA, TestParseFunction1)  
    {
        // Testing test::parser
        
        const char* s = "abcdefgh123456";
        typedef const char* InputIterator;
        typedef test::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        
        InputIterator first = s;
        InputIterator last = s + strlen(s);
        
        bool result = test::parse(first, last);
        EXPECT_TRUE (result == true);
        EXPECT_TRUE (first == last);
    }
    
    // Test the test code
    TEST_F(IteratorTestA, TestParseFunction2)  
    {
        // Testing test::parser
        
        std::string s = "abcdefgh123456";
        std::vector<char> v = std::vector<char>(s.begin(), s.end());
        typedef std::vector<char>::const_iterator InputIterator;
        typedef test::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        
        InputIterator first = v.begin();
        InputIterator last = v.end();
        
        bool result = test::parse(first, last);
        EXPECT_TRUE (result == true);
        EXPECT_TRUE (first == last);
    }
    
    // Test the test code
    TEST_F(IteratorTestA, TestParseFunction3)  
    {
        // Testing test::parser
        
        std::string s = "abcdefgh123456";
        typedef std::string::const_iterator InputIterator;
        typedef test::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        
        InputIterator first = s.begin();
        InputIterator last = s.end();
        
        bool result = test::parse(first, last);
        EXPECT_TRUE (result == true);
        EXPECT_TRUE (first == last);
    }
    
    
}





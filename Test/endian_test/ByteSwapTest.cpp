//
//  ByteSwapTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/21/11.
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

//
//  StringBufferTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 4/8/11.
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

#include "json/endian/byte_swap.hpp"
#include <gtest/gtest.h>

#include "utilities/timer.hpp"
#include "json/unicode/unicode_traits.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/or.hpp>

#include <iostream>
#include <iomanip>

namespace {
    
    using namespace json;
    using json::internal::big_endian_tag;
    using json::internal::little_endian_tag;
    
    
    // The fixture for testing class JsonParser.
    class ByteSwapTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        ByteSwapTest() {
            // You can do set-up work for each test here.
            
#if defined (DEBUG)
            std::cout << "WARNING: Running in DEBUG mode. Benchmark tests will be disabled!\n" ;
#endif        
            
        }
        
        virtual ~ByteSwapTest() {
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
    
    
    TEST_F(ByteSwapTest, EndianConstExpr)
    {
        constexpr bool isLittleEndian = json::internal::host_endianness::is_little_endian;
        constexpr bool isBigEndian = json::internal::host_endianness::is_big_endian;
        EXPECT_TRUE(isLittleEndian != isBigEndian);
    }
    
    TEST_F(ByteSwapTest, ExecutionEnvironment) 
    {
        using std::hex;
        using std::cout;
        using std::endl;
        using std::setw;
        using std::setfill; 
        
        using json::internal::run_time_host_endianness;
        using json::internal::host_endianness;
        
#if defined (DEBUG)
        cout << "Runtime: Host is big endian: " << (run_time_host_endianness::is_big_endian() ? "Yes" : "No")  << endl;
        cout << "Runtime: Host is little endian: " << (run_time_host_endianness::is_little_endian() ? "Yes" : "No")  << endl;
        
        cout << "Compiletime: Host is big endian: " << (host_endianness::is_big_endian ? "Yes" : "No")  << endl;
        cout << "Compiletime: Host is little endian: " << (host_endianness::is_little_endian ? "Yes" : "No")  << endl;
#endif
        
        EXPECT_TRUE(run_time_host_endianness::is_big_endian() == host_endianness::is_big_endian);
        EXPECT_TRUE(run_time_host_endianness::is_little_endian() == host_endianness::is_little_endian);

        EXPECT_TRUE(run_time_host_endianness::is_big_endian() != run_time_host_endianness::is_little_endian());
        EXPECT_TRUE(host_endianness::is_big_endian != host_endianness::is_little_endian);
        
        //int16_t i16_little = byte_swap<host_endianness::type, little_endian>(i16);
        //int16_t i16_big = byte_swap<host_endianness::type, big_endian_tag>(i16);
        //int16_t i16_htons = htons(i16);
        
        /*        
         ntohl() //Network (big endian) to Host byte order (Long)
         htonl() //Host to Network byte order (big endian)  (Long)
         
         ntohs() //Network (big endian) to Host byte order (Short)
         htons() //Host to Network byte order (big endian)  (Short)
         */
        
        //int32_t a = __builtin_bswap32 (0x12345678);
        //int64_t __builtin_bswap64 (int64_t x);
        
        //int16_t i16_darwin_swap = __DARWIN_OSSwapInt16(i16);
        
        /*        
         cout.fill('0');
         cout << "big:    " << hex << setw(sizeof(i16_big)*2) << i16_big << endl;
         cout << "htons:  " << hex << setw(sizeof(i16_htons)*2) << i16_htons << endl;
         cout << "darwin: " << hex << setw(sizeof(i16_htons)*2) << i16_darwin_swap << endl;
         */        
        
        
    }
    
    TEST_F(ByteSwapTest, BasicByteSwap) 
    {
        
        int16_t i16 = 0x1234;
        
        EXPECT_EQ( 0x1234, (byte_swap<big_endian_tag, big_endian_tag>(i16)) );
        EXPECT_EQ( 0x1234, (byte_swap<little_endian_tag, little_endian_tag>(i16)) );
        EXPECT_EQ( 0x3412, (byte_swap<big_endian_tag, little_endian_tag>(i16)) );
        EXPECT_EQ( 0x3412, (byte_swap<little_endian_tag, big_endian_tag>(i16)) );
        
        
        int16_t ni16 = 0x8234;
        
        EXPECT_EQ( (int16_t)0x8234, (byte_swap<big_endian_tag, big_endian_tag>(ni16)) );
        EXPECT_EQ( (int16_t)0x8234, (byte_swap<little_endian_tag, little_endian_tag>(ni16)) );
        EXPECT_EQ( (int16_t)0x3482, (byte_swap<big_endian_tag, little_endian_tag>(ni16)) );
        EXPECT_EQ( (int16_t)0x3482, (byte_swap<little_endian_tag, big_endian_tag>(ni16)) );
        
        
        int32_t i32 = 0x12345678;
        
        EXPECT_EQ( (int32_t)0x12345678, (byte_swap<big_endian_tag, big_endian_tag>(i32)) );
        EXPECT_EQ( (int32_t)0x12345678, (byte_swap<little_endian_tag, little_endian_tag>(i32)) );
        EXPECT_EQ( (int32_t)0x78563412, (byte_swap<big_endian_tag, little_endian_tag>(i32)) );
        EXPECT_EQ( (int32_t)0x78563412, (byte_swap<little_endian_tag, big_endian_tag>(i32)) );
        
        int32_t ni32 = 0x82345678;
        
        EXPECT_EQ( (int32_t)0x82345678, (byte_swap<big_endian_tag, big_endian_tag>(ni32)) );
        EXPECT_EQ( (int32_t)0x82345678, (byte_swap<little_endian_tag, little_endian_tag>(ni32)) );
        EXPECT_EQ( (int32_t)0x78563482, (byte_swap<big_endian_tag, little_endian_tag>(ni32)) );
        EXPECT_EQ( (int32_t)0x78563482, (byte_swap<little_endian_tag, big_endian_tag>(ni32)) );
        
    }
    
    
    
#if defined (DEBUG) 
    TEST_F(ByteSwapTest, DISABLED_BenchByteSwapUInt16) 
#else
    TEST_F(ByteSwapTest, BenchByteSwapUInt16) 
#endif    
    {        
        using utilities::timer;
        
        // no conversions applied:
        timer t;
        t.start();
        volatile uint16_t sum = 0;
        for (int i = 0; i<1000000; ++i) {
            sum += uint16_t(i);
        }        
        t.stop();
        double noconv = t.seconds();
        printf("BenchByteSwap: elapsed time sum:          %.3f µs\n", noconv*1e6);
        
        t.reset();
        t.start();
        sum = 0;
        for (int i = 0; i<1000000; ++i) {
            sum += byte_swap<big_endian_tag, little_endian_tag>(uint16_t(i));
        }
        t.stop();
        
        printf("BenchByteSwap: elapsed time swap and sum: %.3f µs\n", t.seconds()*1e6);
        double deviation = t.seconds() / noconv;
        
        EXPECT_LT( deviation, 2.0 );
        
    }
    
#if defined (DEBUG)
    TEST_F(ByteSwapTest, DISABLED_BenchByteSwapUInt32) 
#else
    TEST_F(ByteSwapTest, BenchByteSwapUInt32) 
#endif    
    {
        using utilities::timer;
        
        // no conversions applied:
        timer t;
        t.start();
        volatile uint32_t sum = 0;
        for (uint32_t i = 0; i<1000000; ++i) {
            sum += i;
        }        
        t.stop();
        double noconv = t.seconds();
        printf("BenchByteSwap: elapsed time sum:          %.3f µs\n", noconv*1e6);
        
        t.reset();
        t.start();
        sum = 0;
        for (uint32_t i = 0; i<1000000; ++i) {
            sum += byte_swap<big_endian_tag, little_endian_tag>(i);
        }
        t.stop();
        
        printf("BenchByteSwap: elapsed time swap and sum: %.3f µs\n", t.seconds()*1e6);
        double deviation = t.seconds() / noconv;
        
        EXPECT_LT( deviation, 2.0 );
        
    }
    
    
#if defined (DEBUG)    
    TEST_F(ByteSwapTest, DISABLED_BenchByteSwapUInt64) 
#else    
    TEST_F(ByteSwapTest, BenchByteSwapUInt64) 
#endif    
    {
        using utilities::timer;
        
        // no conversions applied:
        timer t;
        t.start();
        volatile uint64_t sum = 0;
        for (uint64_t i = 0; i<1000000; ++i) {
            sum += i;
        }        
        t.stop();
        double noconv = t.seconds();
        printf("BenchByteSwap: elapsed time sum:          %.3f µs\n", noconv*1e6);
        
        t.reset();
        t.start();
        sum = 0;
        for (uint64_t i = 0; i<1000000; ++i) {
            sum += byte_swap<big_endian_tag, little_endian_tag>(i);
        }
        t.stop();
        
        printf("BenchByteSwap: elapsed time swap and sum: %.3f µs\n", t.seconds()*1e6);
        double deviation = t.seconds() / noconv;
        
        EXPECT_LT( deviation, 2.0 );
        
    }
    
    
}  // namespace



namespace {


    using namespace json;
    
    
    template <typename T>
    class ByteSwapWithEncodingTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        ByteSwapWithEncodingTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~ByteSwapWithEncodingTest() {
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
    
    
    TYPED_TEST_CASE_P(ByteSwapWithEncodingTest);
    
    
    TYPED_TEST_P(ByteSwapWithEncodingTest, SwapByteWithEncoding) 
    {
        // This test merely shows how to retrieve the endianness from an 
        // Unicode scheme/form and how to use it in the swap_byte() function.
        //
        // Function byte_swap() accepts two kinds of parameter types, which
        // are endianness types:        
        //  - internal::big_endian_tag
        //  - internal::little_endian_tag
        //  where 'endian_tag' is the common base class of 'big_endian_tag' and
        //  'little_endian_tag'.
        //
        // For all Unicode encodings, retrieve the endianness, and
        // try a byte swap. This test merely shows how to retrieve the 
        // endianness from an Unicode scheme/form:
        
        using unicode::encoding_traits;
        using unicode::add_endianness;
        using internal::big_endian_tag;
        using internal::little_endian_tag;
        using internal::endian_tag;
        
        // Inside a test, refer to TypeParam to get the type parameter.        
        // 'TypeParam' is our Unicode encoding scheme/form:
        typedef TypeParam encoding_t;

        
        // Retrieve the endianness:
        // Note: if we don't have an "Unicode scheme", but rather an "Unicode encoding
        // form" (which doesn't inlcude endianness tag) the type 'endian_tag'
        // is not defined. Thus, we "add" host endianness to encoding forms
        // (encoding schemes will not be modified).
        typedef typename add_endianness<encoding_t>::type encoding_scheme_t;
        typedef typename encoding_traits<encoding_scheme_t>::endian_tag endianness_t;
        
        
        BOOST_STATIC_ASSERT( (boost::mpl::or_<
                                boost::is_same<internal::big_endian_tag, endianness_t>,
                                boost::is_same<internal::little_endian_tag, endianness_t>
                              >::value == true) );
        

        // Get the type of the code unit:
        typedef typename encoding_traits<encoding_t>::code_unit_type char_t;
        
        char_t result;
        result = byte_swap<endianness_t, big_endian_tag>(0);
        result = byte_swap<endianness_t, little_endian_tag>(0);
        
    }
    
    
    
    // Register
    REGISTER_TYPED_TEST_CASE_P(ByteSwapWithEncodingTest, SwapByteWithEncoding);
    
    // Instantiate test cases:
    typedef ::testing::Types<
    unicode::UTF_8_encoding_tag, 
    unicode::UTF_16_encoding_tag,
    unicode::UTF_16BE_encoding_tag,
    unicode::UTF_16LE_encoding_tag,
    unicode::UTF_32_encoding_tag,
    unicode::UTF_32BE_encoding_tag,
    unicode::UTF_32LE_encoding_tag
    >  UTF_encodings;
    
    
    INSTANTIATE_TYPED_TEST_CASE_P(SwapByteWithEncodingTests, ByteSwapWithEncodingTest, UTF_encodings);
    
    
}

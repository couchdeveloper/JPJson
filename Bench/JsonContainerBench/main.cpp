//
//  main.cpp
//  JsonContainerBench
//
//  Created by Andreas Grosam on 4/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <boost/config.hpp>
#include "utilities/bench.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sstream>

#include "json/value/value.hpp"


// Feature checking macros enable or disbale certain tests

#ifndef __has_feature           // Optional of course.
    #define __has_feature(x) 0      // Compatibility with non-clang compilers.
#endif



using std::cout;
using std::endl;
using std::pair;

typedef json::value<>   Value;
typedef Value::array_type  Array;
typedef Value::object_type Object;
typedef Value::string_type String;

using std::chrono::duration_cast;
using std::chrono::microseconds;


namespace {

    void shuffleString(String& s)
    {
        std::string str = s.c_str();
        std::random_shuffle(str.begin(), str.end());
        s = str.c_str();
    }
    
}


namespace test {
    
    
    struct benchObjectEmplace : bench_base<benchObjectEmplace>
    {
        typedef bench_base<benchObjectEmplace> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        
        benchObjectEmplace(size_t countObjects)
        : N_(countObjects)
        {}
        
        typedef json::value<>   Value;
        typedef Value::array_type  Array;
        typedef Value::object_type Object;
        typedef Value::string_type String;
        
        void prepare_imp()
        {
            cout << "--- Bench Inserting Values (containing a String) into an Object with random keys ---\n";
            String key = "0123456789ABCDEF";
            std::vector<String> keys(N_, key);
            std::for_each(keys.begin(), keys.end(), shuffleString);
            keys_ = std::move(keys);            
        }
        
        duration bench_imp()
        {
            Value root( (Object()) );
            Object& o = root.as<Object>();
            Value value = "This is the value";
            timer timer;
            auto t0 = timer.now();
            for (int i = 0; i < N_; ++i) {
                o.emplace(keys_[i], value);
            }
            return timer.now() - t0;
        }
        
        void teardown_imp() {};
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for inserting " << N_ << " objects:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        
    private:
        std::size_t N_;
        std::vector<String> keys_;
    };
    
    struct benchArrayEmplaceBack : bench_base<benchArrayEmplaceBack>
    {
        typedef bench_base<benchArrayEmplaceBack> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        
        benchArrayEmplaceBack(size_t countObjects)
        : N_(countObjects)
        {}
        
        typedef json::value<>   Value;
        typedef Value::array_type  Array;
        typedef Value::object_type Object;
        typedef Value::string_type String;
        
        void prepare_imp()
        {
            cout << "--- Bench Inserting Values (containing a String) into an Array ---\n";
        }
        
        duration bench_imp()
        {
            // Insert N Objects into an Array
            Value root = Array();
            Array& a = root.as<Array>();
            Value value = "This is the value";
            timer timer;
            auto t0 = timer.now();
            for (int i = 0; i < N_; ++i) {
                a.emplace_back(value);
            }
            return timer.now() - t0;
        }
        
        void teardown_imp() {};
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for inserting " << N_ << " objects:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
    private:
        std::size_t N_;
    };
    
    struct benchTestStdMapEmplace : bench_base<benchTestStdMapEmplace>
    {
        typedef bench_base<benchTestStdMapEmplace> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        benchTestStdMapEmplace(size_t countObjects)
        : N_(countObjects)
        {}
                
        void prepare_imp() {
            cout << "--- Bench a std::map<std::string, std::string>'s emplace() with random keys ---\n";
            String key = "0123456789ABCDEF";
            std::vector<String> keys(N_, key);
            std::for_each(keys.begin(), keys.end(), shuffleString);
            keys_ = std::move(keys);
        }
        
        duration bench_imp()
        {
            std::string value = "This is the value";
            typedef std::map<std::string, std::string> map_t;
            map_t map;
            timer timer;
            auto t0 = timer.now();
            for (int i = 0; i < N_; ++i) {
                map.emplace(keys_[i], value);
            }
            return timer.now() - t0;
        }
        
        void teardown_imp() {};

        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for inserting " << N_ << " objects:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }        
        
    private:
        std::size_t N_;
        std::vector<String> keys_;
    };
    
    struct benchTestStdVectorEmplaceBack : bench_base<benchTestStdVectorEmplaceBack>
    {
        typedef bench_base<benchTestStdVectorEmplaceBack> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        

        benchTestStdVectorEmplaceBack(size_t countObjects)
        : N_(countObjects)
        {}
        
        void prepare_imp() {
            cout << "--- Bench a std::vector<std::string>'s emplace_back() ---\n";
        }
        
        duration bench_imp()
        {
            std::string value = "This is the value";
            std::vector<std::string> v1;
            timer timer;
            auto t0 = timer.now();
            for (int i = 0; i < N_; ++i) {
                v1.emplace_back(value);
            }
            return timer.now() - t0;
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for inserting " << N_ << " objects:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        void teardown_imp() {};
        
    private:
        std::size_t N_;
    };
    

    void testCompareObjectVsStdMap(const size_t N = 1)
    {
        // Insert N strings into a std::map
        benchTestStdMapEmplace b1(N);
        b1.run();
        
        // Insert N Values(String) into an Object
        benchObjectEmplace b2(N);
        b2.run();
    }
    
    void testCompareArrayVsStdVector(const size_t N = 1)
    {
        // Insert N Values(String) into an Array
        benchTestStdVectorEmplaceBack b1(N);
        b1.run();
        
        benchArrayEmplaceBack b2(N);
        b2.run();
        
    }
    

    struct benchTestCopyStdMap : bench_base<benchTestCopyStdMap>
    {
        typedef bench_base<benchTestCopyStdMap> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        typedef std::map<std::string, std::string> map_t;

        benchTestCopyStdMap(size_t countObjects)
        : N_(countObjects)
        {}
        
        void prepare_imp() {
            cout << "--- Bench a std::map<std::string, std::string>'s emplace() with random keys ---\n";
            String key = "0123456789ABCDEF";
            std::vector<String> keys(N_, key);
            std::for_each(keys.begin(), keys.end(), shuffleString);
            keys_ = std::move(keys);
            std::string value = "This is the value";
            for (int i = 0; i < N_; ++i) {
                map_.emplace(keys_[i], value);
            }
        }
        
        duration bench_imp()
        {
            map_t map;
            timer timer;
            auto t0 = timer.now();
            map = map_;
            return timer.now() - t0;
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for copying std::map<std::string, std::string> with " << N_ << " elements:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        void teardown_imp() {};
        
    private:
        std::size_t N_;
        std::vector<String> keys_;
        map_t map_;
    };


    struct benchTestCopyStdVector : bench_base<benchTestCopyStdVector>
    {
        typedef bench_base<benchTestCopyStdVector> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        typedef std::vector<std::string> vector_t;
        
        benchTestCopyStdVector(size_t countObjects)
        : N_(countObjects)
        {}
        
        void prepare_imp() {
            cout << "--- Bench a std::vector<std::string>'s emplace_back() ---\n";
            std::string value = "This is the value";
            for (int i = 0; i < N_; ++i) {
                vec_.emplace_back(value);
            }
        }
        
        duration bench_imp()
        {
            vector_t vec;
            timer timer;
            auto t0 = timer.now();
            vec = vec_;
            return timer.now() - t0;
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for copying std::vector<std::string> with " << N_ << " elements:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        void teardown_imp() {};
        
    private:
        std::size_t N_;
        vector_t vec_;
    };
    
    
    struct benchCopyArray : bench_base<benchCopyArray>
    {
        typedef bench_base<benchCopyArray> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        
        benchCopyArray(size_t countObjects)
        : N_(countObjects)
        {}
        
        typedef json::value<>   Value;
        typedef Value::array_type  Array;
        typedef Value::object_type Object;
        typedef Value::string_type String;
        
        void prepare_imp()
        {
            cout << "--- Bench Copy Array ---\n";
            // Insert N Objects into an Array
            Value value = "This is the value";
            for (int i = 0; i < N_; ++i) {
                array_.emplace_back(value);
            }
        }
        
        duration bench_imp()
        {
            // Copy Array
            Value root = Array();
            Array& a = root.as<Array>();
            Value value = "This is the value";
            timer timer;
            auto t0 = timer.now();
            a = array_;
            return timer.now() - t0;
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for copying Array with " << N_ << " values:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        void teardown_imp() {};
        
    private:
        std::size_t N_;
        Array array_;
    };
    

    void testBenchCopyArray(const size_t N = 1)
    {
        cout << "=== Bench Compare Copy Array vs Copy std::vector ---\n";
        benchTestCopyStdVector b0(N);
        b0.run();
        
        benchCopyArray b1(N);
        b1.run();
    }
    
    
    
    struct benchCopyObject : bench_base<benchCopyObject>
    {
        typedef bench_base<benchCopyObject> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        
        benchCopyObject(size_t countObjects)
        : N_(countObjects)
        {}
        
        typedef json::value<>   Value;
        typedef Value::array_type  Array;
        typedef Value::object_type Object;
        typedef Value::string_type String;
        
        void prepare_imp()
        {
            cout << "--- Bench Copy Object  ---\n";
            String key = "0123456789ABCDEF";
            std::vector<String> keys(N_, key);
            std::for_each(keys.begin(), keys.end(), shuffleString);
            keys_ = std::move(keys);
            Value value = "This is the value";
            for (int i = 0; i < N_; ++i) {
                object_.emplace(keys_[i], value);
            }
        }
        
        duration bench_imp()
        {
            Value root = Object();
            Object& o = root.as<Object>();
            timer timer;
            auto t0 = timer.now();
            o = object_;
            return timer.now() - t0;
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for copying Object with " << N_ << " values:\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        void teardown_imp() {};
        
        
    private:
        std::size_t N_;
        std::vector<String> keys_;
        Object object_;
    };
    
    
    void testBenchCopyObject(const size_t N = 1)
    {
        cout << "=== Bench Compare Copy Object vs Copy std::map ---\n";
        benchTestCopyStdMap b0(N);
        b0.run();
        
        benchCopyObject b1(N);
        b1.run();
    }
    
    
    
    

//void createSimpleArray()
//{
//    using namespace json;
//    using namespace utilities;
//    
//    cout << "\n--- Create Simple Json Array ---\n" << endl;
//    
//    const int N = 10000;
//    Array a = Array();
//    
//    a.reserve(N);
//    
//    // Insert N Nulls:
//    timer t;
//    t.start();    
//    for (int i = 0; i < N; ++i) {
//        a.emplace_back(json::null);
//    }
//    t.pause();    
//    printf("Creating an Array and inserting %d Values as Nulls:  %.3f µs\n", N, t.seconds()*1e6);
//    
//    t.reset();
//    t.start();
//    Array a_copy = a;
//    t.stop();
//    printf("Creating a copy of this array:  %.3f µs\n", t.seconds()*1e6);
//    
//    t.reset();
//    Array a2;
//    t.start();
//    std::swap(a2, a);
//    t.stop();
//    printf("Creating a copy of this array using std::swap:  %.3f µs\n", t.seconds()*1e6);
//    
//    t.reset();
//    a = a_copy;
//    Array a3;
//    t.start();
//    swap(a3, a);
//    t.stop();
//    printf("Creating a copy of this array using swap:  %.3f µs\n", t.seconds()*1e6);
//    
//
//#if !defined (BOOST_NO_RVALUE_REFERENCES)
//    a = a_copy;
//    t.reset();
//    t.start();
//    Array a4(std::move(a));
//    t.stop();
//    printf("Creating a copy of this array using move semantics:  %.3f µs\n", t.seconds()*1e6);
//#endif 
//    cout << endl;
//}

void testArraySwap() 
{
    using namespace json;
    
    cout << "--- Test Array Swap ----\n" << endl;
    
    
    Value v( (Array()) );
    v.as<Array>().emplace_back("this is string 1");
    v.as<Array>().emplace_back("this is string 2");
    v.as<Array>().emplace_back("this is string 3");
    v.as<Array>().emplace_back("this is string 4");
    v.as<Array>().emplace_back("this is string 5");
    
    Value v_copy = v;
    
    Value v2;
    swap(v2, v);
    
    // TODO:
    //std::cout << "This Value shall be null: " << v << std::endl;
    //std::cout << "This Value shall be an Array containing 6 Strings:\n" << v2 << std::endl;
}

//void 
//createComplexJsonTree(bool print = false)
//{    
//    using namespace json;
//    using namespace utilities;
//    
//    cout << "\n--- Create Complex Json Tree ----\n" << endl;
//    
//    Value v = Array();
//    
//    Object o;
//    o.emplace("key0", "Value 0");
//    o.emplace("key1", "Value 1");
//    o.emplace("key2", true);
//    o.emplace("key3", false);
//    o.emplace("key4", null);
//    o.emplace("key5", 1.0);
//    o.emplace("key6", Object());
//    o.emplace("key7", Array());
//    
//    v.as<Array>().push_back(o);
//    v.as<Array>().push_back(o);
//    v.as<Array>().push_back(o);
//    v.as<Array>().push_back(v);
//    v.as<Array>().push_back(v);
//    
//    timer t;
//    t.start();    
//    for (int i = 0; i < 5; ++i) {
//        v.as<Array>().emplace_back(v);
//    }
//    t.pause();
//    
//    if (print) {
//        v.print(cout);
//        cout << endl;    
//    }
//    printf("time for creating a complex json tree:  %.3f µs\n", t.seconds()*1e6);
//
//    
//    t.reset();
//    t.start();    
//    Value copy = v;
//    t.pause();
//    printf("time for copying a complex json tree:  %.3f µs\n", t.seconds()*1e6);
//        
//    t.reset();
//    t.start();
//    Value v2 = Array();
//    swap(v2, v);
//    t.pause();
//    printf("time for copying a complex json tree with swap using equal types:  %.3f µs\n", t.seconds()*1e6);
//    v2 = null;
//    v = copy;
//
//    
//    // Requires C++0x rvalue references conforming std lib    
//#if !defined (BOOST_NO_RVALUE_REFERENCES)
//    t.reset();
//    t.start();
//    Value v3 = std::move(v);
//    t.pause();
//    printf("time for copying a complex json tree with move semantics:  %.3f µs\n", t.seconds()*1e6);
//    v3 = null;
//#else
//    cout << "INFO: bench for copying a complex json tree with move semantics has been skipped since the compiler is not supporting cxx_rvalue_references" << endl;
//#endif    
//    cout << endl;
//}


//void testMoveArray() 
//{
//#if !defined (BOOST_NO_RVALUE_REFERENCES)
//    
//    using namespace utilities;
//    using namespace json;
//    
//    std::cout << "\n--- Test Move Semantics for Array ---\n";
//    
//    Array a;
//    a.push_back(Value("abcd"));
//    a.push_back(Value(1.0));
//    a.push_back(Value(false));
//    a.push_back(Value(null));
//    
//    Array safedCopy = a;
//        
//    Array a2(std::move(a));
//    
//    bool isSame = a2 == safedCopy;
//    if (not isSame)
//        std::cout << "ERROR: Move semantics yields unequal Array" << std::endl;
//    
//    // original array a shall be empty:
//    std::cout << "Move semantics for Array: " << ((a.size() > 0)? "FAILED" : "PASSED") << std::endl;
//    
//#else
//    cout << "\nINFO: Test \"testMoveArray\" has been skipped since the compiler is not supporting cxx_rvalue_references\n" << endl;
//#endif
//}

//void testMoveSemantics() 
//{
//// Requires C++0x rvalue references conforming std lib    
//#if !defined (BOOST_NO_RVALUE_REFERENCES)
//    
//    using namespace utilities;
//    
//    std::cout << "\n--- Test Move Semantics ---\n";
//    
//    const int N = 10000;
//    std::vector<std::string> v;
//    for (int i = 0; i < N; ++i) {
//        v.push_back("This is a string");
//    }
//    
//    timer t;
//    t.start();
//    std::vector<std::string> v_copy(v);
//    t.stop();
//    printf("time for copying a std::vector<std::string> of %d strings:  %.3f µs\n", N, t.seconds()*1e6);
//    
//    t.reset();
//    t.start();
//    std::vector<std::string> v2(std::move(v));
//    t.stop();    
//    
//    printf("time for copying a std::vector<std::string> with move semantics:  %.3f µs\n", t.seconds()*1e6);
//    cout << "new container size = " << v2.size() << endl;
//    cout << "original (moved) container size: " << v.size()<< endl;
//    
//#else
//    cout << "INFO: Test \"testMoveSemantics\" has been skipped since the compiler is not supporting cxx_rvalue_references" << endl;
//#endif
//}

//void benchCompareArrayCtorVsValueWithArrayCtor()
//{
//    // create a number of Array instances and compare runtime with a
//    // the "pure" std:vector.
//    
//    using namespace utilities;
//    using namespace json;
//    
//    std::cout << "\n--- Bench Compare Array() ctor versus Value(Array) ctor ---\n";
//    // note: if rvalue reference is available and std lib has moveaware containers, the timings should be faster
//
//    const int N = 100000;
//
//    timer t;
//
//    t.start();
//    Array a;
//    std::vector<Array> v1(N, a);
//    t.stop();
//    printf("time for creating %d instances of Array:  %.3f µs\n", N, t.seconds()*1e6);
//    
//    
//    t.reset();
//    t.start();
//    Value v = Array();
//    std::vector<Value> v2(N, v);
//    t.stop();
//    printf("time for creating %d instances of Value(Array)>:  %.3f µs\n", N, t.seconds()*1e6);
//}


//void benchValueSwap()
//{
//    using namespace utilities;
//    using namespace json;
//    
//    std::cout << "\n--- Bench Value swap() With Different Types ---\n";
//    
//    // create a reasonable heavy array:
//    Value v = Array();
//    Array& a = v.as<Array>();
//    const int N = 10000;
//    for (int i = 0; i < N; ++i) {
//        a.push_back("This is a string");
//    }
//    
//    // create a copy of the Value v:
//    timer t;
//    t.start();
//    Value  v_copy(v);
//    t.stop();
//    printf("time for copying a Value whose current type is an Array with size %d:  %.3f µs\n", N, t.seconds()*1e6);
//    
//    t.reset();
//    t.start();
//    // create a default Value, which current type is a Null. 
//    
//    // If the current types of the Values of the args of swap(Value, Value) do not match, this will cause a private swap()
//    // to be called. On c++0x this uses move semantics, otherwise this is an ordinary swap() algorithm using
//    // a temp object which needs to be copied. Here, a default Value will be constructed which current type is a Null. 
//    // Hence, for non-c++0x the following swap() becomes rather slow.
//    Value v2;     
//    swap(v2, v);
//    t.stop();    
//    
//    printf("time for swap(Value, Value) with different types (Null, Array):  %.3f µs\n", t.seconds()*1e6);
//    std::cout << "Note: on c++0x conforming compilers this swap() is expected to be constant time and nearly zero.\n"
//                 "For non-c++0x this is about 3x copy.\n";
//    
//    
//    t.reset();
//    v = v_copy;
//    t.start();
//    // create a Value, which current type is an empty Array.
//    Value v3 = Array();
//    // Since the values which are swapped do now have the same type, this will cause the custom Array::swap(Value)
//    // to be called, which is constant time and fast.
//    swap(v3, v);
//    t.stop();    
//    printf("time for swap(Value, Value) with equals types (Array):  %.3f µs\n", t.seconds()*1e6);
//    std::cout << "Note: this swap() is expected to be constant time and nearly zero.\n" << std::endl;
//}

} // namespace test

int main (int argc, const char * argv[])
{
    using namespace std;
    using namespace json;
    cout << "\n\n*** Json Container Bench ***\n\n" << endl;
    
    cout << "\n=== Compare Array vs std::vector ===\n";
    //test::testCompareArrayVsStdVector(2);
    //test::testCompareArrayVsStdVector(5);
    test::testCompareArrayVsStdVector(10);
    //test::testCompareArrayVsStdVector(20);
    //test::testCompareArrayVsStdVector(50);
    test::testCompareArrayVsStdVector(100);
    //test::testCompareArrayVsStdVector(200);
    //test::testCompareArrayVsStdVector(500);
    test::testCompareArrayVsStdVector(1000);
    //test::testCompareArrayVsStdVector(2000);
    //test::testCompareArrayVsStdVector(5000);
    test::testCompareArrayVsStdVector(10000);
    

    cout << "\n=== Compare Object vs std::map ===\n";
    //test::testCompareObjectVsStdMap(2);
    //test::testCompareObjectVsStdMap(5);
    test::testCompareObjectVsStdMap(10);
    //test::testCompareObjectVsStdMap(20);
    //test::testCompareObjectVsStdMap(50);
    test::testCompareObjectVsStdMap(100);
    //test::testCompareObjectVsStdMap(200);
    //test::testCompareObjectVsStdMap(500);
    test::testCompareObjectVsStdMap(1000);
    //test::testCompareObjectVsStdMap(2000);
    //test::testCompareObjectVsStdMap(5000);
    test::testCompareObjectVsStdMap(10000);
    
    
    cout << "\n=== Copy Array ===\n";
    test::testBenchCopyArray(10);
    test::testBenchCopyArray(100);
    test::testBenchCopyArray(1000);
    test::testBenchCopyArray(10000);

    cout << "\n=== Copy Object ===\n";
    test::testBenchCopyObject(10);
    test::testBenchCopyObject(100);
    test::testBenchCopyObject(1000);
    test::testBenchCopyObject(10000);
    
    
//    testArraySwap();
//    benchValueSwap();
//    testMoveSemantics();
//    testMoveArray();
//    benchCompareArrayCtorVsValueWithArrayCtor();
//    createSimpleArray();
//    createComplexJsonTree();
    
/*    
    printf("%.3f\n\n", 1/3.00);

    Value json;
    json.print(cout);        
    cout << endl;
    
    json = Object();
    json.print(cout);        
    cout << endl;
    
    Object& object = json.as<Object>();
    object.insert("name", "Siri");
    object.insert("friend", "Andreas");
    object.insert("Euro", Number(1/3.0));

    json.print(cout, true);        
    cout << endl;
    
    testCompareObjectAndMap(100, true);
 
    
    Value v = createComplexJsonTree(true);
    
 */    

/*
    testInsertIntoObject(10, true);
    testInsertIntoArray(10, true);
    testObjectsIntoArray(10, true);
*/
    return 0;
}


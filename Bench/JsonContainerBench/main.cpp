//
//  main.cpp
//  JsonContainerBench
//
//  Created by Andreas Grosam on 4/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <algorithm>

#include <utilities/timer.hpp>
#include <json/value/value.hpp>


#if defined (BOOST_STRICT_CONFIG)
#warning BOOST_STRICT_CONFIG defined
#endif

// Feature checking macros enable or disbale certain tests

#ifndef __has_feature           // Optional of course.
    #define __has_feature(x) 0      // Compatibility with non-clang compilers.
#endif



using std::cout;
using std::endl;
using std::pair;

typedef json::value<>   Value;
typedef Value::array_t  Array;
typedef Value::object_t Object;
typedef Value::string_t String;

static void shuffleString(String& s)
{
    std::string str = s.c_str();
    std::random_shuffle(str.begin(), str.end());
    s = str.c_str();
}


void testInsertIntoObject(const size_t N = 1, bool print = false) 
{    
    using namespace json;
    using namespace utilities;
    
    cout << "--- testInsertIntoObject ---\n";
    
    
    typedef std::pair<Object::iterator, bool> result_t;  // result type of member function insert

    String key = "0123456789ABCDEF";    
        
    Value root( (Object()) );
    Object& o = root.as<Object>();
    Value value = "This is the value";
    std::vector<String> keys(N, key);
    std::for_each(keys.begin(), keys.end(), shuffleString);
    
    timer t;
    t.start();
    for (int i = 0; i < N; ++i) {
        result_t result = o.insert(keys[i], value);
    }
    t.pause();
    
    if (print) {
        root.print(std::cout, true);
        std::cout << endl;
    }

    printf("time for one insertion into Object:  %.3f µs\n", t.seconds()*1e6/(N));
} 


void testCompareObjectAndMap(const size_t N = 1, bool print = false) 
{
    using namespace json;
    using namespace utilities;
    using namespace std;
    
    cout << "--- testCompareObjectAndMap ---\n";
    
    
    // Bench the native approach, using a std::vector as container and a std::map as value
    typedef std::map<std::string, Null> map_t;
    std::vector<map_t> v1;
    map_t map;
    timer t2;
    t2.start();
    for (int i = 0; i < N; ++i) {
        v1.push_back(map);
    }
    t2.pause();
    printf("time for %lu insertions of empty map<std::string, Null> into vector:  %.3f µs\n", N, t2.seconds()*1e6);
    
    
    // Insert N Objects into an Array
    Value root = Array();
    Array& a = root.as<Array>();
    Value value = Object();
    
    timer t1;
    t1.start();
    for (int i = 0; i < N; ++i) {
        a.push_back(value);
    }
    t1.pause();
    printf("time for %lu insertions of empty Objects into Array:  %.3f µs\n", N, t1.seconds()*1e6);

        
    std::vector<Value> v2;
    Value object = Object();
    timer t3;
    t3.start();
    for (int i = 0; i < N; ++i) {
        v2.push_back(object);
    }
    t3.pause();
    printf("time for %lu insertions of empty Objects into vector:  %.3f µs\n", N, t3.seconds()*1e6);
 
    
    
    // Bench the native approach, using a std::vector as container and a std::map as value
    std::vector<map_t> v10;
    map.insert(std::pair<std::string, json::Null>("key", json::null));
    timer t20;
    t20.start();
    for (int i = 0; i < N; ++i) {
        v10.push_back(map);
    }
    t20.pause();
    printf("time for %lu insertions of non-empty map<std::string, Null> into vector:  %.3f µs\n", N, t20.seconds()*1e6);
    
    
    // Insert N Objects into an Array
    Value root2 = Array();
    Array& a2 = root.as<Array>();
    Value value2 = Object();
    value2.as<Object>().insert("key", Value(json::null));
    
    timer t10;
    t10.start();
    for (int i = 0; i < N; ++i) {
        a2.push_back(value2);
    }
    t10.pause();
    printf("time for %lu insertions of non-empty Objects ({\"key\":null}) into Array:  %.3f µs\n", N, t10.seconds()*1e6);
    
    
    std::vector<Value> v20;
    Value object2 = Object();
    object2.as<Object>().insert("key", Value(json::null));
    timer t30;
    t30.start();
    for (int i = 0; i < N; ++i) {
        v20.push_back(object2);
    }
    t30.pause();
    printf("time for %lu insertions of non-empty Objects ({\"key\":null}) into vector:  %.3f µs\n", N, t30.seconds()*1e6);
    
}

void testInsertIntoArray(const size_t N = 1, bool print = false) 
{    
    using namespace json;
    using namespace utilities;
    
    cout << "--- Create Array and push_back std::strings into it ---\n";

    
    typedef std::pair<Object::iterator, bool> result_t;  // result type of member function insert
    
    String s = "0123456789ABCDEF";    
    
    Value root( (Array()) );
    Array& a = root.as<Array>();
    std::vector<String> values(N, s);
    std::for_each(values.begin(), values.end(), shuffleString);
    
    // cheating:
    a.imp().reserve(N);
    
    timer t;
    t.start();
    for (int i = 0; i < N; ++i) {
        a.push_back(values[i]);
    }
    t.pause();
    
    if (print) {
        root.print(std::cout, true);
        std::cout << endl;
    }
    
    printf("time for one insertion into Array:  %.3f µs\n", t.seconds()*1e6/(N));
} 

void testObjectsIntoArray(const size_t N = 1, bool print = false)
{
    using namespace json;
    using namespace utilities;
    
    cout << "--- Create Objects and push_back them into Array---\n";
    
    Object o;
    o.insert(Object::element("key", "Value"));
    Value root = Array();
    Array& a = root.as<Array>();
    
    timer t;
    t.start();
    for (int i = 0; i < N; ++i) {
        a.push_back(o);
    }
    t.pause();
    
    if (print) {
        root.print(std::cout, true);
        std::cout << endl;
    }
    
    printf("time for one insertion into Array:  %.3f µs\n", t.seconds()*1e6/(N));
}



void createSimpleArray() 
{
    using namespace json;
    using namespace utilities;
    
    cout << "\n--- Create Simple Json Array ---\n" << endl;
    
    const int N = 10000;
    Array a = Array();
    
    // cheat:
    a.imp().reserve(N);
    
    // Insert N Nulls:
    timer t;
    t.start();    
    for (int i = 0; i < N; ++i) {
        a.push_back(json::null);
    }
    t.pause();    
    printf("Creating an Array and inserting %d Values as Nulls:  %.3f µs\n", N, t.seconds()*1e6);
    
    t.reset();
    t.start();
    Array a_copy = a;
    t.stop();
    printf("Creating a copy of this array:  %.3f µs\n", t.seconds()*1e6);
    
    t.reset();
    Array a2;
    t.start();
    std::swap(a2, a);
    t.stop();
    printf("Creating a copy of this array using std::swap:  %.3f µs\n", t.seconds()*1e6);
    
    t.reset();
    a = a_copy;
    Array a3;
    t.start();
    json::swap(a3, a);
    t.stop();
    printf("Creating a copy of this array using json::swap:  %.3f µs\n", t.seconds()*1e6);
    

#if !defined (BOOST_NO_RVALUE_REFERENCES)
    a = a_copy;
    t.reset();
    t.start();
    Array a4(std::move(a));
    t.stop();
    printf("Creating a copy of this array using move semantics:  %.3f µs\n", t.seconds()*1e6);
#endif 
    cout << endl;
}

void testArraySwap() 
{
    using namespace json;
    
    cout << "--- Test Array Swap ----\n" << endl;
    
    
    Value v( (Array()) );
    v.as<Array>().push_back(Value("this is string 1"));
    v.as<Array>().push_back(Value("this is string 2"));
    v.as<Array>().push_back(Value("this is string 3"));
    v.as<Array>().push_back(Value("this is string 4"));
    v.as<Array>().push_back(Value("this is string 5"));
    
    Value v_copy = v;
    
    Value v2;
    swap(v2, v);
    
    std::cout << "This Value shall be null: " << v << std::endl;
    std::cout << "This Value shall be an Array containing 6 Strings:\n" << v2 << std::endl;
}

void 
createComplexJsonTree(bool print = false)
{    
    using namespace json;
    using namespace utilities;
    
    cout << "\n--- Create Complex Json Tree ----\n" << endl;
    
    Value v = Array();
    
    Object o;
    o.imp().insert(pair<String, Value>("key0", "Value 0"));
    o.imp().insert(pair<String, Value>("key1", "Value 1"));
    o.imp().insert(pair<String, Value>("key2", true));
    o.imp().insert(pair<String, Value>("key3", false));
    o.imp().insert(pair<String, Value>("key4", null));
    o.imp().insert(pair<String, Value>("key5", Number("1.00")));
    o.imp().insert(pair<String, Value>("key6", Object()));
    o.imp().insert(pair<String, Value>("key7", Array()));
    
    v.as<Array>().push_back(o);
    v.as<Array>().push_back(o);
    v.as<Array>().push_back(o);
    v.as<Array>().push_back(v);
    v.as<Array>().push_back(v);
    
    timer t;
    t.start();    
    for (int i = 0; i < 5; ++i) {
        v.as<Array>().push_back(v);
    }
    t.pause();
    
    if (print) {
        v.print(cout, true);
        cout << endl;    
    }
    printf("time for creating a complex json tree:  %.3f µs\n", t.seconds()*1e6);

    
    t.reset();
    t.start();    
    Value copy = v;
    t.pause();
    printf("time for copying a complex json tree:  %.3f µs\n", t.seconds()*1e6);
        
    t.reset();
    t.start();
    Value v2 = Array();
    swap(v2, v);
    t.pause();
    printf("time for copying a complex json tree with swap using equal types:  %.3f µs\n", t.seconds()*1e6);
    v2 = null;
    v = copy;

    
    // Requires C++0x rvalue references conforming std lib    
#if !defined (BOOST_NO_RVALUE_REFERENCES)
    t.reset();
    t.start();
    Value v3 = std::move(v);
    t.pause();
    printf("time for copying a complex json tree with move semantics:  %.3f µs\n", t.seconds()*1e6);
    v3 = null;
#else
    cout << "INFO: bench for copying a complex json tree with move semantics has been skipped since the compiler is not supporting cxx_rvalue_references" << endl;
#endif    
    cout << endl;
}


void testMoveArray() 
{
#if !defined (BOOST_NO_RVALUE_REFERENCES)
    
    using namespace utilities;
    using namespace json;
    
    std::cout << "\n--- Test Move Semantics for Array ---\n";
    
    Array a;
    a.push_back(Value("abcd"));
    a.push_back(Value(1.0));
    a.push_back(Value(false));
    a.push_back(Value(null));
    
    Array safedCopy = a;
        
    Array a2(std::move(a));
    
    bool isSame = a2 == safedCopy;
    if (not isSame)
        std::cout << "ERROR: Move semantics yields unequal Array" << std::endl;
    
    // original array a shall be empty:
    std::cout << "Move semantics for Array: " << ((a.imp().size() > 0)? "FAILED" : "PASSED") << std::endl;
    
#else
    cout << "\nINFO: Test \"testMoveArray\" has been skipped since the compiler is not supporting cxx_rvalue_references\n" << endl;
#endif
}

void testMoveSemantics() 
{
// Requires C++0x rvalue references conforming std lib    
#if !defined (BOOST_NO_RVALUE_REFERENCES)
    
    using namespace utilities;
    
    std::cout << "\n--- Test Move Semantics ---\n";
    
    const int N = 10000;
    std::vector<std::string> v;
    for (int i = 0; i < N; ++i) {
        v.push_back("This is a string");
    }
    
    timer t;
    t.start();
    vector<string> v_copy(v);
    t.stop();
    printf("time for copying a std::vector<std::string> of %d strings:  %.3f µs\n", N, t.seconds()*1e6);
    
    t.reset();
    t.start();
    std::vector<std::string> v2(std::move(v));
    t.stop();    
    
    printf("time for copying a std::vector<std::string> with move semantics:  %.3f µs\n", t.seconds()*1e6);
    cout << "new container size = " << v2.size() << endl;
    cout << "original (moved) container size: " << v.size()<< endl;
    
#else
    cout << "INFO: Test \"testMoveSemantics\" has been skipped since the compiler is not supporting cxx_rvalue_references" << endl;
#endif
}

void benchCompareArrayCtorVsValueWithArrayCtor()
{
    // create a number of Array instances and compare runtime with a
    // the "pure" std:vector.
    
    using namespace utilities;
    using namespace json;
    
    std::cout << "\n--- Bench Compare Array() ctor versus Value(Array) ctor ---\n";
    // note: if rvalue reference is available and std lib has moveaware containers, the timings should be faster

    const int N = 100000;

    timer t;

    t.start();
    Array a;
    std::vector<Array> v1(N, a);
    t.stop();
    printf("time for creating %d instances of Array:  %.3f µs\n", N, t.seconds()*1e6);
    
    
    t.reset();
    t.start();
    Value v = Array();
    std::vector<Value> v2(N, v);
    t.stop();
    printf("time for creating %d instances of Value(Array)>:  %.3f µs\n", N, t.seconds()*1e6);
}


void benchValueSwap()
{
    using namespace utilities;
    using namespace json;
    
    std::cout << "\n--- Bench Value swap() With Different Types ---\n";
    
    // create a reasonable heavy array:
    Value v = Array();
    Array& a = v.as<Array>();
    const int N = 10000;
    for (int i = 0; i < N; ++i) {
        a.push_back("This is a string");
    }
    
    // create a copy of the Value v:
    timer t;
    t.start();
    Value  v_copy(v);
    t.stop();
    printf("time for copying a Value whose current type is an Array with size %d:  %.3f µs\n", N, t.seconds()*1e6);
    
    t.reset();
    t.start();
    // create a default Value, which current type is a Null. 
    
    // If the current types of the Values of the args of swap(Value, Value) do not match, this will cause a private swap()
    // to be called. On c++0x this uses move semantics, otherwise this is an ordinary swap() algorithm using
    // a temp object which needs to be copied. Here, a default Value will be constructed which current type is a Null. 
    // Hence, for non-c++0x the following swap() becomes rather slow.
    Value v2;     
    swap(v2, v);
    t.stop();    
    
    printf("time for swap(Value, Value) with different types (Null, Array):  %.3f µs\n", t.seconds()*1e6);
    std::cout << "Note: on c++0x conforming compilers this swap() is expected to be constant time and nearly zero.\n"
                 "For non-c++0x this is about 3x copy.\n";
    
    
    t.reset();
    v = v_copy;
    t.start();
    // create a Value, which current type is an empty Array.
    Value v3 = Array();
    // Since the values which are swapped do now have the same type, this will cause the custom Array::swap(Value)
    // to be called, which is constant time and fast.
    swap(v3, v);
    t.stop();    
    printf("time for swap(Value, Value) with equals types (Array):  %.3f µs\n", t.seconds()*1e6);
    std::cout << "Note: this swap() is expected to be constant time and nearly zero.\n" << std::endl;
}


int main (int argc, const char * argv[])
{
    using namespace std;
    using namespace json;
    using namespace utilities;
    cout << "\n\n=== Json Container Bench ===\n\n" << endl;
    
    testArraySwap();
    benchValueSwap();
    //testMoveSemantics();
    //testMoveArray();
    benchCompareArrayCtorVsValueWithArrayCtor();
    //createSimpleArray();
    createComplexJsonTree();
    
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


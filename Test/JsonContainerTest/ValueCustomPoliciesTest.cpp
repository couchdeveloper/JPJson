//
//  ValueCustomPoliciesTest.cpp
//
//  Created by Andreas Grosam on 5/7/11.
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


#include "json/value/value.hpp"
#include <gtest/gtest.h>

#include <cstdio>
#include <ctime>
#include <sstream>
#include <string>
#include <typeinfo>

#include <vector>
#include <map>
#include <unordered_map>
#include <scoped_allocator>
#include "json/utility/arena_allocator.hpp"

#include <tuple>


using namespace json;

// TODO: Obsolete description! - make a new one
// -----------------------------------------------------------------------------
// Defining a custom implementation for json::Object
//
// Custom implementation types for the array and object classes can be
// defined using json policies, which is a template argument for the
// template class value.
//
//  The default policies are defined in struct json_policies as follows:
// 
// namespace json {
//    struct json_policies 
//    {
//        typedef std::vector<boost::mpl::_> array_imp_t;
//        typedef std::map<boost::mpl::_, boost::mpl::_> object_imp_t;
//    };
// }
//
// json::object is just a container facade. It's underlaying container requires
// to be signature compatible with a std::map. Although, only a few signatures 
// are used. 
//
// The default implementation is a std::map. It can be exchanged with any
// other associative container, though. If the other container is std::map
// signature-compatible, the required changes are quite easy to apply.
//
// For any other none-std associative container this can be accomplished by a 
// suitable adapter quite easily.
//
// In order to change the underlaying container for json::object with a
// signature compatible associative container you just need to define your 
// own policy and pass it as a template argument to json::value:
//
//    namespace custom {
//        
//        class MyPolicies 
//        {
//            typedef std::vector<boost::mpl::_> array_imp_t;
//            typedef boost::unordered_map<boost::mpl::_, boost::mpl::_> object_imp_t;
//        };
//        
//    }
//
// The above example uses a boost::unordered_map as the underlaying container
// implementation for the json::object.
// json::value requires the policy to supply the type definitions array_imp_t and 
// object_imp_t as shown above.
//
// json:.value's type mechanerie ensures that json::value::array_type and 
// json::value::object_type are correctly calculated as a facade using the 
// specified underlaying container types. For convenience, the following 
// type definitions should be declared:
// 
//  typedef value<MyPolicies> Value;
//  typedef Value::array_type  Array;
//  typedef Value::object_type Object;
//  typedef Value::string_type String;
//
//
//
// For this test, we use a boost associative container, unordered_map,
// which is already signature compatible with std::map.
//
// Note:
// unordered_map requires a hash function for the key to be defined.
// For an Object, the key is type json::String, which is actually a typedef 
// for std::string. If the key is a std::string, a hash function is already
// defined. Otherwise, you must must be a hash function defined for String via 
// a custom overload:
//      std::size_t hash_value(const String& val);
//
// Defining Object with an unordered_map as the associative container type is
// now quite easy:


namespace test {

    template <class T>
    class custom_allocator
    {
    public:
        typedef T value_type;
        
        typedef std::true_type propagate_on_container_copy_assignment;
        typedef std::true_type propagate_on_container_move_assignment;
        typedef std::true_type propagate_on_container_swap;
        
        
        template <class U> struct rebind { typedef custom_allocator<U> other; };
        
        custom_allocator() = delete;  // custom allocator shall not be default constructible
        
        explicit custom_allocator(int id)
        : id_(id)
        {
        }
        
        custom_allocator(const custom_allocator& other)
        : id_(other.id_)
        {
        }
        
        template <class U>
        custom_allocator(const custom_allocator<U>& other) noexcept
        : id_(other.id_)
        {
        }
        
        int id() const { return id_; }
        
        
        T* allocate(std::size_t n)
        {
            //std::cout << "allocator[" << id_ << "] allocating " << n << "*" << sizeof(T) << " bytes" << std::endl;
            const std::size_t sz = n*sizeof(T);
            return reinterpret_cast<T*>( malloc(sz) );
        }
        
        void deallocate(T* p, std::size_t n) noexcept
        {
            //const std::size_t sz = n*sizeof(T);
            std::cout << "allocator[" << id_ << "] deallocating " << n << "*" << sizeof(T) << " bytes" << std::endl;
            free(p);
        }
        
        template <class T1, class T2>
        inline friend
        bool
        operator==(const custom_allocator<T1>& lhv, const custom_allocator<T2>& rhv) noexcept
        {
            return lhv.id_ == rhv.id_;
        }
        
        template <class T1, class T2>
        inline friend
        bool
        operator!=(const custom_allocator<T1>& lhv, const custom_allocator<T2>& rhv) noexcept
        {
            return lhv.id_ != rhv.id_;
        }
        
        template <class U> friend class custom_allocator;
        
    private:
        // stateful allocator
        int id_;
    };
    
    
    
    // Define an object policy which uses a std::unordered_map in terms of
    // Value and Allocator:
    template <typename Value, typename Allocator>
    struct unordered_map_object_policy : object_tag
    {
        typedef typename Allocator::template rebind<char>::other key_allocator;
        typedef std::basic_string<char, std::char_traits<char>, key_allocator> key_type;
        typedef std::pair<const key_type, Value> value_type;
        typedef typename Allocator::template rebind<value_type>::other allocator_type;
        
        typedef std::unordered_map<key_type, Value, std::hash<key_type>, std::equal_to<key_type>, allocator_type> type;
    };
    

    
    
    
    template <typename T>
    struct arena_allocator : json::utility::arena_allocator<T, json::utility::SysArena>
    {
        typedef json::utility::arena_allocator<T, json::utility::SysArena> base;
    public:
        typedef T value_type;
        typedef json::utility::SysArena arena_type;
        
    public:
        template <class U> struct rebind { typedef arena_allocator<U> other; };
        
        
        arena_allocator() = delete;

        arena_allocator(const arena_allocator& other)
        : base(other)
        {
        }
        
        template <typename U>
        arena_allocator(const arena_allocator<U>& other) noexcept
        : base(other)
        {}
        
        arena_allocator(arena_type& arena) noexcept
        : base(arena)
        {}
        
        arena_allocator& operator=(const arena_allocator&) = delete;
        
        
        T* allocate(std::size_t n) {
            //std::cout << "arena_allocator: allocating " << n << "*" << sizeof(T) << " bytes" << std::endl;
            return base::allocate(n);
        }
        
        void deallocate(T* p, std::size_t n) noexcept
        {
            //std::cout << "arena_allocator: deallocating " << n << "*" << sizeof(T) << " bytes" << std::endl;
            return base::deallocate(p, n);
        }
        
        
        
        template <class T1, class T2>
        friend
        bool
        operator==(const arena_allocator<T1>& x, const arena_allocator<T2>& y) noexcept
        {
            return operator==(static_cast<base>(x),static_cast<base>(y));
        }

        template <class T1, class T2>
        friend
        bool
        operator!=(const arena_allocator<T1>& x, const arena_allocator<T2>& y) noexcept
        {
            return operator!=(static_cast<base>(x),static_cast<base>(y));
        }
        
        template <typename U> friend class arena_allocator;
        
    };
    
    
}


namespace {
    

    
    class ValueCustomPoliciesTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        ValueCustomPoliciesTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~ValueCustomPoliciesTest() {
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
    
    
    
    TEST_F(ValueCustomPoliciesTest, CustomPolicies)
    {
        // Class template json::value template template parameters are "policies"
        // which determine the underlaying implementations for the JSON primitives
        // and JSON containers.
        
        // Declare a JSON value type with custom policies:
        
        typedef test::custom_allocator<void> allocator_t;
        typedef json::value<allocator_t, test::unordered_map_object_policy> value_t;
        typedef value_t::array_type  Array;
        typedef value_t::object_type Object;
        typedef value_t::string_type String;
        
        typedef test::unordered_map_object_policy<value_t, typename value_t::propagated_allocator> ObjectPolicy;
        
        EXPECT_TRUE( (std::is_same<
            typename allocator_t::template rebind<typename ObjectPolicy::type::value_type>::other,
            typename Object::allocator_type
                    >::value) );
                    
        EXPECT_TRUE( (std::is_same<
                      std::scoped_allocator_adaptor<typename allocator_t::template rebind<value_t>::other>,
                      typename Array::allocator_type
                      >::value) ) << typeid(typename Array::allocator_type).name() << std::endl;
        
        EXPECT_TRUE( (std::is_same<
                      typename allocator_t::template rebind<typename String::value_type>::other,
                      typename String::allocator_type
                      >::value) );
        
        EXPECT_TRUE( (std::is_same<
                      typename allocator_t::template rebind<void>::other,
                      typename Object::key_type::allocator_type::template rebind<void>::other
                      >::value) );
        
        
        allocator_t a1(1);
        allocator_t a2(2);
        a2 = a1;
        EXPECT_TRUE(a1 == a2);
    }
    
    
    TEST_F(ValueCustomPoliciesTest, ValueEmplace)
    {
        typedef test::custom_allocator<void> allocator_t;
        typedef json::value<allocator_t> Value;
        typedef typename Value::string_type String;
        typedef typename Value::object_type Object;
        typedef typename Value::array_type Array;

        allocator_t a(1);
        Value v1(Value::emplace_string, "abc", a);
        EXPECT_TRUE(v1.is_string());
        EXPECT_TRUE(v1.as<String>().get_allocator() == a);
        
        Value v2(Value::emplace_array, a);
        EXPECT_TRUE(v2.is_array());
        EXPECT_TRUE(v2.as<Array>().get_allocator() == a);
        
        Value v3(Value::emplace_object, a);
        EXPECT_TRUE(v3.is_object());
        EXPECT_TRUE(v3.as<Object>().get_allocator() == a);
    }
    
    
    TEST_F(ValueCustomPoliciesTest, ArrayConstruct)
    {
        typedef test::custom_allocator<void> allocator_t;
        typedef json::value<allocator_t> Value;
        typedef typename Value::string_type String;
        typedef typename Value::null_type Null;
        typedef typename Value::boolean_type Boolean;
        typedef typename Value::integral_number_type IntNumber;
        typedef typename Value::float_number_type FloatNumber;
        typedef typename Value::object_type Object;
        
        typedef typename Value::array_type Array;

        allocator_t a1(1);
        allocator_t a2(2);
        
        Value v;
        Array array1(a1);
        EXPECT_TRUE(array1.get_allocator().id() == 1);
        
        Array array2(array1);
        EXPECT_TRUE(array2.get_allocator().id() == 1);
        
        array1.push_back(v);
        EXPECT_TRUE(array1.back().is_null());
        
        String s1("string 1", a1);
        EXPECT_TRUE(s1.get_allocator().id() == 1);
        Value v1(s1);
        EXPECT_TRUE(v1.is_string());

        array1.push_back(v1);
        //array1.push_back(s1);
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        EXPECT_TRUE(array1.back().as<String>().get_allocator().id() == 1);
        
        array1.push_back(Value(s1));
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        EXPECT_TRUE(array1.back().as<String>().get_allocator().id() == 1);
        
        array1.push_back(std::move(v1));
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        EXPECT_TRUE(array1.back().as<String>().get_allocator().id() == 1);
        
        
        String s2("string 2", a2);
        EXPECT_TRUE(s2.get_allocator().id() == 2);
        Value v2(s2);
        EXPECT_TRUE(v2.is_string());
        
        array1.push_back(s2);
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        EXPECT_TRUE(array1.back().as<String>().get_allocator().id() == 1);
        
        array1.push_back(Value(s2));
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        EXPECT_TRUE(array1.back().as<String>().get_allocator().id() == 1);
        
        array1.push_back(std::move(v2));
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        EXPECT_TRUE(array1.back().as<String>().get_allocator().id() == 1);

        
#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        array1.emplace_back("string", a1);
#else
        array1.emplace_back(Value(Value::emplace_string, "string", a1));
#endif
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
                
        
#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        array1.emplace_back("string", a2);
#else
        array1.emplace_back(Value(Value::emplace_string, "string", a2));
#endif
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        
        
        
        
        array1.push_back(1);
        EXPECT_TRUE(array1.back().is_integral_number());
        
        array1.emplace_back(1);
        EXPECT_TRUE(array1.back().is_integral_number());

        array1.push_back(1.3);
        EXPECT_TRUE(array1.back().is_float_number());
        
        array1.emplace_back(1.3);
        EXPECT_TRUE(array1.back().is_float_number());
    }

    
    
    TEST_F(ValueCustomPoliciesTest, ValuePrimitiveImpConstruct)
    {
        typedef test::custom_allocator<void> allocator_t;
        typedef json::value<allocator_t> Value;
        typedef typename Value::string_type String;
        typedef typename Value::array_type Array;

        Value v1 = json::null;
        EXPECT_TRUE(v1.is_null());
        
        Value v2 = true;
        EXPECT_TRUE(v2.is_boolean());
        
        Value v3 = 1;
        EXPECT_TRUE(v3.is_integral_number());
        
        Value v4 = 1.2;
        EXPECT_TRUE(v4.is_float_number());
        
        Value v5(v4);
        EXPECT_TRUE(v5.is_float_number());
      
        EXPECT_FALSE( (std::is_constructible<Value, Value, allocator_t>::value) );
    }
    
    

    TEST_F(ValueCustomPoliciesTest, ValueStringImpConstruct)
    {
        typedef test::custom_allocator<void> allocator_t;
        typedef json::value<allocator_t> Value;
        typedef typename Value::string_type String;
        typedef typename Value::array_type Array;
        
        allocator_t a(1);

        String str("abc", a);
        EXPECT_TRUE(str.size() == 3);
        
        Value v0 = String("abc", a);
        EXPECT_TRUE(v0.is_string());
        
        Value v1(Value::emplace_string, "abc", a);
        EXPECT_TRUE(v1.is_string());
        
        
#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        Value v2("abc", a);
        EXPECT_TRUE(v2.is_string());
        
        Value v3({"abc", a});
        EXPECT_TRUE(v3.is_string());
        
        Value v4 = {"abc", a};
        EXPECT_TRUE(v4.is_string());
        
        Value v5{"abc", a};
        EXPECT_TRUE(v5.is_string());
#endif
    }
    
    
    
    TEST_F(ValueCustomPoliciesTest, ValueObjectConstruct)
    {
        typedef test::custom_allocator<void> allocator_t;
        typedef json::value<allocator_t> Value;        
        typedef Value::object_type  Object;
        typedef typename Value::string_type String;
        
        typedef typename Object::key_type key_t;
        
        allocator_t a(1);
        Object object(a);
        //aka: object.emplace("key1", 1);
        object.emplace(std::piecewise_construct, std::forward_as_tuple("key1", a), std::forward_as_tuple(1));
        object.insert(std::make_pair(key_t("key2",a), 2));
        object.insert(std::make_pair(key_t("key3",a), Value(3)));
        
#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        const Value s("abc", a);
#else
        const Value s(Value::emplace_string, "abc", a);
#endif
        EXPECT_TRUE(s.is_type<String>());
        Value s2(s);
        EXPECT_TRUE(s2.is_type<String>());
        const Value s3(s);
        auto x = std::make_pair(key_t("key4",a), s3);
        object.insert(x);
    }
    
    
    TEST_F(ValueCustomPoliciesTest, ValueArrayConstruct)
    {
        // Primarily check poper propagation of the outer allocator.
        
#if 1
        // This illustrates the idea of propagating the allocator:
        
        typedef test::custom_allocator<char> string_allocator_t;
        typedef std::basic_string<char, std::char_traits<char>, string_allocator_t> string_t;
        typedef test::custom_allocator<string_t> vector_allocator_t;
        typedef std::scoped_allocator_adaptor<vector_allocator_t> alloc_t;
        typedef std::vector<string_t, alloc_t> vector_t;
        
        alloc_t alloc1(1);
        vector_t vec(alloc1);
        alloc_t alloc2(2);
        string_t str1("abcdefg", alloc1);
        vec.push_back(str1);
        EXPECT_EQ(alloc1, vec.back().get_allocator());
        string_t str2("abcdefg", alloc2);
        vec.push_back(str2);
        EXPECT_EQ(alloc1, vec.back().get_allocator());
        vec.push_back({"abcde", alloc1});
        EXPECT_EQ(alloc1, vec.back().get_allocator());
        vec.push_back({"abcde", alloc2});
        EXPECT_EQ(alloc1, vec.back().get_allocator());
        vec.emplace_back("abc");
        EXPECT_EQ(alloc1, vec.back().get_allocator());
#endif
        
        
        typedef test::custom_allocator<void> allocator_t;
        typedef json::value<allocator_t> Value;
        typedef typename Value::string_type String;
        typedef Value::array_type  Array;
        
        static_assert(std::is_same<Value, typename Array::value_type>::value, "");
        
        allocator_t a1(1);
        allocator_t a2(2);
        
        EXPECT_EQ(1, a1.id());
        EXPECT_EQ(2, a2.id());

        Array array1(a1);
        EXPECT_EQ(a1, array1.get_allocator());
        EXPECT_EQ(a1.id(), array1.get_allocator().id());

        Value v1(1);
        array1.push_back(v1);
        EXPECT_TRUE(array1.back().is_integral_number());

#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        Value v2("abc",a1);
#else
        Value v2(Value::emplace_string, "abc", a1);
#endif
        array1.push_back(v2);
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_EQ(a1, array1.back().as<String>().get_allocator());
        EXPECT_EQ(a1.id(), array1.back().as<String>().get_allocator().id());
        
#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        Value v3("abc",a2);
#else
        Value v3(Value::emplace_string, "abc", a2);
#endif
        std::cout << 1 << std::endl;
        array1.push_back(v3);
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_EQ(a1, array1.back().as<String>().get_allocator());
        EXPECT_EQ(a1.id(), array1.back().as<String>().get_allocator().id());
        
#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        array1.push_back({"abcdefg",a1});
#else
        array1.push_back(Value(Value::emplace_string, "abcdefg", a1));
#endif
        EXPECT_TRUE(array1.back().is_string());
        EXPECT_TRUE(array1.back().as<String>().get_allocator() == a1);
        
#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        Array array2(10, Value({"abcd", a1}), a1);
#else
        Array array2(10, Value(Value::emplace_string,"abcd", a1), a1);
#endif
        EXPECT_EQ(10, array2.size());
        EXPECT_EQ(a1, array2.get_allocator());
        EXPECT_EQ(a1.id(), array2.get_allocator().id());
        EXPECT_EQ(a1, array2.back().as<String>().get_allocator());
        EXPECT_EQ(a1.id(), array2.back().as<String>().get_allocator().id());

#if !defined (VARIANT_NO_MULTIPLE_ARGS_CTOR)
        Array array3(10, Value({"abcd", a2}), a1);
#else
        Array array3(10, Value(Value::emplace_string, "abcd", a2), a1);
#endif
        EXPECT_EQ(10, array3.size());
        EXPECT_EQ(a1, array3.get_allocator());
        EXPECT_EQ(a1.id(), array3.get_allocator().id());
        EXPECT_EQ(a1, array3.back().as<String>().get_allocator());
        EXPECT_EQ(a1.id(), array3.back().as<String>().get_allocator().id());
    }
    

    TEST_F(ValueCustomPoliciesTest, ArenaAllocator)
    {
        
        typedef json::value<test::arena_allocator<void>> Value;
        typedef Value::array_type  Array;
        typedef Value::object_type Object;
        typedef Value::string_type String;
        typedef Value::integral_number_type IntNumber;
        typedef Value::float_number_type FloatNumber;
        typedef Value::boolean_type Boolean;
        typedef Value::null_type Null;
        
        typedef typename Value::scoped_allocator allocator_t;
        typedef typename allocator_t::arena_type arena_t;
        arena_t arena(16*8*1024);
        allocator_t m(arena);

        static_assert( std::is_constructible<test::arena_allocator<int>, test::arena_allocator<void>>::value, "");
        static_assert( std::is_constructible<std::scoped_allocator_adaptor<std::allocator<int>>, std::allocator<void>>::value, "");
        
        static_assert( std::is_constructible<typename Array::allocator_type, test::arena_allocator<typename Array::value_type>>::value  , "");
        static_assert( std::is_constructible<typename Object::allocator_type, test::arena_allocator<void>>::value  , "");
        static_assert( std::is_constructible<typename String::allocator_type, test::arena_allocator<void>>::value  , "");
        
        static_assert( std::is_constructible<typename Array::allocator_type, typename Value::scoped_allocator>::value  , "");
        static_assert( std::is_constructible<typename Object::allocator_type, typename Value::scoped_allocator>::value  , "");
        static_assert( std::is_constructible<typename String::allocator_type, typename Value::scoped_allocator>::value  , "");
        
        // These JSON elements do NOT require an allocator object in its ctor:
        // Null n;
        // Boolean b;
        // IntNumber i = 1;
        // FloatNumber f = 1.1;
        
        // These JSON elements MAY require an allocator object in its ctor:
        String str("abc", m);
        EXPECT_TRUE( str.get_allocator() == m);

        Array a(m);
        EXPECT_TRUE( a.get_allocator() == m);
        
        Object o(m);
        EXPECT_TRUE( o.get_allocator() == m);
        
    }
    
    
    TEST_F(ValueCustomPoliciesTest, CreateValueWithArenaAllocator)
    {
        typedef json::value<test::arena_allocator<void>> Value;
        typedef Value::array_type  Array;
        typedef Value::object_type Object;
        typedef Value::string_type String;
        
        typedef typename Value::scoped_allocator allocator_t;
        typedef typename allocator_t::arena_type arena_t;

        arena_t arena(16*8*1024);
        allocator_t m(arena);
        
        String longString1 = {"012345678901234567890123456789", m};
        Array a(m);
        a.reserve(10);
        for (int i = 0; i < 9; ++i) {
            a.emplace_back(longString1);
        }
        a.emplace_back(std::move(longString1));
        
        Object o(m);
        o.emplace(std::piecewise_construct, std::forward_as_tuple("key1", m), std::forward_as_tuple(a));
        o.emplace(std::piecewise_construct, std::forward_as_tuple("key2", m), std::forward_as_tuple(a));
        o.emplace(std::piecewise_construct, std::forward_as_tuple("key3", m), std::forward_as_tuple(std::move(a)));

        Value v(std::move(o));
        
    }
    
    
} // namespace 

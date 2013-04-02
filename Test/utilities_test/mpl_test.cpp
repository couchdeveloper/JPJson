//
//  mpl_test.cpp
//  Test
//
//  Created by Andreas Grosam on 06.03.13.
//
//

#include "json/utility/mpl.hpp"
#include <gtest/gtest.h>

#include <iostream>
#include <type_traits>
#include <map>
#include <vector>
#include <string>

// for testing


namespace test {
    
    struct Base {};

    template <typename T>
    struct Foo : Base {};

    template <typename T>
    using IsBaseOfBase = typename std::is_base_of<Base, T>::type;
}


namespace test {
    
    using json::utility::mpl::find_first;
    

    struct array_tag {};
    struct object_tag {};
    
    
    template <typename Value, typename Allocator>
    struct default_array_policy : array_tag
    {
        typedef Value value_type;
        typedef typename Allocator::template rebind<value_type>::other allocator_type;
        typedef std::vector<value_type, allocator_type> type;
    };
    
    template <typename Value, typename Allocator>
    struct default_object_policy : object_tag
    {
        typedef typename Allocator::template rebind<char>::other key_allocator;
        typedef std::basic_string<char, std::char_traits<char>, key_allocator> key_type;
        typedef std::pair<const key_type, Value> value_type;
        typedef typename Allocator::template rebind<value_type>::other allocator_type;
        typedef std::map<key_type, Value, std::less<key_type>,  allocator_type> type;
    };
    
    template <typename V, typename A>
    struct other_policy  {
    };
    
    
    template <
        typename A = std::allocator<void>,
        template <typename, typename> class... Policies
    >
    struct value
    {
        template <typename T>
        using IsArrayPolicy = typename std::is_base_of<array_tag, T>::type;
        
        template <typename T>
        using IsObjectPolicy = typename std::is_base_of<object_tag, T>::type;
        
        typedef typename find_first<IsArrayPolicy, Policies<value,A>..., default_array_policy<value,A>>::type array_policy;
        typedef typename find_first<IsObjectPolicy, Policies<value,A>..., default_object_policy<value,A>>::type object_policy;
        
        typedef typename array_policy::type array_type;
        typedef typename object_policy::type object_type;
    };
    
}




namespace {
    
    namespace mpl = json::utility::mpl;
    
    class MplTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        MplTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~MplTest() {
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
    
    
    TEST_F(MplTest, MplFirst)
    {
        EXPECT_TRUE( (std::is_same<void, mpl::first<>::type>::value) );
        EXPECT_TRUE( (std::is_same<int, mpl::first<int>::type>::value) );
        EXPECT_TRUE( (std::is_same<int, mpl::first<int, short>::type>::value) );
        EXPECT_TRUE( (std::is_same<int, mpl::first<int, short, double>::type>::value) );
        
        EXPECT_TRUE( (std::is_same<void, mpl::last<>::type>::value) );
        EXPECT_TRUE( (std::is_same<int, mpl::last<int>::type>::value) );
        EXPECT_TRUE( (std::is_same<short, mpl::last<int, short>::type>::value) );
        EXPECT_TRUE( (std::is_same<double, mpl::last<int, short, double>::type>::value) );
    }
    
    
    TEST_F(MplTest, MplFindFirst)
    {
        bool result = 
        std::is_same<
            test::Foo<void>,
            mpl::find_first<
                test::IsBaseOfBase,
                int, char, test::Foo<void>, test::Foo<int>, test::Foo<char>
            >::type
        >::value;

        EXPECT_TRUE(result);
    }
    
    TEST_F(MplTest, DefaultValueTypeGeneration)
    {
        typedef test::value<> Value;
        
        typedef typename Value::array_type Array;
        typedef typename Value::object_type Object;
        
        EXPECT_TRUE( (std::is_same<
                      std::allocator<typename Array::value_type>,
                      typename Array::allocator_type
                      >::value ) );
        
        EXPECT_TRUE( (std::is_same<
                      std::allocator<typename Object::value_type>,
                      typename Object::allocator_type
                      >::value ) );
        
    }
    
    
}
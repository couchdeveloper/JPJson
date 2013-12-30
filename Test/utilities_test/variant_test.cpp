//
//  variant_test.cpp
//  Test
//
//  Created by Andreas Grosam on 28.02.13.
//
//

#include <gtest/gtest.h>
#include "json/utility/variant.hpp"

#include <iostream>
#include <iomanip>
#include <scoped_allocator>


//#define VARIANT_HAS_NO_USES_ALLOCATOR_CONSTRUCTION

// for testing

namespace test {
    
    template <class T>
    class custom_allocator
    {
    public:
        typedef T value_type;
                
        template <class U> struct rebind { typedef custom_allocator<U> other; };
        
        custom_allocator() = delete;  // custom allocator shall not be default constructible
        
        explicit custom_allocator(int id) noexcept : id_(id)  {}
        
        custom_allocator(const custom_allocator& other) noexcept : id_(other.id_)  {}
        
        template <class U>
        custom_allocator(const custom_allocator<U>& other) noexcept : id_(other.id_) {}
        
        int id() const { return id_; }
        
        custom_allocator& operator=(const custom_allocator&) = delete;
        
        T* allocate(std::size_t n) {
            //std::cout << "allocator[" << id_ << "] allocating " << n << "*" << sizeof(T) << " bytes" << std::endl;
            const std::size_t sz = n*sizeof(T);
            return reinterpret_cast<T*>( malloc(sz) );
        }
        
        void deallocate(T* p, std::size_t n) noexcept {
            //const std::size_t sz = n*sizeof(T);
            //std::cout << "allocator[" << id_ << "] deallocating " << n << "*" << sizeof(T) << " bytes" << std::endl;
            free(p);
        }
        
        
        template <class U, class... Args>
        void construct(U* p, Args&&... args)
        {
            ::new((void *)p) U(std::forward<Args>(args)...);
        }
        
        
        template <class T1, class T2>
        inline friend
        bool
        operator==(const custom_allocator<T1>& lhv, const custom_allocator<T2>& rhv) noexcept {
            return lhv.id_ == rhv.id_;
        }
        
        template <class T1, class T2>
        inline friend
        bool
        operator!=(const custom_allocator<T1>& lhv, const custom_allocator<T2>& rhv) noexcept {
            return lhv.id_ != rhv.id_;
        }
        
        template <class U> friend class custom_allocator;
        
    private:
        // stateful allocator
        int id_;
    };
    
    
    
} // test



namespace {
    
    using json::utility::variant;
    using json::utility::emplace_t;
    
    class variant_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        variant_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~variant_test() {
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
    
    
#pragma mark -

    TEST_F(variant_test, TypeTraits)
    {
        typedef variant<char, int, float> var;
        EXPECT_TRUE(var::contains_type<char>::value);
        EXPECT_TRUE(var::contains_type<int>::value);
        EXPECT_TRUE(var::contains_type<float>::value);
        EXPECT_FALSE(var::contains_type<double>::value);
    }
    
    TEST_F(variant_test, Noexcept)
    {
        // Default Ctor
        typedef variant<char, int, float> var1;
        EXPECT_TRUE(noexcept(var1()));
        
        struct A {
            A() noexcept(false) {}
        };
        typedef variant<A, char, int, float> var2;
        EXPECT_FALSE(noexcept(var2()));
        
        // move ctor
        
        // copy ctor
        
        // other ctors
        
        // swap
        
        // destructor
    }
    
    
    // The __has_select compile time helper function should determine whether a
    // given set of arguments passed to a variant's constructor will unabiguously
    // find the target type, and as such tests the underlying type machinery
    // of class variant: json::utility::detail::constructor.
    //
    // It's intended that its return type becomes std::true_type if the given
    // arguments can be used to be passed to the found target type's constructor.
    // Otherwise, it's return type becomes std::false_type.
    //
    // *Currently* the `Select` facility will only work for *one* argument.
    // Multiple arguments may be enabled if the compiler supports inheriting
    // constructors.

    namespace {
    
        template <typename...Types>
        struct __Selector : json::utility::detail::selector<Types...> {};
        
        template <typename Selector, typename... Args>
        using __Select = decltype(Selector::select(std::declval<Args>()...));
        // Note: static member __Select::select() will fail to compile if there
        // is no target type that can be unabiguously constructed from the given
        // arguments.
    
        
        template <typename Selector, typename... Args>
        inline constexpr auto __has_select_test(Selector&&, Args&&...)
        -> decltype(__Select<Selector, Args...>(), std::true_type() );

        inline constexpr auto __has_select_test(...)
        -> std::false_type;
        
        template <typename Selector, typename... Args>
        struct __has_select : decltype(__has_select_test(std::declval<Selector>(), std::declval<Args>()... ))
        {};
        
    }
    
//    template <typename T>
//    struct _Result {
//        typedef T type;
//    };
//    TEST_F(variant_test, TestTestHasSelect)
//    {
//        // Test our test-helpers itself:
//        struct M {
//            M() = delete;
//            M(int){};
//        };
//        struct Arg {};
//        struct A {A(); A(const Arg&); };
//        struct B {B();};
//        struct D { D(){}; D(const int&, const M& m){}; };
//        
//        // Basicly, mimics the class json::utility::detail::constructor:
//        struct selector {
//            static auto select(const A&) -> _Result<A>;
//            static auto select(const B&) -> _Result<B>;
//            static auto select(const D&) -> _Result<D>;
//        };
//        
//        struct C {};
//        
//        static_assert( __has_select<selector, A>::value == true, "");
//        static_assert( __has_select<selector, Arg>::value == true, "");
//        static_assert( __has_select<selector, B>::value == true, "");
//        static_assert( __has_select<selector, D>::value == true, "");
//        static_assert( __has_select<selector, int>::value == false, "");
//        static_assert( __has_select<selector, short>::value == false, "");
//        static_assert( __has_select<selector, C>::value == false, "");
//    }

    
    
    TEST_F(variant_test, DetailSelectorExactMatch)
    {
        // Exact match of argument type with an implementation type
        
        typedef __Selector<char, short, int, long, double> selector_t;
        
        static_assert( std::is_same<char, __Select<selector_t, char>::type>::value, "");
        static_assert( std::is_same<short, __Select<selector_t, short>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, int>::type>::value, "");
        static_assert( std::is_same<long, __Select<selector_t, long>::type>::value, "");
        static_assert( std::is_same<double, __Select<selector_t, double>::type>::value, "");

        // Test index
        static_assert( 0 == __Select<selector_t, char>::value, "");
        static_assert( 1 == __Select<selector_t, short>::value, "");
        static_assert( 2 == __Select<selector_t, int>::value, "");
        static_assert( 3 == __Select<selector_t, long>::value, "");
        static_assert( 4 == __Select<selector_t, double>::value, "");
        
        // TODO: (not yet implemented: more than one argument in ctor):
        //static_assert( std::is_same<vector, __Select<selector_t, int, int>::type>::value, "");
    }

    
    TEST_F(variant_test, DetailSelectorAmbiguity)
    {
        struct A {A(){}};
        struct B {B(){}};
        struct D { D(int); };
        struct C {};
        
        typedef __Selector<A, B, int> selector1;
        
        static_assert( __has_select<selector1, A>::value == true, "");
        static_assert( __has_select<selector1, B>::value == true, "");
        static_assert( __has_select<selector1, int>::value == true, "");
        static_assert( __has_select<selector1, short>::value == true, "");
        static_assert( __has_select<selector1, long>::value == true, "");
        static_assert( __has_select<selector1, C>::value == false, "");
        
        
        typedef __Selector<short, long> selector2;
        static_assert( __has_select<selector2, short>::value == true, "");
        static_assert( __has_select<selector2, long>::value == true, "");
        static_assert( __has_select<selector2, int>::value == false, "");
        
        struct F { F(int); };
        typedef __Selector<C, D, F> selector3;
        static_assert( __has_select<selector3, short>::value == false, "");
        static_assert( __has_select<selector3, long>::value == false, "");
        static_assert( __has_select<selector3, int>::value == false, "");
        
        
        
        // TODO: (not yet implemented: more than one argument in ctor):
//        struct A {
//            A(short, char) {}
//        };
//        
//        struct B {
//            B(short, long) {}
//        };
    }
    

    TEST_F(variant_test, DetailSelectorConvertibleArg1)
    {
        // implicit conversion
        
        struct A {};
        struct B {};
        typedef __Selector<int, A, B> selector_t;
        
        static_assert( std::is_same<int, __Select<selector_t, char>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, short>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, int>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, long>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, double>::type>::value, "");
                
        // TODO: (not yet implemented: more than one argument in ctor):
    }
    
    
    TEST_F(variant_test, DetailSelectorConvertibleArg2)
    {
        // implicit conversion
        
        struct A {
            A (double) {}
        };
        struct B {};
        typedef __Selector<A, B> selector_t;
        
        static_assert( std::is_same<A, __Select<selector_t, char>::type>::value, "");
        static_assert( std::is_same<A, __Select<selector_t, short>::type>::value, "");
        static_assert( std::is_same<A, __Select<selector_t, int>::type>::value, "");
        static_assert( std::is_same<A, __Select<selector_t, long>::type>::value, "");
        static_assert( std::is_same<A, __Select<selector_t, double>::type>::value, "");
        
        // TODO: (not yet implemented: more than one argument in ctor):
        //static_assert( std::is_same<vector, __Select<selector_t, int, int>::type>::value, "");
    }
    
    TEST_F(variant_test, ConvertibleArg3)
    {
        // Test overload rules
        
        struct A {A(int){}};
        struct B {B(double){}};
        struct C {};

        typedef __Selector<A, B, int> selector_t;
        
        static_assert( std::is_same<A, __Select<selector_t, A>::type>::value, "");
        static_assert( std::is_same<B, __Select<selector_t, B>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, int>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, long>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, double>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, short>::type>::value, "");
        
        static_assert( std::is_same<A, __Select<selector_t, const A>::type>::value, "");
        static_assert( std::is_same<B, __Select<selector_t, const B>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, const int>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, const long>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, const double>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, const short>::type>::value, "");
        
        static_assert( std::is_same<A, __Select<selector_t, A&&>::type>::value, "");
        static_assert( std::is_same<B, __Select<selector_t, B&&>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, int&&>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, long&&>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, double&&>::type>::value, "");
        static_assert( std::is_same<int, __Select<selector_t, short&&>::type>::value, "");
        

        
        static_assert( __has_select<selector_t, A>::value == true, "");
        static_assert( __has_select<selector_t, B>::value == true, "");
        static_assert( __has_select<selector_t, int>::value == true, "");
        static_assert( __has_select<selector_t, long>::value == true, "");
        static_assert( __has_select<selector_t, double>::value == true, "");
        static_assert( __has_select<selector_t, short>::value == true, "");
        static_assert( __has_select<selector_t, C>::value == false, "");

    
        static_assert( __has_select<selector_t, const A>::value == true, "");
        static_assert( __has_select<selector_t, const B>::value == true, "");
        static_assert( __has_select<selector_t, const int>::value == true, "");
        static_assert( __has_select<selector_t, const long>::value == true, "");
        static_assert( __has_select<selector_t, const double>::value == true, "");
        static_assert( __has_select<selector_t, const short>::value == true, "");
        static_assert( __has_select<selector_t, const C>::value == false, "");
    }
    
    
    TEST_F(variant_test, DetailSelector)
    {
        struct A {
            A(int) {}
        };
        struct B {
            explicit B(int) {}
        };
        typedef __Selector<A, B> selector_t;
        
        static_assert( std::is_same<A, __Select<selector_t, A>::type>::value, "");
        static_assert( std::is_same<A, __Select<selector_t, int>::type>::value, "");
        static_assert( std::is_same<B, __Select<selector_t, B>::type>::value, "");
        
        // TODO: (not yet implemeted: more than one argument):
        //static_assert( std::is_same<vector, __Select<selector_t, int, int>::type>::value, "");
    }
    
    
    
    // A mock for an allocator enabled class
    template <class T, class Alloc = std::allocator<T>>
    struct __A {
        typedef Alloc allocator_type;
        typedef T value_type;
        
        explicit __A(const Alloc& alloc = Alloc()) : alloc_(alloc) {};
        
        __A(const __A& other) : alloc_(other.alloc_) {};
        __A(__A&& other) : alloc_(other.alloc_) {}
        
        // prefix allocator style
        __A(std::allocator_arg_t, Alloc const& a, const __A&) : alloc_(a) {};
        __A(std::allocator_arg_t, Alloc const& a, __A&&) : alloc_(a) {};
        __A(std::allocator_arg_t, Alloc const& a, const T&) : alloc_(a) {}
        
        // suffix allocator style
        __A(const __A&, Alloc const& alloc) : alloc_(alloc) {};
        __A(__A&&, Alloc const& alloc) : alloc_(alloc) {};
        __A(const T&, Alloc const& alloc = Alloc()) : alloc_(alloc){}
        
        allocator_type get_allocator() const { return alloc_; }
        
        Alloc alloc_;
    };
    
    
    
//    // This **maybe** become relevant when there is a allocator extended
//    // forwarding ctor in class variant. Only there we need the Select
//    // facility.
//    TEST_F(variant_test, DetailSelectorWithAlloctorExtendedConstructor1)
//    {
//        typedef __A<int, test::custom_allocator<int>> A; 
//        typedef __A<int> A0;
//        typedef typename A::allocator_type alloc_t;
//        
//        struct B {};
//                
//        static_assert( __has_select<__Selector<A,B>, A>::value == true , "");
//        static_assert( __has_select<__Selector<A,B>, B>::value == true , "");
//        static_assert( __has_select<__Selector<A,B>, int>::value == false , ""); // requires default constructible allocator
//        static_assert( __has_select<__Selector<A0,B>, int>::value == true , "");
//        
//        // Prefix style:
//        static_assert( __has_select<__Selector<A,B>, std::allocator_arg_t, alloc_t, A>::value == true, "");
//        static_assert( __has_select<__Selector<A,B>, std::allocator_arg_t, alloc_t, B>::value == false, "");
//        static_assert( __has_select<__Selector<A,B>, std::allocator_arg_t, alloc_t, int>::value == true, "");
//        
//        // Suffix style
//        static_assert( __has_select<__Selector<A,B>, A, alloc_t>::value == true, "");
//        static_assert( __has_select<__Selector<A,B>, B, alloc_t>::value == false, "");
//        static_assert( __has_select<__Selector<A,B>, int, alloc_t>::value == true, "");
//        static_assert( __has_select<__Selector<A,B>, int, alloc_t>::value == true, "");
//        static_assert( __has_select<__Selector<B,A>, int, alloc_t>::value == true, "");
//        
//    }
    
    
    
    TEST_F(variant_test, DefaultCtor)
    {
        typedef variant<char, int, float> var1;
        typedef variant<float, int, char> var2;
        typedef variant<int, char, float> var3;
        
        EXPECT_EQ(0, var1().which());
        EXPECT_EQ(0, var2().which());
        EXPECT_EQ(0, var3().which());
        
        var3 v = {};
        EXPECT_EQ(0, v.which());
        
        struct D {
            D(){};
            int value_ = 1;
        };
        typedef variant<D, int, char, float> var4;
        EXPECT_EQ(0, var4().which());
        EXPECT_TRUE(var4().is_type<D>());
        EXPECT_TRUE(var4().as<D>().value_ == 1);
    }
    

    
    
    TEST_F(variant_test, CopyCtor)
    {
        typedef variant<char, int, float> var;
        var v1;
        
        EXPECT_EQ(0, var(v1).which());
        EXPECT_TRUE(var(v1).is_type<char>());
        
        var v2 = 1;
        EXPECT_EQ(1, v2.which());
        EXPECT_TRUE(v2.is_type<int>());        
        EXPECT_EQ(1, var(v2).which());
        EXPECT_TRUE(var(v2).is_type<int>());
        
        var v3 = 1.1f;
        EXPECT_EQ(2, v3.which());
        EXPECT_TRUE(v3.is_type<float>());
        EXPECT_EQ(2, var(v3).which());
        EXPECT_TRUE(var(v3).is_type<float>());
    }
    
    
    TEST_F(variant_test, MoveCtor)
    {
        typedef variant<char, int, float, std::vector<int>> var;
        var v1;
        
        EXPECT_EQ(0, var(std::move(v1)).which());
        EXPECT_TRUE(var(std::move(v1)).is_type<char>());
        
        var v2 = 1;
        EXPECT_EQ(1, v2.which());
        EXPECT_TRUE(v2.is_type<int>());
        EXPECT_EQ(1, var(std::move(v2)).which());
        EXPECT_TRUE(var(v2).is_type<int>());
        
        var v3 = 1.1f;
        EXPECT_EQ(2, v3.which());
        EXPECT_TRUE(v3.is_type<float>());
        EXPECT_EQ(2, var(std::move(v3)).which());
        EXPECT_TRUE(var(v3).is_type<float>());
        
        std::vector<int> vec = {0, 1, 2, 3, 4};
        var v4 = std::move(vec);
        std::vector<int>& rvec = v4.as<std::vector<int>>();
        EXPECT_TRUE( (rvec == std::vector<int>{0, 1, 2, 3, 4}) );
        EXPECT_TRUE(vec.size() == 0);
    }
    
    
#if !defined (VARIANT_HAS_NO_USES_ALLOCATOR_CONSTRUCTION)
    TEST_F(variant_test, Ctor)
    {
        typedef std::vector<int, test::custom_allocator<int>> vector_t;
        typedef std::basic_string<char, std::char_traits<char>, test::custom_allocator<char>> string_t;

        typedef test::custom_allocator<void> allocator_t;
        allocator_t a1(1);
        
        string_t s = {"abcd", a1};
        vector_t v = {{0,1,2,3,4,5,6}, a1};
        
        typedef variant<vector_t, string_t> var;
        
        // The following statement should create a default vector_t with allocator
        var v00(std::allocator_arg, a1);
        EXPECT_TRUE(v00.as<vector_t>().get_allocator().id() == 1);
        
        
        var v0(s);
        string_t& rs = v0.as<string_t>();
        EXPECT_TRUE(rs.get_allocator().id() == 1);
        
        allocator_t a2(2);
        var v1(std::allocator_arg, a2, v0);
        EXPECT_TRUE(v1.as<string_t>().get_allocator().id() == 2);
        
        var v2(std::allocator_arg, a2, v);
        EXPECT_TRUE(v2.as<vector_t>().get_allocator().id() == 2);
        
//        vector_alloc_t a;
//        vector_t vec;
//        
//        var v(vec);
//        var v1(vec,a);
        
    }
#endif
    

    TEST_F(variant_test, EmplaceCtor1)
    {
        typedef std::vector<int> vector_t;
        typedef variant<char, int, float, vector_t> var;
        
        var v( emplace_t<vector_t>(), {3, 0} );
        EXPECT_EQ(3, v.which());
        EXPECT_TRUE(v.is_type<vector_t>());
        vector_t& rvec = v.as<vector_t>();
        EXPECT_TRUE( (rvec == vector_t{3,0}) );
    }


    TEST_F(variant_test, EmplaceCtor2)
    {
        typedef std::vector<std::string> vector_t;
        typedef variant<int, float, vector_t> var;
        
        var v( emplace_t<vector_t>(), {{"a"}, {"b"}, {"c"}} );
        EXPECT_EQ(2, v.which()); // vector_t
        EXPECT_TRUE(v.is_type<vector_t>());
        vector_t& rvec = v.as<vector_t>();
        EXPECT_TRUE( (rvec == vector_t{"a", "b", "c"}) );
    }
    
    
    
    TEST_F(variant_test, CopyAssignment)
    {
        typedef std::vector<int> vector_t;
        typedef variant<char, int, float, vector_t> var;

        var v;
        
        int i = 1;
        v = i;
        EXPECT_EQ(1, v.which());
        EXPECT_TRUE(v.is_type<int>());
        int& ri = v.as<int>();
        EXPECT_TRUE( (ri == i) );
        
        char c = 'a';
        v = c;
        EXPECT_EQ(0, v.which());
        EXPECT_TRUE(v.is_type<char>());
        char& rc = v.as<char>();
        EXPECT_TRUE( (rc == c) );
        
        float f = 1.1f;
        v = f;
        EXPECT_EQ(2, v.which());
        EXPECT_TRUE(v.is_type<float>());
        float& rf = v.as<float>();
        EXPECT_DOUBLE_EQ(f, rf);

        vector_t vec = {0,1,2,3};
        v = vec;
        EXPECT_EQ(3, v.which());
        EXPECT_TRUE(v.is_type<vector_t>());
        vector_t& rvec = v.as<vector_t>();
        EXPECT_TRUE( (rvec == vec) );
        
    }
    
    
    TEST_F(variant_test, MoveAssignment)
    {
        typedef std::vector<int> vector_t;
        typedef variant<char, int, float, vector_t> var;
        
        var v;
        
        v = 1;
        EXPECT_EQ(1, v.which());
        EXPECT_TRUE(v.is_type<int>());
        int& ri = v.as<int>();
        EXPECT_TRUE( (ri == 1) );
        
        v = 'a';
        EXPECT_EQ(0, v.which());
        EXPECT_TRUE(v.is_type<char>());
        char& rc = v.as<char>();
        EXPECT_TRUE( (rc == 'a') );
        
        v = 1.1f;
        EXPECT_EQ(2, v.which());
        EXPECT_TRUE(v.is_type<float>());
        float& rf = v.as<float>();
        EXPECT_DOUBLE_EQ(1.1f, rf);
        
        v = vector_t{0,1,2,3};
        EXPECT_EQ(3, v.which());
        EXPECT_TRUE(v.is_type<vector_t>());
        vector_t& rvec = v.as<vector_t>();
        EXPECT_TRUE( (rvec == vector_t{0,1,2,3}) );
        
        vector_t vec = {0,1,2,3,4};
        v = std::move(vec);
        EXPECT_EQ(3, v.which());
        EXPECT_TRUE(v.is_type<vector_t>());
        vector_t& rvec2 = v.as<vector_t>();
        EXPECT_TRUE( (rvec2 == vector_t{0,1,2,3,4}) );
        EXPECT_EQ(0, vec.size());
    }
    
    TEST_F(variant_test, EmplaceAssignment)
    {
        typedef std::vector<int> vector_t;
        typedef variant<char, int, float, vector_t> var;
        
        var v;
        
        v.emplace<int>(1);
        EXPECT_EQ(1, v.which());
        EXPECT_TRUE(v.is_type<int>());
        int& ri = v.as<int>();
        EXPECT_TRUE( (ri == 1) );
        
        v.emplace<char>(static_cast<int>('a'));
        EXPECT_EQ(0, v.which());
        EXPECT_TRUE(v.is_type<char>());
        char& rc = v.as<char>();
        EXPECT_TRUE( (rc == 'a') );
        
        v.emplace<float>(1.1);
        EXPECT_EQ(2, v.which());
        EXPECT_TRUE(v.is_type<float>());
        float& rf = v.as<float>();
        EXPECT_DOUBLE_EQ(1.1f, rf);
        
        v.emplace<vector_t>({0,1,2,3});
        EXPECT_EQ(3, v.which());
        EXPECT_TRUE(v.is_type<vector_t>());
        vector_t& rvec = v.as<vector_t>();
        EXPECT_TRUE( (rvec == vector_t{0,1,2,3}) );
    }


    TEST_F(variant_test, Accessors)
    {
        typedef std::vector<int> vector_t;
        typedef variant<char, int, vector_t> var;
        
        var v = 1;
        EXPECT_EQ(1, v.which());
        EXPECT_EQ(1, v.as<int>());
        EXPECT_TRUE(v.is_type<int>());
        int& ri = v.as<int>();
        ri = 2;
        int* pi = v.as<int*>();
        EXPECT_TRUE(pi != nullptr);
        EXPECT_TRUE(*pi == 2);
        
        EXPECT_TRUE( noexcept(v.interpret_as<char>()) );
        EXPECT_NO_THROW(v.interpret_as<char>());
        
        //       EXPECT_COMPILE_ERROR(v0.as<float*>());
        EXPECT_THROW(v.as<char>(), json::utility::bad_variant_access);
        EXPECT_TRUE(v.as<char*>() == nullptr);
        
        const var cv = 1;
        EXPECT_EQ(1, cv.which());
        EXPECT_EQ(1, cv.as<int>());
        EXPECT_TRUE(cv.is_type<int>());
        // int& cri = cv.as<int>();  shall yield compiler error
        const int& cri = cv.as<int>();
        // int* cpi = cv.as<int*>();  shall yield compiler error
        int const* cpi = cv.as<int*>();
        EXPECT_TRUE(cpi != nullptr);
        EXPECT_TRUE(*cpi == 1);
        EXPECT_TRUE(*cpi == cri);
    }
    
    
    TEST_F(variant_test, Comparison)
    {
        typedef std::vector<int> vector_t;
        typedef variant<char, int, vector_t> var;

        var v1 = 1;
        var v2 = 1;
        
        EXPECT_TRUE(v1 == v2);
        EXPECT_FALSE(v1 != v2);
        
        v1 = 2;
        EXPECT_FALSE(v1 == v2);
        EXPECT_TRUE(v1 != v2);
        
        v1 = 'a';
        v2 = 'a';
        EXPECT_TRUE(v1 == v2);
        EXPECT_FALSE(v1 != v2);
        
        v2 = 'b';
        EXPECT_FALSE(v1 == v2);
        EXPECT_TRUE(v1 != v2);
        
        
        v1 = vector_t{1,2,3,4};
        v2 = vector_t{1,2,3,4};
        EXPECT_TRUE(v1 == v2);
        EXPECT_FALSE(v1 != v2);
        
        v2 = vector_t{1,2};
        EXPECT_FALSE(v1 == v2);
        EXPECT_TRUE(v1 != v2);


        v1 = 1;
        EXPECT_TRUE(v1 == 1);
        EXPECT_TRUE(1 == v1);
        EXPECT_FALSE(v1 != 1);
        EXPECT_FALSE(1 != v1);
    }
    

    // TODO
//    TEST_F(variant_test, AllocatorExtendedCtors)
//    {
//        typedef test::custom_allocator<char> string_alloc_t;
//        typedef std::basic_string<char, std::char_traits<char>, string_alloc_t> string;
//        
//        typedef variant<int, std::vector<int>, string> var;
//        
//        var v0;
//        var v01{};
//        var v02 = {};
//
//        // var v03 = std::allocator<void>();  Shall not Compile
//        EXPECT_FALSE( (std::is_constructible<var, std::allocator<void>>::value) );
//        
//        string_alloc_t a(1);
//        EXPECT_TRUE( (std::is_constructible<var, std::allocator_arg_t, string_alloc_t, decltype("abc")>::value) );
//        var v1(std::allocator_arg, a, "abc");
//
//        EXPECT_TRUE( (std::is_constructible<var, decltype("abc"), string_alloc_t>::value) );
//        var v2("abc", a);
//    }
    
    
//TODO:
//    TEST_F(variant_test, ScopedAllocatorModel)
//    {
//        
//        typedef test::custom_allocator<char> string_alloc_t;
//        typedef std::basic_string<char, std::char_traits<char>, string_alloc_t> string_t;
//        typedef std::vector<string_t, std::scoped_allocator_adaptor<test::custom_allocator<string_t>>> vector_t;
//        typedef typename vector_t::allocator_type alloc_t; // scoped allocator adaptor
//        
//        alloc_t _a1(1);
//        string_t _s("abcdefghijklmnopqrstuvwxyz01234567890", _a1);
//        alloc_t _a2(2);
//        vector_t _v(_a2);
//        _v.push_back(_s);
//        EXPECT_EQ(_v.get_allocator(), _v.back().get_allocator() );
//        
//        
//        
//        typedef test::custom_allocator<char> CustomAllocator;
//        static_assert(std::is_constructible<test::custom_allocator<char>, test::custom_allocator<void>>::value, "");
//        static_assert(std::is_constructible<test::custom_allocator<void>, test::custom_allocator<char>>::value, "");
//        
//        // A variant uses any allocator
//        EXPECT_TRUE( (std::uses_allocator< variant<char, int, float>, CustomAllocator >::value) );
//        EXPECT_TRUE( (std::uses_allocator< variant<char, int, float>, std::allocator<void> >::value) );
//        
//        // Check if the Scoped Allocator Model holds true:
//        typedef std::basic_string<char, std::char_traits<char>, test::custom_allocator<char>> string;
//        typedef variant<int, string> var;
//        typedef std::vector<var, std::scoped_allocator_adaptor<test::custom_allocator<var>>> vector;
//        static_assert( std::uses_allocator<string, test::custom_allocator<char>>::value, "");
//        static_assert( std::uses_allocator<string, std::scoped_allocator_adaptor<test::custom_allocator<var>>>::value, "");
//        
//        typedef std::scoped_allocator_adaptor<test::custom_allocator<var>> ScopedVectorAllocator;
//        CustomAllocator a1(1);
//        ScopedVectorAllocator a2(2);
//        EXPECT_TRUE( a1 != a2 );
//        EXPECT_FALSE( a1 == a2 );
//        
//        string s1("abcdefghijklmnopqrstuvwxyz01234567890", a1);
//        vector vec(a2);
//        EXPECT_FALSE(vec.get_allocator() == s1.get_allocator() );
//        
//        var v1(s1);
//        EXPECT_TRUE(v1.is_type<string>());
//        EXPECT_EQ(a1, v1.as<string>().get_allocator() );
//        
//        
//        vec.push_back(v1);
//        string& rs = vec.back().as<string>();
//        EXPECT_EQ(vec.get_allocator(), vec.back().as<string>().get_allocator() );
//
//        // TODO:
////        vec.push_back(std::move(s1));
////        EXPECT_EQ(vec.get_allocator(), vec.back().as<string>().get_allocator() );
//        
//        // TODO:
//        //vec.emplace_back("abcdefghijklmnopqrstuvwxyz01234567890");
//    }
    
    
#if 0
    TEST_F(variant_test, Test1)
    {
        typedef variant<int, char> var;
        
        var v0;
        EXPECT_EQ(0, v0.which());
        EXPECT_TRUE(v0.is_type<int>());
        v0 = 1;
        EXPECT_EQ(0, v0.which());
        EXPECT_EQ(1, v0.as<int>());
        EXPECT_TRUE(v0.is_type<int>());
        int& ri0 = v0.as<int>();
        ri0 = 2;
        int* pi0 = v0.as<int*>();
        EXPECT_TRUE(pi0 != nullptr);
        EXPECT_TRUE(*pi0 == 2);
        
        

        EXPECT_TRUE( noexcept(v0.interpret_as<char>()) );
        EXPECT_NO_THROW(v0.interpret_as<char>());

//       EXPECT_COMPILE_ERROR(v0.as<float*>());
        EXPECT_THROW(v0.as<char>(), json::utility::bad_variant_access);
        EXPECT_TRUE(v0.as<char*>() == nullptr);
        
        
        const var cv0;
        var v1(v0);
        assert(v0.which() == v1.which());
        assert(v1.is_type<int>());
        var v2(cv0);
        assert(v2.which() == cv0.which());
        var v3(var{});
        assert(v3.which() == 0);
        assert(v3.is_type<int>());
        
        
        var v10 = 'a';
        assert(v10.which() == 1);
        assert(v10.is_type<char>());
        const char ch = 'b';
        var v11 = ch;
        assert(v11.which() == 1);
        assert(v11.is_type<char>());
        var v12{'c'};
        assert(v12.which() == 1);
        assert(v12.is_type<char>());
        var v13 = {'d'};
        assert(v13.which() == 1);
        assert(v13.is_type<char>());
        
        
        static_assert( std::is_constructible<var, int>::value == true , "");
        var(1);
        static_assert( std::is_convertible<int, var>::value == true , "");
        var v4 = 1;
        static_assert( std::is_constructible<var, char>::value == true , "");
        var('a');
        static_assert( std::is_convertible<char, var>::value == true , "");
        var v5 = 'a';
        
        var v6 = {1};
        
        var v7(std::move(v0));
        
        
        //static_assert( std::is_constructible<variant<double>, int>::value == true, "");
        variant<double> vd0 = 1;
    }
#endif
    
}




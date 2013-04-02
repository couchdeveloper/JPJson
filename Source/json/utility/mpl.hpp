//
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

#ifndef JSON_UTILITY_MPL_HPP
#define JSON_UTILITY_MPL_HPP

#include <type_traits>
#include <cstdlib>



namespace json { namespace utility { namespace mpl {
    
    
    // A few handy template aliases:
    
    template <typename T>
    using Type = typename T::type;
    
    
    template<typename T>
    using Unqualified = typename std::remove_cv<
    typename std::remove_reference<T>::type
    >::type;
    
    template <typename If, typename Then, typename Else>
    using Conditional = Type<std::conditional<If::value, Then, Else>>;
    
    
}}}


#pragma mark first, last
namespace json { namespace utility { namespace mpl {
    
    
    template <typename...>
    struct first;
    
    template <typename...>
    struct first {
        typedef void type;
    };
    
    template <typename Head, typename ...Tail>
    struct first<Head, Tail...> {
        typedef Head type;
    };
    
    
    template <typename...>
    struct last;
    
    template <typename...>
    struct last {
        typedef void type;
    };
    
    template <typename Last>
    struct last<Last> {
        typedef Last type;
    };
    
    template <typename Head, typename ...Tail>
    struct last<Head, Tail...> : last<Tail...> {
    };
    
    
    namespace test {
        
        static_assert( std::is_same<void, mpl::first<>::type>::value , "");
        static_assert( std::is_same<int, mpl::first<int>::type>::value , "");
        static_assert( std::is_same<int, mpl::first<int, short>::type>::value , "");
        static_assert( std::is_same<int, mpl::first<int, short, double>::type>::value , "");
        
        
        static_assert( std::is_same<void, mpl::last<>::type>::value , "");
        static_assert( std::is_same<int, mpl::last<int>::type>::value , "");
        static_assert( std::is_same<short, mpl::last<int, short>::type>::value , "");
        static_assert( std::is_same<double, mpl::last<int, short, double>::type>::value , "");
    }
    
}}}






#pragma mark and_, or_
namespace json { namespace utility { namespace mpl {
    
    template<typename...>
    struct and_ : std::true_type {};
    
    template<typename Head, typename... Tail>
    struct and_<Head, Tail...>
    : std::integral_constant<bool, Head::value and and_<Tail...>::value>
    {};
    
    
    
    template<typename...>
    struct or_ : std::false_type {};;
    
    template<typename Head, typename... Tail>
    struct or_<Head, Tail...>
    :  std::integral_constant<bool, Head::value or or_<Tail...>::value>
    {};
    
    
    template<bool...>
    struct and_c;
    
    template<bool Last>
    struct and_c<Last>
    {
        static const constexpr bool value = Last;
    };
    
    template<bool Head, bool... Tail>
    struct and_c<Head, Tail...>
    {
        static const constexpr bool value = Head and and_c<Tail...>::value;
    };
    
    
    template<bool...>
    struct or_c;
    
    template<bool Last>
    struct or_c<Last>
    {
        static const constexpr bool value = Last;
    };
    
    template<bool Head, bool... Tail>
    struct or_c<Head, Tail...>
    {
        static const constexpr bool value = Head or or_c<Tail...>::value;
    };
    
}}}


#pragma mark is_homogenous
namespace json { namespace utility { namespace mpl {
    
    
    template <typename...>
    struct is_homogenous;
    
    template <typename T>
    struct is_homogenous<T> : std::true_type {};
    
    template <typename Head, typename Next, typename... Tail>
    struct is_homogenous<Head, Next, Tail...> :   std::conditional<std::is_same<
    Head,
    Next
    >::value,
    typename is_homogenous<Next, Tail...>::type,
    std::false_type
    >::type
    {};
    
}}}


namespace json { namespace utility { namespace mpl {
    
    
    // TODO
    
    template <typename...>
    struct is_unique;
    
    
    namespace test {
        //
        //        static_assert(is_unique<int>::value == true, "");
        //        static_assert(is_unique<int, char>::value == true, "");
        //        static_assert(is_unique<int, int>::value == false, "");
        //        static_assert(is_unique<int, int, char>::value == false, "");
        //        static_assert(is_unique<int, int, int, char>::value == false, "");
        //        static_assert(is_unique<int, int, int, int>::value == false, "");
        //        static_assert(is_unique<char, short, int, long>::value == true, "");
        //        static_assert(is_unique<char, short, int, long, char>::value == false, "");
    }
}}}


#pragma mark min_, max_
namespace json { namespace utility { namespace mpl {
    
    template <typename T, T...>
    struct max_;
    
    template <typename T, T t>
    struct max_<T, t> : std::integral_constant<T, t> {};
    
    template <typename T, T Head, T... Tail>
    struct max_<T, Head, Tail...> : std::integral_constant<T, (Head >= max_<T, Tail...>::value ? Head : max_<T, Tail...>::value)>
    {
    };
    
    
    
    template <typename T, T...>
    struct min_;
    
    template <typename T, T t>
    struct min_<T, t> : std::integral_constant<T, t> {};
    
    template <typename T, T Head, T... Tail>
    struct min_<T, Head, Tail...> : std::integral_constant<T, (Head <= min_<T, Tail...>::value ? Head : min_<T, Tail...>::value)>
    {
    };
    
    
    namespace test_max {
        
        static_assert( max_<int, 0,1,2,3,4,5,6,7,8,9>::value == 9 ,"");
        static_assert( max_<int, 9,0,1,2,3,4,5,6,7,8>::value == 9 ,"");
        static_assert( max_<int, 0,1,2,3,4,9,5,6,7,8>::value == 9 ,"");
        static_assert( max_<int, 0,9,1,8,2,7,3,6,4,5>::value == 9 ,"");
        static_assert( max_<int, 5,4,6,3,7,2,8,1,9,0>::value == 9 ,"");
        
        // transform function
        template <typename T>
        struct sizeof_ {
            static constexpr std::size_t value = sizeof(T);
        };
        
        template <typename... Args>
        using max_sizeof = typename max_<std::size_t, sizeof_<Args>::value...>::type;
        
        static_assert(max_sizeof<char, short, int, long>::value == sizeof(long), "");
        
    }
    
    
    
    namespace test_min {
        
        static_assert( min_<int, 1,2,3,4,5,6,7,8,9,0>::value == 0 ,"");
        static_assert( min_<int, 0,1,2,3,4,5,6,7,8,9>::value == 0 ,"");
        static_assert( min_<int, 1,2,3,4,5,0,6,7,8,9>::value == 0 ,"");
        static_assert( min_<int, 0,9,1,8,2,7,3,6,4,5>::value == 0 ,"");
        static_assert( min_<int, 5,4,6,3,7,2,8,1,9,0>::value == 0 ,"");
        
        // transform function
        template <typename T>
        struct sizeof_ {
            static constexpr std::size_t value = sizeof(T);
        };
        
        template <typename... Args>
        using min_sizeof = typename min_<std::size_t, sizeof_<Args>::value...>::type;
        
        static_assert(min_sizeof<char, short, int, long>::value == sizeof(char), "");
        
    }
    
    
    
}}}


#pragma mark join
namespace json { namespace utility { namespace mpl {
    
    
    template <typename...>
    struct join;
    
    template <template <typename...> class S,
    typename ...Args1,
    typename ...Args2>
    struct join<S<Args1...>, S<Args2...>>
    {
        typedef S<Args1..., Args2...> type;
    };
    
    template <template <typename...> class S,
    typename ...Args1,
    typename ...Args2,
    typename ...Tail>
    struct join<S<Args1...>, S<Args2...>, Tail...>
    {
        typedef typename join<S<Args1..., Args2...>, Tail...>::type type;
    };
    
}}}


#pragma mark at
namespace json { namespace utility { namespace mpl {
    
    namespace detail {
        
        template <int N, int I, typename...>
        struct _At;
        
        template <int N, int I, typename...>
        struct _At
        {
            typedef void type;
        };
        
        template <int N, int I, typename Head, typename... Tail>
        struct _At<N, I, Head, Tail...>
        {
            typedef typename  std::conditional<N==I,
            Head,
            typename _At<N+1, I, Tail...>::type
            >::type type;
        };
    }
    
    template <int I, typename...Args>
    struct at : detail::_At<0, I, Args...>
    {
    };
    
    namespace test {
        
        static_assert( std::is_same<void, mpl::at<0>::type>::value , "");
        static_assert( std::is_same<int, mpl::at<0, int>::type>::value , "");
        static_assert( std::is_same<void, mpl::at<1, int>::type>::value , "");
        static_assert( std::is_same<int, mpl::at<0, int, short>::type>::value , "");
        static_assert( std::is_same<short, mpl::at<1, int, short>::type>::value , "");
        static_assert( std::is_same<void, mpl::at<2, int, short>::type>::value , "");
        
    }
    
    
}}}



#pragma mark index_of

namespace json { namespace utility { namespace mpl {
    
    namespace detail {
        
        template <int N, typename...>
        struct _Index_of;
        
        template <int N, typename T>
        struct _Index_of<N,T> : std::integral_constant<int, N> {};
        
        template <int N, typename T, typename Head, typename... Tail>
        struct _Index_of<N, T, Head, Tail...>
        : std::conditional<
        std::is_same<T, Head>::value,
        std::integral_constant<int, N>,
        typename _Index_of<N+1, T, Tail...>::type
        >::type
        {
        };
        
    }
    
    template <typename T, typename... Args>
    struct index_of : detail::_Index_of<0, T, Args...>
    {
    };
    
}}}

namespace json { namespace utility { namespace mpl {
    
    using mpl::first;
    using mpl::last;
    
    namespace test {
        
        static_assert( std::is_same<long, typename first<long>::type >::value, "");
        static_assert( std::is_same<long, typename first<long, int>::type >::value, "");
        static_assert( std::is_same<long, typename first<long, int, short>::type >::value, "");
        
        static_assert( std::is_same<long, typename last<long>::type >::value, "");
        static_assert( std::is_same<int, typename last<long, int>::type >::value, "");
        static_assert( std::is_same<short, typename last<long, int, short>::type >::value, "");
        
        
        using mpl::join;
        template <typename... Args> struct S {};
        static_assert( std::is_same<S<int, short, long>, join<S<int>, S<short>, S<long>>::type >::value, "");
        
        
    }
    
}}}


#pragma mark bind
namespace json { namespace utility { namespace mpl {
    
    template <template <typename, typename...> class Op, typename... Args>
    struct bind {
        template <typename T>
        using type = Op<T, Args...>;
    };
    
}}}

#pragma mark find_first
namespace json { namespace utility { namespace mpl {
    
    
    template <template <typename> class Pred, typename...>
    struct find_first;
    
    template <template <typename> class Pred>
    struct find_first<Pred> {
        typedef void type;
    };
    
    template <template <typename> class Pred, typename Head, typename... Tail>
    struct find_first<Pred, Head, Tail...> {
        typedef typename std::conditional<
        Pred<Head>::value,
        Head,
        typename find_first<Pred, Tail...>::type
        >::type type;
        
    };
    
    namespace find_first_test {
        
        template <typename T>
        struct is_size_1 : std::integral_constant<bool, sizeof(T) == 1>
        {
        };
        
        template <size_t N>
        struct size {
            template <typename T>
            struct equals_sizeof : std::conditional<sizeof(T) == N, std::true_type, std::false_type>::type {
            };
        };
        
        static_assert( size<1>::equals_sizeof<char>::value, "");
        
        static_assert(std::is_same<char, typename find_first<size<1>::equals_sizeof, char, int, long, double>::type>::value, "");
        static_assert(std::is_same<char, typename find_first<size<1>::equals_sizeof, long, char, int, double>::type>::value, "");
        static_assert(std::is_same<char, typename find_first<size<1>::equals_sizeof, long, int, double, char>::type>::value, "");
        
        
        
        template <template <typename> class Pred, typename... Args>
        using findFirst = typename find_first<Pred, Args...>::type;
        
        static_assert(std::is_same<
                      void,
                      findFirst<is_size_1>
                      >::value, "");
        
        static_assert(std::is_same<
                      char,
                      findFirst<is_size_1, char>
                      >::value, "");
        
        static_assert(std::is_same<
                      char,
                      findFirst<is_size_1, char, int, long>
                      >::value, "");
        
        static_assert(std::is_same<
                      char,
                      findFirst<is_size_1, int, long, char>
                      >::value, "");
        
        static_assert(std::is_same<
                      char,
                      findFirst<is_size_1, int, char, long>
                      >::value, "");
        
    }
        
}}}


#pragma mark reduce
namespace json { namespace utility { namespace mpl {
    
    
    
    template <
    template<typename Rhv, typename Lhv> class BinaryOp,
    typename...
    >
    struct reduce;
    
    template <
    template<typename Rhv, typename Lhv> class BinaryOp,
    typename T>
    struct reduce<BinaryOp, T> : T
    {
    };
    
    template <
    template<typename Rhv, typename Lhv> class BinaryOp,
    typename Head, typename... Tail
    >
    struct reduce<BinaryOp, Head, Tail...>
    : BinaryOp<Head, typename reduce<BinaryOp, Tail...>::type>::type
    {
    };
    
    
    namespace test {
        
        template <typename Rhv, typename Lhv>
        struct sum : std::integral_constant<int, Rhv::value + Lhv::value>
        {
        };
        
        
        template <int N>
        using int_ = typename std::integral_constant<int, N>::type;
        
        static_assert(std::is_same<std::integral_constant<int, 1>, int_<1> >::value == 1, "");
        static_assert(int_<1>::value == 1, "");
        static_assert(int_<1>::value == std::integral_constant<int, 1>::value, "");
        
        static_assert( reduce<sum, int_<1>, int_<1> >::value == 2  , "");
        
    }
    
}}}



#pragma mark count_if
namespace json { namespace utility { namespace mpl {
    
    template <
    template<typename> class UnaryPred,
    typename...
    >
    struct count_if;
    
    template <
    template<typename> class UnaryPred,
    typename T
    >
    struct count_if<UnaryPred, T> : std::conditional<UnaryPred<T>::value,
    std::integral_constant<int, 1>,
    std::integral_constant<int, 0>>::type
    {
    };
    
    template <
    template<typename> class UnaryPred,
    typename Head, typename... Tail
    >
    struct count_if<UnaryPred, Head, Tail...> : std::conditional<UnaryPred<Head>::value,
    std::integral_constant<int, 1 + count_if<UnaryPred, Tail...>::value >,
    std::integral_constant<int, count_if<UnaryPred, Tail...>::value>>::type
    {
    };
    
    
    /**
    namespace test {
        
    #include <string>
        
        template <typename T>
        struct is_int : std::is_same<int, T>::type
        {
        };
        
        static_assert(count_if<is_int, char>::value == 0 ,"");
        static_assert(count_if<is_int, int>::value == 1 ,"");
        static_assert(count_if<is_int, int, char>::value == 1 ,"");
        static_assert(count_if<is_int, char, int>::value == 1 ,"");
        static_assert(count_if<is_int, int, char, int>::value == 2 ,"");
        static_assert(count_if<is_int, float, char, double>::value == 0 ,"");
        
        
        template <typename... Args>
        struct is_constructible
        {
            template <typename T>
            using to = typename std::is_constructible<T, Args...>::type;
        };
        
        static_assert(count_if<is_constructible<int>::to, int>::value == 1 ,"");
        static_assert(count_if<is_constructible<int>::to, char>::value == 1 ,"");
        
        static_assert(count_if<is_constructible<std::size_t, std::string>::to,
                      std::vector<std::string>, int, char, float, double, std::map<int, int>
                      >::value == 1 ,"");
        
        static_assert(count_if<is_constructible<std::size_t, std::string>::to,
                      std::vector<int>, int, char, float, double, std::map<int, int>
                      >::value == 0 ,"");
        
        static_assert(count_if<is_constructible<std::initializer_list<int>>::to,
                      std::vector<int>, int, char, float, double, std::map<int, int>
                      >::value == 1 ,"");
        
    }
     */
    
    
}}}



#pragma mark count
namespace json { namespace utility { namespace mpl {
    
    template <
    typename Type,
    typename...
    >
    struct count;
    
    template <typename Type, typename T>
    struct count<Type, T> : std::conditional<
    std::is_same<Type, T>::value,
    std::integral_constant<int, 1>,
    std::integral_constant<int, 0>
    >::type
    {
    };
    
    template <
    typename Type,
    typename Head, typename... Tail
    >
    struct count<Type, Head, Tail...> : std::conditional<std::is_same<Type, Head>::value,
    std::integral_constant<int, 1 + count<Type, Tail...>::value >,
    std::integral_constant<int, count<Type, Tail...>::value>>::type
    {
    };
    
    
    namespace test {
        
        static_assert(count<int, char>::value == 0 ,"");
        static_assert(count<int, int>::value == 1 ,"");
        static_assert(count<int, int, char>::value == 1 ,"");
        static_assert(count<int, char, int>::value == 1 ,"");
        static_assert(count<int, int, char, int>::value == 2 ,"");
        static_assert(count<int, float, char, double>::value == 0 ,"");
        
        static_assert( count<std::true_type, std::is_constructible<int, int>::type >::value == 1, "");
    }
    
}}}


#pragma mark any_of
namespace json { namespace utility { namespace mpl {
    
    template <typename...>
    struct any_of;
    
    template <typename T>
    struct any_of<T> : std::conditional<T::value == true, std::true_type, std::false_type>::type
    {};
    
    template <typename Head, typename... Tail>
    struct any_of<Head, Tail...> : std::conditional<
    Head::value == true or any_of<Tail...>::value,
    std::true_type,
    std::false_type>::type
    {};
    
    
    namespace test {
        
        static_assert(any_of<std::true_type>::value == true ,"");
        static_assert(any_of<std::false_type>::value == false ,"");
        static_assert(any_of<std::true_type, std::false_type>::value == true ,"");
        static_assert(any_of<std::false_type, std::true_type>::value == true ,"");
        static_assert(any_of<std::false_type, std::false_type, std::false_type>::value == false ,"");
        static_assert(any_of<std::true_type, std::true_type, std::true_type>::value == true ,"");
        
    }
    
    
}}}


#pragma mark all_of
namespace json { namespace utility { namespace mpl {
    
    template <typename...>
    struct all_of;
    
    template <typename T>
    struct all_of<T> : std::conditional<T::value == true, std::true_type, std::false_type>::type
    {};
    
    template <typename Head, typename... Tail>
    struct all_of<Head, Tail...> : std::conditional<
    Head::value == true and all_of<Tail...>::value,
    std::true_type,
    std::false_type>::type
    {};
    
    
    namespace test {
        
        static_assert(all_of<std::true_type>::value == true ,"");
        static_assert(all_of<std::false_type>::value == false ,"");
        static_assert(all_of<std::true_type, std::false_type>::value == false ,"");
        static_assert(all_of<std::false_type, std::true_type>::value == false ,"");
        static_assert(all_of<std::false_type, std::false_type, std::false_type>::value == false ,"");
        static_assert(all_of<std::true_type, std::true_type, std::true_type>::value == true ,"");
        
    }
    
    
}}}





#pragma mark is_list_constructible
namespace json { namespace utility { namespace mpl {
    
    namespace detail {
        
        template <typename T>
        struct ctor {
            static int select(T v);
        };
        
        template <typename T, typename...Args,
        typename IsConstructible = decltype(ctor<T>::select(std::declval<Args>...))
        >
        auto is_constructible(T, Args&&...) -> std::true_type;
        
        template <typename T, typename...Args>
        auto is_constructible(T, Args&&...) -> std::false_type;
        
    }
    
    template <typename T, typename...>
    struct is_list_constructible;
    
    template <typename T, typename...Args>
    struct is_list_constructible : decltype(detail::is_constructible(std::declval<T>(), std::declval<Args>()...))
    {
    };
    
    
    
    namespace test {
        
        
        static_assert( std::is_constructible<int, int>::value == true , "");
        
        
    }
    
}}}


#include <string>
namespace test { namespace mpl {
    
    
    template <typename...>
    struct find_first_constructible;
    
    template <typename T>
    struct find_first_constructible<T>
    {
        template <typename... Args>
        using with = typename std::conditional<
        std::is_constructible<T, Args...>::value,
        T,
        void
        >::type;
    };
    
    template <typename Head, typename... Tail>
    struct find_first_constructible<Head, Tail...>
    {
        template <typename... Args>
        using with = typename std::conditional<
        std::is_constructible<Head, Args...>::value,
        Head,
        typename find_first_constructible<Tail...>::template with<Args...>
        >::type;
    };
    
    
    template <typename... Types>
    struct is_any_constructible
    {
        template <typename... Args>
        using with = typename std::conditional<
        std::is_same<
        void,
        typename find_first_constructible<Types...>::template with<Args...>
        >::value,
        std::false_type,
        std::true_type
        >::type;
        
    };
    
    
    struct A {};
    struct B {};
    
    
    static_assert( std::is_same<std::string,
                  find_first_constructible<std::string>::template with<decltype("abc")> >::value, "");
    static_assert( std::is_same<std::string,
                  find_first_constructible<std::string, A, B>::template with<decltype("abc")> >::value, "");
    static_assert( std::is_same<std::string,
                  find_first_constructible<A, B, std::string>::template with<decltype("abc")> >::value, "");
    static_assert( std::is_same<void,
                  find_first_constructible<int, long, double>::template with<decltype("abc")> >::value, "");
    static_assert( std::is_same<A,
                  find_first_constructible<A, B, std::string>::template with<A> >::value, "");
    
    static_assert( is_any_constructible<std::string>::with<std::string>::value, "");
    static_assert( is_any_constructible<std::string, int, long>::with<std::string>::value, "");
    static_assert( is_any_constructible<int, long, std::string>::with<std::string>::value, "");
    static_assert( is_any_constructible<int, std::string, long>::with<std::string>::value, "");
    static_assert( is_any_constructible<int, std::string, long>::with<decltype("ab")>::value, "");
    static_assert( is_any_constructible<int, std::string, long>::with<const char*>::value, "");
    static_assert( is_any_constructible<int, std::string, long>::with<const char*, const char*>::value, "");
    static_assert( is_any_constructible<std::string>::with<A>::value == false, "");
    static_assert( is_any_constructible<int, std::string>::with<A>::value == false, "");
    static_assert( is_any_constructible<std::string, int>::with<A>::value == false, "");
    static_assert( is_any_constructible<A, B, int>::with<A>::value, "");
    
    
    
    // is_homogenous
    using json::utility::mpl::is_homogenous;
    static_assert(is_homogenous<int>::value == true, "");
    static_assert(is_homogenous<int, int>::value == true, "");
    static_assert(is_homogenous<int, int>::value == true, "");
    static_assert(is_homogenous<int, int, int>::value == true, "");
    static_assert(is_homogenous<int, int, int, int>::value == true, "");
    
    
}}





#endif // JSON_UTILITY_MPL_HPP
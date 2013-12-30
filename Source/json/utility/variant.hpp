//
//  variant.hpp
//
//  Created by Andreas Grosam on 13.02.13.
//  Copyright (c) 2013 Andreas
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


// STATUS: alpha


#ifndef JSON_UTILITY_VARIANT_HPP
#define JSON_UTILITY_VARIANT_HPP

#include "json/config.hpp"
#include <type_traits>
#include <utility>  // swap
#include <stdexcept>
#include <cassert>


//#define VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION
#define NO_INHERITING_CONSTRUCTORS


#if defined (HAS_MPL_HEADER)
#include "mpl.hpp"
#else
namespace json {namespace utility { namespace mpl {
    
    
#pragma mark mpl::any_of
    template <typename...>
    struct any_of;
    
    template <typename T>
    struct any_of<T> : std::conditional<T::value == true,
        std::true_type, std::false_type>::type
    {};
    
    template <typename Head, typename... Tail>
    struct any_of<Head, Tail...> : std::conditional<
        Head::value == true or any_of<Tail...>::value,
        std::true_type,
        std::false_type>::type
    {};
    
    
    
#pragma mark mpl::all_of
    template <typename...>
    struct all_of;
    
    template <typename T>
    struct all_of<T> : std::conditional<T::value == true,
        std::true_type, std::false_type>::type
    {};
    
    template <typename Head, typename... Tail>
    struct all_of<Head, Tail...> : std::conditional<
        Head::value == true and all_of<Tail...>::value,
        std::true_type,
        std::false_type>::type
    {};
    
    
#pragma mark mpl::max_
    
    template <typename T, T...>
    struct max_;
    
    template <typename T, T t>
    struct max_<T, t> : std::integral_constant<T, t> {};
    
    template <typename T, T Head, T... Tail>
    struct max_<T, Head, Tail...> : std::integral_constant<
        T, (Head >= max_<T, Tail...>::value ? Head : max_<T, Tail...>::value)>
    {
    };
    
    
    
#pragma mark mpl::first
    
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
    
    
    
#pragma mark mpl::at
    
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
    
    
    
#pragma mark mpl::index_of
    
    
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
    using index_of = typename detail::_Index_of<0, T, Args...>::type;
    
    
}}} // namespace json::utility::mpl
#endif


namespace json { namespace utility { namespace detail {
    
    
    // uses-allocator construction
    
    template <class T, class Alloc, class... Args>
    struct uses_alloc_ctor_imp  {
        static const bool uses_allocator_ = std::uses_allocator<T, Alloc>::value;
        static const bool is_constructible_ = std::is_constructible<T, std::allocator_arg_t, Alloc, Args...>::value;
        static const int value = uses_allocator_ ? 2 - is_constructible_ : 0;
    };
    
    template <class T, class Alloc, class... Args>
    struct uses_alloc_ctor
    : std::integral_constant<int, uses_alloc_ctor_imp<T, Alloc, Args...>::value>
    {};
    
    
    
}}}

namespace json { namespace utility {
    
    // tag used to disambiguate forwarding constructors which "in place" construct
    // the type with the given arguments. emplace_t's template paramater indicates
    // which implementation type (one of [Types...]) shall be selected when constructing
    // the variant.
    template <typename T>
    struct emplace_t{};
    
    
    // class bad_variant_access
    class bad_variant_access : public std::logic_error {
    public:
        explicit bad_variant_access(const std::string& what_arg) : std::logic_error{what_arg} {}
        explicit bad_variant_access(const char* what_arg) : std::logic_error{what_arg} {}
    };
    
    
    // Handy helper template aliases
    namespace detail {
        
        template<typename T>
        using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        
        template<typename T>
        using UnqualifiedRemovedPointer = typename std::remove_pointer<Unqualified<T>>::type;
        
        
        template <typename T>
        struct sizeof_ {
            static const constexpr std::size_t value = sizeof(T);
        };
        
        template <typename... Args>
        using max_sizeof = typename mpl::max_<std::size_t, sizeof_<Args>::value...>::type;
        
        template <typename... Args>
        using max_alignof = typename mpl::max_<std::size_t, std::alignment_of<Args>::value...>::type;
    
    }
    
        
    // Aligned Storage
    namespace detail {
        
        // TODO: specialize _AlignedStorage for trivial types? Make
        // c-tors constexpr. That way, we could define constexpr variants.
        template<typename... Types>
        struct _AlignedStorage
        {
            void* address() { return &storage_; }
            void const* address() const { return &storage_; }
            
        private:
            typedef typename std::aligned_storage<
            max_sizeof<Types...>::value,
            max_alignof<Types...>::value
            >::type storage_type;
            storage_type storage_;
        };
        
    }
    
    
    // Selector
    namespace detail {

        // Helper classes to deduce the implementation type T in [Types...] which
        // shall be selected for construction for a given set of arguments.
        // Currently this works only for *one* argument (and possibly
        // "tagged adorned" ctors, e.g. elmplace_t and allocator_arg_t).
        
        constexpr static struct no_alloc_t {} no_alloc{};
        
        template <int N, typename T>
        struct _Result {
            static constexpr const int value = N;
            typedef T type;
        };
        
#if !defined (NO_INHERITING_CONSTRUCTORS)
        // TODO: C++11: use inherited constructors, when supported.
        //
        //#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
        //        template <class Alloc, int Which, typename T, typename... Ts>
        //        struct _Selector<Alloc, Which, T, Ts...> : _Selector<Alloc, Which+1, Ts...>
        //        {
        //            using _Selector<Alloc, Which+1, Ts...>::__select;
        //            static constexpr _Result<Which, T> __select(T const&, const Alloc&);
        //            static constexpr _Result<Which, T> __select(std::allocator_arg_t, const Alloc&, T const&);
        //        };
        //#endif
#else
        // *Effectively* defines a class with a set of overloaded member functions
        // `__select(const T&)` for each type in Types.
        template <int Which, typename...>
        struct _Selector;
        
        template <int Which>
        struct _Selector<Which> {
            static constexpr void __select();  // never select
        };
        
        template <int Which, typename T, typename... Ts>
        struct _Selector<Which, T, Ts...> : _Selector<Which+1, Ts...>
        {
            using _Selector<Which+1, Ts...>::__select;
            static constexpr auto __select(T const&) -> decltype(_Result<Which, T>());
        };
#endif
        
        // The static member function `select()`'s return type equals class
        // template '_Result'.
        // Invoking it in a `decltype` operator with a parameter of type `Arg`,
        // e.g.: `decltype(select(std::declval<Arg>()))`, yields a `_Result` type
        // whose embedded type `type` equals the type `T` in [Types...]. This type
        // will be selected by the compiler through overload resolution when implicitly
        // converting `Arg` to `T`, for all `T` in [Types]. `_Result`'s static member
        // `value` equals the index of `T` in `[Types...]`.
        template <typename... Types>
        struct selector
        {
            typedef detail::_Selector<0, Types...> _Sel;
            
#if !defined (NO_INHERITING_CONSTRUCTORS)
        // TODO: C++11: use inherited constructors, when supported.
#else
  #if !defined (USE_NARROW_SELECT)
            template <typename U>
            static constexpr auto select(U const&)
                -> decltype(_Sel::__select(std::declval<U>()));
  #else
            template <typename U>
            static constexpr auto narrow_select(U const&)
                -> decltype(_Sel::__select({std::declval<U>()}));
  #endif
#endif
        };
        
    }  // detail
    

//    template <typename... Args>
//    struct variant_traits {
//        typedef int which_type;
//    };
//    

    typedef int which_type;
    
    
    
#pragma mark - class variant
    
    template <typename... Types>
    class variant
    {
        //typedef typename variant_traits<Types...>::which_type which_type;
        typedef typename mpl::first<Types...>::type First;
        typedef detail::_AlignedStorage<Types...> storage_type;
        typedef detail::selector<Types...> sel;
        
        
        // The following template aliases are used internally for convenience:
#if !defined (USE_NARROW_SELECT)
        template <typename... Args>  
        using Select = decltype(sel::select(std::declval<Args>()...));
#else
        template <typename... Args>
        using Select = decltype(sel::narrow_select(std::declval<Args>()...));
#endif
        
        template <typename... Args>
        using IsViableImp = typename std::integral_constant<bool,
            !std::is_same<variant, detail::Unqualified<typename mpl::first<Args...>::type>>::value
            //and mpl::any_of<std::is_convertible<T, Types>...>::value
            and mpl::any_of<std::is_constructible<Types, Args...>...>::value
        >::type;
        
        template <typename T>
        using IndexOf = typename mpl::index_of<T, Types...>::type;
        
        template <typename T>
        using IsImpType = typename mpl::any_of<std::is_same<Types, T>...>::type;
        
        
    public:
        
        // type traits helper
        // (may be used by clients in order to check if a type is an element in [Types...])
        template <typename T>
        struct contains_type : std::integral_constant<bool, mpl::any_of<std::is_same<Types, T>...>::value>
        {
        };
        
        
        // Default Constructor
        variant() noexcept (std::is_nothrow_default_constructible<First>::value)
        : which_(0)
        {
            this->template construct<First>();
        }
        
        // Copy Constructor
        variant(variant const& other)
            noexcept(mpl::all_of<std::is_nothrow_copy_constructible<Types>...>::value)
        : which_(other.which_)
        {
            copy_constructor_visitor copy_ctor;
            other.apply_visitor(copy_ctor, *this);
        }
        
        // Move Constructor
        variant(variant&& other)
            noexcept(mpl::all_of<std::is_nothrow_move_constructible<Types>...>::value)
        : which_(other.which_)
        {
            move_constructor_visitor move_ctor;
            other.apply_visitor(move_ctor, *this);
        }
        
#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
        // allocator-extended constructor
        template <class Alloc,
            typename Enable = typename std::enable_if<std::uses_allocator<First, Alloc>::value>::type
        >
        variant(std::allocator_arg_t, const Alloc& a)
            noexcept (std::is_nothrow_default_constructible<First>::value)
        : which_(0)
        {
            this->template construct<First>(std::allocator_arg, a);
        }
        
        // allocator-extended copy constructor
        template <class Alloc>
        variant(std::allocator_arg_t, const Alloc& a, variant const& other)
            noexcept(mpl::all_of<std::is_nothrow_copy_constructible<Types>...>::value)
        : which_(other.which_)
        {
            copy_constructor_visitor copy_ctor;
            other.apply_visitor(copy_ctor, *this, a);
        }
        
        // allocator-extended move constructor
        template <typename Alloc>
        variant(std::allocator_arg_t, const Alloc& a, variant&& other)
            noexcept(mpl::all_of<std::is_nothrow_move_constructible<Types>...>::value)
        : which_(other.which_)
        {
            move_constructor_visitor move_ctor;
            other.apply_visitor(move_ctor, *this, a);
        }
#endif
        
        ~variant() noexcept(mpl::all_of<std::is_nothrow_destructible<Types>...>::value)
        {
            invoke_destructor();
        }
        
        
        
        // Forwarding constructor
#if !defined (NO_INHERITING_CONSTRUCTORS)
        template <typename... Args,
            typename IsViable = typename std::enable_if<IsViableImp<Args...>::value>::type,
            typename Which = Select<Args...>
        >
        variant(Args&&... args) noexcept(std::is_nothrow_constructible<typename Which::type, Args...>::value)
        : which_(Which::value)
        {
            construct<typename Which::type>(std::forward<Args>(args)...);
        }
#else
#define VARIANT_NO_MULTIPLE_ARGS_CTOR        
        
        // Allow only one argument
        template <typename T,
            typename IsViable = typename std::enable_if<IsViableImp<T>::value>::type,
            typename Which = Select<T>
        >
        variant(T&& v) noexcept(std::is_nothrow_constructible<typename Which::type, T>::value)
        : which_(Which::value)
        {
            construct<typename Which::type>(std::forward<T>(v));
        }        
#endif
        
        // Explicit emplace 
        template <typename T, typename... Args,
            typename IsViable = typename std::enable_if<
                IsImpType<T>::value
                and std::is_constructible<T, Args...>::value  // TODO: check
            >::type
        >
        variant(emplace_t<T>, Args&&... args)
        : which_(IndexOf<T>::value)
        {
            construct<T>(std::forward<Args>(args)...);
        }
        
        template <typename T,
            typename ValueType = typename T::value_type,
            typename IsViable = typename std::enable_if<
                IsImpType<T>::value
                and std::is_constructible<T, std::initializer_list<ValueType>>::value
            >::type
        >
        variant(emplace_t<T>, std::initializer_list<ValueType> il)
        : which_(IndexOf<T>::value)
        {
            construct<T>(il);
        }
        
        
        //
        // Assignment
        //
        
        variant& operator=(variant const& rhv) noexcept(mpl::all_of<std::is_nothrow_copy_constructible<Types>...>::value &&
                                                        mpl::all_of<std::is_nothrow_copy_assignable<Types>...>::value)
        {
            copy_assignment_visitor copy_assigner;
            rhv.apply_visitor(copy_assigner, *this);
            return *this;
        }
        
        variant& operator=(variant&& rhv) noexcept(mpl::all_of<std::is_nothrow_move_constructible<Types>...>::value &&
                                                   mpl::all_of<std::is_nothrow_move_assignable<Types&>...>::value)
        {
            move_assignment_visitor move_assigner;
            rhv.apply_visitor(move_assigner, *this);
            return *this;
        }
        
        template <typename T,
            typename IsViable = typename std::enable_if<
                IsViableImp<T>::value
            >::type,
            typename Which = Select<T>
        >
        variant& operator=(T&& rhv) noexcept(std::is_nothrow_constructible<typename Which::type, T>::value &&
                                               std::is_nothrow_assignable<typename Which::type&, T>::value)
        {
            if (which() == Which::value) {
                assign<typename Which::type>(std::forward<T>(rhv));
            }
            else {
                reassign<typename Which::type>(std::forward<T>(rhv));
            }
            return *this;
        }
        
        
        template <typename T, typename... Args,
            typename IsViable = typename std::enable_if<
                IsImpType<T>::value
                and std::is_constructible<T, Args...>::value
            >::type
        >
        void emplace(Args&&... args)
        {
            invoke_destructor();
            construct<T>(std::forward<Args>(args)...);
            which_ = IndexOf<T>::value;
        }
        
        
        template <typename T,
            typename ValueType = typename T::value_type,
            typename IsViable = typename std::enable_if<
                IsImpType<T>::value
                and std::is_constructible<T, std::initializer_list<ValueType>>::value
            >::type
        >
        void emplace(std::initializer_list<ValueType> il)
        {
            invoke_destructor();
            construct<T>(il);
            which_ = IndexOf<T>::value;
        }
        
        
        
        //
        //  Observer
        //
        
        int which() const noexcept { return which_; }
        
        
        template <typename U>
        bool is_type() const  noexcept
        {
            constexpr int which = mpl::index_of<U, Types...>::value;
            return which_ == which;
        }
        
        
        
        //
        //  Accessor
        //
        
        template <typename T,
            typename Enable = typename std::enable_if<
                std::is_pointer<T>::value
                and IsImpType<detail::UnqualifiedRemovedPointer<T>>::value
            >::type
        >
        detail::UnqualifiedRemovedPointer<T>* as() noexcept
        {
            return accessor<detail::UnqualifiedRemovedPointer<T>>::pointer(storage_, which_);
        }
        
        template <typename T,
            typename Enable = typename std::enable_if<
                std::is_pointer<T>::value
                and IsImpType<detail::UnqualifiedRemovedPointer<T>>::value
            >::type
        >
        detail::UnqualifiedRemovedPointer<T> const* as() const noexcept {
            return accessor<detail::UnqualifiedRemovedPointer<T>>::pointer(storage_, which_);
        }
        
        
        template <typename T,
            typename Enable = typename std::enable_if<
                !std::is_pointer<T>::value
                and IsImpType<detail::Unqualified<T>>::value
            >::type
        >
        detail::Unqualified<T>& as()
        {
            return accessor<detail::Unqualified<T>>::reference(storage_, which_);
        }
        
        template <typename T,
            typename Enable = typename std::enable_if<
                !std::is_pointer<T>::value
                and IsImpType<detail::Unqualified<T>>::value
            >::type
        >
        detail::Unqualified<T> const& as() const {
            return accessor<detail::Unqualified<T>>::reference(storage_, which_);
        }
        
        
        
        template <typename T,
            typename Enable = typename std::enable_if<
                IsImpType<T>::value
            >::type
        >
        T& interpret_as() noexcept
        {
            //assert( is_type<T>() );
            return *accessor<T>::pointer(storage_, which_);
        }
        
        template <typename T,
            typename Enable = typename std::enable_if<
                IsImpType<T>::value
            >::type
        >
        T const& interpret_as() const noexcept {
            //assert( is_type<T>() );
            return *accessor<T>::pointer(storage_, which_);
        }
        
        
        //
        // Comparison
        //
        
        bool equal(variant const& rhv) const noexcept {
            is_equal_visitor comp;
            return apply_visitor(comp, rhv, *this);
        }
        
        
        //
        //  Visitor support
        //
        
        template <typename Visitor, typename... Args>
        typename Visitor::result_type
        apply_visitor(Visitor& visitor, Args&&... args) {
            return visitor_dispatcher<Visitor, storage_type, Args...>()(which_, visitor, storage_, std::forward<Args>(args)...);
        }
        
        template <typename Visitor, typename... Args>
        typename Visitor::result_type
        apply_visitor(Visitor& visitor, Args&&... args) const {
            return visitor_dispatcher<Visitor, const storage_type, Args...>()(which_, visitor, storage_, std::forward<Args>(args)...);
        }
        
        
        
        // Comparison
        bool is_equal(variant const& rhv) const {
            is_equal_visitor is_equal;
            return apply_visitor(is_equal, rhv);
        }
        
        
        
        // swap
        void swap(variant& rhv) noexcept(mpl::all_of<std::is_nothrow_move_constructible<Types>...>::value
                                         and mpl::all_of<std::is_nothrow_destructible<Types>...>::value)
        {
            swap_visitor swapper;
            apply_visitor(swapper, *this, rhv);
        }
        
        
    private:
#pragma mark - construct


#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
        
        template <typename T, typename Alloc>
        void __construct(std::integral_constant<int, 0>, const Alloc&) {
            ::new (storage_.address()) T();
        }
        
        template <typename T, typename Alloc>
        void __construct(std::integral_constant<int, 1>, const Alloc& a) {
            ::new (storage_.address()) T(std::allocator_arg_t(), a);
        }
        
        template <typename T, typename Alloc>
        void __construct(std::integral_constant<int, 2>, const Alloc& a) {
            ::new (storage_.address()) T(a);
        }
        
        template <typename T, typename Alloc, typename... Args,
            typename Enable = typename std::enable_if<std::is_constructible<T, Args...>::value>::type
        >
        void __construct(std::integral_constant<int, 0>, const Alloc&, Args&&... args) {
            ::new (storage_.address()) T(std::forward<Args>(args)...);
        }
        
        template <typename T, typename Alloc, typename... Args,
            typename Enable = typename std::enable_if<std::is_constructible<T, Args...>::value>::type
        >
        void __construct(std::integral_constant<int, 1>, const Alloc& a, Args&&... args) {
            ::new (storage_.address()) T(std::allocator_arg_t(), a, std::forward<Args>(args)...);
        }
        
        template <typename T, typename Alloc, typename... Args,
            typename Enable = typename std::enable_if<std::is_constructible<T, Args...>::value>::type
        >
        void __construct(std::integral_constant<int, 2>, const Alloc& a, Args&&... args) {
            ::new (storage_.address()) T(std::forward<Args>(args)..., a);
        }
        
        // Should we enable this function only if T uses allocator Alloc???
        template <typename T, typename Alloc, typename... Args>
        void construct(std::allocator_arg_t, Alloc const& a, Args&&... args)
        {
            // Dispatch to the appropriate extended allocator ctor for type T:
            // Can we utilize std::allocator_traits here???
            __construct<T>(detail::uses_alloc_ctor<T, Alloc, Args...>(), a, std::forward<Args>(args)...);
        }
        
        template <typename T, typename... Args>
        void construct(Args&&... args) {
            ::new (storage_.address()) T(std::forward<Args>(args)...);
        }
        
#else  // #if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
        template <typename T, typename... Args>
        void construct(Args&&... args) {
            ::new (storage_.address()) T(std::forward<Args>(args)...);
        }
#endif
        
        
        
        
        
#pragma mark -

        void invoke_destructor() {
            destructor_visitor dtor;
            apply_visitor(dtor);
        }
        
        template <typename T, typename Arg>
        void assign(Arg&& rhv) {
            assert( (which() == mpl::index_of<T, Types...>::value) );
            *reinterpret_cast<T*>(this->storage_.address()) = std::forward<Arg>(rhv);
        }
        
        
        template <typename T, typename Arg>
        void __reassign_nothrow(Arg&& rhv, std::true_type) {
            invoke_destructor();
            which_ = mpl::index_of<T, Types...>::value;
            construct<T>(std::forward<Arg>(rhv));
        }
        
        template <typename T, typename Arg>
        void __reassign_nothrow(Arg&& rhv, std::false_type) {
            T tmp(std::forward<Arg>(rhv));
            invoke_destructor();
            which_ = mpl::index_of<T, Types...>::value;
            construct<T>(std::move(tmp));
        }
        
        template <typename T, typename Arg>
        void reassign(Arg&& rhv) {
            __reassign_nothrow<T>(std::forward<Arg>(rhv), std::is_nothrow_constructible<T, Arg>());
        }
        
        
        template <typename T>
        bool __is_equal(T const& rhv) const {
            assert( (which() == mpl::index_of<T, Types...>::value) );
            return *reinterpret_cast<T const*>(this->storage_.address()) == rhv;
        }
        
        template <typename T>
        void __swap(variant& rhv) {
            assert( (which() == rhv.which()) );
            assert( (which() == mpl::index_of<T, Types...>::value) );
            using std::swap;
            swap(*reinterpret_cast<T*>(this->storage_.address()), *reinterpret_cast<T*>(rhv.storage_.address()));
            swap(which_, rhv.which_);
        }
        
        
        template <typename T, typename U>
        void __swap(variant& rhv)
        {
            assert( (which() == mpl::index_of<T, Types...>::value) );
            assert( (rhv.which() == mpl::index_of<U, Types...>::value) );
            T& vlh = *reinterpret_cast<T*>(this->storage_.address());
            T tmp = std::move(vlh);
            vlh.T::~T();
            U& vrh = *reinterpret_cast<U*>(rhv.storage_.address());
            this->template construct<U>(std::move(vrh));
            vrh.U::~U();
            rhv.template construct<T>(std::move(tmp));
            std::swap(which_, rhv.which_);
        }
        
        
        // Accessor Support
        
        static void __throw_bad_access() {
            throw bad_variant_access("bad type");
        }
        
        template <typename U,
            int Which = IndexOf<U>::value,
            typename Enable = typename std::enable_if<
                Which < sizeof...(Types) // <==> mpl::any_of<std::is_same<Types, U>...>::value
            >::type
        >
        struct accessor {
            
            static U* pointer(storage_type& storage, int which)
            {
                if (Which == which) {
                    return reinterpret_cast<U*>(storage.address());
                }
                else {
                    return nullptr;
                }
            }
            static U const* pointer(storage_type const& storage, int which)
            {
                if (Which == which) {
                    return reinterpret_cast<U const*>(storage.address());
                }
                else {
                    return nullptr;
                }
            }
            
            static inline U& reference(storage_type& storage, int which)
            {
                if (Which == which) {
                    return *reinterpret_cast<U*>(storage.address());
                }
                else {
                    __throw_bad_access();  // don't inline
                }
                __builtin_unreachable();
            }
            static inline U const& reference(storage_type const& storage, int which)
            {
                if (Which == which) {
                    return *reinterpret_cast<U const*>(storage.address());
                }
                else {
                    __throw_bad_access(); // don't inline
                }
                __builtin_unreachable();
            }
            
        };
        
        
        
        //
        // ostream support
        //
        
        friend inline std::ostream& operator<<(std::ostream& os, variant const& var) {
            ostream_visitor out;
            var.apply_visitor(out, os);
            return os;
        }
        
        
        // Visitor support
        
        // TODO: Optimize away const vs non-const in the jump table
        template <typename Visitor, typename Storage, typename T, typename... Args>
        static typename Visitor::result_type
        visit(Visitor& visitor, Storage& storage, Args&&... args) {
            static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
            return visitor(*reinterpret_cast<T*>(storage.address()), std::forward<Args>(args)...);
        }
        
        template <typename Visitor, typename Storage, typename T, typename... Args>
        static typename Visitor::result_type
        visit(Visitor& visitor, Storage const& storage, Args&&... args) {
            static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
            return visitor(*reinterpret_cast<T const*>(storage.address()), std::forward<Args>(args)...);
        }
        
        template <typename Visitor, typename Storage, typename... Args>
        struct visitor_dispatcher
        {
            typedef typename Visitor::result_type result_type;
            result_type operator()(int which, Visitor& visitor, Storage& storage, Args&&... args) const
            {
                typedef result_type (*visit_func) (Visitor&, Storage&, Args&&...);
                static visit_func f_table[sizeof...(Types)] = {
                    &visit<Visitor, Storage, Types, Args&&...>
                    ...
                };
                return (*f_table[which])(visitor, storage, std::forward<Args>(args)...);
            }
        };
        
        
        struct destructor_visitor
        {
            // TODO: use allocator_traits
            
            typedef void result_type;
            
            template <typename T>
            void operator()(T& v) const {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                v.T::~T();
            }
        };
        
        struct copy_constructor_visitor
        {
            typedef void result_type;
            
            template <typename T>
            void operator()(T const& arg, variant& var) const {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                var.construct<T>(arg);
            }
            
#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
            template <typename T, typename Alloc>
            void operator()(T const& arg, variant& var, Alloc& a) const {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                var.construct<T>(std::allocator_arg, a, arg);
            }
#endif
            
        };
        
        struct move_constructor_visitor
        {
            typedef void result_type;
            
            template <typename T>
            void operator()(T& arg, variant& var) const {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                var.construct<T>(std::move(arg));
            }
            
#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
            template <typename T, typename Alloc>
            void operator()(T& arg, variant& var, Alloc& a) const {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                var.construct<T>(std::allocator_arg, a, std::move(arg));
            }
#endif
        };
        
        
        struct copy_assignment_visitor
        {
            typedef void result_type;
            
            template <typename T>
            void operator()(T const& rhv, variant& lhv) const
            {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                constexpr int rhv_which = mpl::index_of<T, Types...>::value;
                if (rhv_which == lhv.which()) {
                    lhv.assign<T>(rhv);
                }
                else {
                    lhv.reassign<T>(rhv);
                }
            }
        };
        
        struct move_assignment_visitor
        {
            typedef void result_type;
            
            template <typename T>
            void operator()(T& rhv, variant& lhv) const {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                constexpr int rhv_which = mpl::index_of<T, Types...>::value;
                if (rhv_which == lhv.which()) {
                    lhv.assign<T>(std::move(rhv));
                }
                else {
                    lhv.reassign<T>(std::move(rhv));
                }
            }
        };
        
        struct is_equal_visitor
        {
            typedef bool result_type;
            
            template <typename T>
            bool operator()(T const& rhv, variant const& lhv) const
            {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                constexpr int rhv_which = mpl::index_of<T, Types...>::value;
                if (rhv_which == lhv.which()) {
                    return lhv.__is_equal<T>(rhv);
                }
                else {
                    return false;
                }
            }
        };
        
        
        struct swap_visitor
        {
            typedef void result_type;
            
            template <typename T>
            void operator()(T const& vlh, variant& lhv, variant& rhv) const
            {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                assert( (lhv.which() == mpl::index_of<T, Types...>::value) );
                if (lhv.which() == rhv.which()) {
                    lhv.__swap<T>(rhv);
                }
                else {
                    swap_visitor swapper;
                    rhv.apply_visitor(swapper, rhv, vlh, lhv);
                }
            }
            
            template <typename T, typename U>
            void operator()(U const&, variant& rhv, T const&, variant& lhv) const
            {
                static_assert(mpl::any_of<std::is_same<Types, T>...>::value, "bogus type T: T shall be one of [Types...]");
                static_assert(mpl::any_of<std::is_same<Types, U>...>::value, "bogus type U: U shall be one of [Types...]");
                lhv.__swap<T,U>(rhv);
            }
            
        };
        
        
        struct ostream_visitor
        {
            typedef void result_type;
            
            template <typename T>
            void operator()(T const& v, std::ostream& os) const
            {
                os << v;
            }
        };
        
        
        
        
    private:
        which_type which_;
        storage_type storage_;
        
        
    private:
        
        
        // comparison operators
        
        friend inline
        bool operator== (const variant& lhv, const variant& rhv) noexcept {
            return lhv.is_equal(rhv);
        }
        
        friend inline
        bool operator!= (const variant& lhv, const variant& rhv) noexcept {
            return !lhv.is_equal(rhv);
        }
        
        
        template <typename T>
        friend inline
        typename std::enable_if<variant::IsImpType<T>::value, bool>::type
        operator== (const variant& lhv, const T& rhv) noexcept
        {
            if (lhv.which() == IndexOf<T>::value) {
                return lhv.as<T>() == rhv;
            }
            else {
                return false;
            }
        }
        
        template <typename T>
        friend inline
        typename std::enable_if<variant::IsImpType<T>::value, bool>::type
        operator!= (const variant& lhv, const T& rhv) noexcept
        {
            if (lhv.which() == IndexOf<T>::value) {
                return not (lhv.as<T>() == rhv);
            }
            else {
                return true;
            }
        }
        
        template <typename T>
        friend inline
        typename std::enable_if<variant::IsImpType<T>::value, bool>::type
        operator== (const T& lhv, const variant& rhv) noexcept
        {
            if (rhv.which() == IndexOf<T>::value) {
                return rhv.as<T>() == lhv;
            }
            else {
                return false;
            }
        }
        
        template <typename T>
        friend inline
        typename std::enable_if<variant::IsImpType<T>::value, bool>::type
        operator!= (const T& lhv, const variant& rhv) noexcept
        {
            if (rhv.which() == IndexOf<T>::value) {
                return not (rhv.as<T>() == lhv);
            }
            else {
                return true;
            }
        }
    };  // class variant
    
    
    
    template <typename... Types>
    inline
    void swap(variant<Types...>& x, variant<Types...>& y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }
    
    
    
    
    
}} // namespace json::utility



namespace std
{
#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
    
    // This specialization of std::uses_allocator informs other library components
    // that class template json::utility::variant supports uses-allocator construction,
    // even though it does not have a nested allocator_type.
    
    template<class... Types, class Alloc>
    struct uses_allocator<json::utility::variant<Types...>, Alloc> : std::true_type
    {};
    
#endif
}



#endif // JSON_UTILITY_VARIANT_HPP

//
//  value.hpp
//
//  Created by Andreas Grosam on 13.02.13.
//  Copyright (c) 2013 Andreas Grosam. All rights reserved.
//

#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP


#include "json/utility/mpl.hpp"
#define HAS_MPL_HEADER
#include "json/utility/variant.hpp"
#include "Null.hpp"
#include "Boolean.hpp"
#include "float_number.hpp"
#include "integral_number.hpp"
#include <scoped_allocator>
#include <string>
#include <vector>
#include <map>
#include <type_traits>
#include <iostream>
#include <memory>


namespace json { namespace detail {
    
    

    template <typename Value>
    struct type_name_getter
    {
        typedef std::string result_type;
        
        typedef typename Value::integral_number_type    IntNumber;
        typedef typename Value::float_number_type       FloatNumber;
        typedef typename Value::null_type               Null;
        typedef typename Value::boolean_type            Boolean;
        typedef typename Value::string_type             String;
        typedef typename Value::array_type              Array;
        typedef typename Value::object_type             Object;
        
        
        std::string operator()(const Null& v) const noexcept {
            return "Null";
        }
        
        std::string operator()(const Boolean& v) const noexcept {
            return "Boolean";
        }
        
        std::string operator()(const IntNumber& v) const noexcept {
            return "Number";
        }
        
        std::string operator()(const FloatNumber& v) const noexcept {
            return "Number";
        }
        
        std::string operator()(const String& str) const noexcept {
            return "String";
        }
        
        std::string operator()(const Array& array) const noexcept {
            return "Array";
        }
        
        std::string operator()(const Object& objs) const noexcept {
            return "Object";
        }
    };
    
    
}} // json::detail



// Value Policies

namespace json  {
    
    
    struct array_tag {};
    struct object_tag {};
    struct string_tag {};
    struct float_number_tag {};
    struct integral_number_tag {};
    
    
    template <typename Allocator>
    using GetScopedAllocator = typename std::conditional<
        std::is_empty<Allocator>::value,
        Allocator,
        std::scoped_allocator_adaptor<Allocator>
    >::type;
            
        
    template <typename Value, typename Allocator>
    struct default_array_policy : array_tag
    {
    private:
        typedef Value value_type;
        typedef typename Allocator::template rebind<value_type>::other value_type_allocator;
        typedef GetScopedAllocator<value_type_allocator> allocator_type;
    public:
        typedef std::vector<value_type, allocator_type> type;
    };
    
    template <typename Value, typename Allocator>
    struct default_object_policy : object_tag
    {
    private:
        typedef typename Allocator::template rebind<char>::other key_allocator;
        typedef std::basic_string<char, std::char_traits<char>, key_allocator> key_type;
        typedef Value mapped_type;
        typedef std::pair<const key_type, Value> value_type;
        typedef typename Allocator::template rebind<value_type>::other value_type_allocator;
        typedef GetScopedAllocator<value_type_allocator> allocator_type;
    public:
        typedef std::map<key_type, Value, std::less<key_type>, allocator_type> type;
    };
    
    template <typename Value, typename Allocator>
    struct default_string_policy : string_tag
    {
        typedef char char_type;
        typedef typename Allocator::template rebind<char_type>::other allocator_type;            
        typedef std::basic_string<char_type, std::char_traits<char_type>, allocator_type> type;
    };
    
    template <typename Value, typename Allocator>
    struct default_float_number_policy : float_number_tag
    {
        typedef json::float_number<> type;
    };
    
    template <typename Value, typename Allocator>
    struct default_integral_number_policy : integral_number_tag
    {
        typedef json::integral_number<> type;
    };
    
    
    typedef std::allocator<void> default_allocator;
    
}



// class value
namespace json {
    
    namespace mpl = json::utility::mpl;    
    using json::utility::variant;
        

    template <
        typename A = std::allocator<void>,
        template <typename, typename> class... Policies
    >
    class value
    {
        template <typename T>
        using IsArrayPolicy = typename std::is_base_of<array_tag, T>::type;
        
        template <typename T>
        using IsObjectPolicy = typename std::is_base_of<object_tag, T>::type;
        
        template <typename T>
        using IsStringPolicy = typename std::is_base_of<string_tag, T>::type;
        
        template <typename T>
        using IsFloatNumberPolicy = typename std::is_base_of<float_number_tag, T>::type;

        template <typename T>
        using IsIntegralNumberPolicy = typename std::is_base_of<integral_number_tag, T>::type;
        
        
        typedef typename mpl::find_first<IsArrayPolicy, Policies<value,A>..., default_array_policy<value,A>>::type array_policy;
        typedef typename mpl::find_first<IsObjectPolicy, Policies<value,A>..., default_object_policy<value,A>>::type object_policy;
        typedef typename mpl::find_first<IsStringPolicy, Policies<value,A>..., default_string_policy<value,A>>::type string_policy;
        typedef typename mpl::find_first<IsFloatNumberPolicy, Policies<value,A>..., default_float_number_policy<value,A>>::type float_number_policy;
        typedef typename mpl::find_first<IsIntegralNumberPolicy, Policies<value,A>..., default_integral_number_policy<value,A>>::type integral_number_policy;
        
        
    public:
        typedef A propagated_allocator; // allocator, whose value_type us usually erased (rebound) while it is propagated to the containers and primitives.
        
        typedef json::Null                              null_type;
        typedef json::Boolean                           boolean_type;
        typedef typename float_number_policy::type      float_number_type;
        typedef typename integral_number_policy::type   integral_number_type;
        typedef typename string_policy::type            string_type;
        typedef typename object_policy::type            object_type;
        typedef typename array_policy::type             array_type;
        
        typedef typename object_type::key_type          key_type;        
        typedef GetScopedAllocator<A>                   scoped_allocator;
        
        
        
        typedef variant<
            null_type
          , boolean_type
          , float_number_type
          , integral_number_type
          , string_type
          , object_type
          , array_type
        > variant_type;
        
        
        struct
        emplace_tag {};
        
        constexpr static struct
        emplace_string_t : emplace_tag {
            typedef string_type type;
        } emplace_string{};
        
        constexpr static struct
        emplace_array_t : emplace_tag {
            typedef array_type type;
        } emplace_array{};
        
        constexpr static struct
        emplace_object_t : emplace_tag {
            typedef object_type type;
        } emplace_object{};
        
        constexpr static struct
        emplace_integral_number_t : emplace_tag {
            typedef integral_number_type type;
        } emplace_integral_number{};
        
        constexpr static struct
        emplace_float_number_t : emplace_tag {
            typedef float_number_type type;
        } emplace_float_number{};
        
        constexpr static struct
        emplace_boolean_t : emplace_tag {
            typedef boolean_type type;
        } emplace_boolean{};
        
        constexpr static struct
        emplace_null_t : emplace_tag {
            typedef null_type type;
        } emplace_null{};
        
        
        
        // type traits helper
        template <typename T>
        struct contains_type : variant_type::template contains_type<T> {
        };
        
        // handy template aliases:
        template <typename T>
        using IsImpType = typename variant_type::template contains_type<T>::type;
        
        
        
        
        
        // default ctor
        value() noexcept(std::is_nothrow_default_constructible<variant_type>::value)
        {
        };
        
        // copy ctor
        value(const value& other) noexcept(std::is_nothrow_copy_constructible<variant_type>::value)
        : value_(other.value_)
        {
        }
                
        // rvalue move ctor
        value(value&& other)  noexcept(std::is_nothrow_move_constructible<variant_type>::value)
        : value_(std::move(other.value_))
        {
        }
        
        
#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
        // default ctor - extended allocator
        template <typename Alloc>
        value(std::allocator_arg_t, const Alloc&) = delete; // n.a.
        
        // copy ctor - extended allocator
        template <typename Alloc>
        value(std::allocator_arg_t, const Alloc& a, const value& other) noexcept(std::is_nothrow_copy_constructible<variant_type>::value)
        : value_(std::allocator_arg, a, other.value_)
        {
        }
        
        // rvalue move ctor - extended allocator
        template <typename Alloc>
        value(std::allocator_arg_t, const Alloc& a, value&& other)  noexcept(std::is_nothrow_move_constructible<variant_type>::value)
        : value_(std::allocator_arg, a, std::move(other.value_))
        {
        }
#endif
        
        
        ~value() noexcept(std::is_nothrow_destructible<variant_type>::value)
        {
        }
        
        // forwarding ctor
#if 0
        template <typename T,
            typename = typename std::enable_if<
                !std::is_same<value, mpl::Unqualified<T>>::value
                and std::is_constructible<variant_type, T>::value
            >::type
        >
        value(T&& v) noexcept(std::is_nothrow_constructible<variant_type, T>::value)
        : value_(std::forward<T>(v))
        {
        }
#else
        template <typename... Args,
            typename _First = mpl::Unqualified<typename mpl::first<Args...>::type>,
            typename Enable = typename std::enable_if<
                !std::is_same<value, _First>::value
                and !std::is_same<std::allocator_arg_t, _First>::value
                and std::is_constructible<variant_type, Args...>::value
            >::type
        >
        value(Args&&... args) noexcept(std::is_nothrow_constructible<variant_type, Args...>::value)
        : value_(std::forward<Args>(args)...)
        {
        }
#endif
        
        
        // Explicit emplace
        template <typename EmplaceTag, typename... Args,
        typename IsViable = typename std::enable_if<
                std::is_base_of<emplace_tag, EmplaceTag>::value
                //and IsImpType<typename EmplaceTag::type>::value
                //and std::is_constructible<T, Args...>::value
            >::type
        >
        value(EmplaceTag, Args&&... args)
        : value_(json::utility::emplace_t<typename EmplaceTag::type>(), args...)
        {
        }
        
        
        // lvalue copy assignment
        value& operator=(const value& other) noexcept (std::is_nothrow_assignable<variant_type&, variant_type const>::value)
        {
            value_ = other.value_;
            return *this;
        }
        
        // rvalue move assignment
        value& operator=(value&& other) noexcept(std::is_nothrow_assignable<variant_type&, variant_type>::value)
        {
            value_ = std::move(other.value_);
            return *this;
        }
        
        // forwarding assignment
        template <typename T,
            typename = typename std::enable_if<
                std::is_assignable<variant_type&, T&>::value
            >::type
        >
        value& operator=(T&& v) noexcept(std::is_nothrow_assignable<variant_type&, T>::value)
        {
            value_ = std::forward<T>(v);
            return *this;
        }
        
        
//        // Comparison
//        bool operator==(value const& rhv) const {
//            return value_ == rhv.value_;
//        }
//        
//        bool operator!=(value const& rhv) const {
//            return value_ != rhv.value_;
//        }
        
        
        
        // Observer
        
        template <typename T>
        bool is_type() const noexcept {
            return value_.template is_type<T>();
        }
        
        std::string type_name() const noexcept {
            detail::type_name_getter<value> typeName;
            return value_.apply_visitor(typeName);
        }
        
        
        bool is_null() const noexcept  {
            return is_type<null_type>();
        }
        bool is_boolean() const noexcept {
            return is_type<boolean_type>();
        }
        bool is_string() const noexcept {
            return is_type<string_type>();
        }
        bool is_number() const noexcept {
            return is_type<integral_number_type>() or is_type<float_number_type>();
        }
        bool is_object() const noexcept {
            return is_type<object_type>();
        }
        bool is_array() const noexcept {
            return is_type<array_type>();
        }
        bool is_integral_number() const noexcept {
            return is_type<integral_number_type>();
        }
        bool is_float_number() const noexcept {
            return is_type<float_number_type>();
        }
        
        
        // Value access
        template <typename T
//          , typename Enable = typename std::enable_if<
//                IsImpType<mpl::Unqualified<RemovePointer<T>>>::value
//            >::type
        >
        auto as() const -> decltype(std::declval<const variant_type>().template as<T>())
        {
            return value_.template as<T>();
        }
        
        template <typename T
//          , typename Enable = typename std::enable_if<
//                IsImpType<mpl::Unqualified<RemovePointer<T>>>::value
//            >::type
        >
        auto as() -> decltype(std::declval<variant_type>().template as<T>())
        {
            return value_.template as<T>();
        }
        
        template <typename T>
        T as(typename std::enable_if<
              !IsImpType<mpl::Unqualified<T>>::value
              and json::is_numeric<mpl::Unqualified<T>>::value and std::is_integral<mpl::Unqualified<T>>::value,
              integral_number_tag
              >::type* = 0) const
        {
            // Intentionally implicit conversion may lose precision. Watch out for warnings!
            return value_.template as<integral_number_type>();
        }
        
        template <typename T>
        T as(typename std::enable_if<
             !IsImpType<mpl::Unqualified<T>>::value
             and std::is_floating_point<T>::value,
             float_number_tag
              >::type* = 0) const
        {
            // Intentionally implicit conversion may lose precision. Watch out for warnings!
            return value_.template as<float_number_type>();
        }
        
        
        template <typename T,
            typename Enable = typename std::enable_if<
                IsImpType<mpl::Unqualified<T>>::value
            >::type
        >
        T& interpret_as() noexcept {
            return value_.template interpret_as<T>();
        }
        
        template <typename T,
            typename Enable = typename std::enable_if<
                IsImpType<mpl::Unqualified<T>>::value
            >::type
        >
        T const& interpret_as() const noexcept {
            return value_.template interpret_as<T>();
        }
        
        
        //
        // Subscript Operator
        //
        value& operator[](std::size_t n) {
            return this->interpret_as<array_type>()[n];
        }
        
        value const& operator[](std::size_t n) const {
            return this->interpret_as<array_type>()[n];
        }
        
        value& operator[](key_type key) {
            return this->interpret_as<object_type>()[key];
        }
        
        value const& operator[](key_type key) const {
            return this->interpret_as<object_type>()[key];
        }
        
        
        
        // swap
        void swap(value& rhv) noexcept (noexcept(std::declval<variant_type>().swap(std::declval<variant_type&>())))
        {
            value_.swap(rhv.value_);
        }
        
        
        //
        //  Visitor support
        //
        
        template <typename Visitor, typename... Args>
        typename Visitor::result_type
        apply_visitor(Visitor& visitor, Args&&... args) {
            return value_.apply_visitor(visitor, std::forward<Args>(args)...);
        }
        
        template <typename Visitor, typename... Args>
        typename Visitor::result_type
        apply_visitor(Visitor& visitor, Args&&... args) const {
            return value_.apply_visitor(visitor, std::forward<Args>(args)...);
        }
        
        
//        //
//        // Print support
//        //
//        template <typename... Args>
//        void print(std::ostream& os, Args&&... args) const
//        {
//            detail::printer<value> printer;
//            value_.apply_visitor(printer, os, std::forward<Args>(args)...);
//            os << std::endl;
//        }
//        
//        template <typename Printer, typename... Args>
//        void print(std::ostream& os, Printer& printer, Args&&... args) const
//        {
//            value_.apply_visitor(printer, os, std::forward<Args>(args)...);
//        }
        

    private:
        
        
        // Comparision  (friends)
        
        friend inline
        bool operator==(value const& lhv, value const& rhv) noexcept {
            return lhv.value_.is_equal(rhv.value_);
        }
        
        friend inline
        bool operator!=(value const& lhv, value const& rhv) noexcept {
            return !lhv.value_.is_equal(rhv.value_);
        }
        
        
        
        template <typename U>
        friend inline
        typename std::enable_if<value::IsImpType<U>::value, bool>::type
        operator==(value const& lhv, U const& rhv)
        noexcept
        {
            return lhv.value_ == rhv;
        }
        
        template <typename U>
        friend inline
        typename std::enable_if<value::IsImpType<U>::value, bool>::type
        operator==(U const& lhv, value const& rhv) noexcept {
            return rhv.value_ == lhv;
        }
        
        
        template <typename U>
        friend inline
        typename std::enable_if<value::IsImpType<U>::value, bool>::type
        operator!=(value const& lhv, U const& rhv) noexcept {
            return lhv.value_ != rhv;
        }
        
        template <typename U>
        friend inline
        typename std::enable_if<value::IsImpType<U>::value, bool>::type
        operator!=(U const& lhv, value const& rhv) noexcept {
            return rhv.value_ != lhv;
        }

        
        // TODO: specialize operator== and operator != for certain types 
        
        
        
    private:
        variant_type value_;
    };  // class value
    
    
    template <typename A, template <typename, typename> class... P>
    typename value<A, P...>::emplace_string_t constexpr value<A, P...>::emplace_string;
    
    template <typename A, template <typename, typename> class... P>
    typename value<A, P...>::emplace_array_t constexpr value<A, P...>::emplace_array;
    
    template <typename A, template <typename, typename> class... P>
    typename value<A, P...>::emplace_object_t constexpr value<A, P...>::emplace_object;
    
    template <typename A, template <typename, typename> class... P>
    typename value<A, P...>::emplace_integral_number_t constexpr value<A, P...>::emplace_integral_number;
    
    template <typename A, template <typename, typename> class... P>
    typename value<A, P...>::emplace_float_number_t constexpr value<A, P...>::emplace_float_number;
    
    template <typename A, template <typename, typename> class... P>
    typename value<A, P...>::emplace_boolean_t constexpr value<A, P...>::emplace_boolean;
    
    template <typename A, template <typename, typename> class... P>
    typename value<A, P...>::emplace_null_t constexpr value<A, P...>::emplace_null;
    
    
    
    
} // namespace json


namespace std
{
#if !defined (VARIANT_NO_USES_ALLOCATOR_CONSTRUCTION)
    
    // This specialization of std::uses_allocator informs other library components
    // that class template json::value supports uses-allocator construction,
    // even though it does not have a nested allocator_type.
    
    template<
        typename A,
        template <typename, typename> class... P,
        class Alloc
    >
    struct uses_allocator<json::value<A, P...>, Alloc> : std::true_type
    {};
    
#endif
}



namespace json { namespace detail { namespace test {
    
    typedef json::value<> Value;
    typedef typename Value::propagated_allocator Allocator;
    typedef typename Value::object_type JsonObject;
    typedef typename Value::array_type JsonArray;
    typedef typename Value::integral_number_type JsonIntNumber;
    typedef typename Value::float_number_type JsonFloatNumber;
    typedef typename Value::string_type JsonString;
    typedef typename Value::null_type JsonNull;
    typedef typename Value::boolean_type JsonBoolean;
    
    static_assert( std::is_same<
                  Value,
                  typename JsonObject::mapped_type
                  >::value, "");
    
    static_assert( std::is_same<
                  Value,
                  typename JsonArray::value_type
                  >::value, "");

    static_assert(std::is_same<std::allocator<void>, Allocator>::value, "");
    
    
    // Object and Array use a scoped_allocator_adapter unless the allocator is stateless.
    typedef json::GetScopedAllocator<Allocator> possibly_scoped_allocator_t;
    
    static_assert( std::is_same<
                  typename possibly_scoped_allocator_t::template rebind<Value>::other,
                  typename JsonArray::allocator_type
                  >::value, "");
    
    static_assert( std::is_same<
                  typename possibly_scoped_allocator_t::template rebind<typename JsonObject::value_type>::other,
                  typename JsonObject::allocator_type
                  >::value, "");
    
    static_assert( std::is_same<
                  typename Allocator::template rebind<typename JsonString::value_type>::other,
                  typename JsonString::allocator_type
                  >::value, "");
    

    
    
    struct A {};
    struct B {};
    typedef typename Value::variant_type variant_type;
    
//    static_assert( (std::is_same<std::vector<Value>, Value::array_type>::value), "");
//    static_assert( (std::is_same<std::map<std::string, Value>, Value::object_type>::value), "");
    
    
    
    // variant _v; Value v;
    static_assert (std::is_default_constructible<variant_type>::value, "");
    static_assert (std::is_default_constructible<Value>::value, "");
    
    // variant _v = "abc"
    static_assert (std::is_constructible<variant_type, decltype("ab")>::value, "");
    static_assert (std::is_constructible<Value, std::string>::value, "");
    
    // variant _v = 1.0
    static_assert (std::is_constructible<variant_type, decltype(1.0)>::value, "");
    
    // variant _v = A(); ERROR
    // TODO: static_assert (std::is_constructible<variant_type, A>::value == false, "");
    
    // _v = "abc"
    //static_assert (std::is_assignable<variant_type&, decltype("ab")>::value, "");
    
    // variant _v = v;  ERROR
    // TODO: static_assert (std::is_constructible<variant_type, Value>::value == false, "");
    
    // _v = v;  ERROR
    static_assert (std::is_assignable<variant_type&, A>::value == false, "");
    // TODO: static_assert (std::is_assignable<Value&, A>::value == false, "");
    
    
    
}}}




#endif // JSON_VALUE_HPP
#ifndef JSON_UTILITY_ATOMIC_HPP
#define JSON_UTILITY_ATOMIC_HPP
//
//  atomic.hpp
//
//  Created by Andreas Grosam on 9/13/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//



namespace json { namespace utility { 
    
    namespace atomic_detail {
        
        // platform specific
        
        template <typename ArithmeticT>
        inline ArithmeticT atomic_fetch(ArithmeticT const& lhv) {
            return __sync_fetch_and_add(&(const_cast<ArithmeticT&>(lhv)), 0);
        }
        
        template <typename ArithmeticT>
        inline void atomic_set(ArithmeticT& lhv, ArithmeticT v) {
            __sync_lock_test_and_set(&lhv, v);
        }
        
        
        
        template <typename ArithmeticT>
        inline ArithmeticT atomic_post_increment(ArithmeticT& lhv) {
            return __sync_fetch_and_add(&lhv, 1);
        }
        
        template <typename ArithmeticT>
        inline ArithmeticT atomic_pre_increment(ArithmeticT& lhv) {
            return __sync_add_and_fetch(&lhv, 1);
        }
        
        template <typename ArithmeticT>
        inline ArithmeticT atomic_post_decrement(ArithmeticT& lhv) {
            return __sync_fetch_and_add(&lhv, -1);
        }
        
        template <typename ArithmeticT>
        inline ArithmeticT atomic_pre_decrement(ArithmeticT& lhv) {
            return __sync_add_and_fetch(&lhv, -1);
        }
    
    }  // atomic_detail
    
    
    // T shall be arithmentic type
    template <typename T>
    class atomic {
    public:
        
        // copy d-tor
        atomic(const atomic& other) : v_(other) {}
        
        // conversion d-tor
        atomic(const T& v = 0) : v_(v)  {}

        // onversion operator
        operator T () const {
            return atomic_detail::atomic_fetch(v_);
        }
        
        // assignment
        atomic& operator=(const atomic& other) {
            atomic_detail::atomic_set(v_, other);
            return *this;
        }
        
        atomic& operator=(const T& v) {
            atomic_detail::atomic_set(v_, v);
            return *this;
        }
        
        T operator++() {  // prefix  ++x
            return atomic_detail::atomic_pre_increment(v_);
        }
        
        T operator++(int) {  // postfix  x++
            return atomic_detail::atomic_post_increment(v_);
        }
        
        T operator--() {  // prefix  --x
            return atomic_detail::atomic_pre_decrement(v_);
        }
        
        T operator--(int) {  // postfix  x--
            return atomic_detail::atomic_post_decrement(v_);
        }
                
    private:
        T v_;
    };
    
}}



#endif

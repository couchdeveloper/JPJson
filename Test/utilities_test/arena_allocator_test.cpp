//
//  arena_allocator_test.cpp
//  Test
//
//  Created by Andreas Grosam on 02.03.13.
//
//

#include "json/utility/arena_allocator.hpp"
#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <cstdlib>

#include <memory>
#include <scoped_allocator>

#include <chrono>

#include <cstddef>
#include <cassert>

namespace nm {
    
    
//    template <class T>
//    struct custom_allocator {
//        typedef T value_type;
//        custom_allocator() noexcept {}
//        template <class U> custom_allocator (const custom_allocator<U>&) noexcept {}
//        T* allocate (std::size_t n) {
//            return static_cast<T*>(::new(n*sizeof(T)));
//        }
//        void deallocate (T* p, std::size_t n) { ::delete(p); }
//    };
//    
//    template <class T, class U>
//    constexpr bool operator== (const custom_allocator<T>&, const custom_allocator<U>&) noexcept
//    {return true;}
//    
//    template <class T, class U>
//    constexpr bool operator!= (const custom_allocator<T>&, const custom_allocator<U>&) noexcept
//    {return false;}
    
    
    template <std::size_t N>
    class arena
    {
        static const std::size_t alignment = 16;
        alignas(alignment) char buf_[N];
        char* ptr_;

        std::size_t
        align_up(std::size_t n) noexcept {
            return n + (alignment-1) & ~(alignment-1);
        }

        bool pointer_in_buffer(char* p) noexcept {
            return (buf_<=p) && (p <= buf_+N);
        }

    public:
        arena() noexcept : ptr_(buf_)
        {
        }
        ~arena() {
            ptr_ = nullptr;
        }
        arena(const arena&) = delete;
        arena& operator=(const arena&) = delete;

        char* allocate(std::size_t n);
        void deallocate(char* p, std::size_t n) noexcept;

        static constexpr std::size_t size()
        {
            return N;
        }
        std::size_t used() const
        {
            return static_cast<std::size_t>(ptr_ - buf_);
        }
        void reset() {
            ptr_ = buf_;
        }
    };
    
    
    template <std::size_t N>
    char*
    arena<N>::allocate(std::size_t n)
    {
        assert(pointer_in_buffer(ptr_) && "short_alloc has outlived arena");
        n = align_up(n);
        if (buf_ + N - ptr_ >= n)
        {
            char* r = ptr_;
            ptr_ += n;
            return r;
        }
        return static_cast<char*>(::operator new(n));
    }
    
    template <std::size_t N>
    void
    arena<N>::deallocate(char* p, std::size_t n) noexcept
    {
        assert(pointer_in_buffer(ptr_) && "short_alloc has outlived arena");
        if (pointer_in_buffer(p))
        {
            n = align_up(n);
            if (p + n == ptr_)
                ptr_ = p;
                }
        else
            ::operator delete(p);
            }
    
    
    
    template <class T, std::size_t N>
    class short_alloc
    {
        arena<N>& a_;
    public:
        typedef T value_type;
        
    public:
        template <class _Up> struct rebind {typedef short_alloc<_Up, N> other;};
        
        short_alloc(arena<N>& a) noexcept : a_(a) {}

        template <class U>
        short_alloc(const short_alloc<U, N>& a) noexcept
        : a_(a.a_)
        {
        }
        
        short_alloc(const short_alloc&) = default;
        
        short_alloc& operator=(const short_alloc&) = delete;
        
        T* allocate(std::size_t n)
        {
            return reinterpret_cast<T*>(a_.allocate(n*sizeof(T)));
        }

        void deallocate(T* p, std::size_t n) noexcept
        {
            a_.deallocate(reinterpret_cast<char*>(p), n*sizeof(T));
        }
        
        template <class T1, std::size_t N1, class U, std::size_t M>
        friend
        bool
        operator==(const short_alloc<T1, N1>& x, const short_alloc<U, M>& y) noexcept;
        
        template <class U, std::size_t M> friend class short_alloc;
    };
    
    template <class T, std::size_t N, class U, std::size_t M>
    inline
    bool
    operator==(const short_alloc<T, N>& x, const short_alloc<U, M>& y) noexcept
    {
        return N == M && &x.a_ == &y.a_;
    }
    
    template <class T, std::size_t N, class U, std::size_t M>
    inline
    bool
    operator!=(const short_alloc<T, N>& x, const short_alloc<U, M>& y) noexcept
    {
        return !(x == y);
    }
    
    
}



namespace test {

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
        
        arena_allocator(const arena_allocator&) = default;
        
        template <typename U>
        arena_allocator(const arena_allocator<U>& other) noexcept
        : base(other.arena)
        {}
        
        arena_allocator(arena_type& arena) noexcept
        : base(arena)
        {}
                
        
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
    };

}


namespace {
    
    
    typedef std::chrono::high_resolution_clock timer_t;
    typedef std::chrono::time_point<timer_t> time_t;
    
    using std::chrono::duration_cast;
    using std::chrono::microseconds;
    
    using json::utility::SysArena;
    using json::utility::arena_allocator;
    
    template <typename T>
    using ArenaAllocator = typename json::utility::arena_allocator<T, json::utility::SysArena>;
    
    
    class ArenaAllocatorTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        ArenaAllocatorTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~ArenaAllocatorTest() {
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
#pragma mark Unicode Code Point
    
    
    TEST_F(ArenaAllocatorTest, Ctors)
    {
        
        typedef ArenaAllocator<int> Alloc;

        SysArena arena1;
        SysArena arena2;
        
        Alloc a1_a1(arena1);
        Alloc a2_a1 = a1_a1;
        EXPECT_TRUE( a1_a1 == a2_a1);
        
        Alloc a3_a2(arena2);
        EXPECT_FALSE( a3_a2 == a1_a1);
        EXPECT_FALSE( a3_a2 == a2_a1);
        
        
        typedef test::arena_allocator<int> alloc_t;
        alloc_t alloc1(arena1);
        alloc_t alloc2 = alloc1;
        EXPECT_TRUE(alloc1 == alloc2);
        
        alloc_t alloc3(arena2);
        EXPECT_FALSE( alloc3 == alloc1);
        EXPECT_FALSE( alloc3 == alloc2);
        
        alloc3 = alloc1;
        EXPECT_TRUE( alloc3 == alloc1);
    }

    
    TEST_F(ArenaAllocatorTest, Equality)
    {
        typedef ArenaAllocator<int> alloc_int_t;
        typedef ArenaAllocator<char> alloc_char_t;
        typedef ArenaAllocator<std::pair<int, char>> alloc_pair_t;
        
        SysArena arena1;
        
        alloc_int_t a1(arena1);
        alloc_char_t a2(arena1);
        alloc_pair_t a3(arena1);
        EXPECT_TRUE( a1 == a2);
        EXPECT_FALSE( a1 != a1);
        EXPECT_TRUE( a1 == a3);
        EXPECT_FALSE( a1 != a3);
        
    }
    
    
    
    TEST_F(ArenaAllocatorTest, BasicAllocatorTest)
    {
        typedef ArenaAllocator<char> allocator;
        typedef std::basic_string<char, std::char_traits<char>, allocator> string;
        
        SysArena arena;
        allocator a(arena);
        
        std::string str(22, 'a');

        string s = {22, 'a', a};        
    }

    
    
    
    TEST_F(ArenaAllocatorTest, Test1)
    {
        timer_t timer;
        
        char data[640]= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#";
        
        int length[16] = {24, 28, 33, 44, 55, 66, 77, 88, 99, 111, 133, 233, 333, 444, 555, 640 };
        
        int count = 10000;
        
        std::vector<std::string> v;
        v.reserve(count);
        

        auto t0 = timer.now();
        
        for (int i = 0; i < count; ++i)
        {
            int len = length[i&0x0F];
            v.emplace_back(data, len);            
        }
        
        auto t1 = timer.now();
        std::cout << "elapsed time: " << duration_cast<microseconds>(t1 - t0).count() << " µs.\n";
    }

    
//    TEST_F(ArenaAllocatorTest, Test2)
//    {
//        timer_t timer;
//        
//        char data[640]= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#"
//        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#";
//        
//        int length[16] = {24, 28, 33, 44, 55, 66, 77, 88, 99, 111, 133, 233, 333, 444, 555, 640 };
//        
//        int count = 10000;
//        
//        
//        typedef std::basic_stringbuf<char, std::char_traits<char>, ArenaAllocator<char>> string_t;
//        typedef ArenaAllocator<string_t> Alloc;
//        typedef std::vector<string_t, Alloc> vector_t;
//
//        SysArena arena(1024*8);
//        vector_t v{Alloc(arena)};
//        
//        v.reserve(count);
//        
//        auto t0 = timer.now();
//        
//        for (int i = 0; i < count; ++i)
//        {
//            int len = length[i&0x0F];
//            v.emplace_back(data, len);
//        }
//        
//        auto t1 = timer.now();
//        std::cout << "elapsed time: " << duration_cast<microseconds>(t1 - t0).count() << " µs.\n";
//    }
    
}




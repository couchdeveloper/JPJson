//
//  arena_allocator.hpp
//  
/*
 * Copyright 2012 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JSON_UTILITY_ARENA_ALLOCATOR_HPP
#define JSON_UTILITY_ARENA_ALLOCATOR_HPP

#include "mpl.hpp"
#include <type_traits>
#include <boost/intrusive/slist.hpp>
#include <cassert>
#include <utility>
#include <limits>
#include <new>
#include <stdlib.h>



#define LIKELY(x)   (__builtin_expect((x), 1))


namespace json { namespace utility {
    
    
    template <typename T, typename Arena>
    class arena_allocator
    {
        Arena* arena_;
        
    public:
        typedef T value_type;
        
    public:
        template <typename _Up> struct rebind
        {
            typedef arena_allocator<_Up, Arena> other;
        };
        
        
        arena_allocator() = delete;
        
        arena_allocator(Arena& a) noexcept : arena_(&a) {}
                
        template <typename U>
        arena_allocator(const arena_allocator<U, Arena>& a) noexcept
        : arena_(a.arena_)
        {
        }
        
        arena_allocator(const arena_allocator&) = default;
        
        
        T* allocate(std::size_t n) {
            assert(arena_ != nullptr);
            return reinterpret_cast<T*>(arena_->allocate(n*sizeof(T)));
        }
        
        void deallocate(T* p, std::size_t n) noexcept
        {
            // TODO: arena_->deallocate(reinterpret_cast<char*>(p), n*sizeof(T));
            assert(arena_ != nullptr);
            arena_->deallocate(reinterpret_cast<char*>(p));
        }
        
        template <typename T1, typename Arena1, typename T2, typename Arena2>
        friend
        bool
        operator==(const arena_allocator<T1, Arena1>& a1, const arena_allocator<T2, Arena2>& a2) noexcept;

        template <typename T1, typename Arena1, typename T2, typename Arena2>
        friend
        bool
        operator!=(const arena_allocator<T1, Arena1>& a1, const arena_allocator<T2, Arena2>& a2) noexcept;

        
        template <typename U, typename A> friend class arena_allocator;
    };

    
    template <typename T1, typename Arena1, typename T2, typename Arena2>
    bool
    operator==(const arena_allocator<T1, Arena1>& a1, const arena_allocator<T2, Arena2>& a2) noexcept
    {
        return a1.arena_ == a2.arena_;  // pointer comparison
    }
    
    template <typename T1, typename Arena1, typename T2, typename Arena2>
    bool
    operator!=(const arena_allocator<T1, Arena1>& a1, const arena_allocator<T2, Arena2>& a2) noexcept
    {
        return !(a1 == a2);
    }
    

    
}}



namespace json { namespace utility {
    
    
    namespace detail {

        inline std::size_t goodMallocSize(std::size_t minSize)
        {
            if (minSize <= 64) {
                // Choose smallest allocation to be 64 bytes - no tripping over
                // cache line boundaries, and small string optimization takes care
                // of short strings anyway.
                return 64;
            }
            if (minSize <= 512) {
                // Round up to the next multiple of 64; we don't want to trip over
                // cache line boundaries.
                return (minSize + 63) & ~size_t(63);
            }
            if (minSize <= 3840) {
                // Round up to the next multiple of 256
                return (minSize + 255) & ~size_t(255);
            }
            if (minSize <= 4072 * 1024) {
                // Round up to the next multiple of 4KB
                return (minSize + 4095) & ~size_t(4095);
            }
            // Holy Moly
            // Round up to the next multiple of 4MB
            return (minSize + 4194303) & ~size_t(4194303);
        }
        
        /**
         * Trivial wrappers around malloc, calloc, realloc that check for allocation
         * failure and throw std::bad_alloc in that case.
         */
        inline void* checkedMalloc(size_t size) {
            void* p = malloc(size);
            if (!p) throw std::bad_alloc();
            return p;
        }
        
        inline void* checkedCalloc(size_t n, size_t size) {
            void* p = calloc(n, size);
            if (!p) std::bad_alloc();
            return p;
        }
        
        inline void* checkedRealloc(void* ptr, size_t size) {
            void* p = realloc(ptr, size);
            if (!p) std::bad_alloc();
            return p;
        }
        
    }
    
    
    /**
     * Simple arena: allocate memory which gets freed when the arena gets
     * destroyed.
     *
     * The arena itself allocates memory using a custom allocator which provides
     * the following interface (same as required by StlAllocator in StlAllocator.h)
     *
     *   void* allocate(size_t size);
     *      Allocate a block of size bytes, properly aligned to the maximum
     *      alignment required on your system; throw std::bad_alloc if the
     *      allocation can't be satisfied.
     *
     *   void deallocate(void* ptr);
     *      Deallocate a previously allocated block.
     *
     * You may also specialize ArenaAllocatorTraits for your allocator type to
     * provide:
     *
     *   size_t goodSize(const Allocator& alloc, size_t size) const;
     *      Return a size (>= the provided size) that is considered "good" for your
     *      allocator (for example, if your allocator allocates memory in 4MB
     *      chunks, size should be rounded up to 4MB).  The provided value is
     *      guaranteed to be rounded up to a multiple of the maximum alignment
     *      required on your system; the returned value must be also.
     *
     * An implementation that uses malloc() / free() is defined below, see
     * SysAlloc / SysArena.
     */
    template <class Alloc> struct ArenaAllocatorTraits;
    
    template <class Alloc>
    class arena {
    public:
        explicit arena(const Alloc& alloc,
                       size_t minBlockSize = kDefaultMinBlockSize)
            : allocAndSize_(alloc, minBlockSize)
            , ptr_(nullptr)
            , end_(nullptr)
            , totalAllocatedSize_(0)
            , bytesUsed_(0)
        {
        }
        
        ~arena();
        
        void* allocate(size_t size)
        {
            size = roundUp(size);
            bytesUsed_ += size;
            
            if (LIKELY(end_ - ptr_ >= size)) {
                // Fast path: there's enough room in the current block
                char* r = ptr_;
                ptr_ += size;
                assert(isAligned(r));
                return r;
            }
            
            // Not enough room in the current block
            void* r = allocateSlow(size);
            assert(isAligned(r));
            return r;
        }
        
        void deallocate(void* p) noexcept {
            // Deallocate? Never!
        }
        
        // Transfer ownership of all memory allocated from "other" to "this".
        void merge(arena&& other);
        
        // Gets the total memory used by the arena
        size_t totalSize() const {
            return totalAllocatedSize_ + sizeof(arena);
        }
        
        size_t numberAllocatedBlocks() const {
            typename BlockList::size_type sz = blocks_.size();
            return sz;
        }
        
        // Gets the total number of "used" bytes, i.e. bytes that the arena users
        // allocated via the calls to `allocate`. Doesn't include fragmentation, e.g.
        // if block size is 4KB and you allocate 2 objects of 3KB in size,
        // `bytesUsed()` will be 6KB, while `totalSize()` will be 8KB+.
        size_t bytesUsed() const {
            return bytesUsed_;
        }
        
        void clear() {
            auto disposer = [this] (Block* b) { b->deallocate(this->alloc()); };
            while (!blocks_.empty()) {
                blocks_.pop_front_and_dispose(disposer);
            }
            blocks_.clear();
            ptr_ = nullptr;
            end_ = nullptr;
            totalAllocatedSize_ = 0;
            bytesUsed_ = 0;
            
        }
        
    private:
        // not copyable
        arena(const arena&) = delete;
        arena& operator=(const arena&) = delete;
        
        // movable
        arena(arena&&) = default;
        arena& operator=(arena&&) = default;
        
        struct Block;
        typedef boost::intrusive::slist_member_hook<
        boost::intrusive::tag<arena>> BlockLink;
        
        struct Block {
            BlockLink link;
            
            // Allocate a block with at least size bytes of storage.
            // If allowSlack is true, allocate more than size bytes if convenient
            // (via ArenaAllocatorTraits::goodSize()) as we'll try to pack small
            // allocations in this block.
            static std::pair<Block*, size_t> allocate(
                                                      Alloc& alloc, size_t size, bool allowSlack);
            void deallocate(Alloc& alloc);
            
            char* start() {
                return reinterpret_cast<char*>(this + 1);
            }
            
        private:
            Block() { }
            ~Block() { }
        } __attribute__((aligned));
        // This should be alignas(std::max_align_t) but neither alignas nor
        // max_align_t are supported by gcc 4.6.2.
        
    public:
        static constexpr size_t kDefaultMinBlockSize = 4096 - sizeof(Block);
        
    private:
        static constexpr size_t maxAlign = alignof(Block);
        static constexpr bool isAligned(uintptr_t address) {
            return (address & (maxAlign - 1)) == 0;
        }
        static bool isAligned(void* p) {
            return isAligned(reinterpret_cast<uintptr_t>(p));
        }
        
        // Round up size so it's properly aligned
        static constexpr size_t roundUp(size_t size) {
            return (size + maxAlign - 1) & ~(maxAlign - 1);
        }
        
        // cache_last<true> makes the list keep a pointer to the last element, so we
        // have push_back() and constant time splice_after()
        typedef boost::intrusive::slist<
            Block,
            boost::intrusive::member_hook<Block, BlockLink, &Block::link>,
            boost::intrusive::constant_time_size<false>,
            boost::intrusive::cache_last<true>
        > BlockList;
        
        void* allocateSlow(size_t size);
        
        // Empty member optimization: package Alloc with a non-empty member
        // in case Alloc is empty (as it is in the case of SysAlloc).
        struct AllocAndSize : public Alloc {
            explicit AllocAndSize(const Alloc& a, size_t s)
            : Alloc(a), minBlockSize(s) {
            }
            
            size_t minBlockSize;
        };
        
        size_t minBlockSize() const {
            return allocAndSize_.minBlockSize;
        }
        Alloc& alloc() { return allocAndSize_; }
        const Alloc& alloc() const { return allocAndSize_; }
        
        AllocAndSize allocAndSize_;
        BlockList blocks_;
        char* ptr_;
        char* end_;
        size_t totalAllocatedSize_;
        size_t bytesUsed_;
    };
    
    /**
     * By default, don't pad the given size.
     */
    template <class Alloc>
    struct ArenaAllocatorTraits {
        static size_t goodSize(const Alloc& alloc, size_t size) {
            return size;
        }
    };
    
    /**
     * arena-compatible allocator that calls malloc() and free(); see
     * goodMallocSize() in Malloc.h for goodSize().
     */
    class SysAlloc {
    public:
        void* allocate(size_t size) {
            return detail::checkedMalloc(size);
        }
        
        void deallocate(void* p) {
            free(p);
        }
    };
    
    template <>
    struct ArenaAllocatorTraits<SysAlloc> {
        static size_t goodSize(const SysAlloc& alloc, size_t size) {
            return detail::goodMallocSize(size);
        }
    };
    
    /**
     * arena that uses the system allocator (malloc / free)
     */
    class SysArena : public arena<SysAlloc> {
    public:
        explicit SysArena(size_t minBlockSize = kDefaultMinBlockSize)
        : arena<SysAlloc>(SysAlloc(), minBlockSize) {
        }
    };
    
}}  // namespace json::utiltiy




namespace json { namespace utility {
    
    template <class Alloc>
    std::pair<typename arena<Alloc>::Block*, size_t>
    arena<Alloc>::Block::allocate(Alloc& alloc, size_t size, bool allowSlack) {
        size_t allocSize = sizeof(Block) + size;
        if (allowSlack) {
            allocSize = ArenaAllocatorTraits<Alloc>::goodSize(alloc, allocSize);
        }
        
        void* mem = alloc.allocate(allocSize);
        assert(isAligned(mem));
        return std::make_pair(new (mem) Block(), allocSize - sizeof(Block));
    }
    
    template <class Alloc>
    void arena<Alloc>::Block::deallocate(Alloc& alloc) {
        this->~Block();
        alloc.deallocate(this);
    }
    
    template <class Alloc>
    void* arena<Alloc>::allocateSlow(size_t size) {
        std::pair<Block*, size_t> p;
        char* start;
        if (size > minBlockSize()) {
            // Allocate a large block for this chunk only, put it at the back of the
            // list so it doesn't get used for small allocations; don't change ptr_
            // and end_, let them point into a normal block (or none, if they're
            // null)
            p = Block::allocate(alloc(), size, false);
            start = p.first->start();
            blocks_.push_back(*p.first);
        } else {
            // Allocate a normal sized block and carve out size bytes from it
            p = Block::allocate(alloc(), minBlockSize(), true);
            start = p.first->start();
            blocks_.push_front(*p.first);
            ptr_ = start + size;
            end_ = start + p.second;
        }
        
        assert(p.second >= size);
        totalAllocatedSize_ += p.second + sizeof(Block);
        return start;
    }
    
    template <class Alloc>
    void arena<Alloc>::merge(arena<Alloc>&& other) {
        blocks_.splice_after(blocks_.before_begin(), other.blocks_);
        other.blocks_.clear();
        other.ptr_ = other.end_ = nullptr;
        totalAllocatedSize_ += other.totalAllocatedSize_;
        other.totalAllocatedSize_ = 0;
    }
    
    template <class Alloc>
    arena<Alloc>::~arena() {
        auto disposer = [this] (Block* b) { b->deallocate(this->alloc()); };
        while (!blocks_.empty()) {
            blocks_.pop_front_and_dispose(disposer);
        }
    }
    
}}  // namespace json::utility





#undef LIKELY

#endif /* JSON_UTILITY_ARENA_ALLOCATOR_HPP */
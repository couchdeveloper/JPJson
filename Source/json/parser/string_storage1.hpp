//
//  string_storage1.hpp
//  
//
//  Created by Andreas Grosam on 4/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_INTERNAL_STRING_STORAGE1_HPP
#define JSON_PARSER_INTERNAL_STRING_STORAGE1_HPP


#include "json/unicode/unicode_traits.hpp"
#include "json/unicode/unicode_converter.hpp"
#include "json/endian/byte_swap.hpp"
#include "json/endian/endian.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <limits>


#include <boost/static_assert.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>



namespace json { namespace parser_internal {
    
    // The current SAX style API and the recursive decent JSON parser implemen-
    // tation requires that keys from JSON objects need to be temporarily safed 
    // on a stack storage. The stack's size will be equal the "level" of the
    // JSON objects within the JSON document.
    //
    // Key strings are assumed to be "small" strings. The SAX style API will
    // therefore pass key strings as a whole to the underlaying Semantic Actions
    // object.
    // 
    // Data strings, on the other hand, may be possibly "large strings". Large
    // strings will be passed in "chunks" to the underlaying Semantic Actions
    // object.
    //
    // A "stack storage" can be accomplished by using 
    // a)  a fixed size buffer with sufficient size allocated on the stack
    // b)  a dynamically allocated buffer per occurance
    // c)  a dynamically allocated buffer shared for all keys.
    //
    // There is a trade of between space and time complexity - where "space"
    // also means bytes allocated on the stack.
    
    
    // The subsequent class implements a stack storage for JSON keys, whose 
    // storage resides on the heap. That is, for a recursive decent parser, the
    // amount of stack space will be reduced since no buffer for key strings
    // must be allocated on the stack. 
    //
    // The same raw storage will be used to store "large data strings", whose 
    // actual buffer size per string is bounded, and as a result the large string 
    // will be passed in chunks to the underlaying Semantic Actions object.
    // 
    // A "chunk" is a partial string, whose encding is possibly a multi byte
    // encoding. The partial string always starts at the beginning of a possible
    // multibyte sequence and ends with a complete character. 
    //
    // A "string buffer" may contain a "partial string".
    //
    // The string_storage may "sync" a string buffer to the underlaying
    // Semantic Actions object if the size of the string buffer becomes larger
    // than the available storage space and if the size of the current buffer is 
    // larger than MIN_STRING_SIZE. Synchronizing the buffer will call the 
    // Semantic Actions function value_string_write(), which is not appropriate 
    // for "key strings". This shall only happen when the string is a "data string".
    // Thus, for any key string, its size (in code units) shall be smaller than 
    // MIN_STRING_SIZE!
    // The semantic actions object needs to take care of an overflowing key 
    // string.
    
    //  Models string_storage:
    //
    //  void append(buffer_type const& buffer);
    //  void append(code_unit_type code_unit);
    //  buffer_type buffer() const;
    //  void reset();
    //  size_t size() const;
    //  void extend(size_t size);
    //
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::encoding_traits;
    using json::internal::host_endianness;
    using json::unicode::add_endianness;
    using json::unicode::converter;
    
    
    struct no_partial_keystring_error : public std::runtime_error
    {
        no_partial_keystring_error() : std::runtime_error("string_storage: cannot write partial key string") 
        {}
    };
    

    template <typename EncodingT, typename SemanticActionsT>
    class string_storage : boost::noncopyable
    {
        // Although not strictly required, EncodingT's endianness shall be
        // equal host endianness:
        BOOST_STATIC_ASSERT( (boost::is_same<
                              typename encoding_traits<typename add_endianness<EncodingT>::type>::endian_tag,
                              typename host_endianness::type
                              >::value == true)  );
        
        // For efficiency reasons, EncodingT shall be either UTF_32_encoding_tag in
        // host endianness, or it shall be equal the SemanticActionsT's encoding:
        // (With the rule above, this also implies, that SemanticActionsT's encoding 
        // must have host endianness as well)
        BOOST_STATIC_ASSERT((boost::mpl::or_<
                             boost::is_same<
                             typename add_endianness<EncodingT>::type, 
                             typename add_endianness<UTF_32_encoding_tag>::type
                             >
                             , boost::is_same<EncodingT, typename SemanticActionsT::encoding_t>
                             >::value == true));
        
    public:
        enum Mode {Key, Data};
        
        typedef EncodingT                                               encoding_type;
        typedef typename encoding_traits<EncodingT>::code_unit_type     code_unit_type;
        typedef typename std::pair<code_unit_type*, std::size_t>        buffer_type;        
        typedef typename std::pair<code_unit_type const*, std::size_t>  const_buffer_type;        
        typedef code_unit_type*                                         storage_pointer;
        typedef typename SemanticActionsT::encoding_t                   sa_encoding_type;
        
        
    public:
        string_storage(SemanticActionsT& sa, Mode mode = Data) 
        :   sa_(sa), storage_start_(0), storage_end_(0), storage_cap_(0),
            enable_partial_strings_(false), mode_(mode)
        {
        }
        
        string_storage(SemanticActionsT& sa, std::size_t initial_storage_capacity, Mode mode = Data)
        :   sa_(sa),
            enable_partial_strings_(false), mode_(mode)
        {
            storage_end_ = storage_start_ = (code_unit_type*)malloc(initial_storage_capacity*sizeof(code_unit_type));
            storage_cap_ = storage_start_ + initial_storage_capacity;   
            stack_.reserve(initial_storage_capacity/16);
        }
        
        ~string_storage() {
            free(storage_start_);
        }
        
        
        // If member enable_partial_strings_ is true, large strings may be
        // split into chunks, otherwise the internal buffer's size will be 
        // extented as needed.
        void enable_partial_strings(bool set) {
            enable_partial_strings_ = set;
        }
        bool partial_strings_enabled() const { return enable_partial_strings_; }
        
        // A sync is effectively not allowed for key strings, since the semantic
        // actions interface is not suitable for this.
        // If a sync is triggered, and the current mode is set to "Key" an 
        // exception will be thrown.
        // A client shall set the mode before using the storage buffer, otherwise
        // the semantic actions object would perform a data_string_write, which
        // is a logic error when storing key strings. Setting the mode does not
        // prevent this from happen, but it is a measurement to signal this error 
        // and catch it appropriately in order to set suitable parser error,
        // e.g.: "JSON Key string too large", etc.
        // In order to prevent an exception because of an invalid attempt to
        // sync() the client may disable "partial strings" through calling
        // enable_partial_strings(false). Then the internal storage will grow
        // as needed. However, this may cause other issues in corner cases (e.g.
        // very large key strings will cause the buffer to grow very large as well)
        void set_mode(Mode mode) { mode_ = mode; }
        Mode mode() const { return Data; }
        
        
        // Extend the buffer in order to hold additional 'size' code units. The
        // buffer might possibly grow in order to hold at least 'size' code 
        // units, and it might possibly sync the current buffer.
        void extend(size_t size) 
        {
            // If the available code units in the buffer are equal or greater
            // than the requested size do nothing.
            // Otherwise, if the current size of the buffer is greater or equal 
            // than MIN_STRING_SIZE and synching is enabled perform a sync and 
            // grow the buffer if required. Otherwise, if the size of the buffer 
            // is smaller than MIN_STRING_SIZE grow the buffer by at least the 
            // requested size.
            // In any case, guarantee the buffer can hold the requested number 
            // of code units.
            //
            // Synching the buffer writes the current characters from the buffer 
            // to the underlaying Semantic Actions object as a "partial string" 
            // and then restes the current buffer.
            //
            // Synching requires that we take care of the boundaries of a possibly
            // multi byte sequence. 
            // 
            // The current implementation simply does not split any partial strings 
            // by itself, but just takes them as they come in.
            
            if (size > storage_avail()) {
                if (this->enable_partial_strings_ && (this->size() >= MIN_STRING_SIZE)) {
                    sync();
                    if (size > storage_avail()) {                    
                        storage_grow(size + storage_size());
                    }
                }
                else {
                    storage_grow(size + storage_size());
                }
            }
        }
        
        // Append the content of the string buffer 'buffer' to the current string 
        // buffer on top of the stack. A string buffer always starts at the 
        // beginning of a character and ends with a complete character. When
        // synching we must take care of these boundaries, that is we cannot
        // arbitrarily cut a possibly multi byte sequence.
        void append(const const_buffer_type& buffer)
        {
            extend(buffer.second);
            storage_end_ = std::uninitialized_copy(buffer.first, buffer.first + buffer.second, storage_end_);
            assert(storage_end_ <= storage_cap_);
        }
        
        // Append the code unit 'code_unit' to the current string buffer on top of the stack
        void append(code_unit_type code_unit)
        {
            if (__builtin_expect(storage_end_ == storage_cap_, 0)) {
                extend(1);
            }
            *storage_end_++ = code_unit;
        }
        
        // Return the current buffer on top of the stack
        const_buffer_type buffer() const 
        {
            if (stack_size()) {
                code_unit_type* p = storage_start_ + stack_.back();
                size_t size = static_cast<size_t>(std::distance(p, storage_end_));
                return const_buffer_type(p, size);
            }
            else {
                return (const_buffer_type(0,0));
            }
        }
        
        // Return the current buffer on top of the stack
        buffer_type buffer() 
        {
            if (stack_size()) {
                code_unit_type* p = storage_start_ + stack_.back();
                size_t size = static_cast<size_t>(std::distance(p, storage_end_));
                return buffer_type(p, size);
            }
            else {
                return (buffer_type(0,0));
            }
        }
        
        // Resets the current buffer on top of the stack, that is its size becomes 
        // zero.
        // The original content - that is, content past the new end pointer will
        // not be invalidated, unless it will be overridden or the storage needs
        // to be reallocated.
        void reset() {
            storage_end_ = storage_start_ + stack_.back();
        }
        
        // Returns the size of the buffer on top of the stack
        size_t size() const {
            if (stack_size() > 0)
                return static_cast<size_t>(std::distance(storage_start_ + stack_.back(), storage_end_));
            else
                return 0;
        }
        
        // Returns a referene to the current end pointer of the buffer on top of the stack.
        storage_pointer& dest() {
            return storage_end_;
        }
        
        // Pop a string buffer
        void stack_pop() {
            assert(stack_size()>0);
            storage_end_ = storage_start_ + stack_.back();
            stack_.pop_back();
        }
        
        // Push an empty string buffer
        void stack_push()
        {
            stack_.push_back(storage_size());
        }
        
        // Push a string buffer
        void stack_push(const const_buffer_type& buffer)
        {
            if (storage_avail() < buffer.second) {
                storage_grow(buffer.second + storage_capacity());
            }
            stack_.push_back(storage_size());
            storage_end_ = std::uninitialized_copy(buffer.first, buffer.first + buffer.second, storage_end_);
            assert(storage_end_ <= storage_cap_);
        }
        
        // Return the size of the stack
        size_t stack_size() const { return stack_.size(); }
        
        
        
        
        // Clears the stack and resets the storage
        void clear() {
            stack_.clear();
            storage_end_ = storage_start_;
        }
        
        // Reserve a storage size for the string buffers
        void storage_reserve(size_t size) {
            if (storage_capacity() < size) {
                storage_grow(size);
            }
        }
        
        size_t storage_avail() const {
            size_t result = static_cast<size_t>(std::distance(storage_end_, storage_cap_));
            return result;
        }
        
        size_t storage_capacity() const {
            return std::distance(storage_start_, storage_cap_);
        }
        
        size_t storage_size() const {
            return std::distance(storage_start_, storage_end_);
        }
        
        size_t min_string_size() const { 
            if (enable_partial_strings_)
                return MIN_STRING_SIZE; 
            else 
                return std::numeric_limits<size_t>::max()/sizeof(code_unit_type);
        }
        
        
        void flush() {
            if (mode_ == Key) {
                throw no_partial_keystring_error();
            }
            this->write<encoding_type>(false);
            reset();
        }
        
    protected:        
        typedef int seq_type;
        typedef std::vector<seq_type> seq_stack_type;
        
        
        void storage_grow(size_t min_size) {
            size_t newcap = storage_capacity();
            if (newcap == 0) {
                newcap = INITIAL_CAPACITY;
                stack_.reserve(INITIAL_CAPACITY/16);
            }
            while (min_size > newcap) {
                newcap += std::min(newcap, size_t(MAX_GROW_SIZE));
            }
            if (newcap > storage_capacity()) {
                size_t saved_size = storage_size();
                code_unit_type* new_start = (code_unit_type*)malloc(newcap*sizeof(code_unit_type));
                std::uninitialized_copy(storage_start_, storage_cap_, new_start);
                free(storage_start_);
                storage_start_ = new_start;
                storage_end_ = storage_start_ + saved_size;
                storage_cap_ = storage_start_ + newcap;
            }
        }
        
        
        void sync() {
            this->write<encoding_type>(true);
            reset();
        }
        
        
    private:
        
        struct write_policy_direct {};
        struct write_policy_inplace_convert {};
        
        template <typename E1, typename E2, class Enable = void>
        struct write_policy_traits {
            // Shall not be instantiated. Use different encodings to ensure 
            // efficient string buffers.
            BOOST_STATIC_ASSERT(sizeof(Enable)==0);  
        };
        
        template <typename E1, typename E2>
        struct write_policy_traits<E1, E2,
        typename boost::enable_if<
        boost::is_same<E1, E2>
        >::type
        > 
        {
            typedef write_policy_direct type;
        };
        
        
        template <typename E1, typename E2>
        struct write_policy_traits<E1, E2,
        typename boost::enable_if<
        boost::mpl::and_<
        boost::is_same<typename encoding_traits<E1>::encoding_form, UTF_32_encoding_tag>,
        boost::mpl::not_<
        boost::is_same<typename encoding_traits<E2>::encoding_form, UTF_32_encoding_tag>
        >
        >
        >::type
        > 
        {
            typedef write_policy_inplace_convert type;
        };
        
        
        
        
        
        
        // The string buffer's encoding and the semantic actions' encoding are
        // the same:
        template <typename E>
        void 
        write(bool hasMore, 
              typename boost::enable_if<
              boost::is_same<typename write_policy_traits<E, sa_encoding_type>::type, write_policy_direct>    
              >::type* dummy = 0) 
        {
            sa_.value_string(buffer(), hasMore);
        }
        
        // The string buffer's encoding can be converted "in place" to the 
        // semantic actions' encoding:
        template <typename E>
        void 
        write(bool hasMore,
              typename boost::enable_if<
              boost::is_same<typename write_policy_traits<E, sa_encoding_type>::type, write_policy_inplace_convert>    
              >::type* dummy = 0) 
        {
            typedef converter<E, sa_encoding_type, unicode::Validation::UNSAFE, unicode::Stateful::No, unicode::ParseOne::No> cvt_t;
            typedef typename encoding_traits<sa_encoding_type>::code_unit_type sa_char_t;            
            
            code_unit_type const* first = storage_start_;
            code_unit_type const* last = storage_end_;
            sa_char_t* start = reinterpret_cast<sa_char_t*>(static_cast<void*>(storage_start_));
            sa_char_t* dest = start;
#if defined (DEBUG)
            int cvt_result = 
#endif            
            cvt_t().convert(first, last, dest);
            assert(cvt_result == 0);
            size_t length = static_cast<size_t>(std::distance(start, dest));
            sa_.value_string(const_buffer_t(start, length), hasMore);
        }
        
        
    private:
        
        static const size_t MAX_GROW_SIZE = 4*1024;
        static const size_t INITIAL_CAPACITY = 1024;
        static const size_t MIN_STRING_SIZE = MAX_GROW_SIZE/2;
        
        SemanticActionsT&   sa_;        
        code_unit_type*     storage_start_;
        code_unit_type*     storage_end_;
        code_unit_type*     storage_cap_;
        seq_stack_type      stack_;
        Mode                mode_;
        bool                enable_partial_strings_;
    };
    
}}



#endif  // JSON_PARSER_INTERNAL_STRING_STORAGE1_HPP

//
//  string_storage3.hpp
//  
//
//  Created by Andreas Grosam on 4/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_PARSER_INTERNAL_STRING_STORAGE3_HPP
#define JSON_PARSER_INTERNAL_STRING_STORAGE3_HPP


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
    
    struct string_buffer_overflow_error : public std::runtime_error
    {
        string_buffer_overflow_error() : std::runtime_error("string_storage: string buffer too small") 
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
        string_storage(SemanticActionsT& sa) 
        :   sa_(sa), storage_start_(storage_), storage_end_(storage_), storage_cap_(storage_ + sizeof(storage_)),
        enable_partial_strings_(false) /*, mode_(mode)*/
        {
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
        void set_mode(Mode mode) { /*mode_ = mode;*/ }
        //Mode mode() const { return mode_; }
        
        
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
            
            if (__builtin_expect(size <= storage_avail(), 1)) {
                return;
            }
            else {
                extend_priv(size);
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
        
        // Return the current buffer
        const_buffer_type buffer() const 
        {
            return const_buffer_type(storage_start_, size());
        }
        
        // Return the current buffer on top of the stack
        buffer_type buffer() 
        {
            return buffer_type(storage_start_, size());
        }
        
        // Resets the current buffer, that is its size becomes zero.
        // The original content - that is, content past the new end pointer will
        // not be invalidated, unless it will be overridden or the storage needs
        // to be reallocated.
        void reset() {
            storage_end_ = storage_start_;
        }
        
        // Returns the size of the buffer
        size_t size() const {
            return static_cast<size_t>(std::distance(storage_start_, storage_end_));
        }
        
        // Returns a referene to the current end pointer of the buffer
        storage_pointer& dest() {
            return storage_end_;
        }
        
        
        // resets the storage
        void clear() {
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
            this->write<encoding_type>(false);
            reset();
        }
        
    protected:                
        
        void storage_grow(size_t min_size) {
            throw string_buffer_overflow_error();
        }
        
        
        void sync() {
            this->write<encoding_type>(true);
            reset();
        }
        
        
    private:
        
        void extend_priv(size_t size)
        {
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
        inline void 
        write(bool hasMore, 
              typename boost::enable_if<
              boost::is_same<typename write_policy_traits<E, sa_encoding_type>::type, write_policy_direct>    
              >::type* dummy = 0) 
        {
            sa_.value_string(const_buffer_type(storage_start_, size()), hasMore);
            //            
            //            if (mode_ == Data) {
            //                sa_.value_string(buffer(), hasMore);
            //            }
            //            else {
            //                throw no_partial_keystring_error();
            //            }
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
            
            //            if (mode_ == Key) {
            //                throw no_partial_keystring_error();
            //            }
            
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
        
        static const size_t CAPACITY = 16*1024;
        static const size_t MIN_STRING_SIZE = CAPACITY;
        
        SemanticActionsT&   sa_;        
        code_unit_type*     storage_start_;
        code_unit_type*     storage_end_;
        code_unit_type*     storage_cap_;
        code_unit_type      storage_[CAPACITY];
        bool                enable_partial_strings_;
    };
    
}}



#endif  // JSON_PARSER_INTERNAL_STRING_STORAGE3_HPP

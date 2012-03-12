//
//  string.hpp
//
//  Created by Andreas Grosam on 5/14/11.
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

#ifndef JSON_STRING_HPP
#define JSON_STRING_HPP


#include "json/config.hpp"

#if !defined (BOOST_SP_USE_QUICK_ALLOCATOR)
    #define BOOST_SP_USE_QUICK_ALLOCATOR
#endif

#include <boost/config.hpp>
#include <boost/checked_delete.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <memory>
#include <algorithm>   // std::swap
#include <string>
#include <assert.h>
#include <ios>
#include <cstring>
#include "json_traits.hpp"
#include "json/utility/string_hasher.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_traits.hpp"




    
    
namespace json { namespace internal { namespace string_detail {
    
    template <typename CharT> struct ref_counted_char_array;    

    template <typename CharT> 
    void intrusive_ptr_add_ref(json::internal::string_detail::ref_counted_char_array<CharT> const* s); 
    
    template <typename CharT> 
    void intrusive_ptr_release(json::internal::string_detail::ref_counted_char_array<CharT>* s); 

}}}


namespace json { namespace internal { namespace string_detail {
    
    
    // This class shall be a "standard-layout-class"!
    template <typename CharT>
    struct ref_counted_char_array
    {
    private:  
        mutable boost::detail::atomic_count ref_count_;
        size_t size_;
        CharT data_[1]; 
        
    public:        
        ref_counted_char_array() : size_(0), ref_count_(0) {}
        ref_counted_char_array(size_t size) : size_(size), ref_count_(0)  {}
        
        size_t size() const { return size_; }
        
        long ref_count() const { 
            return ref_count_; 
        }
        
        const CharT* data() const { 
            return &(data_[0]); 
        }
        
        CharT* data() { 
            return &(data_[0]); 
        }
        
        //
        // friends
        //
        template <typename T>
        friend inline
        void intrusive_ptr_add_ref(ref_counted_char_array<T> const* s);
        
        template <typename T>
        friend inline
        void intrusive_ptr_release(ref_counted_char_array<T>* s);        
        
    };
    
    
    
}}}  // namespace json::internal::string_detail   



namespace json { namespace internal { namespace string_detail {

    template <typename CharT>
    inline void intrusive_ptr_add_ref(json::internal::string_detail::ref_counted_char_array<CharT> const* s) 
    {
        assert(s->ref_count_ >= 0);
        assert(s != 0);
        ++s->ref_count_;
    }
    
    template <typename CharT>
    inline void intrusive_ptr_release(json::internal::string_detail::ref_counted_char_array<CharT>* s)
    {
        assert(s->ref_count_ > 0);
        assert(s != 0);
        if (--s->ref_count_ == 0)
            free(s); 
    }
    
}}} // namespace json::internal::string_detail 




namespace json {
    
    template<typename CharT>
    struct char_traits
    {
    };
    
}

namespace json {
    
    using unicode::UTF_8_encoding_tag;
    using unicode::encoding_traits;
    
    template<>
    struct char_traits<encoding_traits<UTF_8_encoding_tag>::code_unit_type>
    {
        BOOST_STATIC_ASSERT( sizeof(encoding_traits<UTF_8_encoding_tag>::code_unit_type) == sizeof(char) );
        
        typedef encoding_traits<UTF_8_encoding_tag>::code_unit_type              char_type;
        typedef int               int_type;
        typedef std::streampos    pos_type;
        typedef std::streamoff    off_type;
        typedef std::mbstate_t    state_type;
        
        static void
        assign(char_type& c1, const char_type& c2)
        { c1 = c2; }
        
        static  bool
        eq(const char_type& c1, const char_type& c2)
        { return c1 == c2; }
        
        static  bool
        lt(const char_type& c1, const char_type& c2)
        { return c1 < c2; }
        
        static int
        compare(const char_type* s1, const char_type* s2, size_t n)
        { return memcmp(s1, s2, n); }
        
        static std::size_t
        length(const char_type* s)
        { return strlen(reinterpret_cast<const char*>(s)); }
        
        static const char_type*
        find(const char_type* s, size_t n, const char_type& a)
        { return static_cast<const char_type*>(memchr(s, a, n)); }
        
        static char_type*
        move(char_type* s1, const char_type* s2, size_t n)
        { return static_cast<char_type*>(memmove(s1, s2, n)); }
        
        static char_type*
        copy(char_type* s1, const char_type* s2, size_t n)
        { return static_cast<char_type*>(memcpy(s1, s2, n)); }
        
        static char_type*
        assign(char_type* s, size_t n, char_type a)
        { return static_cast<char_type*>(memset(s, a, n)); }
        
        static char_type
        to_char_type(const int_type& c)
        { return static_cast<char_type>(c); }
        
        static int_type
        to_int_type(const char_type& c)
        { return static_cast<int_type>(static_cast<unsigned char>(c)); }
        
        static bool
        eq_int_type(const int_type& c1, const int_type& c2)
        { return c1 == c2; }
        
        static int_type
        eof()
        { return static_cast<int_type>(-1); }
        
        static int_type
        not_eof(const int_type& c)
        { return (c == eof()) ? 0 : c; }
    };
    
}


namespace json { 
    
    using unicode::UTF_16_encoding_tag;
    using unicode::encoding_traits;
    
    template<>
    struct char_traits<encoding_traits<UTF_16_encoding_tag>::code_unit_type>
    {
        typedef encoding_traits<UTF_16_encoding_tag>::code_unit_type          char_type;
        typedef uint16_t          int_type;
        typedef std::streamoff    off_type;
        typedef std::streampos    pos_type;
        typedef mbstate_t         state_type;
        
        static void
        assign(char_type& c1, const char_type& c2)
        { c1 = c2; }
        
        static bool
        eq(const char_type& c1, const char_type& c2)
        { return c1 == c2; }
        
        static bool
        lt(const char_type& c1, const char_type& c2)
        { return c1 < c2; }
        
        static int
        compare(const char_type* s1, const char_type* s2, std::size_t n)
        {
            for (std::size_t i = 0; i < n; ++i)
                if (lt(s1[i], s2[i]))
                    return -1;
                else if (lt(s2[i], s1[i]))
                    return 1;
            return 0;
        }
        
        static std::size_t
        length(const char_type* s)
        {
            std::size_t i = 0;
            while (!eq(s[i], char_type()))
                ++i;
            return i;
        }
        
        static const char_type*
        find(const char_type* s, std::size_t n, const char_type& a)
        {
            for (std::size_t i = 0; i < n; ++i)
                if (eq(s[i], a))
                    return s + i;
            return 0;
        }
        
        static char_type*
        move(char_type* s1, const char_type* s2, std::size_t n)
        {
            return (static_cast<char_type*>
                    (memmove(s1, s2, n * sizeof(char_type))));
        }
        
        static char_type*
        copy(char_type* s1, const char_type* s2, std::size_t n)
        {
            return (static_cast<char_type*>
                    (memcpy(s1, s2, n * sizeof(char_type))));
        }
        
        static char_type*
        assign(char_type* s, std::size_t n, char_type a)
        {
            for (std::size_t i = 0; i < n; ++i)
                assign(s[i], a);
            return s;
        }
        
        static char_type
        to_char_type(const int_type& c)
        { return char_type(c); }
        
        static int_type
        to_int_type(const char_type& c)
        { return int_type(c); }
        
        static bool
        eq_int_type(const int_type& c1, const int_type& c2)
        { return c1 == c2; }
        
        static int_type
        eof()
        { return static_cast<int_type>(-1); }
        
        static int_type
        not_eof(const int_type& c)
        { return eq_int_type(c, eof()) ? 0 : c; }
    };
}


namespace json {
    
    using unicode::UTF_32_encoding_tag;
    using unicode::encoding_traits;

    template<>
    struct char_traits<encoding_traits<UTF_32_encoding_tag>::code_unit_type>
    {
        typedef encoding_traits<UTF_32_encoding_tag>::code_unit_type          char_type;
        typedef uint32_t          int_type;
        typedef std::streamoff    off_type;
        typedef std::streampos    pos_type;
        typedef mbstate_t         state_type;
        
        static void
        assign(char_type& c1, const char_type& c2)
        { c1 = c2; }
        
        static bool
        eq(const char_type& c1, const char_type& c2)
        { return c1 == c2; }
        
        static bool
        lt(const char_type& c1, const char_type& c2)
        { return c1 < c2; }
        
        static int
        compare(const char_type* s1, const char_type* s2, std::size_t n)
        {
            for (std::size_t i = 0; i < n; ++i)
                if (lt(s1[i], s2[i]))
                    return -1;
                else if (lt(s2[i], s1[i]))
                    return 1;
            return 0;
        }
        
        static std::size_t
        length(const char_type* s)
        {
            std::size_t i = 0;
            while (!eq(s[i], char_type()))
                ++i;
            return i;
        }
        
        static const char_type*
        find(const char_type* s, std::size_t n, const char_type& a)
        {
            for (std::size_t i = 0; i < n; ++i)
                if (eq(s[i], a))
                    return s + i;
            return 0;
        }
        
        static char_type*
        move(char_type* s1, const char_type* s2, std::size_t n)
        {
            return (static_cast<char_type*>(memmove(s1, s2, n * sizeof(char_type))));
        }
        
        static char_type*
        copy(char_type* s1, const char_type* s2, std::size_t n)
        { 
            return (static_cast<char_type*>(memcpy(s1, s2, n * sizeof(char_type))));
        }
        
        static char_type*
        assign(char_type* s, std::size_t n, char_type a)
        {
            for (std::size_t i = 0; i < n; ++i)
                assign(s[i], a);
            return s;
        }
        
        static char_type
        to_char_type(const int_type& c)
        { return char_type(c); }
        
        static int_type
        to_int_type(const char_type& c)
        { return int_type(c); }
        
        static bool
        eq_int_type(const int_type& c1, const int_type& c2)
        { return c1 == c2; }
        
        static int_type
        eof()
        { return static_cast<int_type>(-1); }
        
        static int_type
        not_eof(const int_type& c)
        { return eq_int_type(c, eof()) ? 0 : c; }
    };
}


namespace json {
    
    using unicode::utf_encoding_tag;
    using unicode::UTF_8_encoding_tag;
    using unicode::UTF_16_encoding_tag;
    using unicode::UTF_16LE_encoding_tag;
    using unicode::UTF_16BE_encoding_tag;
    using unicode::UTF_32_encoding_tag;
    using unicode::UTF_32LE_encoding_tag;
    using unicode::UTF_32BE_encoding_tag;
    using unicode::platform_encoding_tag;
    using unicode::encoding_traits;
    
    //
    // class string
    // 
    // string holds a pointer to a char array which may be shared between different
    // instances of strings.
    //
    // Note: the tempalate specialization is required to avoid ambigous
    // or undesired implicit instantiations.
    
    template <typename EncodingT, class Enable = void>
    class string {
        //BOOST_STATIC_ASSERT(false);
    };
    
    
    template <typename EncodingT>
    class string<EncodingT,
        typename boost::enable_if<
            boost::is_base_and_derived<utf_encoding_tag, EncodingT> 
        >::type
    >
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<utf_encoding_tag, EncodingT>::value) );
    public:
        typedef typename encoding_traits<EncodingT>::code_unit_type       char_type;
        BOOST_STATIC_ASSERT( (sizeof(char_type) > 0) );
        
    private:        
        typedef internal::string_detail::ref_counted_char_array<char_type> ref_counted_char_array;
        typedef boost::intrusive_ptr<ref_counted_char_array> intrusive_char_ptr;
        
        intrusive_char_ptr      buffer_;
        static char_type const  s_zero_;
        
    public:        
        string() {}
        
        string(const string& other) 
        : buffer_(other.buffer_) {
        }
        
        ~string() {}
        
        string(const char_type* s, std::size_t len) {
            initialize(s, len, EncodingT());
        }
        
        template <typename T>
        string(const T* s, std::size_t len,
               typename boost::enable_if<
               boost::mpl::and_<
                boost::is_same<T, char>,
                boost::mpl::not_<boost::is_same<T, char_type> >
               >
               >::type* dummy = 0)
        {
            initialize(s, len, UTF_8_encoding_tag());
        }
        
        
        string(const char_type* s) {
            typedef typename unicode::char_type_to_encoding<char_type>::type encoding_t;
            const char_type* last = s;
            while (*last) { ++last; }
            initialize(s, last-s, EncodingT());
        }
    
        template <typename T>
        string(const T* s,
               typename boost::enable_if<
                    boost::mpl::and_<
                        boost::is_same<T, char>,
                        boost::mpl::not_<boost::is_same<T, char_type> >
                    >
               >::type* dummy = 0)
        {
            const char* last = s;
            while (*last) { ++last; }
            initialize(s, std::size_t(last-s), UTF_8_encoding_tag());
        }
        
        template <int N>
        string(const char s[N])
        {
            initialize(s, N, UTF_8_encoding_tag());
        }
        
        template <typename TraitsT, typename AllocT>
        string(const std::basic_string<char_type, TraitsT, AllocT>& stdstr) {
            typedef typename TraitsT::char_type char_type;
            typedef typename unicode::char_type_to_encoding<char_type>::type encoding_type;
            initialize(stdstr.c_str(), stdstr.size(), encoding_type());
        }
        
        string& operator= (const string& other) {
            if (this != &other) {
                buffer_ = other.buffer_;
            }
            return *this;
        }
        string& operator= (const char_type* s) {
            string tmp(s);
            swap(tmp);
            return *this;
        }
      
        template <typename T>
        typename boost::enable_if<
            boost::mpl::and_<
                boost::is_same<T, char>,
                boost::mpl::not_<boost::is_same<T, char_type> >
            >,
            string&
        >::type
        operator= (const char* s)
        {
            string tmp(s);
            swap(tmp);
            return *this;
        }
        
        
        
        
        template <typename TraitsT, typename AllocT>
        string& operator= (const std::basic_string<char_type, TraitsT, AllocT>& stdstr) {
            string tmp(stdstr);
            swap(*this, tmp);
            return *this;
        }
        
        
        const char_type*    c_str() const  { return buffer_ ? buffer_->data() : &string::s_zero_; }
        std::size_t     size() const   { return buffer_ ? buffer_->size() : 0; }
        
        
        void swap(string& other) {
            boost::swap(buffer_, other.buffer_);
        }
        
        //
        // Implementation specific:
        // 
        long ref_count() const { return buffer_ ? buffer_->ref_count() : 0; }
        
    private:
        
        
        template <typename Char, typename Encoding>
        void initialize(const Char* s, std::size_t len, Encoding encoding,
                        typename boost::enable_if<
                        boost::is_same<Encoding, EncodingT>
                        >::type* dummy = 0)
        {
            BOOST_STATIC_ASSERT( (sizeof(Char) == sizeof(typename encoding_traits<EncodingT>::code_unit_type)) );
            
            if (len and s) {
                // allocate a buffer big enough to hold the struct ref_counted_char_array 
                // plus len CharTs and one char_type for zero termination (note: 
                // sizeof(ref_counted_char_ptr counts for one char_type already): 
                void* p = malloc(sizeof(ref_counted_char_array) + len*sizeof(char_type));
                // Construct the ref_counted_char_array into the buffer and also memcpy the
                // given string into it:
                ref_counted_char_array* chref = new (p) ref_counted_char_array(len);  
                char_traits<char_type>::copy(chref->data(), reinterpret_cast<const char_type*>(s), len);
                //std::copy(s, s+len, chref->data()); 
                //memcpy(chref->data(), s, len*sizeof(char_type));
                chref->data()[len] = 0;
                buffer_ = chref;
            } else {
                // buffer_ = 0;  already default constructed
            }
        }
        
        template <typename Char, typename Encoding>
        void initialize(const Char* s, std::size_t len, Encoding encoding,
                        typename boost::enable_if<
                            boost::mpl::not_<boost::is_same<EncodingT, Encoding> >
                        >::type* dummy = 0)
        {
            assert("not yet implemented" == NULL);
        }
        
        
    private:
        
        //
        //  Comparisons
        // 
        friend inline 
        bool        
        operator== (const string<EncodingT>& lhv, const string<EncodingT>& rhv) {
            typedef typename encoding_traits<EncodingT>::code_unit_type char_type;
            if (lhv.buffer_ == rhv.buffer_) 
                return true;
            return ( 0 == char_traits<char_type>::compare(lhv.c_str(), rhv.c_str(), std::min(lhv.size(), rhv.size()) + 1 ) );
        }
        
        friend inline 
        bool        
        operator< (const string<EncodingT>& lhv, const string<EncodingT>& rhv) {
            typedef typename encoding_traits<EncodingT>::code_unit_type char_type;
            return ( char_traits<char_type>::compare(lhv.c_str(), rhv.c_str(), std::min(lhv.size(), rhv.size()) + 1) < 0 );
        }
        
        
        friend inline 
        bool        
        operator!= (const string<EncodingT>& lhv, const string<EncodingT>& rhv) {
            return ! (lhv == rhv);
        }
        
        friend inline 
        bool        
        operator> (const string<EncodingT>& lhv, const string<EncodingT>& rhv) {
            typedef typename encoding_traits<EncodingT>::code_unit_type char_type;
            return ( char_traits<char_type>::compare(lhv.c_str(), rhv.c_str(), std::min(lhv.size(), rhv.size()) + 1) > 0 );
        }
        
        /*
        // Stream operator
        friend inline
        std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const string<EncodingT>& s) {
            std::ostream_iterator<char> out_iter;
            unicode::convert_unsafe();
            return os;
        }
        */
        
        
    };
    
    
    template<typename EncodingT> 
    typename string<EncodingT,typename boost::enable_if<boost::is_base_and_derived<utf_encoding_tag, EncodingT> >::type>::char_type const 
    string<EncodingT, typename boost::enable_if<boost::is_base_and_derived<utf_encoding_tag, EncodingT> >::type>::s_zero_ = 0;

    
    
    template <typename EncodingT>    
    inline typename boost::enable_if<boost::is_base_and_derived<utf_encoding_tag, EncodingT>, void>::type 
    swap(string<EncodingT>& lhv, string<EncodingT>& rhv) {
        lhv.swap(rhv);
    } 
    
        
    template <typename EncodingT> 
    struct is_json_type<string<EncodingT> > : public boost::mpl::true_
    { 
        static const bool value = true; 
    };
    
    

    template <typename EncodingT> 
    inline 
    typename boost::enable_if<boost::is_same<UTF_8_encoding_tag, EncodingT>, std::basic_ostream<char>&>::type 
    operator<<(std::basic_ostream<char>& os, const string<EncodingT>& s) {
        os << s.c_str();
        return os;
    }
    
//    template <typename EncodingT> 
//    inline 
//    typename boost::enable_if<boost::is_same<UTF_16_encoding_tag, EncodingT>::type, std::basic_ostream<EncodingT>&>::type 
//    std::basic_ostream<EncodingT>& 
//    operator<<(std::basic_ostream<EncodingT>& os, const string<EncodingT>& s) {
//        assert("not yet implemented" == NULL);
//        return os;
//    }
    

    // boost hash
    template <typename EncodingT>
    inline typename boost::enable_if<boost::is_base_and_derived<utf_encoding_tag, EncodingT>, std::size_t>::type 
    hash_value(const string<EncodingT> s) {
        typedef typename encoding_traits<EncodingT>::code_unit_type char_type;
        return json::utility::string_hasher<char_type>()(s.c_str(), s.size());
    }
    
}  // namespace json



#endif // JSON_STRING_HPP

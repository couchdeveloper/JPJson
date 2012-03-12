//
//  intrusive_json_path.hpp
//  
//
//  Created by Andreas Grosam on 12/2/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_OBJC_INTRUSIVE_JSON_PATH_HPP
#define JSON_OBJC_INTRUSIVE_JSON_PATH_HPP


#include "json/config.hpp"
#include <vector>
#include <iterator>
#include <utility>
#include <algorithm>
#include <string>
#include <sstream>
#include <cstring>
#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <json/unicode/unicode_utilities.hpp>
#include <json/unicode/unicode_traits.hpp>
#include <json/unicode/unicode_converter.hpp>


namespace json { namespace objc { namespace objc_internal {
    
    using json::unicode::encoding_traits;
    
    // A json path is a vector of path components. A path component is either a 
    // string which represents the key of a key-value pair of a JSON object, or
    // an unsigned integral value which represents the index of a JSON array.
    // 
    // An intrusive json path stores a pointer to a string object rather than a
    // copy of the string. Index objects are strored directly, though.
    // More precisely, string objects are represented by  a std::pair<const 
    // char_type*, size_t> while the storage for the strings itself is managed 
    // externally.
    
    
    template <typename EncodingT, typename IndexT = std::size_t>
    class intrusive_json_path 
    {   
        // Encoding's endianness shall be equal host endianness!
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, EncodingT>::value) );                
        BOOST_STATIC_ASSERT( (boost::is_integral<IndexT>::value) ); 
        
    public:
        typedef IndexT index_type;
        typedef typename encoding_traits<EncodingT>::code_unit_type char_type;
        typedef std::pair<const char_type*, size_t>  string_buffer_type;
        struct root_component {};
        typedef boost::variant<root_component, index_type, string_buffer_type> component_type;
        
    private:
        typedef std::vector<component_type> components_vector;
        
    private:    
        class are_strict_equals
        : public boost::static_visitor<bool>
        {
        public:
            
            template <typename T, typename U>
            bool operator()( const T &, const U & ) const
            {
                return false; // cannot compare different types
            }
            
            template <typename T>
            bool operator()( const T & lhs, const T & rhs ) const
            {
                return lhs == rhs;
            }
            
        };
        
        
        class ostream_visitor : public boost::static_visitor<>
        {
        public:    
            ostream_visitor(std::ostream& os, bool quote_keys = true) : os_(os), quote_keys_(quote_keys) {}
            
            void operator()(root_component) {
            }
            
            void operator()(string_buffer_type const& buffer) {
                std::ostream_iterator<char> dest(os_);
                const char_type* first = buffer.first;
                if (quote_keys_)
                    os_ << '"';
                int result = escape_convert_unsafe(
                      first, first + buffer.second, EncodingT(),
                      dest, json::unicode::UTF_8_encoding_tag());
                if (result != 0) {
                    throw std::runtime_error("Unicode conversion failed");
                }
                if (quote_keys_)
                    os_ << '"';
            }
            
            void operator()(index_type i) {
                os_ << i;
            }                        
            
        private:
            
            
            // Returns zero on success, otherwise the a negative integer indicating an error.
            template <
                typename InIteratorT, typename InEncodingT, 
                typename OutIteratorT, typename OutEncodingT
            >
            inline int
            escape_convert_unsafe(
                                  InIteratorT&     first, 
                                  InIteratorT      last, 
                                  InEncodingT      inEncoding,
                                  OutIteratorT&    dest,
                                  OutEncodingT     outEncoding) const
            {
                // !! Requires that encodings are in host endianness!
                typedef typename encoding_traits<InEncodingT>::code_unit_type in_char_t;
                typedef typename encoding_traits<OutEncodingT>::code_unit_type out_char_t;
                typedef json::unicode::converter<
                    InEncodingT, OutEncodingT, 
                    json::unicode::Validation::UNSAFE, json::unicode::Stateful::No, json::unicode::ParseOne::Yes
                > converter_t;
                converter_t cvt;
                while (first != last) {
                    if (*first == static_cast<in_char_t>('"')) {
                        *dest++ = static_cast<out_char_t>('\\');
                    }
                    
                    int result = cvt.convert(first, last, dest);
                    if (result < 0)
                        return result;
                }
                return 0;
            }    
            
        private:
            std::ostream& os_;
            bool quote_keys_;
        };
        
    public:
        intrusive_json_path() {
            components_.push_back(root_component());
        };
        
        void push_index(IndexT index) {
            components_.push_back(index_type(index));
        }
        
        index_type& back_index() {
            return boost::get<index_type>(components_.back());
        }
        
        void push_key(string_buffer_type const& buffer) {
            components_.push_back(buffer);
        }        
        
        string_buffer_type& back_key() {
            return boost::get<string_buffer_type>(components_.back());
        }
        
        
        void pop_component() {
            assert(components_.size() > 0);
            components_.pop_back();
        }
        
        void clear() {
            components_.clear();
            components_.push_back(root_component());
        }
        
        size_t level() const { return components_.size() - 1; }
        
        component_type const&
        component_at(size_t pos) const {
            assert(pos >= 0 and pos < components_.size());
            return components_[pos];
        }
        
        component_type&
        component_at(size_t pos) {
            assert(pos >= 0 and pos < components_.size());
            return components_[pos];
        }
        
        
        void write(std::ostream& os) const 
        {
            ostream_visitor vis(os);
            for (int i = 0; i < components_.size(); ++i) {
                boost::apply_visitor(vis, component_at(i));
                os << '/';
            }
        }
        
        std::string path() const 
        {
            std::stringstream ss(std::stringstream::out);
            write(ss);
            return ss.str();
        }
        
        
        
        /*        
         template <typename OtherEncodingT>
         std::basic_string<typename OtherEncodingT::code_unit_type> 
         path() const 
         {
         typedef typename OtherEncodingT::code_unit_type  CharT;
         typedef std::basic_string<CharT> string_type;
         assert(positions_.size());
         string_type result;
         std::back_insert_iterator<string_type> dest = std::back_insert_iterator<string_type> (result);
         for (size_t i = 0; i < positions_.size(); ++i) {
         const std::pair<ComponentType,key_type>& c = component_at(i);
         const char_type* first = c.second.first;
         const char_type* last = c.second.second;
         if (c.first == Key) {
         result.append(1, static_cast<CharT>('"'));
         --last;
         }
         int error = 0;
         size_t count = this->template escape_convert_unsafe(
         first, last, EncodingT(), 
         dest, OtherEncodingT(), 
         error);
         if (count == 0) {
         throw std::runtime_error("Unicode conversion failed");
         }
         if (c.first == Key) {
         result.append(1, static_cast<CharT>('"'));
         result.append(1, static_cast<CharT>('/'));
         }
         }
         return result;
         }
         */ 
        
    private:
        /*
         friend inline 
         std::ostream& <<(std::ostream& os, const intrusive_json_path& v) {
         v.write(os);
         return os;
         }
         */
        
        friend inline 
        bool operator== (intrusive_json_path const& lhv, intrusive_json_path const& rhv) {
            return (lhv.components_.size() == rhv.components_.size() and
                    std::equal(lhv.components_.begin(), lhv.components_.end(), rhv.components_.begin(),
                               boost::apply_visitor(intrusive_json_path::are_strict_equals())));
        }
        
    private:
        components_vector components_;
    };
    
    
    
    
}}}  // namespace json::objc::objc_internal


#endif   // JSON_OBJC_INTRUSIVE_JSON_PATH_HPP



//
//  json_path.hpp
//  
//
//  Created by Andreas Grosam on 10/14/11.
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

#ifndef JSON_JSON_PATH_HPP
#define JSON_JSON_PATH_HPP


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
#include <boost/iterator/iterator_traits.hpp>
#include <json/unicode/unicode_utilities.hpp>
#include <json/unicode/unicode_traits.hpp>
#include <json/unicode/unicode_converter.hpp>


namespace json { namespace json_internal {
    
    using json::unicode::encoding_traits;
    
    
    template <typename EncodingT, typename IndexT = std::size_t>
    class json_path 
    {   
        // Encoding's endianness shall be equal host endianness!
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, EncodingT>::value) );                
        BOOST_STATIC_ASSERT( (boost::is_integral<IndexT>::value) ); 
        
        // Ensure the start of a sequence of code_units and Index values are propery aligned
        // in the storage:
        BOOST_STATIC_ASSERT( (sizeof(IndexT) >= sizeof(typename encoding_traits<EncodingT>::code_unit_type)) ); 
        typedef typename boost::aligned_storage<sizeof(IndexT), boost::alignment_of<IndexT>::value>::type storage_value_type;
        typedef std::vector<storage_value_type> storage_type;
        

        enum ComponentType {
            RootComponent,
            KeyComponent,
            IndexComponent
        };
        typedef std::pair<ComponentType, size_t> pos_type;
        typedef std::vector<pos_type> positions_type;
        
        
    public:
        typedef EncodingT encoding_type;
        typedef typename encoding_traits<EncodingT>::code_unit_type char_type;        
        
        struct root_type {};
        typedef std::pair<const char_type*, const char_type*> key_type;
        typedef IndexT index_type;
        typedef boost::variant<root_type, key_type, index_type> path_component_type;
        
    private:
        
        class ostream_visitor : public boost::static_visitor<>
        {
        public:    
            ostream_visitor(std::ostream& os, bool quote_keys = true) : os_(os), quote_keys_(quote_keys) {}
            
            void operator()(root_type&) const {
            }
            
            void operator()(key_type& range) {
                std::ostream_iterator<char> dest(os_);
                const char_type* first = range.first;
                if (quote_keys_)
                    os_ << '"';
                int result = escape_convert_unsafe(
                        first, range.second, EncodingT(),
                        dest, json::unicode::UTF_8_encoding_tag());
                if (result != 0) {
                    throw std::runtime_error("Unicode conversion failed");
                }
                if (quote_keys_)
                    os_ << '"';
            }
            
            void operator()(IndexT& i) const {
                os_ << i;
            }                        
            
        private:
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
                // Use a unsafe, stateless converter which only converts one character:
                typedef unicode::converter<
                    InEncodingT, OutEncodingT, 
                    unicode::Validation::UNSAFE, unicode::Stateful::No, unicode::ParseOne::Yes
                >  converter_t;
                typedef typename encoding_traits<InEncodingT>::code_unit_type in_char_t;
                converter_t cvt;
                int result = 0;
                while (first != last) {
                    if (*first == static_cast<in_char_t>('"')) {
                        *dest++ = static_cast<in_char_t>('\\');
                    }
                    result = cvt.convert(first, last, dest);
                    if (result != 0) {
                        return result;
                    }
                }
                return result;
            }    
            
        private:
            std::ostream& os_;
            bool quote_keys_;
        };

    public:
        json_path() {
            positions_.push_back(pos_type(RootComponent, -1));
        };
        
        void push_index(IndexT index)
        {
            size_t storage_index = storage_.size();
            positions_.push_back(pos_type(IndexComponent, storage_index));
            storage_.resize(storage_index + 1);
            IndexT* dest = static_cast<IndexT*>(static_cast<void*>(&storage_[storage_index]));
            *dest = index;
        }
        
        index_type& back_index() {
            const pos_type& pc = positions_.back();
            assert(pc.first == IndexComponent);
            IndexT* index_ptr = static_cast<IndexT*>(static_cast<void*>(&storage_[pc.second]));
            return *index_ptr;
        }
        
        
        template <typename IteratorT>
        void push_key(IteratorT first, IteratorT last)
        {
            // The iterator's value size shall match the size of the code unit:
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<IteratorT>::type)
                                == sizeof(typename encoding_traits<EncodingT>::code_unit_type));
            assert(first != last);            
            positions_.push_back(pos_type(KeyComponent, storage_.size()));
            const size_t len = std::distance(first, last);
            size_t element_size = 1 + (sizeof(char_type)*len+(sizeof(storage_value_type)-1))/sizeof(storage_value_type);
            storage_.resize(storage_.size()+element_size);
            pos_type const& pc = positions_.back();
            IndexT* length_ptr = static_cast<IndexT*>(static_cast<void*>(&storage_[pc.second]));
            *length_ptr = static_cast<IndexT>(len);
            char_type* dest = static_cast<char_type*>(static_cast<void*>(&storage_[pc.second+1]));
            std::copy(first, last, dest);
        }        
        
        template <typename IteratorT>
        void back_key_assign(IteratorT first, IteratorT last)
        {
            pos_type const& pc = positions_.back();
            assert(pc.first == KeyComponent);
            size_t new_len = std::distance(first, last);
            size_t new_element_size = 1 + (sizeof(char_type)*new_len+(sizeof(storage_value_type)-1))/sizeof(storage_value_type);
            size_t old_element_size = storage_.size() - pc.second;
            storage_.resize(storage_.size() + new_element_size - old_element_size); 
            IndexT* length_ptr = static_cast<IndexT*>(static_cast<void*>(&storage_[pc.second]));
            *length_ptr = static_cast<IndexT>(new_len);
            char_type* dest = static_cast<char_type*>(static_cast<void*>(&storage_[pc.second+1]));
            std::copy(first, last, dest);
        }
        
        
        void pop_component() {
            assert(positions_.size() > 0);
            storage_.resize(positions_.back().second);
            positions_.pop_back();
        }
                
        void clear() {
            storage_.resize(1);
            positions_.resize(1);
        }
        
        size_t level() const { return positions_.size() - 1; }
        
        // Returns a path component at the specified position pos.
        // In case of a key component, the range specifies an unescaped sequence
        // of characters in encoding EncodingT.
        // In case of a index component the value specifies the index.
        // In case of a root component there is no value
        path_component_type
        component_at(size_t pos) const {
            assert(pos < positions_.size());
            switch (positions_[pos].first) {
                case RootComponent:
                    return path_component_type(root_type());
                    break;
                case KeyComponent: {
                    size_t storage_index = positions_[pos].second;
                    const size_t len = *static_cast<IndexT const*>(static_cast<void const*>(&storage_[storage_index]));
                    const char_type* first = static_cast<char_type const*>(static_cast<void const*>(&storage_[storage_index + 1]));
                    return path_component_type(key_type(first, first + len));
                    break;
                }
                case IndexComponent: {
                    size_t storage_index = positions_[pos].second;
                    IndexT index = *static_cast<IndexT const*>(static_cast<void const*>(&storage_[storage_index]));
                    return path_component_type(index);                    
                    break;
                }
            }
        }
        
        
        void write(std::ostream& os) const 
        {
            ostream_visitor vis(os);
            for (int i = 0; i < positions_.size(); ++i) {
                path_component_type pc = component_at(i);
                boost::apply_visitor(vis, pc);
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
        std::ostream& <<(std::ostream& os, const json_path& v) {
            v.write(os);
            return os;
        }
        */
        
    private:
        storage_type    storage_;
        positions_type  positions_;
    };
    
    
    
    
}}  // namespace json::json_internal


#endif   // JSON_JSON_PATH_HPP

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


#include <vector>
#include <iterator>
#include <utility>
#include <algorithm>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>
#include <json/unicode/unicode_utilities.hpp>
#include <json/unicode/unicode_conversions.hpp>


namespace json { namespace json_internal {
    
    
    struct path_component {};    
    struct key_path_component : path_component {};
    struct index_path_component : path_component {};
    
    template <typename EncodingT>
    class json_path 
    {   
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, EncodingT>::value) );                
    public:
        enum ComponentType {
            Root,
            Key,
            Index
        };
        typedef EncodingT encoding_type;
        typedef std::pair<ComponentType, size_t> pos_type;
        typedef typename EncodingT::code_unit_type char_type;        
        typedef std::pair<const char_type*, const char_type*> seq_range;
        typedef std::pair<ComponentType,seq_range> path_component;
    private:
        typedef std::vector<pos_type> positions_type;
        
    public:
        json_path()
        {
            positions_.push_back(pos_type(Root, path_.size()));
            path_.push_back(static_cast<char_type>('/'));
        };
        
        void pop_component() {
            assert(positions_.size() > 0);
            path_.resize(positions_.back().second);
            positions_.pop_back();
        }
        
        void push_index(size_t index)
        {
            std::string numberString = boost::lexical_cast<std::string>(index);
            positions_.push_back(pos_type(Index, path_.size()));
            std::copy(numberString.begin(), numberString.end(), std::back_inserter(path_));
            path_.push_back(static_cast<char_type>('/'));
        }
        
        template <typename IteratorT>
        void push_key(IteratorT first, IteratorT last)
        {
            assert(first != last);
            positions_.push_back(pos_type(Key, path_.size()));
            std::copy(first, last, std::back_inserter(path_));
            path_.push_back(static_cast<char_type>('/'));
        }
        
        void clear() {
            path_.resize(1);
            positions_.resize(1);
        }
        
        size_t level() const { return positions_.size() - 1; }
        
        // Returns a path component at the specified position pos containig the 
        // component type and the range specifing the path as an unescaped 
        // sequence of characters in encoding EncodingT (including the trailing '/').
        path_component
        component_at(size_t pos) const {
            assert(pos < positions_.size());
            const char_type* first = &path_[positions_[pos].second];
            const char_type* last;
            if (pos == positions_.size()-1) {
                last = &path_.back()+1;
            } else {
                last = &path_[0] + positions_[pos+1].second;
            }
            return std::pair<ComponentType,seq_range>(positions_[pos].first, seq_range(first,last));
        }
        
        
        // component_as() returns a path component containing the component
        // type and a copy of the path as a std::string converted to the 
        // specified encoding as a std::pair.
        // The path is unescaped and contains the trailing slash '/'.
        template <typename OtherEncodingT>
        std::pair<
            ComponentType,
            std::basic_string<typename OtherEncodingT::code_unit_type> 
        >
        component_as(size_t pos) const 
        {
            typedef std::basic_string<typename OtherEncodingT::code_unit_type> string_type;
            const path_component& pc = component_at(pos);
            const char_type* first = pc.second.first;
            std::pair<ComponentType,string_type> result = 
                std::pair<ComponentType,string_type>(pc.first, string_type());
            std::back_insert_iterator<string_type> dest = 
                std::back_insert_iterator<string_type>(result.second);
            int error;
            std::size_t count = 
                json::unicode::convert_unsafe(first, pc.second.second, EncodingT(),
                                          dest, OtherEncodingT(),
                                          error);
            if (count == 0) {
                throw std::runtime_error("Unicode conversion failed");
            }
            return result;
        }
        
        
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
                const std::pair<ComponentType,seq_range>& c = component_at(i);
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
        
    private:
        template <
            typename InIteratorT, typename InEncodingT, 
            typename OutIteratorT, typename OutEncodingT
        >
        inline std::size_t
        escape_convert_unsafe(
                              InIteratorT&     first, 
                              InIteratorT      last, 
                              InEncodingT      inEncoding,
                              OutIteratorT&    dest,
                              OutEncodingT     outEncoding,
                              int&             error) const
        {
            std::size_t count = 0;
            while (first != last) {
                if (*first == static_cast<typename InEncodingT::code_unit_type>('"')) {
                    *dest++ = static_cast<typename InEncodingT::code_unit_type>('\\');
                    ++count;
                }
                int result = json::unicode::convert_one_unsafe(first, last, inEncoding, 
                                                               dest, outEncoding);
                if (result > 0)
                    count += result;
                else {
                    error = result;
                    return 0;
                    break;
                }
            }
            error = 0;
            return count;
        }    
        
    private:
        std::vector<char_type> path_;
        positions_type positions_;
    };
    
    
    
    
}}  // namespace json::json_internal


#endif   // JSON_JSON_PATH_HPP

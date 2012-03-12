//
//  semantic_actions_test.hpp
//  
//
//  Created by Andreas Grosam on 7/22/11.
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

#ifndef JSON_INTERNAL_SEMANTIC_ACTIONS_TEST_HPP
#define JSON_INTERNAL_SEMANTIC_ACTIONS_TEST_HPP


#include "json/config.hpp"
#include "semantic_actions_base.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_conversion.hpp"
#include "json/generator/generate.hpp"
#include "json/value/string.hpp"
#include <map>
#include <stack>
#include <vector>
#include <iterator>
#include <assert.h>

namespace json { namespace internal {
    
    //
    //  Test only
    //  
    // Template parameter EncodingT shall be derived from json::utf_encoding_tag.
    // EncodingT specifies the StringBufferEncoding of the parser.
    //    
    template <typename EncodingT>
    class semantic_actions_test : public json::semantic_actions_base<semantic_actions_test<EncodingT>, EncodingT>
    {
    private:   
        typedef semantic_actions_base<semantic_actions_test, EncodingT> base;
        
    public:    
        typedef typename base::error_t          error_t;
        typedef typename base::char_t           char_t;
        typedef typename base::encoding_t       encoding_t;
        typedef typename base::nb_number_t      nb_number_t;
        
    private:    
        typedef boost::shared_ptr<boost::any>   value_t;
        typedef std::string                     string_t; 
        typedef std::string                     number_t;
        typedef bool                            boolean_t;
        struct null_t {};
        typedef std::vector<value_t>            array_t;
        typedef std::map<string_t, value_t>     object_t;
        typedef std::vector<value_t>            stack_t;
        
    public:    
        typedef value_t                         result_type;
        
    public:    
        semantic_actions_test() 
        :   array_count_(0), object_count_(0), string_count_(0), boolean_count_(0),
            null_count_ (0), number_count_(0)
        {
        }
        
        semantic_actions_test& operator= (const semantic_actions_test& other) {
            result_ = other.result_;
        }
        
        void parse_begin_imp() {}
        void parse_end_imp() { 
            if (stack_.size() != 1) 
                throw std::logic_error("json::internal::SemanticActionsTest: logic error");
            result_ = stack_.back();
            stack_.pop_back();
        }        
        void finished_imp() {}
        
        void begin_array_imp() {
            ++array_count_; 
            string_representation_.append(1, '[');
            markers_.push_back(stack_.size());
            stack_.push_back(value_t(new boost::any(array_t()))); 
        }
        
        void end_array_imp() {
            typedef typename stack_t::iterator iterator;
            string_representation_.append(1, ']');
            iterator container_iter = stack_.begin() + markers_.back();
            iterator first = container_iter + 1;
            array_t& a = boost::any_cast<array_t&>(*(*container_iter));
            while (first != stack_.end()) {
                a.push_back(*first++);
            }
            stack_.erase(container_iter+1, stack_.end());
            markers_.pop_back();
        }
        
        void begin_object_imp() { 
            ++object_count_; 
            string_representation_.append(1, '{');
            markers_.push_back(stack_.size());
            stack_.push_back(value_t(new boost::any(object_t()))); 
        }
        
        bool end_object_imp() 
        {            
            typedef typename stack_t::iterator iterator;
            typedef typename object_t::iterator object_iterator;
            string_representation_.append(1, '}');
            iterator container_iter = stack_.begin() + markers_.back();
            iterator first = container_iter + 1;
            bool result = true;
            object_t& o = boost::any_cast<object_t&>(*(*container_iter));
            while (first != stack_.end()) {
                string_t key = boost::any_cast<string_t&>(*(*first++));
                assert(first != stack_.end());
                value_t v = *first++;
                std::pair<object_iterator, bool> res = o.insert(std::pair<string_t, value_t>(key, v));
                if (res.second == false) {
                    while (0) {};
                }
                result = res.second and result;
            }
            stack_.erase(container_iter+1, stack_.end());
            markers_.pop_back();
            return result; 
        }
        
        void begin_value_at_index_imp(size_t index) {
            if (index > 0) {
                string_representation_.append(1, ',');
            }
        }
        
        void end_value_at_index_imp(size_t index) {}
        
        void begin_value_with_key_imp(const char_t* s, size_t len, size_t nth) {
            // push the key
            ++string_count_; 
            if (nth > 0) {
                string_representation_.append(1, ',');
            }
            string_representation_.append(1, '"');
            const char_t* first = s;
            std::back_insert_iterator<std::string> dest(string_representation_);
            int result = json::generator_internal::escape_convert_unsafe(
                first, first+len, EncodingT(),
                dest, json::unicode::UTF_8_encoding_tag(), 
                false /*escape solidus*/);   
            if (result != 0) {
                throw std::runtime_error("escaping JSON string failed");
            }
            string_representation_.append("\":");
            stack_.push_back(value_t(new boost::any(make_string(s, len)))); 
        }
        
        void end_value_with_key_imp(const char_t* s, size_t len, size_t nth) {}
        
        void value_string_imp(const char_t* s, std::size_t len) { 
            assert(s != 0 and *s != 0 and len > 0);
            ++string_count_; 
            string_representation_.append(1, '"');
            const char_t* first = s;
            std::back_insert_iterator<std::string> dest(string_representation_);
            int result = json::generator_internal::escape_convert_unsafe(
                first, first+len, EncodingT(),
                dest, json::unicode::UTF_8_encoding_tag(), 
                false /*escape solidus*/);   
            if (result != 0) {
                throw std::runtime_error("escaping JSON string failed");
            }
            string_representation_.append(1, '"');
            std::string value = make_string(s, len);
            stack_.push_back(value_t(new boost::any(value))); 
        }
        
        void value_number_imp(const nb_number_t& number) { 
            ++number_count_; 
            string_representation_.append(number.c_str(), number.c_str_len());
            stack_.push_back(value_t(new boost::any(number_t(number.c_str(), number.c_str_len())))); 
        }
        
        void value_boolean_imp(bool b) { 
            ++boolean_count_; 
            const char* v = b ? "true" : "false";
            string_representation_.append(v);
            stack_.push_back(value_t(new boost::any(boolean_t(b)))); 
        }
        
        void value_null_imp() { 
            ++null_count_; 
            string_representation_.append("null");
            stack_.push_back(value_t(new boost::any(null_t()))); 
        }
        
        
        void clear_imp() { 
            array_count_ = object_count_ = string_count_ = boolean_count_ = null_count_ = number_count_ = 0;
            stack_ = stack_t(); 
            result_ = result_type(); 
            string_representation_.clear();
            markers_.clear();
        }
        
        void error_imp(int code, const char* description) {
            error_.set(code, description);
        }
        void error_imp(const error_t& error) {
            error_ = error;
        }        
        const error_t& error_imp() const {
            return error_;
        }
        
        
        result_type&        result()          { return result_; }
        const result_type&  result() const    { return result_; }
        
        std::string         str() const       { return string_representation_; }
        
        
        int array_count()   const { return array_count_; }
        int object_count()  const { return object_count_; }
        int string_count()  const { return string_count_; }
        int boolean_count() const { return boolean_count_; }
        int null_count()    const { return null_count_; }
        int number_count()  const { return number_count_; }

        
        void print_imp(std::ostream& os) { 
            os << *this; 
        }                    
        
    protected:
        std::string make_string(const char_t* s, size_t len) 
        {
            assert(s != 0 and *s != 0 and len > 0);
            std::string result;
            std::back_insert_iterator<std::string> dest(result);
            const char_t* first = s;
            int cvt_result = unicode::convert(first, first + len, EncodingT(), 
                                              dest, json::unicode::UTF_8_encoding_tag());
            assert(cvt_result==unicode::NO_ERROR);
            return result;
        }
        
        
    protected:
        stack_t                 stack_;
        std::vector<size_t>     markers_;    
        result_type             result_;
        error_t                 error_;        
        std::string             string_representation_;
        
        int array_count_;
        int object_count_;
        int string_count_;
        int boolean_count_;
        int null_count_;
        int number_count_;
        
        
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const semantic_actions_test& sa) 
        {
            os << "SemanticActionsTest number of parsed items:\n"
            << "   object_count:  " << sa.object_count() << '\n'
            << "   array_count:   " << sa.array_count() << '\n'
            << "   string_count:  " << sa.string_count() << '\n'
            << "   number_count:  " << sa.number_count() 
            << "   boolean_count: " << sa.boolean_count() << '\n'
            << "   null_count:    " << sa.null_count() << '\n'        
            << std::endl;
            return os;
        }
        
    };
    
    
    
    
}} // namespace json::internal






#endif // JSON_INTERNAL_SEMANTIC_ACTIONS_TEST_HPP

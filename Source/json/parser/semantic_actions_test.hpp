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


#include "semantic_actions_base.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include "json/value/string.hpp"
#include <map>
#include <stack>
#include <vector>


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
        typedef json::string<char_t>            string_t; 
        typedef std::string                     number_t;
        typedef bool                            boolean_t;
        struct null_t {};
        typedef std::vector<value_t>            array_t;
        typedef std::map<string_t, value_t>     object_t;
        typedef std::stack<value_t>             stack_t;    
        
    public:    
        typedef value_t                         result_type;
        
    public:    
        semantic_actions_test() 
        :   array_count(0), object_count(0), string_count(0), boolean_count(0),
        null_count (0), number_count(0)
        {
        }
        
        semantic_actions_test& operator= (const semantic_actions_test& other) {
            result_ = other.result_;
        }
        
        void parse_begin_imp()                          {
            error_.first = json::JP_NO_ERROR;
            error_.second = "";
        }
        void parse_end_imp() { 
            if (stack_.size() != 1) 
                throw std::logic_error("json::internal::SemanticActionsTest: logic error");
            result_ = stack_.top();
            stack_.pop();
        }
        
        void finished_imp() {}
        
        void push_key_imp(const char_t* s, std::size_t len) { 
            ++string_count; stack_.push(value_t(new boost::any(string_t(s, len)))); 
        }        
        
        void push_string_imp(const char_t* s, std::size_t len) { 
            ++string_count; stack_.push(value_t(new boost::any(string_t(s, len)))); 
        }
        
        void push_number_imp(const nb_number_t& number) { 
            ++number_count; stack_.push(value_t(new boost::any(number_t(number.string_)))); 
        }
        
        void push_boolean_imp(bool b) { 
            ++boolean_count; stack_.push(value_t(new boost::any(boolean_t(b)))); 
        }
        
        void push_null_imp() { 
            ++null_count; stack_.push(value_t(new boost::any(null_t()))); 
        }
        
        void begin_array_imp() { 
            ++array_count; stack_.push(value_t(new boost::any(array_t()))); 
        }
        
        void end_array_imp() {}
        
        void begin_object_imp() { 
            ++object_count; stack_.push(value_t (new boost::any(object_t()))); 
        }
        
        bool end_object_imp() { 
            return true; 
        }
        
        void begin_value_at_index_imp(size_t index) {}
        void end_value_at_index_imp(size_t index) {}
        
        void begin_value_with_key_imp(const char_t* s, size_t len, size_t nth) {}
        void end_value_with_key_imp(const char_t* s, size_t len, size_t nth) {}
        
        
        void pop_imp() { 
            stack_.pop(); 
        }
        
        bool pop_pair_insert_into_object() {
            value_t v = stack_.top();
            pop_imp();
            value_t v2 = stack_.top();
            string_t& key = boost::any_cast<string_t&>(*v2);
            pop_imp();
            value_t& v3 = stack_.top();
            object_t& o = boost::any_cast<object_t&>(*v3);
            typedef typename object_t::iterator iter_t;
            std::pair<iter_t, bool> result = o.insert(std::pair<string_t, value_t>(key, v));
            
            return result.second;
        }
        
        void pop_value_push_back_into_array() {
            value_t v = stack_.top();
            pop_imp();
            value_t& v2 = stack_.top();
            array_t& a = boost::any_cast<array_t&>(*v2);
            a.push_back(v);
        }
        
        
        void print_imp(std::ostream& os) { 
            os << *this; 
        }
        
        
        void clear_imp() { 
            array_count = object_count = string_count = boolean_count = null_count = number_count = 0;
            stack_ = stack_t(); 
            result_ = result_type(); 
        }
        
        void error_imp(int code, const char* description) {
            error_.set(code, description);
        }
        
        const error_t& error_imp() const {
            return error_;
        }
        
        
        result_type&        result()          { return result_; }
        const result_type&  result() const    { return result_; }
        
        
    protected:
        stack_t         stack_;
        result_type     result_;
        error_t         error_;
        
        int array_count;
        int object_count;
        int string_count;
        int boolean_count;
        int null_count;
        int number_count;
        
        
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const semantic_actions_test& sa) 
        {
            os << "SemanticActionsTest number of parsed items:\n"
            << "   array_count:   " << sa.array_count << '\n'
            << "   object_count:  " << sa.object_count << '\n'
            << "   string_count:  " << sa.string_count << '\n'
            << "   boolean_count: " << sa.boolean_count << '\n'
            << "   null_count:    " << sa.null_count << '\n'        
            << "   number_count:  " << sa.number_count 
            << std::endl;
            return os;
        }
        
    };
    
    
    
    
}} // namespace json::internal






#endif // JSON_INTERNAL_SEMANTIC_ACTIONS_TEST_HPP

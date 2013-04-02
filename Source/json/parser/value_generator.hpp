//
//  value_generator.hpp
//
//
//  Created by Andreas Grosam on 5/22/11.
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

#ifndef JSON_INTERNAL_SEMANTIC_ACTIONS_HPP
#define JSON_INTERNAL_SEMANTIC_ACTIONS_HPP 


#include "json/config.hpp"
#include "semantic_actions_base.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/value/value.hpp"
#include "json/utility/arena_allocator.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <utility>
#include <iomanip>





#if defined (JSON_INTERNAL_SEMANTIC_ACTIONS_TESTING)
// for performance counters:
#include "utilities/timer.hpp"
#else
namespace  {
    struct timer {
        void start() {}
        void stop() {}
        void reset() {}
        void pause() {}
        double seconds() const {return 0;}
    };
}
#endif



    

/**
    Example for user defined semantic actions class:

    Requires:
    Copy Constructible, Assignable
 
    template <typename EncodingT>
    class MySemanticActions : public json::semantic_actions_base<MySemanticActions, EncodingT>
    {
    public:
        typedef json::internal::semantic_actions<MySemanticActions, EncodingT> base;
        typedef typename base::char_t char_t;
        typdef typename base::nb_number_t nb_number_t;

        void  parse_begin_imp();
        void  parse_end_imp();
        void  finished_imp();
        void  push_key_imp(const const_buffer_t& buffer);
        void  push_string_imp(const const_buffer_t& buffer);
        void  push_number_imp(const nb_number_t& number);
        void  push_null_imp();
        void  push_boolean_imp(bool b);
        void  begin_array_imp();
        void  end_array_imp();
        void  begin_object_imp();
        bool  end_object_imp();
        void  pop_value_push_back_into_array_imp();
        void  pop_imp();
        void  clear_imp(bool);
        void  error_imp(const error_t&);


        const error_t& error() const;


    };
*/






#pragma mark -
#pragma mark SemanticActions


namespace json {
       
#if defined (JSON_INTERNAL_SEMANTIC_ACTIONS_TESTING)
    using utilities::timer;
#endif
    
    
    
    //
    //  class value_generator
    //
    //
    //  Uses json_value_t as the underlaying JSON representation. 
    //  
    // Template parameter EncodingT shall be derived from json::utf_encoding_tag.
    // EncodingT shall match the StringBufferEncoding of the parser.
    //
    
    
    
    template <
        typename EncodingT = json::unicode::UTF_8_encoding_tag,
        typename AllocatorT = std::allocator<void>,
        template <typename, typename > class... ValueImpPolicies
    >
    class value_generator : 
        public semantic_actions_base<value_generator<EncodingT,AllocatorT,ValueImpPolicies...>, EncodingT>
    {
        typedef semantic_actions_base<value_generator<EncodingT,AllocatorT,ValueImpPolicies...>, EncodingT> base;
        typedef json::value<AllocatorT, ValueImpPolicies...> json_value_t;
        typedef typename json_value_t::string_type::value_type _CharT;
        
        static_assert(std::is_same<typename json::unicode::encoding_traits<EncodingT>::code_unit_type, _CharT>::value,
                      "code_unit type from EncodingT does not match the char_type of the data string");
        
    public:    
        typedef typename base::error_t                  error_t;
        typedef typename base::number_desc_t            number_desc_t;
        
        typedef json_value_t                            Value;
        typedef typename Value::array_type              Array;
        typedef typename Value::object_type             Object;
        typedef typename Value::string_type             String;
        typedef typename Value::integral_number_type    IntNumber;
        typedef typename Value::float_number_type       FloatNumber;
        typedef typename Value::boolean_type            Boolean;
        typedef typename Value::null_type               Null;
        
        typedef typename Value::key_type                KeyString;
        
        // Required by the implementation:
        static_assert(std::is_convertible<String, KeyString>::value,
                      "Value::String must be convertible to Value::KeyString");
        
        typedef typename base::char_t                   char_t;     // char type of the StringBuffer
        typedef typename base::encoding_t               encoding_t;
        typedef Value                                   result_type;
        typedef typename base::buffer_t                 buffer_t;
        typedef typename base::const_buffer_t           const_buffer_t;

    private:                
        typedef std::vector<Value>      stack_t;
        typedef std::vector<size_t>     markers_t;
        typedef std::vector<char_t>     string_temp_buffer_t;

    public:   
        
        value_generator(const AllocatorT& a = AllocatorT())
        :   allocator_(a),
            array_count(0), object_count(0), string_count(0), key_string_count(0),
            boolean_count(0), null_count (0), number_count(0),
            max_stack_size(0)
        {
            ++s_count_instances_;
            assert(stack_.size() == 0);
            assert(markers_.size() == 0);
        }
        
        ~value_generator() {
            --s_count_instances_;
        }
        
        
        void parse_begin_imp() {
            //error_.first = 0;
            error_.reset();
            stack_.reserve(200);
            markers_.reserve(20);
            assert(stack_.size() == 0);
            assert(markers_.size() == 0);            
            //t_.start();
        }
        
        void parse_end_imp()                            
        {
            if (stack_.size() != 1) 
                throw std::logic_error("json::SemanticActions: logic error");
            result_ = std::move(stack_.back());
            stack_.pop_back();
            //t_.stop();
        }
        
        void finished_imp() {}
        
        void begin_array_imp() 
        {
            //t0_.start();
            //++c0_;
            ++array_count;
            stack_.emplace_back(Array(allocator_));
            assert(stack_.back().is_array());
            markers_.push_back(stack_.size() -1);  // marker's top value is the index of the array on the stack
            //t0_.pause();
        }
        
        void end_array_imp()
        {
            typedef typename stack_t::iterator stack_iter;
            //t1_.start();
            max_stack_size = std::max(max_stack_size, stack_.size());
            
            size_t first_idx = markers_.back();     // index of the array on the stack
            markers_.pop_back();
            stack_iter first = stack_.begin(); 
            std::advance(first, first_idx);         // now, first points to the array
            stack_iter array_iter = first;
            std::advance(first, 1);                 // now, first points to the first element belonging to the array
            stack_iter first_saved = first;
            stack_iter last = stack_.end();
            Value& arrayVal = *array_iter;
            assert(arrayVal.is_array());
            Array& a = arrayVal.template interpret_as<Array>();
            assert(a.size() == 0);                  // we expect an empty array
            size_t count = std::distance(first, last); // number of elements on the stack which belong to this array
            a.reserve(count);
            a.insert(a.end(), std::make_move_iterator(first), std::make_move_iterator(last));
            stack_.erase(first_saved, last);
            //t1_.pause();
        }
        
        void begin_object_imp() 
        {
            //t0_.start();
            //++c0_;
            ++object_count;
            stack_.emplace_back(Object(allocator_));
            assert(stack_.back().is_object());
            markers_.push_back(stack_.size() - 1); // marker's top value equals the index of the object on the stack
            //t0_.pause();
        }
        
        bool end_object_imp() 
        {            
            // The top of the stack looks as follows:
            // [end]
            // [top]        Value       value[n-1]
            // [top - 1]    String      key[n-1] 
            // [top - 2]    Value       value[n-2] 
            // [top - 3]    String      key[n-2] 
            // ...
            // ...            
            // [top - 2n-2] Value       value[0] 
            // [top - 2n-1] String      key[0] 
            // [top - 2n]   Object      The object where pair(key, value) will be inserted
            //
            // where n equals the number of elements which shall be inserted into object.
            // The marker points to the key[0];
                        
            //t2_.start();
            max_stack_size = std::max(max_stack_size, stack_.size());
            
            typedef typename stack_t::iterator stack_iter;
            
            size_t first_idx = markers_.back();     // index of the object on the stack
            markers_.pop_back();
            stack_iter first = stack_.begin();
            std::advance(first, first_idx);         // first points to the object
            stack_iter object_iter = first;
            std::advance(first, 1);                 // first points to the first element (key) belonging to the object
            stack_iter first_saved = first;
            stack_iter last = stack_.end();
            
            // The number of elements equals (last - fist) / 2:
            // (last - first) shall be an even integer!
            Value& objectVal = *object_iter;
            assert(objectVal.is_object());
            Object& o = objectVal.template interpret_as<Object>();

            bool duplicateKeyError = false;
            while (first != last and not duplicateKeyError) 
            {
                typedef typename Object::iterator obj_iter;
                
                // get the key from the stack:
                Value& keyVal = *first;
                assert(keyVal.is_string());
                String& keyString = keyVal.template interpret_as<String>();                
                ++first;
                // emplace the key-value pair. Note: String must be convertible to KeyString!
                std::pair<obj_iter, bool> result = o.emplace(std::move(keyString), std::move(*first));
                if (not result.second)
                {
                    duplicateKeyError = true;
#if defined (DEBUG)
                    std::cerr << "ERROR: json::SemanticAction: duplicate key error with key '"<< (*(result.first)).first << "'.\n";
                    std::cerr << "Current key-value pairs in object:\n";
                    for (auto kv : o) {
                        std::cerr << '\''<< kv.first << '\'' << ": [" << kv.second.type_name() << "]" <<'\n';
                    }
#endif                    
                }
                ++first;
            }
            
            // Regardless of duplicateKeyError erase the stack from the objects which have to be inserted into the object:
            stack_.erase(first_saved, last);
            
            //t2_.pause();
            return not duplicateKeyError;
        }
        
        void begin_value_at_index_imp(size_t index) {}
        void end_value_at_index_imp(size_t index) {}
        
        void begin_key_value_pair_imp(const const_buffer_t& buffer, size_t nth) 
        {
            ++key_string_count;
            stack_.emplace_back(Value::emplace_string, buffer.first, buffer.second, allocator_);
            assert(stack_.back().is_string());
        }
        void end_key_value_pair_imp() {}
                
        void value_string_imp(const const_buffer_t& buffer, bool hasMore) 
        { 
            //t0_.start();
            if (!hasMore) {
                if (tmp_buffer_.size() == 0) {
                    stack_.emplace_back(Value::emplace_string, buffer.first, buffer.second, allocator_);
                }
                else {
                    // append last chunk to the tmp buffer:
                    tmp_buffer_.insert(tmp_buffer_.end(), buffer.first, buffer.first+buffer.second);
                    // copy from tmp buffer to the stack:
                    stack_.emplace_back(Value::emplace_string, buffer.first, tmp_buffer_.size(), allocator_);
                    tmp_buffer_.clear();
                }
                assert(stack_.back().is_string());
                ++string_count;
            } else {
                // append chunk to the tmp buffer:
                tmp_buffer_.insert(tmp_buffer_.end(), buffer.first, buffer.first+buffer.second);
            }
            //t0_.pause();
        }        
        
        void value_number_imp(const number_desc_t& number) 
        {
            //t0_.start();
            ++number_count;
            if (number.is_integer()) {
                stack_.emplace_back(Value::emplace_integral_number, number.c_str(), number.c_str_len());
                assert(stack_.back().is_integral_number());
            }
            else {
                stack_.emplace_back(Value::emplace_float_number, number.c_str(), number.c_str_len());
                assert(stack_.back().is_float_number());
            }
            //t0_.pause();
        }
        
        void value_boolean_imp(bool b) 
        {
            //t0_.start();
            ++boolean_count;
            stack_.emplace_back(Value::emplace_boolean, b);
            assert(stack_.back().is_boolean());
            //t0_.pause();
        }
        
        void value_null_imp() 
        {
            //t0_.start();
            ++null_count;
            stack_.emplace_back(Value::emplace_null);
            assert(stack_.back().is_null());
            //t0_.pause();
        }
        
        
        void print_imp(std::ostream& os) {
            os << *this;
        }
                
        void clear_imp(bool shrink_buffers)
        {            
            std::size_t stack_size = stack_.size();
            if (stack_size > 0) {
                stack_.clear();
            }
            result_ = result_type();
            error_.reset();
            markers_.clear();
            tmp_buffer_.clear();
            // TODO: allocator_.clear();
            
            if (shrink_buffers) {
                tmp_buffer_.shrink_to_fit();
            }
            
//            t_.reset();
//            t0_.reset();
//            t1_.reset();
//            t2_.reset();
//            t3_.reset();
            array_count = object_count = string_count = key_string_count =
            boolean_count = null_count = number_count = max_stack_size = 0;            
        } 
        
        void error_imp(int code, const char* description) {
            error_.set(code, description);
        }
        
        const error_t& error_imp() const {
            return error_;
        }
        
        result_type&        result()            { return result_; }
        const result_type&  result() const      { return result_; }
                
//        double              t() const           { return t_.seconds(); }
//        double              t0() const          { return t0_.seconds(); }
//        double              t1() const          { return t1_.seconds(); }
//        double              t2() const          { return t2_.seconds(); }
//        double              t3() const          { return t3_.seconds(); }
        
        
        
        
    protected:
        static size_t   s_count_instances_;
        
    protected:
        stack_t                 stack_;
        result_type             result_;
        markers_t               markers_;
        string_temp_buffer_t    tmp_buffer_;
        error_t                 error_;
        AllocatorT              allocator_;

//        timer           t_;
//        timer           t0_;
//        timer           t1_;
//        timer           t2_;
//        timer           t3_;
//        size_t          c0_;
//        size_t          c1_;
//        size_t          c2_;
        
        size_t array_count;
        size_t object_count;
        size_t string_count;
        size_t key_string_count;
        size_t boolean_count;
        size_t null_count;
        size_t number_count;
        size_t max_stack_size;
        
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const value_generator& sa) 
        {
            typedef value_generator::base base;
            
            os << static_cast<base const&>(sa);            
            
            std::cout << "Semantic Actions number of parsed items:\n"
            << "   array_count:   " << sa.array_count << '\n'
            << "   object_count:  " << sa.object_count << '\n'
            << "   string_count:  " << sa.string_count << '\n'
            << "   key_string_count:  " << sa.key_string_count << '\n'
            << "   boolean_count: " << sa.boolean_count << '\n'
            << "   null_count:    " << sa.null_count << '\n'
            << "   number_count:  " << sa.number_count << '\n'
            << "   max stack items:  " << sa.max_stack_size
            << std::endl;
            
//            std::cout << "Performance counters: \n" 
//                << "push string: " << std::fixed << std::setprecision(3) << sa.t0() * 1.0e6 << " µs\n"
//                << "build array: " << std::fixed << std::setprecision(3) << sa.t1() * 1.0e6 << " µs\n"
//                << "build object: " << std::fixed << std::setprecision(3) << sa.t2() * 1.0e6 << " µs\n"; 
            
            return os;
        }
        
        
    };
    
    

    
    template <
        typename E,
        typename A,
        template <typename, typename > class... P
    >
    size_t value_generator<E, A, P...>::s_count_instances_ = 0;
    
} // namespace json



#endif // JSON_INTERNAL_SEMANTIC_ACTIONS_HPP

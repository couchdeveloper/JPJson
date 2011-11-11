//
//  semantic_actions.hpp
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


#include <boost/config.hpp>
#include <boost/tr1/unordered_map.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <utility>
#include <iomanip>
#include "semantic_actions_base.hpp"
#include "json/utility/string_hasher.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/value/value.hpp"


// JSON_SEMANTIC_ACTIONS_STRING_CACHE_USE_BOOST_POOL_ALLOCATOR
// If defind, uses a fast pool allocator (json::fast_pool_allocator) for 
// boost::unordered_map container which is used in the implementation of
// string cache.
// Due to bugs in boost::fast_pool_allocator and other undesired effects, it is 
// not recommended to use this allocator.
// 
//#define JSON_SEMANTIC_ACTIONS_STRING_CACHE_USE_BOOST_POOL_ALLOCATOR


// JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP
// If defined, the json::value class uses a boost unordered_map for its
// underlaying implementation for a json object. Otherwise it uses a std::map.
//#define JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP

// JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_UNORDERED_MAP_USE_BOOST_FAST_POOL_ALLOCATOR 
// If defined and if JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP is 
// defined then json::value class uses a boost::unordered_map with a 
// fast_pool_allocator for its underlaying implementation.
// Otherwise it uses an alternative allocator (std::allocator).
// If macro JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP is not defined
// this macro has no effect.
//
// Due to bugs in boost::fast_pool_allocator and other undesired effects, it is 
// not recommended to use this allocator.
// 
//#define JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_UNORDERED_MAP_USE_BOOST_FAST_POOL_ALLOCATOR



// JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_MAP_USE_BOOST_FAST_POOL_ALLOCATOR 
// If defined and if JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP is 
// *not* defined then json::value class uses a std::map with a fast_pool_allocator 
// for its underlaying implementation.
// Otherwise it uses an alternative allocator.
// If macro JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP is defined
// this macro has no effect.
//
// Due to bugs in boost::fast_pool_allocator and other undesired effects, it is 
// not recommended to use this allocator.
// 
//#define JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_MAP_USE_BOOST_FAST_POOL_ALLOCATOR


#if defined (JSON_SEMANTIC_ACTIONS_STRING_CACHE_USE_BOOST_POOL_ALLOCATOR)                        \
            || defined (JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_UNORDERED_MAP_USE_BOOST_FAST_POOL_ALLOCATOR) \
            || defined (JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_MAP_USE_BOOST_FAST_POOL_ALLOCATOR)
    
 #include "fast_pool_alloc.hpp"
 #include <boost/pool/pool_alloc.hpp>

#endif

#if !defined (JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_MAP_USE_BOOST_FAST_POOL_ALLOCATOR)        
 // when using custom allocators:
 #include "Allocators/FSBAllocator.hh"
#endif


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



    

//
//    Example for user defined semantic actions class:
//
//    Requires:
//    Copy Constructable, Assignable
// 
//    template <typename EncodingT>
//    class MySemanticActions : public json::semantic_actions::semantic_actions_base<MySemanticActions, EncodingT>
//    {
//    public:
//        typedef json::internal::semantic_actions<MySemanticActions, EncodingT> base;
//        typedef typename base::char_t char_t;
//        typdef typename base::nb_number_t nb_number_t;
//
//        void  parse_begin_imp();
//        void  parse_end_imp();
//        void  finished_imp();
//        void  push_key_imp(char_t* s, size_t len);
//        void  push_string_imp(char_t* s, size_t len);
//        void  push_number_imp(const nb_number_t& number);
//        void  push_null_imp();
//        void  push_boolean_imp(bool b);
//        void  begin_array_imp();
//        void  end_array_imp();
//        void  begin_object_imp();
//        bool  end_object_imp();
//        void  pop_value_push_back_into_array_imp();
//        void  pop_imp();
//        void  clear_imp();
//        void  error_imp(const error_t&);
//
//
//        const error_t& error() const;


//    };
//







#pragma mark -
#pragma mark SemanticActions


namespace json {
    
    // The following ValuePolicies may use a std::basic_string for the Value's
    // internal string implementation. Doing so, we need to define a traits
    // class which determines that std::basic_string is a "json type" as
    // follows:

    template <typename CharT, typename TraitsT, typename AllocT> 
    struct is_json_type<std::basic_string<CharT, TraitsT, AllocT> > : public boost::mpl::true_
    { 
        static const bool value = true; 
    };

}

namespace json { namespace policies {
    
    //
    //  class ValuePolicies
    // 
    // Value policies define the underlaying implementations of the array and
    // object container used in the json container (template class json::value).
    // This class replaces the default policy json::value_policies.
    //
    class ValuePolicies 
    {
    public:
        // Typedef for the implementation container type for json::array:
        typedef std::vector<boost::mpl::_>                          array_imp_tt;

        // Typedef for the implementation container type for json::object:

#if !defined (JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP)        
        typedef std::map<
              boost::mpl::_                         /* Key */
            , boost::mpl::_                         /* T */
            , std::less<boost::mpl::_1>             /* Compare */
#if defined (JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_MAP_USE_BOOST_FAST_POOL_ALLOCATOR)        
            , json::fast_pool_allocator< std::pair<const boost::mpl::_1, boost::mpl::_2> >  /* Allocator */
#else        
           , FSBAllocator2< std::pair<const boost::mpl::_1, boost::mpl::_2> >  /* Allocator */
#endif        
        >  object_imp_tt;
#else        
        
//          typedef boost::unordered_map<boost::mpl::_, boost::mpl::_>  object_imp_tt;      
            typedef boost::unordered_map<
                  boost::mpl::_1
                , boost::mpl::_2
                , boost::hash<boost::mpl::_1>
                , std::equal_to<boost::mpl::_1>
                //, json::fast_pool_allocator<std::pair<boost::mpl::_1, boost::mpl::_2> > 
                //-> won't compile:, FSBAllocator2< std::pair<boost::mpl::_1, boost::mpl::_2> >  /* Allocator */
            > object_imp_tt;        
#endif        
        
        typedef json::string<boost::mpl::_>                    string_imp_tt;        
        //typedef std::basic_string<boost::mpl::_, std::char_traits<boost::mpl::_1> , boost::pool_allocator<boost::mpl::_1> > string_imp_tt;
        //typedef std::basic_string<boost::mpl::_>  string_imp_tt;        
    };


}}    // namespace json::policies



        
namespace json { 
       
#if defined (JSON_INTERNAL_SEMANTIC_ACTIONS_TESTING)
    using utilities::timer;
#endif
    
    //
    //  class semantic_actions
    //
    //
    //  Uses json::Value as the underlaying container. The Value uses the 
    //  previously defined value polcies "ValuePlocies" in namespace 
    //  json::policies.
    //  
    
    // Template parameter EncodingT shall be derived from json::utf_encoding_tag.
    // EncodingT shall match the StringBufferEncoding of the parser.
    //
    // This semantic-actions class provides a string cache, which uses a map as
    // its underlaying container, and a hash of the string for the key. The cache
    // can significantly improve performance if the underlaying string type of the
    // json::value will share string contents. This is usually true for a std::string 
    // in a pre-C++0x library. In newer C++0x libraries, std::string will usually 
    // not share its string content with other string instances, but will always
    // copy the content. The actual implementation of the json::string type can
    // be specified in the policies class for json::value. 
    // 
    // This implementation makes sense only if std::string share its string
    // content, and if the underlaying string type in json::string is a 
    // std::string also.
    //
    // Other implementations using a similar technique but other string types
    // are possible - but require to change the code accordingly. A generic
    // solution seems difficult.
    //
    // TODO: 
    // Building arrays: push all items onto the stack belonging to an array. Then count 
    // them, set the capacity and insert them into the array.
    //    
    
    
    template <typename EncodingT = json::unicode::UTF_8_encoding_tag>
    class semantic_actions : 
        public semantic_actions_base<semantic_actions<EncodingT>, EncodingT>
    {
    private:     
        typedef semantic_actions_base<semantic_actions, EncodingT> base;
        
    public:    
        typedef typename base::error_t              error_t;
        typedef typename base::nb_number_t          nb_number_t;
        
        typedef value<policies::ValuePolicies>      Value;
        typedef Value::array_t                      Array;
        typedef Value::object_t                     Object; 
        typedef Value::string_t                     String;
        typedef json::Number                        Number;        
        typedef json::Boolean                       Boolean;
        typedef json::Null                          Null;
        
        typedef typename base::char_t               char_t;     // char type of the StringBuffer
        typedef typename base::encoding_t           encoding_t;
        typedef Value                               result_type;
        
    private:
        
        // The struct SemanticActions_system_allocator_new_delete is used only 
        // to unify the type of boost pool allocator. boost::pool_allocator uses
        // a singleton to implement a pool which actual singleton instance is 
        // determined by the template parameters. In order to associate a certain 
        // singleton pool instance to all instances of SemanticActions, we use 
        // a private type for the system allocator which is just a wrapper for
        // the existing default system allocator.
        struct SemanticActions_system_allocator_new_delete {
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;
            
            static char * malloc (const size_type bytes)
            { 
                char * p = new (std::nothrow) char[bytes];
                //printf("System Allocator: allocated block %p with size %lu\n", p, bytes);
                return p; 
            }
            static void free (char * const block)
            { 
                delete [] block; 
                //printf("System Allocator: freed block %p\n", block);
            }
        };
        
        //typedef std::list<Value, FSBAllocator2<Value> >  stack_t;
        //typedef std::list<Value, json::fast_pool_allocator<Value> >  stack_t;
        typedef std::vector<Value>                  stack_t;
        typedef String                              map_mapped_value_t;
        typedef std::size_t                         map_key_t;        
        

        //typedef std::allocator<std::pair<const map_key_t, map_mapped_value_t> > map_allocator_type;
        //typedef FSBAllocator<std::pair<const map_key_t, map_mapped_value_t> > map_allocator_type;
        //typedef FSBAllocator2<std::pair<const map_key_t, map_mapped_value_t> > map_allocator_type;
#if defined (JSON_SEMANTIC_ACTIONS_STRING_CACHE_USE_BOOST_POOL_ALLOCATOR)
        typedef json::fast_pool_allocator<
            std::pair<const map_key_t, map_mapped_value_t>,
            SemanticActions_system_allocator_new_delete
        >                                           map_allocator_type;
#else
        typedef std::allocator<std::pair<
            const map_key_t, map_mapped_value_t> 
        >                                           map_allocator_type;
#endif        
        //typedef boost::pool_allocator<std::pair<const map_key_t, map_mapped_value_t>  map_allocator_type;
        
//        typedef std::map<map_key_t, map_mapped_value_t, std::less<map_key_t>,
//            map_allocator_type>        map_t;

        typedef boost::unordered_map<
            map_key_t, 
            map_mapped_value_t, 
            boost::hash<map_key_t>, 
            std::equal_to<map_key_t>,  
            map_allocator_type>                     map_t;
        
        
#if defined (JSON_SEMANTIC_ACTIONS_STRING_CACHE_USE_BOOST_POOL_ALLOCATOR)
        // The following five typedefs are necessary to extract the size of
        // the allocator's value_type which allocates the elements in the 
        // boost::unordered_map.
        typedef typename boost::unordered_detail::rebind_wrap<
            map_allocator_type, 
            typename map_t::value_type>::type       map_value_allocator_t;
        typedef boost::unordered_detail::map<
            map_key_t, 
            boost::hash<map_key_t>, 
            std::equal_to<map_key_t>,
            map_value_allocator_t>                  map_types; 
        typedef typename map_types::impl            map_table;
        typedef typename map_table::node            map_table_node_t;        
        typedef boost::singleton_pool<
            boost::fast_pool_allocator_tag, 
            sizeof(map_table_node_t), 
            SemanticActions_system_allocator_new_delete> singleton_pool_type;
#endif
        
        
        typedef typename map_t::value_type          map_elem_t;
        typedef typename map_t::const_iterator      map_c_iter_t;
        
        typedef std::vector<size_t>                 markers_t;
        
        
    public:   
        
        // Frees system memory allocated by instances of SemanticActions.
        // Returns true if the resources were not locked by any instance
        // of SemanticActions.
        static bool cleanup_caches() 
        {
            
            if (s_count_instances_ == 0) 
            {                
                // free system memory allocated by instances of string_cache_:
#if defined (JSON_SEMANTIC_ACTIONS_STRING_CACHE_USE_BOOST_POOL_ALLOCATOR)                
                singleton_pool_type::purge_memory();
#endif                
                return true;
            }
            else {
                return false;
            }
        }
        
        semantic_actions() 
        :   c0_(0), c1_(0), c2_(0),
            array_count(0), object_count(0), string_count(0), 
            boolean_count(0), null_count (0), number_count(0),
            max_stack_size(0)
        {
            ++s_count_instances_;            
        }
        
        ~semantic_actions() {
            string_cache_.clear();
            --s_count_instances_;
        }
        
        
        void parse_begin_imp() {
            //error_.first = 0;
            error_.second = 0;
            stack_.reserve(200);
            markers_.reserve(20);
            string_cache_.rehash(64);
            t_.start();
        }
        
        void parse_end_imp()                            
        {
            if (stack_.size() != 1) 
                throw std::logic_error("json::SemanticActions: logic error");
            result_ = stack_.back().make_default();
            swap(result_, stack_.back());
            stack_.pop_back();
            t_.stop();
        }
        
        void finished_imp() {}
        
        void push_string_cached(const char_t* s, std::size_t len) 
        {
            // (performance critical function)
            const std::size_t hash = json::utility::string_hasher<char_t>()(s, len);
            map_c_iter_t iter = string_cache_.find(hash);
            if (iter == string_cache_.end()) {
                std::pair<map_c_iter_t, bool> result = 
                    string_cache_.insert(map_elem_t(hash, map_mapped_value_t(s, len)));
                iter = result.first;
            }
            stack_.push_back( (*iter).second );
        }        
        
        void push_string_imp(const char_t* s, std::size_t len) 
        {
            //t0_.start();            
            //++c0_;
            //++string_count;
            stack_.push_back(String(s, len));
            //t0_.pause();
        }

        void push_key_imp(const char_t* s, std::size_t len) 
        { 
            //t0_.start();
            //++c0_;
            //++string_count;
            push_string_cached(s, len); 
            //t0_.pause();
        }
        
        void push_number_imp(const nb_number_t& number) {
            //t0_.start();
            //++c0_;
            //++number_count;
            stack_.push_back(Number(number.string_));
            //t0_.pause();
        }
        
        
        void push_boolean_imp(bool b) {
            //t0_.start();
            //++c0_;
            //++boolean_count;
            stack_.push_back(Value(b));
            //t0_.pause();
        }
        
        void push_null_imp() {
            //t0_.start();
            //++c0_;
            //++null_count;
            stack_.push_back(Null());
            //t0_.pause();
        }
        
        void begin_array_imp() {
            //t0_.start();
            //++c0_;
            //++array_count;
            stack_.push_back(Array());
            markers_.push_back(stack_.size() -1);  // marker's top value is the index of the array on the stack
            //t0_.pause();
        }
        
        void end_array_imp() 
        {
            //t1_.start();
            max_stack_size = std::max(max_stack_size, stack_.size());
            
            size_t first_idx = markers_.back();     // index of the array on the stack
            markers_.pop_back();
            stack_t::iterator first = stack_.begin(); 
            std::advance(first, first_idx);         // now, first points to the array
            stack_t::iterator array_iter = first;
            std::advance(first, 1);                 // now, first points to the first element belonging to the array
            stack_t::iterator first_saved = first;
            stack_t::iterator last = stack_.end();
            Array& a = (*array_iter).template as<Array>();
            assert(a.size() == 0);
            size_t count = std::distance(first, last);
            a.reserve(count);
            while (first != last) {
                a.push_back( (*first).make_default() );
                swap(a.back(), *first);
                ++first;
            }
            stack_.erase(first_saved, last);
            //t1_.pause();
        }
        
        void begin_object_imp() {
            //t0_.start();
            //++c0_;
            //++object_count;
            stack_.push_back(Object());
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
            
            typedef typename Object::iterator iter_t;
            typedef typename Object::element elem_t;
            
            //t2_.start();
            max_stack_size = std::max(max_stack_size, stack_.size());
            
            size_t first_idx = markers_.back();     // index of the object on the stack
            markers_.pop_back();
            stack_t::iterator first = stack_.begin(); 
            std::advance(first, first_idx);         // first points to the object
            stack_t::iterator object_iter = first;
            std::advance(first, 1);                 // first points to the first element (key) belonging to the object
            stack_t::iterator first_saved = first;
            stack_t::iterator last = stack_.end();
            
            // The number of elements equals (last - fist) / 2:
            // (last - first) shall be an even integer!
            
            Object& o = (*object_iter).template as<Object>();

#if defined (JSON_SEMANTIC_ACTIONS_VALUE_POLICIES_USE_UNORDERED_MAP)        
            size_t count = std::distance(first, last);
            o.imp().rehash(count);
#endif                        
            bool duplicateKeyError = false;
            while (first != last and not duplicateKeyError) 
            {
                // get the key from the stack:
                String& key = (*first).as<String>();
                ++first;
                // Insert the key-value with a default constructed value and check result:
                std::pair<iter_t, bool> result = o.insert(elem_t(key, Value()));
                if (result.second) {
                    // get a reference of the inserted value and swap it with the one to be inserted:
                    Value& vr = (*(result.first)).second;
                    // get the value from the stack:
                    vr = (*first).make_default();
                    swap(vr, *first);
                } else {
                    duplicateKeyError = true;
#if defined (DEBUG)
                    std::cerr << "ERROR: json::SemanticAction: duplicate key error with key '"<< key.c_str() << "'.\n";
                    Object::const_iterator first, last;
                    first = o.begin();
                    last = o.end();
                    std::cerr << "Current key-value pairs in object:\n";
                    while (first != last) {
                        std::cerr << '\''<< (*first).first.c_str() << '\'' << ": (" << (*first).second.type_name() << ")" <<'\n';
                        ++first;
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
        
        void begin_value_with_key_imp(const char_t* s, size_t len, size_t nth) {}
        void end_value_with_key_imp(const char_t* s, size_t len, size_t nth) {}
                
        
        void pop_imp() {
            //t3_.start();
            stack_.pop_back();
            //t3_.pause();
        }
        
        void print_imp(std::ostream& os) { 
            os << *this; 
        }
                
        void clear_imp() 
        {
            stack_.clear();
            string_cache_.clear();            
            result_ = result_type();
            //error_.first = 0;
            error_.second = 0;
            markers_.clear();
            t_.reset();
            t0_.reset();
            t1_.reset();
            t2_.reset();
            t3_.reset();
            c0_ = c1_ = c2_ = 0;
            array_count = object_count = string_count = 
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
        
        const map_t&        string_cache() const { return string_cache_; }
        
        double              t() const          { return t_.seconds(); }
        double              t0() const          { return t0_.seconds(); }
        double              t1() const          { return t1_.seconds(); }
        double              t2() const          { return t2_.seconds(); }
        double              t3() const          { return t3_.seconds(); }
        
        size_t              c0() const          { return c0_; }
        size_t              c1() const          { return c1_; }
        size_t              c2() const          { return c2_; }
        
        
    protected:
        static size_t   s_count_instances_;
        
    protected:
        
        stack_t         stack_;
        map_t           string_cache_;
        result_type     result_;
        markers_t       markers_;
        error_t         error_;
        
        timer           t_;
        timer           t0_;
        timer           t1_;
        timer           t2_;
        timer           t3_;
        size_t          c0_;
        size_t          c1_;
        size_t          c2_;
        
        size_t array_count;
        size_t object_count;
        size_t string_count;
        size_t boolean_count;
        size_t null_count;
        size_t number_count;
        size_t max_stack_size;
        
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const semantic_actions& sa) 
        {
            typedef semantic_actions::base base;
            
            os << static_cast<base const&>(sa);            
            
            os << "Semantic Actions elapsed time:" 
            << std::fixed << std::setprecision(3) << sa.t() * 1.0e6 << " µs" 
            << std::endl;
            
            std::cout << "Semantic Actions number of parsed items:\n"
            //<< "   array_count:   " << sa.array_count << '\n'
            //<< "   object_count:  " << sa.object_count << '\n'
            //<< "   string_count:  " << sa.string_count << '\n'
            //<< "   boolean_count: " << sa.boolean_count << '\n'
            //<< "   null_count:    " << sa.null_count << '\n'        
            //<< "   number_count:  " << sa.number_count << '\n'
            << "   string-cache size:  " << sa.string_cache_.size() << '\n'
            << "   max stack items:  " << sa.max_stack_size
            << std::endl;
            
            std::cout << "Performance counters: \n" 
                << "push string: " << std::fixed << std::setprecision(3) << sa.t0() * 1.0e6 << " µs\n"
                << "build array: " << std::fixed << std::setprecision(3) << sa.t1() * 1.0e6 << " µs\n"
                << "build object: " << std::fixed << std::setprecision(3) << sa.t2() * 1.0e6 << " µs\n"; 
                                    
            return os;
        }
        
        
    };
    
    
} // namespace json

namespace json { 
    
    // TODO: make it thread safe
    template <typename EncodingT>
    size_t   semantic_actions<EncodingT>::s_count_instances_ = 0;
  
}


#endif // JSON_INTERNAL_SEMANTIC_ACTIONS_HPP

//
//  SemanticActions.hpp
//
//  Created by Andreas Grosam on 6/16/11.
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

#ifndef JSON_OBJC_SEMANTIC_ACTIONS_HPP
#define JSON_OBJC_SEMANTIC_ACTIONS_HPP 

#include "json/config.hpp"


// Workarounds:
// If this macro is defined, Core Foundation mutable dictionaries will be created
// using Foundation objects leveraging "toll-free-bridging".
// This is a workaround for poorly performing CFMutableDictionaries when created 
// via CF functions. CFMutbaleDictionaries created via Foundation do not expose 
// this poor performance. 
// Note: iOS 4 does not seem to have this issue.
#define SEMANTIC_ACTIONS_NO_CREATE_MUTABLE_DICTIONARY_USING_CF


// JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE shall be defined.
#define JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE


// JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH should be in order to enable 
// the JSON Path API and feature.
//#define JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH



#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH) && !defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)   
#error  JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH requires JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE
#endif        


#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>  // description
#include <iomanip>   // description   
#include <limits>
#include <stdlib.h>  // numeric conversion
#include <xlocale.h>

#include "SemanticActionsBase.hpp"
#include "unicode_traits.hpp"

#include "json/unicode/unicode_traits.hpp"
#include "json/utility/simple_log.hpp"
#include "json/utility/flags.hpp"

#include <boost/static_assert.hpp>

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>


#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)
#include "json/json_path/intrusive_json_path.hpp"
#endif




#if defined (JSON_INTERNAL_SEMANTIC_ACTIONS_PERFORMANCE_COUNTER)
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



#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)
#include "CFDataCache.hpp"
#endif



namespace {
    
#pragma mark - Private Functions
    
    
    void throwRuntimeError(const char* msg) {
        throw std::runtime_error(msg);
    }
    
    void throwLogicError(const char* msg) {
        throw std::logic_error(msg);
    }
    void throwRangeError(const char* msg) {
        throw std::logic_error(msg);
    }

    
    NSDecimalNumber*  newDecimalNumberFromString(const char* buffer, std::size_t len) 
    {
        CFStringRef numberString = 
            CFStringCreateWithBytesNoCopy(NULL,
                                          (const UInt8 *)buffer,
                                          len,
                                          kCFStringEncodingUTF8,
                                          false,
                                          NULL);
        NSScanner* scanner = [[NSScanner alloc] initWithString:(id)numberString];
        NSDecimal decimal;
        BOOL result = [scanner scanDecimal:&decimal];
        [scanner release];
        if (not result) {
            throwRuntimeError("could not create NSDecimal");
        }
        NSDecimalNumber* decimalNumber = [[NSDecimalNumber alloc] initWithDecimal:decimal];
        return decimalNumber;
    }
    
    
#pragma mark CFString Creation
    
    // Creates a CFString from a buffer 's' containing 'len' code units encoded 
    // in 'Encoding'. Ownership of the string folows the "Create" rule.
    template <typename Encoding, typename CharT>
    CFStringRef createString(const CharT* s, std::size_t len, Encoding) 
    {
        typedef typename json::unicode::add_endianness<Encoding>::type encoding_type;
        typedef typename json::unicode::encoding_traits<encoding_type>::code_unit_type code_unit_t;
        CFStringEncoding cf_encoding = json::cf_unicode_encoding_traits<encoding_type>::value;
        CFStringRef str = CFStringCreateWithBytes(NULL, 
                                                  static_cast<const UInt8*>(static_cast<const void*>(s)), 
                                                  sizeof(code_unit_t)*len, 
                                                  cf_encoding, 0);
        assert(str != NULL);
        return str;
    }
    
    
    // Creates a CFMutableString from a buffer 's' containing 'len' code units encoded in
    // 'Encoding'. Ownership of the string folows the "Create" rule.
    template <typename Encoding, typename CharT>
    CFMutableStringRef createMutableString(const CharT* s, std::size_t len, Encoding) 
    {
        typedef typename json::unicode::add_endianness<Encoding>::type encoding_type;
        typedef typename json::unicode::encoding_traits<encoding_type>::code_unit_type code_unit_t;
        CFStringEncoding cf_encoding = json::cf_unicode_encoding_traits<encoding_type>::value;
        
        // First, create a temp const string
        CFStringRef tmp = CFStringCreateWithBytesNoCopy(NULL, 
                                                        static_cast<const UInt8*>(static_cast<const void*>(s)), 
                                                        sizeof(code_unit_t)*len, 
                                                        cf_encoding, 
                                                        false, kCFAllocatorNull);
        assert(tmp != NULL);
        // Create a string with unlimited capazity:
        CFMutableStringRef str = CFStringCreateMutable(kCFAllocatorDefault, 0);
        assert(str != NULL);
        CFStringAppend(str, tmp);
        CFRelease(tmp);
        return str;
    }
    
    
    // Appends a buffer of code units with length 'len' and encding 'Encoding' to 
    // the mutable string 'str'.
    template <typename Encoding, typename CharT>
    void appendString(CFMutableStringRef str, const CharT* s, std::size_t len, Encoding) 
    {
        typedef typename json::unicode::add_endianness<Encoding>::type encoding_type;
        typedef typename json::unicode::encoding_traits<encoding_type>::code_unit_type code_unit_t;
        CFStringEncoding cf_encoding = json::cf_unicode_encoding_traits<encoding_type>::value;
        
        // First, create a temp const string
        CFStringRef tmp = CFStringCreateWithBytesNoCopy(NULL, 
                                                        static_cast<const UInt8*>(static_cast<const void*>(s)), 
                                                        sizeof(code_unit_t)*len, 
                                                        cf_encoding, 
                                                        false, kCFAllocatorNull);
        assert(tmp != NULL);
        // Append to the mutable string:
        CFStringAppend(str, tmp);
        CFRelease(tmp);
    }
    
    
    
    
} // unnamed namespace 






namespace json { namespace objc { namespace sa_options {

    //
    // Semantic Actions Options
    // 
    enum number_generator_option {
        NumberGeneratorGenerateAuto,
        NumberGeneratorGenerateString,
        NumberGeneratorGenerateDecimalNumber
    };
    
}}}




namespace json { namespace objc { 
        
    using json::numberbuilder::normalized_number_t;
    //using json::semanticactions::json_path;
    
    
    using json::unicode::UTF_8_encoding_traits;
    using json::unicode::encoding_traits;
    
#if defined (JSON_INTERNAL_SEMANTIC_ACTIONS_PERFORMANCE_COUNTER)
    using utilities::timer;
#endif
    
    
    
    
#pragma mark - Class SemanticActions
    //
    //  Class SemanticActions
    //
    //  The SemanticActions instance is responsible for creating the correspon-
    //  ding JSON representation as Foundation objects. 
    //
    //  An instance of this class is used as the implementation object for
    //  the Objective-C class JPSemanticActions. It ties together the JSON
    //  parser implemented in pure C++ with its semantic actions class, which
    //  is implemnted in Objective-C. It would be possible, that most of the
    //  features implemented here may be implemented within the corresponding 
    //  Objective-C class, JPSemanticActions. 
    // 
    //  Uses NSArray and NSDictionary as the underlaying containers. 
    //
    //  Only the *required* delegate methods as defined in the 
    //  JPSemanticActionsProtocol will be forwarded to the corresponding delegate.    
    //  
    //
    //
    // Template parameter EncodingT shall be derived from json::utf_encoding_tag.
    // EncodingT shall match the StringBufferEncoding of the parser. Usullay, 
    // this is a UTF-16 encoding.
    //
    // This semantic-actions class provides a string cache which stores NSString
    // objects. It uses a fast associative container and a hash to access them. 
    //
    // The following options are available:
    // 
    //  - CheckForDuplicateKey:     Checks whether a key for a JSON object  
    //                              already exists. If it exists, an "duplicate 
    //                              key error" will be logged to error console 
    //                              and error state will be set.
    //                              Default: false
    //
    //  - KeepStringCacheOnClear:   Doesn't clear the string cache if the semantic
    //                              actions is cleared via function clear().
    //                              Default: false
    //
    //  - CacheDataStrings:         Caches data strings in addition to key strings.
    //                              Default: false
    //
    //  - parseMultipleDocuments    If this option is set, the parser parses more 
    //                              than one document from the input until it receives
    //                              the EOF of the input data. The semantics actions
    //                              class stores each finished document in a list.
    //                              
    //  Number Parsing Options:
    //
    //  - NumberGeneratorGenerateDecimalNumber
    //  - NumberGeneratorGenerateAuto
    //  - NumberGeneratorGenerateString
    //
    // 
    //  Log Levels:
    //
    //  - LogLevelDebug
    //  - LogLevelWarning
    //  - LogLevelError
    //  - LogLevelNone
    
    // 
    //  Implementation notes:
    //  Objective-C objects hold in STL containers are always retained.
    
    template <typename EncodingT>
    class SemanticActions : public SemanticActionsBase<EncodingT>
    {
    public:     
        typedef SemanticActionsBase<EncodingT>      base;
        
        
#pragma mark -  struct performance_counter
        struct performance_counter {
            performance_counter() :
                c0_(0), c1_(0), c2_(0),
                array_count_(0), object_count_(0), string_count_(0), 
                boolean_count_(0), null_count_ (0), number_count_(0),
                max_stack_size_(0)
            {}
            
            void clear() {
                t_.reset();
                t0_.reset();
                t1_.reset();
                t2_.reset();
                t3_.reset();
                c0_ = c1_ = c2_ = 0;
                array_count_ = object_count_ = string_count_ = 
                boolean_count_ = null_count_ = number_count_ = max_stack_size_ = 0;            
            }
            
            double   t() const           { return t_.seconds(); }
            double   t0() const          { return t0_.seconds(); }
            double   t1() const          { return t1_.seconds(); }
            double   t2() const          { return t2_.seconds(); }
            double   t3() const          { return t3_.seconds(); }
            
            size_t   c0() const          { return c0_; }
            size_t   c1() const          { return c1_; }
            size_t   c2() const          { return c2_; }
            
            size_t   max_stack_size() const { return max_stack_size_; }

        public:            
            timer           t_;
            timer           t0_;
            timer           t1_;
            timer           t2_;
            timer           t3_;
            size_t          c0_;
            size_t          c1_;
            size_t          c2_;
            
            size_t          array_count_;
            size_t          object_count_;
            size_t          string_count_;
            size_t          boolean_count_;
            size_t          null_count_;
            size_t          number_count_;
            size_t          max_stack_size_;
        };
        
    public:    
        typedef typename base::error_t              error_t;
        typedef typename base::nb_number_t          nb_number_t;
        typedef typename base::char_t               char_t;     // char type of the StringBuffer
        typedef typename base::encoding_t           encoding_t;
        typedef typename base::result_type          result_type;
        
        typedef typename base::buffer_t             buffer_t;
        typedef typename base::const_buffer_t       const_buffer_t;
        
        typedef typename UTF_8_encoding_traits::code_unit_type utf8_code_unit;
        
    private:        
        typedef std::vector<size_t>                 markers_t;
        typedef std::vector<id>                     stack_t;
        
        // temporary vector used when creating immutable CFDictionaries
        typedef CFTypeRef                           tmp_array_obj_t;
        typedef std::vector<tmp_array_obj_t>        tmp_array_t;   
        
        //typedef json_path<char_t>                   json_path_t;
        

#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)   
        typedef CFDataCache<char_t>                 cache_t;
        typedef typename cache_t::key_type          cache_key_t;
        typedef typename cache_t::value_type        cache_value_t;
        typedef typename cache_t::mapped_type       cache_mapped_t;
        typedef typename cache_t::iterator          cache_iterator;
        typedef typename cache_t::const_iterator    cache_const_iterator;
#endif
        
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)
        typedef  objc_internal::intrusive_json_path<encoding_t> json_path_t;
#endif
        
        
        // nb_number_t's char_t shall be 8-bit wide.
        BOOST_STATIC_ASSERT(sizeof(typename nb_number_t::char_t) == 1 );
        
    public:    
        
#pragma mark - Public Members        
        
                
        SemanticActions(id<JPSemanticActionsProtocol> delegate = nil) 
        :   base(delegate),
            result_(nil),
            tmp_data_str_(NULL),
            number_generator_option_(sa_options::NumberGeneratorGenerateAuto),
            opt_keep_string_cache_on_clear_(false),
            opt_cache_data_strings_(false),
            opt_create_mutable_json_containers_(false)
        {
            f_cf_retain = CFRetain;
            f_cf_release = CFRelease;
            numberGeneratorOption(number_generator_option_);
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE) 
            init_cache();
#endif            
        }
        
        virtual ~SemanticActions() 
        {
            clear_stack();
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE) 
            //clear_cache();
#endif      
            // If result_ is not nil, we need to release it:
            [result_ release];
            assert(tmp_data_str_ == NULL);
            if (tmp_data_str_) {
                CFRelease(tmp_data_str_);
            }
        }
        
        
        virtual void parse_begin_imp() {
            pc_.t_.start();
            stack_.reserve(200);
            markers_.reserve(20);
            tmp_keys_.reserve(100);
            tmp_values_.reserve(100);
            base::parse_begin_imp(); // notifyies delegate
            assert(tmp_data_str_ == NULL);
            if (tmp_data_str_) {
                CFRelease(tmp_data_str_);
            }
        }
        
        
        virtual void parse_end_imp()                            
        {
            if (stack_.size() != 1 or markers_.size() != 0) 
                throwLogicError("json::SemanticActions: logic error");
            
            // The last and only element is the result of the parser. This object
            // is retained when put on the stack. When assigned to result_ we need 
            // not to forget to release result_ (the previous if any) if it is 
            // not nil!
            [result_ release];
            result_ = stack_.back(); 
            stack_.clear();
            //markers_.clear();
            pc_.t_.stop();
            // result_ contains the result of the parser. It's retained.
            assert (tmp_data_str_ == 0);            
            base::parse_end_imp();  // notifyies delegate
        }
        
        
        virtual void finished_imp() { 
            base::finished_imp(); // notifyies delegate
        }
        
        
        virtual void begin_array_imp() 
        {
            //t0_.start();
            //++c0_;
            ++pc_.array_count_;
            markers_.push_back(stack_.size());  // marker points to the next value which shall be inserted into the array, if any.
            //t0_.pause();
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)            
            json_path_.push_index(-1);  // push dummy index
#endif            
        }
        
        
        virtual void end_array_imp() 
        {
            //t1_.start();
            // Calculate the max stack size - for performance counter
            pc_.max_stack_size_ = std::max(pc_.max_stack_size_, stack_.size());
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)            
            json_path_.pop_component();  // pop the index component
#endif            
            size_t first_idx = markers_.back();
            markers_.pop_back();
            stack_t::iterator first = stack_.begin();
            std::advance(first, first_idx);
            stack_t::iterator first_saved = first;
            stack_t::iterator last = stack_.end();
            size_t count = std::distance(first, last);
            
            id a;
            if (opt_create_mutable_json_containers_) 
            {                
                typedef void (*CFArrayAppendValue_t)(CFMutableArrayRef theArray, const void *value);
                CFArrayAppendValue_t f_cf_ArrayAppendValue = CFArrayAppendValue;
                
                // Create a mutable array with a capacity just big enough to hold 
                // the required number of elements (count):
                //a = static_cast<id>(CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks));       
                a = (id)CFMutableArrayRef([[NSMutableArray alloc] initWithCapacity: count]);
                // Populate the array with the values on the stack from first to last:
                while (first != last) {
                    f_cf_ArrayAppendValue(CFMutableArrayRef(a), CFTypeRef(*first));
                    f_cf_release(CFTypeRef(*first));
                    (*first) = NULL; 
                    ++first;
                }                     
            }
            else 
            {
                // Create an immutable array with count elements
                
                // Get the address of the start of the range of values:this only works if stack_t is a vector!!
                //TODO: BOOST_STATIC_ASSERT(boost::is_xx);                
                const void** values = const_cast<const void**>(reinterpret_cast<void**>(&stack_[first_idx]));
                //CFTypeRef* values = reinterpret_cast<void**>(&stack_[first_idx]));
                a = (id)(CFArrayCreate(kCFAllocatorDefault, values, count, &kCFTypeArrayCallBacks));
                // release the CFTypes:
                while (first != last) {
                    f_cf_release(CFTypeRef(*first));
                    (*first) = NULL; 
                    ++first;
                }                     
            }
            // Remove those values from the stack:
            stack_.erase(first_saved, last);            
            // finally, add the array onto the stack:
            stack_.push_back(a);
            //t1_.pause();
        }
        
        
        virtual void begin_object_imp() 
        {
            //t0_.start();
            //++c0_;
            ++pc_.object_count_;
            markers_.push_back(stack_.size()); // marker points to the next value which shall be inserted into the object, if any.
            //t0_.pause();
            
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)            
            json_path_.push_key(typename json_path_t::string_buffer_type(0, 0));  // push dummy Key
#endif            
        }
        
        
        virtual bool end_object_imp() 
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
            //
            // where n equals the number of elements which shall be inserted into object.
            // The marker points to the key[0];
            
            
            //t2_.start();

#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)            
            json_path_.pop_component(); // pop the key component
#endif                        
            pc_.max_stack_size_ = std::max(pc_.max_stack_size_, stack_.size());            
            size_t first_idx = markers_.back();
            markers_.pop_back();
            stack_t::iterator first = stack_.begin();
            std::advance(first, first_idx);
            stack_t::iterator first_saved = first;
            stack_t::iterator last = stack_.end();
            size_t count = std::distance(first, last);
            // The number of elements equals (last - fist) / 2:
            size_t num_elements = count >> 1;
            
            // (last - first) shall be an even integer!
            
            // Create a new object (NSDictionary / NSMutableDictionary) with a capacity 
            // just big enough to hold the required number of elements:
            id object = nil;
            bool duplicateKeyError = false;
            if (opt_create_mutable_json_containers_) 
            {
                //
                //  Create a mutable JSON object
                //                
                typedef void (*CFDictionaryAddValue_t)(CFMutableDictionaryRef theDict, const void *key, const void *value);
                CFDictionaryAddValue_t f_cf_DictionaryAddValue_t = CFDictionaryAddValue;
                
#if !defined (SEMANTIC_ACTIONS_NO_CREATE_MUTABLE_DICTIONARY_USING_CF)
                // use CF to create a dictionary
                CFMutableDictionaryRef o =
                CFDictionaryCreateMutable(NULL, num_elements, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
#else                
                // Workaround: use Foundation to create a dictionary
                CFMutableDictionaryRef o = CFMutableDictionaryRef([[NSMutableDictionary alloc] initWithCapacity:num_elements + 16]);
#endif                
                while (first != last) 
                {
                    // get the key and the value from the stack and insert it:
                    CFTypeRef key =  CFTypeRef(*first++);
                    f_cf_DictionaryAddValue_t(o, key, CFTypeRef(*first++));
                }
                
                if (this->checkDuplicateKey()) {
                    if (CFDictionaryGetCount(CFDictionaryRef(o)) != num_elements) {
                        duplicateKeyError = true;
#if defined (DEBUG)
                        const id* pk = reinterpret_cast<const id*>(static_cast<const void*>(&tmp_keys_[0]));
                        NSLog(@"ERROR: json::SemanticActions: duplicate key error with keys: %@",
                              [NSArray arrayWithObjects:pk count:num_elements]);
                        NSLog(@"Current keys in object: %@", [id(o) allKeys]);
#endif                    
                    }
                }
                object = (id)o;
            }   
            else 
            {
                //
                //  Create an immutable JSON object
                //
                
                // Copy the key-refs and value-refs from the stack to a temporary 
                // vector from which we can create a CFDictionary/NSDictionary
                // NOTE: there is no noticable performance penalty using a temp vector 
                // as opposed to a const size vector on the stack (at max 3µsec).
                tmp_keys_.clear();
                tmp_values_.clear();
                //                tmp_keys_.reserve(num_elements);
                //                tmp_values_.reserve(num_elements);                
                while (first != last)
                {
                    tmp_keys_.push_back(static_cast<tmp_array_obj_t>(*first++));
                    tmp_values_.push_back(static_cast<tmp_array_obj_t>(*first++));
                }
                tmp_array_obj_t* keys = &tmp_keys_[0];
                tmp_array_obj_t* values = &tmp_values_[0];
                // Create immutable JSON object via CF functions:
                CFDictionaryRef o = 
                CFDictionaryCreate(NULL, reinterpret_cast<const void**>(keys), reinterpret_cast<const void**>(values), num_elements, 
                                   &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                
                if (this->checkDuplicateKey()) {
                    if (CFDictionaryGetCount(CFDictionaryRef(o)) != num_elements) {
                        duplicateKeyError = true;
#if defined (DEBUG)
                        const id* pk = reinterpret_cast<const id*>(static_cast<const void*>(&tmp_keys_[0]));                        
                        NSLog(@"ERROR: json::SemanticActions: duplicate key error with keys: %@",
                              [NSArray arrayWithObjects:pk count:num_elements]);
                        NSLog(@"Current keys in object: %@", [id(o) allKeys]);
#endif                    
                    }
                }
                object = (id)o;
            }
            
            assert(object != nil);
            
            // Regardless of duplicateKeyError erase the stack from the objects 
            // which have to be inserted into the object:
            first = first_saved;
            while (first != last) {
                f_cf_release(CFTypeRef(*first));   
                ++first;
            }
            stack_.erase(first_saved, last);
            
            // finally, add the JSON object onto the stack:
            stack_.push_back(id(object));
            
            //t2_.pause();
            return not duplicateKeyError;                
        }
        
        
        virtual void begin_value_at_index_imp(size_t index) {
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)            
            json_path_.back_index() = index;
#endif            
        }        
        virtual void end_value_at_index_imp(size_t) {
        }
        
        virtual void begin_key_value_pair_imp(const const_buffer_t& buffer, size_t nth) 
        {
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)            
            const_buffer_t string_buffer = 
#endif            
            push_key(buffer);
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)
            typedef typename json_path_t::string_buffer_type string_buffer_type;
            json_path_.back_key() = string_buffer_type(string_buffer.first, string_buffer.second);
#endif                        
        }
        virtual void end_key_value_pair_imp(const const_buffer_t& buffer, size_t nth) {
        }


        virtual void value_string_imp(const const_buffer_t& buffer, bool hasMore) { 
            //++c0_;
            pc_.string_count_ += static_cast<int>(!hasMore);
            
            //t0_.start();
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)
            if (cacheDataStrings() and not hasMore) {
                // Only cache *small* data strings
                push_string_cached(buffer);
            } else {
                push_string_uncached(buffer, hasMore);
            }
#else
            push_string_uncached(buffer, hasMore);
#endif            
            //t0_.pause();
        }
        
        
        virtual void value_number_imp(const nb_number_t& number) 
        {
            ++pc_.number_count_;
            switch (number_generator_option_) {
                case sa_options::NumberGeneratorGenerateString:
                    push_number_string(number.string_, number.len_, unicode::UTF_8_encoding_tag());
                    break;
                case sa_options::NumberGeneratorGenerateAuto: {
                    id num = nil;
                    char* endPtr;
                    
                    if (number.isInteger()) {
                        int digits = number.digits();
                        if (digits <= std::numeric_limits<int>::digits10) {
                            int val;
                            //boost::spirit::qi::parse(number.c_str(), number.c_str() + number.c_str_len(), boost::spirit::int_, val);
                            val = int(strtol_l(number.c_str(), &endPtr, 10, NULL));
                            num = (id)CFNumberCreate(NULL, kCFNumberIntType, &val);
                        }
                        else if (digits <= std::numeric_limits<long>::digits10) {
                            long val;
                            //boost::spirit::qi::parse(number.c_str(), number.c_str() + number.c_str_len(), boost::spirit::long_, val);                            
                            val = strtol_l(number.c_str(), &endPtr, 10, NULL);
                            num = (id)CFNumberCreate(NULL, kCFNumberLongType, &val);
                        }
                        else if (digits <= std::numeric_limits<long long>::digits10) {
                            errno = 0;
                            long long val = strtoll_l(number.c_str(), &endPtr, 10, NULL);
                            if (errno != 0) {
                                perror("ERROR: Conversion to long long failed");
                            }
                            num = (id)CFNumberCreate(NULL, kCFNumberLongLongType, &val);
                        } 
                        else if (digits <= 38){
                            // use NSDecimalNumber  
                            num = newDecimalNumberFromString(number.c_str(), number.c_str_len());
                        }
                        else {                            
                            // use NSDecimalNumber                              
                            num = newDecimalNumberFromString(number.c_str(), number.c_str_len());
                            if (digits > 38) {
                                (this->logger_).log(json::utility::LOG_WARNING, 
                                                    "WARNING: JSON number to NSDecimalNumber conversion may loose precision. Original JSON number: %.*s", 
                                                    number.c_str_len(), number.c_str());
                            }
                        }
                        // finished creating an integer
                    } 
                    else /* number is float */
                    {
                        // If there is an exponent, and the exponent is greater
                        // than 127 or smaller then -127 we always use NSNumber
                        // with an underlaying double:
                        int exponent;
                        if ((number.digits() <= std::numeric_limits<double>::digits10) // a double has sufficient precision
                            or ((exponent = number.exp()) > 127 or exponent < -127))  // A NSDecimalNumber would not accept this 
                            // exponent, thus generate a NSNumber with double:                            
                        {                            
                            errno = 0;
                            double d = strtod_l(number.c_str(), &endPtr, NULL);
                            if (errno == ERANGE) {
                                throwRangeError("floating point value out of range");
                            }
                            num = (id)CFNumberCreate(NULL, kCFNumberDoubleType, &d);
                            if (number.digits() > std::numeric_limits<double>::digits10) {
                                // log precision warning:
                                (this->logger_).log(json::utility::LOG_WARNING, 
                                                    "WARNING: JSON number to NSNumber with double conversion may loose precision. Original JSON number: %.*s", 
                                                    number.c_str_len(), number.c_str());
                            }
                        }
                        else 
                        {
                            // use NSDecimalNumber  
                            num = newDecimalNumberFromString(number.c_str(), number.c_str_len());
                            if (number.digits() > 38) {
                                // log precision warning:
                                (this->logger_).log(json::utility::LOG_WARNING, 
                                                    "WARNING: JSON number to NSDecimalNumber conversion may loose precision. Original JSON number: %.*s", 
                                                    number.c_str_len(), number.c_str());
                            }
                        }
                        // finished generating a float
                    }
                    
                    stack_.push_back(num);
                }
                    break;
                    
                case sa_options::NumberGeneratorGenerateDecimalNumber: {
                    // use NSDecimalNumber                            
                    NSDecimalNumber* num = newDecimalNumberFromString(number.c_str(), number.c_str_len());
                    if (number.digits() > 38) {
                        // log precision warning:
                        (this->logger_).log(json::utility::LOG_WARNING, 
                                            "WARNING: JSON number to NSDecimalNumber conversion may loose precision. Original JSON number: %.*s", 
                                            number.c_str_len(), number.c_str());
                    }
                    stack_.push_back(num);
                }
                    break;
            }
        }
        
        
        virtual void value_boolean_imp(bool b) 
        {
            //t0_.start();
            //++c0_;
            ++pc_.boolean_count_;
            CFBooleanRef boolean = b ? kCFBooleanTrue : kCFBooleanFalse;
            stack_.push_back(id(boolean));
            //t0_.pause();
        }
        
        
        virtual void value_null_imp() 
        {
            //t0_.start();
            //++c0_;
            ++pc_.null_count_;
            stack_.push_back(id(kCFNull));
            //t0_.pause();
        }
        
        
        
        virtual void clear_imp() 
        {
            base::clear_imp();
            clear_stack();
            markers_.clear();
            pc_.clear();
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)
            if (not keepStringCacheOnClear())
                clear_cache();          
#endif            
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)
            json_path_.clear();
#endif            
            [result_ release], result_ = nil;
        } 
        
        virtual void print_imp(std::ostream& os) 
        { 
            os << *this; 
        }
                
        
    public:                
        result_type result()            { return result_; }
        
        void    createMutableContainers(bool set)  { opt_create_mutable_json_containers_ = set; }
        bool    createMutableContainers() const    { return opt_create_mutable_json_containers_; }
        
        sa_options::number_generator_option numberGeneratorOption() const { return number_generator_option_; }
        void    numberGeneratorOption(sa_options::number_generator_option opt) { number_generator_option_ = opt; }
        
        bool    keepStringCacheOnClear() const { return opt_keep_string_cache_on_clear_; }
        void    keepStringCacheOnClear(bool set) { opt_keep_string_cache_on_clear_ = set; }
        
        bool    cacheDataStrings() const { return opt_cache_data_strings_; }  // TODO: possibly, data string caching is a bad idea anyway, so remove all referenes, flags, ect. including documentation.
        void    cacheDataStrings(bool set) { opt_cache_data_strings_ = set; }
        
        size_t  string_cache_size() const { 
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)
            return string_cache_.size(); 
#else
            return 0;
#endif            
        }
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)
        size_t  cache_hit_count() const { return cache_hit_count_; }
        size_t  cache_miss_count() const { return cache_miss_count_; }
        size_t  cache_size() const { return string_cache_.size(); }
#endif
        
        
        
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)
        
        size_t json_path_level() const { return json_path_.level(); }
        std::string json_path() const { return json_path_.path(); }
#endif        
        
    protected:
        
        const_buffer_t 
        push_key(const const_buffer_t& buffer) 
        {
            //++c0_;
            ++pc_.string_count_;
            
            //t0_.start();
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)
            return push_string_cached(buffer); 
#else
            push_string_uncached(buffer);
            return const_buffer_t(0,0);
#endif
            //t0_.pause();
        }
        
        
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)
        // Returns a string buffer, which is the key where the string has been
        // associated in the cache.
        const_buffer_t push_string_cached(const const_buffer_t& buffer) 
        {
            // (performance critical function)
            const cache_key_t cache_key = cache_key_t(buffer.first, buffer.second);
            cache_iterator iter = string_cache_.find(cache_key);
            cache_mapped_t str;    // CFTypeRef         
            if (iter == string_cache_.end()) {
                ++cache_miss_count_;
                // The string is not in the cache, create one and insert it into the cache:
                str = createString(buffer.first, buffer.second, EncodingT());
                iter = string_cache_.insert(cache_key, str).first;
            } 
            else {
                ++cache_hit_count_;
                str = (*iter).second;
                f_cf_retain(str);
            }
            stack_.push_back(id(CFTypeRef(str)));
            return const_buffer_t((*iter).first.first, (*iter).first.second);
        }
#endif
        
        void push_string_uncached(const const_buffer_t& buffer, bool hasMore)
        {
            if (tmp_data_str_ == 0 and not hasMore) {
                // Create an immutable string
                CFStringRef str = createString(buffer.first, buffer.second, EncodingT());
                stack_.push_back(id(str));  // Do not release str!
            }
            else if (tmp_data_str_ == 0 and hasMore)
            {
                // Create a mutable string with the partial characters
                tmp_data_str_ = createMutableString(buffer.first, buffer.second, EncodingT());
            }
            else if (tmp_data_str_ != 0 and not hasMore) {
                // append to mutable tmp_data_str_ and push it to the stack:
                appendString(tmp_data_str_, buffer.first, buffer.second, EncodingT());
                stack_.push_back(id(tmp_data_str_));
                tmp_data_str_ = NULL;  // don't CFRelease!
            }
            else /* tmp_data_str_ != 0 and hasMore */ {
                // append to mutable tmp_data_str_
                appendString(tmp_data_str_, buffer.first, buffer.second, EncodingT());
            }
        }
        
        template<typename Encoding>        
        void push_number_string(const utf8_code_unit* s, size_t len, Encoding,
                                typename boost::disable_if<
                                boost::is_same<Encoding, EncodingT>
                                >::type* = 0)
        {
            // A number string is ASCII, so we can simply copy the source
            // to its destination encoded in UTF-16 or UTF-32  - effectively 
            // using no conversion. 
            // One caveat here, though: there is no byte-swapping. This is
            // a non-isssue as long the internal encoding's endianness is 
            // limited to host endianness.
            typedef typename encoding_traits<EncodingT>::code_unit_type char_t;
            char_t buffer[256];
            if(len > 256) {
                throwRuntimeError("bogus number string");
            }
            char_t* dest = buffer;
            std::size_t count = len;
            while (count) {
                --count;
                *dest++ = static_cast<uint8_t>(*s++);
            }
            push_string_uncached(const_buffer_t(buffer, len), false);
        }
        
        // number strings are encoded in ASCII
        template<typename Encoding>        
        void push_number_string(const utf8_code_unit* s, size_t len, Encoding,
                                typename boost::enable_if<
                                boost::is_same<Encoding, EncodingT>
                                >::type* = 0)
        {
            push_string_uncached(const_buffer_t(s, len), false);
        }
        
        
        
    private:
        void clear_stack() {
            typedef stack_t::iterator iterator;
            iterator first = stack_.begin();
            iterator last = stack_.end();
            while (first != last) {
#if 1
                CFRelease((CFTypeRef)(*first));
#else                
                [*first release];
#endif                
                ++first;
            }
            stack_.clear();
        }
        
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE) 
        void clear_cache() {
            string_cache_.clear();
            assert(string_cache_.size() == 0);
            cache_hit_count_ = cache_miss_count_ = 0;
        }
        
        void init_cache() {
            cache_hit_count_ = cache_miss_count_ = 0; 
        }
#endif        
        
    protected:
        result_type     result_;
        
#pragma mark -   Private Members
    private:    
        sa_options::number_generator_option number_generator_option_;
        int parser_non_conformance_options_;
        markers_t           markers_;
        stack_t             stack_;
        tmp_array_t         tmp_keys_;      // used when creating immutable dictionaries
        tmp_array_t         tmp_values_;    // dito
        CFMutableStringRef  tmp_data_str_;
        
        //json_path_t     path_;
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)   
        cache_t             string_cache_;
        size_t              cache_hit_count_;
        size_t              cache_miss_count_;        
#endif

#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH)
 #if !defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)   
  #error  JSON_OBJC_SEMANTIC_ACTIONS_USE_JSON_PATH requires JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE
 #endif        
        json_path_t         json_path_;
#endif
        
        typedef CFTypeRef (*cf_retain_t)(CFTypeRef);
        cf_retain_t         f_cf_retain;
        
        typedef void (*cf_release_t)(CFTypeRef);
        cf_release_t        f_cf_release;
        
        bool                opt_keep_string_cache_on_clear_;
        bool                opt_cache_data_strings_;        
        bool                opt_create_mutable_json_containers_;
        
    protected:
        performance_counter pc_;
        
#pragma mark - Stream Output Operator
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const SemanticActions& sa) 
        {
            typedef SemanticActions::base base;
            
            os << static_cast<base const&>(sa);            
            
            os << "\nExtended Json Parser Options:"
            << "\n\tCreate Mutable Containers : " << (sa.opt_create_mutable_json_containers_ ? "YES" : "NO")
            << "\n\tKeep String Cache On Clear: " << (sa.opt_keep_string_cache_on_clear_ ? "YES" : "NO")
            << "\n\tCache Data Strings:       " << (sa.opt_cache_data_strings_ ? "YES" : "NO")
            << "\n\tNumber Parsing Option:  " << (sa.number_generator_option_ == sa_options::NumberGeneratorGenerateString ? 
                                                "NumberGeneratorGenerateString" 
                                                : sa.number_generator_option_ == sa_options::NumberGeneratorGenerateAuto ? "NumberGeneratorGenerateAuto"
                                                : sa.number_generator_option_ == sa_options::NumberGeneratorGenerateDecimalNumber ? "NumberGeneratorGenerateDecimalNumber"
                                                : "")
            << std::endl;
            
            os << "\nNumber of parsed items:"
            << "\n\tArray count:      " << std::setw(8) << sa.pc_.array_count_ 
            << "\n\tObject count:     " << std::setw(8) << sa.pc_.object_count_ 
            << "\n\tString count:     " << std::setw(8) << sa.pc_.string_count_ 
            << "\n\tBoolean_count:    " << std::setw(8) << sa.pc_.boolean_count_
            << "\n\tNull count:       " << std::setw(8) << sa.pc_.null_count_ 
            << "\n\tNumber count:     " << std::setw(8) << sa.pc_.number_count_
            << "\n\tmax stack items:  " << std::setw(8) << sa.pc_.max_stack_size() << '\n';
            
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_USE_CACHE)   
            os << "\nString Cache:"
            << "\n\telement count:    " << std::setw(8) << sa.string_cache_.size()
            << "\n\thit count:        " << std::setw(8) << sa.cache_hit_count_
            << "\n\tmiss count:       " << std::setw(8) << sa.cache_miss_count_
            << std::endl;
#endif            
            os << "\nPerformance counters for last parsed document:" 
            << "\n\telapsed time:     " << std::fixed << std::setprecision(3) << sa.pc_.t() * 1.0e6 << " µs" 
            << "\n\tpush string:      " << std::fixed << std::setprecision(3) << sa.pc_.t0() * 1.0e6 << " µs"
            << "\n\tbuild array:      " << std::fixed << std::setprecision(3) << sa.pc_.t1() * 1.0e6 << " µs"            
            << "\n\tbuild object:     " << std::fixed << std::setprecision(3) << sa.pc_.t2() * 1.0e6 << " µs"
            << std::endl;
            
            return os;
        }
        
    };  // class SemanitcActions
    
    
    
}}   // namespace json::objc


namespace json { namespace objc {
    
    
}}


#endif // JSON_OBJC_SEMANTIC_ACTIONS_HPP



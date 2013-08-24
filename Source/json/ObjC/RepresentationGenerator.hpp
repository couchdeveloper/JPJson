//
//  RepresentationGenerator.hpp
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

#ifndef JSON_OBJC_REPRESENTATION_GENERATOR_HPP
#define JSON_OBJC_REPRESENTATION_GENERATOR_HPP 

#if !__has_feature(objc_arc)
#warning error This Objective-C file shall be compiled with ARC enabled.
#endif



#include "json/config.hpp"

// JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_MUTABLE_DICTIONARY
// If this macro is defined, a mutable representation of a JSON Object will be
// implemented with a `NSMutableDictionary`. Otherwise a `CFMutableDictionaryRef`
// will be used.
// A CFMutableDictionaryRef shows bad performance, thus it is recommened to
// define this macro.
// Note: iOS 4 does not seem to have this issue.
#define JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_MUTABLE_DICTIONARY

// JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_MUTABLE_ARRAY
// If this macro is defined, a mutable representation of a JSON Array will be
// implemented with a `NSMutableArray`. Otherwise a `CFMutableArrayRef`
// will be used.
//#define JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_MUTABLE_ARRAY


// JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_DICTIONARY
// If this macro is defined, an imutable representation of a JSON Object will be
// implemented with a `NSDictionary`. Otherwise a `CFDictionaryRef` will be used.
//#define JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_DICTIONARY

// JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_ARRAY
// If this macro is defined, an imutable representation of a JSON Array will be
// implemented with a `NSArray`. Otherwise a `CFArrayRef` will be used.
//#define JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_ARRAY




// USE_OPTIMIZED_IMMUTABLE_CALLBACKS
// If this macro is defined, immutable CF containers (CFDictionaryRef and CFArrayRef)
// will use callbacks which avoid one retain/release cycle per element when possible.
#define USE_OPTIMIZED_IMMUTABLE_CALLBACKS


// Note: An arena alloctor is generally only available when creating immutable
// containers (opt_create_mutable_json_containers_ euals false) and when option
// opt_use_arena_allocator_ equals true (default: false).
// Foundation containers do generally not use custom allocators.

// NO_ARENA_ALLOCATOR_FOR_STRINGS
// If defined, does not use the arena allocator for NSStrings (if available).
// Note: The area allocator is only available when creating immutable containers
// and when option `opt_use_arena_allocator_` is set 
//#define NO_ARENA_ALLOCATOR_FOR_STRINGS

// NO_ARENA_ALLOCATOR_FOR_NUMBERS
// If defined, does not use the arena allocator for NSNumbers (if available).
// Note: The area allocator is only available when creating immutable containers
// and when option `opt_use_arena_allocator_` is set.
#define NO_ARENA_ALLOCATOR_FOR_NUMBERS



// JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE shall be defined.
#define JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE


// JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH should be in order to enable 
// the JSON Path API and feature.
//#define JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH


#if defined (DEBUG)
    #define JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER
#endif


#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH) && !defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)   
#error  JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH requires JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE
#endif        


#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>  // description
#include <iomanip>   // description   
#include <limits>
#include <cassert>

#include "SemanticActionsBase.hpp"
#include "unicode_traits.hpp"

#include "json/unicode/unicode_traits.hpp"
#include "json/utility/simple_log.hpp"
#include "json/utility/flags.hpp"
#include "json/utility/string_to_number.hpp"
#include "json/utility/arena_allocator.hpp"

#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)
#include "json/json_path/intrusive_json_path.hpp"
#endif

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>




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



#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)
#include "CFDataCache.hpp"
#endif



#pragma mark - Arena Allocator
namespace {
    
    using json::utility::SysArena;
    
    
    
    void* arenaAllocator_allocate(CFIndex size, CFOptionFlags hint, void *info)
    {
        return reinterpret_cast<SysArena*>(info)->allocate(size);
    }

//    void*   arenaAllocator_reallocate(void *ptr, CFIndex newsize, CFOptionFlags hint, void *info)
//    {
//    }
    
    void arenaAllocator_deallocate(void *ptr, void *info)
    {
        reinterpret_cast<SysArena*>(info)->deallocate(ptr);
    }
    
    void arenaAllocator_deleteArena(const void *info)
    {
        delete reinterpret_cast<SysArena const*>(info);
    }

    CFAllocatorRef ArenaAllocatorCreate(int blockSize = 4096)
    {
        CFAllocatorContext context =
        {
            0,          // version;
            NULL,       // arena;
            NULL,       // CFAllocatorRetainCallBack retain
            arenaAllocator_deleteArena,       // CFAllocatorReleaseCallBack release
            NULL,       // CFAllocatorCopyDescriptionCallBack	copyDescription;
            arenaAllocator_allocate,
            NULL,       // CFAllocatorReallocateCallBack	reallocate
            arenaAllocator_deallocate,
            NULL //CFAllocatorPreferredSizeCallBack	preferredSize;
        };
        
        context.info = new SysArena(blockSize);
        return CFAllocatorCreate(NULL, &context);
    }
    
    void ArenaAllocatorClear(CFAllocatorRef arenaAllocator)
    {
        CFAllocatorContext context;
        CFAllocatorGetContext(arenaAllocator, &context);
        SysArena* arena = reinterpret_cast<SysArena*>(context.info);
        if (arena) {
            arena->clear();
        }
    }
    
}

    
#pragma mark - Private Functions
namespace {
        
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
        NSString* numberString = CFBridgingRelease(
            CFStringCreateWithBytesNoCopy( NULL,
                                          (const UInt8 *)buffer,
                                          len,
                                          kCFStringEncodingUTF8,
                                          false,
                                          kCFAllocatorNull));
        NSScanner* scanner = [[NSScanner alloc] initWithString:numberString];
        numberString = nil;
        NSDecimal decimal;
        BOOL result = [scanner scanDecimal:&decimal];
        scanner = nil;
        if (not result) {
            throwRuntimeError("could not create NSDecimal");
        }
        return [[NSDecimalNumber alloc] initWithDecimal:decimal];
    }
    
    
#pragma mark CFString Creation
    
    // Creates a CFString from a buffer 's' containing 'len' code units encoded 
    // in 'Encoding'. Ownership of the string folows the "Create" rule.
    template <typename Encoding, typename CharT>
    CFStringRef createString(const CharT* s, std::size_t len, Encoding, CFAllocatorRef allocator)
    {
        typedef typename json::unicode::add_endianness<Encoding>::type encoding_type;
        typedef typename json::unicode::encoding_traits<encoding_type>::code_unit_type code_unit_t;
        constexpr CFStringEncoding cf_encoding = json::cf_unicode_encoding_traits<encoding_type>::value;
        constexpr Boolean isExternalRepresentation = false; //cf_encoding == kCFStringEncodingUTF8 ? true : false;
        CFStringRef str = CFStringCreateWithBytes(allocator, 
                                                  static_cast<const UInt8*>(static_cast<const void*>(s)), 
                                                  sizeof(code_unit_t)*len, 
                                                  cf_encoding, isExternalRepresentation);
        assert(str != NULL);
        return str;
    }
    
    
} // unnamed namespace 






namespace json { namespace objc { namespace sa_options {

    //
    // Semantic Actions Options
    // 
    enum number_generator_option {
        NumberGeneratorGenerateAuto,
        NumberGeneratorGenerateAutoWithDecimal,
        NumberGeneratorGenerateString,
        NumberGeneratorGenerateDecimalNumber
    };
    
}}}




namespace json { namespace objc { 
        
    //using json::semanticactions::json_path;
    
    
    using json::unicode::UTF_8_encoding_traits;
    using json::unicode::encoding_traits;
    
#if defined (JSON_INTERNAL_SEMANTIC_ACTIONS_PERFORMANCE_COUNTER)
    using utilities::timer;
#endif
    
    
    
    
#pragma mark - Class RepresentationGenerator
    //
    //  Class RepresentationGenerator
    //
    //  The class `RepresentationGenerator` is a private C++ class which is a
    //  concrete implementation of an abstract semantic actions class. It is
    //  responsible for creating the corresponding JSON representation, which is
    //  itself composed of Foundation objects. It is not meant to be subclassed.
    //  
    //  An instance of this class is used as the implementation object for the
    //  Objective-C class `JPRepresentationGenerator`, the associated Objective-C 
    //  class which is just a wrapper for this C++ class. 
    //
    //  The RepresentationGenerator is "self-contained" which means that it consumes
    //  and handles ALL "parse events" itself and thus, will not require a 
    //  delegate nor is it meant to be subclassed.
    //
    //  Note: It would be possible, that most of the features implemented here 
    //  may be implemented within the corresponding  Objective-C class 
    // `JPRepresentationGenerator` directly. The reason for doing it this way
    //  is performance: dispatching Objective-C methods is still slower than
    //  C++ virtual function dispatch.
    // 
    //  Only the *required* delegate methods as defined in the Objectivive-C 
    //  protocol `JPSemanticActionsProtocol` will be forwarded to the 
    //  corresponding delegate.    
    //  
    //
    //
    // Template parameter `EncodingT` shall be derived from `json::utf_encoding_tag`.
    // `EncodingT` shall match the StringBufferEncoding of the parser. Usullay, 
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
    //  - NumberGeneratorGenerateAutoWithDecimal
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
    class RepresentationGenerator : public SemanticActionsBase<EncodingT>
    {
    public:     
        typedef SemanticActionsBase<EncodingT>      base;
        
  
        
#pragma mark -  struct performance_counter
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)        
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
#endif // JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER        
        
    public:    
        typedef typename base::error_t              error_t;
        typedef typename base::number_desc_t        number_desc_t;
        typedef typename base::char_t               char_t;     // char type of the StringBuffer
        typedef typename base::encoding_t           encoding_t;
        typedef typename base::result_type          result_type;  // __strong id
        
        typedef typename base::buffer_t             buffer_t;
        typedef typename base::const_buffer_t       const_buffer_t;
        
        typedef typename UTF_8_encoding_traits::code_unit_type utf8_code_unit;
        
    private:
        
        typedef std::vector<size_t>                 markers_t;
        typedef std::vector<CFTypeRef>              stack_t;
        
        // temporary vector used when creating immutable CFDictionaries
        typedef std::vector<CFTypeRef>              cf_vector_t;
        
        //typedef json_path<char_t>                   json_path_t;
        

#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)   
        typedef CFDataCache<char_t>                 cache_t;
        typedef typename cache_t::key_type          cache_key_t;
        typedef typename cache_t::value_type        cache_value_t;
        typedef typename cache_t::mapped_type       cache_mapped_t;
        typedef typename cache_t::iterator          cache_iterator;
        typedef typename cache_t::const_iterator    cache_const_iterator;
#endif
        
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)
        typedef  objc_internal::intrusive_json_path<encoding_t> json_path_t;
#endif
        
        
        
    public:    
        
#pragma mark - Public Members        
        
                
        RepresentationGenerator(__unsafe_unretained id<JPSemanticActionsProtocol> delegate = nil) noexcept
        :   base(delegate),
            result_(nil),
            number_generator_option_(sa_options::NumberGeneratorGenerateAuto),
            opt_keep_string_cache_on_clear_(false),
            opt_cache_data_strings_(false),
            opt_create_mutable_json_containers_(false),
            opt_use_arena_allocator_(false),
            allocator_(NULL)
        {
            numberGeneratorOption(number_generator_option_);
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE) 
            init_cache();
#endif            
        }
        
        virtual ~RepresentationGenerator() noexcept
        {
            if (allocator_)
                CFRelease(allocator_);
            clear_stack();
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE) 
            //clear_cache();
#endif      
            assert(tmp_data_str_.size() == 0);
            tmp_data_str_.clear();
        }
        
        virtual void parse_begin_imp() {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)            
            pc_.t_.start();
#endif    
            // Each representation uses its own arena allocator - release the
            // previous one if any:
            if (allocator_) {
                CFRelease(allocator_);
                allocator_ = NULL;
            }
            
#if defined (USE_OPTIMIZED_IMMUTABLE_CALLBACKS)
            immutableArrayCallBacks_ = kCFTypeArrayCallBacks;
            immutableDictionaryKeyCallBacks_ = kCFTypeDictionaryKeyCallBacks;
            immutableDictionaryValueCallBacks_ = kCFTypeDictionaryValueCallBacks;
            if (!opt_create_mutable_json_containers_) {
                immutableArrayCallBacks_.retain = NULL;
                immutableDictionaryKeyCallBacks_.retain = NULL;
                immutableDictionaryValueCallBacks_.retain = NULL;
            }
#endif
            
            if (!opt_create_mutable_json_containers_ && opt_use_arena_allocator_) {
                allocator_ = ArenaAllocatorCreate(4*1024);
            }
            stack_.reserve(200);
            markers_.reserve(20);
            tmp_cf_keys_.reserve(100);
            tmp_cf_values_.reserve(100);
            base::parse_begin_imp(); // notifyies delegate
            assert(tmp_data_str_.size() == 0);
            tmp_data_str_.clear();
        }
        
        
        virtual void parse_end_imp()                            
        {
            // If we are using an arena allocator, release it:
            // (the arena allocator will eventually be deallocated when there
            // are no more objects allocated from this allocator)
//            if (arena_allocator_) {
//                CFRelease(arena_allocator_);
//                arena_allocator_ = NULL;
//            }
            
            // When the parser finished, we have only one remaining element (an
            // instance of CFTypeRef whose retain count equals one) which is stored
            // in stack_[0]. There shall be no other elements.
            assert(stack_.size() == 1);
            assert(markers_.size() == 0);

            if (stack_.size() != 1 or markers_.size() != 0) {
//                for (CFTypeRef o : stack_) {
//                    CFRelease(o);
//                }
//                stack_.clear();
                throwLogicError("json::RepresentationGenerator: logic error");
            }
            
            // The the only remaining element in stack_[0] becomes the result of
            // the parser. This object is retained when put on the stack. When
            // assigned to member object result_ we transfer ownership.
            // The previous object result_ (if not nil) will be released and
            // possibly deallocated.
            result_ = nil;
            CFTypeRef root = stack_.back();
            result_ = CFBridgingRelease(root);
            stack_.clear();
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)            
            pc_.t_.stop();
#endif            
            assert (tmp_data_str_.size() == 0);  // tmp_data_str_ shall be cleared whenever a string has been completed.
            base::parse_end_imp();  // notifies delegate
        }
        
        
        virtual void finished_imp() { 
            base::finished_imp(); // notifyies delegate
        }
        
        
        virtual void begin_array_imp() 
        {
            //t0_.start();
            //++c0_;
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)            
            ++pc_.array_count_;
#endif            
            markers_.emplace_back(stack_.size());  // marker points to the next value which shall be inserted into the array, if any.
            //t0_.pause();
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)            
            json_path_.push_index(-1);  // push dummy index
#endif            
        }
        
        
        virtual void end_array_imp() 
        {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)
            //t1_.start();
            // Calculate the max stack size - for performance counter            
            pc_.max_stack_size_ = std::max(pc_.max_stack_size_, stack_.size());
#endif            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)            
            json_path_.pop_component();  // pop the index component
#endif            
            size_t first_idx = markers_.back();
            markers_.pop_back();
            stack_t::iterator first = stack_.begin();
            std::advance(first, first_idx);
            stack_t::iterator first_saved = first;
            stack_t::iterator last = stack_.end();
            size_t count = std::distance(first, last);
            
            CFTypeRef cfArray;
            if (opt_create_mutable_json_containers_) 
            {
#if !defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_MUTABLE_ARRAY)
                assert(allocator_ == NULL);
                // Create a mutable array with a capacity just big enough to hold
                // the required number of elements (count):
                cfArray = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
                // Populate the array with the values on the stack from first to last:
                while (first != last) {
                    CFArrayAppendValue((CFMutableArrayRef)cfArray, *first);
                    CFRelease(*first++);
                }
#else
                const id* a = (const id *)(void *)(&(*first));
                cfArray = CFBridgingRetain([[NSMutableArray alloc] initWithObjects:a count:count]);
                while (first != last) {
                    CFRelease(*first++);
                }
#endif
            }
            else 
            {
#if !defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_MUTABLE_ARRAY)
                // Create an immutable array with count elements
                // Get the address of the start of the range of values:this only works if stack_t is a vector!!
                CFTypeRef* values = stack_.data() + first_idx;
    #if defined (USE_OPTIMIZED_IMMUTABLE_CALLBACKS)
                cfArray = CFArrayCreate(allocator_, values, count, &immutableArrayCallBacks_);
                // Don't release the items on the stack! (because immutableArrayCallBacks_)
    #else
                cfArray = CFArrayCreate(allocator_, values, count, &kCFTypeArrayCallBacks);
                stack_t::iterator f = first_saved;
                while (f != last) {
                    CFRelease(*f++);
                }
    #endif
#else
                const id* a = (const id *)(void *)(&(*first));
                cfArray = CFBridgingRetain([[NSArray alloc] initWithObjects:a count:count]);
                while (first != last) {
                    CFRelease(*first++);
                }
#endif
            }
            // Remove those values from the stack:
            stack_.erase(first_saved, last);            
            // finally, add the array onto the stack:
            stack_.emplace_back(cfArray);
            //t1_.pause();
        }
        
        
        virtual void begin_object_imp() 
        {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                        
            //pc_.t0_.start();
            //++c0_;
            ++pc_.object_count_;
#endif            
            markers_.emplace_back(stack_.size()); // marker points to the next value which shall be inserted into the object, if any.
            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                        
            //pc_.t0_.pause();
#endif            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)            
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

#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)            
            json_path_.pop_component(); // pop the key component
#endif      
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                        
            pc_.max_stack_size_ = std::max(pc_.max_stack_size_, stack_.size());            
#endif            
            size_t first_idx = markers_.back();
            markers_.pop_back();
            stack_t::iterator first = stack_.begin();
            std::advance(first, first_idx);
            stack_t::iterator last = stack_.end();
            size_t count = std::distance(first, last);
            // The number of elements equals (last - fist) / 2:
            size_t num_elements = count >> 1;
            
            // (last - first) shall be an even integer!
            
            // Create a new object (NSDictionary / NSMutableDictionary) with a capacity 
            // just big enough to hold the required number of elements:
            CFTypeRef cfObject = NULL;
            bool duplicateKeyError = false;
            if (opt_create_mutable_json_containers_) 
            {
                //
                //  Create a mutable JSON object
                //
                assert(allocator_ == NULL);
                
#if !defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_MUTABLE_DICTIONARY)
                // use CF to create a dictionary
                CFMutableDictionaryRef o =
                CFDictionaryCreateMutable(NULL, num_elements + 16, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                stack_t::iterator f = first;
                while (f != last) {
                    // get the key and the value from the stack and insert it:
                    CFTypeRef key =  *f++;
                    CFTypeRef value =  *f++;
                    CFDictionaryAddValue(o, key, value);
                    CFRelease(key);
                    CFRelease(value);
                }
#else
                // use Foundation to create a dictionary
                tmp_cf_keys_.clear();
                tmp_cf_values_.clear();
                stack_t::iterator f = first;
                while (f != last) {
                    tmp_cf_keys_.emplace_back(*f++);
                    tmp_cf_values_.emplace_back(*f++);
                }
                const id* ka = (const id *)(void *)tmp_cf_keys_.data();
                const id* va = (const id *)(void *)tmp_cf_values_.data();
                NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithObjects:va forKeys:ka count:num_elements];
                for (auto e : tmp_cf_keys_) { CFRelease(e); }
                for (auto e : tmp_cf_values_) { CFRelease(e); }
                CFMutableDictionaryRef o = (__bridge_retained CFMutableDictionaryRef)(dict);
#endif
                if (this->checkDuplicateKey()) {
                    if (CFDictionaryGetCount((CFDictionaryRef)o) != num_elements) {
                        duplicateKeyError = true;
#if defined (DEBUG)
                        cf_vector_t _keys;
                        auto _f = first;
                        while (_f != last) {
                            _keys.emplace_back(*_f++);
                            _f++;
                        }
                        NSArray* keysArray = CFBridgingRelease(
                            CFArrayCreate(kCFAllocatorDefault, _keys.data(), _keys.size(), &kCFTypeArrayCallBacks));
                        NSLog(@"ERROR: json::RepresentationGenerator: duplicate key error with keys: %@", keysArray);
                        NSLog(@"Current keys in object: %@", [(__bridge NSDictionary*)o allKeys]);
#endif                    
                    }
                }
                cfObject = o;
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
                tmp_cf_keys_.clear();
                tmp_cf_values_.clear();
                stack_t::iterator f = first;
                while (f != last) {
                    tmp_cf_keys_.emplace_back(*f++);
                    tmp_cf_values_.emplace_back(*f++);
                }
#if !defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_NS_DICTIONARY)
                // Use a CFDictionaryRef
    #if defined (USE_OPTIMIZED_IMMUTABLE_CALLBACKS)
                CFDictionaryRef o =
                CFDictionaryCreate(allocator_, tmp_cf_keys_.data(), tmp_cf_values_.data(),
                                   num_elements,
                                   &immutableDictionaryKeyCallBacks_,
                                   &immutableDictionaryValueCallBacks_);
    #else
                CFDictionaryRef o =
                CFDictionaryCreate(allocator_, tmp_cf_keys_.data(), tmp_cf_values_.data(),
                                   num_elements,
                                   &kCFTypeDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks);
                f = first;
                while (f != last) {
                    CFRelease(*f++);
                }
    #endif
#else
                // Use a NSDictionary
                const id* ka = (const id *)(void *)tmp_cf_keys_.data();
                const id* va = (const id *)(void *)tmp_cf_values_.data();
                NSDictionary* dict = [[NSDictionary alloc] initWithObjects:va forKeys:ka count:num_elements];
                f = first;
                while (f != last) {
                    CFRelease(*f++);
                }
                CFDictionaryRef o = (__bridge_retained CFDictionaryRef)(dict);
#endif
                if (this->checkDuplicateKey()) {
                    if (CFDictionaryGetCount(CFDictionaryRef(o)) != num_elements) {
                        duplicateKeyError = true;
#if defined (DEBUG)
                        NSArray* keysArray = CFBridgingRelease(
                            CFArrayCreate(kCFAllocatorDefault, tmp_cf_keys_.data(), tmp_cf_keys_.size(), &kCFTypeArrayCallBacks));
                        NSLog(@"ERROR: json::RepresentationGenerator: duplicate key error with keys: %@", keysArray);
                        NSLog(@"Current keys in object: %@", [(__bridge NSDictionary*)o allKeys]);
#endif                    
                    }
                }
                cfObject = o;
            }
            
            assert(cfObject != NULL);
            
            // Regardless of duplicateKeyError erase the stack from the objects
            // which have to be inserted into the object:
            stack_.erase(first, last);
            
            // finally, add the JSON object onto the stack:
            stack_.emplace_back(cfObject);
            
            //t2_.pause();
            return not duplicateKeyError;                
        }
        
        
        virtual void begin_value_at_index_imp(size_t index) {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)            
            json_path_.back_index() = index;
#endif            
        }        
        virtual void end_value_at_index_imp(size_t) {
        }
        
        virtual void begin_key_value_pair_imp(const const_buffer_t& buffer, size_t nth) 
        {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)            
            const_buffer_t string_buffer = 
#endif            
            push_key(buffer);
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)
            typedef typename json_path_t::string_buffer_type string_buffer_type;
            json_path_.back_key() = string_buffer_type(string_buffer.first, string_buffer.second);
#endif                        
        }
        virtual void end_key_value_pair_imp() {
        }


        virtual void value_string_imp(const const_buffer_t& buffer, bool hasMore) { 
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            //++c0_;
            pc_.string_count_ += static_cast<int>(!hasMore);
            //t0_.start();
#endif            
            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)
            if (cacheDataStrings() and not hasMore) {
                // Only cache *small* data strings
                push_string_cached(buffer);
            } else {
                push_string_uncached(buffer, hasMore);
            }
#else
            push_string_uncached(buffer, hasMore);
#endif            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            //t0_.pause();
#endif            
        }
        
        
        virtual void value_number_imp(const number_desc_t& number) 
        {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                                
            ++pc_.number_count_;
#endif
#if defined (NO_ARENA_ALLOCATOR_FOR_NUMBERS)
            CFAllocatorRef numberAlloc = NULL;
#else
            CFAllocatorRef numberAlloc = allocator_;
#endif
            switch (number_generator_option_) {
                case sa_options::NumberGeneratorGenerateString:
                    push_number_string(number.c_str(), number.c_str_len(), unicode::UTF_8_encoding_tag());
                    break;
                case sa_options::NumberGeneratorGenerateAuto:
                case sa_options::NumberGeneratorGenerateAutoWithDecimal:
                {
                    CFTypeRef cfNumber = NULL;
                    
                    if (number.is_integer()) 
                    {
                        // signed or unsigned integers
                        int digits = number.digits();
                        if (digits <= std::numeric_limits<int>::digits10) {
                            int val = json::utility::string_to_number<int>(number.c_str(), number.c_str_len());
                            cfNumber = CFNumberCreate(numberAlloc, kCFNumberIntType, &val);
                        }
                        else if (digits <= std::numeric_limits<long>::digits10) {
                            long val = json::utility::string_to_number<long>(number.c_str(), number.c_str_len());
                            cfNumber = CFNumberCreate(numberAlloc, kCFNumberLongType, &val);
                        }
                        else if (digits <= std::numeric_limits<long long>::digits10) {
                            long long val = json::utility::string_to_number<long long>(number.c_str(), number.c_str_len());
                            cfNumber = CFNumberCreate(numberAlloc, kCFNumberLongLongType, &val);
                        } 
                        else if (number_generator_option_ == sa_options::NumberGeneratorGenerateAutoWithDecimal) {
                            // use NSDecimalNumber
                            cfNumber = CFBridgingRetain(newDecimalNumberFromString(number.c_str(), number.c_str_len()));
                            if (digits > 38) {
                                (this->logger_).log(json::utility::LOG_WARNING,
                                                    "JSON number to NSDecimalNumber conversion may loose precision. Original JSON number: %.*s",
                                                    number.c_str_len(), number.c_str());
                            }
                        }
                        else {
                            // Try to represent the integral JSON number in a double:
                            (this->logger_).log(json::utility::LOG_WARNING,
                                                "Integral JSON number is out of range to be represented in largest underlaying integral number representation. Using a double as underlaying type. Original JSON number: %.*s",
                                                number.c_str_len(), number.c_str());
                            double d = json::utility::string_to_number<double>(number.c_str(), number.c_str_len());
                            cfNumber = CFNumberCreate(numberAlloc, kCFNumberDoubleType, &d);
                        }
                            
                        // finished creating an integer
                    } 
                    else if (number.is_scientific()) 
                    {
                        // A number with an exponent
                        // Always use an NSNumber with an underlaying double. If the conversion will possibly loose precision,
                        // log a message but convert it anyway:
                        if (number.digits() > std::numeric_limits<double>::max_digits10) {
                            // The JSON number has more digits than neccesary to be represented by a double precisely enough.
                            // log precision warning:
                            (this->logger_).log(json::utility::LOG_WARNING, 
                                "JSON number to NSNumber with double conversion may loose precision. Original JSON number: %.*s",
                                number.c_str_len(), number.c_str());
                        }
                        double d = json::utility::string_to_number<double>(number.c_str(), number.c_str_len());
                        cfNumber = CFNumberCreate(numberAlloc, kCFNumberDoubleType, &d);
                    }
                    else /* number is fixed*/
                    {
                        // If the precision of a double is sufficient to represent the
                        // decimal number, use an underlaying double, otherwise use
                        // a NSDecimalNumber:
                        if (number.digits() <= std::numeric_limits<double>::max_digits10) // a double has sufficient precision
                        {      
                            // use an NSNumber with underlaying double
                            double d = json::utility::string_to_number<double>(number.c_str(), number.c_str_len());
                            cfNumber = CFNumberCreate(numberAlloc, kCFNumberDoubleType, &d);
                        }
                        else if (number_generator_option_ == sa_options::NumberGeneratorGenerateAutoWithDecimal)
                        {
                            // use NSDecimalNumber  
                            cfNumber = CFBridgingRetain(newDecimalNumberFromString(number.c_str(), number.c_str_len()));
                            if (number.digits() > 38) {
                                // log precision warning:
                                (this->logger_).log(json::utility::LOG_WARNING, 
                                    "JSON number to NSDecimalNumber conversion may loose precision. Original JSON number: %.*s",
                                    number.c_str_len(), number.c_str());
                            }
                        }
                        else {
                            // use an NSNumber with underlaying double
                            // log precision warning:
                            double d = json::utility::string_to_number<double>(number.c_str(), number.c_str_len());
                            cfNumber = CFNumberCreate(numberAlloc, kCFNumberDoubleType, &d);
                            (this->logger_).log(json::utility::LOG_WARNING,
                                                "JSON number to NSNumber with double conversion may loose precision. digits: %d, maximum digits for retaining pecision: %d. Original JSON number: \"%.*s\"",
                                                (int)number.digits(), (int)std::numeric_limits<double>::max_digits10,
                                                number.c_str_len(), number.c_str());
                            
                        }
                        // finished generating a float
                    }
                    assert(cfNumber != NULL);
                    stack_.emplace_back(cfNumber);
                }
                    break;
                    
                case sa_options::NumberGeneratorGenerateDecimalNumber: {
                    // use NSDecimalNumber                            
                    CFTypeRef cfNumber = CFBridgingRetain(newDecimalNumberFromString(number.c_str(), number.c_str_len()));
                    if (number.digits() > 38) {
                        // log precision warning:
                        (this->logger_).log(json::utility::LOG_WARNING, 
                                            "JSON number to NSDecimalNumber conversion may loose precision. Original JSON number: %.*s",
                                            number.c_str_len(), number.c_str());
                    }
                    stack_.emplace_back(cfNumber);
                }
                    break;
            }
        }
        
        
        virtual void value_boolean_imp(bool b) 
        {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            //t0_.start();
            //++c0_;
            ++pc_.boolean_count_;
#endif            
            CFBooleanRef boolean = b ? kCFBooleanTrue : kCFBooleanFalse;
            stack_.emplace_back(boolean);
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            //t0_.pause();
#endif            
        }
        
        
        virtual void value_null_imp() 
        {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            //t0_.start();
            //++c0_;
            ++pc_.null_count_;
#endif            
            stack_.emplace_back(kCFNull);
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            //t0_.pause();
#endif            
        }
        
        
        
        virtual void clear_imp(bool shrink_buffers)
        {
            base::clear_imp(shrink_buffers);
            clear_stack();
            if (allocator_) {
                CFRelease(allocator_);
                allocator_ = NULL;
            }
            markers_.clear();
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            pc_.clear();
#endif            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)
            if (not keepStringCacheOnClear())
                clear_cache();          
#endif            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)
            json_path_.clear();
#endif            
            result_ = nil;
        } 
        
        virtual void print_imp(std::ostream& os) 
        { 
            os << *this; 
        }
                
        
    public:                
        result_type result()            { return result_; }
        
        void    createMutableContainers(bool set)  { opt_create_mutable_json_containers_ = set; }
        bool    createMutableContainers() const    { return opt_create_mutable_json_containers_; }
        
        void    useArenaAllocator(bool set)  { opt_use_arena_allocator_ = set; }
        bool    useArenaAllocator() const    { return opt_use_arena_allocator_; }
        
        sa_options::number_generator_option numberGeneratorOption() const { return number_generator_option_; }
        void    numberGeneratorOption(sa_options::number_generator_option opt) { number_generator_option_ = opt; }
        
        bool    keepStringCacheOnClear() const { return opt_keep_string_cache_on_clear_; }
        void    keepStringCacheOnClear(bool set) { opt_keep_string_cache_on_clear_ = set; }
        
        bool    cacheDataStrings() const { return opt_cache_data_strings_; }  // TODO: possibly, data string caching is a bad idea anyway, so remove all referenes, flags, ect. including documentation.
        void    cacheDataStrings(bool set) { opt_cache_data_strings_ = set; }
        
        size_t  string_cache_size() const { 
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)
            return string_cache_.size(); 
#else
            return 0;
#endif            
        }
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)
        size_t  cache_hit_count() const { return cache_hit_count_; }
        size_t  cache_miss_count() const { return cache_miss_count_; }
        size_t  cache_size() const { return string_cache_.size(); }
#endif
        
        
        
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)
        
        size_t json_path_level() const { return json_path_.level(); }
        std::string json_path() const { return json_path_.path(); }
#endif        
        
    protected:
        
        const_buffer_t 
        push_key(const const_buffer_t& buffer) 
        {
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)                                    
            ++pc_.string_count_;
#endif            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)
            return push_string_cached(buffer); 
#else
            push_string_uncached(buffer);
            return const_buffer_t(0,0);
#endif
        }
        
        
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)
        // Returns a string buffer, which is the key where the string has been
        // associated in the cache.
        const_buffer_t push_string_cached(const const_buffer_t& buffer) 
        {
#if defined (NO_ARENA_ALLOCATOR_FOR_STRINGS)
            CFAllocatorRef stringAllocator = NULL;
#else
            CFAllocatorRef stringAllocator = allocator_;
#endif
            // (performance critical function)
            const cache_key_t cache_key = cache_key_t(buffer.first, buffer.second);
            cache_iterator iter = string_cache_.find(cache_key);
            cache_mapped_t cfStr;    // CFTypeRef
            if (iter == string_cache_.end()) {
                ++cache_miss_count_;
                // The string is not in the cache, create one and insert it into the cache:
                cfStr = createString(buffer.first, buffer.second, EncodingT(), stringAllocator);
                assert(cfStr != NULL);
                iter = string_cache_.insert(cache_key, cfStr).first;
            } 
            else {
                ++cache_hit_count_;
                cfStr = (*iter).second;
                CFRetain(cfStr);
            }
            stack_.emplace_back(cfStr);
            return const_buffer_t((*iter).first.first, (*iter).first.second);
        }
#endif
        
        void push_string_uncached(const const_buffer_t& buffer, bool hasMore)
        {
#if defined (NO_ARENA_ALLOCATOR_FOR_STRINGS)
            CFAllocatorRef stringAllocator = NULL;
#else
            CFAllocatorRef stringAllocator = allocator_;
#endif
            
            if (tmp_data_str_.size() == 0 and not hasMore) {
                // Create an immutable string
                CFStringRef cfStr = createString(buffer.first, buffer.second, EncodingT(), stringAllocator);
                stack_.emplace_back(cfStr);  // Do not release str!
            }
            else if (hasMore) {
                // append the current chunk in temporary buffer
                tmp_data_str_.insert(tmp_data_str_.end(), buffer.first, buffer.first + buffer.second);
            }
            else { // if (tmp_data_str_.size() != 0 and not hasMore)
                // append to mutable tmp_data_str_, then create a CFString and push it to the stack:
                tmp_data_str_.insert(tmp_data_str_.end(), buffer.first, buffer.first + buffer.second);
                CFStringRef cfStr = createString(tmp_data_str_.data(), tmp_data_str_.size(), EncodingT(), stringAllocator);
                stack_.emplace_back(cfStr);
                tmp_data_str_.clear();
            }
        }
        
        template<typename Encoding>        
        void push_number_string(const utf8_code_unit* s, size_t len, Encoding,
                                typename boost::disable_if<
                                boost::is_same<Encoding, EncodingT>
                                >::type* = 0)
        {
            // TODO: consider to cache short number strings
            
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
        void clear_stack() noexcept {
            for (CFTypeRef o : stack_) {
                CFRelease(o);
            }
            stack_.clear();
        }
        
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE) 
        void clear_cache() noexcept {
            string_cache_.clear();
            cache_hit_count_ = cache_miss_count_ = 0;
        }
        
        void init_cache() noexcept {
            cache_hit_count_ = cache_miss_count_ = 0; 
        }
#endif        
        
    protected:
        CFAllocatorRef              allocator_;
#if defined (USE_OPTIMIZED_IMMUTABLE_CALLBACKS)
        CFArrayCallBacks            immutableArrayCallBacks_;
        CFDictionaryKeyCallBacks    immutableDictionaryKeyCallBacks_;
        CFDictionaryValueCallBacks  immutableDictionaryValueCallBacks_;
#endif
        result_type                 result_;
        
#pragma mark -   Private Members
    private:    
        sa_options::number_generator_option number_generator_option_;
        int                         parser_non_conformance_options_;
        markers_t                   markers_;
        stack_t                     stack_;
        cf_vector_t                 tmp_cf_keys_;      // used when creating immutable dictionaries
        cf_vector_t                 tmp_cf_values_;    // dito
//        std::vector<const id>     tmp_id_keys_;      // used when creating immutable dictionaries
//        std::vector<const id>     tmp_id_values_;    // dito
        
        std::vector<char_t>         tmp_data_str_;
        
        //json_path_t     path_;
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)   
        cache_t                     string_cache_;
        size_t                      cache_hit_count_;
        size_t                      cache_miss_count_;
#endif

#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH)
 #if !defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)   
  #error  JSON_OBJC_REPRESENTATION_GENERATOR_USE_JSON_PATH requires JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE
 #endif        
        json_path_t         json_path_;
#endif
        bool                opt_keep_string_cache_on_clear_;
        bool                opt_cache_data_strings_;        
        bool                opt_create_mutable_json_containers_;
        bool                opt_use_arena_allocator_;
        
    protected:
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)        
        performance_counter pc_;
#endif        
        
#pragma mark - Stream Output Operator
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const RepresentationGenerator& sa) 
        {
            typedef RepresentationGenerator::base base;
            
            os << static_cast<base const&>(sa);            
            
            os << "\nExtended Json Parser Options:"
            << "\n\tCreate Mutable Containers : " << (sa.opt_create_mutable_json_containers_ ? "YES" : "NO")
            << "\n\tUses Arean Allocator for immutable Objects : " << (sa.opt_use_arena_allocator_ ? "YES" : "NO")
            << "\n\tKeep String Cache On Clear: " << (sa.opt_keep_string_cache_on_clear_ ? "YES" : "NO")
            << "\n\tCache Data Strings:       " << (sa.opt_cache_data_strings_ ? "YES" : "NO")
            << "\n\tNumber Parsing Option:  " << (sa.number_generator_option_ == sa_options::NumberGeneratorGenerateString ? 
                                                "NumberGeneratorGenerateString" 
                                                : sa.number_generator_option_ == sa_options::NumberGeneratorGenerateAuto ? "NumberGeneratorGenerateAuto"
                                                : sa.number_generator_option_ == sa_options::NumberGeneratorGenerateDecimalNumber ? "NumberGeneratorGenerateDecimalNumber"
                                                : "")
            << std::endl;
            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)            
            os << "\nNumber of parsed items:"
            << "\n\tArray count:      " << std::setw(8) << sa.pc_.array_count_ 
            << "\n\tObject count:     " << std::setw(8) << sa.pc_.object_count_ 
            << "\n\tString count:     " << std::setw(8) << sa.pc_.string_count_ 
            << "\n\tBoolean_count:    " << std::setw(8) << sa.pc_.boolean_count_
            << "\n\tNull count:       " << std::setw(8) << sa.pc_.null_count_ 
            << "\n\tNumber count:     " << std::setw(8) << sa.pc_.number_count_
            << "\n\tmax stack items:  " << std::setw(8) << sa.pc_.max_stack_size() << '\n';
#endif            
            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_CACHE)   
            os << "\nString Cache:"
            << "\n\telement count:    " << std::setw(8) << sa.string_cache_.size()
            << "\n\thit count:        " << std::setw(8) << sa.cache_hit_count_
            << "\n\tmiss count:       " << std::setw(8) << sa.cache_miss_count_
            << std::endl;
#endif        
            
#if defined (JSON_OBJC_REPRESENTATION_GENERATOR_USE_PERFORMANCE_COUNTER)            
            os << "\nPerformance counters for last parsed document:" 
            << "\n\telapsed time:     " << std::fixed << std::setprecision(3) << sa.pc_.t() * 1.0e6 << " µs" 
            << "\n\tpush string:      " << std::fixed << std::setprecision(3) << sa.pc_.t0() * 1.0e6 << " µs"
            << "\n\tbuild array:      " << std::fixed << std::setprecision(3) << sa.pc_.t1() * 1.0e6 << " µs"            
            << "\n\tbuild object:     " << std::fixed << std::setprecision(3) << sa.pc_.t2() * 1.0e6 << " µs"
            << std::endl;
#endif            
            
            return os;
        }
        
    };  // class SemanitcActions
    
    
    
}}   // namespace json::objc


namespace json { namespace objc {
    
    
}}


#endif // JSON_OBJC_REPRESENTATION_GENERATOR_HPP



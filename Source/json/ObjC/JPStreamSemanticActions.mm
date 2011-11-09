//
//  JPStreamSemanticActions.mm
//  
//
//  Created by Andreas Grosam on 8/24/11.
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

#import "JPStreamSemanticActions.h"
#import "JPJsonCommon.h"
#include "SemanticActionsBase.hpp"
#include "JPSemanticActionsBase_private.h"
#include "json/unicode/unicode_utilities.hpp"
#include "json/ObjC/unicode_traits.hpp"



namespace {
    
    
    //
    // Private Semantic Actions Implementation Class
    //
    
    template <typename EncodingT>
    class StreamSemanticActionsBase : 
        public json::objc::SemanticActionsBase<EncodingT>
    {
        typedef json::objc::SemanticActionsBase<EncodingT> base;
    public:
        typedef typename EncodingT::code_unit_type char_t;
        typedef typename base::nb_number_t nb_number_t;
    public:
        StreamSemanticActionsBase(id<JPSemanticActionsProtocol> delegate = nil)
        : base(delegate)
        {}
        
        StreamSemanticActionsBase() {}
        
        
        virtual void parse_begin_imp() {
            base::parse_begin_imp();
        }
        
        virtual void parse_end_imp() {
            base::parse_end_imp();
        }
        
        virtual void finished_imp() {
            base::finished_imp();
        }
        
        
        virtual void push_string_imp(const char_t* s, std::size_t len) {
            typedef void (*imp_func_ptr_t)(id, SEL, const void*, size_t, NSStringEncoding);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonString:length:encoding:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel, 
                    static_cast<const void*>(s), len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }        
        
        virtual void push_key_imp(const char_t* s, std::size_t len) {
            typedef void (*imp_func_ptr_t)(id, SEL, const void*, size_t, NSStringEncoding);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonKey:length:encoding:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel,  
                    static_cast<const void*>(s), len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void push_number_imp(const nb_number_t& number) {
            typedef void (*imp_func_ptr_t)(id, SEL, const char*, size_t);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonNumber:length:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel, number.c_str(), number.c_str_len());
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void push_boolean_imp(bool b) {
            typedef void (*imp_func_ptr_t)(id, SEL, BOOL);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonBoolean:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel, static_cast<BOOL>(b));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void push_null_imp() {
            typedef void (*imp_func_ptr_t)(id, SEL);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonNull);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void begin_array_imp() {
            typedef void (*imp_func_ptr_t)(id, SEL);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonArrayBegin);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_array_imp() {
            typedef void (*imp_func_ptr_t)(id, SEL);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonArrayEnd);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void begin_object_imp() {
            typedef void (*imp_func_ptr_t)(id, SEL);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonObjectBegin);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual bool end_object_imp() {
            typedef BOOL (*imp_func_ptr_t)(id, SEL);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonObjectEnd);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            BOOL result = YES;
            if (imp) {
                result = imp(this->delegate_, sel);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
            return result;
        }

        virtual void begin_value_at_index_imp(size_t index) {
            typedef void (*imp_func_ptr_t)(id, SEL, size_t);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonValueBeginAtIndex:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel, index);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_value_at_index_imp(size_t index) {
            typedef void (*imp_func_ptr_t)(id, SEL, size_t);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonValueEndAtIndex:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel, index);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void begin_value_with_key_imp(const char_t* s, size_t len, size_t nth) {
            typedef void (*imp_func_ptr_t)(id, SEL, const char_t*, size_t, NSStringEncoding, size_t);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonValueBeginWithKey:length:encoding:index:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel, 
                    s, len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value, nth);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_value_with_key_imp(const char_t* s, size_t len, size_t nth) {
            typedef void (*imp_func_ptr_t)(id, SEL, const char_t*, size_t, NSStringEncoding, size_t);
            static imp_func_ptr_t imp = reinterpret_cast<imp_func_ptr_t>(0x01);
            const SEL sel = @selector(parserFoundJsonValueEndWithKey:length:encoding:index:);
            if (imp == reinterpret_cast<imp_func_ptr_t>(0x01)) {            
                imp = [this->delegate_ respondsToSelector:sel] ? 
                (imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: sel]
                : 0;
            }
            if (imp) {
                imp(this->delegate_, sel, 
                    s, len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value, nth);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
/*        
        virtual void pop_imp() {
            base::pop_imp();
        }
        
        virtual void clear_imp() {
            base::clear_imp();
        } 
        
        virtual void error_imp(const error_t& error) {
            base::error_imp(error);
        }
        
        virtual result_type result() { 
            return nil; 
        }
 */        
        
        virtual void print_imp(std::ostream& os) { 
            os << *this; 
        }

        
#pragma mark - Friend Stream Output Operator
    private:
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const StreamSemanticActionsBase& sa) 
        {
            typedef StreamSemanticActionsBase::base base;
            os << static_cast<base const&>(sa);   
            
            // Insert code ...
            
            return os;
        }
             
    };
}



#pragma mark -


//
//  Defines the internal semantic actions class 'SemanticActions'
// 
typedef StreamSemanticActionsBase<JP_CFStringEncoding>    SemanticActions;



#pragma mark -

@interface JPStreamSemanticActions()
// As defined in the header file JPSemanticActionsBase_private.h,
// the property imp shall be implemented:
@property (readonly, nonatomic) SemanticActionsBase* imp;
@end


@implementation JPStreamSemanticActions
{
@private
    SemanticActions*                    sa_;  // the implementation 
    
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_BASE_JSON_PATH)
    json::json_internal::json_path<JP_CFStringEncoding>    json_path_;
#endif    
    
    // We provide the property "inputEncoding" and "hasBOM" for clients, as 
    // they will probably detect this.
    BOOL                                hasBOM_;
    BOOL                                parseMultipleDocumentsAsynchronously_;
    // TODO: timeout
}


//
// Designated Initializer
//
- (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue
{
    self = [super initWithHandlerDispatchQueue:handlerQueue];
    if (self) {
        sa_ = new SemanticActions(self);
    }    
    return self;
}



- (void) dealloc 
{
    delete sa_;
    [super dealloc];
}

#pragma mark - 

//@property (readonly, nonatomic) SemanticActionsBase* imp;
- (SemanticActionsBase*) imp {
    assert(sa_);
    return sa_;
}

             
             
/*          
- (void) parserFoundJsonString:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding
{
}

 - (void) parserFoundJsonKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding
{
}

- (void) parserFoundJsonNumber:(const char*)numberString length:(size_t)length
{
}

- (void) parserFoundJsonBoolean:(BOOL)value
{
}

- (void) parserFoundJsonNull
{
}

- (void) parserFoundJsonArrayBegin
{
}

- (void) parserFoundJsonArrayEnd
{
}

- (void) parserFoundJsonObjectBegin
{
}

- (bool) parserFoundJsonObjectEnd
{
    return true;
}
 
- (void) parserFoundJsonValueBeginAtIndex:(size_t)index
{
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_BASE_JSON_PATH)
    json_path_.push_index(index);
    //std::cout << json_path_.path<json::unicode::UTF_8_encoding_tag>() << std::endl;
#endif            
    
}

- (void) parserFoundJsonValueEndAtIndex:(size_t)index
{
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_BASE_JSON_PATH)
    json_path_.pop_component();
#endif            
}

- (void) parserFoundJsonValueBeginWithKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding
{
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_BASE_JSON_PATH)
    typedef JP_CFStringEncoding::code_unit_type char_t;
    const char_t* s = reinterpret_cast<char_t const*>(bytes);
    const size_t len = length / sizeof(char_t);
    json_path_.push_key(s, s+len);
    //std::cout << json_path_.path<json::unicode::UTF_8_encoding_tag>() << std::endl;
#endif            
}

- (void) parserFoundJsonValueEndWithKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding
{
#if defined (JSON_OBJC_SEMANTIC_ACTIONS_BASE_JSON_PATH)
    json_path_.pop_component();
#endif            
}
*/
 
             

@end

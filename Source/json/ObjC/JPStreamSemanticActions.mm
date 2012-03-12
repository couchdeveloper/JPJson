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
#include "json/unicode/unicode_traits.hpp"
#include "json/ObjC/unicode_traits.hpp"



namespace {
    
    
    using json::unicode::encoding_traits;
    
    //
    // Private Semantic Actions Implementation Class
    //
    
    template <typename EncodingT>
    class StreamSemanticActionsBase : 
        public json::objc::SemanticActionsBase<EncodingT>
    {
        typedef json::objc::SemanticActionsBase<EncodingT> base;
        
    public:
        typedef typename encoding_traits<EncodingT>::code_unit_type char_t;
        typedef typename base::nb_number_t nb_number_t;

    private:                
        typedef void (*push_string_imp_func_ptr_t)(id, SEL, const void*, size_t, NSStringEncoding);
        typedef void (*push_key_imp_func_ptr_t)(id, SEL, const void*, size_t, NSStringEncoding);
        typedef void (*push_number_imp_func_ptr_t)(id, SEL, const char*, size_t);
        typedef void (*push_boolean_imp_func_ptr_t)(id, SEL, BOOL);
        typedef void (*push_null_imp_func_ptr_t)(id, SEL);
        typedef void (*begin_array_imp_func_ptr_t)(id, SEL);
        typedef void (*end_array_imp_func_ptr_t)(id, SEL);
        typedef void (*begin_object_imp_func_ptr_t)(id, SEL);
        typedef BOOL (*end_object_imp_func_ptr_t)(id, SEL);
        typedef void (*begin_value_at_index_imp_func_ptr_t)(id, SEL, size_t);
        typedef void (*end_value_at_index_imp_func_ptr_t)(id, SEL, size_t);
        typedef void (*begin_value_with_key_imp_func_ptr_t)(id, SEL, const char_t*, size_t, NSStringEncoding, size_t);
        typedef void (*end_value_with_key_imp_func_ptr_t)(id, SEL, const char_t*, size_t, NSStringEncoding, size_t);
        
    public:
        StreamSemanticActionsBase(id<JPSemanticActionsProtocol> delegate = nil)
        : base(delegate)
        {}
        
        StreamSemanticActionsBase() {}
        
        
        virtual void parse_begin_imp() {
            push_string_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonString:length:encoding:)] ? 
                (push_string_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonString:length:encoding:)]
                : 0;        
            push_key_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonKey:length:encoding:)] ? 
                (push_key_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonKey:length:encoding:)]
                : 0;
            push_number_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonNumber:length:)] ? 
                (push_number_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:@selector(parserFoundJsonNumber:length:)]
                : 0;
            push_boolean_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonBoolean:)] ? 
                (push_boolean_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:@selector(parserFoundJsonBoolean:)]
                : 0;
            push_null_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonNull)] ? 
                (push_null_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:@selector(parserFoundJsonNull)]
                : 0;
            begin_array_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonArrayBegin)] ? 
                (begin_array_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonArrayBegin)]
                : 0;
            end_array_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonArrayEnd)] ? 
                (end_array_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonArrayEnd)]
                : 0;
            begin_object_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonObjectBegin)] ? 
                (begin_object_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonObjectBegin)]
                : 0;
            end_object_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonObjectEnd)] ? 
                (end_object_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonObjectEnd)]
                : 0;
            begin_value_at_index_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonValueBeginAtIndex:)] ? 
                (begin_value_at_index_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonValueBeginAtIndex:)]
                : 0;
            end_value_at_index_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonValueEndAtIndex:)] ? 
                (end_value_at_index_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonValueEndAtIndex:)]
                : 0;
            begin_value_with_key_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonValueBeginWithKey:length:encoding:index:)] ? 
                (begin_value_with_key_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonValueBeginWithKey:length:encoding:index:)]
                : 0;
            end_value_with_key_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundJsonValueEndWithKey:length:encoding:index:)] ? 
                (end_value_with_key_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundJsonValueEndWithKey:length:encoding:index:)]
                : 0;

            base::parse_begin_imp();
        }
        
        virtual void parse_end_imp() {
            base::parse_end_imp();
        }
        
        virtual void finished_imp() {
            base::finished_imp();
        }
        
        virtual void push_string_imp(const char_t* s, std::size_t len) {
            if (push_string_imp_) {
                push_string_imp_(this->delegate_, @selector(parserFoundJsonString:length:encoding:), 
                    static_cast<const void*>(s), len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }        
        
        virtual void push_key_imp(const char_t* s, std::size_t len) {
            if (push_key_imp_) {
                push_key_imp_(this->delegate_, @selector(parserFoundJsonKey:length:encoding:),  
                    static_cast<const void*>(s), len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void push_number_imp(const nb_number_t& number) {
            if (push_number_imp_) {
                push_number_imp_(this->delegate_, @selector(parserFoundJsonNumber:length:), number.c_str(), number.c_str_len());
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void push_boolean_imp(bool b) {
            if (push_boolean_imp_) {
                push_boolean_imp_(this->delegate_, @selector(parserFoundJsonBoolean:), static_cast<BOOL>(b));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void push_null_imp() {
            if (push_null_imp_) {
                push_null_imp_(this->delegate_, @selector(parserFoundJsonNull));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void begin_array_imp() {
            if (begin_array_imp_) {
                begin_array_imp_(this->delegate_, @selector(parserFoundJsonArrayBegin));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_array_imp() {
            if (end_array_imp_) {
                end_array_imp_(this->delegate_, @selector(parserFoundJsonArrayEnd));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void begin_object_imp() {
            if (begin_object_imp_) {
                begin_object_imp_(this->delegate_, @selector(parserFoundJsonObjectBegin));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual bool end_object_imp() {
            BOOL result = YES;
            if (end_object_imp_) {
                result = end_object_imp_(this->delegate_, @selector(parserFoundJsonObjectEnd));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
            return result;
        }

        virtual void begin_value_at_index_imp(size_t index) {
            if (begin_value_at_index_imp_) {
                begin_value_at_index_imp_(this->delegate_, @selector(parserFoundJsonValueBeginAtIndex:), index);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_value_at_index_imp(size_t index) {
            if (end_value_at_index_imp_) {
                end_value_at_index_imp_(this->delegate_, @selector(parserFoundJsonValueEndAtIndex:), index);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void begin_value_with_key_imp(const char_t* s, size_t len, size_t nth) {
            if (begin_value_with_key_imp_) {
                begin_value_with_key_imp_(this->delegate_, @selector(parserFoundJsonValueBeginWithKey:length:encoding:index:), 
                    s, len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value, nth);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_value_with_key_imp(const char_t* s, size_t len, size_t nth) {
            if (end_value_with_key_imp_) {
                end_value_with_key_imp_(this->delegate_, @selector(parserFoundJsonValueEndWithKey:length:encoding:index:), 
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
        
    private:        
        push_string_imp_func_ptr_t          push_string_imp_;        
        push_key_imp_func_ptr_t             push_key_imp_;
        push_number_imp_func_ptr_t          push_number_imp_;
        push_boolean_imp_func_ptr_t         push_boolean_imp_;
        push_null_imp_func_ptr_t            push_null_imp_;
        begin_array_imp_func_ptr_t          begin_array_imp_;
        end_array_imp_func_ptr_t            end_array_imp_;
        begin_object_imp_func_ptr_t         begin_object_imp_;
        end_object_imp_func_ptr_t           end_object_imp_;
        begin_value_at_index_imp_func_ptr_t begin_value_at_index_imp_;
        end_value_at_index_imp_func_ptr_t   end_value_at_index_imp_;
        begin_value_with_key_imp_func_ptr_t begin_value_with_key_imp_;
        end_value_with_key_imp_func_ptr_t   end_value_with_key_imp_;

        
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

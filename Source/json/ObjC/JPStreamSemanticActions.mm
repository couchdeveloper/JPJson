//
//  JPStreamSemanticActions.mm
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

#if __has_feature(objc_arc) 
#error This Objective-C file shall be compiled with ARC disabled.
#endif


#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "json/ObjC/unicode_traits.hpp"
#include "SemanticActionsBase.hpp"
#import "JPStreamSemanticActions.h"
#import "JPSemanticActionsBase_private.h"
#import "JPJsonCommon.h"



namespace {
    
    
    using json::unicode::encoding_traits;
    
    //
    // Private Semantic Actions Implementation Class
    //
    
    template <typename EncodingT>
    class StreamSemanticActionsBase : 
        public json::objc::SemanticActionsBase<EncodingT>
    {
        typedef json::objc::SemanticActionsBase<EncodingT>          base;
        
    public:
        typedef typename encoding_traits<EncodingT>::code_unit_type char_t;
        typedef typename base::number_desc_t                        number_desc_t;
        typedef typename base::buffer_t                             buffer_t;
        typedef typename base::const_buffer_t                       const_buffer_t;

    private:                
        typedef void (*write_string_imp_func_ptr_t)(id, SEL, const void*, size_t, BOOL, NSStringEncoding);
        //typedef void (*push_string_imp_func_ptr_t)(id, SEL, const void*, size_t, NSStringEncoding);
        //typedef void (*push_key_imp_func_ptr_t)(id, SEL, const void*, size_t, NSStringEncoding);
        typedef void (*push_number_imp_func_ptr_t)(id, SEL, const char*, size_t);
        typedef void (*push_boolean_imp_func_ptr_t)(id, SEL, BOOL);
        typedef void (*push_null_imp_func_ptr_t)(id, SEL);
        typedef void (*begin_array_imp_func_ptr_t)(id, SEL);
        typedef void (*end_array_imp_func_ptr_t)(id, SEL);
        typedef void (*begin_object_imp_func_ptr_t)(id, SEL);
        typedef BOOL (*end_object_imp_func_ptr_t)(id, SEL);
        typedef void (*begin_value_at_index_imp_func_ptr_t)(id, SEL, size_t);
        typedef void (*end_value_at_index_imp_func_ptr_t)(id, SEL, size_t);
        typedef void (*begin_key_value_pair_imp_func_ptr_t)(id, SEL, const char_t*, size_t, NSStringEncoding, size_t);
        typedef void (*end_key_value_pair_imp_func_ptr_t)(id, SEL);
        
    public:
        StreamSemanticActionsBase(id<JPSemanticActionsProtocol> delegate = nil)
        : base(delegate)
        {}
        
        StreamSemanticActionsBase() {}
        
        
        virtual void parse_begin_imp() {
            write_string_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundString:length:hasMore:encoding:)] ? 
            (write_string_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundString:length:hasMore:encoding:)]
            : 0;        
            
            push_number_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundNumber:length:)] ? 
                (push_number_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:@selector(parserFoundNumber:length:)]
                : 0;
            push_boolean_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundBoolean:)] ? 
                (push_boolean_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:@selector(parserFoundBoolean:)]
                : 0;
            push_null_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundNull)] ? 
                (push_null_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector:@selector(parserFoundNull)]
                : 0;
            begin_array_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundArrayBegin)] ? 
                (begin_array_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundArrayBegin)]
                : 0;
            end_array_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundArrayEnd)] ? 
                (end_array_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundArrayEnd)]
                : 0;
            begin_object_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundObjectBegin)] ? 
                (begin_object_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundObjectBegin)]
                : 0;
            end_object_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundObjectEnd)] ? 
                (end_object_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundObjectEnd)]
                : 0;
            begin_value_at_index_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundValueBeginAtIndex:)] ? 
                (begin_value_at_index_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundValueBeginAtIndex:)]
                : 0;
            end_value_at_index_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundValueEndAtIndex:)] ? 
                (end_value_at_index_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundValueEndAtIndex:)]
                : 0;
            begin_key_value_pair_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundKeyValuePairBeginWithKey:length:encoding:index:)] ? 
                (begin_key_value_pair_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundKeyValuePairBeginWithKey:length:encoding:index:)]
                : 0;
            end_key_value_pair_imp_ = [this->delegate_ respondsToSelector:@selector(parserFoundKeyValuePairEnd)] ? 
                (end_key_value_pair_imp_func_ptr_t)[(NSObject*)this->delegate_ methodForSelector: @selector(parserFoundKeyValuePairEnd)]
                : 0;

            base::parse_begin_imp();
        }
        
        virtual void parse_end_imp() {
            base::parse_end_imp();
        }
        
        virtual void finished_imp() {
            base::finished_imp();
        }
        
        
        virtual void begin_array_imp() {
            if (begin_array_imp_) {
                begin_array_imp_(this->delegate_, @selector(parserFoundArrayBegin));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_array_imp() {
            if (end_array_imp_) {
                end_array_imp_(this->delegate_, @selector(parserFoundArrayEnd));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        

        virtual void begin_object_imp() {
            if (begin_object_imp_) {
                begin_object_imp_(this->delegate_, @selector(parserFoundObjectBegin));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual bool end_object_imp() {
            BOOL result = YES;
            if (end_object_imp_) {
                result = end_object_imp_(this->delegate_, @selector(parserFoundObjectEnd));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
            return result;
        }
        
        
        virtual void begin_value_at_index_imp(size_t index) {
            if (begin_value_at_index_imp_) {
                begin_value_at_index_imp_(this->delegate_, @selector(parserFoundValueBeginAtIndex:), index);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_value_at_index_imp(size_t index) {
            if (end_value_at_index_imp_) {
                end_value_at_index_imp_(this->delegate_, @selector(parserFoundValueEndAtIndex:), index);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void begin_key_value_pair_imp(const const_buffer_t& buffer, size_t nth) {
            if (begin_key_value_pair_imp_) {
                begin_key_value_pair_imp_(this->delegate_, @selector(parserFoundKeyValuePairBeginWithKey:length:encoding:index:), 
                                          buffer.first, buffer.second*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value, nth);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void end_key_value_pair_imp() {
            if (end_key_value_pair_imp_) {
                end_key_value_pair_imp_(this->delegate_, @selector(parserFoundKeyValuePairEnd));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
                
        virtual void value_string_imp(const const_buffer_t& buffer, bool hasMore) { 
            if (write_string_imp_) {
                write_string_imp_(this->delegate_, @selector(parserFoundString:length:hasMore:encoding:), 
                                 static_cast<const void*>(buffer.first), buffer.second*sizeof(char_t), 
                                  (BOOL)hasMore, json::ns_unicode_encoding_traits<EncodingT>::value);
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        
//        virtual void push_key_imp(const char_t* s, std::size_t len) {
//            if (push_key_imp_) {
//                push_key_imp_(this->delegate_, @selector(parserFoundKey:length:encoding:),  
//                    static_cast<const void*>(s), len*sizeof(char_t), json::ns_unicode_encoding_traits<EncodingT>::value);
//                if (!this->ok()) {
//                    throw SemanticActionsStateError();
//                }
//            }
//        }
        
        virtual void value_number_imp(const number_desc_t& number) {
            if (push_number_imp_) {
                push_number_imp_(this->delegate_, @selector(parserFoundNumber:length:), number.c_str(), number.c_str_len());
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void value_null_imp() {
            if (push_null_imp_) {
                push_null_imp_(this->delegate_, @selector(parserFoundNull));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        virtual void value_boolean_imp(bool b) {
            if (push_boolean_imp_) {
                push_boolean_imp_(this->delegate_, @selector(parserFoundBoolean:), static_cast<BOOL>(b));
                if (!this->ok()) {
                    throw SemanticActionsStateError();
                }
            }
        }
        
        
/*        
        virtual void pop_imp() {
            base::pop_imp();
        }
        
        virtual void clear_imp(bool shrink_buffers) {
            base::clear_imp(shrink_buffers);
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
        write_string_imp_func_ptr_t         write_string_imp_;        
        //push_string_imp_func_ptr_t          push_string_imp_;        
        //push_key_imp_func_ptr_t             push_key_imp_;
        push_number_imp_func_ptr_t          push_number_imp_;
        push_boolean_imp_func_ptr_t         push_boolean_imp_;
        push_null_imp_func_ptr_t            push_null_imp_;
        begin_array_imp_func_ptr_t          begin_array_imp_;
        end_array_imp_func_ptr_t            end_array_imp_;
        begin_object_imp_func_ptr_t         begin_object_imp_;
        end_object_imp_func_ptr_t           end_object_imp_;
        begin_value_at_index_imp_func_ptr_t begin_value_at_index_imp_;
        end_value_at_index_imp_func_ptr_t   end_value_at_index_imp_;
        begin_key_value_pair_imp_func_ptr_t begin_key_value_pair_imp_;
        end_key_value_pair_imp_func_ptr_t   end_key_value_pair_imp_;

        
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
 
    //
    // JSON Containers
    // 
    - (void) parserFoundArrayBegin {}
    - (void) parserFoundArrayEnd {}

    - (void) parserFoundObjectBegin {}
    - (bool) parserFoundObjectEnd { return true; }


    //
    // Key-Value Pairs
    //
    - (void) parserFoundKeyValuePairBeginWithKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding {
    }
    - (void) parserFoundKeyValuePairEnd {
    }


    //
    // Array Elements
    //
    - (void) parserFoundValueBeginAtIndex:(size_t)index  {
    }
    - (void) parserFoundValueEndAtIndex:(size_t)index {
    }



    //
    // Primitive Values 
    // 
    - (void) parserFoundString:(const void*)bytes length:(size_t)length hasMore(BOOL):hasMore encoding:(NSStringEncoding)encoding {
    }

    - (void) parserFoundNumber:(const char*)numberString length:(size_t)length {
    }

    - (void) parserFoundBoolean:(BOOL)value {
    }

    - (void) parserFoundNull {
    }

*/
 
             

@end

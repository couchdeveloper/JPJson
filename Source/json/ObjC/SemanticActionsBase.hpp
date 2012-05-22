//
//  SemanticActionsBase.hpp
//  
//
//  Created by Andreas Grosam on 7/1/11.
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

#ifndef JSON_OBJC_SEMANTIC_ACTIONS_BASE_HPP
#define JSON_OBJC_SEMANTIC_ACTIONS_BASE_HPP 


#include "json/config.hpp"
#include <iostream>  // description
#include "json/parser/semantic_actions_base.hpp"
#import "JPSemanticActionsProtocol.h"
#import <Foundation/Foundation.h>



namespace json { namespace objc { 
    
    using json::semantic_actions_base;
    
#pragma mark - Class SemanticActionsBase

    //  Class `SemanticActionsBase` is the base class of all semantic actions 
    //  classes which is used as the 'implementation' of an Objective-C semantic 
    //  actions class. An instance of a subclass will be created by an Objective-C 
    //  semantic actions object which itself is a subclass of JPSemanticActionsBase. 
    //  The creater sets itself as the "delegate" for an instance of the SemanticActionsBase. 
    //
    //  The purpose of the SemanticActionsBase class is to provide a common
    //  interface for the internal parser implemented in pure C++ and the 
    //  semantic actions class implemented in Objective-C. Class SemanticActionsBase
    //  is itself a C++ class but has a member variable `delegate_` which is an
    //  Objective-C object pointer defined as `id<JPSemanticActionsProtocol>`.
    //
    //  The internal parser uses this exposed API and calls the virtual member 
    //  functions - aka "parse events" - for its semantic actions object (a C++ 
    //  instance). The  SemanticActionsBase instance maps these "parse events" 
    //  to corresponding Objective-C messages which will be send to its delegate. 
    //  (This is in contrast to the pure C++ version, where semantic action classes 
    //  are template parameters and use the curiously recurring template pattern 
    //  (CRTP) to provide static polymorphism).
    //
    //  Thats is, "parse events" sent from the internal JSON parser will be
    //  forwarded as Objective-C messages via Objective-C dispatching to a 
    //  delegate object which eventually handles these "parse event".
    //
    //  However, class `SemanticActionsBase` implements only the *required* delegate 
    //  methods. This means, if other delegate methods - aka "parse events" must 
    //  be send to a delegate, class `SemanticActionsBase` must be subclassed in 
    //  order to implement this.
    //  
    // 
    //  
    //  Template parameter EncodingT specifies the encoding of the internal 
    //  string buffer encoding. Usually, this should be set to UTF-16 encoding 
    //  (platform endianess) which seems to yields the best performance in 
    //  conjunction with NSString/CFString objects and is defined at compile
    //  time when compiling the JPJson library. (The definition of the actual 
    //  encoding and concrete type of this class which is used in Objective-C 
    //  classes is found in file "JPSematicActionsbase_private.h")
    //
    
    template <typename EncodingT>
    class SemanticActionsBase : 
        public semantic_actions_base<SemanticActionsBase<EncodingT>, EncodingT>
    {
    private:     
        typedef semantic_actions_base<
            SemanticActionsBase<EncodingT> 
          , EncodingT
        >                                           base;
        

    public:    
        typedef typename base::error_t              error_t;
        typedef typename base::number_info_t        number_info_t;
        typedef typename base::char_t               char_t;     // char type of the StringBuffer
        typedef typename base::encoding_t           encoding_t;
        typedef id                                  result_type;
        
        typedef typename base::buffer_t             buffer_t;
        typedef typename base::const_buffer_t       const_buffer_t;

    public:    
        
#pragma mark - Public Members        
                
        SemanticActionsBase(id<JPSemanticActionsProtocol> delegate = nil) 
        : delegate_(delegate)
        {
        }
        
        virtual ~SemanticActionsBase() {}
                
        virtual void parse_begin_imp() {
            error_.reset();
            //error_str_.clear();
            [delegate_ parserFoundJsonBegin];
        }
        
        virtual void parse_end_imp() {
            [delegate_ parserFoundJsonEnd];
        }
        
        virtual void finished_imp() {
            [delegate_ parserFinished];
        }
        
        virtual void begin_array_imp() {}
        virtual void end_array_imp() {}
        
        virtual void begin_object_imp() {}
        virtual bool end_object_imp() {            
            bool duplicateKeyError = false;
            return not duplicateKeyError;                
        }

        virtual void begin_value_at_index_imp(size_t index) {}        
        virtual void end_value_at_index_imp(size_t) {}
        
        virtual void begin_key_value_pair_imp(const const_buffer_t& buffer, size_t nth) {}
        virtual void end_key_value_pair_imp() {}
                
        virtual void value_string_imp(const const_buffer_t& buffer, bool hasMore) {}        
        virtual void value_number_imp(const number_info_t& number) {}
        virtual void value_null_imp() {}
        virtual void value_boolean_imp(bool b) {}

        
        virtual void clear_imp() {
            error_.reset();
        } 
        
        virtual void print_imp(std::ostream& os) { 
            os << *this; 
        }

        virtual void error_imp(int code, const char* description) {
            error_.first = code;
            error_.second = description;
            [delegate_ parserDetectedError];
        }

        virtual const error_t& error_imp() const {
            return error_;
        }
        
        virtual result_type result() { return nil; }
        
    protected:    
        id<JPSemanticActionsProtocol>  delegate_;  
        
    private:    
        error_t         error_;

        
#pragma mark - Friend Stream Output Operator
    private:
        //    
        // Stream Output operator, defined as inline friend:    
        //  
        friend inline 
        std::ostream& 
        operator<< (std::ostream& os, const SemanticActionsBase& sa) 
        {
            typedef SemanticActionsBase::base base;
            os << static_cast<base const&>(sa);   
            return os;
        }
        
    };
    
    
    
}}   // namespace json::objc



#endif // JSON_OBJC_STREAMING_SEMANTIC_ACTIONS_HPP



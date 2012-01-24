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
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>  // description
#include <iomanip>   // description   
#include <limits>
#include <stdlib.h>  // numeric conversion
#include "json/parser/semantic_actions_base.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/utility/simple_log.hpp"
#include "json/utility/flags.hpp"
#include <boost/mpl/if.hpp>
#include <boost/static_assert.hpp>
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import "JPSemanticActionsProtocol.h"



namespace json { namespace objc { 
    
    using json::semantic_actions_base;
    
#pragma mark - Class SemanticActionsBase

    //  Class SemanticActionsBase is the (abstract) base class of all concrete 
    //  definitions of a semantic actions class. An instance of a subclass will 
    //  be created by an Objective-C semantic actions object which itself is a 
    //  subclass of JPSemanticActionsBase. The creater sets itself as the 
    //  "delegate" for this class. 
    //
    //  The purpose of the SemanticActionsBase class is to provide a common
    //  interface for a) the parser and b) the Objective-C semantic actions
    //  class. To accomplish this, the SemanticActionsBase class simply maps
    //  "parser events" to corresponding virtual member functions, which are
    //  - with a few excpetions - emtpy by default. (This is in contrast to 
    //  the pure C++ version, where semantic action classes are template para-
    //  meters and use the curiously recurring template pattern (CRTP) to 
    //  provide static polymorphism).
    //
    //  These virtual member functions eventually notify the parser events by
    //  sending the "delegate", that is the JPSemanticActions instance, the 
    //  corresponding message via Objective-C dispatching. Here, only the 
    //  *required* delegate methods will actually be forwarded to the Objective-
    //  C instance. Other delegate methods must be invoked by sub classes if
    //  required.
    //  
    // 
    //  The concrete C++ subclass is required to implement the virtual member
    //  functions accordingly. Most of the features of a Objective-C Semantic 
    //  Actions class is thereof implemented in the concrete definition of the 
    //  C++ semantic actions class.
    //  The public interface required to implement an Objective-C semantic
    //  actions class is kept minimal and C++ implementations can be reused in
    //  differnet Objective-C classes.
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
        typedef typename base::nb_number_t          nb_number_t;
        typedef typename base::char_t               char_t;     // char type of the StringBuffer
        typedef typename base::encoding_t           encoding_t;
        typedef id                                  result_type;

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
        
        virtual void begin_value_with_key_imp(const char_t* s, size_t len, size_t nth) {}
        virtual void end_value_with_key_imp(const char_t* s, size_t len, size_t nth) {}
                
        virtual void value_string_imp(const char_t* s, std::size_t len) {}                
        virtual void value_number_imp(const nb_number_t& number) {}
        virtual void value_null_imp() {}
        virtual void value_boolean_imp(bool b) {}

        
        virtual void clear_imp() {
            error_.reset();
            //error_str_.clear();
        } 
        
        virtual void print_imp(std::ostream& os) { 
            os << *this; 
        }

        virtual void error_imp(int code, const char* description) {
            error_.first = code;
            // make a copy of the error string
            //error_str_ = error.second;
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
        //std::string     error_str_;

        
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
            
            // Insert code ...
            
            return os;
        }
        
    };
    
    
    
}}   // namespace json::objc



#endif // JSON_OBJC_STREAMING_SEMANTIC_ACTIONS_HPP



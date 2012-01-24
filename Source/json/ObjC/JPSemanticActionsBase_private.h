//
//  JPSemanticActionsBase_private.hpp
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

#ifndef JSON_OBJC_JP_SEMANTIC_ACTIONS_BASE_PRIVATE_HPP
#define JSON_OBJC_JP_SEMANTIC_ACTIONS_BASE_PRIVATE_HPP


#include "json/config.hpp"
#import "json/ObjC/SemanticActionsBase.hpp"
#include "json/unicode/unicode_utilities.hpp"


//
//  Defines the encoding of the internal string buffer and the
//  the concrete type of the base class of the internal semantic 
//  actions class.
// 
#if 1 /* USE_UTF_16 */
typedef json::unicode::to_host_endianness<json::unicode::UTF_16_encoding_tag>::type JP_CFStringEncoding;
#else
typedef json::unicode::UTF_8_encoding_tag JP_CFStringEncoding;
#endif
typedef json::objc::SemanticActionsBase<JP_CFStringEncoding> SemanticActionsBase;


//
// Private Interface
//
@interface JPSemanticActionsBase ()

// Returns a pointer to an instance of a concrete subclass of SemanticActionsBase.
// This property shall be implemented in subclasses of JPSemanticActionsBase.
@property (readonly, nonatomic) SemanticActionsBase* imp;


@end


// This class is used to throw exceptions from C++ code when the Objective-C code has
// set an error state via setErrorCode:description. The error information shall be
// retrieved from the error sate property of the semantic actions instance.
__attribute__ ((visibility ("default")))
struct SemanticActionsStateError {};


#endif // JSON_OBJC_JP_SEMANTIC_ACTIONS_BASE_PRIVATE_HPP
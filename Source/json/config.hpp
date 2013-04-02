//
//  config.hpp
//  
//  Created by Andreas Grosam on 1/8/12.
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

#ifndef JSON_CONFIG_HPP
#define JSON_CONFIG_HPP


/**
 Endianness
 If JSON_SWAP_NO_BUILTIN is defined, an internal implementation will be chosen
 instead of a builtin or library function.
 */
//#define JSON_SWAP_NO_BUILTIN


/**
 If JSON_UTILITY_STRING_TO_NUMBER_USE_QI is defined, the implemenation uses
 boost::spirit::qi for string to number conversions. This is slightly faster
 that when using standard conversions defined in header <xlocale.h>. 
 */
#define JSON_UTILITY_STRING_TO_NUMBER_USE_QI


#define JSON_UTILITY_NUMBER_TO_STRING_USE_KARMA


/**
 JSON_PARSER_INTERNAL_STRING_STORAGE_MIN_CHUNK_SIZE specifies the minimum
 chunk size for partial strings. The semantic actions object receives partial
 data strings of this size. The actual size of the chunk may be possibly larger.
*/ 
#define JSON_PARSER_INTERNAL_STRING_STORAGE_MIN_CHUNK_SIZE (4*1024)


/**
 JSON_PARSER_INTERNAL_STRING_STORAGE_MAX_SIZE specifies the maximum size (in
 code units) for the internal buffer allocated for key and data strings. The
 maximum size may only be exceeded if a key string is larger than this. If this
 happens, the parser throws a runtime error.
 */ 
#define JSON_PARSER_INTERNAL_STRING_STORAGE_MAX_SIZE (32*1024)



#define JSON_SEMANTIC_ACTIONS_STRING_ENCODING_UTF_8      1
#define JSON_SEMANTIC_ACTIONS_STRING_ENCODING_UTF_16     2
/**
 SEMANTIC_ACTIONS_STRING_ENCODING specifies the encoding of the string buffer
 which a semantic actions object receives when the parser detects a JSON string.
 The encoding may be specied from the macros defined above.
 */ 
#define JSON_SEMANTIC_ACTIONS_STRING_ENCODING    JSON_SEMANTIC_ACTIONS_STRING_ENCODING_UTF_16



/**
 JSON Path - not yet implemented
*/
// #define JSON_USE_JSON_PATH  //currently not used



// json::Value implementation
#define USE_JSON_UTILITY_VARIANT


#endif   // JSON_CONFIG_HPP

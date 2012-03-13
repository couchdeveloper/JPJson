//
//  json_parse.mm
//
//  Created by Andreas Grosam on 6/17/11.
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

#warning deprecated file

#include "json/parser/parse.hpp"
#include "json/parser/semantic_actions_base.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/utility/buffer_queue_iterator.hpp"

#include <assert.h>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>

#import "json_parse.h"
#import "JPSemanticActions.h"
#import "JPSemanticActions_private.hpp"
#import <Foundation/Foundation.h>



namespace {
    
    
    NSError*  makeError(int errorCode, const char* errorStr)
    {
        NSString* errStr = [[NSString alloc] initWithUTF8String:errorStr];
        NSString* localizedErrStr = NSLocalizedString(errStr, errStr);
        [errStr release];
        NSArray* objectsArray = [[NSArray alloc] initWithObjects: localizedErrStr, nil];
        NSArray* keysArray = [[NSArray alloc] initWithObjects: NSLocalizedDescriptionKey, nil];            
        NSDictionary* userInfoDict = [[NSDictionary alloc] initWithObjects:objectsArray forKeys: keysArray];
        [objectsArray release];
        [keysArray release];        
        NSError* error = [NSError errorWithDomain:@"json parser" code:errorCode userInfo:userInfoDict];
        [userInfoDict release];
        return error;
    }
}


namespace {

    bool json_parse_UTF8_sa(const char* jsonString, size_t len, JPSemanticActions* sa)
    {
        if (sa == nil)
            return false;
        
        const char* first = jsonString;
        const char* last = first + len;
        
        bool success = false;
        try {        
            typedef const char* iterator;
            
            SemanticActions* sa_imp = [sa imp];
            success = json::parse(first, last, *sa_imp);
        }
        
        // When we get an exception here, those exceptions are really "unexpected".
        // In this case, the semantic actions' error member is most likely not set. 
        // We must also not pass through C++ exceptions into Objective-C context.
        // So, we catch here all execptions and set the error member of the 
        // semantic actions accordingly:
        
        catch (std::exception& ex) 
        {
            typedef SemanticActions::error_t error_t;
            SemanticActions* sa_imp = [sa imp];
            assert(sa_imp);
            if (sa_imp)
                sa_imp->error(error_t(json::JP_INTERNAL_RUNTIME_ERROR, ex.what()));
        }
        catch (...) {
            typedef SemanticActions::error_t error_t;
            SemanticActions* sa_imp = [sa imp];
            assert(sa_imp);
            if (sa_imp)
                sa_imp->error(error_t(json::JP_UNKNOWN_ERROR, json::parser_error_str(json::JP_UNKNOWN_ERROR)));
        }
        
        return success;    
    }


    


    bool json_parse_UTF8_validating(const char* jsonString, size_t len, NSError** error)
    {
        typedef const char* iterator;
        typedef json::unicode::iterator_encoding<iterator>::type  SourceEncoding;
        typedef json::semantic_actions_noop<SourceEncoding> SemanticActions_t;
        typedef SemanticActions_t::error_t sa_error_t;
            
        const char* first = jsonString;
        const char* last = first + len;
        SemanticActions_t sa;
        try {
            bool success = json::parse(first, last, sa);
            if (not success and error) {
                *error = makeError(sa.error().first, sa.error().second);
            }            
        }
        catch (std::exception& ex) {
            *error = makeError(1001, sa.error().second);
        }
        catch (...) {
            *error = makeError(1001, "unknown error");
        }

        return false;    
    }

}   // anonymous namespace 
    


bool json_parse_UTF8(const char* s, size_t len, JPSemanticActions* sa)
{
    if (s == NULL or len <= 0 or sa == nil)
        return false;
    bool result = json_parse_UTF8_sa(s, len, sa);
    return result;        
}


bool json_parse_string(NSString* jsonString, JPSemanticActions* sa)
{
    if (not sa)
        return false;
    
    size_t len = [jsonString lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    const char* first = [jsonString UTF8String];
    bool success = json_parse_UTF8_sa(first, len, sa);
    return success;    
}


bool json_validate_string(NSString* jsonString, NSError** error) 
{
    size_t len = [jsonString lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    const char* first = [jsonString UTF8String];
    bool success = json_parse_UTF8_validating(first, len, error);
    return success;
}



bool json_parse_NSData(NSData* data, JPSemanticActions* sa) 
{
    // Determine encoding:
    uint8_t const* first = static_cast<uint8_t const*>([data bytes]);
    uint8_t const* last = first + [data length];    
    int encoding = json::detect_encoding(first, last);    
    if (encoding <= 0) {
        switch (encoding) {
            case -1:
                [sa setErrorCode:json::JP_UNEXPECTED_END_ERROR description:NULL];
                break;
            default:
                [sa setErrorCode:json::JP_UNEXPECTED_END_ERROR description:NULL];
                break;
        }
        return false;
    }        
    bool success;
    switch (encoding) {
        case json::unicode::UNICODE_ENCODING_UTF_8: 
            success = json::parse(first, last, json::unicode::UTF_8_encoding_tag(), *[sa imp]);
            break;
        case json::unicode::UNICODE_ENCODING_UTF_16BE: {
            const uint16_t* first16 = reinterpret_cast<uint16_t const*>(first);
            const uint16_t* last16 = reinterpret_cast<uint16_t const*>(last);
            success = json::parse(first16, last16, json::unicode::UTF_16BE_encoding_tag(), *[sa imp]);
            break;
        }
        case json::unicode::UNICODE_ENCODING_UTF_16LE: {
            const uint16_t* first16 = reinterpret_cast<uint16_t const*>(first);
            const uint16_t* last16 = reinterpret_cast<uint16_t const*>(last);
            success = json::parse(first16, last16, json::unicode::UTF_16LE_encoding_tag(), *[sa imp]);
            break;
        }
        case json::unicode::UNICODE_ENCODING_UTF_32BE: {
            const uint32_t* first32 = reinterpret_cast<uint32_t const*>(first);
            const uint32_t* last32 = reinterpret_cast<uint32_t const*>(last);
            success = json::parse(first32, last32, json::unicode::UTF_32BE_encoding_tag(), *[sa imp]);
            break;
        }
        case json::unicode::UNICODE_ENCODING_UTF_32LE: {
            const uint32_t* first32 = reinterpret_cast<uint32_t const*>(first);
            const uint32_t* last32 = reinterpret_cast<uint32_t const*>(last);
            success = json::parse(first32, last32, json::unicode::UTF_32LE_encoding_tag(), *[sa imp]);
            break;
        }
        default:
            success = false;
            [sa setErrorCode:json::JP_ENCODING_NOT_SUPPORTED_ERROR description:NULL];
            return false;            
    }
    
    return success;
}



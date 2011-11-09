//
//  JsonPathSemanticActions.mm
//  FoundationExamples
//
//  Created by Andreas Grosam on 10/25/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "JsonPathSemanticActions.h"

#import "JPJson/JPJsonCommon.h"



// Note:
// This Objective-C class uses some private headers and a private
// C++ class "json_path" which can be found in the source package. 
// The reason for doing this is just simple: I'm too lazy to imple-
// ment an Objective-C class for a JSON path structure. ;)
// So, this implementation is some sort of hack. But also invaluable.
// if you want to hack into the implementation details yourself.

// Headers found in the source package:
#include "json/ObjC/JPSemanticActionsBase_private.h"
#include "json/ObjC/SemanticActionsBase.hpp"
#include "json/ObjC/JPSemanticActionsBase_private.h"
#include "json/ObjC/unicode_traits.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_conversions.hpp"
#include "json/parser/json_path.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>


@interface JsonPathSemanticActions () 
- (void) printBranch;
@end



typedef JP_CFStringEncoding::code_unit_type char_t;

@implementation JsonPathSemanticActions
{
@private
    json::json_internal::json_path<JP_CFStringEncoding>    json_path_;
    size_t level_;
    NSMutableData* jsonBranch_;
    NSStringEncoding encoding_;
}


//
// Designated Initializer
//
- (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue;
{
    self = [super initWithHandlerDispatchQueue:handlerQueue];
    if (self) {
        jsonBranch_ = [[NSMutableData alloc] initWithCapacity:1024];
        encoding_ = json::ns_unicode_encoding_traits<JP_CFStringEncoding>::value;
    }    
    return self;
}

- (void) dealloc 
{
    [jsonBranch_ release];
    [super dealloc];
}



#pragma mark -

- (void) parserFoundJsonString:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding 
{
    assert(encoding_ == encoding);
    char_t quote = static_cast<char_t>('"');
    [jsonBranch_ appendBytes:&quote length:sizeof(quote)];    
    [jsonBranch_ appendBytes:bytes length:length];
    [jsonBranch_ appendBytes:&quote length:sizeof(quote)];    
}


//- (void) parserFoundJsonKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding

- (void) parserFoundJsonNumber:(const char*)numberString length:(size_t)length
{
    char_t buffer[128];
    // We don't really need to elaborately convert the encoding like this:
    char_t* dest = buffer;
    int error;
    std::size_t count = json::unicode::convert_unsafe(numberString, numberString+length, json::unicode::UTF_8_encoding_tag(),
                                  dest, JP_CFStringEncoding(),
                                  error);
    // since a number string is pure ASCII the following would suffice:
    //std::copy(numberString, numberString+length, buffer);
    [jsonBranch_ appendBytes:buffer length:count*sizeof(char_t)];    
}

- (void) parserFoundJsonBoolean:(BOOL)value
{
    // Since "true" and "false" is true ASCII, we don't really need to elaborately convert the encoding:
    const char_t true_buffer[] = {'t', 'r', 'u', 'e'};
    const char_t false_buffer[] = {'f', 'a', 'l', 's', 'e'};
    const char_t* bytes = value ? true_buffer : false_buffer;
    [jsonBranch_ appendBytes:bytes length: (value ? sizeof(true_buffer) : sizeof(false_buffer))];    
}

- (void) parserFoundJsonNull {
    // Since "null" is true ASCII, we don't really need to elaborately convert the encoding:
    const char_t buffer[] = {'n', 'u', 'l', 'l'};
    [jsonBranch_ appendBytes:buffer length:sizeof(buffer)];    
}


- (void) parserFoundJsonArrayBegin {
    ++level_;
    char_t ch = static_cast<char_t>('[');
    [jsonBranch_ appendBytes:&ch length:sizeof(ch)];    
}

- (void) parserFoundJsonArrayEnd {
    --level_;
    char_t ch = static_cast<char_t>(']');
    [jsonBranch_ appendBytes:&ch length:sizeof(ch)];    
}

- (void) parserFoundJsonObjectBegin {
    ++level_;
    char_t ch = static_cast<char_t>('{');
    [jsonBranch_ appendBytes:&ch length:sizeof(ch)];    
}

- (bool) parserFoundJsonObjectEnd {
    --level_;
    return true;
    char_t ch = static_cast<char_t>('}');
    [jsonBranch_ appendBytes:&ch length:sizeof(ch)];    
}

- (void) parserFoundJsonValueBeginAtIndex:(size_t)index
{
    if (level_ == 1) {
        json_path_.push_index(index);
        printf("%s = ", json_path_.path<json::unicode::UTF_8_encoding_tag>().c_str());  
        [jsonBranch_ setLength:0];
    }
    else {
        if (index != 0) {
            char_t ch = static_cast<char_t>(',');
            [jsonBranch_ appendBytes:&ch length:sizeof(ch)];    
        }
    }
}

- (void) parserFoundJsonValueEndAtIndex:(size_t)index
{
    if (level_ == 1) {
        json_path_.pop_component();
        // Print the json branch to the console - and convert it to UTF-8:
        [self printBranch];
        std::cout << "\n" << std::endl;
    }    
}

- (void) parserFoundJsonValueBeginWithKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding index:(size_t)index
{
    assert(encoding_ == encoding);
    typedef JP_CFStringEncoding::code_unit_type char_t;
    
    if (level_ == 1) {    
        const char_t* s = reinterpret_cast<char_t const*>(bytes);
        const size_t len = length / sizeof(char_t);
        json_path_.push_key(s, s+len);
        printf("%s = ", json_path_.path<json::unicode::UTF_8_encoding_tag>().c_str());  
        [jsonBranch_ setLength:0];
    }
    else {
        if (index != 0) {
            char_t ch = static_cast<char_t>(',');
            [jsonBranch_ appendBytes:&ch length:sizeof(ch)];    
        }
        char_t quote = static_cast<char_t>('"');
        [jsonBranch_ appendBytes:&quote length:sizeof(quote)];    
        [jsonBranch_ appendBytes:bytes length:length];
        [jsonBranch_ appendBytes:&quote length:sizeof(quote)];    
        char_t ch = static_cast<char_t>(':');
        [jsonBranch_ appendBytes:&ch length:sizeof(ch)];            
    }
}

- (void) parserFoundJsonValueEndWithKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding index:(size_t)index
{
    if (level_ == 1) {
        json_path_.pop_component();
        // Print the json branch to the console - and convert it to UTF-8:
        [self printBranch];
        std::cout << "\n" << std::endl;
    }
}



#pragma mark -
// Private

// Convert content of jsonBranch_ from JP_CFStringEncoding to UTF-8 and print
// it to the console
- (void) printBranch 
{
    std::ostream_iterator<char> out_it(std::cout);    
    char_t const* first = static_cast<char_t const*>([jsonBranch_ bytes]);
    char_t const* last = first + [jsonBranch_ length]/sizeof(char_t);
    int error;
    json::unicode::convert_unsafe(first, last, JP_CFStringEncoding(),
                                  out_it, json::unicode::UTF_8_encoding_tag(),
                                  error);
}

@end
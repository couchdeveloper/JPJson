//
//  JPJsonWriter.mm
//
//  Created by Andreas Grosam on 7/20/11.
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

#import "JPJsonWriter.h"
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

#include "json/generator/generate.hpp"
#include "json/unicode/unicode_conversions.hpp"
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/type_traits.hpp>
#include <boost/iterator/iterator_traits.hpp>



namespace {
    
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;    
    using json::generator_internal::copyBOM;
    using json::generator_internal::escape_convert_unsafe;
    
    
    
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
        NSError* error = [NSError errorWithDomain:@"JPJsonWriter" code:errorCode userInfo:userInfoDict];
        [userInfoDict release];
        return error;
    }
    
    
    // Insert a BOM into the buffer for the specified encoding.
    //
    // On success, returns the number of bytes written. Otherwise a negative number 
    // indicating the error.
    //
    // Errors: 
    //  Buffer Is NULL:     -1,
    //  Buffer Too Small:   -2,
    //  Invalid Encoding:   -3
    //  
    int insertBOMIntoBuffer(void* buffer, size_t bufferSize, JPUnicodeEncoding encoding)
    {
        if (!buffer)
            return -1;
        int result = -4;
        switch (encoding) {
            case JPUnicodeEncoding_UTF8: 
                if (bufferSize < UTF_8_encoding_tag::bom_byte_size) {
                    result = -2;
                }
                else {
                    UTF_8_encoding_tag::code_unit_type* dest = static_cast<UTF_8_encoding_tag::code_unit_type*>(buffer);
                    copyBOM(dest, UTF_8_encoding_tag());
                    result = UTF_8_encoding_tag::bom_byte_size;
                }
                break;
            case JPUnicodeEncoding_UTF16BE:
                if (bufferSize < UTF_16BE_encoding_tag::bom_byte_size) {
                    result = -2;
                } else {
                    UTF_16BE_encoding_tag::code_unit_type* dest = static_cast<UTF_16BE_encoding_tag::code_unit_type*>(buffer);
                    copyBOM(dest, UTF_16BE_encoding_tag());
                    result =  UTF_16BE_encoding_tag::bom_byte_size;
                }
                break;
            case JPUnicodeEncoding_UTF16LE: 
                if (bufferSize < UTF_16LE_encoding_tag::bom_byte_size) {
                    return -2;       
                } else {
                    UTF_16LE_encoding_tag::code_unit_type* dest = static_cast<UTF_16LE_encoding_tag::code_unit_type*>(buffer);
                    copyBOM(dest, UTF_16LE_encoding_tag());
                    result = UTF_16LE_encoding_tag::bom_byte_size;
                }
                break;
            case JPUnicodeEncoding_UTF32BE:
                if (bufferSize < UTF_32BE_encoding_tag::bom_byte_size) {
                    result = -2;
                }
                else {
                    UTF_32BE_encoding_tag::code_unit_type* dest = static_cast<UTF_32BE_encoding_tag::code_unit_type*>(buffer);
                    copyBOM(dest, UTF_32BE_encoding_tag());
                    result = UTF_32BE_encoding_tag::bom_byte_size;
                }
                break;
            case JPUnicodeEncoding_UTF32LE:
                if (bufferSize < UTF_32LE_encoding_tag::bom_byte_size) {
                    result = -2;
                } else {
                    UTF_32LE_encoding_tag::code_unit_type* dest = static_cast<UTF_32LE_encoding_tag::code_unit_type*>(buffer);
                    copyBOM(dest, UTF_32LE_encoding_tag());
                    result = UTF_32LE_encoding_tag::bom_byte_size;
                }
                break;
            default: 
                result = -3;
        }
        return result;
    }


    // TODO: replace number to string conversions with karma, which should be
    // much faster than snprintf or Cocoa's methods.
    // Returns zero on success, otherwise a negative number indicating the error.
    int insertNumberIntoBuffer(CFNumberRef number, void* buffer, size_t bufferSize, 
                               size_t* outCount, JPUnicodeEncoding encoding)
    {
        static CFNumberFormatterRef s_numberFormatter;
        
        assert(encoding == JPUnicodeEncoding_UTF8);
        
        size_t count = 0;      // the number of bytes in buffer
        
        CFNumberType numberType = CFNumberGetType(number);    
        Boolean conversionSucceeded = false;
        switch (numberType) {
            case kCFNumberSInt8Type:
            case kCFNumberSInt16Type:
            case kCFNumberSInt32Type:
            case kCFNumberCharType:
            case kCFNumberShortType:
            case kCFNumberIntType: {
                int result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberIntType, &result);
                count = snprintf(static_cast<char*>(buffer), bufferSize, "%d", result);
            }
                break;
                
            case kCFNumberLongType: {
                long result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberLongType, &result);
                count = snprintf(static_cast<char*>(buffer), bufferSize, "%ld", result);
            }
                break;
                
            case kCFNumberSInt64Type:
            case kCFNumberLongLongType: {
                long long result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberLongLongType, &result);
                count = snprintf(static_cast<char*>(buffer), bufferSize, "%lld", result);
            }
                break;
                
            case kCFNumberFloat32Type:    
            case kCFNumberFloatType: {
                float result;
                int digits = FLT_DIG;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberFloatType, &result);
                count = snprintf(static_cast<char*>(buffer), bufferSize, "%#.*e",digits-1, result);
            }
                break;
                
            case kCFNumberFloat64Type:
            case kCFNumberDoubleType:{
                assert(sizeof(double) == 8);
                double result;
                int digits = DBL_DIG;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberDoubleType, &result);
                count = snprintf(static_cast<char*>(buffer), bufferSize, "%#.*e", digits-1, result);
            }
                break;
                
            default:
                break;
        }
        if (!conversionSucceeded) {
            if (s_numberFormatter == NULL) {
                CFLocaleRef locale = CFLocaleCreate(kCFAllocatorDefault, CFSTR("en_US_POSIX"));
                s_numberFormatter = CFNumberFormatterCreate(NULL, locale, kCFNumberFormatterDecimalStyle);    
                CFRelease(locale);
            }        
            CFStringRef numberString = CFNumberFormatterCreateStringWithNumber(kCFAllocatorDefault, s_numberFormatter, number);
            CFIndex usedBufLen;
#if defined (DEBUG)            
            CFIndex numConverted = 
#endif            
            CFStringGetBytes(numberString, CFRangeMake(0, CFStringGetLength(numberString)),
                                                    kCFStringEncodingUTF8, '?', FALSE,
                                                    static_cast<UInt8*>(buffer), bufferSize, &usedBufLen);
            assert(numConverted == CFStringGetLength(numberString));
            CFRelease(numberString);
            count = usedBufLen;        
        } 
        
        // TODO: Convert the buffer to the target encoding
        
        if (outCount) {
            *outCount = count;
        }
        
        return 0;
    }
    
    
    
    // Escape a string as required for a JSON string and encode it with
    // the specified Unicode encoding.
    // Returns the number of inserted bytes into NSData.
    size_t escapeStringAndInsertIntoData(NSString* string, NSMutableData* data, 
                                         JPUnicodeEncoding outputEncoding,
                                         JPJsonWriterOptions options)
    {
        typedef json::unicode::UTF_8_encoding_tag   source_encoding_t;

        typedef json::unicode::UTF_8_encoding_tag   dest_encoding_t;        
        typedef dest_encoding_t::code_unit_type     code_unit_t;
        typedef std::vector<code_unit_t>            buffer_t;
        typedef buffer_t::iterator                  iterator;
        
        static buffer_t s_buffer;        
        
        assert(outputEncoding == JPUnicodeEncoding_UTF8);
        
        const char* first = [string cStringUsingEncoding:NSUTF8StringEncoding]; // use source_encoding_t 
        NSUInteger length = [string lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        if (length > 0) {
            const char* last = first + length;        
            s_buffer.reserve(length + 32);
            s_buffer.clear();
            std::back_insert_iterator<buffer_t> back_it(s_buffer);
            
            int error;
            size_t count = escape_convert_unsafe(first, last, source_encoding_t(),
                                                 back_it, dest_encoding_t(), 
                                                 ((options & JPJsonWriterEscapeSolidus)!=0),
                                                 error);   
            // TODO: check error!
            assert(count > 0 and error == 0);
            const void* buffer = &s_buffer[0];
            [data appendBytes:buffer length:(count * sizeof(code_unit_t))];
            return count * sizeof(code_unit_t);
        }
        else {
            return 0;
        }        
    }
    
    
}  // namesapace


// Various Categories 

// -----------------------------------------------------------------------------
//  Category NSArray
// -----------------------------------------------------------------------------
@interface NSArray (JPJsonWriter) 
@end

@implementation NSArray (JPJsonWriter)

- (int) JPJsonWriter_insertIntoData:(NSMutableData*)data 
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);

    char buffer[32];
    char* p = buffer;

    const NSUInteger count = [self count];
    *p++ = '[';
    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        *p++ = '\n';
        NSUInteger indent = std::min(NSUInteger(8), level+1);
        while (indent--) {
            *p++ = '\t';
        }
    }    
    [data appendBytes:buffer length:(p-buffer)];
    
    NSUInteger i = count;
    for (id value in self) {
        int result = [value JPJsonWriter_insertIntoData:data encoding:encoding 
                                                options:options level:level + 1];
        if (result < 0) {
            return result;
        }
        if (--i) {
            p = buffer;
            *p++ = ',';
            if ((options & JPJsonWriterPrettyPrint) != 0) {
                *p++ = '\n';
                NSUInteger indent = std::min(NSUInteger(8), level+1);
                while (indent--) {
                    *p++ = '\t';
                }
            }
            [data appendBytes:buffer length:(p-buffer)];
        }
    }
    
    p = buffer;    
    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        *p++ = '\n';
        NSUInteger indent = std::min(NSUInteger(8), level);
        while (indent--) {
            *p++ = '\t';
        }
    }
    *p++ = ']';    
    [data appendBytes:buffer length:(p-buffer)];
    
    return 0;
}

@end



// -----------------------------------------------------------------------------
//  Category NSDictionary
// -----------------------------------------------------------------------------
@interface NSDictionary (JPJsonWriter) 
@end

@implementation NSDictionary (JPJsonWriter)
- (int) JPJsonWriter_insertIntoData:(NSMutableData*)data
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);
    
    const NSUInteger count = [self count];
    
    char buffer[32];
    char* p = buffer;    
    *p++ = '{';
    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        *p++ = '\n';
        NSUInteger indent = std::min(NSUInteger(8), level+1);
        while (indent--) {
            *p++ = '\t';
        }
    }
    [data appendBytes:buffer length:(p-buffer)];

    id o;
    if ((options & JPJsonWriterSortKeys) != 0) {    
        o = [[self allKeys] sortedArrayUsingSelector:@selector(compare:)];
    }
    else {
        o = self;
    }
    NSUInteger i = count;
    for (id key in o) {
        [key JPJsonWriter_insertIntoData:data encoding:encoding options:options 
                                   level:level + 1];
        
        p = buffer;
        if ((options & JPJsonWriterPrettyPrint) != 0) {
            *p++ = ' ';
        }
        *p++ = ':';        
        if ((options & JPJsonWriterPrettyPrint) != 0) {
            *p++ = ' ';
        }
        [data appendBytes:buffer length:(p-buffer)];
        // TODO: check if blocks give a performance advantage
        [[self objectForKey:key] JPJsonWriter_insertIntoData:data encoding:encoding 
                                                     options:options level:level + 1];
        if (--i) {
            p = buffer;
            *p++ = ',';
            if ((options & JPJsonWriterPrettyPrint) != 0) {
                *p++ = '\n';
                NSUInteger indent = std::min(NSUInteger(8), level+1);
                while (indent--) {
                    *p++ = '\t';
                }
            }
            [data appendBytes:buffer length:(p-buffer)];
        }
    }    
    p = buffer;    
    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        *p++ = '\n';
        NSUInteger indent = std::min(NSUInteger(8), level);
        while (indent--) {
            *p++ = '\t';
        }
    }
    *p++ = '}';    
    [data appendBytes:buffer length:(p-buffer)];
    
    return 0;
}

@end


// -----------------------------------------------------------------------------
//  Category NSString
// -----------------------------------------------------------------------------
@interface NSString (JPJsonWriter) 
@end

@implementation  NSString (JPJsonWriter) 
- (int) JPJsonWriter_insertIntoData:(NSMutableData*)data 
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);  
    
    char buffer[32];
    char* p = buffer;    
    *p++ = '"';
    [data appendBytes:buffer length:(p-buffer)];
    escapeStringAndInsertIntoData(self, data, encoding, options);
    p = buffer;
    *p++ = '"';
    [data appendBytes:buffer length:(p-buffer)];
    return 0;
}

@end



// -----------------------------------------------------------------------------
//  Category NSNumber
// -----------------------------------------------------------------------------
@interface NSNumber (JPJsonWriter) 
@end

@implementation NSNumber (JPJsonWriter) 
- (int) JPJsonWriter_insertIntoData:(NSMutableData*)data
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);  
    
    char buffer[256];
    char* p = buffer;    
    if ((CFTypeRef)self == kCFBooleanTrue) {
        memcpy(p, "true", 4);
        p += 4;
    }
    else if ((CFTypeRef)self == kCFBooleanFalse) {
        memcpy(p, "false", 5);
        p += 5;
    }
    else {
        size_t count;
        insertNumberIntoBuffer(CFNumberRef(self), p, (sizeof(buffer) - (p-buffer)), 
                               &count, encoding);
        p += count;
    }
    
    [data appendBytes:buffer length:(p-buffer)];    
    
    return 0;
}

@end


// -----------------------------------------------------------------------------
//  Category NSDecimalNumber
// -----------------------------------------------------------------------------
@interface NSDecimalNumber (JPJsonWriter) 
@end

@implementation NSDecimalNumber (JPJsonWriter) 
- (int) JPJsonWriter_insertIntoData:(NSMutableData*)data
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8); 
    
    NSString* numberString = [self descriptionWithLocale:nil];
    const char* s = [numberString cStringUsingEncoding:NSUTF8StringEncoding];
    NSUInteger length = [numberString lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    assert(s);
    [data appendBytes:s length:length];
    
    return 0;
}

@end


// -----------------------------------------------------------------------------
//  Category NSNull
// -----------------------------------------------------------------------------
@interface NSNull (JPJsonWriter) 
@end

@implementation NSNull (JPJsonWriter) 

- (int) JPJsonWriter_insertIntoData:(NSMutableData*)data
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);    
    char buffer[64];
    char* p = buffer;    
    memcpy(p, "null", 4);
    p += 4;
    [data appendBytes:buffer length:(p-buffer)];
    
    return 0;
}

@end






@implementation JPJsonWriter

- (id)init
{
    self = [super init];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}



+ (NSData*)dataWithObject:(id)object 
                 encoding:(JPUnicodeEncoding)encoding
                  options:(JPJsonWriterOptions)options 
                    error:(NSError**)error
{
    if (object == nil) {
        if (error) {
            *error = makeError(1, "Parameter error: 'object' equals nil");
        }
        return nil;
    }
    if(![object respondsToSelector:@selector(JPJsonWriter_insertIntoData:encoding:options:level:)]) {
        if (error) {
            *error = makeError(2, "Parameter error: 'object' is not a JSON object");
        }
        return nil;
    }
    switch (encoding) {
        case JPUnicodeEncoding_UTF8: 
            break;
        case JPUnicodeEncoding_UTF16BE:
        case JPUnicodeEncoding_UTF16LE: 
        case JPUnicodeEncoding_UTF32BE:
        case JPUnicodeEncoding_UTF32LE:
            if (error) {
                *error = makeError(6, "Encoding not suported in this version");
            }
            return nil;
            break;
        default: 
            if (error) {
                *error = makeError(3, "Parameter error: invalid encoding specified");
            }
            return nil;
    }
        
    NSMutableData* data = [[[NSMutableData alloc] init] autorelease];
    if ((options & JPJsonWriterWriteBOM) != 0) {
        char buffer[4];
        int result = insertBOMIntoBuffer(buffer, sizeof(buffer), encoding);
        if (!result) {
            if (error) {
                *error = makeError(4, "Unknown error occured");
            }
            return nil;
        } else {
            [data appendBytes:buffer length:result];
        }     
    }
    int result = [object JPJsonWriter_insertIntoData:data encoding:encoding options:options level:0];
    if (result < 0) {
        if (error) {
            *error = makeError(5, "Could not serialize JSON into NSData object");
        }
        return nil;
    }
    return data;
}


@end

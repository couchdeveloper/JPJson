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


#include "json/unicode/unicode_traits.hpp"
#include "json/unicode/unicode_converter.hpp"
#include "json/unicode/unicode_errors.hpp"
#include "json/ObjC/unicode_traits.hpp"
#include "json/generator/generate.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

#include <boost/type_traits.hpp>
#include <boost/iterator/iterator_traits.hpp>

#import "JPJsonWriter.h"
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>



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
    
    using json::unicode::to_host_endianness;
    using json::unicode::encoding_traits;
    
    
    //
    //  Maps NSStringEncoding constants to json::unicode encoding tags
    //
    template <int NSStringEncoding>
    struct NSStringEncodingToJsonUnicodeEncoding {};
    
    template <>
    struct NSStringEncodingToJsonUnicodeEncoding<NSUTF8StringEncoding> {
        typedef typename json::unicode::UTF_8_encoding_tag type;
    };
    
    template <>
    struct NSStringEncodingToJsonUnicodeEncoding<NSUTF16BigEndianStringEncoding> {
        typedef typename json::unicode::UTF_16BE_encoding_tag type;
    };
    
    template <>
    struct NSStringEncodingToJsonUnicodeEncoding<NSUTF16LittleEndianStringEncoding> {
        typedef typename json::unicode::UTF_16LE_encoding_tag type;
    };
    
    template <>
    struct NSStringEncodingToJsonUnicodeEncoding<NSUTF32BigEndianStringEncoding> {
        typedef typename json::unicode::UTF_32BE_encoding_tag type;
    };
    
    template <>
    struct NSStringEncodingToJsonUnicodeEncoding<NSUTF32LittleEndianStringEncoding> {
        typedef typename json::unicode::UTF_32LE_encoding_tag type;
    };

    
    struct NSMutableDataPushbackBuffer 
    {
        NSMutableDataPushbackBuffer(NSMutableData* data) 
        : data_(data), p_(0), start_(0), last_(0)
        {
            assert(data != nil);
            assert([data length] == 0);            
        }
        
        size_t size() const { return p_ - start_; } 
        size_t capacity() const { return last_ - start_; } 
        
        template <typename T>
        void push_back(const T& v) {
            if (std::distance(p_, last_) < sizeof(T)) {
                [data_ increaseLengthBy:1024];
                uint8_t* tmp = static_cast<uint8_t*>([data_ mutableBytes]);
                p_ = tmp + size();
                last_ = tmp + capacity() + 1024;                
                start_ = tmp;
            }
            *static_cast<T*>(static_cast<void*>(p_)) = v;
            p_ += sizeof(T);
        }
        
        void write(const char* p, size_t n) {
            if (std::distance(p_, last_) < sizeof(char)*n) {
                size_t delta = 1024 + (sizeof(char)*n)/1024;
                [data_ increaseLengthBy:delta];
                uint8_t* tmp = static_cast<uint8_t*>([data_ mutableBytes]);
                p_ = tmp + size();
                last_ = tmp + capacity() + delta;                
                start_ = tmp;
            }
            memcpy(p_, p, n);
            p_ += n;            
        }
        
        
        
        NSMutableData*  data_;        
        uint8_t*        p_;
        uint8_t*        start_;
        uint8_t*        last_;
    };
    
    
    
    //
    //  Maps JPUnicodeEncoding encoding constants to NSStringEncoding constants
    //
    
    inline  NSStringEncoding
    JPUnicodeEncodingToNSStringEncoding(JPUnicodeEncoding encoding) 
    {
        switch (encoding) {
            case JPUnicodeEncoding_UTF8: return NSUTF8StringEncoding;
            case JPUnicodeEncoding_UTF16BE: return NSUTF16BigEndianStringEncoding;
            case JPUnicodeEncoding_UTF16LE: return NSUTF16LittleEndianStringEncoding;
            case JPUnicodeEncoding_UTF32BE: return NSUTF32BigEndianStringEncoding;
            case JPUnicodeEncoding_UTF32LE: return NSUTF32LittleEndianStringEncoding;
            default: return -1;
        }
    };
    
}

namespace std {
    
    
    template <>
    class back_insert_iterator<NSMutableDataPushbackBuffer> 
    : public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    protected:
        NSMutableDataPushbackBuffer& buffer_;
        
    public:
        explicit back_insert_iterator(NSMutableDataPushbackBuffer& buffer) 
        : buffer_(buffer) 
        {
        }
        template <typename T>
        back_insert_iterator& operator= (T const& value)
        { 
            buffer_.push_back(value); return *this; 
        }
        back_insert_iterator& operator* ()
        { return *this; }
        back_insert_iterator& operator++ ()
        { return *this; }
        back_insert_iterator operator++ (int)
        { return *this; }
    };
    
    
}
    
    
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
    //  Invalid Encoding:   -3
    //  
    int insertBOMIntoBuffer(NSMutableDataPushbackBuffer& buffer, JPUnicodeEncoding encoding)
    {
        int result;
        switch (encoding) {
            case JPUnicodeEncoding_UTF8: 
                    copyBOM(std::back_inserter(buffer), UTF_8_encoding_tag());
                    result = encoding_traits<UTF_8_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF16BE:
                    copyBOM(std::back_inserter(buffer), UTF_16BE_encoding_tag());
                    result =  encoding_traits<UTF_16BE_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF16LE: 
                    copyBOM(std::back_inserter(buffer), UTF_16LE_encoding_tag());
                    result = encoding_traits<UTF_16LE_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF32BE:
                    copyBOM(std::back_inserter(buffer), UTF_32BE_encoding_tag());
                    result = encoding_traits<UTF_32BE_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF32LE:
                    copyBOM(std::back_inserter(buffer), UTF_32LE_encoding_tag());
                    result = encoding_traits<UTF_32LE_encoding_tag>::bom_byte_size;
                break;
            default: 
                result = -3;
        }
        return result;
    }


    // TODO: replace number to string conversions with karma, which should be
    // much faster than snprintf or Cocoa's methods.
    // Returns zero on success, otherwise a negative number indicating the error.
    int insertNumberIntoBuffer(CFNumberRef number, NSMutableDataPushbackBuffer& buffer, 
                               size_t* outCount, JPUnicodeEncoding encoding)
    {
        static CFNumberFormatterRef s_numberFormatter;
        
        assert(encoding == JPUnicodeEncoding_UTF8);
        
        char tmp_buffer[128];
        size_t count = 0;      // the number of bytes in tmp_buffer
        
        
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
                count = snprintf(tmp_buffer, sizeof(tmp_buffer), "%d", result);
            }
                break;
                
            case kCFNumberLongType: {
                long result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberLongType, &result);
                count = snprintf(tmp_buffer, sizeof(tmp_buffer), "%ld", result);
            }
                break;
                
            case kCFNumberSInt64Type:
            case kCFNumberLongLongType: {
                long long result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberLongLongType, &result);
                count = snprintf(tmp_buffer, sizeof(tmp_buffer), "%lld", result);
            }
                break;
                
            case kCFNumberFloat32Type:    
            case kCFNumberFloatType: {
                float result;
                int digits = FLT_DIG;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberFloatType, &result);
                count = snprintf(tmp_buffer, sizeof(tmp_buffer), "%#.*e",digits-1, result);
            }
                break;
                
            case kCFNumberFloat64Type:
            case kCFNumberDoubleType:{
                assert(sizeof(double) == 8);
                double result;
                int digits = DBL_DIG;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberDoubleType, &result);
                count = snprintf(tmp_buffer, sizeof(tmp_buffer), "%#.*e", digits-1, result);
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
                                                    static_cast<UInt8*>(static_cast<void*>(tmp_buffer)), sizeof(tmp_buffer), &usedBufLen);
            assert(numConverted == CFStringGetLength(numberString));
            CFRelease(numberString);
            count = usedBufLen;        
        } 
        
        // TODO: Convert the buffer to the target encoding        
        std::copy(tmp_buffer, tmp_buffer+count, std::back_inserter(buffer));
        
        if (outCount) {
            *outCount = count;
        }
        
        return 0;
    }
    
    
    //
    //  The input sequence [start, end) will be interpreted as an Unicode character
    //  sequence in encoding 'inEncoding' and properly escaped for a JSON string
    //  and converted to the encoding 'outEncoding' and copied into output iterator
    //  'dest'.
    //
    //  Parameter 'escapeSolidus' specifies whether the escape solidus '/' shall be
    //  escaped in the JSON string.
    //
    //  Returns unicode::NO_ERROR on success, otherwise a negative integer indicating 
    //  an error. The return value are error codes as decribed by json::unicode::ErrorT 
    //  or error codes as described below:
    //
    // Errors:
    //  Any json::unicode::ErrorT value, and
    //  json::unicode::E_UNKNOWN_ERROR-1:   Input Encoding not yet implemented or invalid.
    //  json::unicode::E_UNKNOWN_ERROR-2:   Output Encoding not yet implemented or invalid.
    //
    template <typename OutIterator>
    inline int
    escape_convert(
                   const void*      start, 
                   const void*      end, 
                   NSStringEncoding inEncoding,
                   OutIterator      dest,
                   NSStringEncoding outEncoding,
                   bool             escapeSolidus)
    {
        switch (inEncoding) {
            case NSUTF8StringEncoding: 
                switch (outEncoding) {
                    case NSUTF8StringEncoding: {
                        const char* first = static_cast<const char*>(start);
                        const char* last = static_cast<const char*>(end);
                        
                        // TODO: use class json::generator_internal::string_encoder
                        // escape_convert_unsafe() returns unicode::NO_ERROR on success, otherwise a 
                        // negative number indicating an error code as decribed in unicode::ErrorT.
                        int result = escape_convert_unsafe(first, last, UTF_8_encoding_tag(), dest, UTF_8_encoding_tag(), escapeSolidus);
                        return result; // returns a json:unicode::ErrorT
                    }
                        break;
                    default: return (json::unicode::E_UNKNOWN_ERROR - 2);
                }
                break;
            case NSUTF16BigEndianStringEncoding:
                switch (outEncoding) {
                    case NSUTF8StringEncoding: {
                        const uint16_t* first = static_cast<const uint16_t*>(start);
                        const uint16_t* last = static_cast<const uint16_t*>(end);
                        // TODO: use class json::generator_internal::string_encoder
                        // escape_convert_unsafe() returns unicode::NO_ERROR on success, otherwise a 
                        // negative number indicating an error code as decribed in unicode::ErrorT.
                        int result = escape_convert_unsafe(first, last, UTF_16BE_encoding_tag(), dest, UTF_8_encoding_tag(), escapeSolidus);
                        return result;
                    }
                        break;
                    default: return (json::unicode::E_UNKNOWN_ERROR - 2);
                }
                break;
            case NSUTF16LittleEndianStringEncoding:
                switch (outEncoding) {
                    case NSUTF8StringEncoding: {
                        const uint16_t* first = static_cast<const uint16_t*>(start);
                        const uint16_t* last = static_cast<const uint16_t*>(end);
                        // TODO: use class json::generator_internal::string_encoder
                        // escape_convert_unsafe() returns unicode::NO_ERROR on success, otherwise a 
                        // negative number indicating an error code as decribed in unicode::ErrorT.
                        int result =  escape_convert_unsafe(first, last, UTF_16LE_encoding_tag(), dest, UTF_8_encoding_tag(), escapeSolidus);
                        return result;
                    }
                        break;
                    default: return (json::unicode::E_UNKNOWN_ERROR - 2);
                }
                break;
                
            default: return (json::unicode::E_UNKNOWN_ERROR - 1);
        }
    }
    
    
    
    
    // Escape a NSString as required for a JSON string and encode it to
    // encoding 'outputEncoding'.
    // Returns zero on success, otherwise a negative number indicating an
    // error.
    //
    //  TODO: implement other encodings for output.
    //
    // Parameter NSString shall not be nil but may be empty.
    //
    // Errors:
    //  Any json::unicode::ErrorT value, and
    //  json::unicode::E_UNKNOWN_ERROR-2:   Output Encoding not yet implemented or invalid.
    int 
    escapeStringAndInsertIntoBuffer(NSString* string, 
                                           NSMutableDataPushbackBuffer& buffer, 
                                           JPUnicodeEncoding outputEncoding,
                                           JPJsonWriterOptions options)
    {
        typedef json::unicode::UTF_8_encoding_tag                   source_encoding_t;
        typedef json::unicode::UTF_8_encoding_tag                   dest_encoding_t;        
        typedef encoding_traits<dest_encoding_t>::code_unit_type    code_unit_t;        
        typedef NSMutableDataPushbackBuffer                         buffer_t;
                
        assert(string);
        
        // TODO: enable other output encodings
        assert(outputEncoding == JPUnicodeEncoding_UTF8);
        
        // Try to get the NSString's content in internal encoding. That way, we
        // may apply faster conversion routines. For optimal performance we
        // require the internal encoding be in Unicode.
        NSStringEncoding ns_string_encoding = -1;
        const void* bytes = string ? CFStringGetCharactersPtr(CFStringRef(string)) : NULL;
        size_t length = bytes ? [string length]*2 : 0; // length equals number of bytes 
        if (bytes) {
            // UTF-16. We strongly assume, the endianess is platform:
            ns_string_encoding = json::ns_unicode_encoding_traits<to_host_endianness<UTF_16_encoding_tag>::type>::value;
        }
        else {
            // No UTF-16 Unicode, try UTF-8:
            bytes = CFStringGetCStringPtr(CFStringRef(string), kCFStringEncodingUTF8);
            if (bytes) {
                ns_string_encoding = NSUTF8StringEncoding;
            }
            else {
                // Internal encoding is not Unicode. Performance is suboptimal. Convert
                // NSString to output encoding:
                ns_string_encoding = JPUnicodeEncodingToNSStringEncoding(outputEncoding);
                bytes = [string cStringUsingEncoding:ns_string_encoding];
            }
            length = [string lengthOfBytesUsingEncoding:ns_string_encoding];
        }
        
        
        if (length > 0) {
            const void* first = static_cast<const char*>(bytes);        
            const void* last = static_cast<const char*>(bytes) + length;        
            
            std::back_insert_iterator<buffer_t> back_it(buffer);            
            // escape_convert() returns:
            //  Any json::unicode::ErrorT value, and
            //  json::unicode::E_UNKNOWN_ERROR-1:   Input Encoding not yet implemented or invalid.
            //  json::unicode::E_UNKNOWN_ERROR-2:   Output Encoding not yet implemented or invalid.
            int result = escape_convert(first, last, ns_string_encoding,
                                        back_it, JPUnicodeEncodingToNSStringEncoding(outputEncoding), 
                                        ((options & JPJsonWriterEscapeSolidus)!=0));   
            return result;
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

- (int) JPJsonWriter_insertIntoBuffer:(NSMutableDataPushbackBuffer*)buffer 
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);    
    assert(buffer);

    const NSUInteger count = [self count];
    buffer->push_back('[');
    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        buffer->push_back('\n');
        NSUInteger indent = std::min(NSUInteger(8), level+1);
        while (indent--) {
            buffer->push_back('\t');
        }
    }    
    
    NSUInteger i = count;
    for (id value in self) {
        int result = [value JPJsonWriter_insertIntoBuffer:buffer 
                                                 encoding:encoding 
                                                options:options 
                                                    level:level + 1];
        if (result < 0) {
            return result;
        }
        if (--i) {
            buffer->push_back(',');
            if ((options & JPJsonWriterPrettyPrint) != 0) {
                buffer->push_back('\n');
                NSUInteger indent = std::min(NSUInteger(8), level+1);
                while (indent--) {
                    buffer->push_back('\t');
                }
            }
        }
    }
    
    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        buffer->push_back('\n');
        NSUInteger indent = std::min(NSUInteger(8), level);
        while (indent--) {
            buffer->push_back('\t');
        }
    }
    buffer->push_back(']');    
    
    return 0;
}

@end



// -----------------------------------------------------------------------------
//  Category NSDictionary
// -----------------------------------------------------------------------------
@interface NSDictionary (JPJsonWriter) 
@end

@implementation NSDictionary (JPJsonWriter)
- (int) JPJsonWriter_insertIntoBuffer:(NSMutableDataPushbackBuffer*)buffer
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);
    assert(buffer);
    
    const NSUInteger count = [self count];
    
    buffer->push_back('{');
    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        buffer->push_back('\n');
        NSUInteger indent = std::min(NSUInteger(8), level+1);
        while (indent--) {
            buffer->push_back('\t');
        }
    }

    id o;
    if ((options & JPJsonWriterSortKeys) != 0) {    
        o = [[self allKeys] sortedArrayUsingSelector:@selector(compare:)];
    }
    else {
        o = self;
    }
    NSUInteger i = count;
    for (id key in o) {
        [key JPJsonWriter_insertIntoBuffer:buffer encoding:encoding options:options 
                                   level:level + 1];
        
        if ((options & JPJsonWriterPrettyPrint) != 0) {
            buffer->push_back(' ');
        }
        buffer->push_back(':');        
        if ((options & JPJsonWriterPrettyPrint) != 0) {
            buffer->push_back(' ');
        }

        // TODO: check if blocks give a performance advantage
        [[self objectForKey:key] JPJsonWriter_insertIntoBuffer:buffer encoding:encoding 
                                                     options:options level:level + 1];
        if (--i) {
            buffer->push_back(',');
            if ((options & JPJsonWriterPrettyPrint) != 0) {
                buffer->push_back('\n');
                NSUInteger indent = std::min(NSUInteger(8), level+1);
                while (indent--) {
                    buffer->push_back('\t');
                }
            }
        }
    }    

    if (count and (options & JPJsonWriterPrettyPrint) != 0) {
        buffer->push_back('\n');
        NSUInteger indent = std::min(NSUInteger(8), level);
        while (indent--) {
            buffer->push_back('\t');
        }
    }
    buffer->push_back('}');    
    
    return 0;
}

@end


// -----------------------------------------------------------------------------
//  Category NSString
// -----------------------------------------------------------------------------
@interface NSString (JPJsonWriter) 
@end

@implementation  NSString (JPJsonWriter) 
- (int) JPJsonWriter_insertIntoBuffer:(NSMutableDataPushbackBuffer*)buffer 
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);  
    assert(buffer);
    
    buffer->push_back('"');
    int result = escapeStringAndInsertIntoBuffer(self, *buffer, encoding, options);
    // TODO: handle error    
    // Errors:
    //  Any json::unicode::ErrorT value, and
    //  json::unicode::E_UNKNOWN_ERROR-2:   Output Encoding not yet implemented or invalid.
    buffer->push_back('"');
    return result;
}

@end



// -----------------------------------------------------------------------------
//  Category NSNumber
// -----------------------------------------------------------------------------
@interface NSNumber (JPJsonWriter) 
@end

@implementation NSNumber (JPJsonWriter) 
- (int) JPJsonWriter_insertIntoBuffer:(NSMutableDataPushbackBuffer*)buffer
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);  
    assert(buffer);
    
    if ((CFTypeRef)self == kCFBooleanTrue) {
        buffer->write("true", 4);
    }
    else if ((CFTypeRef)self == kCFBooleanFalse) {
        buffer->write("false", 5);
    }
    else {
        size_t count;
        insertNumberIntoBuffer(CFNumberRef(self), *buffer, &count, encoding);
    }

    return 0;
}

@end


// -----------------------------------------------------------------------------
//  Category NSDecimalNumber
// -----------------------------------------------------------------------------
@interface NSDecimalNumber (JPJsonWriter) 
@end

@implementation NSDecimalNumber (JPJsonWriter) 
- (int) JPJsonWriter_insertIntoBuffer:(NSMutableDataPushbackBuffer*)buffer
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8); 
    assert(buffer);
    
    NSString* numberString = [self descriptionWithLocale:nil];
    const char* s = [numberString cStringUsingEncoding:NSUTF8StringEncoding];
    NSUInteger length = [numberString lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    assert(s);
    buffer->write(s, length);
    
    return 0;
}

@end


// -----------------------------------------------------------------------------
//  Category NSNull
// -----------------------------------------------------------------------------
@interface NSNull (JPJsonWriter) 
@end

@implementation NSNull (JPJsonWriter) 

- (int) JPJsonWriter_insertIntoBuffer:(NSMutableDataPushbackBuffer*)buffer
                             encoding:(JPUnicodeEncoding)encoding
                              options:(JPJsonWriterOptions)options
                                level:(NSUInteger)level
{
    assert(encoding == JPUnicodeEncoding_UTF8); 
    assert(buffer);
    
    buffer->write("null", 4);
    
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
    if(![object respondsToSelector:@selector(JPJsonWriter_insertIntoBuffer:encoding:options:level:)]) {
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
    NSMutableDataPushbackBuffer buffer(data);
    
    if ((options & JPJsonWriterWriteBOM) != 0) {
        int result = insertBOMIntoBuffer(buffer, encoding);
        if (result <= 0) {
            if (error) {
                *error = makeError(4, "Unknown error occured");
            }
            return nil;
        }     
    }
    int result = [object JPJsonWriter_insertIntoBuffer:&buffer encoding:encoding options:options level:0];
    NSUInteger len = buffer.size();
    [data setLength:len];
    if (result < 0) {
        if (error) {
            *error = makeError(5, "Could not serialize JSON into NSData object");
        }
        return nil;
    }
    return data;
}


@end

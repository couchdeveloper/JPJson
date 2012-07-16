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

#if __has_feature(objc_arc) 
#error This Objective-C file shall be compiled with ARC disabled.
#endif

// #include <boost/spirit/include/karma.hpp>

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

#import "JPJsonWriterExtensions.h"
#include "json/ObjC/NSDataStreambuf.hpp"


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
    template <uint32_t NSStringEncoding>
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

    
    //
    //  Maps JPUnicodeEncoding encoding constants to NSStringEncoding constants
    //
    
    inline  NSStringEncoding
    JPUnicodeEncodingToNSStringEncoding(JPUnicodeEncoding encoding) 
    {
        switch (encoding) {
            case JPUnicodeEncoding_UTF8:    return NSUTF8StringEncoding;
            case JPUnicodeEncoding_UTF16:   return NSUTF16StringEncoding;
            case JPUnicodeEncoding_UTF16BE: return NSUTF16BigEndianStringEncoding;
            case JPUnicodeEncoding_UTF16LE: return NSUTF16LittleEndianStringEncoding;
            case JPUnicodeEncoding_UTF32:   return NSUTF32StringEncoding;
            case JPUnicodeEncoding_UTF32BE: return NSUTF32BigEndianStringEncoding;
            case JPUnicodeEncoding_UTF32LE: return NSUTF32LittleEndianStringEncoding;
            default: return -1;
        }
    };
    
}


#if 0
#pragma mark - NSMutableDataPushbackBuffer
namespace {
    
    
    class NSMutableDataPushbackBuffer 
    {
    public:
        NSMutableDataPushbackBuffer(NSMutableData* data = nil) 
        : data_(data), p_(0), start_(0), last_(0)
        {
            [data_ retain];
        }
        
        ~NSMutableDataPushbackBuffer() {
            [data_ release];
        }
        
        void setData(NSMutableData* data) {
            if (data_ != data) {
                [data_ release];
                data_ = [data retain];
            }
        }
        size_t size() const { return p_ - start_; } 
        size_t capacity() const { return last_ - start_; } 
        
        template <typename T>
        void put(const T& v) {
#if 0            
            if (__builtin_expect((last_ - p_) < sizeof(T), 0)) {
                [data_ increaseLengthBy:1024];
                uint8_t* tmp = static_cast<uint8_t*>([data_ mutableBytes]);
                p_ = tmp + size();
                last_ = tmp + capacity() + 1024;                
                start_ = tmp;
            }
            *static_cast<T*>(static_cast<void*>(p_)) = v;
            p_ += sizeof(T);
#else       
            if ((last_ - p_) >= sizeof(T)) {
                *static_cast<T*>(static_cast<void*>(p_)) = v;
                p_ += sizeof(T);
                return;
            }
            else {
                [data_ increaseLengthBy:1024];
                uint8_t* tmp = static_cast<uint8_t*>([data_ mutableBytes]);
                p_ = tmp + size();
                last_ = tmp + capacity() + 1024;                
                start_ = tmp;
                *static_cast<T*>(static_cast<void*>(p_)) = v;
                p_ += sizeof(T);
                return;
            }            
#endif            
        }
        
        void write(const void* p, size_t n) {
            if ( (last_ - p_) < sizeof(char)*n) {
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
        
    private:
        
        NSMutableData*  data_;        
        uint8_t*        p_;
        uint8_t*        start_;
        uint8_t*        last_;
    };
    
    
    
}
#endif



#if 0
#pragma mark - JPJsonOutputStream
// -----------------------------------------------------------------------------
//  A concrete JPJsonOutputStream Implementation
// -----------------------------------------------------------------------------
@interface JPJsonWriterMutableDataBuffer : NSObject <JPJsonOutputStreamProtocol>
@end

@implementation JPJsonWriterMutableDataBuffer {
    NSMutableDataPushbackBuffer  _imp;        
}

- (id)initWithMutableData:(NSMutableData*)data
{
    self = [super init];
    if (self) {
        _imp.setData(data);
    }
    return self;
}

- (void) writeData:(const void*)data length:(NSUInteger)length
{
    _imp.write(data, length);
}

- (NSUInteger) size { 
    return _imp.size();
} 

- (NSUInteger) capacity { 
    return _imp.capacity();
} 

- (NSMutableDataPushbackBuffer*) imp_pointer {
    return &_imp;
}

@end
#endif



#pragma mark - JPDataStreambuffer

//  A concrete JPJsonStreambuffer Implementation
@interface JPDataStreambuffer : NSObject <JPJsonStreambufferProtocol>
@end

@implementation JPDataStreambuffer {
    json::objc::NSDataStreambuf<char>  _streambuf;        
}

- (id)initWithData:(NSData*)data
{
    self = [super init];
    if (self) {
        _streambuf.data(data);
    }
    return self;
}

- (id)init {
    return [self initWithData:nil];
}

- (int) write:(const void*)buffer length:(int)length;
{
    int result;
    if (length == 1) {
        result = _streambuf.sputc(*static_cast<const char*>(buffer)) != EOF ? 1 : 0;
    }
    else {
        result = static_cast<int>(_streambuf.sputn(static_cast<const char*>(buffer), length));
    }
    return result;
}

- (NSData*) data {
    return _streambuf.data();
}

@end


@interface JPDataStreambuffer (Private)
@end
@implementation JPDataStreambuffer (Private)
- (std::streambuf*) internal_streambuf {
    return &_streambuf;
}
@end




#if 0
#pragma mark - std 
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
        back_insert_iterator& operator= (T const& value) { 
            buffer_.put(value); return *this; 
        }
        back_insert_iterator& operator* ()
        { return *this; }
        back_insert_iterator& operator++ ()
        { return *this; }
        back_insert_iterator operator++ (int)
        { return *this; }
    };
    
    typedef JPJsonWriterMutableDataBuffer* JPJsonOutputStream_t;
   
    template <>
    class back_insert_iterator<JPJsonOutputStream_t> 
    : public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    protected:
        NSMutableDataPushbackBuffer* buffer_;
        
    public:
        explicit back_insert_iterator(JPJsonOutputStream_t buffer) 
        : buffer_([buffer imp_pointer]) 
        {
        }
        template <typename T>
        back_insert_iterator& operator= (T const& value) { 
            buffer_->put(value); return *this; 
        }
        back_insert_iterator& operator* ()
        { return *this; }
        back_insert_iterator& operator++ ()
        { return *this; }
        back_insert_iterator operator++ (int)
        { return *this; }
    };
    
}
#endif
    
    
namespace std {
    
    template <>
    class back_insert_iterator<std::streambuf> 
    : public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    protected:
        std::streambuf* streambuf_;
        
    public:
        explicit back_insert_iterator(std::streambuf* streambuffer) 
        : streambuf_(streambuffer) 
        {
        }
        template <typename CharT>
        back_insert_iterator& operator= (CharT const& value) { 
#if defined (DEBUG)            
            int result = 
#endif            
            streambuf_->sputn(static_cast<char const*>(static_cast<void const*>(&value)), sizeof(value)); 
            assert(result == sizeof(value));
            return *this; 
        }
        back_insert_iterator& operator* ()
        { return *this; }
        back_insert_iterator& operator++ ()
        { return *this; }
        back_insert_iterator operator++ (int)
        { return *this; }
    };
    
        
    // Note: any concrete subclass of JPJsonStreambufferProtocol must be implemented
    // using a std::streambuf.

    template <>
    class back_insert_iterator< id<JPJsonStreambufferProtocol> > 
    : public std::iterator<std::output_iterator_tag,void,void,void,void>
    {
    protected:
        std::streambuf* streambuf_;
        
    public:
        explicit back_insert_iterator(id<JPJsonStreambufferProtocol> streambuffer) 
        : streambuf_([(id)streambuffer internal_streambuf]) 
        {
        }
        template <typename CharT>
        back_insert_iterator& operator= (CharT const& value) { 
            if (sizeof(value) == 1) {
                streambuf_->sputc(*static_cast<const char*>(static_cast<const void*>(&value)));
            }
            else {
                streambuf_->sputn(static_cast<const char*>(static_cast<const void*>(&value)), sizeof(value));
            }
            return *this; 
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
    int insertBOMIntoBuffer(id<JPJsonStreambufferProtocol> streambuf, JPUnicodeEncoding encoding)
    {
        int result;
        switch (encoding) {
            case JPUnicodeEncoding_UTF8: 
                    copyBOM(std::back_inserter(streambuf), UTF_8_encoding_tag());
                    result = encoding_traits<UTF_8_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF16BE:
                    copyBOM(std::back_inserter(streambuf), UTF_16BE_encoding_tag());
                    result =  encoding_traits<UTF_16BE_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF16LE: 
                    copyBOM(std::back_inserter(streambuf), UTF_16LE_encoding_tag());
                    result = encoding_traits<UTF_16LE_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF32BE:
                    copyBOM(std::back_inserter(streambuf), UTF_32BE_encoding_tag());
                    result = encoding_traits<UTF_32BE_encoding_tag>::bom_byte_size;
                break;
            case JPUnicodeEncoding_UTF32LE:
                    copyBOM(std::back_inserter(streambuf), UTF_32LE_encoding_tag());
                    result = encoding_traits<UTF_32LE_encoding_tag>::bom_byte_size;
                break;
            default: 
                result = -3;
        }
        return result;
    }
}


namespace {
    
    using namespace boost::spirit;
    using boost::spirit::karma::real_policies;
    using boost::spirit::karma::real_generator;
    using boost::spirit::karma::generate;
    

    // float to number string generator:
    // define a new real number formatting policy
    template <typename Num>
    struct scientific_policy : real_policies<Num>
    {
        // we want the numbers always to be in scientific format
        static int floatfield(Num n) { return real_policies<Num>::fmtflags::scientific; }
        static int unsigned precision(Num n) { return std::numeric_limits<Num>::digits10 + 1; }
        //static bool trailing_zeros(Num n) { return true; } 
    };
    
    // define a new generator type based on the new policy
    typedef real_generator<double, scientific_policy<double> > science_type;
    
    static science_type const scientific_generator = science_type();
    
    


    // TODO: replace number to string conversions with karma, which should be
    // much faster than snprintf or Cocoa's methods.
    // Returns zero on success, otherwise a negative number indicating the error.
    int serializeNumberIntoBuffer(CFNumberRef number, id<JPJsonStreambufferProtocol> streambuf, 
                               size_t* outCount, JPUnicodeEncoding encoding)
    {
        
        
        static CFNumberFormatterRef s_numberFormatter;
        
        assert(encoding == JPUnicodeEncoding_UTF8);
        
        char tmp_buffer[128];
        char* begin = tmp_buffer;
        char* p = tmp_buffer;
#if defined (DEBUG)        
        char* end_cap = tmp_buffer + sizeof(tmp_buffer);
#endif        
        int count = 0;      // the number of bytes in tmp_buffer
                
        
        char numberType = *[(id)number objCType];
        Boolean conversionSucceeded = false;
        
        switch (numberType) {
            case 'c':
            case 'C':
            case 's':
            case 'S':
            case 'i':
            case 'l':  { // 32-bit
                assert(sizeof(int) >= 4);
                int result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberIntType, &result);
                generate(p, int_, result);
                count = static_cast<int>(p - begin);
                *p = 0;
            }
                break;
        
            case 'I':
            case 'L': {
                unsigned int result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberIntType, &result);
                generate(p, uint_, result);
                count = static_cast<int>(p - begin);
                *p = 0;
            }
                break;

            case 'q': {
                long long result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberLongLongType, &result);
                generate(p, long_long, result);
                assert(p <= end_cap);
                count = static_cast<int>(p - begin);
                *p = 0;
            }
                break;

            case 'Q': {
                unsigned long long result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberLongLongType, &result);
                generate(p, ulong_long, result);
                assert(p <= end_cap);
                count = static_cast<int>(p - begin);
                *p = 0;
            }
                break;
                                
            case 'f': {
                float result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberFloatType, &result);
                generate(p, scientific_generator, result);
                count = static_cast<int>(p - begin);
                *p = 0;
            }
                break;
                
            case 'd': {
                assert(sizeof(double) == 8);
                double result;
                conversionSucceeded = CFNumberGetValue(number, kCFNumberDoubleType, &result);
                generate(p, scientific_generator, result);
                count = static_cast<int>(p - begin);
                *p = 0;
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
            count = static_cast<int>(usedBufLen);
        } 
        
        // TODO: Convert the buffer to the target encoding        
        //std::copy(tmp_buffer, tmp_buffer+count, std::back_inserter(buffer));
        int wcount = [streambuf write:tmp_buffer length:count];
        if (wcount != count) {
            return -1; // ERROR
        }

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
    escapeStringAndInsertIntoBuffer(NSString* string, bool withQuotes,
                                           id<JPJsonStreambufferProtocol> streambuf, 
                                           JPUnicodeEncoding outputEncoding,
                                           JPJsonWriterOptions options)
    {
        typedef json::unicode::UTF_8_encoding_tag                   source_encoding_t;
        typedef json::unicode::UTF_8_encoding_tag                   dest_encoding_t;        
        typedef encoding_traits<dest_encoding_t>::code_unit_type    code_unit_t;
        typedef id<JPJsonStreambufferProtocol>                      streambuf_t;
                
        assert(string);
        // TODO: enable other output encodings
        assert(outputEncoding == JPUnicodeEncoding_UTF8);
        
        std::back_insert_iterator<streambuf_t> back_it(streambuf);
        if (withQuotes) {
            *back_it++ = '\"';
        }        
        // Try to get the NSString's content in internal encoding. That way, we
        // may apply faster conversion routines. For optimal performance
        // the internal encoding should be Unicode UTF-16 or UTF-8.
        NSStringEncoding ns_string_encoding = -1;
        size_t length = 0; // number of bytes (not code units)
        const void* bytes = CFStringGetCharactersPtr(CFStringRef(string));
        if (bytes) {
            // UTF-16. We strongly assume, the endianess is platform:
            ns_string_encoding = json::ns_unicode_encoding_traits<to_host_endianness<UTF_16_encoding_tag>::type>::value;
            length = CFStringGetLength(CFStringRef(string)) * 2;
        }
        else {
            // No UTF-16 Unicode, try UTF-8:
            bytes = CFStringGetCStringPtr(CFStringRef(string), kCFStringEncodingUTF8);
            if (bytes) {
                ns_string_encoding = NSUTF8StringEncoding;
                length = [string lengthOfBytesUsingEncoding:ns_string_encoding];
            }
        }
        int result = 0;  // the result of the conversion function - should always succeed (returns 0).
        if (bytes) {
            // UTF-8 or UTF-16 encoding
            assert(ns_string_encoding != -1);
            if (length > 0) {
                const void* first = static_cast<const char*>(bytes);
                const void* last = static_cast<const char*>(bytes) + length;
                // escape_convert() returns:
                //  Any json::unicode::ErrorT value, and
                //  json::unicode::E_UNKNOWN_ERROR-1:   Input Encoding not yet implemented or invalid.
                //  json::unicode::E_UNKNOWN_ERROR-2:   Output Encoding not yet implemented or invalid.
                result = escape_convert(first, last, ns_string_encoding,
                                        back_it, JPUnicodeEncodingToNSStringEncoding(outputEncoding),
                                        ((options & JPJsonWriterEscapeSolidus)!=0));
            }
        }
        else {
            // Internal encoding is not Unicode. Performance is suboptimal since we need
            // CFString to convert from its internal representation to UTF-16 using an
            // intermediate buffer. This conversion is also not that fast, even for any
            // 8-bit (system) encoding which is frequently the default for smaller strings
            // (e.g. kCFStringEncodingMacRoman) - regardless of how the string was created.
            // An option which might slightly improve performance would involve to try to
            // get the C-String pointer with kCFStringEncodingMacRoman encoding and if this
            // returned a non-NULL pointer, converting the bytes manually to UTF-16 using
            // a simple map table. The approach below is sufficiently fast and works
            // with any internal encoding, though:
            // 
            // Convert NSString to UTF-16 encoding using a chunk buffer, then repeately
            // convert the buffer to outputEncoding:
            // We strongly assume, the endianess is platform:
            ns_string_encoding = json::ns_unicode_encoding_traits<to_host_endianness<UTF_16_encoding_tag>::type>::value;
            NSStringEncoding const ns_output_encoding = JPUnicodeEncodingToNSStringEncoding(outputEncoding);
            int const BufferSize = 128;
            UniChar buffer[BufferSize];
            CFIndex const len = CFStringGetLength(CFStringRef(string));
            CFIndex pos = 0;
            while (pos < len and result == 0) {
                CFIndex rangeLen = std::min(len-pos, static_cast<CFIndex>(BufferSize));
                CFRange range = {pos, rangeLen};
                CFStringGetCharacters(CFStringRef(string), range, buffer);
                const void* first = static_cast<const UniChar*>(buffer);
                const void* last = static_cast<const UniChar*>(buffer + rangeLen);
                // escape_convert() returns:
                //  Any json::unicode::ErrorT value, and
                //  json::unicode::E_UNKNOWN_ERROR-1:   Input Encoding not yet implemented or invalid.
                //  json::unicode::E_UNKNOWN_ERROR-2:   Output Encoding not yet implemented or invalid.
                result = escape_convert(first, last, ns_string_encoding,
                                        back_it, ns_output_encoding,
                                        ((options & JPJsonWriterEscapeSolidus)!=0));
                pos += rangeLen;
            }
        }

        if (withQuotes) {
            *back_it++ = '\"';
        }
        return result;
    }
    
    
}  // namesapace





#pragma mark - Inserters for Containers JSON Array and JSON Object

namespace {
    
    //
    //  serializeJsonArray
    //
    //    Parameter `object` shall respond to message `count` and shall implement the
    //    protocol NSFastEnumeration.
    
    int serializeJsonArray(id object, id<JPJsonStreambufferProtocol> streambuf, 
                    JPUnicodeEncoding encoding, JPJsonWriterOptions options, 
                    int level)
    {
        typedef id<JPJsonStreambufferProtocol>          streambuf_t;
        assert(encoding == JPUnicodeEncoding_UTF8);    
        assert(streambuf);
        assert(object != nil);
        
        std::back_insert_iterator<streambuf_t> back_it(streambuf);
        
        
        const NSUInteger count = [object count];
        //[streambuf write:"[" length:1]; //buffer->push_back('[');
        *back_it++ = '[';
        if (count and (options & JPJsonWriterPrettyPrint) != 0) {
            int wcount = [streambuf write:"\n" length:1]; //buffer->push_back('\n');
            if (wcount != 1) {
                return -1; // ERROR
            }
            int indent = level+1;
            while (indent > 0) {
                int n  = std::min(10, indent);
                int wcount = [streambuf write:"\t\t\t\t\t\t\t\t\t\t" length:n];
                if (wcount != n) {
                    return -1; // ERROR: streambuffer write failed
                }
                indent -=n;
            }
        }    
        NSUInteger i = count;
        for (id value in object) {
            if (value == nil) {
                value = [NSNull null];
            }
            int result = [value JPJson_serializeTo:streambuf
                                          encoding:encoding
                                           options:options
                                             level:level + 1];
            if (result < 0) {
                return result;
            }
            if (--i) {
                //[streambuf write:"," length:1]; //buffer->push_back(',');
                *back_it++ = ',';
                if ((options & JPJsonWriterPrettyPrint) != 0) {
                    int wcount = [streambuf write:"\n" length:1]; //buffer->push_back('\n');
                    if (wcount != 1) {
                        return -1; // ERROR
                    }
                    int indent = level+1;
                    while (indent > 0) {
                        int n  = std::min(10, indent);
                        int wcount = [streambuf write:"\t\t\t\t\t\t\t\t\t\t" length:n];
                        if (wcount != n) {
                            return -1; // ERROR: streambuffer write failed
                        }
                        indent -=n;
                    }
                }
            }
        }
        if (count and (options & JPJsonWriterPrettyPrint) != 0) {
            int wcount = [streambuf write:"\n" length:1]; //buffer->push_back('\n');
            if (wcount != 1) {
                return -1;// ERROR: streambuffer write failed
            }
            int indent = level;
            while (indent > 0) {
                int n  = std::min(10, indent);
                int wcount = [streambuf write:"\t\t\t\t\t\t\t\t\t\t" length:n];
                if (wcount != n) {
                    return -1;// ERROR: streambuffer write failed
                }
                indent -=n;
            }
        }
        //[streambuf write:"]" length:1]; //buffer->push_back(']');    
        *back_it++ = ']';
        
        return 0;
    }
    
    
    //
    //  serializeJsonObject
    //
    //  Parameter `object` shall respond to message `count` and message objectForKey: 
    //  and shall implement the protocol NSFastEnumeration.
    //
    int serializeJsonObject(id object, id<JPJsonStreambufferProtocol> streambuf,
                        JPUnicodeEncoding encoding, JPJsonWriterOptions options, 
                        int level)
    {
        typedef id<JPJsonStreambufferProtocol>          streambuf_t;
        
        assert(encoding == JPUnicodeEncoding_UTF8);
        assert(streambuf);
        assert(object != nil);
        
        const NSUInteger count = [object count];

        std::back_insert_iterator<streambuf_t> back_it(streambuf);        
        
        //[streambuf write:"{" length:1]; //buffer->push_back('{');
        *back_it++ = '{';
        
        if (count and (options & JPJsonWriterPrettyPrint) != 0) {
            int wcount = [streambuf write:"\n" length:1]; //buffer->push_back('\n');
            if (wcount != 1) {
                return -1; //ERROR streambuf write failed
            }
            int indent = level+1;
            while (indent > 0) {
                int n  = std::min(10, indent);
                int wcount = [streambuf write:"\t\t\t\t\t\t\t\t\t\t" length:n];
                if (wcount != n) {
                    return -1; // ERROR
                }
                indent -=n;
            }
        }
        id o;
        if ((options & JPJsonWriterSortKeys & [object respondsToSelector:@selector(allKeys)]) != 0) {
            o = [[(id)object allKeys] sortedArrayUsingSelector:@selector(compare:)];
        }
        else {
            o = object;
        }
        NSUInteger i = count;
        for (id key in o) {
            int result = [key JPJson_serializeTo:streambuf
                                        encoding:encoding
                                         options:options
                                           level:level + 1];
            if (result != 0) {
                return result;
            }
            if ((options & JPJsonWriterPrettyPrint) != 0) {
                int wcount = [streambuf write:" " length:1]; //buffer->push_back(' ');
                if (wcount != 1) {
                    return -1; // ERROR
                }
            }
            //[streambuf write:":" length:1]; //buffer->push_back(':');        
            *back_it++ = ':';
            if ((options & JPJsonWriterPrettyPrint) != 0) {
                int wcount = [streambuf write:" " length:1]; //buffer->push_back(' ');
                if (wcount != 1) {
                    return -1; // ERROR
                }
            }
            
            // TODO: check if blocks give a performance advantage
            id value = [object objectForKey:key];
            if (value == nil) {
                value = [NSNull null];
            }
            result = [value JPJson_serializeTo:streambuf encoding:encoding options:options level:level + 1];
            if (result != 0) {
                return result;
            }
            if (--i) {
                //[streambuf write:"," length:1]; //buffer->push_back(',');
                *back_it++ = ',';
                if ((options & JPJsonWriterPrettyPrint) != 0) {
                    int wcount = [streambuf write:"\n" length:1]; //buffer->push_back('\n');
                    if (wcount != 1) {
                        return -1; // ERROR
                    }
                    int indent = level+1;
                    while (indent > 0) {
                        int n  = std::min(10, indent);
                        int wcount = [streambuf write:"\t\t\t\t\t\t\t\t\t\t" length:n];
                        if (wcount != n) {
                            return -1; // ERROR
                        }
                        indent -=n;
                    }
                }
            }
                
        }    
        if (count and (options & JPJsonWriterPrettyPrint) != 0) {
            //[streambuf write:"\n" length:1]; //buffer->push_back('\n');
            *back_it++ = '\n';
            int indent = level;
            while (indent > 0) {
                int n  = std::min(10, indent);
                int wcount = [streambuf write:"\t\t\t\t\t\t\t\t\t\t" length:n];
                if (wcount != n) {
                    return -1; // ERROR
                }
                indent -=n;
            }
        }
        //[streambuf write:"}" length:1]; //buffer->push_back('}');    
        *back_it++ = '}';
        
        return 0;
    }
    
    
    
}





#pragma mark - NSArray Category

// -----------------------------------------------------------------------------
//  Category NSArray
// -----------------------------------------------------------------------------
@interface NSArray (JPJsonWriter) <JPJsonSerializableProtocol>
@end

@implementation NSArray (JPJsonWriter)

- (int) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>)streambuf
                  encoding:(JPUnicodeEncoding)encoding
                   options:(JPJsonWriterOptions)options
                     level:(int)level
{
    return serializeJsonArray(self, streambuf, encoding, options, level);
}

@end



#pragma mark - NSDictionary Category
// -----------------------------------------------------------------------------
//  Category NSDictionary
// -----------------------------------------------------------------------------
@interface NSDictionary (JPJsonWriter) <JPJsonSerializableProtocol>
@end

@implementation NSDictionary (JPJsonWriter)

- (int) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>)streambuf
                  encoding:(JPUnicodeEncoding)encoding
                   options:(JPJsonWriterOptions)options
                     level:(int)level
{
    return serializeJsonObject(self, streambuf, encoding, options, level);
}
@end


#pragma mark - NSString Category
// -----------------------------------------------------------------------------
//  Category NSString
// -----------------------------------------------------------------------------
@interface NSString (JPJsonWriter) <JPJsonSerializableProtocol>
@end

@implementation  NSString (JPJsonWriter) 
- (int) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>) streambuf
                  encoding:(JPUnicodeEncoding) encoding
                   options:(JPJsonWriterOptions) options
                     level:(int) level;
{
    assert(encoding == JPUnicodeEncoding_UTF8);  
    assert(streambuf);
    
    const bool withQuotes = true;
    int result = escapeStringAndInsertIntoBuffer(self, withQuotes, streambuf, encoding, options);
    // TODO: handle error    
    // Errors:
    //  Any json::unicode::ErrorT value, and
    //  json::unicode::E_UNKNOWN_ERROR-2:   Output Encoding not yet implemented or invalid.
    return result;
}

@end



#pragma mark - NSNumber Category
// -----------------------------------------------------------------------------
//  Category NSNumber
// -----------------------------------------------------------------------------
@interface NSNumber (JPJsonWriter) <JPJsonSerializableProtocol>
@end

@implementation NSNumber (JPJsonWriter) 
- (int) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>)streambuf
                  encoding:(JPUnicodeEncoding)encoding
                   options:(JPJsonWriterOptions)options
                     level:(int)level
{
    assert(encoding == JPUnicodeEncoding_UTF8);  
    assert(streambuf);
    
    // TODO: encoding!
    CFNumberType numberType = CFNumberGetType((CFNumberRef)self);
    int boolValue;
    if (numberType == kCFNumberCharType) {
        CFNumberGetValue((CFNumberRef)self, kCFNumberIntType, &boolValue);
        if (boolValue == 0) {
            int wcount = [streambuf write:"false" length:5]; // buffer->write("false", 5);
            if (wcount != 5) {
                return -1; // ERROR
            }
            return 0;  // success
        }
        else if (boolValue == 1) {
            //assert(boolValue == 1);
            int wcount = [streambuf write:"true" length:4]; // buffer->write("true", 4);
            if (wcount != 4) {
                return -1; // ERROR
            }
            return 0;  // success
        }
    }
    return serializeNumberIntoBuffer(CFNumberRef(self), streambuf, NULL, encoding);
}

@end


#pragma mark - NSDecimalNumber Category
// -----------------------------------------------------------------------------
//  Category NSDecimalNumber
// -----------------------------------------------------------------------------
@interface NSDecimalNumber (JPJsonWriter) <JPJsonSerializableProtocol>
@end

@implementation NSDecimalNumber (JPJsonWriter) 
- (int) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>)streambuf
                  encoding:(JPUnicodeEncoding)encoding
                   options:(JPJsonWriterOptions)options
                     level:(int)level
{
    assert(encoding == JPUnicodeEncoding_UTF8); 
    assert(streambuf);
    
    NSString* numberString = [self descriptionWithLocale:nil];
    const char* s = [numberString cStringUsingEncoding:NSUTF8StringEncoding];
    NSUInteger length = [numberString lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    assert(s);
    // TODO: encoding!
    int wcount = [streambuf write:s length:static_cast<int>(length)]; // buffer->write(s, length);
    if (wcount != length) {
        return -1; // ERROR
    }
    else {
        return 0;
    }
}

@end


#pragma mark - NSNull Category
// -----------------------------------------------------------------------------
//  Category NSNull
// -----------------------------------------------------------------------------
@interface NSNull (JPJsonWriter) <JPJsonSerializableProtocol>
@end

@implementation NSNull (JPJsonWriter) 

- (int) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>)streambuf
                  encoding:(JPUnicodeEncoding)encoding
                   options:(JPJsonWriterOptions)options
                     level:(int)level
{
    assert(encoding == JPUnicodeEncoding_UTF8); 
    assert(streambuf);
    // TODO: encoding!
    int wcount = [streambuf write:"null" length:4]; // buffer->write("null", 4);
    if (wcount != 4) {
        return -1; // ERROR
    }
    else {
        return 0;
    }
}

@end





#pragma mark - JPJsonWriter


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
    if(![object respondsToSelector:@selector(JPJson_serializeTo:encoding:options:level:)]) {
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
        
    JPDataStreambuffer* streambuf = [[JPDataStreambuffer alloc] init];
    
    if ((options & JPJsonWriterWriteBOM) != 0) {
        int result = insertBOMIntoBuffer(streambuf, encoding);
        if (result <= 0) {
            if (error) {
                *error = makeError(4, "Unknown error occured");
            }
            [streambuf release];
            return nil;
        }     
    }
    int result = [object JPJson_serializeTo:streambuf encoding:encoding options:options level:0];
    if (result < 0) {
        if (error) {
            *error = makeError(5, "Could not serialize JSON into NSData object");
        }
        [streambuf release];
        return nil;
    }
    NSData*  data = [streambuf data];
    [streambuf release];

    return data;
}


@end


@implementation JPJsonWriter (Extension)


+ (NSNumberFormatter*) defaultDecimalNumberFormatter
{
    static NSNumberFormatter* defaultDecimalNumberFormatter;
    static dispatch_once_t once;
    dispatch_once(&once, ^{ 
        defaultDecimalNumberFormatter = [[NSNumberFormatter alloc] init];
        [defaultDecimalNumberFormatter setNumberStyle: NSNumberFormatterDecimalStyle];
    });

    return defaultDecimalNumberFormatter;
}


+ (int) serializeObjectAsJSONArray:(id) object 
                            buffer:(id<JPJsonStreambufferProtocol>) streambuf
                          encoding:(JPUnicodeEncoding) encoding
                           options:(JPJsonWriterOptions) options
                             level:(int) level
{
    return serializeJsonArray(object, streambuf, encoding, options, level);
}


+ (int) serializeObjectAsJSONObject:(id) object 
                             buffer:(id<JPJsonStreambufferProtocol>) streambuf
                           encoding:(JPUnicodeEncoding) encoding
                            options:(JPJsonWriterOptions) options
                              level:(int) level
{
    return serializeJsonObject(object, streambuf, encoding, options, level);
}

@end


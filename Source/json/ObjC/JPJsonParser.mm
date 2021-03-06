//
//  JPJsonParser.mm
//
//  Created by Andreas Grosam on 7/2/11.
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

#if !__has_feature(objc_arc)
#error This Objective-C file shall be compiled with ARC enabled.
#endif


#include "json/parser/parse.hpp"
#include "json/unicode/unicode_detect_bom.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "json/parser/parser_errors.hpp"
#include <dispatch/dispatch.h>
#include <cstdlib>
#import "JPJsonParser.h"
#import "JPSemanticActionsBase.h"
#import "JPSemanticActionsBase_private.h"
#import "JPRepresentationGenerator.h"  // Default Semantic Actions




namespace json { namespace objc {
    //
    //  Maps JPUnicodeEncoding constants to json::unicode encoding_tag
    //
    template <JPUnicodeEncoding C>
    struct jp_unicode_encoding_traits {
        //BOOST_STATIC_ASSERT(0);
    };
    
    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_Unknown> {
        //BOOST_STATIC_ASSERT(0);
    };
    
    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_UTF8> {
        typedef json::unicode::UTF_8_encoding_tag type;
    };
    
    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_UTF16> {
        typedef json::unicode::UTF_16_encoding_tag type;
    };
    
    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_UTF16BE> {
        typedef json::unicode::UTF_16BE_encoding_tag type;
    };
    
    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_UTF16LE> {
        typedef json::unicode::UTF_16LE_encoding_tag type;
    };

    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_UTF32> {
        typedef json::unicode::UTF_32_encoding_tag type;
    };
    
    
    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_UTF32BE> {
        typedef json::unicode::UTF_32BE_encoding_tag type;
    };
    
    template <>
    struct jp_unicode_encoding_traits<JPUnicodeEncoding_UTF32LE> {
        typedef json::unicode::UTF_32LE_encoding_tag type;
    };
    
    
    //
    //  Map json::unicode encoding_tag to JPUnicodeEncoding constants
    //
    template <typename EncodingT, typename Enable = void>
    struct jp_map_unicode_encoding {
        BOOST_STATIC_ASSERT(sizeof(Enable) == 0);
    };
    
    template <typename EncodingT>
    struct jp_map_unicode_encoding<
        EncodingT,
        typename boost::enable_if<boost::is_same<UTF_8_encoding_tag, EncodingT> >::type
    >
    {
        static constexpr JPUnicodeEncoding value = JPUnicodeEncoding_UTF8;
    };
    
    template <typename EncodingT>
    struct jp_map_unicode_encoding<
        EncodingT,
        typename boost::enable_if<boost::is_same<UTF_16_encoding_tag, EncodingT> >::type
    >
    {
        static constexpr JPUnicodeEncoding value = JPUnicodeEncoding_UTF16;
    };
    
    template <typename EncodingT>
    struct jp_map_unicode_encoding<
        EncodingT,
        typename boost::enable_if<boost::is_same<UTF_16BE_encoding_tag, EncodingT> >::type
    >
    {
        static constexpr JPUnicodeEncoding value = JPUnicodeEncoding_UTF16BE;
    };
    
    template <typename EncodingT>
    struct jp_map_unicode_encoding<
        EncodingT,
        typename boost::enable_if<boost::is_same<UTF_16LE_encoding_tag, EncodingT> >::type
    >
    {
        static constexpr JPUnicodeEncoding value = JPUnicodeEncoding_UTF16LE;
    };
    
    
    
    template <typename EncodingT>
    struct jp_map_unicode_encoding<
        EncodingT,
        typename boost::enable_if<boost::is_same<UTF_32_encoding_tag, EncodingT> >::type
    >
    {
        static constexpr JPUnicodeEncoding value = JPUnicodeEncoding_UTF32;
    };
    
    template <typename EncodingT>
    struct jp_map_unicode_encoding<
        EncodingT,
        typename boost::enable_if<boost::is_same<UTF_32BE_encoding_tag, EncodingT> >::type
    >
    {
        static constexpr JPUnicodeEncoding value = JPUnicodeEncoding_UTF32BE;
    };
    
    template <typename EncodingT>
    struct jp_map_unicode_encoding<
        EncodingT,
        typename boost::enable_if<boost::is_same<UTF_32LE_encoding_tag, EncodingT> >::type
    >
    {
        static constexpr JPUnicodeEncoding value = JPUnicodeEncoding_UTF32LE;
    };
    
    
    //
    //  "Add" host endianness to JPUnicodeEncoding constants:
    //
    template <JPUnicodeEncoding C>
    struct jp_add_host_endianness {
        typedef typename json::unicode::add_endianness<typename jp_unicode_encoding_traits<C>::type>::type  unicode_tag;
        static constexpr JPUnicodeEncoding value = jp_map_unicode_encoding<unicode_tag>::value;
    };
    

}}



namespace {
    
    // Exception sync_parser_runtime_error
    // Thrown to indicate errors during the pre-parse phase (detecting
    // BOM, determining encoding, check for supported encoding, etc.
    class sync_parser_runtime_error : public std::runtime_error 
    {
    public:
        explicit sync_parser_runtime_error(const std::string& msg) 
        : std::runtime_error(msg)
        {}        
        virtual ~sync_parser_runtime_error() {}
    };
    
    
    // Run the parser (possibly in a parse loop, for multiple JSON documents).
    // The stream shall point to the first occurence of a code unit (that is,
    // it must not point to a BOM).
    // The streambuf's char_type shall be a "compatible" type for the
    // encoding (that is, sizeof(StreamBufferT::char_type) equals 
    // sizeof(EncodingT::code_unit_type).
    //
    // Returns true if the parser was successful, otherwise if a parse error
    // occured, returns false.
    //
    // The function may throw exceptions for unexpected errors.
    template <typename Iterator, typename EncodingT>
    bool run(Iterator& first, Iterator last, EncodingT encoding, SemanticActionsBase& sa) 
    {
        bool success = json::parse(first, last, encoding, sa);
        return success;    
    }
    
    
    
    
    // bool run(const char* first, const char* last, JPUnicodeEncoding encoding, 
    //          JPSemanticActionsBase* sa)
    //
    // If encoding is not JPUnicodeEncoding_Unknown detect the encoding of the 
    // byte sequence starting at first.
    // Run the parser with the appropriate iterators and encoding.
    //
    // Result:
    // Returns true if the parsing ran successfully, otherwise returns false if
    // the parsers detects any parsing errors.
    //
    // Throws:
    //     std::runtime_error:      "received EOF or broken BOM"
    //     std::runtime_error:      "unknown encoding - malformed Unicode"
    //     std::runtime_error:      "encoding not supported"
    //     std::runtime_error:      "streambuf seek failed"
    //     std::runtime_error:      "an unknown error has been occured"
    //
    //     Other expections may be thrown by the underlying parser.
    //
    bool run(const char* first, const char* last, JPUnicodeEncoding encoding, 
             JPSemanticActionsBase* sa)
    {
        assert(sa);
        
        int unicode_encoding = -1;
        if (encoding != JPUnicodeEncoding_Unknown) 
        {
            // If the encoding is explicitly specified, there must be no BOM!
            
            // Map JPUnicodeEncoding constant to json::unicode::UNICODE_ENCODING constants:
            if (encoding == JPUnicodeEncoding_UTF16) {
                encoding = json::objc::jp_add_host_endianness<JPUnicodeEncoding_UTF16>::value;
            }
            else if (encoding == JPUnicodeEncoding_UTF32) {
                encoding = json::objc::jp_add_host_endianness<JPUnicodeEncoding_UTF32>::value;
            }
            switch (encoding) {
                case JPUnicodeEncoding_UTF8:    unicode_encoding = json::unicode::UNICODE_ENCODING_UTF_8; break;
                case JPUnicodeEncoding_UTF16BE: unicode_encoding = json::unicode::UNICODE_ENCODING_UTF_16BE; break;
                case JPUnicodeEncoding_UTF16LE: unicode_encoding = json::unicode::UNICODE_ENCODING_UTF_16LE; break;
                case JPUnicodeEncoding_UTF32BE: unicode_encoding = json::unicode::UNICODE_ENCODING_UTF_32BE; break;
                case JPUnicodeEncoding_UTF32LE: unicode_encoding = json::unicode::UNICODE_ENCODING_UTF_32LE; break;
                default:
                    throw sync_parser_runtime_error("Parameter error: unknown encoding");
            }
        }
        else 
        {
            // Autodetect encoding
            // Check if there is a BOM using detect_bom() utility function. Possible 
            // results:
            // positive values in case of success:
            //    json::unicode::UNICODE_ENCODING_UTF_8    
            //    json::unicode::UNICODE_ENCODING_UTF_16BE
            //    json::unicode::UNICODE_ENCODING_UTF_16LE
            //    json::unicode::UNICODE_ENCODING_UTF_32BE
            //    json::unicode::UNICODE_ENCODING_UTF_32LE 
            //  zero, if no BOM
            //  and negative values in case of an error:
            //   -1: unexpected EOF
            const char* saved_first = first;
            int bom_result = json::unicode::detect_bom(first, last);
            if (bom_result < 0) {
                throw sync_parser_runtime_error("unexpecetd EOF while trying to determine BOM");
            } else if (bom_result == 0) {
                // We need to reset the iterator
                first = saved_first;
            }        
            // Now, the iterator charFirst should point to the first code unit.
            
            if (bom_result > 0) {
                unicode_encoding = bom_result;
                if ([sa respondsToSelector:@selector(setHasBOM:)]) {
                    [sa setHasBOM:YES];
                }
            }
            else
            {
                // There is no BOM.
                // Call the detect_encoding() utility function. (This reads ahead a 
                // few bytes, thus we must be able to reuse the iterators - which
                // is the case with our random access iterators.):
                // Return values:
                //   json::unicode::UNICODE_ENCODING_UTF_8
                //   json::unicode::UNICODE_ENCODING_UTF_16LE
                //   json::unicode::UNICODE_ENCODING_UTF_16BE
                //   json::unicode::UNICODE_ENCODING_UTF_32LE
                //   json::unicode::UNICODE_ENCODING_UTF_32BE
                //   -1:     unexpected EOF
                //   -2:     unknown encoding
                unicode_encoding = json::detect_encoding(first, last);
                if (unicode_encoding <= 0) {
                    if (unicode_encoding == -1)
                        throw sync_parser_runtime_error("unexpecetd EOF while trying to determine encoding");
                    else
                        throw sync_parser_runtime_error("unknown encoding - possibly malformed Unicode");
                }
            }
        }
        
        assert(unicode_encoding > 0);
        // After the above, we can say this:
        // The iterator 'first' points to the start of the first code unit. If
        // there was a BOM, we skipped it.
        bool  parser_success;
        SemanticActionsBase* sa_ptr = [(id)sa imp];  // (invoking base class method)
        assert(sa_ptr);
        if (unicode_encoding == json::unicode::UNICODE_ENCODING_UTF_8) {
            parser_success = run(first, last, json::unicode::UTF_8_encoding_tag(), *sa_ptr);
        }
        else {
            // We need to create new iterators which are appropriate for the encoding.
            switch (unicode_encoding) {
                case json::unicode::UNICODE_ENCODING_UTF_16BE: {
                    const uint16_t* first16 = reinterpret_cast<const uint16_t*>(first);
                    const uint16_t* last16 = first16 + (last-first)/sizeof(uint16_t);
                    parser_success = run(first16, last16, json::unicode::UTF_16BE_encoding_tag(), *sa_ptr);
                    break;
                }
                case json::unicode::UNICODE_ENCODING_UTF_16LE: {
                    const uint16_t* first16 = reinterpret_cast<const uint16_t*>(first);
                    const uint16_t* last16 = first16 + (last-first)/sizeof(uint16_t);
                    parser_success = run(first16, last16, json::unicode::UTF_16LE_encoding_tag(), *sa_ptr);
                    break;
                }
                case json::unicode::UNICODE_ENCODING_UTF_32BE: {
                    const uint32_t* first32 = reinterpret_cast<const uint32_t*>(first);
                    const uint32_t* last32 = first32 + (last-first)/sizeof(uint32_t);
                    parser_success = run(first32, last32, json::unicode::UTF_32BE_encoding_tag(), *sa_ptr);
                    break;
                }
                case json::unicode::UNICODE_ENCODING_UTF_32LE: {
                    const uint32_t* first32 = reinterpret_cast<const uint32_t*>(first);
                    const uint32_t* last32 = first32 + (last-first)/sizeof(uint32_t);
                    parser_success = run(first32, last32, json::unicode::UTF_32LE_encoding_tag(), *sa_ptr);
                    break;
                }
                default:
                    throw sync_parser_runtime_error("encoding not supported");
                    break;                    
            }
        }        
        return parser_success;
    }
    
    
}   // unnamed namespace



#pragma mark - JPJsonParser Private
@interface JPJsonParser ()

// Private
+ (BOOL) runWithBytes:(const void*) start 
               length:(size_t)length 
             encoding:(JPUnicodeEncoding)encoding
      semanticActions:(JPSemanticActionsBase*)sa;

@end



#pragma mark - JPJsonParser

@implementation JPJsonParser

// Private
//  Run the parser with the byte sequence starting at start and with length
//  bytes using the specified encoding.
//  If the encoding is not fully specified (encoding equals JPUnicodeEncoding_Unknown)
//  the encoding will be determined.
//  Parameter sa is an object instantiated from a sublass of JPSemanticActionsBase 
//  implementing the JPSemanticActionsProtocol protocol.
// 
//  and must not be nil.
//  The method is a simple wrapper abround the C-functions above to catch exceptions,
//  and set the error object.
//
+ (BOOL) runWithBytes:(const void*) start 
               length:(size_t)length 
             encoding:(JPUnicodeEncoding)encoding
      semanticActions:(JPSemanticActionsBase*)sa
{
    assert(sa);
    if (start == NULL or length <= 0 ) {
        [sa setErrorCode:json::JP_EMPTY_TEXT_ERROR 
             description:[NSString stringWithUTF8String:json::parser_error_str(json::JP_EMPTY_TEXT_ERROR)]];
        return NO;
    }
    
    const char* first = static_cast<const char*>(start);
    const char* last = first + length;    
    bool success = false;
    try {
        success = run(first, last, encoding, sa);
    }
    catch (sync_parser_runtime_error& ex) {
        [sa setErrorCode:json::JP_PARSER_CLIENT description:[NSString stringWithUTF8String:ex.what()]];
    }
    catch (SemanticActionsStateError& ex) {
        // error state shall be already set.
        assert([sa error] != nil);
    }
    catch (std::exception& ex) {            
        [sa setErrorCode:json::JP_UNEXPECTED_ERROR description:[NSString stringWithUTF8String:ex.what()]];
    }
    catch (...) {
        [sa setErrorCode:json::JP_UNKNOWN_ERROR description:[NSString stringWithUTF8String:json::parser_error_str(json::JP_UNKNOWN_ERROR)]];
    }
    return success;
}


// Synchronously parse a single json document from a NSString as input.
//
//  Notes:
//  The string should not contain large amount of text. The performance of
//  the implementation may not be optimal for this use case. For better
//  results of large amount of text use parseData: methods.
//
//  Parameter 'string' shall not be nil.
// 
+ (id) parseString:(NSString*)string 
           options:(JPJsonParserOptions)options 
             error:(__autoreleasing NSError**)error
{
    NSParameterAssert(string);
        
    // Use the "default semantic actions" aka "JPRepresentationGenerator".
    // Multiple documents cannot be parsed - silently clear the
    // flag "JPJsonParserParseMultipleDocuments" - if set.
    options &= ~JPJsonParserParseMultipleDocuments;
    
    
    id result;
    BOOL success;
    JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] initWithHandlerDispatchQueue:NULL];
    @autoreleasepool {
        // Since we don't need handlers, we do not need a dispatch queue, so
        // create a semantic actions object without a dispatch queue.
        [sa configureWithOptions:options];
        
        // Try to get the string's content in UTF-16:
        JPUnicodeEncoding encoding = JPUnicodeEncoding_Unknown;
        bool doFreeBuffer = false;
        const void* buffer = string ? CFStringGetCharactersPtr(CFStringRef(string)) : NULL;
        size_t length = buffer ? CFStringGetLength(CFStringRef(string))*2 : 0; // length equals number of bytes
        if (not buffer and string) {
            // attempt to get the internal buffer in UTF-8:
            buffer = CFStringGetCStringPtr(CFStringRef(string), kCFStringEncodingUTF8);
            if (buffer) {
                length = [string lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
                encoding = JPUnicodeEncoding_UTF8;
            } else {
                // No Unicode: performance is suboptimal.
                // Retrive the NSString's content as UTF-16 and store it into an allocated buffer:
                // We strongly assume, the endianess is platform:
                length = CFStringGetLength(CFStringRef(string))*2; // number of bytes
                void* buf = malloc(length);
                CFRange range = {0, static_cast<CFIndex>(length>>1)};
                CFStringGetCharacters(CFStringRef(string), range, (UniChar*)buf);
                buffer = buf;
                doFreeBuffer = true;
                encoding = JPUnicodeEncoding_UTF16;  // platform endianness
            }
        }
        success = [JPJsonParser runWithBytes:buffer
                                      length:length
                                    encoding:encoding
                             semanticActions:sa];
        if (doFreeBuffer) {
            free(const_cast<void*>(buffer));
        }
    }
    
    if (success) {
        // result is owned by sa only  - need to retain,autorelease it since we
        // dealloc sa.
        result = [sa result]; 
    } else {
        if (error)
            *error = [sa error];  // error returns an autorelease object
    }
    return result;
}


// Synchronously parse a single json document from a NSData object while
// autodetecting the input encoding.
+ (id) parseData:(NSData*)data 
         options:(JPJsonParserOptions)options
           error:(__autoreleasing NSError**)error
{
    // Use the "default semantic actions" aka "JPRepresentationGenerator".
    // Multiple documents cannot be parsed - silently clear the 
    // flag "JPJsonParserParseMultipleDocuments" - if set.
    options &= ~JPJsonParserParseMultipleDocuments;
    
    id result = nil;
    BOOL success;
    JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] initWithHandlerDispatchQueue:NULL];
    @autoreleasepool {
        // Since we don't need handlers, we do not need a dispatch queue, so
        // create a semantic actions object without a dispatch queue:
        [sa configureWithOptions:options];
        
        const void* bytes = static_cast<const void*>([data bytes]);
        success = [JPJsonParser runWithBytes:bytes
                                      length:[data length]
                                    encoding:JPUnicodeEncoding_Unknown
                             semanticActions:sa];
    }
    if (success) {
        // result is owned by sa only  - need to retain,autorelease it since we
        // dealloc sa.
        result = [sa result];
    } else {
        if (error)
            *error = [sa error]; // error may return an autoreleased object, but we don't know for sure ...
    }
    
    return result;
}


// Synchronously parse a single json document from an NSData object with
// a known encoding. There must not be a BOM preceeding the JSON content.
+ (id) parseData:(NSData*)data 
        encoding:(JPUnicodeEncoding)encoding
         options:(JPJsonParserOptions)options
           error:(__autoreleasing NSError**)error
{
    // Use the "default semantic actions" aka "JPRepresentationGenerator".
    // Multiple documents cannot be parsed - silently clear the 
    // flag "JPJsonParserParseMultipleDocuments" - if set.
    options &= ~JPJsonParserParseMultipleDocuments;
    
    id result = nil;
    BOOL success;
    JPRepresentationGenerator* sa = [[JPRepresentationGenerator alloc] initWithHandlerDispatchQueue:NULL];
    @autoreleasepool {
        // Since we don't need handlers, we do not need a dispatch queue, so
        // create a semantic actions object without a dispatch queue:
        [sa configureWithOptions:options];
        
        const void* bytes = static_cast<const void*>([data bytes]);
        success = [JPJsonParser runWithBytes:bytes
                                      length:[data length]
                                    encoding:JPUnicodeEncoding_Unknown
                             semanticActions:sa];
    }
    if (success) {
        // result is owned by sa only  - need to retain,autorelease it since we
        // dealloc sa.
        result = [sa result];
    } else {
        if (error)
            *error = [sa error]; // error may return an autoreleased object, but we don't know for sure ...
    }
    return result;
}



+ (BOOL) parseData:(NSData*)data semanticActions:(JPSemanticActionsBase*)sa
{
    NSParameterAssert(sa != nil);
    BOOL success;
    @autoreleasepool {
        const void* bytes = static_cast<const void*>([data bytes]);
        success = [JPJsonParser runWithBytes:bytes
                                      length:[data length]
                                    encoding:JPUnicodeEncoding_Unknown
                             semanticActions:sa];
    }
    return success;
}




#if NOT_YET_IMPLEMENTED

+ (BOOL) parseString:(NSString*)string semanticActions:(JPSemanticActionsBase*)sa
{
    typedef SemanticActions::error_t error_t;
    NSParameterAssert(sa != nil);
    
    bool success = false;
    @autoreleasepool {
        // Try to get the string's content in UTF-16:
        const void* bytes = CFStringGetCharactersPtr(CFStringRef(string));
        size_t length = 0; // length in bytes
#if 0
        // We cannot be sure we get a UTF-8 Encoding!!
        if (not bytes) {
            bytes = CFStringGetCStringPtr();  // UTF-8?
            length = [string lengthOfBytesUsingEncoding:NSUTF16StringEncoding];
        }
#endif
        if (not bytes) {
            // Performance may be suboptimal - even if we choose to use
            // UTF-16 encoding - this may CFString require to internally
            // allocate a contigues buffer and copy bytes.
            bytes = [string UTF8String];
            length = [string lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        }
        const char* first = static_cast<const char*>(bytes);
        const char* last = first + length;
        SemanticActions* sa_imp_ptr = [sa imp];
        assert(sa_imp_ptr !=  NULL);
        try {
            success = run(first, last, sa);
        }
        catch (sync_parser_runtime_error& ex) {
            sa_imp_ptr->error(error_t(json::JP_PARSER_CLIENT, ex.what()));
        }
        catch (std::exception& ex) {
            sa_imp_ptr->error(error_t(json::JP_UNEXPECTED_ERROR, ex.what()));
        }
        catch (...) {
            sa_imp_ptr->error(error_t(json::JP_UNKNOWN_ERROR,
                                      json::parser_error_str(json::JP_UNKNOWN_ERROR)));
        }
    }
    return success;
}

#endif // not yet implemented



@end

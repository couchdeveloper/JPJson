//
//  JPAsyncJsonParser.mm
//
//  Created by Andreas Grosam on 7/8/11.
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
#include "json/utility/syncqueue_streambuf.hpp"
#include "json/utility/synchronous_queue.hpp"
#include "json/utility/istreambuf_iterator.hpp"
#include "CFDataBuffer.hpp"
#import "JPAsyncJsonParser.h"
#import "JPSemanticActionsBase.h"
#import "JPSemanticActionsBase_private.h"
#import "JPRepresentationGenerator.h"  // Default Semantic Actions
#include <dispatch/dispatch.h>
#include "OSCompatibility.h"
#include <iterator>
#include <stdexcept>


namespace {
    
    using json::utility::synchronous_queue;
    using json::utility::basic_syncqueue_streambuf;

    typedef json::objc::CFDataBuffer<char>                  CFDataByteBuffer;
    typedef synchronous_queue<CFDataByteBuffer>             sync_queue_t;

    typedef basic_syncqueue_streambuf<CFDataByteBuffer, char>       char_streambuf_t;    
    typedef basic_syncqueue_streambuf<CFDataByteBuffer, uint16_t>   uint16_streambuf_t;    
    typedef basic_syncqueue_streambuf<CFDataByteBuffer, uint32_t>   uint32_streambuf_t;    

    typedef json::utility::istreambuf_iterator<char>                char_iterator;
    typedef json::utility::istreambuf_iterator<uint16_t>            uint16_iterator;
    typedef json::utility::istreambuf_iterator<uint32_t>            uint32_iterator;

    
    // Exception parser_runtime_error
    // Thrown to indicate errors during the pre-parse phase (detecting
    // BOM, determining encoding, check for supported encoding, etc.
    class parser_runtime_error : public std::runtime_error 
    {
    public:
        explicit parser_runtime_error(const std::string& msg) 
        : std::runtime_error(msg)
        {}        
        virtual ~parser_runtime_error() {}
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
    template <typename StreamBufferT, typename EncodingT>
    bool run(StreamBufferT* streambuffer, SemanticActionsBase& sa, EncodingT encoding) 
    {
        typedef typename StreamBufferT::char_type               char_type;        
        typedef json::utility::istreambuf_iterator<char_type>   iterator;
        
        assert(streambuffer);
        
        iterator eof;
        iterator iter(streambuffer);        
        bool success = json::parse(iter, eof, encoding, sa);
        return success;    
    }
    
    
    
    // bool run(sync_queue_t& syncQueue, SemanticActions& sa)
    // (or, a rather elaborated apporach to implement the encoding detection
    // when having InputIterators and streambufs)
    //
    //  1) Create a streambuf for type char and init it with the syncQueue.
    //     This streambuf is used to detect BOM and encoding.
    //  2) Wait until a buffer is available.
    //  3) Detect a BOM and strip it if there is any (this may consume buffers).
    //  4) If no BOM was found, determine the encoding using heuristic methods.
    //  5) Once the encoding is determined, create an appropriate streambuf for
    //     the type of code unit, and initialize it using the current queue's
    //     buffer (retrieving it from the char streambuf) and set the stream's
    //     position accordingly.
    //  6) Call the template function run()
    
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
    // Notes:
    // The function may require to look-ahead into the actual characters in
    // order to determine encoding. Thus, we require to be able to reset the
    // stream position to a previous position. This will usually succeed, but
    // one pathological case could occure when the size of the buffers were
    // unusual small - say for instance one byte. Here, we may not be able to
    // reset the stream pos. If this is the case, the function throws an 
    // exception.
    //
    bool run(sync_queue_t& syncQueue, JPSemanticActionsBase* sa)
    {
        assert(sa);
                
        // Create a local streambuf with char type and streambuf_iterators which
        // will be used to determine the encoding.
        typedef basic_syncqueue_streambuf<CFDataByteBuffer, char>     streambuf_t;  
        std::auto_ptr<char_streambuf_t> streambuf_ptr(new char_streambuf_t(syncQueue, -1.0));        
        json::utility::istreambuf_iterator<char> eof;
        json::utility::istreambuf_iterator<char> first(streambuf_ptr.get());
        
        // Check if there is a BOM using detect_bom() utility function. Possible 
        // results:
        // positive values in case of success:
        //        enum UNICODE_ENCODING {
        //            UNICODE_ENCODING_UTF_8 =    1,
        //            UNICODE_ENCODING_UTF_16BE = 2,
        //            UNICODE_ENCODING_UTF_16LE = 3,
        //            UNICODE_ENCODING_UTF_32BE = 4,
        //            UNICODE_ENCODING_UTF_32LE = 5
        //        };
        //  zero, if no BOM
        //  and negative values in case of an error:
        //   -1: unexpected EOF
        // (Note: this may require a look-ahead. Thus we must be able to reset
        // the stream - which may fail if we require to read several buffers)
        int bom_result = json::unicode::detect_bom(first, eof);
        if (bom_result < 0) {
            throw parser_runtime_error("unexpecetd EOF while trying to determine BOM");
        } else if (bom_result == 0) {
            // We need to reset the stream buf
            streambuf_t::streampos pos = streambuf_ptr->pubseekoff(0, std::ios_base::beg);
            if (pos == -1) {
                // we were unable to reset the streambuf - 
                throw parser_runtime_error("streambuf seek failed");
            }
        }
        
        // Now, the current pos of the streambuf should point to the first
        // code unit. Safe the current stream pos, we need it later:
        streambuf_t::streampos start_pos = streambuf_ptr->pubseekoff(0, std::ios_base::cur);
        
        int encoding = -1;
        if (bom_result > 0) {
            encoding = bom_result;
            if ([sa respondsToSelector:@selector(setHasBOM:)]) {
                [sa setHasBOM:YES];            
            }
        }
        else
        {
            // There is no BOM.
            // Call the detect_encoding() utility function. This reads ahead a 
            // few bytes, thus we must be able to reset the stream pos.
            // Return values:
            //   json::unicode::UNICODE_ENCODING_UTF_8
            //   json::unicode::UNICODE_ENCODING_UTF_16LE
            //   json::unicode::UNICODE_ENCODING_UTF_16BE
            //   json::unicode::UNICODE_ENCODING_UTF_32LE
            //   json::unicode::UNICODE_ENCODING_UTF_32BE
            //   -1:     unexpected EOF
            //   -2:     unknown encoding
            encoding = json::detect_encoding(first, eof);
            if (encoding <= 0) {
                if (encoding == -1)
                    throw parser_runtime_error("unexpecetd EOF while trying to determine encoding");
                else
                    throw parser_runtime_error("unknown encoding - possibly malformed Unicode");
            }
            // We got an encoding, but in order to be able to proceed with parsing, 
            // we need to reset the streambuf and create new iterators.
            streambuf_t::streampos pos = streambuf_ptr->pubseekoff(start_pos, std::ios_base::beg);
            if (pos == -1) {
                streambuf_ptr.release();
                // we were unable to reset the streambuf - 
                throw parser_runtime_error("streambuf seek failed");
            }
        }
        assert(encoding > 0);
        // After the above, we can say this:
        // The current stream pos points to the start of the first code unit. If
        // there was a BOM, we skipped it.
        bool  parser_success;
        SemanticActionsBase* sa_ptr = [sa imp];
        assert(sa_ptr);
        if (encoding == json::unicode::UNICODE_ENCODING_UTF_8) {
            parser_success = run(streambuf_ptr.get(), *sa_ptr, json::unicode::UTF_8_encoding_tag());
        }
        else {
            // We need to create a new streambuf which is appropriate for the encoding.
            // The streambuf still holds the current buffer, and the offset (in bytes) 
            // from the start of this buffer equals the difference of the streambuf's 
            // current pos and its base_pos:
            int byte_offset = (int)(streambuf_ptr->pubseekoff(0, std::ios_base::cur) - streambuf_ptr->base_pos());
            
            // In order to run, we need a streambuf whose CharT is appropriate for
            // the encoding's code_unit. We need a new streambuf, then passing it 
            // the (old) streambuf's current buffer, and adjust the new streambuf's
            // position with the byte_offset div /sizeof(CharT).  
            CFDataByteBuffer buffer = streambuf_ptr->buffer();
            streambuf_ptr.release(); // we can free the old stremabuf here.
            switch (encoding) {
                case json::unicode::UNICODE_ENCODING_UTF_16BE: {
                    std::auto_ptr<uint16_streambuf_t> streambuf_ptr(new uint16_streambuf_t(syncQueue, buffer, -1.0));
                    streambuf_t::streampos pos = streambuf_ptr->pubseekoff(byte_offset/sizeof(uint16_t), std::ios_base::beg);
                    if (pos == -1) {
                        streambuf_ptr.release();
                        // we were unable to reset the streambuf - 
                        throw parser_runtime_error("streambuf seek failed");
                    }
                    parser_success = run(streambuf_ptr.get(), *sa_ptr, json::unicode::UTF_16BE_encoding_tag());
                    break;
                }
                case json::unicode::UNICODE_ENCODING_UTF_16LE: {
                    std::auto_ptr<uint16_streambuf_t> streambuf_ptr(new uint16_streambuf_t(syncQueue, buffer, -1.0));
                    streambuf_t::streampos pos = streambuf_ptr->pubseekoff(byte_offset/sizeof(uint16_t), std::ios_base::beg);
                    if (pos == -1) {
                        streambuf_ptr.release();
                        // we were unable to reset the streambuf - 
                        throw parser_runtime_error("streambuf seek failed");
                    }
                    parser_success = run(streambuf_ptr.get(), *sa_ptr, json::unicode::UTF_16LE_encoding_tag());
                    break;
                }
                case json::unicode::UNICODE_ENCODING_UTF_32BE: {
                    std::auto_ptr<uint32_streambuf_t> streambuf_ptr(new uint32_streambuf_t(syncQueue, buffer,  -1.0));
                    streambuf_t::streampos pos = streambuf_ptr->pubseekoff(byte_offset/sizeof(uint32_t), std::ios_base::beg);
                    if (pos == -1) {
                        streambuf_ptr.release();
                        // we were unable to reset the streambuf - 
                        throw parser_runtime_error("streambuf seek failed");
                    }
                    parser_success = run(streambuf_ptr.get(), *sa_ptr, json::unicode::UTF_32BE_encoding_tag());
                    break;
                }
                case json::unicode::UNICODE_ENCODING_UTF_32LE: {
                    std::auto_ptr<uint32_streambuf_t> streambuf_ptr(new uint32_streambuf_t(syncQueue, buffer, -1.0));
                    streambuf_t::streampos pos = streambuf_ptr->pubseekoff(byte_offset/sizeof(uint32_t), std::ios_base::beg);
                    if (pos == -1) {
                        streambuf_ptr.release();
                        // we were unable to reset the streambuf - 
                        throw parser_runtime_error("streambuf seek failed");
                    }
                    parser_success = run(streambuf_ptr.get(), *sa_ptr, json::unicode::UTF_32LE_encoding_tag());
                    break;
                }
                default:
                    throw parser_runtime_error("encoding not supported");
                    break;                    
            }
        }        
        return parser_success;
    }
    
    
}   // unnamed namespace




#pragma mark - JPAsyncJsonParser

@implementation JPAsyncJsonParser {
    sync_queue_t                            syncQueue_;
    JPSemanticActionsBase*                  sa_;
    dispatch_queue_t                        workerDispatchQueue_;
    dispatch_semaphore_t                    idle_;
    bool                                    killed_;
    bool                                    finished_;
}

@synthesize bufferQueueCapacity;
@synthesize bufferQueueSize;
@synthesize semanticActions = sa_;

//
// Designated initializer.
//
- (id) initWithSemanticActions:(JPSemanticActionsBase*)sa
            workerDispatchQueue:(dispatch_queue_t)workerQueue
{
    self = [super init];
    if (self) { 
        if (sa == nil) {
            // Create the default semantic actions object:
            dispatch_queue_t handlerDispatchQueue = dispatch_queue_create("com.JPAsyncParser.handler_queue", NULL);
            sa_ = [[JPRepresentationGenerator alloc] initWithHandlerDispatchQueue:handlerDispatchQueue];
            JP_DISPATCH_RELEASE(handlerDispatchQueue);
        } else {
            sa_ = sa;
        }
        if (workerQueue == NULL) {
            workerDispatchQueue_ = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0); 
        } else {
            workerDispatchQueue_ = workerQueue;
            JP_DISPATCH_RETAIN(workerDispatchQueue_);
        }     
        idle_ = dispatch_semaphore_create(1);        
    }
    return self;
}


- (id) init 
{
    self = [self initWithSemanticActions:NULL workerDispatchQueue:NULL];
    return self;    
}


- (void) dealloc 
{
    if (workerDispatchQueue_) {
        JP_DISPATCH_RELEASE(workerDispatchQueue_);
    }
    if (idle_) {
        JP_DISPATCH_RELEASE(idle_);
    }
    
}




// If not yet invoked already, it starts parseing single or multiple JSON 
// documents using the input text provided by the synchronous queue.
// start can be called on any thread. 
// start should only be calle once. Subsequent calls have no effect.

- (BOOL) start
{
    if (dispatch_semaphore_wait(idle_, DISPATCH_TIME_NOW) != 0) {
        return NO;
    }    
    if (killed_) {
        return NO;
    }
    NSParameterAssert(sa_);
    dispatch_async(workerDispatchQueue_, ^{
        @autoreleasepool {
        
            SemanticActionsBase* sa_imp_ptr = [sa_ imp];
            assert(sa_imp_ptr !=  NULL);
            bool success = false;
            try {
                success = run(syncQueue_, sa_);
            }
            catch (parser_runtime_error& ex) {
                typedef SemanticActionsBase::error_t error_t;
                killed_ = true;
                sa_imp_ptr->error(json::JP_PARSER_CLIENT, ex.what());
            }
            catch (std::exception& ex) {            
                typedef SemanticActionsBase::error_t error_t;
                killed_ = true;
                sa_imp_ptr->error(json::JP_UNEXPECTED_ERROR, ex.what());
            }
            catch (...) {
                typedef SemanticActionsBase::error_t error_t;
                killed_ = true;
                sa_imp_ptr->error(json::JP_UNKNOWN_ERROR, 
                                          json::parser_error_str(json::JP_UNKNOWN_ERROR));
            }
            if (not success) 
            {
                // We should cancel any waiting producers, as well clearing the
                // buffer queue.
                while (syncQueue_.get(0.0).first == sync_queue_t::OK) 
                {
#if defined (DEBUG)        
                    NSLog(@"JPAsyncJsonParser removed buffer from synchronous queue");
#endif                
                }                        
#if defined (DEBUG)        
                NSLog(@"JPAsyncParser did fail with error %@", [sa_ error]);
#endif
            }
            dispatch_semaphore_signal(idle_);
            finished_ = true;        
        
        }
    });   
    
    return YES;
}


// Cancel the parser
// 
//  Cancelation might be tricky!
// 
// In order to cancel the parser which lets it exit as soon as possible rather 
// than when encountering a timeout, it should receive an EOF or otherwise 
// encounter an error in the input. This can be achieved by erasing the 
// buffers in the queue waiting for processing and putting a nil buffer 
// into the queue. Since produceBuffer: may block, and cancel could be invoked 
// on the main thread we avoid blocking by dispatching the statements onto a 
// concurrent dispatch queue.
- (void) cancel
{
    // If the parser is idle, return immediately ...
    long result = dispatch_semaphore_wait(idle_, DISPATCH_TIME_NOW);
    if (result == 0) {
        dispatch_semaphore_signal(idle_);
        return;
    }
    
    // ... otherwise, the parser's thread is executing:    
    killed_ = true;
    
    // Send the semantic actions object a cancel message. This method simply sets 
    // a flag, which can be concurrently accessed by the sa and the parser. The
    // parser accesses it at its next cancelation point (currently, this is at 
    // the start of new json text).
    // If the parser detects this cancel flag, it will stop and the parser's 
    // thread will eventually exit. The method can be seen as an asynchronous 
    // invocation, since the parser might still executing after returning from 
    // the method.
    if ([sa_ respondsToSelector:@selector(cancel)]) {
        [sa_ cancel];
    }
    
    // Nonetheless, the parser might still running ...
    // eat all produced buffers until nothing is offered:
    while (syncQueue_.get(0.0).first == sync_queue_t::OK) {
        NSLog(@"JPAsyncJsonParser cancel: removed buffer");
    }
    
    
    // nonetheless, the parser might still running ...
    // If the parser is possibly waiting for new text, send it a nil buffer:
    // We need to be sure the parser did receive a nil buffer or it is 
    // not running anymore:
    while ( (syncQueue_.put(CFDataByteBuffer(CFDataRef(nil)), 1.0)) != sync_queue_t::OK) {
        // buffer was not consumed - check if the parsers is running at all:
        if ([self isRunning] == NO) {
            return;
        }
    }    
    
    // the nil-buffer has been consumed - wait until the parser's thread exits:
    // Usually, exiting should happen within milli seconds. Otherwise, this is a
    // sign of a bad error somewhere.
    int count = 0;
    while ((result = dispatch_semaphore_wait(idle_, dispatch_time(DISPATCH_TIME_NOW, 1*NSEC_PER_SEC)))) 
    {   
        // timed out
        // TODO: use logger facility
        ++count;
        if (count > 10) {
            NSLog(@"ERROR: JPAsyncJsonParser cancel: waiting for parser thread to exit failed. Giving up.");
            return;
        }
        NSLog(@"WARNING: JPAsyncJsonParser cancel: waiting for parser thread to exit");
    }
    if (count) {
        NSLog(@"JPAsyncJsonParser cancel: parser thread did exit");
    }
    if (result == 0) {
        dispatch_semaphore_signal(idle_);
        return;
    }    
}


- (BOOL) isRunning {
    long result = dispatch_semaphore_wait(idle_, DISPATCH_TIME_NOW);
    if (result == 0) {
        dispatch_semaphore_signal(idle_);
    }
    return result != 0;
}



- (BOOL) parseBuffer:(NSData*)buffer
{
    if (killed_ or finished_) {
        return NO;
    }
    
    // Try putting the buffer. Be patient for 5 seconds. If put failed, 
    // check if the receiver was killed, and if so return NO, otherwise
    // retry.
    int timedout = 0;
    while (syncQueue_.put(CFDataByteBuffer((__bridge CFDataRef)buffer), 5.0) == sync_queue_t::TIMEOUT_NOT_DELIVERED)
    {
        if (killed_  or finished_) {
            return NO;
        }
        NSLog(@"Caution: JPAsyncJsonParser: SyncQueue timed out (5.0 sec) while attempting to put a buffer. Retrying ...");
        ++timedout;
    }
    if (timedout) {
        NSLog(@"JPAsyncJsonParser: ... SyncQueue 'put' finally succeeded after %d timeouts (a 5 sec).", timedout);
    }
    return YES;
}

@end

//
//  JPNSDataBuffers.mm
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

#warning deprecated file

#import "JPNSDataBuffers.h"
#import "JPNSDataBuffers_private.hpp"


#define LOG_DEBUG


@implementation JPNSDataBuffers
{
    ConstBuffers_t* buffers_;
    double timeout_;
}

@synthesize timeout = timeout_;

- (id) initWithMaxBuffers:(NSUInteger)maxBuffers
{
    self = [super init];
    if (self) {
        buffers_ = new ConstBuffers_t(maxBuffers);
        timeout_ = 60;  // 60 secs
    }
    
    return self;
}

- (void)dealloc
{
    [self commitAllBuffers];
    delete buffers_;
    [super dealloc];
}


- (BOOL) produceBuffer:(NSData*) buffer
{
#if defined (LOG_DEBUG)    
    NSLog(@"JPNSDataBuffers: produceBuffer NSData(%p), size: %lu", buffer, (unsigned long)[buffer length]);
#endif    
    CFDataBuffer_t b = CFDataBuffer_t(reinterpret_cast<CFDataRef>(buffer));
    BOOL success = buffers_->produce(b, timeout_);
#if defined (LOG_DEBUG)
    if (!success) {
        NSLog(@"JPNSDataBuffers: produceBuffer NSData(%p) timed out", buffer);
    }
#endif    
    return success;
}

- (void) commitBuffer:(NSData*) buffer
{
    CFDataBuffer_t tmp = CFDataBuffer_t(reinterpret_cast<CFDataRef>(buffer));
    buffers_->commit(&tmp);
}

// May return an NSData object whose size equals zero.
// Returns nil if a timeout occurred.
- (NSData*) aquireBuffer;
{
    CFDataBuffer_t* buf = buffers_->aquire(timeout_);
    NSData* result;
    if (buf) {
        id data = id(buf->get_CFDataRef());        
        result = data ? data : [NSData data];
    } else {
        result = nil;
    }
#if defined (LOG_DEBUG)    
    NSLog(@"JPNSDataBuffers: aquireBuffer NSData(%p), size: %lu", result, (unsigned long)([result length]));
#endif    
    
    return result;
}

- (size_t) avail {
    return buffers_->avail();
}

- (size_t) processing {
    return buffers_->processing();
}

- (BOOL) empty {
    return buffers_->empty();
}

- (void) clear {
    buffers_->clear();
}

- (void) commitAllBuffers;
{
    buffers_->commit();
}

- (void) lock {
    buffers_->lock();
}

- (void) unlock {
    buffers_->unlock();
}


// private


- (ConstBuffers_t*) buffers_imp
{
    return buffers_;
}

@end

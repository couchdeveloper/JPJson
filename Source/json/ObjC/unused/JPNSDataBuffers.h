//
//  JPNSDataBuffers.h
//  json_parser
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

#ifndef JSON_OBJC_JP_NSDATA_BUFFERS_H
#define JSON_OBJC_JP_NSDATA_BUFFERS_H

#warning deprecated file

#import <Foundation/Foundation.h>


@interface JPNSDataBuffers : NSObject

- (id) initWithMaxBuffers:(NSUInteger)maxBuffers;


@property (nonatomic, assign) double timeout;

- (BOOL) produceBuffer:(NSData*) buffer;
- (void) commitBuffer:(NSData*) buffer;
- (NSData*) aquireBuffer;

// Returns the number of buffers currently processed. Note that the
// result may be possibly stale immediately after the method returns.
- (size_t) processing;

// Returns the number of buffers which still needs to be processed. Note that the
// result may be possibly stale immediately after the method returns.
- (size_t) avail;

// Returns YES if the avail == 0 and processing == 0. Note that the
// result may be possibly stale immediately after the method returns.
- (BOOL) empty;

// Clears the avail queue.
- (void) clear;

// Commits any remaining uncommitted buffers.
- (void) commitAllBuffers;

// Blocks the current thread until the buffer queue becomes empty, or
// until the timeout expired. Returns true if the queue was empty at the
// time it was checked.
//- (bool) waitUntilEmpyt:(double)timeout_seconds;

@end


#endif

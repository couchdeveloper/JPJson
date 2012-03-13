//
//  JPSynchronousQueue.mm
//
//  Created by Andreas Grosam on 5/18/11.
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


#import "JPSynchronousQueue.h"
#include "json/utility/synchronous_queue.hpp"

typedef json::utility::simple_synchronous_queue<id> queue_t; 



@implementation SynchronousQueue {
    queue_t queue_;
}

- (id)init {
    self = [super init];
    if (self) {
    }
    return self;
}

- (void) putData:(id) data {
    queue_.put([data retain]);
}


- (int) putData:(id)data withTimeout:(double) timeout
{
    queue_t::result_type result = queue_.put([data retain], timeout);
    switch (result) {
        case queue_t::OK: return SYNC_QUEUE_OK;
        case queue_t::TIMEOUT_NOT_DELIVERED: return SYNC_QUEUE_TIMEOUT_NOT_DELIVERED;
        case queue_t::TIMEOUT_NOT_PICKED: return SYNC_QUEUE_TIMEOUT_NOT_PICKED;
        default: return -111;
    }
}

- (id) getData {
    id result = queue_.get();
    return [result autorelease];
}


// Returns nil if nothing was offered until timeout
- (id) getDataWithTimeout:(double) timeout
{
    std::pair<queue_t::result_type, id> result = queue_.get(timeout);
    if (result.first != queue_t::OK) {
        return nil;
    } else {
        return [result.second autorelease];
    }
}

- (id) acquireData {
    return queue_.aquire();
}

- (id) acquireDataWithTimeout:(double) timeout
{
    std::pair<queue_t::result_type, id> result = queue_.aquire(timeout);
    if (result.first != queue_t::OK) {
        return nil;
    } else {
        return result.second;
    }
}

- (void) commitData:(id)data {
    [data release];
    queue_.commit();
}




@end

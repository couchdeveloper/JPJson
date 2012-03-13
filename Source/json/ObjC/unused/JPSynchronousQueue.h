//
//  JPSynchronousQueue.h
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

#import <Foundation/Foundation.h>


enum {
    SYNC_QUEUE_OK = 0,
    SYNC_QUEUE_TIMEOUT_NOT_DELIVERED = -1,
    SYNC_QUEUE_TIMEOUT_NOT_PICKED = -2
};



@interface JPSynchronousQueue : NSObject


- (void) putData:(NSData*) data;

- (int) putData:(NSData*)data withTimeout:(double) timeout;

- (NSData*) getData;

- (NSData*) getDataWithTimeout:(double) timeout;

- (NSData*) acquireData;

- (NSData*) acquireDataWithTimeout:(double) timeout;

- (void) commitData:(NSData*)data;


@end


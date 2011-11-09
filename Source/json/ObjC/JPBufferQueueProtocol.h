//
//  JPBufferQueueProtocol.h
//  JPJsonLib
//
//  Created by Andreas Grosam on 9/13/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


// Implementations shall meet the following requirements:
//
// Thread safe, blocking FIFO queue with support for resource utilization.
//
// The queue shall be capable to handle one producer and one consumer
// concurrently, each running in a different thread.

// - (size_t) capacity;
// 
// Returns the capacity of the receiver. The capacity determines the maximum
// of available and processed objects until after producers will be blocked
// in produce: after adding objects to the receiver.

//
// 
// - (BOOL) produce:(id) object
//
// Add an object 'object' to the queue which shall become available for 
// consuming. When the method enters, it increments avail (and increments size).
// The method shall block until after size is less or equal than capacity.
// Returns YES if the block has become available and can be consumed. Otherwise 
// if it returns NO, this may indicate a timeout. If the result is NO, it must 
// be guaranteed that object will never be consumed.
// object will be retained by the receiver.


// - (id) aquire
//
// Returns the next available object from the queue. If an object is available, 
// it will immediately return this object. Otherwise, the calling thread will be 
// blocked until one object becomes available, or a timeout occurs. If a timeout 
// occurred, aquire will return nil.
// If aquire returns an object, processing will be incremented and avail will be
// decremented.


// - (void) commit:(id) object
//
// commit: shall be send to the receiver from the consumer when it has finished 
// processing the object. On exit, processing and size will be decremented, and
// object will be released by the receiver.


// - (size_t) processing
//
// Returns the number of objects currently processed by consumers. Note that the
// result may be possibly stale immediately after the method returns.


// - (size_t) avail
//
// Returns the number of objects which still need to be processed. Note that the
// result may be possibly stale immediately after the method returns.


// - (size_t) size
//
// Returns the sum of the available objects and objects currently processed. 
// Note that the result may be possibly stale immediately after the method 
// returns.


//- (BOOL) empty

// Returns YES if avail equals zero and processing equals zero. Note that the
// result may be possibly stale immediately after the method returns.



// - (void) clear
//
// Clears the avail queue.


// - (void) commitAll
//
// Commits any remaining uncommitted buffers.



@protocol JPBufferQueueProtocol <NSObject>

- (size_t)  capacity;
- (BOOL)    produce:(id) object;
- (id)      aquire;
- (void)    commit:(id) object;

@optional
- (size_t)  processing;
- (size_t)  avail;
- (size_t)  size;
- (void)    clear;
- (void)    commitAll;

@end

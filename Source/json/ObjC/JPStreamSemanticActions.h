//
//  JPStreamSemanticActions.h
//  
//
//  Created by Andreas Grosam on 8/24/11.
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

#import "JPSemanticActionsBase.h"


/**

 JPStreamSemanticActions is a base class for semantic actions classes 
 implementing an event-based sequential access parser API.
 
 The API is documented in protocol `JPSemanticActionsProtocol`. 
 For more information see <JPSemanticActionsProtocol>
 
 */


@interface JPStreamSemanticActions : JPSemanticActionsBase


/** 
 *Designated Initializer*
 
 Initializes a `JPStreamSemanticActions` instance with the specified dispatch queue.
 If parameter `handlerQueue` equals `nil` no dispatch queue will be used and no
 handler blocks will be scheduled.
 
 @param handlerQueue The dispatch queue where handler blocks will be scheduled,
 or `nil`, in which case no handlers will be executed.
 */
- (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue;



@end

//
//  mutex.hpp
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

#ifndef JSON_OBJC_GCD_MUTEX_HPP
#define JSON_OBJC_GCD_MUTEX_HPP


#include "json/config.hpp"
#include <dispatch/dispatch.h>
#include <boost/utility.hpp>
#include <assert.h>

#if defined(DEBUG)
#include <iostream>
#endif


namespace json { namespace objc { namespace gcd {
    
    
    class mutex : boost::noncopyable {
    public:
        mutex() : sem_(dispatch_semaphore_create(1)) {
            assert(sem_);
        }
        ~mutex() { dispatch_release(sem_); }
        void lock() {
#if defined (DEBUG)
            uint64_t timeout = NSEC_PER_SEC;
            uint64_t locktime = 0;
            while (dispatch_semaphore_wait(sem_, dispatch_time(DISPATCH_TIME_NOW, timeout)))
            {
                locktime += (timeout/NSEC_PER_SEC);
                printf("WARNING: mutex %p locking for %llu seconds\n", this, locktime);
                if (timeout < 64*NSEC_PER_SEC)
                    timeout *= 2;
            }
#else
            dispatch_semaphore_wait(sem_, DISPATCH_TIME_FOREVER);
#endif                
            
        }
        void unlock() { 
            dispatch_semaphore_signal(sem_); 
        }
    private:
        dispatch_semaphore_t sem_;
    };
    
    
    
}}} // namespace json::objc::gcd    


#endif

//
//  semaphore.hpp
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

#ifndef JSON_OBJC_GCD_SEMAPHORE_HPP
#define JSON_OBJC_GCD_SEMAPHORE_HPP


#include "json/config.hpp"
#include <dispatch/dispatch.h>
#include <cassert>
#include <stdexcept>
#include <iostream>


namespace json { namespace objc { namespace gcd {
    
    
    class semaphore {
    public:
        typedef double      duration_type;

        
        static duration_type wait_forever() { return -1.0; }
        
        semaphore(const semaphore&) = delete;
        semaphore& operator=(const semaphore&) = delete;
        
        explicit semaphore(long n) : sem_(dispatch_semaphore_create(n)) {
            assert(sem_);
        }
        
        ~semaphore() {
            dispatch_semaphore_t tmp = sem_;
            sem_ = 0;
            int count = 0;
            while (dispatch_semaphore_signal(tmp)) {
                ++count;
                usleep(100);  // this is primarily a workaround for a bug in lib dispatch
#if defined (DEBUG)                
                std::cerr << "semaphore: resumed waiting thread in d-tor" << std::endl;
#endif                
            }
            dispatch_release(tmp);
        }
        
        void signal()  { 
            dispatch_semaphore_signal(sem_); 
        }
        
        bool wait()  { 
            long result = dispatch_semaphore_wait(sem_, DISPATCH_TIME_FOREVER);
            if (sem_ == 0) {
                throwInterrupted();
            }
            return result == 0;
        }
        
        bool wait(semaphore::duration_type timeout_sec)  { 
            long result = dispatch_semaphore_wait(sem_, 
                                                  timeout_sec >= 0 ? 
                                                  dispatch_time(DISPATCH_TIME_NOW, timeout_sec*NSEC_PER_SEC) 
                                                  : DISPATCH_TIME_FOREVER);
            if (sem_ == 0) {
                throwInterrupted();
            }
            return result == 0;
        }
        
    private:
        void throwInterrupted() {
            //throw std::runtime_error("interrupted");
        }
        
    private:
        dispatch_semaphore_t sem_;
    };
    
    
}}} // namespace json::objc::gcd    


#endif  // JSON_OBJC_GCD_SEMAPHORE_HPP

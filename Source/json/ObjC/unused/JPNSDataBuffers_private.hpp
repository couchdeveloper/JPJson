//
//  JPNSDataBuffers_private.hpp
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

#ifndef JSON_OBJC_JP_NSDATABUFFERS_PRIVATE_HPP
#define JSON_OBJC_JP_NSDATABUFFERS_PRIVATE_HPP

#warning deprecated file

#include "json/utility/buffer_queue.hpp"
#include "CFDataBuffer.hpp"
#include "semaphore.hpp"
#include "mutex.hpp"


using json::buffer_queue;
using json::objc::CFDataBuffer;
using json::objc::gcd::semaphore;
using json::objc::gcd::mutex;


typedef CFDataBuffer<char> CFDataBuffer_t;
typedef buffer_queue<CFDataBuffer_t, mutex, semaphore> ConstBuffers_t;


@interface JPNSDataBuffers (Private)

- (ConstBuffers_t*) buffers_imp;

@end




#endif // JSON_OBJC_JPNSDATABUFFERS_PRIVATE_HPP
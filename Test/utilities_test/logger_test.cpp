//
//  logger_test.cpp
//  Test
//
//  Created by Andreas Grosam on 8/4/11.
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

#include "json/utility/simple_log.hpp"
#include <gtest/gtest.h>

#include <iomanip>


// for testing

using namespace json;



namespace {
    
    using json::utility::logger;
    
    class logger_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        logger_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~logger_test() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
#pragma mark -
#pragma mark Unicode Code Point

    TEST_F(logger_test, internal) {
        
        
        int process_id =  json::utility::internal::processID();
        int thread_id = json::utility::internal::threadID();
        const char* executable_name = json::utility::internal::executableName();
        
        printf("process_id: %d, thread_id %x, executable_name \"%s\"\n",
               process_id, thread_id, executable_name);
    } 
    
    
    
    

    TEST_F(logger_test, logging) 
    {        
        logger<utility::log_error> logger;
        
        logger.log(utility::LOG_ERROR, "ERROR: %s", "test");
        logger.log(utility::LOG_WARNING, "WARNING: %s", "test");
        logger.log(utility::LOG_FATAL, "FATAL: %s", "test");
        
        logger.log_level(utility::LOG_LEVEL_FATAL);
        logger.log(utility::LOG_ERROR, "ERROR: %s", "test");
        
    } 
    

}

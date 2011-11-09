//
//  semaphore_test.cpp
//  Test
//
//  Created by Andreas Grosam on 9/20/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//
#include "json/ObjC/semaphore.hpp"

#include "gtest/gtest.h"

#include <iostream>
#include <iomanip>
#include <dispatch/dispatch.h>




namespace {
    
    using json::objc::gcd::semaphore;
    
    class semaphore_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        semaphore_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~semaphore_test() {
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
    
    TEST_F(semaphore_test, CrashTest) {
        
        semaphore* sem_ptr = new semaphore(0);

        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            //std::cout << "Enter 1" << std::endl;
            sem_ptr->wait();
            /*
            if (sem_ptr == NULL) 
                std::cerr << "1 interrupted" << std::endl;
            else
                std::cout << "Exit 1" << std::endl;
             */
        });

        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            //std::cout << "Enter 2" << std::endl;
            
            sem_ptr->wait();
            /*
            if (sem_ptr == NULL) 
                std::cerr << "2 interrupted" << std::endl;
            else
                std::cout << "Exit 2" << std::endl;
             */
        });
        
        bool success = sem_ptr->wait(2.0);
        if (not success) {
            std::cout << "timed out" << std::endl;
            semaphore* tmp = sem_ptr;
            sem_ptr = 0;
            usleep(1000*200);
            delete tmp;
        }
        if (sem_ptr) {
            delete sem_ptr;
            sem_ptr = 0;
        }

        usleep(1000*200);
        std::cout << "finished." << std::endl;
    } 
    
}
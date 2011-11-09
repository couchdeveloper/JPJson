//
//  main.c
//  Allocator_bench
//
//  Created by Andreas Grosam on 9/20/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include "utilities/timer.hpp"
//#include "Allocators/dl_malloc.h"

#include <vector>

using utilities::timer;



static void test1(const int N, const int S) 
{
    typedef std::vector<void*> pointer_vector;
    
    printf("-------------------------------------------------------------------\n"
           "                     Default CFAllocator \n"
           " Allocating and destryoing %d objects with size %d\n"
           "-------------------------------------------------------------------\n",
           N, S);
    
    CFAllocatorRef defaultAllocator = CFAllocatorGetDefault();
    
    pointer_vector pv;
    pv.reserve(N);
    
    timer tc;
    tc.start();
    for (int i = 0; i < N; ++i) {
        pv.push_back(0);
    }
    tc.stop();
    pv.clear();
    pv.reserve(N);
    
    timer t;
    t.start();
    for (int i = 0; i < N; ++i) {
        void* p = CFAllocatorAllocate(defaultAllocator, S, 0);
        pv.push_back(p);
    }
    t.stop();
    
    double ta = t.seconds() - tc.seconds();
    
    timer td;
    td.start();
    
    pointer_vector::iterator first = pv.begin();
    pointer_vector::iterator last = pv.end();
    while (first != last) {
        CFAllocatorDeallocate(defaultAllocator, *first++);
    }
    td.stop();
    
    
    //double ta = t.seconds();
    
    printf("Allocate: %.3fms", ta*1.0e3);
    printf("\nDestroy:  %.3fms\n\n", td.seconds()*1.0e3);
}



static void test2(const int N, const int S) 
{
    typedef std::vector<void*> pointer_vector;
    
    printf("-------------------------------------------------------------------\n"
           "                     malloc and free \n"
           " Allocating and destryoing %d objects with size %d\n"
           "-------------------------------------------------------------------\n",
           N, S);
        
    pointer_vector pv;
    pv.reserve(N);
    
    timer tc;
    tc.start();
    for (int i = 0; i < N; ++i) {
        pv.push_back(0);
    }
    tc.stop();
    pv.clear();
    pv.reserve(N);
    
    timer t;
    t.start();
    for (int i = 0; i < N; ++i) {
        void* p = malloc(S);
        pv.push_back(p);
    }
    t.stop();
    
    double ta = t.seconds() - tc.seconds();
    
    timer td;
    td.start();
    
    pointer_vector::iterator first = pv.begin();
    pointer_vector::iterator last = pv.end();
    while (first != last) {
        free(*first++);
    }
    td.stop();
    
    
    //double ta = t.seconds();
    
    printf("Allocate: %.3fms", ta*1.0e3);
    printf("\nDestroy:  %.3fms\n\n", td.seconds()*1.0e3);
}


static void test3(const int N, const int S) 
{
    typedef std::vector<char*> charp_vector;
    
    printf("-------------------------------------------------------------------\n"
           "                     new and delete \n"
           " Allocating and destryoing %d objects with size %d\n"
           "-------------------------------------------------------------------\n",
           N, S);
    
    charp_vector pv;
    pv.reserve(N);
    
    timer tc;
    tc.start();
    for (int i = 0; i < N; ++i) {
        pv.push_back(0);
    }
    tc.stop();
    pv.clear();
    pv.reserve(N);
    
    timer t;
    t.start();
    for (int i = 0; i < N; ++i) {
        char* p = new char[S];
        pv.push_back(p);
    }
    t.stop();
    
    double ta = t.seconds() - tc.seconds();
    
    timer td;
    td.start();
    
    charp_vector::iterator first = pv.begin();
    charp_vector::iterator last = pv.end();
    while (first != last) {
        delete [] (*first);
        ++first;
    }
    td.stop();
    
    
    //double ta = t.seconds();
    
    printf("Allocate: %.3fms", ta*1.0e3);
    printf("\nDestroy:  %.3fms\n\n", td.seconds()*1.0e3);
}



int main (int argc, const char * argv[])
{
    test1(100000, 48);
    test2(100000, 48);
    test3(100000, 48);
    
    test1(100000, 48);
    test2(100000, 48);
    test3(100000, 48);
    
    return 0;
}


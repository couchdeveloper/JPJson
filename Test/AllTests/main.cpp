//
//  main.cpp
//  AllTests
//
//  Created by Andreas Grosam on 11/3/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include <gtest/gtest.h>

GTEST_API_ int main(int argc, char **argv) 
{
    std::cout << "Running main() from gtest_main.cc\n";
    
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << std::endl;
    }
    
    //::testing::GTEST_FLAG(print_time) = false;    
    
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

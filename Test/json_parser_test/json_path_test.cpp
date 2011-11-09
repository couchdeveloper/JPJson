//
//  json_path_test.cpp
//  Test
//
//  Created by Andreas Grosam on 10/20/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "json/parser/json_path.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <iomanip>
#include "json/unicode/unicode_utilities.hpp"

namespace {
    
    using json::json_internal::json_path;
    using json::internal::big_endian_tag;
    using json::internal::little_endian_tag;
    using json::unicode::UTF_8_encoding_tag;
    using json::unicode::UTF_16_encoding_tag;
    using json::unicode::UTF_16LE_encoding_tag;
    using json::unicode::UTF_16BE_encoding_tag;
    using json::unicode::UTF_32_encoding_tag;
    using json::unicode::UTF_32LE_encoding_tag;
    using json::unicode::UTF_32BE_encoding_tag;
    using json::unicode::platform_encoding_tag;
    
    // The fixture for testing class JsonParser.
    class json_path_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        json_path_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~json_path_test() {
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
    
    
    TEST_F(json_path_test, DefaultCtor) 
    {
        typedef json_path<UTF_8_encoding_tag> json_path_type;
        typedef json_path_type::ComponentType component_type;
        typedef std::basic_string<UTF_8_encoding_tag::code_unit_type> internal_string;
        
        json_path_type jsonPath;
        EXPECT_EQ(0, jsonPath.level());
        EXPECT_EQ(std::string("/"), jsonPath.path<UTF_8_encoding_tag>());
        
        std::pair<component_type, std::string> component = jsonPath.component_as<UTF_8_encoding_tag>(0);
        EXPECT_EQ(json_path_type::Root, component.first);
        EXPECT_EQ(std::string("/"), component.second);
    }
    
    TEST_F(json_path_test, BasicTests) 
    {
        typedef json_path<UTF_8_encoding_tag> json_path_type;
        typedef json_path_type::ComponentType component_type;
        typedef std::basic_string<UTF_8_encoding_tag::code_unit_type> internal_string;

        json_path_type jsonPath;
        jsonPath.push_index(123);
        EXPECT_EQ(1, jsonPath.level());
        internal_string s = "key1";
        jsonPath.push_key(s.begin(), s.end());
        //std::cout << '\'' << jsonPath.path<UTF_8_encoding_tag>() << '\''  << std::endl;
        EXPECT_EQ(2, jsonPath.level());
        s = "escaped\"abc";
        jsonPath.push_key(s.begin(), s.end());
        EXPECT_EQ(3, jsonPath.level());
        //std::cout << '\'' << jsonPath.path<UTF_8_encoding_tag>() << '\'' << std::endl;
        EXPECT_TRUE(std::string("/123/\"key1\"/\"escaped\\\"abc\"/") == jsonPath.path<UTF_8_encoding_tag>());
        
        jsonPath.pop_component();
        EXPECT_EQ(2, jsonPath.level());
        //std::cout << '\'' << jsonPath.path<UTF_8_encoding_tag>() << '\'' << std::endl;
        EXPECT_TRUE(std::string("/123/\"key1\"/") == jsonPath.path<UTF_8_encoding_tag>());
        
        jsonPath.pop_component();
        EXPECT_EQ(1, jsonPath.level());
        //std::cout << '\'' << jsonPath.path<UTF_8_encoding_tag>() << '\'' << std::endl;
        EXPECT_TRUE(std::string("/123/") == jsonPath.path<UTF_8_encoding_tag>());        
        
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        //std::cout << '\'' << jsonPath.path<UTF_8_encoding_tag>() << '\'' << std::endl;
        EXPECT_TRUE(std::string("/") == jsonPath.path<UTF_8_encoding_tag>());        
        
    }
}

//
//  json_path_test.cpp
//  Test
//
//  Created by Andreas Grosam on 10/20/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "json/json_path/json_path.hpp"
#include <gtest/gtest.h>

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
    
    
    /*
     
     json_path(); // c-tor

     void pop_component();
     
     void push_index(IndexT index);
     
     template <typename IteratorT>
     void push_key(IteratorT first, IteratorT last)

     void clear();
     
     size_t level() const;
     
     path_component_type
     component_at(size_t pos) const;
     
     void write(std::ostream& os) const;
     
     std::string path() const;
     
     free function, operator
     std::ostream <<(std::ostream& os, const json_path& v);
     
     template <typename IteratorT>
     void back_key_assign(IteratorT first, IteratorT last)

     */
    
    
    
    TEST_F(json_path_test, DefaultCtor) 
    {
        typedef json_path<UTF_8_encoding_tag> json_path_t;
        typedef json_path_t::path_component_type path_component_t;
        typedef json_path_t::key_type key_type;
        typedef json_path_t::index_type index_t;
        
        json_path_t jsonPath;
        EXPECT_EQ(0, jsonPath.level());
        std::string path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path);
    }
    
    
    TEST_F(json_path_test, BasicTests) 
    {
        typedef json_path<UTF_8_encoding_tag> json_path_t;
        typedef json_path_t::path_component_type path_component_t;
        typedef json_path_t::key_type key_type;
        typedef json_path_t::index_type index_t;
        
        json_path_t jsonPath;
        jsonPath.push_index(1);        
        EXPECT_EQ(1, jsonPath.level());
        std::string path = jsonPath.path();
        EXPECT_EQ(std::string("/1/"), path) << "path: '" << path << "'";
        
        jsonPath.push_index(12);        
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/1/12/"), path) << "path: '" << path << "'";
        
        jsonPath.push_index(123);        
        EXPECT_EQ(3, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/1/12/123/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/1/12/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/1/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
        
        
        
        std::string s = "key1";
        jsonPath.push_key(s.begin(), s.end());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/"), path) << "path: '" << path << "'";
                
        s = "key2";
        jsonPath.push_key(s.begin(), s.end());
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/\"key2\"/"), path) << "path: '" << path << "'";
        
        s = "key3";
        jsonPath.push_key(s.begin(), s.end());
        EXPECT_EQ(3, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/\"key2\"/\"key3\"/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/\"key2\"/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/"), path) << "path: '" << path << "'";

        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
        
        
        s = "escaped\"abc";
        jsonPath.push_key(s.begin(), s.end());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"escaped\\\"abc\"/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
        

        jsonPath.push_index(1);        
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/1/"), path) << "path: '" << path << "'";

        jsonPath.back_index() = 0;
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/0/"), path) << "path: '" << path << "'";
        
        jsonPath.back_index() = 100000;
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/100000/"), path) << "path: '" << path << "'";
                
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
        
        
        s = "key1";
        jsonPath.push_key(s.begin(), s.end());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/"), path) << "path: '" << path << "'";
        
        
        s = "xxx";
        jsonPath.back_key_assign(s.begin(), s.end());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/"), path) << "path: '" << path << "'";
        
        s = "yyyyyyyyy";
        jsonPath.back_key_assign(s.begin(), s.end());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"yyyyyyyyy\"/"), path) << "path: '" << path << "'";
        
        s = "xxx";
        jsonPath.back_key_assign(s.begin(), s.end());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/"), path) << "path: '" << path << "'";
        
        s = "key2";
        jsonPath.push_key(s.begin(), s.end());
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/\"key2\"/"), path) << "path: '" << path << "'";
        
                
        jsonPath.push_index(0);        
        EXPECT_EQ(3, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/\"key2\"/0/"), path) << "path: '" << path << "'";
        
        jsonPath.back_index() = 1;
        EXPECT_EQ(3, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/\"key2\"/1/"), path) << "path: '" << path << "'";
        
        jsonPath.back_index() = 2;
        EXPECT_EQ(3, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/\"key2\"/2/"), path) << "path: '" << path << "'";
        
        jsonPath.back_index() = 100000;
        EXPECT_EQ(3, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/\"key2\"/100000/"), path) << "path: '" << path << "'";
     
        
        jsonPath.pop_component();
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/\"key2\"/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
    }

}

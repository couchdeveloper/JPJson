//
//  intrusive_json_path_test.mm
//  Test
//
//  Created by Andreas Grosam on 12/3/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

//
//  json_path_test.cpp
//  Test
//
//  Created by Andreas Grosam on 10/20/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "json/json_path/intrusive_json_path.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <iomanip>
#include "json/unicode/unicode_utilities.hpp"
#include <stack>

namespace {
    
    using json::objc::objc_internal::intrusive_json_path;

    
    // The fixture for testing class JsonParser.
    class intrusive_json_path_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        intrusive_json_path_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~intrusive_json_path_test() {
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
    
    template <typename EncodingT, typename IndexT = std::size_t>
    class intrusive_json_path 
    {   
    public:
        typedef IndexT                              index_type;
        typedef typename EncodingT::code_unit_type  char_type;
        typedef std::pair<const char_t*, size_t>    string_buffer_type;
        struct root_component {};        
        typedef variant<root_component, index_type, string_buffer_t> component_type;
        
    public:
        intrusive_json_path();        
        void            push_index(IndexT index);        
        index_type&     back_index();        
        void            push_key(string_buffer_type const& buffer):        
        string_buffer_type& back_key();        
        void            pop_component();        
        void            clear();        
        size_t          level() const;
        component_type const& component_at(size_t pos) const;        
        component_type&    component_at(size_t pos);                
        void            write(std::ostream& os) const;        
        std::string     path() const;
    };
    
    */
    
    TEST_F(intrusive_json_path_test, DefaultCtor) 
    {
        typedef intrusive_json_path<json::unicode::UTF_8_encoding_tag> json_path_t;
        typedef json_path_t::component_type component_t;
        typedef json_path_t::string_buffer_type string_buffer_t;
        typedef json_path_t::index_type index_t;
        
        json_path_t jsonPath;
        EXPECT_EQ(0, jsonPath.level());
        std::string path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path);
    }
    
    
    TEST_F(intrusive_json_path_test, BasicTests) 
    {
        typedef intrusive_json_path<json::unicode::UTF_8_encoding_tag> json_path_t;
        typedef json_path_t::component_type component_t;
        typedef json_path_t::string_buffer_type string_buffer_t;
        typedef json_path_t::index_type index_t;
        
        typedef std::stack<std::string> storage_type;
        
        storage_type storage;
        
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
        
        
        
        storage.push("key1");
        jsonPath.push_key(string_buffer_t(&storage.top()[0], storage.top().size()));
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/"), path) << "path: '" << path << "'";
        
        storage.push("key2");
        jsonPath.push_key(string_buffer_t(&storage.top()[0], storage.top().size()));
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/\"key2\"/"), path) << "path: '" << path << "'";
        
        storage.push("key3");
        jsonPath.push_key(string_buffer_t(&storage.top()[0], storage.top().size()));
        EXPECT_EQ(3, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/\"key2\"/\"key3\"/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(2, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/\"key2\"/"), path) << "path: '" << path << "'";
        storage.pop();
        
        jsonPath.pop_component();
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/"), path) << "path: '" << path << "'";
        storage.pop();
        
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
        storage.pop();
        
        
        storage.push("escaped\"abc");
        jsonPath.push_key(string_buffer_t(&storage.top()[0], storage.top().size()));
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"escaped\\\"abc\"/"), path) << "path: '" << path << "'";
        
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
        storage.pop();
        
        
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
        
        
        storage.push("key1");
        jsonPath.push_key(string_buffer_t(&storage.top()[0], storage.top().size()));
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"key1\"/"), path) << "path: '" << path << "'";
        
        
        storage.pop();
        storage.push("xxx");
        jsonPath.back_key() = string_buffer_t(&storage.top()[0], storage.top().size());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/"), path) << "path: '" << path << "'";
        
        storage.pop();
        storage.push("yyyyyyyyy");
        jsonPath.back_key() = string_buffer_t(&storage.top()[0], storage.top().size());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"yyyyyyyyy\"/"), path) << "path: '" << path << "'";
        
        storage.pop();
        storage.push("xxx");
        jsonPath.back_key() = string_buffer_t(&storage.top()[0], storage.top().size());
        EXPECT_EQ(1, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/\"xxx\"/"), path) << "path: '" << path << "'";
        
        storage.push("key2");
        jsonPath.push_key(string_buffer_t(&storage.top()[0], storage.top().size()));
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
        storage.pop();
        
        jsonPath.pop_component();
        EXPECT_EQ(0, jsonPath.level());
        path = jsonPath.path();
        EXPECT_EQ(std::string("/"), path) << "path: '" << path << "'";
        storage.pop();
    }
    
}

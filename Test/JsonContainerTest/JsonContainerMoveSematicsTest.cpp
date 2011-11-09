//
//  JsonContainerMoveSematicsTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 5/3/11.
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

#include "json/value/value.hpp"
#include "gtest/gtest.h"

#include <iostream>
#include <boost/config.hpp>
#include <boost/variant.hpp>


using namespace json;

namespace test {
    
    
    typedef json::value<>   Value;
    typedef Value::array_t  Array;
    typedef Value::object_t Object;
    
    
    // Test class for verifying the application of move semantics in boost::variants:
    class Resource
    {
    private:
        size_t length_; // The length of the resource.
        int* data_; // The resource.
        
    public:
        Resource() throw()
        : length_(0), data_(0) 
        {
            std::cout << "Resource:: default ctor" << std::endl;
        }
        
        // Simple constructor that initializes the resource.
        explicit Resource(size_t length)
        : length_(length)
        , data_(new int[length])
        {
            std::cout << "Resource::ctor with param length = " << length_ << "." << std::endl;
        }
        
        // Destructor.
        ~Resource() throw() {
            std::cout << "Resource::dtor. length_ = " << length_ << ".";        
            if (data_ != NULL) {
                std::cout << " Deleting resource.";
                // Delete the resource.
                delete[] data_;
            }        
            std::cout << std::endl;
        }
        
        // Copy constructor.
        Resource(const Resource& other)
        : length_(other.length_)
        , data_(new int[other.length_])
        {
            std::cout << "Resource::copy_ctor. length_ = " << other.length_ << ". Copying resource." << std::endl;        
            std::copy(other.data_, other.data_ + length_, data_);
        }
        
        // Copy assignment operator.
        Resource& operator=(const Resource& other)
        {
            std::cout << "Resource::operator= (const Resource& other). other.length_ = " << other.length_ << ". Copying resource." << std::endl;
            if (this != &other) {
                // Free the existing resource.
                delete[] data_;            
                length_ = other.length_;
                data_ = new int[length_];
                std::copy(other.data_, other.data_ + length_, data_);
            }
            return *this;
        }
        
    #if !defined (BOOST_NO_RVALUE_REFERENCES)
        // Move construtor
        Resource(Resource&& other) throw()
        : data_(0)
        , length_(0)
        {
            std::cout << "Resource::move_ctor. other.length_ = " << other.length_ << ". Moving resource." << std::endl;                
            data_ = other.data_;
            length_ = other.length_;
            other.data_ = 0;
            other.length_ = 0;
        }
        
        // Move assignment operator
        Resource& operator=(Resource&& other) throw() 
        {
            std::cout << "Resource::move_operator=. other.length_ = " << other.length_ << ". Moving resource." << std::endl;                        
            if (this != &other) {
                delete[] data_;
                data_ = other.data_;
                length_ = other.length_;
                other.data_ = 0;
                other.length_ = 0;
            }
            return *this;
        }
        
    #endif    
        
        // Retrieves the length of the data resource.
        size_t length() const  { return length_;  }
        
    };

    typedef boost::variant<
          bool 
        , double
        , std::string
        , Resource
    > variant_t;
    
    
} // namespace test


namespace {
    
#if !defined (BOOST_NO_RVALUE_REFERENCES)

    // The fixture for testing class json::Value:
    
    class JsonContainerMoveSematicsTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonContainerMoveSematicsTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonContainerMoveSematicsTest() {
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
    
    
    TEST_F(JsonContainerMoveSematicsTest, BasicTest) {
        using namespace test;
        
        Resource r1(10);
        Resource r2 = std::move(r1);
        EXPECT_EQ(0, r1.length());
        EXPECT_EQ(10, r2.length());
    }


#endif

} // anonymous namespace
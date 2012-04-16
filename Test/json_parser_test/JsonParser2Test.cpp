//
//  JsonParser2Test.cpp
//  Test
//
//  Created by Andreas Grosam on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "json/parser/parse.hpp"
#include "json/parser/parser.hpp"
#include "json/parser/semantic_actions.hpp"
#include "json/parser/semantic_actions_test.hpp"
#include "json/parser/parser_errors.hpp"

#include "gtest/gtest.h"


// for testing
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <stdexcept>


#include "json/unicode/unicode_utilities.hpp"

#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/algorithm/string.hpp>


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>


#include <stdlib.h>
#include <string.h>
#include <cctype>

//
// Test parser with input in different Unicode encoding schemes.
// 



namespace test {
    
    
    using namespace std;
    
    struct test_result_s {
        bool            parse_result_;
        std::string     str_rep_;
        int             array_count_;
        int             object_count_;
        int             key_string_count_;
        int             data_string_count_;
        int             boolean_count_;
        int             null_count_;
        int             number_count_;
        int             max_level_;
    };
    
    struct test_case_s {
        string         json_text_;
        test_result_s  test_result_;    
    };
    
    
    
    inline test_case_s
    read_test_case(const std::string& path) 
    {
        test_case_s test_case;
        
        ifstream is;
        is.exceptions(ifstream::failbit | ifstream::badbit);
        
        is.open(path.c_str());
        
        // skip ws:
        while (isspace(is.peek())) {
            is.get();
        }
        
        // read first line to get the marker:
        string marker;
        getline(is, marker);
        boost::algorithm::trim(marker); // remove trailing spaces
        // skip ws:
        while (isspace(is.peek())) {
            is.get();
        }
        
        
        
        // Read line by line and copy it into a string until we find a line
        // whose content equals marker:
        while (true) {
            string line;
            getline(is, line);
            if (line.compare(0, marker.size(), marker) == 0) {
                // after the marker, the line must end with white_spaces:
                size_t pos = marker.size();
                string::iterator first = line.begin() + pos;
                while (first != line.end()) {
                    if (isspace(*first)) {
                        ++first;
                    }
                }
                if (first == line.end()) {
                    break; // done
                }
            }
            test_case.json_text_.append(line);
            test_case.json_text_.append(1, '\n');
        }
        // skip ws:
        while (isspace(is.peek())) {
            is.get();
        }
        
        is.exceptions(ifstream::goodbit);
        
        // Create an empty property tree object:
        using boost::property_tree::ptree;        
        ptree pt;
        
        // Load the INFO from the stream into the property tree. If reading fails
        // (parse error, etc.), an exception is thrown.
        read_info(is, pt);
        
        // Get the vairous properties and set them in the test_case variable:
        // (Note: if there is no value for the key, a default value will be set instead)
        test_case.test_result_.parse_result_ = pt.get<bool>("test_result.parse_result");
        test_case.test_result_.str_rep_ = pt.get<std::string>("test_result.str_rep");
        test_case.test_result_.array_count_ = pt.get<int>("test_result.array_count", 0);
        test_case.test_result_.object_count_ = pt.get<int>("test_result.object_count",0);
        test_case.test_result_.key_string_count_ = pt.get<int>("test_result.key_string_count",0);
        test_case.test_result_.data_string_count_ = pt.get<int>("test_result.data_string_count",0);
        test_case.test_result_.boolean_count_ = pt.get<int>("test_result.boolean_count",0);
        test_case.test_result_.null_count_ = pt.get<int>("test_result.null_count",0);
        test_case.test_result_.number_count_ = pt.get<int>("test_result.number_count",0);
        test_case.test_result_.max_level_ = pt.get<int>("test_result.max_level",0);
        
        return test_case;
    }
}


namespace {
    
    using namespace json;
    

    template <typename T>
    class JsonParser2Test : public ::testing::TestWithParam<std::string> 
    {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonParser2Test() {
            // You can do set-up work for each test here.
            std::cout << "==== JsonParser2Test c-tor ====" << std::endl;
        }
        
        virtual ~JsonParser2Test() {
            // You can do clean-up work that doesn't throw exceptions here.
            std::cout << "==== JsonParser2Test dtor ====" << std::endl;
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
            std::cout << "==== JsonParser2Test SetUp() ====" << std::endl;
            
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
            std::cout << "==== JsonParser2Test TearDown() ====" << std::endl;
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
        
    TYPED_TEST_CASE_P(JsonParser2Test);
    
    
    
    
    
    TYPED_TEST_P(JsonParser2Test, ParseValidInput1) 
    {
        // Inside a test, refer to TypeParam to get the type parameter.        
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<unicode::encoding_tag, TypeParam>::value) );
        
        typedef unicode::UTF_8_encoding_tag             from_encoding_t;
        typedef TypeParam                               to_encoding_t;
        
        // Convert the source into the required encoding:
        typedef typename encoding_traits<from_encoding_t>::code_unit_type       source_char_t;
        typedef std::basic_string<source_char_t>                                source_t;
        typedef typename source_t::iterator                                     source_iterator;
        typedef typename source_t::const_iterator                               source_const_iterator;
        
        typedef typename encoding_traits<to_encoding_t>::code_unit_type         input_char_t;
        typedef std::vector<input_char_t>                                       input_source_t;
        typedef typename input_source_t::iterator                               input_iterator_t;
        typedef typename input_source_t::const_iterator                         const_input_iterator_t;
        
        typedef unicode::converter<from_encoding_t, to_encoding_t, unicode::Validation::SAFE> converter_t;
        
        
        // Inside a test, access the test parameter with the GetParam() method
        // of the TestWithParam<T> class:
        std::string test_file = ::testing::TestWithParam<std::string>::GetParam();

        // Read in the test case:
        test::test_case_s test_case = test::read_test_case(test_file);
        source_iterator s_first = test_case.json_text_.begin();
        source_iterator s_last = test_case.json_text_.end();

        // Convert the source to the required encoding:
        input_source_t input;
        std::back_insert_iterator<input_source_t> s_dest(input);
        int cvt_result = converter_t().convert(s_first, s_last, s_dest);
        ASSERT_TRUE(cvt_result == 0);
        
        // Create a semantic actions object:        
        typedef internal::semantic_actions_test<unicode::UTF_16_encoding_tag>  sa_t;
        sa_t sa;
        
        // Start the parser
        input_iterator_t first = input.begin();
        input_iterator_t last = input.end();
        bool parse_result = parse(first, last, to_encoding_t(), sa);
        
        
        // Perform tests:
        test::test_result_s& r = test_case.test_result_;
        ASSERT_EQ(r.parse_result_,         parse_result);         
        EXPECT_EQ(r.array_count_,          sa.array_count());
        EXPECT_EQ(r.object_count_,         sa.object_count());
        EXPECT_EQ(r.key_string_count_,     sa.key_string_count());
        EXPECT_EQ(r.data_string_count_,    sa.data_string_count());
        EXPECT_EQ(r.boolean_count_,        sa.boolean_count());
        EXPECT_EQ(r.null_count_,           sa.null_count());
        EXPECT_EQ(r.number_count_,         sa.number_count());
        EXPECT_EQ(r.max_level_,            sa.max_level());
    } 


    
    // Register
    REGISTER_TYPED_TEST_CASE_P(JsonParser2Test, 
                               ParseValidInput1);
    
    
    // Instantiate test cases:
    typedef ::testing::Types<
    unicode::UTF_8_encoding_tag, 
    unicode::UTF_16_encoding_tag,
    unicode::UTF_16BE_encoding_tag,
    unicode::UTF_16LE_encoding_tag,
    unicode::UTF_32_encoding_tag,
    unicode::UTF_32BE_encoding_tag,
    unicode::UTF_32LE_encoding_tag
    >  UTF_encodings;
    
    
    INSTANTIATE_TYPED_TEST_CASE_P(JsonParserUnicodeVariant, JsonParser2Test, UTF_encodings);
    
    
    const char* pets[] = {
        "Resources/TestJson/test1.test",
        "Resources/TestJson/test2.test"
    };
    
    INSTANTIATE_TEST_CASE_P(JsonParserTestFile, 
                            JsonParser2Test,
                            ::testing::ValuesIn(pets));    
    
}

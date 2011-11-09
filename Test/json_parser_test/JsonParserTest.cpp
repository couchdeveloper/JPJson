//
//  JsonParserTest.cpp
//  json_parser
//
//  Created by Andreas Grosam on 4/5/11.
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

#include "json/parser/parser.hpp"
#include "json/parser/parse.hpp"
#include "json/unicode/unicode_utilities.hpp"
#include "gtest/gtest.h"
#include <string.h>


#include <boost/iterator/iterator_traits.hpp>
#include "json/endian/byte_swap_iterator.hpp"
#include "json/parser/semantic_actions.hpp"
#include "json/parser/semantic_actions_test.hpp"
#include "json/unicode/unicode_conversions.hpp"
#include "json/parser/parser_errors.hpp"



namespace {
    
    using namespace json;
    
    
    
    
#if 0  // Not required     
    using testing::Types;
    
    template <class T>
    class TypeSanityTest : public testing::Test {
    };
    
    
    // The list of Input Iterators we support
    typedef Types<char*, const char*, signed char*, const signed char*, 
        unsigned char*, const unsigned char*,
        wchar_t*, const wchar_t*,
        std::vector<char>::iterator, std::vector<char>::const_iterator,
        std::vector<uint16_t>::iterator, std::vector<uint16_t>::const_iterator,
        std::vector<wchar_t>::iterator, std::vector<wchar_t>::const_iterator
    > InputIterator_Types;
    

    
    TYPED_TEST_CASE(TypeSanityTest, InputIterator_Types);
    TYPED_TEST(TypeSanityTest, InputIteratorSanity)
    {
        typedef typename json::internal::iterator_encoding<TypeParam>::type encoding_t;
        typedef typename json::parser<TypeParam, encoding_t, 
                                      internal::SemanticActionsTest<encoding_t> 
                                     > parser_t;
        
        typedef typename parser_t::iterator     iterator_adapter;        
        typedef json::value<>                   value_t;        
        
        EXPECT_FALSE ( (boost::is_convertible<iterator_adapter, value_t>::value) );
        
    }
#endif    
    
    
    // The fixture for testing class JsonParser.
    class JsonParserTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        JsonParserTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~JsonParserTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
            
            //const char* buffer = {"abcd\uFFFDabcd\n"};
            //printf("%s", buffer);
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
    TEST_F(JsonParserTest, TestDefaultCtor) 
    {
        typedef json::semantic_actions_noop<UTF_8_encoding_tag> sa_noop_t;
        typedef parser<const char*, UTF_8_encoding_tag, 
            json::internal::semantic_actions_test<UTF_8_encoding_tag> > TestParser;
        typedef parser<const char*, UTF_8_encoding_tag, sa_noop_t> ValidatingParser;
        
        TestParser::semantic_actions_t sa;
        TestParser parser(sa);
        const TestParser::state_t& state = parser.state();
        TestParser::result_t result = parser.result();
        EXPECT_EQ( JP_NO_ERROR, state.error() );
        EXPECT_EQ("no error", std::string(state.error_str()));
        
        parser.reset();
        result = parser.result();
        EXPECT_EQ( JP_NO_ERROR, state.error() );
        EXPECT_EQ("no error", std::string(state.error_str()));
        result = parser.move_result();
    }
    
/*    
    TEST_F(JsonParserTest, TestParseMemberFunction0) 
    {
        typedef std::string::const_iterator  iterator;
        typedef parser<iterator, UTF_8_encoding_tag> ValidatingParser;
        typedef ValidatingParser::result_t result_t;
        
        ValidatingParser parser;
        
        const std::string s1 = "[]";
        iterator first = s1.begin();
        iterator last = s1.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ( JP_NO_ERROR, err );
        EXPECT_TRUE( (first == last) );        
    }
 */
    
    // 
    //  Test Error Conditions
    // 
     
    TEST_F(JsonParserTest, TestParse_Error_ENOTEXT)  
    {
        // Validating Parser
        
        const char* s = "     ";
        typedef const char* InputIterator;
        typedef json::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        
        InputIterator first = s;
        InputIterator last = s + strlen(s);
        
        int error;
        ParseOptions options = ParseOptions::None | ParseOptions::LogLevelNone;
        bool result = json::parse(first, last, options, &error);
        EXPECT_TRUE (result == false);
        EXPECT_TRUE (first == last);
        EXPECT_EQ(json::JP_EMPTY_TEXT_ERROR, error);
    }
    
    
    TEST_F(JsonParserTest, DISABLED_TestParse_SyntaxErrors)  
    {
        //
        // JSON syntax errors
        //
        
        //
        // Syntax errors
        //
        //    JP_SYNTAX_ERROR,                        // "syntax error" - general error
        //    JP_EMPTY_TEXT_ERROR,                    // "text is empty"
        //    JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,      // "control character not allowed in json string"
        //    JP_UNEXPECTED_END_ERROR,                // "unexpected end of text" (p_ == last_)
        //    JP_UNICODE_NULL_NOT_ALLOWED_ERROR,      // "encountered U+0000" (An Unicode NULL is not a valid character in this implementation)
        //    JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,      // "expected array or object"    
        //    JP_EXPECTED_TOKEN_OBJECT_END_ERROR,     // "expected end-of-object '}'"
        //    JP_EXPECTED_TOKEN_ARRAY_END_ERROR,      // "expected end-of-array ']'"
        //    JP_EXPECTED_TOKEN_KEY_VALUE_SEP_ERROR,  // "expected key-value-separator ':'"
        //    JP_INVALID_HEX_VALUE_ERROR,             // "invalid hexadecimal number"
        //    JP_INVALID_ESCAPE_SEQ_ERROR,            // "invalid escape sequence"
        //    JP_BADNUMBER_ERROR,                     // "bad number"
        //    JP_EXPECTED_STRING_ERROR,               // "expected string"
        //    JP_EXPECTED_NUMBER_ERROR,               // "expected number"
        //    JP_EXPECTED_VALUE_ERROR,                // "expected value"
        
        
        // use a validating parser
        
        struct syntax_error_s {
            const char input[64];
            int error_code;
            const char* error_str;
            int where;
        };
        
        syntax_error_s syntax_errors[] = {   
            {"[\"abc",      json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       5},
            {"[",           json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       1},
            {"{",           json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       1},
            {"{\"a",        json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       3},
            {"{\"a\"",      json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       4},
            {"{\"a\":",     json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       5},
            {"{\"a\":t",    json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       6},
            {"[f",          json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       2},
            {"[fa",         json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       3},
            {"[false",      json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       6},
            {"[[]",         json::JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       3},
            
            {"[f]",         json::JP_EXPECTED_VALUE_ERROR,    "expected value",               2},
            {"[fals]",      json::JP_EXPECTED_VALUE_ERROR,    "expected value",               5},
            {"[nul]",       json::JP_EXPECTED_VALUE_ERROR,    "expected value",               4},
            {"[t]",         json::JP_EXPECTED_VALUE_ERROR,    "expected value",               2},

            
            {"x",           json::JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,     "expected array or object",            0},
            {"]",           json::JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,     "expected array or object",            0},
            {"\"a\"",       json::JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,     "expected array or object",            0},
            
            {"{123}",       json::JP_EXPECTED_STRING_ERROR,   "expected string",              1},
            
            {"{\"a\" 1}",   json::JP_EXPECTED_TOKEN_KEY_VALUE_SEP_ERROR,"expected key-value-separator ':'",  5},
            {"{\"a\" :}",   json::JP_EXPECTED_VALUE_ERROR,    "expected value",  6},
            
            {"[\"\aa\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\ba\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\fa\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\na\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\ra\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\ta\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\va\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\aa\"]",   json::JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            
            {"[\"\\x0123\"]",   json::JP_INVALID_ESCAPE_SEQ_ERROR,    "invalid escape sequence",  3},
            {"[\"\\90123\"]",   json::JP_INVALID_ESCAPE_SEQ_ERROR,    "invalid escape sequence",  3},
            
            {"[\"\\u00GKabc\"]",   json::JP_INVALID_HEX_VALUE_ERROR,    "invalid hexadecimal number",  6},
            

            //{"\x00[]",        json::JP_UNICODE_NULL_NOT_ALLOWED_ERROR,      "received U+0000", 0},
            //{"[\"a\x00bc\"]", json::JP_UNICODE_NULL_NOT_ALLOWED_ERROR,      "received U+0000", 9},
            
            
            {"", json::JP_EMPTY_TEXT_ERROR, "text is empty", 0}
        };
        
        typedef const char* InputIterator;
        typedef json::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        
        syntax_error_s* first = syntax_errors;
        syntax_error_s* last = first + sizeof(syntax_errors) / sizeof(syntax_error_s);
    
        while (first != last) {
            InputIterator p = (*first).input;
            InputIterator end = p + sizeof((*first).input);
        
            ParseOptions options = ParseOptions::None;
            int error;            
            bool result = json::parse(p, end, options, &error);
            EXPECT_TRUE(result == false) << "with input: " << (*first).input;
            EXPECT_EQ( (*first).where, std::distance((*first).input, p) ) << "with input: " << (*first).input;
            EXPECT_EQ( (*first).error_code, error ) << "with input: " << (*first).input;
         
            ++first;
        }
    }
    
    
    TEST_F(JsonParserTest, TestParse_Malformed_UTF8)  
    {
        //
        // ill-formed Unicode sequence
        //
        //  JP_INVALID_UNICODE_ERROR,               // "invalid unicode code point" - general error
        //  JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    // "illformed Unicode sequence" - general error
        //  JP_EXPECTED_HIGH_SURROGATE_ERROR,       // "expected high surrogate code point"  - illformed Unicode sequence 
        //  JP_EXPECTED_LOW_SURROGATE_ERROR,        // "expected low surrogate code point" - illformed Unicode sequence
        // 
        // 
        // use a validating parser
        
        struct error_s {
            const char input[64];
            int error_code;
            const char* error_str;
            int where;
        };
        
        error_s errors[] = {   
            {"[\"abc\xC0xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xC1xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xF5xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xF6xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xF7xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xFAxyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xFExyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xFFxyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\x80xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xBFxyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xC2xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xC2\x80\x80xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 7},
            {"[\"abc\xDFxyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE0xyz\"]",    json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE0\x80\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE0\x9F\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xED\xA0\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE1\x80\x80\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 8},
            {"[\"abc\xF0\x80\x80\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xF0\x8F\x80\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xF0\x90\x80\x80\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 9},
            {"[\"abc\xF4\x90\x80\x80\x80xyz\"]",json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xF4\x80xyz\"]",       json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 7},
            {"[\"abc\xF4\x80\x80xyz\"]",   json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 8},
            {"[\"abc\xF4\x80\x80\x80\x80xyz\"]",   json::JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 9}
                        
        };
        
        typedef const char* InputIterator;
        typedef json::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        
        error_s* first = errors;
        error_s* last = first + sizeof(errors) / sizeof(error_s);

        
        int idx = 0;
        while (first != last) {
            InputIterator p = (*first).input;
            InputIterator end = p + strlen(p);
            char buffer[64];
            InputIterator s = p;
            
            // For display purposes only, convert the malformed UTF-8 into a well 
            // formed sequence by replacing malformed chars with Unicode replacement 
            // characters.
            char* d = buffer;
            std::size_t count = json::unicode::convert(s, end, UTF_8_encoding_tag(), d, UTF_8_encoding_tag());
            buffer[count] = 0;
            
            ParseOptions options = ParseOptions::LogLevelNone;
            int error;
            bool result = json::parse(p, end, options, &error);

            std::size_t consumed = std::distance((*first).input, p);
            EXPECT_TRUE(result == false) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).where, consumed ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).error_code, error ) << "with input[" << idx << "]: " << buffer;
            
            ++first;
            ++idx;
        }
    }
    
    
    
    TEST_F(JsonParserTest, TestParse_Noncharacters)  
    {
        //
        // Invalid Unicode code points in JSON text
        //
        //    JP_UNICODE_NONCHARACTER_ERROR,          // "encountered unicode noncharacter"
        //    JP_UNICODE_REJECTED_BY_FILTER,          // "Unicode code point rejected by filter"
        
        // use a validating parser
        
        struct error_s {
            const char input[64];
            int error_code;
            const char* error_str;
            int where;
        };
        
        error_s errors[] = {   
            {"[\"abc\uFFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 8},
            {"[\"abc\uFFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 8},
            {"[\"abc\U0001FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0001FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0002FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0002FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0003FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0003FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0004FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0004FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0005FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0005FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0006FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0006FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0007FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0007FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0008FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0008FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0009FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0009FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000AFFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000AFFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000BFFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000BFFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000CFFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000CFFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000DFFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000DFFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000EFFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000EFFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000FFFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000FFFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0010FFFExyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0010FFFFxyz\"]",    json::JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9}
        };
        
        typedef const char* InputIterator;
        typedef json::parser<InputIterator, UTF_8_encoding_tag> parser_t;
        
        error_s* first = errors;
        error_s* last = first + sizeof(errors) / sizeof(error_s);
        int idx = 0;
        while (first != last) {
            InputIterator p = (*first).input;
            InputIterator end = p + strlen(p);
            char buffer[64];
            InputIterator s = p;
            char* d = buffer;
            std::size_t count = json::unicode::convert(s, end, UTF_8_encoding_tag(), d, UTF_8_encoding_tag());
            buffer[count] = 0;
            
            ParseOptions options = ParseOptions::LogLevelNone;
            int error;
            bool result = json::parse(p, end, options, &error);
            EXPECT_TRUE(result == false) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).where, std::distance((*first).input, p) ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).error_code, error ) << "with input[" << idx << "]: " << buffer;
            
            ++first;
            ++idx;
        }
    }
    
    
    
    
    TEST_F(JsonParserTest, TestParseFunction2)  
    {
        std::string s = "   ";
        std::vector<char> v = std::vector<char>(s.begin(), s.end());
        typedef std::vector<char>::const_iterator InputIterator;
        typedef json::parser<InputIterator, UTF_8_encoding_tag> parser_t;

        InputIterator first = v.begin();
        InputIterator last = v.end();
        ParseOptions options = ParseOptions::LogLevelNone;
        int error = 0;
        bool result = json::parse(first, last, options, &error);
        EXPECT_TRUE (result == false);
        EXPECT_TRUE (first == last);
        EXPECT_EQ (json::JP_EMPTY_TEXT_ERROR, error);
    }

    TEST_F(JsonParserTest, TestParseFunction3)  
    {
        std::string s = "   ";
        typedef std::string::const_iterator InputIterator;
        typedef json::parser<InputIterator, UTF_8_encoding_tag> parser_t;

        InputIterator first = s.begin();
        InputIterator last = s.end();

        ParseOptions options = ParseOptions::LogLevelNone;
        int error = 0;
        bool result = json::parse(first, last, options, &error);
        EXPECT_TRUE (result == false);
        EXPECT_TRUE (first == last);
        EXPECT_EQ (json::JP_EMPTY_TEXT_ERROR, error);
    }

    
    
    
    // Various Array Tests:
    TEST_F(JsonParserTest, ParseEmptyArray) 
    {
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        
        const std::string s1 = "[]";
        input_iterator first = s1.begin();
        input_iterator last = s1.end();
        parser_t parser(sa);
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ( JP_NO_ERROR, err );
        EXPECT_TRUE( first == last );
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    
    
    TEST_F(JsonParserTest, ParseArrayOfEmptyArrays) 
    {
        typedef std::string::const_iterator                input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag> parser_t;
        typedef parser_t::result_t                         result_t;
        typedef parser_t::state_t                          state_t;
        typedef parser_t::semantic_actions_t               semantic_actions_t;

        semantic_actions_t sa;
        parser_t parser(sa);
        std::string jsonText = "[ [] , [ ] , [  ] , [   ],   [   ] ,[],[],[]]";
        input_iterator first = jsonText.begin();
        input_iterator last = jsonText.end();

        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseArrayOfArrays) 
    {
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string jsonText = "[[[[[[]]]]]]";
        input_iterator first = jsonText.begin();
        input_iterator last = jsonText.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseEmptyArrayWithWhiteSpaces) {
        const char* jsonText = "\n\r\t[   \n\r\n\r\t\t\n\r\n\n\r\r\n ]  \t\r ";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseArrayOfStrings01) 
    {
        const char* jsonText = "[\n\t\"string1\" ,\n\t\"string2\"\n , \"\", \"xxx\\nxxx\" ] ";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;

        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();

        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ControlCharacterNotAllowedInString) {
        const char* jsonText = "[\"Xx\x1FxX\"]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        sa.log_level(json::semanticactions::LogLevelNone);
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        
        EXPECT_EQ(JP_CONTROL_CHAR_NOT_ALLOWED_ERROR, err);
        EXPECT_TRUE( (first != last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_CONTROL_CHAR_NOT_ALLOWED_ERROR, state.error());
        EXPECT_EQ("control character not allowed in json string", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseArrayOfOneStringWithEscapeSequences) {
        const char* jsonText = "[\"s\\\"s s\\\\s s\\/s s\\bs s\\fs s\\ns s\\rs s\\ts \"]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseValuesTrueFalseNull) {
        const char* jsonText = "[true,false,null]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseEmptyObject) {
        const char* jsonText = "{}";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseArrayOfEmptyObjects) {
        const char* jsonText = "[{},{}, {} , { } ,  {  }  ]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
        
    TEST_F(JsonParserTest, ParseObjectWithOneKeyValuePair) {
        const char* jsonText = "{\"key0\":false}";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseObjectWithManyKeyValuePairs) {
        const char* jsonText = "{\"key0\":false, \"key1\" : true  ,  \"key2\"  :  null  , \"key3\":\"string\"}";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    
#pragma mark - JSON Numbers
    
    /*
         
        2.4.  Numbers
        
        The representation of numbers is similar to that used in most
        programming languages.  A number contains an integer component that
        may be prefixed with an optional minus sign, which may be followed by
        a fraction part and/or an exponent part.
        
        Octal and hex forms are not allowed.  Leading zeros are not allowed.
        
        A fraction part is a decimal point followed by one or more digits.
        
        An exponent part begins with the letter E in upper or lowercase,
        which may be followed by a plus or minus sign.  The E and optional
        sign are followed by one or more digits.
        
        Numeric values that cannot be represented as sequences of digits
        (such as Infinity and NaN) are not permitted.
        
        
        
        Crockford                    Informational                      [Page 3]
        
        RFC 4627                          JSON                         July 2006
        
        number = [ minus ] int [ frac ] [ exp ]
        decimal-point = %x2E       ; .
        digit1-9 = %x31-39         ; 1-9
        e = %x65 / %x45            ; e E
        exp = e [ minus / plus ] 1*DIGIT
        frac = decimal-point 1*DIGIT
        int = zero / ( digit1-9 *DIGIT )
        minus = %x2D               ; -
        plus = %x2B                ; +
        zero = %x30                ; 0
     */
    
    // Testing valid numbers:
    TEST_F(JsonParserTest, ParseIntegers) {
        const char* jsonText = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -0, -1, -2,-3,-4,-5,-6,-7,-8,-9, 1234567890, -1234567890, 0]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }

    TEST_F(JsonParserTest, ParseDecimals) {
        const char* jsonText = "[0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, -0.0, -1.0, -2.0,-3.0,-4.0,-5.0,-6.0,-7.0,-8.0,-9.0,"
                               "0.1234567890, -0.1234567890, 1234567890.1234567890, -1234567890.1234567890]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }

    TEST_F(JsonParserTest, ParseSientificFloats) {
        const char* jsonText = "[0.1e1, 0.1e99 , -0.1E01, 0.1E-1,0.1e-10,-1234.1234e100, -1234.1234E-100, 0e0, 0E0, 0e-0, 0E-0]";        
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    TEST_F(JsonParserTest, ParseBadNumbers) 
    {
        typedef const char*                                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        
        struct test_s {
            const char* json;
            bool result;
            json::parser_error_type error_code;
            std::size_t where;
        };
        
        
        test_s tests[] = {
            {"[00.0]",      false, JP_EXPECTED_TOKEN_ARRAY_END_ERROR, 2},
            {"[+1]",        false, JP_EXPECTED_VALUE_ERROR,  1},
            {"[.0]",        false, JP_EXPECTED_VALUE_ERROR,  1},
            {"[0.]",        false, JP_BADNUMBER_ERROR, 3},
            {"[-s]",        false, JP_BADNUMBER_ERROR,  2},
            {"[-.0]",       false, JP_BADNUMBER_ERROR,  2},
            {"[1.e10]",     false, JP_BADNUMBER_ERROR,  3},
            {"[1.0e 10]",   false, JP_BADNUMBER_ERROR,  5},
            {"[1.0E s]",    false, JP_BADNUMBER_ERROR,  5},
            {"[1.0E+ 10s]", false, JP_BADNUMBER_ERROR,  6},
            {"[1.0E- 10s]", false, JP_BADNUMBER_ERROR,  6},
        };
        
        const test_s* first = tests;
        const test_s* last = first + sizeof(tests)/sizeof(test_s);
        
        while (first != last) {        
            
            semantic_actions_t sa;
            sa.log_level(json::semanticactions::LogLevelNone);
            parser_t parser(sa);
            input_iterator p = (*first).json;
            input_iterator end = p + strlen(p);            
            parser_error_type err = parser.parse(p, end);
            
            EXPECT_EQ((*first).error_code, err) << "with json: " << (*first).json <<  ", expected error str: " << json::parser_error_str((*first).error_code);
            EXPECT_EQ((*first).where, std::distance((*first).json, p));        
            //EXPECT_EQ("no error", std::string(state.error_str()));
            ++first;
        }
    }
    
    
    
    TEST_F(JsonParserTest, ParseUTF8String) {
        const char* jsonText = "[\"\u00dcT\", \"säs\", \"süs\", \"sös\", \"sÄs\", \"sÜs\", \"sÖs\"]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_t                semantic_actions_t;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        std::string s = jsonText;
        input_iterator first = s.begin();
        input_iterator last = s.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
    }
    
    
    
    
}  // namespace

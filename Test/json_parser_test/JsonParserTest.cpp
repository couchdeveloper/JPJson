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
#include <gtest/gtest.h>
#include <string.h>


#include <boost/iterator/iterator_traits.hpp>
//#include "json/parser/value_generator.hpp"
#include "json/parser/semantic_actions_test.hpp"
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
    class JsonParserTest : public ::testing::Test 
    {
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
    
    
    typedef json::semantic_actions_noop<unicode::UTF_8_encoding_tag> sa_noop_t;
    typedef json::internal::semantic_actions_test<unicode::UTF_8_encoding_tag>  sa_test_t;
    
    typedef parser<const char*, unicode::UTF_8_encoding_tag, sa_noop_t> NoopParser;
    typedef parser<const char*, unicode::UTF_8_encoding_tag, sa_test_t> TestParser;
    
    
    TEST_F(JsonParserTest, TestParserDefaultCtor) 
    {
        typedef TestParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        EXPECT_EQ(json::JP_NO_ERROR, parser.state().error());
        EXPECT_EQ("no error", std::string(parser.state().error_str()));
        EXPECT_TRUE(sa.result().empty());
        
        parser.reset();
        EXPECT_EQ( JP_NO_ERROR, parser.state().error() );
        EXPECT_EQ("no error", std::string(parser.state().error_str()));
        EXPECT_TRUE(sa.result().empty());
    }
    
    TEST_F(JsonParserTest, NoopParserDefaultCtor) 
    {
        typedef NoopParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        semantic_actions_t sa;
        parser_t parser(sa);
        EXPECT_EQ(json::JP_NO_ERROR, parser.state().error());
        EXPECT_EQ("no error", std::string(parser.state().error_str()));
        EXPECT_TRUE(sa.result() == 0);
        
        parser.reset();
        EXPECT_EQ( JP_NO_ERROR, parser.state().error() );
        EXPECT_EQ("no error", std::string(parser.state().error_str()));
        EXPECT_TRUE(sa.result() == 0);
    }
    
    
    
    
    TEST_F(JsonParserTest, TestParserBasicErrors)  
    {
        typedef TestParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        const char* s = "     ";
        const char* first = s;
        const char* last = s + strlen(s);
        
        semantic_actions_t sa;        
        sa.log_level(json::semanticactions::LogLevelNone);
        parser_t parser(sa);
        error_type error = parser.parse(first, last);
        const parse_state_t& state = parser.state();
        EXPECT_TRUE (first == last);
        EXPECT_EQ(json::JP_EMPTY_TEXT_ERROR, error);
        EXPECT_EQ(json::JP_EMPTY_TEXT_ERROR, state.error() );
        EXPECT_TRUE(sa.result().empty());
    }
    
    TEST_F(JsonParserTest, NoopParserBasicErrors)  
    {
        typedef NoopParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        const char* s = "     ";
        const char* first = s;
        const char* last = s + strlen(s);
        
        semantic_actions_t sa;
        sa.log_level(json::semanticactions::LogLevelNone);
        parser_t parser(sa);
        error_type error = parser.parse(first, last);
        const parse_state_t& state = parser.state();
        EXPECT_TRUE (first == last);
        EXPECT_EQ(json::JP_EMPTY_TEXT_ERROR, error);
        EXPECT_EQ(json::JP_EMPTY_TEXT_ERROR, state.error() );
        EXPECT_TRUE(sa.result() == 0);
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
     
    
    
    TEST_F(JsonParserTest, NoopParser_SyntaxErrors)  
    {
        // use a NOOP parser
        
        using namespace json;
        
        typedef NoopParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        //
        //    Parser Errors:
        
        //    type: json::parser_error_type
        //
        //    // Syntax errors
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
        //
        
        
        
        struct test_s {
            char input[64];
            json::parser_error_type error_code;
            const char* error_str;
            int where;
        };
        
        test_s tests[] = {  
            // JP_EMPTY_TEXT_ERROR
            {"",                JP_EMPTY_TEXT_ERROR,          "text is empty",        0},
            {" ",               JP_EMPTY_TEXT_ERROR,          "text is empty",        1},
            {"\n",              JP_EMPTY_TEXT_ERROR,          "text is empty",        1},
            {"\n\t",            JP_EMPTY_TEXT_ERROR,         "text is empty",         2},
            {"\n\r",            JP_EMPTY_TEXT_ERROR,         "text is empty",         2},
            
            // JP_CONTROL_CHAR_NOT_ALLOWED_ERROR
            {"[\"\aa0\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\ba1\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\fa2\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\na3\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\ra4\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\ta5\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\va6\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            {"[\"\aa7\"]",       JP_CONTROL_CHAR_NOT_ALLOWED_ERROR,    "control character not allowed in json string",  2},
            
            // JP_UNEXPECTED_END_ERROR
            {"[\"abc",          JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       5},
            {"[",               JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       1},
            {"{",               JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       1},
            {"{\"a",            JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       3},
            {"{\"a\"",          JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       4},
            {"{\"a\":",         JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       5},
            {"{\"a\":t",        JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       6},
            {"[f",              JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       2},
            {"[fa",             JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       3},
            {"[false",          JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       6},
            {"[[]",             JP_UNEXPECTED_END_ERROR,    "unexpected end of text",       3},
            
            
            // JP_UNICODE_NULL_NOT_ALLOWED_ERROR
            // Note: the iterator to the input will be incremented past the Unicode Null character (U+0000),
            // since the detection of Unicode Nulls will be determined by a filter predicate in the parser.
            {"[\"a\x00x\"]",    json::JP_UNICODE_NULL_NOT_ALLOWED_ERROR,    "encountered U+0000",       4},
            
            // JP_EXPECTED_ARRAY_OR_OBJECT_ERROR
            {"x",               JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,     "expected array or object",      0},
            {"]",               JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,     "expected array or object",      0},
            {"\"a\"",           JP_EXPECTED_ARRAY_OR_OBJECT_ERROR,     "expected array or object",      0},            
                        
            //    JP_EXPECTED_TOKEN_OBJECT_END_ERROR
            {"[{\"k1\":1 ] 2]",  JP_EXPECTED_TOKEN_OBJECT_END_ERROR,        "expected end-of-object '}'",     9},
            {"[{\"k1\":1 false, 2]",  JP_EXPECTED_TOKEN_OBJECT_END_ERROR,   "expected end-of-object '}'",     9},
            {"[{\"k1\":1 \"abc\", 2]",  JP_EXPECTED_TOKEN_OBJECT_END_ERROR, "expected end-of-object '}'",     9},
            
            //    JP_EXPECTED_TOKEN_ARRAY_END_ERROR
            {"[1, 2 3]",  JP_EXPECTED_TOKEN_ARRAY_END_ERROR,            "expected end-of-array ']'",    6},
            {"[1, 2 false]",  JP_EXPECTED_TOKEN_ARRAY_END_ERROR,        "expected end-of-array ']'",    6},
            {"[1, 2 {}]",  JP_EXPECTED_TOKEN_ARRAY_END_ERROR,           "expected end-of-array ']'",    6},
            
            
            //    JP_EXPECTED_TOKEN_KEY_VALUE_SEP_ERROR
            {"{\"a\" 1}",       JP_EXPECTED_TOKEN_KEY_VALUE_SEP_ERROR,"expected key-value-separator ':'",  5},
            
            //    JP_INVALID_HEX_VALUE_ERROR
            {"[\"\\u00GKabc\"]",JP_INVALID_HEX_VALUE_ERROR,    "invalid hexadecimal number",  6},
            
            //    JP_INVALID_ESCAPE_SEQ_ERROR,            // "invalid escape sequence"
            {"[\"\\x0123\"]",   JP_INVALID_ESCAPE_SEQ_ERROR,    "invalid escape sequence",  3},
            {"[\"\\90123\"]",   JP_INVALID_ESCAPE_SEQ_ERROR,    "invalid escape sequence",  3},
                        
            //    JP_BADNUMBER_ERROR,                     // "bad number"
            
            //    JP_EXPECTED_STRING_ERROR
            {"{123}",           JP_EXPECTED_STRING_ERROR,   "expected string",              1},
            
            //    JP_EXPECTED_NUMBER_ERROR,               // "expected number"
            
            
            // JP_EXPECTED_VALUE_ERROR
            {"[f]",             JP_EXPECTED_VALUE_ERROR,    "expected value",               2},
            {"[fals]",          JP_EXPECTED_VALUE_ERROR,    "expected value",               5},
            {"[nul]",           JP_EXPECTED_VALUE_ERROR,    "expected value",               4},
            {"[t]",             JP_EXPECTED_VALUE_ERROR,    "expected value",               2},            
            {"{\"a\" :}",       JP_EXPECTED_VALUE_ERROR,    "expected value",               6}
            
            

            //{"\x00[]",        JP_UNICODE_NULL_NOT_ALLOWED_ERROR,      "received U+0000", 0},
            //{"[\"a\x00bc\"]", JP_UNICODE_NULL_NOT_ALLOWED_ERROR,      "received U+0000", 9},
        };
        
        typedef const char* InputIterator;
        
        test_s* first = tests;
        test_s* last = first + sizeof(tests) / sizeof(test_s);
        int idx = 0;
        while (first != last) 
        {
            semantic_actions_t sa;
            sa.log_level(semanticactions::LogLevelNone);
            parser_t parser(sa);
                        
            InputIterator start = &(*first).input[0];
            InputIterator p = start;
            InputIterator end = p + sizeof((*first).input);
            while (end > start and *(end-1) == 0) {
                --end;
            }
            
            json::parser_error_type result = parser.parse(p, end);
            const parse_state_t& state = parser.state();
            EXPECT_EQ( (*first).error_code, result )  << "[" << idx << "]: " << "with input: '" << (*first).input << "'";
            EXPECT_EQ( (*first).error_code, state.error() )<< "[" << idx << "]: "   << "with input: '" << (*first).input << "'";
            EXPECT_TRUE( sa.result() == 0 )<< "[" << idx << "]: "  << "with input: '" << (*first).input << "'";
            EXPECT_EQ( (*first).where, std::distance(start, p) )  << "with input: '" << (*first).input << "'";
            if ((*first).error_code != result or (*first).where != std::distance(start, p)) {
                while (0) {};
            }
         
            ++idx;
            ++first;
        }
    }
    
    
    TEST_F(JsonParserTest, NoopParser_Malformed_UTF8)  
    {
        using namespace json;
        
        // use a NOOP parser
        typedef NoopParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        //
        // ill-formed Unicode sequence
        //
        //  JP_INVALID_UNICODE_ERROR,               // "invalid unicode code point" - general error
        //  JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    // "illformed Unicode sequence" - general error
        //  JP_EXPECTED_HIGH_SURROGATE_ERROR,       // "expected high surrogate code point"  - illformed Unicode sequence 
        //  JP_EXPECTED_LOW_SURROGATE_ERROR,        // "expected low surrogate code point" - illformed Unicode sequence
        // 
        
        struct test_s {
            char input[64];
            json::parser_error_type error_code;
            const char* error_str;
            int where;
        };
        
        test_s tests[] = {   
            {"[\"abc\xC0xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xC1xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xF5xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xF6xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xF7xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xFAxyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xFExyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xFFxyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\x80xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xBFxyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 5},
            {"[\"abc\xC2xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xC2\x80\x80xyz\"]",        JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 7},
            {"[\"abc\xDFxyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE0xyz\"]",                JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE0\x80\x80xyz\"]",        JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE0\x9F\x80xyz\"]",        JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xED\xA0\x80xyz\"]",        JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xE1\x80\x80\x80xyz\"]",    JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 8},
            {"[\"abc\xF0\x80\x80\x80xyz\"]",    JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xF0\x8F\x80\x80xyz\"]",    JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xF0\x90\x80\x80\x80xyz\"]",JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 9},
            {"[\"abc\xF4\x90\x80\x80\x80xyz\"]",JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 6},
            {"[\"abc\xF4\x80xyz\"]",            JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 7},
            {"[\"abc\xF4\x80\x80xyz\"]",        JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 8},
            {"[\"abc\xF4\x80\x80\x80\x80xyz\"]",JP_ILLFORMED_UNICODE_SEQUENCE_ERROR,    "illformed Unicode sequence", 9}
                        
        };
        
        typedef const char* InputIterator;
        
        test_s* first = tests;
        test_s* last = first + sizeof(tests) / sizeof(test_s);

        
        int idx = 0;
        while (first != last) 
        {
            semantic_actions_t sa;
            sa.log_level(semanticactions::LogLevelNone);
            parser_t parser(sa);
            
            InputIterator start = &(*first).input[0];
            InputIterator p = start;
            InputIterator end = p + sizeof((*first).input);
            while (end > start and *(end-1) == 0) {
                --end;
            }
            
            char buffer[64];
            InputIterator s = p;
            
            // For display purposes only, convert the malformed UTF-8 into a well 
            // formed sequence by replacing malformed chars with Unicode replacement 
            // characters.
            char* d = buffer;
#if !defined (NDEBUG)
            int cvt_result =
#endif
            json::unicode::convert(s, end, UTF_8_encoding_tag(), d, UTF_8_encoding_tag(), unicode::ReplaceIllFormed);
            assert(cvt_result == 0);
            buffer[d-buffer] = 0;
            
            json::parser_error_type result = parser.parse(p, end);
            const parse_state_t& state = parser.state();
            std::size_t consumed = std::distance(start, p);
            
            EXPECT_EQ( (*first).error_code, result ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).error_code, state.error() ) << "with input[" << idx << "]: " << buffer;
            EXPECT_TRUE( sa.result() == 0 ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).where, consumed) << "with input[" << idx << "]: " << buffer;
            
            if ((*first).error_code != result) {
                while (0) {};
            }
            
            ++first;
            ++idx;
        }
    }
    
    
    TEST_F(JsonParserTest, DISABLED_NoopParser_Malformed_UTF16) {
        EXPECT_TRUE(0=="TEST NOT YET IMPLEMENTED");
    }  
    
    
    
    TEST_F(JsonParserTest, NoopParser_Noncharacters)  
    {
        using namespace json;
        
        typedef NoopParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        //
        // Invalid Unicode code points in JSON text
        //
        //    JP_UNICODE_NONCHARACTER_ERROR,          // "encountered unicode noncharacter"
        //    JP_UNICODE_REJECTED_BY_FILTER,          // "Unicode code point rejected by filter"
                
        struct test_s {
            char input[64];
            int error_code;
            const char* error_str;
            int where;
        };
        
        test_s tests[] = {   
            {"[\"abc\uFFFExyz\"]",        JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 8},
            {"[\"abc\uFFFFxyz\"]",        JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 8},
            {"[\"abc\U0001FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0001FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0002FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0002FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0003FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0003FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0004FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0004FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0005FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0005FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0006FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0006FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0007FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0007FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0008FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0008FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0009FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0009FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000AFFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000AFFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000BFFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000BFFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000CFFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000CFFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000DFFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000DFFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000EFFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000EFFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000FFFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U000FFFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0010FFFExyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9},
            {"[\"abc\U0010FFFFxyz\"]",    JP_UNICODE_NONCHARACTER_ERROR,    "encountered unicode noncharacter", 9}
        };
        
        typedef const char* InputIterator;
        
        test_s* first = tests;
        test_s* last = first + sizeof(tests) / sizeof(test_s);

        int idx = 0;
        while (first != last) 
        {
            // Default behavior for Unicode noncharacters: signal error
            semantic_actions_t sa;
            sa.log_level(semanticactions::LogLevelNone);
            parser_t parser(sa);
            
            InputIterator start = &(*first).input[0];
            InputIterator p = start;
            InputIterator end = p + sizeof((*first).input);
            while (end > start and *(end-1) == 0) {
                --end;
            }
            
            char buffer[64];
            InputIterator s = p;
            char* d = buffer;
            std::size_t count = json::unicode::convert(s, end, UTF_8_encoding_tag(), d, UTF_8_encoding_tag(), unicode::ReplaceIllFormed);
            buffer[count] = 0;
            
            json::parser_error_type result = parser.parse(p, end);
            const parse_state_t& state = parser.state();
            std::size_t consumed = std::distance(start, p);
            
            EXPECT_EQ( (*first).error_code, static_cast<int>(result) ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).error_code, static_cast<int>(state.error()) ) << "with input[" << idx << "]: " << buffer;
            EXPECT_TRUE( sa.result() == 0 ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).where, consumed) << "with input[" << idx << "]: " << buffer;
            
            ++first;
            ++idx;
        }
    }
    
    
    //
    //  Perform a number of tests with a semantic_actions_test class in order
    //  to verify proper function of the parser and basic semantic actions.
    //
    
    TEST_F(JsonParserTest, TestParserBasicSuccess)  {
        using namespace json;
        
        typedef TestParser parser_t;
        typedef parser_t::state_t parse_state_t;
        typedef parser_t::semantic_actions_type semantic_actions_t;
        typedef json::parser_error_type error_type;
        
        
        struct test_s {
            std::string input;
            std::string output;
            int result;
            int count_objects;
            int count_arrays;
            int count_key_strings;
            int count_data_strings;
            int count_numbers;
            int count_booleans;
            int count_nulls;
        };
        
        test_s tests[] = {   
            // input | output         
            {" [ ] ",                           "[]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      0,       0,     0,     0 },
            
            {" { } ",                           "{}",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      1,     0,     0,      0,      0,     0,     0 },
            
            {" [ [ [ [ [ [ [ [ [ [ ] ] ] ] ] ] ] ] ] ] ",      "[[[[[[[[[[]]]]]]]]]]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,    10,    0,      0,       0,     0,     0 },
            
            {" [\"string\"] ",                  "[\"string\"]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [1] ",                           "[1]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      0,       1,     0,     0 },
            
            {" [true] ",                        "[true]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      0,       0,     1,     0 },
            
            {" [false] ",                       "[false]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      0,       0,     1,     0 },

            {" [null] ",                        "[null]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      0,       0,     0,     1 },
            
            {" [{}] ",                          "[{}]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      1,     1,    0,      0,       0,     0,     0 },
            
            {" [{} , {}] ",                     "[{},{}]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      2,     1,    0,      0,       0,     0,     0 },
            
            {" [1, 2, 3] ",                     "[1,2,3]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      0,       3,     0,     0 },

            {" [{}, \"string\", 1, true, null] ", "[{},\"string\",1,true,null]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      1,     1,    0,      1,       1,     1,     1 },
            
            {" [\"quote '\\\"'\"] ",            "[\"quote '\\\"'\"]",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [\"reverse solidus '\\\\'\"] ",  "[\"reverse solidus '\\\\'\"]",
                // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [\"solidus '/'\"] ",            "[\"solidus '/'\"]",
                // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [\"line feed '\\n'\"] ",         "[\"line feed '\\n'\"]",
                // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [\"backspace '\\b'\"] ",         "[\"backspace '\\b'\"]",
                // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [\"form feed '\\f'\"] ",         "[\"form feed '\\f'\"]",
                // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [\"carriage return '\\r'\"] ",   "[\"carriage return '\\r'\"]",
                // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls
                0,      0,     1,    0,      1,       0,     0,     0 },
            
            {" [\"tab '\\t'\"] ",               "[\"tab '\\t'\"]",
                // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls
                0,      0,     1,    0,      1,       0,     0,     0 },
            

            {"{\"key0\" : 0}",                  "{\"key0\":0}",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      1,     0,    1,      0,       1,     0,     0 },
            
            {"{\"key0\" : 0, \"key1\" : 1}",    "{\"key0\":0,\"key1\":1}",
            // err  | objs | arrs | k-strs | d-strs | nbrs | bools| nulls 
                0,      1,     0,    2,      0,       2,     0,     0 }
            
            
        };
        
        
        typedef const char* InputIterator;
        
        test_s* first = tests;
        test_s* last = first + sizeof(tests) / sizeof(test_s);
        
        int idx = 0;
        while (first != last) 
        {
            // Default behavior for Unicode noncharacters: signal error
            semantic_actions_t sa;
            sa.log_level(semanticactions::LogLevelNone);
            parser_t parser(sa);
            
            InputIterator start = &(*first).input[0];
            InputIterator p = start;
            InputIterator end = p + (*first).input.size();
            char buffer[64];
            InputIterator s = p;
            char* d = buffer;
            int res = json::unicode::convert(s, end, UTF_8_encoding_tag(), d, UTF_8_encoding_tag(), unicode::ReplaceIllFormed);
            assert(res == 0);
            buffer[std::distance(buffer, d)] = 0;
            json::parser_error_type result = parser.parse(p, end);
            const parse_state_t& state = parser.state();
            //std::size_t consumed = std::distance(start, p);
            std::string output = sa.str();
            
            EXPECT_EQ( (*first).result, static_cast<int>(result) ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).result, static_cast<int>(state.error()) ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).output, output ) << "with input[" << idx << "]: " << buffer;
            EXPECT_EQ( (*first).count_objects, sa.object_count() );
            EXPECT_EQ( (*first).count_arrays, sa.array_count() );
            EXPECT_EQ( (*first).count_key_strings, sa.key_string_count() );
            EXPECT_EQ( (*first).count_data_strings, sa.data_string_count() );
            EXPECT_EQ( (*first).count_numbers, sa.number_count() );
            EXPECT_EQ( (*first).count_booleans, sa.boolean_count() );
            EXPECT_EQ( (*first).count_nulls, sa.null_count() );
            
            if ((*first).result != static_cast<int>(result) ) {
                while (0) {};
            }
            
            
            //EXPECT_EQ( (*first).where, consumed) << "with input[" << idx << "]: " << buffer;
            
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type               semantic_actions_t;

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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;

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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
    TEST_F(JsonParserTest, ParseInteger)
    {
        typedef json::internal::semantic_actions_test<unicode::UTF_8_encoding_tag>  sa_test_t;
        typedef parser<const char*, unicode::UTF_8_encoding_tag, sa_test_t> parser_t;
        typedef typename sa_test_t::number_desc_t::NumberType number_type_t;
        
        typedef typename sa_test_t::json_value_type Value;      // boost::shared_ptr<boost::any>
        typedef typename sa_test_t::json_array_type Array;      // std::vector<Value>
        typedef typename sa_test_t::json_number_type Number;    // std::pair<number_type_t, std::string>
        
        struct test_s {
            const char* number_string;
            number_type_t number_type;
        };
        
        test_s tests[] = {
            {"0", number_type_t::UnsignedInteger},
            {"1", number_type_t::UnsignedInteger},
            {"10", number_type_t::UnsignedInteger},
            {"100", number_type_t::UnsignedInteger},
            {"1000", number_type_t::UnsignedInteger},
            {"1234567890", number_type_t::UnsignedInteger},
            {"-0", number_type_t::Integer},
            {"-1", number_type_t::Integer},
            {"-10", number_type_t::Integer},
            {"-100", number_type_t::Integer},
            {"-1000", number_type_t::Integer},
            {"-1234567890", number_type_t::Integer}
        };
        
        const int count = sizeof(tests)/sizeof(test_s);
        
        for (int i = 0; i < count; ++i)
        {
            std::string json;
            json.append("[");
            json.append(tests[i].number_string);
            json.append("]");
            
            sa_test_t sa;
            parser_t parser(sa);
            
            const char* first = json.c_str();
            const char* last = first + json.size();
            parser_error_type err = parser.parse(first, last);
            EXPECT_EQ(JP_NO_ERROR, err);
            EXPECT_TRUE( (first == last) );
            
            Value v = sa.result();
            Array const a = boost::any_cast<Array>(v);
            Number const n = boost::any_cast<Number>(a[0]);
            
            EXPECT_EQ(tests[i].number_type, n.first);
            EXPECT_EQ(std::string(tests[i].number_string), n.second);
        }
        
    }

    
    
    TEST_F(JsonParserTest, ParseDecimal)
    {
        typedef json::internal::semantic_actions_test<unicode::UTF_8_encoding_tag>  sa_test_t;
        typedef parser<const char*, unicode::UTF_8_encoding_tag, sa_test_t> parser_t;
        typedef typename sa_test_t::number_desc_t::NumberType number_type_t;
        
        typedef typename sa_test_t::json_value_type Value;      // boost::shared_ptr<boost::any>
        typedef typename sa_test_t::json_array_type Array;      // std::vector<Value>
        typedef typename sa_test_t::json_number_type Number;    // std::pair<number_type_t, std::string>
        
        struct test_s {
            const char* number_string;
            number_type_t number_type;
        };
        
        test_s tests[] = {
            {"0.0", number_type_t::UnsignedDecimal},
            {"1.0", number_type_t::UnsignedDecimal},
            {"10.0", number_type_t::UnsignedDecimal},
            {"100.00", number_type_t::UnsignedDecimal},
            {"1000.000", number_type_t::UnsignedDecimal},
            {"1234567890.00000", number_type_t::UnsignedDecimal},
            {"-0.0", number_type_t::Decimal},
            {"-1.0", number_type_t::Decimal},
            {"-10.0", number_type_t::Decimal},
            {"-100.00", number_type_t::Decimal},
            {"-1000.000", number_type_t::Decimal},
            {"-1234567890.000", number_type_t::Decimal}
        };
        
        const int count = sizeof(tests)/sizeof(test_s);
        
        for (int i = 0; i < count; ++i)
        {
            std::string json;
            json.append("[");
            json.append(tests[i].number_string);
            json.append("]");
            
            sa_test_t sa;
            parser_t parser(sa);
            
            const char* first = json.c_str();
            const char* last = first + json.size();
            parser_error_type err = parser.parse(first, last);
            EXPECT_EQ(JP_NO_ERROR, err);
            EXPECT_TRUE( (first == last) );
            
            Value v = sa.result();
            Array const a = boost::any_cast<Array>(v);
            Number const n = boost::any_cast<Number>(a[0]);
            
            EXPECT_EQ(tests[i].number_type, n.first);
            EXPECT_EQ(std::string(tests[i].number_string), n.second);
        }
        
    }
    
    
    TEST_F(JsonParserTest, ParseScientific)
    {
        typedef json::internal::semantic_actions_test<unicode::UTF_8_encoding_tag>  sa_test_t;
        typedef parser<const char*, unicode::UTF_8_encoding_tag, sa_test_t> parser_t;
        typedef typename sa_test_t::number_desc_t::NumberType number_type_t;
        
        typedef typename sa_test_t::json_value_type Value;      // boost::shared_ptr<boost::any>
        typedef typename sa_test_t::json_array_type Array;      // std::vector<Value>
        typedef typename sa_test_t::json_number_type Number;    // std::pair<number_type_t, std::string>
        
        struct test_s {
            const char* number_string;
            number_type_t number_type;
        };
        
        test_s tests[] = {
            {"0.0e00", number_type_t::Scientific},
            {"1.0e00", number_type_t::Scientific},
            {"10.0e00", number_type_t::Scientific},
            {"100.00e00", number_type_t::Scientific},
            {"1000.000e00", number_type_t::Scientific},
            {"1234567890.00000e00", number_type_t::Scientific},
            {"-0.0e00", number_type_t::Scientific},
            {"-1.0e00", number_type_t::Scientific},
            {"-10.0e00", number_type_t::Scientific},
            {"-100.00e00", number_type_t::Scientific},
            {"-1000.000e00", number_type_t::Scientific},
            {"-1234567890.000e-00", number_type_t::Scientific},
            {"0.0e-00", number_type_t::Scientific},
            {"1.0e-00", number_type_t::Scientific},
            {"10.0e-00", number_type_t::Scientific},
            {"100.00e-00", number_type_t::Scientific},
            {"1000.000e-00", number_type_t::Scientific},
            {"1234567890.00000e-00", number_type_t::Scientific},
            {"-0.0e-00", number_type_t::Scientific},
            {"-1.0e-00", number_type_t::Scientific},
            {"-10.0e-00", number_type_t::Scientific},
            {"-100.00e-00", number_type_t::Scientific},
            {"-1000.000e-00", number_type_t::Scientific},
            
            {"-1234567890.000E-00", number_type_t::Scientific},
            {"0.0E00", number_type_t::Scientific},
            {"1.0E00", number_type_t::Scientific},
            {"10.0E00", number_type_t::Scientific},
            {"100.00E00", number_type_t::Scientific},
            {"1000.000E00", number_type_t::Scientific},
            {"1234567890.00000E00", number_type_t::Scientific},
            {"-0.0E00", number_type_t::Scientific},
            {"-1.0E00", number_type_t::Scientific},
            {"-10.0E00", number_type_t::Scientific},
            {"-100.00E00", number_type_t::Scientific},
            {"-1000.000E00", number_type_t::Scientific},
            {"-1234567890.000E-00", number_type_t::Scientific},
            {"0.0E-00", number_type_t::Scientific},
            {"1.0E-00", number_type_t::Scientific},
            {"10.0E-00", number_type_t::Scientific},
            {"100.00E-00", number_type_t::Scientific},
            {"1000.000E-00", number_type_t::Scientific},
            {"1234567890.00000E-00", number_type_t::Scientific},
            {"-0.0E-00", number_type_t::Scientific},
            {"-1.0E-00", number_type_t::Scientific},
            {"-10.0E-00", number_type_t::Scientific},
            {"-100.00E-00", number_type_t::Scientific},
            {"-1000.000E-00", number_type_t::Scientific},
            {"-1234567890.000E-00", number_type_t::Scientific}
        };
        
        const int count = sizeof(tests)/sizeof(test_s);
        
        for (int i = 0; i < count; ++i)
        {
            std::string json;
            json.append("[");
            json.append(tests[i].number_string);
            json.append("]");
            
            sa_test_t sa;
            parser_t parser(sa);
            
            const char* first = json.c_str();
            const char* last = first + json.size();
            parser_error_type err = parser.parse(first, last);
            EXPECT_EQ(JP_NO_ERROR, err);
            EXPECT_TRUE( (first == last) );
            
            Value v = sa.result();
            Array const a = boost::any_cast<Array>(v);
            Number const n = boost::any_cast<Number>(a[0]);
            
            EXPECT_EQ(tests[i].number_type, n.first);
            EXPECT_EQ(std::string(tests[i].number_string), n.second);
        }
    }
    
    
    
    
    TEST_F(JsonParserTest, ParseIntegers) {
        const char* jsonText = "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -0, -1, -2,-3,-4,-5,-6,-7,-8,-9, 1234567890, -1234567890, 0]";
        typedef std::string::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
        
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
        typedef parser_t::semantic_actions_type                semantic_actions_t;
        
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
    
    
    
#pragma mark - Large JSON String
    
    
    TEST_F(JsonParserTest, LargeJSONString) 
    {
        typedef std::vector<char> json_text_t;
        typedef json::internal::semantic_actions_test<UTF_8_encoding_tag>  semantic_actions_t;
        typedef json_text_t::const_iterator                 input_iterator;
        typedef parser<input_iterator, UTF_8_encoding_tag, semantic_actions_t>  parser_t;
        typedef parser_t::result_t                          result_t;
        typedef parser_t::state_t                           state_t;
        
        // create the JSON input:
        const size_t Size = 128*1024;
        json_text_t jsonText;
        jsonText.push_back('[');
        jsonText.push_back('\"');
        
        for (int i = 0; i < Size; ++i) {
            jsonText.push_back('a');
        }
        jsonText.push_back('\"');
        jsonText.push_back(']');
        
        
        semantic_actions_t sa;
        parser_t parser(sa);
        input_iterator first = jsonText.begin();
        input_iterator last = jsonText.end();
        
        parser_error_type err = parser.parse(first, last);
        EXPECT_EQ(JP_NO_ERROR, err);
        EXPECT_TRUE( (first == last) );        
        const state_t& state = parser.state();
        EXPECT_EQ(JP_NO_ERROR, state.error());
        EXPECT_EQ("no error", std::string(state.error_str()));
        
                
        typedef semantic_actions_t::json_value_type value_t;
        typedef semantic_actions_t::json_array_type array_t;
        typedef semantic_actions_t::json_string_type string_t;
        
        value_t json_value = sa.result();
        array_t array = boost::any_cast<array_t>(json_value);
        EXPECT_EQ(1, array.size());
        
        string_t str = boost::any_cast<string_t>(array[0]);
        EXPECT_EQ(Size, str.size());
    }

    
}  // namespace

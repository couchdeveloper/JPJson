//
//  json_path_parser_test.cpp
//  Test
//
//  Created by Andreas Grosam on 12/27/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "json/json_path/parser.hpp"
#include <gtest/gtest.h>

#include <iostream>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <sstream>

#include <boost/mpl/contains.hpp>

namespace {
    
    
    // The fixture for testing class JsonParser.
    class json_path_parser_test : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        json_path_parser_test() {
            // You can do set-up work for each test here.
        }
        
        virtual ~json_path_parser_test() {
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
     
     using namespace json::jsonpath;
     using namespace json::json_path::namespace::json_path_internal;
     

     // Convert a sequence of an infix-expression to a postfix-expression
     template <typename InIterator, typename OutIterator> 
     inline OutIterator postfix(InIterator first, InIterator last, OutIterator dest)
     
     
     
     Token types:
     ===========
     
     Operators:
     ---------
     struct kleenstar;   // unary operator   '*'
     struct concat;      // binary operator  '+'
     struct join;        // binary operator  '|'
     
     Parenthesis:
     ------------
     struct open_paren;
     struct close_paren;

     Operands:
     ---------
     template <typename SymbolValueT>
     struct basic_symbol
     {
        basic_symbol(const SymbolValueT& value)     
     };
     
     struct epsilon : symbol_base {};     
     
     
     typedef basic_symbol<char> symbol;
     
     
     */
    
    
    // For each type T in symbol_types operator<<(ostream&, T) must be
    // defined.
    
    struct Alphabet {
        typedef boost::mpl::vector<char, int>::type symbol_types;
    };
    
    
    
    template <typename RegExpr>
    struct print : boost::static_visitor<void> 
    {
        typedef typename RegExpr::symbol_types      symbol_types;
        typedef typename RegExpr::epsilon           epsilon;
        typedef typename RegExpr::open_paren        open_paren;
        typedef typename RegExpr::close_paren       close_paren;
        typedef typename RegExpr::concat            concat;
        typedef typename RegExpr::join              join;
        typedef typename RegExpr::kleenstar         kleenstar;

        
        print() : os_(std::cout) {}
        print(std::ostream& os) : os_(os) {}
        
        void operator()(const open_paren&) {
            os_ << "(";
            return;
        }
        
        void operator()(const close_paren&) {
            os_ << ")";
            return;
        }
        
        void operator()(const concat&) {
            os_ << "+";
            return;
        }
        
        void operator()(const join&) {
            os_ << "|";
            return;
        }
        
        void operator()(const kleenstar&) {
            os_ << "*";
            return;
        }
        
        // Symbols
        template <typename SymbolT>    
        inline typename boost::enable_if<
              typename boost::mpl::contains<symbol_types, SymbolT>::type
            , void
        >::type
        operator()(const SymbolT& symbol) {
            os_ << symbol;
        }
        
        
        
    private:
        std::ostream& os_;
    };
    
    
    TEST_F(json_path_parser_test, BasicTest) 
    {        
        typedef json::jsonpath::regex_traits<Alphabet> regex_type;
        typedef regex_type::token_types       token_types;
        typedef regex_type::epsilon           epsilon;
        typedef regex_type::open_paren        open_paren;
        typedef regex_type::close_paren       close_paren;
        typedef regex_type::concat            concat;
        typedef regex_type::join              join;
        typedef regex_type::kleenstar         kleenstar;
        typedef regex_type::token_type        token;
        typedef regex_type::symbol_type       symbol;
        
        typedef regex_type::token_types        token_seq;
        typedef regex_type::symbol_types       symbol_seq;
        typedef regex_type::operator_types     operator_seq;
        typedef regex_type::paren_types        paren_seq;

        
        EXPECT_TRUE( (boost::mpl::contains<symbol_seq, epsilon>::value) );
        EXPECT_TRUE( (boost::mpl::contains<symbol_seq, char>::value) );
        EXPECT_TRUE( (boost::mpl::contains<symbol_seq, int>::value) );
        EXPECT_FALSE( (boost::mpl::contains<symbol_seq, open_paren>::value) );
        EXPECT_FALSE( (boost::mpl::contains<symbol_seq, close_paren>::value) );
        EXPECT_FALSE( (boost::mpl::contains<symbol_seq, concat>::value) );
        EXPECT_FALSE( (boost::mpl::contains<symbol_seq, join>::value) );
        EXPECT_FALSE( (boost::mpl::contains<symbol_seq, kleenstar>::value) );
        
        EXPECT_TRUE( (boost::mpl::contains<operator_seq, concat>::value) );
        EXPECT_TRUE( (boost::mpl::contains<operator_seq, join>::value) );
        EXPECT_TRUE( (boost::mpl::contains<operator_seq, kleenstar>::value) );        
        EXPECT_FALSE( (boost::mpl::contains<operator_seq, open_paren>::value) );
        EXPECT_FALSE( (boost::mpl::contains<operator_seq, close_paren>::value) );
        EXPECT_FALSE( (boost::mpl::contains<operator_seq, epsilon>::value) );
        EXPECT_FALSE( (boost::mpl::contains<operator_seq, char>::value) );
        EXPECT_FALSE( (boost::mpl::contains<operator_seq, int>::value) );
        
        EXPECT_TRUE( (boost::mpl::contains<paren_seq, open_paren>::value) );
        EXPECT_TRUE( (boost::mpl::contains<paren_seq, close_paren>::value) );
        EXPECT_FALSE( (boost::mpl::contains<paren_seq, concat>::value) );        
        EXPECT_FALSE( (boost::mpl::contains<paren_seq, join>::value) );
        EXPECT_FALSE( (boost::mpl::contains<paren_seq, kleenstar>::value) );
        EXPECT_FALSE( (boost::mpl::contains<paren_seq, epsilon>::value) );
        EXPECT_FALSE( (boost::mpl::contains<paren_seq, char>::value) );
        EXPECT_FALSE( (boost::mpl::contains<paren_seq, int>::value) );
        
        EXPECT_TRUE( (boost::mpl::contains<token_seq, epsilon>::value) );
        EXPECT_TRUE( (boost::mpl::contains<token_seq, char>::value) );
        EXPECT_TRUE( (boost::mpl::contains<token_seq, int>::value) );
        EXPECT_TRUE( (boost::mpl::contains<token_seq, open_paren>::value) );
        EXPECT_TRUE( (boost::mpl::contains<token_seq, close_paren>::value) );
        EXPECT_TRUE( (boost::mpl::contains<token_seq, concat>::value) );
        EXPECT_TRUE( (boost::mpl::contains<token_seq, join>::value) );
        EXPECT_TRUE( (boost::mpl::contains<token_seq, kleenstar>::value) );
        
        
        std::vector<token> tokens;
        tokens.push_back(epsilon());
        tokens.push_back(open_paren());
        tokens.push_back(close_paren());
        tokens.push_back(concat());
        tokens.push_back(join());
        tokens.push_back(kleenstar());
        tokens.push_back('c');
        tokens.push_back(5);
        
        EXPECT_FALSE( kleenstar() < kleenstar() );
        EXPECT_FALSE( concat() < concat() );
        EXPECT_FALSE( join() < join() );
        EXPECT_TRUE( concat() < kleenstar() );
        EXPECT_TRUE( join() < kleenstar() );
        EXPECT_TRUE( join() < concat() );
        
        print<regex_type> printer;
        std::for_each(tokens.begin(), tokens.end(), boost::apply_visitor(printer));
    }    
    
    TEST_F(json_path_parser_test, ConvertInfixExpressionToPostfixExpression) 
    {
        typedef json::jsonpath::regex_traits<Alphabet> regex_type;
        typedef regex_type::epsilon           epsilon;
        typedef regex_type::open_paren        open_paren;
        typedef regex_type::close_paren       close_paren;
        typedef regex_type::concat            concat;
        typedef regex_type::join              join;
        typedef regex_type::kleenstar         kleenstar;
        //typedef regex_type::symbol_type       symbol;
        typedef regex_type::token_type        token;
        
        std::vector<token> infix_expr;

        // Given the regular expression in infix notation:        
        // (a+(b|c))*a
        // shall result in the regular expression in postfix expression:
        // abc|+a*
        // Note: the operators are not algebraic operators but regular expression operators!
        
        infix_expr.push_back(open_paren());
        infix_expr.push_back('a');
        infix_expr.push_back(concat());
        infix_expr.push_back(open_paren());
        infix_expr.push_back('b');
        infix_expr.push_back(join());
        infix_expr.push_back('c');
        infix_expr.push_back(close_paren());
        infix_expr.push_back(close_paren());        
        infix_expr.push_back(kleenstar());
        infix_expr.push_back('a');
        
        std::vector<token> postfix_expr;
        regex_type::to_postfix(infix_expr.begin(), infix_expr.end(), std::back_inserter(postfix_expr));
        
        std::stringstream ss;
        print<regex_type> printer = print<regex_type>(ss);
        std::for_each(postfix_expr.begin(), postfix_expr.end(), boost::apply_visitor(printer));
        
        EXPECT_TRUE(std::string("abc|+a*") == ss.str());        
    }
}

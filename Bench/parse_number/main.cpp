//
//  main.cpp
//  parse_number
//
//  Created by Andreas Grosam on 10/17/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//


#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>


#include <boost/spirit/home/phoenix/bind.hpp>

#include <iostream>
#include <string>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <sstream>
#include <algorithm>

#include "utilities/timer.hpp"

#include <assert.h>
#include <string.h>



namespace {
    
    void throwRuntimeError(const char* msg) {
        throw std::runtime_error(msg);
    }
}


#if 1
namespace  json_test {
    
    
    template <
        typename InputIterator
    >    
    inline 
    bool 
    parse_number(InputIterator first, InputIterator last) 
    {
        typedef uint32_t code_t;
        
        enum number_state {
            number_state_point,
            number_state_start,
            number_state_sign,
            number_int_is_zero,
            number_state_int,
            number_state_fractional,
            number_state_exponent_start,
            number_state_exponent_sign,
            number_state_exponent
        };
        
        number_state s = number_state_start;
        bool isNegative = false;
        bool exponentIsNegative = false;        
        while (first != last)
        {
            code_t c = *first;
            switch (s) {
                case number_state_point:
                    switch (c) {
                        case '0'...'9': 
                            s = number_state_fractional; 
                            ++first;
                            //while (first != last and (c = *first) >= '0' and c <= '9') {
                            while (first != last and ( (static_cast<code_t>(*first) - '0') <= '9') ) {
                                ++first;
                            }
                            continue;
                        default: goto Number_done; // error: expected digit
                    }
                    break;
                case number_state_start:
                    switch (c) {
                        case '-': 
                            s = number_state_sign; 
                            isNegative = true; 
                            break;
                        case '0': 
                            s = number_int_is_zero; 
                            break;
                        case '1'...'9': 
                            s = number_state_int; 
                            ++first;
                            //while (first != last and (*first >= '0' and *first <= '9')) {
                            while (first != last and ( (static_cast<code_t>(*first) - '0') <= '9') ) {
                                ++first;
                            }
                            continue;
                        default: goto Number_done; // error: not a number
                    }
                    break;
                case number_state_sign:
                    switch (c) {
                        case '0':       
                            s = number_int_is_zero; 
                            break;
                        case '1'...'9': 
                            s = number_state_int; 
                            ++first;
                            //while (first != last and (c = *first) >= '0' and c <= '9') {
                            while (first != last and ( (static_cast<code_t>(*first) - '0') <= '9') ) {
                                ++first;
                            }
                            continue;
                            //break;
                        default: goto Number_done;  // error: expecting a digit
                    }
                    break;
                case number_int_is_zero:
                    switch (c) {
                        case '.': 
                            s = number_state_point;
                            break;
                        case 'e':
                        case 'E': 
                            s = number_state_exponent_start;
                            break;
                        default: goto Number_done; // finished.
                    }
                    break;
                case number_state_int:
                    switch (c) {
                        case '.': 
                            s = number_state_point; 
                            break;
                        case 'e':
                        case 'E': 
                            s = number_state_exponent_start; 
                            break;
                        default: goto Number_done; // finished with integer
                    }
                    break;
                case number_state_fractional:
                    switch (c) {
                        case 'e':
                        case 'E': 
                            s = number_state_exponent_start; 
                            break;
                        default: goto Number_done; // finished with fractional or start exponent
                    }
                    break;
                case number_state_exponent_start:
                    if (c == '-' or c == '+') {
                        s = number_state_exponent_sign; 
                        exponentIsNegative = c == '-'; 
                        ++first;
                        if (first == last) {
                            goto Number_done;  // error
                        }
                        c = *first;
                    } 
                    if (c >= '0' and c <= '9') {
                        s = number_state_exponent; 
                        ++first;
                        //while (first != last and (c = *first) >= '0' and c <= '9') {
                        while (first != last and ( (static_cast<code_t>(*first) - '0') <= '9') ) {
                            ++first;
                        }
                        continue;
                    }
                    else {
                        goto Number_done;  // error
                    }
                case number_state_exponent_sign:
                    break;
                case number_state_exponent:
                    goto Number_done;  // finished
                default:
                    assert(0);
            } //switch
            
            ++first;
        } // while (p_ != last_)
        
    Number_done:        
        switch (s) {
            case number_int_is_zero:    
            case number_state_int:
                return true;
                
            case number_state_fractional:
                return true;
                
            case number_state_exponent:
                return true;
                
            default:
                return false;
        }        
    }
    
}  // namespace json_test    
    
    
    
namespace test {
    // The hand written number parser.
    template <typename ForwardIterator>
    bool 
    validateNumber(ForwardIterator first, ForwardIterator last) 
    {
        enum number_type {
            BAD_NUMBER,
            Integer,
            Decimal,
            Float,        
        };
        
        enum number_state {
            number_state_start,
            number_state_sign,
            number_int_is_zero,
            number_state_int,
            number_state_point,
            number_state_fractional,
            number_state_exponent_start,
            number_state_exponent_sign,
            number_state_exponent,
        };
        
        number_type numberType = BAD_NUMBER;
        
        number_state s = number_state_start;
        bool isNegative = false;
        bool exponentIsNegative = false;        
        bool done = false;
        while (not done)
        {
            char ch = *first;
            switch (s) {
                case number_state_int:
                    switch (ch) {
                        case '0'...'9': break;
                        case '.': s = number_state_point;  break;
                        case 'e':
                        case 'E': s = number_state_exponent_start; break;
                        default: done = true; // finished with integer
                    }
                    break;
                case number_state_fractional:
                    switch (ch) {
                        case '0'...'9':  break;
                        case 'e':
                        case 'E': s = number_state_exponent_start; break;
                        default: done = true; // finished with fractional or start exponent
                    }
                    break;
                case number_state_exponent:
                    switch (ch) {
                        case '0' ... '9':  break;
                        default: done = true;  // finished
                    }
                    break;
                default:
                case number_state_start:
                    switch (ch) {
                        case '-': s = number_state_sign; isNegative = true;  break;
                        case '0': s = number_int_is_zero; break;
                        case '1'...'9': s = number_state_int; break;
                        default: done = true; // error: not a number
                    }
                    break;
                case number_state_sign:
                    switch (ch) {
                        case '0':       s = number_int_is_zero; break;
                        case '1'...'9': s = number_state_int; break;
                        default: done = true;  // error: expecting a digit
                    }
                    break;
                case number_int_is_zero:
                    switch (ch) {
                        case '.': s = number_state_point; break;
                        case 'e':
                        case 'E': s = number_state_exponent_start; break;
                        default: done = true; // finished.
                    }
                    break;
                case number_state_point:
                    switch (ch) {
                        case '0'...'9': s = number_state_fractional; break;
                        default: done = true; // error: expected digit
                    }
                    break;
                case number_state_exponent_start:
                    switch (ch) {
                        case '-': s = number_state_exponent_sign; exponentIsNegative = true; break;
                        case '+': s = number_state_exponent_sign; break;
                        case '0' ... '9': s = number_state_exponent; break;
                        default: done = true;  // error
                    }
                    break;
                case number_state_exponent_sign:
                    switch (ch) {
                        case '0' ... '9': s = number_state_exponent; break;
                        default: done = true;  // finished                            
                    }
                    break;
                    assert(0);
            } //switch
            
            if (not done and first != last)
                ++first;
            
        } // while (not done)
        
        switch (s) {
            case number_int_is_zero:    
            case number_state_int:
                numberType = Integer; 
                break;
                
            case number_state_fractional:
                numberType = Decimal; 
                break;
                
            case number_state_exponent:
                numberType = Float; 
                break;
                
            default:
                numberType = BAD_NUMBER;
        }
        
        
        return first == last and numberType != BAD_NUMBER;
    }
    
} // namespace anonymous
#endif


//#include <boost/fusion.hpp>

//    number parser/compiler
//
//    EBNF:
//
//    number = integer, [ fraction ], [ exponent ]
//    integer = ["-"], +{digits} 
//    fraction = "." digit, {digits}
//    exponent = ("e" | "E"), ["-"] digit, {digits}

#if 1
namespace client
{
    struct number_builder {
        enum typeT {
            Integer,
            UnsignedInteger,
            Decimal,
            Float,
        };
        number_builder() : type_(Integer), isNegative_(false), p_(buffer_), hasLeadingZero_(false)
        { e_ = p_+ sizeof(buffer_) - 1;}
        void integer_push_back(char c) {
            if (p_ != e_) {
                *p_++ = c;
            }
        }
        void push_back(char c) { 
            if (p_ != e_) {
                *p_++ = c;
            }
        }
        size_t size() const { return p_ - buffer_; }
        const char* c_str() { *p_ = 0; return buffer_ ; }
        
        typeT type_;
        char buffer_[32];
        char* p_;
        char* e_;
        bool isNegative_;
        
        // state:
        bool hasLeadingZero_;
    };
    
    template <typename Iterator>
    bool parse_double(Iterator first, Iterator last, double& d)
    {
        using boost::spirit::qi::double_;
        using boost::spirit::qi::_1;
        using boost::spirit::ascii::space;
        using boost::phoenix::ref;
        using boost::spirit::qi::phrase_parse;
        
        bool r = phrase_parse(first, last,
                              ( double_[ref(d) = _1] ),
                              space);
        if (!r || first != last) // fail if we did not get a full match
            return false;
        return r;
    }
    
    
    template <typename Iterator>
    bool validate_double(Iterator first, Iterator last)
    {
        using boost::spirit::qi::double_;
        using boost::spirit::ascii::space;
        using boost::spirit::qi::phrase_parse;
        
        bool r = phrase_parse(first, last,
                              ( double_ ),
                              space);
        if (!r || first != last) // fail if we did not get a full match
            return false;
        return r;
    }
    
    
    template <typename Iterator>
    bool parse_number(Iterator first, Iterator last, number_builder& n)
    {
        using boost::spirit::qi::digit;
        using boost::spirit::qi::char_;
        using boost::spirit::qi::parse;
        using boost::spirit::ascii::space;
        using boost::spirit::qi::_1;
        using boost::phoenix::ref;
        using boost::phoenix::push_back;        
        using boost::phoenix::bind;        
        
        bool r = parse(first, last,
                       (
                        -char_('-')[ push_back(ref(n), _1) ][ ref(n.isNegative_) = true ][ ref(n.type_) = number_builder::UnsignedInteger ] 
                        >> +digit[ bind(&number_builder::integer_push_back, n, _1) ]
                        >> -(char_('.')[ push_back(ref(n), _1) ][ ref(n.type_) = number_builder::Decimal ] >> +digit[ push_back(ref(n), _1) ] )         
                        >> -(char_("eE")[ ref(n.type_) = number_builder::Float ][ push_back(ref(n),'e') ] >> -char_('-')[ push_back(ref(n), _1) ]
                             >> +digit[ push_back(ref(n), _1) ])                              
                        )
                       );
        
        if (!r || first != last) // fail if we did not get a full match
            return false;
        return r;
    }
    
    template <typename Iterator>
    bool validate_number(Iterator first, Iterator last)
    {
        using boost::spirit::qi::digit;
        using boost::spirit::qi::char_;
        using boost::spirit::qi::parse;
        
        bool r = parse(first, last,
                       (
                        -char_('-')
                        >> +digit
                        >> -(char_('.') >> +digit )         
                        >> -(char_("eE") >> -char_('-')
                             >> +digit)                              
                        )
                       );
        
        if (!r || first != last) // fail if we did not get a full match
            return false;
        return r;
    }
    
}


namespace {
    
    using namespace boost::xpressive;
    
    
    
    void interactive()
    {
        using namespace client;
        
        std::cout << "\t\tA number micro parser for Spirit...\n\n";
        
        std::cout << "Give me a number of the form  [-]digits[.digits][exponent] \n";
        std::cout << "Type [q or Q] to quit\n\n";
        
        std::string str;
        while (getline(std::cin, str))
        {
            if (str.empty() || str[0] == 'q' || str[0] == 'Q')
                break;
            
            number_builder n;
            if (client::parse_number(str.begin(), str.end(), n))
            {
                std::cout << "-------------------------\n";
                std::cout << "Parsing succeeded\n";
                std::cout << "got type: " << 
                (n.isNegative_ ?  "(negative) " : "" ) << 
                (n.type_ == number_builder::Integer ? "Integer" 
                 : (n.type_ == number_builder::Decimal ? "Decimal" 
                    : "Float")) << std::endl;
                std::cout << "got number: " << n.c_str() << std::endl;
                std::cout << "\n-------------------------\n";
            }
            else
            {
                std::cout << "-------------------------\n";
                std::cout << "Parsing failed\n";
                std::cout << "-------------------------\n";
            }
        }
        
        std::cout << "Bye... :-) \n\n";
    }


    
    bool match_number(const std::string& string, sregex& number_exp) {
        // [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
        //sregex number_exp = !('+'|'-') >> +_d >> !('.' >> +_d) >> !('e'|'E' >> +_d);
        bool match = regex_match(string, number_exp);
        return match;
    }
        
    bool bench() 
    {
        using namespace client;
        using namespace utilities;
        
        // This is a typedef for a random number generator.
        // Try boost::mt19937 or boost::ecuyer1988 instead of boost::minstd_rand
        typedef boost::mt19937 base_generator_type;
        
        const int kN = 100;  // number of loops
        const int kZ = 100000;    // number of number-strings generated
        
        typedef std::vector<std::string> vector_t;
        
        // Define a random number generator and initialize it with a reproducible seed.
        // (The seed is unsigned, otherwise the wrong overload may be selected when using 
        // mt19937 as the base_generator_type.)
        base_generator_type generator(42u);
        
        // Define a uniform random number distribution which produces "double"
        // values between -1.0e10 and 1.0e10 (min inclusive, max exclusive).
        boost::uniform_int<> integer_dist(-10000, 10000);
        boost::variate_generator<base_generator_type&, boost::uniform_int<> > integer_rand(generator, integer_dist);
        
        boost::uniform_int<> fract_dist(0, 100000);
        boost::variate_generator<base_generator_type&, boost::uniform_int<> > fract_rand(generator, fract_dist);
        
        boost::uniform_int<> exp_dist(-128, 128);
        boost::variate_generator<base_generator_type&, boost::uniform_int<> > exp_rand(generator, exp_dist);
        
        vector_t v;
        for(int i = 0; i < kZ; i++) {
            std::stringstream oss;        
            oss << integer_rand() << '.' << fract_rand() << 'e' << exp_rand ();
            v.push_back(oss.str());
        }
        //std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
        
        timer t = timer();
        
        for (int i = 0; i < kN; ++i) 
        {
            number_builder n;
            vector_t::iterator first = v.begin();
            vector_t::iterator last = v.end();
            bool done = false;
            t.start();                                    
            while (first != last and !done) 
            {
                bool result = client::parse_number((*first).begin(), (*first).end(), n);
                if (not result) {
                    done = true;
                }
                ++first;            
            }
            t.pause();        
            if (first != last) {
                throwRuntimeError("client::parse_number failed");
            }
            
        }
        printf("%25.25s %8.5f µs\n", "spirit parse_number:", t.nanoSeconds()*1e-3/(kN*kZ));
        

        t.reset();
        for (int i = 0; i < kN; ++i) 
        {
            vector_t::iterator first = v.begin();
            vector_t::iterator last = v.end();
            bool done = false;
            t.start();                            
            while (first != last) 
            {
                bool result = client::validate_number((*first).begin(), (*first).end());
                if (not result) {
                    done = true;
                }
                ++first;
            }
            t.pause();
            if (first != last) {
                throwRuntimeError("client::validate_number failed");
            }
        }
        printf("%25.25s %8.5f µs\n", "spirit validate_number:", t.nanoSeconds()*1e-3/(kN*kZ));
        
        
        
        
        t.reset();
        for (int i = 0; i < kN; ++i) 
        {
            vector_t::iterator first = v.begin();
            vector_t::iterator last = v.end();
            bool done = false;
            t.start();                            
            while (first != last) 
            {
                double d;
                bool result = client::parse_double((*first).begin(), (*first).end(), d);
                if (not result) {
                    done = true;
                }
                ++first;
            }
            t.pause();
            if (first != last) {
                throwRuntimeError("client::parse_double failed");
            }
        }
        printf("%25.25s %8.5f µs\n", "spirit parse_double:", t.nanoSeconds()*1e-3/(kN*kZ));
        
        
        t.reset();
        for (int i = 0; i < kN; ++i) 
        {
            vector_t::iterator first = v.begin();
            vector_t::iterator last = v.end();
            bool done = false;
            t.start();                            
            while (first != last and !done) 
            {
                bool result = client::validate_double((*first).begin(), (*first).end());
                if (not result) {
                    done = true;
                }
                ++first;
            }
            t.pause();
            if (first != last) {
                throwRuntimeError("client::validate_double failed");
            }
        }
        printf("%25.25s %8.5f µs\n", "spirit validate_double:", t.nanoSeconds()*1e-3/(kN*kZ));


        t.reset();
        for (int i = 0; i < kN; ++i) 
        {
            vector_t::iterator first = v.begin();
            vector_t::iterator last = v.end();
            bool done = false;
            t.start();                            
            while (first != last and !done) 
            {
                bool result = test::validateNumber((*first).begin(), (*first).end());
                if (not result) {
                    done = true;
                }
                ++first;
            }
            t.pause();
            if (first != last) {
                throwRuntimeError("test::validateNumber failed");
            }
        }
        printf("%25.25s %8.5f µs\n", "test::validateNumber:", t.nanoSeconds()*1e-3/(kN*kZ));

 
        t.reset();
        for (int i = 0; i < kN; ++i) 
        {
            vector_t::iterator first = v.begin();
            vector_t::iterator last = v.end();
            bool done = false;
            t.start();                            
            while (first != last and !done) 
            {
                bool result = json_test::parse_number((*first).begin(), (*first).end());
                if (not result) {
                    done = true;
                }
                ++first;
            }
            t.pause();
            if (first != last) {
                throwRuntimeError("json_test::parse_number failed");
            }
        }
        printf("%25.25s %8.5f µs\n", "json_test::parse_number:", t.nanoSeconds()*1e-3/(kN*kZ));
        
        
        t.reset();
        sregex number_exp = !(as_xpr('+')|as_xpr('-')) >> +_d >> !('.' >> +_d) >> !((as_xpr('e')|'E') >> +_d);
        for (int i = 0; i < kN; ++i) 
        {
            vector_t::iterator first = v.begin();
            vector_t::iterator last = v.end();
            t.start();                            
            while (first != last) 
            {
                bool result = match_number(*first, number_exp);
                if (not result) {
                    //throw std::runtime_error("Xpressive failed");
                }
                ++first;
            }
            t.pause();
        }
        printf("%25.25s %8.5f µs\n", "Xpressive match_number:", t.nanoSeconds()*1e-3/(kN*kZ));
        
        
        return true;
    }
    

}
#endif

int main()
{
    //interactive();
    
    try {
        bench();
    }
    catch (std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
        
    return 0;
}
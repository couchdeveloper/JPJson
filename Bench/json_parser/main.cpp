//
//  main.cpp
//  json_parser
//
//  Created by Andreas Grosam on 4/1/11.
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

#include <boost/config.hpp>

// using boost file streams
//#include <boost/iostreams/device/file.hpp>
//#include <boost/iostreams/stream.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <errno.h>

// #define JSON_INTERNAL_SEMANTIC_ACTIONS_TESTING

#include "json/parser/parser.hpp"
#include "json/parser/parse.hpp"
#include "json/parser/semantic_actions.hpp"
#include "json/parser/semantic_actions_test.hpp"

#include "utilities/timer.hpp"



namespace {
    
    
    const char* TEST_JSON = "Test-UTF8-esc.json";
    
    std::vector<char> loadFileFromResourceFolder(const char* fileName) 
    {        
        std::string filePath("Resources/");
        filePath.append(fileName);
        std::ifstream ifs(filePath.c_str());
        if (!ifs)
            throw std::runtime_error(std::string("could not open file: " + filePath));
        
        // get the length of the file:
        ifs.seekg(0, std::ios::end);
        std::streampos length = ifs.tellg();
        ifs.seekg(0,std::ios::beg);
        
        // use a vector as buffer
        std::vector<char> buffer(length, 0);
        ifs.read(&buffer[0],length);
        if (!ifs) {
            throw std::runtime_error("could not read file");
        }
        
        ifs.close();
        
        return buffer;
    }

}


#if 0
static void bench_validating_parser()
{
    using namespace json;
    using namespace utilities;
    
    std::vector<char> buffer = loadFileFromResourceFolder(TEST_JSON);
    
    /*
    // Create your string stream.
    // Get the stringbuffer from the stream and set the vector as it source.
    std::stringstream localStream;
    localStream.rdbuf()->pubsetbuf(&buffer[0],length);
    */
    
    timer t0 = timer();
#if defined (DEBUG)
    const int N = 1;
#else    
    const int N = 1000;
#endif    
    
    bool result;
    for (int i = 0; i < N; ++i) {
        t0.start();
        std::vector<char>::const_iterator first = buffer.begin();
        std::vector<char>::const_iterator last = buffer.end();
        result = parse(first, last);
        t0.pause();
    }
    
    if (result) {
        printf("Validating Parser: elapsed time:  %.3f ms\n", t0.seconds()*1e3 / N);
    }
    else {
        printf("Validating Parser: error\n");
    }    
}
#endif


static void bench_test_parser() 
{
    using namespace json;
    using utilities::timer;
    
    std::cout << "using vector<char> iterators" << std::endl;
    
    std::vector<char> buffer = loadFileFromResourceFolder(TEST_JSON);

    timer t0 = timer();
#if defined (DEBUG)
    const int N = 1;
#else    
    const int N = 1000;
#endif    
    
    typedef std::vector<char>::iterator iterator;
    typedef json::semantic_actions<json::unicode::UTF_8_encoding_tag> SemanticActions;
    
    bool result;
    SemanticActions sa;
    std::cout << "start parsing " << N << " times ... \n";
    for (int i = 0; i < N; ++i) {
        sa.clear();
        t0.start();
        iterator first = buffer.begin();
        iterator last = buffer.end();
        result = parse(first, last, sa);
        t0.pause();
#if 1   
        if (i == N-1) {
            if (result) 
            {
                //SemanticActions::result_type& r = sa.result();
                std::cout << sa << std::endl;
            }
        }
#endif    
    }
    std::cout << "... end parsing\n";
    
    
    if (result) {
        printf("Test Parser: elapsed time:  %.3f ms\n", t0.seconds()*1e3 / N);
    }
    else {
        printf("json test parser: error\n");
    }    
}


static void bench_parser() 
{
    using namespace json;
    using utilities::timer;
    
    std::cout << "using vector<char> iterators" << std::endl;
    
    std::vector<char> buffer = loadFileFromResourceFolder(TEST_JSON);
    
    timer t = timer();
#if defined (DEBUG)
    const int N = 1;
#else    
    const int N = 1000;
#endif    
    
    typedef std::vector<char>::iterator iterator;
    typedef json::semantic_actions<json::unicode::UTF_8_encoding_tag> SemanticActions;
    
    bool result;
    SemanticActions sa;
    std::cout << "start parsing " << N << " times ... \n";
    double t0_sum = 0.0;
    double t1_sum = 0.0;
    double t2_sum = 0.0;
    double t3_sum = 0.0;
    double t_min = 1.0e99;
    double t_max = 0.0;
    double t_tot = 0.0;
    for (int i = 0; i < N; ++i) {
        sa.clear();
        t.start();
        iterator first = buffer.begin();
        iterator last = buffer.end();
        result = parse(first, last, sa);
        t.stop();
        t_min = std::min(t_min, t.seconds());
        t_max = std::max(t_max, t.seconds());
        t_tot += t.seconds();
        t.reset();
        
        t0_sum += sa.t0();
        t1_sum += sa.t1();
        t2_sum += sa.t2();
        t3_sum += sa.t3();
#if 1   
        if (i == N-1) {
            if (result) 
            {
                //SemanticActions::result_type& r = sa.result();
                // std::cout << sa << std::endl;
                std::cout << "SemanticActionsTest average elapsed time for:\n"
                << "push string (" <<  sa.c0() << "): " 
                << std::fixed << std::setprecision(3) << (t0_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "pop operations: " 
                << std::fixed << std::setprecision(3) << (t3_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "array push_back operations(" <<  sa.c1() << "): " 
                << std::fixed << std::setprecision(3) << (t1_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "object insert operations(" <<  sa.c2() << "): " 
                << std::fixed << std::setprecision(3) << (t2_sum / N) * 1.0e3 << " ms" 
                << std::endl;
                
                std::cout << sa << std::endl;
                
            }
        }
#endif    
    }
    
    SemanticActions::cleanup_caches();
    std::cout << "... end parsing\n";
    
    
    if (result) {
        printf("JsonParser: elapsed time:  min: %5.3f ms, max: %5.3f ms, avg: %5.3f ms\n", 
               t_min*1e3, t_max*1e3, t_tot*1e3 / N );
    }
    else {
        printf("JsonParser: error\n");
    }    
}



static void bench_parser2() 
{
    using namespace json;
    using utilities::timer;
    
    std::cout << "using istream iterators" << std::endl;    
    
    std::ifstream ifs(TEST_JSON);
    if (!ifs)
        throw std::runtime_error("could not open file");
    
    
    timer t = timer();
#if defined (DEBUG)
    const int N = 1;
#else    
    const int N = 1000;
#endif    
    
    typedef std::istream_iterator<char> iterator;
    typedef json::semantic_actions<json::unicode::UTF_8_encoding_tag> SemanticActions;
    
    bool result;
    SemanticActions sa;
    std::cout << "start parsing " << N << " times ... \n";
    double t0_sum = 0.0;
    double t1_sum = 0.0;
    double t2_sum = 0.0;
    double t3_sum = 0.0;
    double t_min = 1.0e99;
    double t_max = 0.0;
    double t_tot = 0.0;
    for (int i = 0; i < N; ++i) {
        sa.clear();
        ifs.clear();
        ifs.seekg(0);
        if (ifs.eof()) {
            std::cout << "error: eos bit set for file stream\n";
            break;
        }
        t.start();
        iterator first(ifs);
        iterator last;  // EOS
        result = parse(first, last, sa);
        t.stop();
        t_min = std::min(t_min, t.seconds());
        t_max = std::max(t_max, t.seconds());
        t_tot += t.seconds();
        t.reset();
        
        t0_sum += sa.t0();
        t1_sum += sa.t1();
        t2_sum += sa.t2();
        t3_sum += sa.t3();
#if 1   
        if (i == N-1) {
            if (result) 
            {
                //SemanticActions::result_type& r = sa.result();
                // std::cout << sa << std::endl;
                std::cout << "SemanticActionsTest average elapsed time for:\n"
                << "push string (" <<  sa.c0() << "): " 
                << std::fixed << std::setprecision(3) << (t0_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "pop operations: " 
                << std::fixed << std::setprecision(3) << (t3_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "array push_back operations(" <<  sa.c1() << "): " 
                << std::fixed << std::setprecision(3) << (t1_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "object insert operations(" <<  sa.c2() << "): " 
                << std::fixed << std::setprecision(3) << (t2_sum / N) * 1.0e3 << " ms" 
                << std::endl;
                
                std::cout << sa << std::endl;
                
            }
        }
#endif    
    }
    std::cout << "... end parsing\n";
    
    ifs.close();
    
    
    if (result) {
        printf("JsonParser: elapsed time:  min: %5.3f ms, max: %5.3f ms, avg: %5.3f ms\n", 
               t_min*1e3, t_max*1e3, t_tot*1e3 / N );
    }
    else {
        printf("JsonParser: error\n");
    }    
}


#if 0
// Note: using boost file streams
static void bench_parser3() 
{
    using namespace json;
    using utilities::timer;
    
    typedef boost::iostreams::stream<boost::iostreams::file_source> ifstream;
    
    std::cout << "using istream iterators" << std::endl;

    
    std:ifstream ifs(TEST_JSON);
    if (!ifs)
        throw std::runtime_error("could not open file");
    
    
    timer t = timer();
#if defined (DEBUG)
    const int N = 1;
#else    
    const int N = 1000;
#endif    
    
    typedef std::istream_iterator<char> iterator;
    typedef json::semantic_actions<json::unicode::UTF_8_encoding_tag> SemanticActions;
    
    bool result;
    SemanticActions sa;
    std::cout << "start parsing " << N << " times ... \n";
    double t0_sum = 0.0;
    double t1_sum = 0.0;
    double t2_sum = 0.0;
    double t3_sum = 0.0;
    double t_min = 1.0e99;
    double t_max = 0.0;
    double t_tot = 0.0;
    for (int i = 0; i < N; ++i) {
        sa.clear();
        ifs.clear();
        ifs.seekg(0);
        if (ifs.eof()) {
            std::cout << "error: eos bit set for file stream\n";
            break;
        }
        t.start();
        iterator first(ifs);
        iterator last;  // EOS
        result = parse(first, last, sa);
        t.stop();
        t_min = std::min(t_min, t.seconds());
        t_max = std::max(t_max, t.seconds());
        t_tot += t.seconds();
        t.reset();
        
        t0_sum += sa.t0();
        t1_sum += sa.t1();
        t2_sum += sa.t2();
        t3_sum += sa.t3();
#if 1   
        if (i == N-1) {
            if (result) 
            {
                //SemanticActions::result_type& r = sa.result();
                // std::cout << sa << std::endl;
                std::cout << "SemanticActionsTest average elapsed time for:\n"
                << "push string (" <<  sa.c0() << "): " 
                << std::fixed << std::setprecision(3) << (t0_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "pop operations: " 
                << std::fixed << std::setprecision(3) << (t3_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "array push_back operations(" <<  sa.c1() << "): " 
                << std::fixed << std::setprecision(3) << (t1_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "object insert operations(" <<  sa.c2() << "): " 
                << std::fixed << std::setprecision(3) << (t2_sum / N) * 1.0e3 << " ms" 
                << std::endl;
                
                std::cout << sa << std::endl;
                
            }
        }
#endif    
    }
    std::cout << "... end parsing\n";
    
    ifs.close();
    
    
    if (result) {
        printf("JsonParser: elapsed time:  min: %5.3f ms, max: %5.3f ms, avg: %5.3f ms\n", 
               t_min*1e3, t_max*1e3, t_tot*1e3 / N );
    }
    else {
        printf("JsonParser: error\n");
    }    
}
#endif


#if 0
// Note: using streambuf iterators
// Usually, this is the way to parse a file.
static void bench_parser4() 
{
    using namespace json;
    using utilities::timer;
    
    typedef boost::iostreams::stream<boost::iostreams::file_source> ifstream;
    
    std::cout << "using streambuf iterators" << std::endl;
    
    ifstream ifs(TEST_JSON);
    if (!ifs)
        throw std::runtime_error("could not open file");
    
    
    timer t = timer();
#if defined (DEBUG)
    const int N = 1;
#else    
    const int N = 1000;
#endif    
    
    typedef std::istreambuf_iterator<char> iterator;
    typedef json::semantic_actions<json::unicode::UTF_8_encoding_tag> SemanticActions;
    
    bool result;
    SemanticActions sa;
    std::cout << "start parsing " << N << " times ... \n";
    double t0_sum = 0.0;
    double t1_sum = 0.0;
    double t2_sum = 0.0;
    double t3_sum = 0.0;
    double t_min = 1.0e99;
    double t_max = 0.0;
    double t_tot = 0.0;
    for (int i = 0; i < N; ++i) {
        sa.clear();
        ifs.clear();
        ifs.seekg(0);
        if (ifs.eof()) {
            std::cout << "error: eos bit set for file stream\n";
            break;
        }
        t.start();
        iterator first(ifs.rdbuf());
        iterator last;  // EOS
        result = parse(first, last, sa);
        t.stop();
        t_min = std::min(t_min, t.seconds());
        t_max = std::max(t_max, t.seconds());
        t_tot += t.seconds();
        t.reset();
        
        t0_sum += sa.t0();
        t1_sum += sa.t1();
        t2_sum += sa.t2();
        t3_sum += sa.t3();
#if 1   
        if (i == N-1) {
            if (result) 
            {
                //SemanticActions::result_type& r = sa.result();
                // std::cout << sa << std::endl;
                std::cout << "SemanticActionsTest average elapsed time for:\n"
                << "push string (" <<  sa.c0() << "): " 
                << std::fixed << std::setprecision(3) << (t0_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "pop operations: " 
                << std::fixed << std::setprecision(3) << (t3_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "array push_back operations(" <<  sa.c1() << "): " 
                << std::fixed << std::setprecision(3) << (t1_sum / N) * 1.0e3 << " ms" 
                << std::endl  
                << "object insert operations(" <<  sa.c2() << "): " 
                << std::fixed << std::setprecision(3) << (t2_sum / N) * 1.0e3 << " ms" 
                << std::endl;
                
                std::cout << sa << std::endl;
                
            }
        }
#endif    
    }
    std::cout << "... end parsing\n";
    
    ifs.close();
    
    
    if (result) {
        printf("JsonParser: elapsed time:  min: %5.3f ms, max: %5.3f ms, avg: %5.3f ms\n", 
               t_min*1e3, t_max*1e3, t_tot*1e3 / N );
    }
    else {
        printf("JsonParser: error\n");
    }    
}
#endif

int main ()
{   
    std::cout << "===============================\n";
    std::cout << "Parser Benchmark\n";
    std::cout << "===============================\n";
    time_t time_info;    
    std::time(&time_info);
    std::cout << ctime(&time_info) << "\n";
    std::cout << BOOST_COMPILER << "\n";
    
    try {
        //bench_validating_parser();
        //bench_test_parser();
        bench_parser();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return -1;
    }
    return 0;
}



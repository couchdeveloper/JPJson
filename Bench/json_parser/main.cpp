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

#include <iostream>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <chrono>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <errno.h>

// #define JSON_INTERNAL_SEMANTIC_ACTIONS_TESTING

#include "json/parser/parser.hpp"
#include "json/parser/parse.hpp"
#include "json/parser/value_generator.hpp"
#include "json/utility/arena_allocator.hpp"
#include "utilities/bench.hpp"

namespace {
    
    
    const char* TEST_JSON = "mesh.json";
    //const char* TEST_JSON = "sample.json";
    
    std::vector<char> loadFileFromResourceFolder(const char* fileName) 
    {        
        std::string filePath("Resources/");
        filePath.append(fileName);
        std::ifstream ifs(filePath.c_str(), std::ios_base::in|std::ios_base::binary);
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


namespace test {
    
#if defined (DEBUG)
    constexpr int N = 1;
#else
    constexpr int N = 1000;
#endif
    
    
    template <typename Derived>
    struct bench_parser_base : bench_base<Derived, N>
    {
        typedef bench_base<Derived> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        
        bench_parser_base(std::string title) : title_(title) {}
        
        void prepare_imp()
        {
            std::cout << "\n--- " << title_ << " ---\n";
        }
                
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            std::cout << "elapsed time for parsing :\n"
            << "min: " << formatted_duration(min) <<
            ", max: " << formatted_duration(max) <<
            ", avg: " << formatted_duration(tot/n) << std::endl;
        }
        
        bool result() const { return result_; }
        bool& result() { return result_; }
        
        void teardown_imp() {};        
        

    private:
        bool result_;
        std::string title_;
    };
    
    
    
    struct bench_validating_parser : bench_parser_base<bench_validating_parser>
    {
        typedef bench_parser_base<bench_validating_parser> base;
        typedef typename bench_parser_base::timer timer;
        typedef typename bench_parser_base::duration duration;
        
        bench_validating_parser() : base("Bench Validating Parser")  {}
        
        void prepare_imp(std::string filePath)
        {
            base::prepare_imp();
            std::cout << "parsing file at path " << filePath << std::endl;
            std::cout << "using vector<char> iterators" << std::endl;
            json_ = loadFileFromResourceFolder(filePath.c_str());
        }
        
        duration bench_imp()
        {
            timer timer;
            std::vector<char>::const_iterator first = json_.begin();
            std::vector<char>::const_iterator last = json_.end();
            auto t0 = timer.now();
            result() = json::parse(first, last);
            return timer.now() - t0;
        }
        
    private:
        std::vector<char> json_;
    };
    
    
    struct bench_test_parser : bench_parser_base<bench_test_parser>
    {
        typedef bench_parser_base<bench_test_parser> base;
        typedef typename bench_parser_base::timer timer;
        typedef typename bench_parser_base::duration duration;
        
        typedef std::vector<char>::iterator iterator;
        typedef json::value_generator<json::unicode::UTF_8_encoding_tag> SemanticActions;
        
        bench_test_parser() : base("Bench Test Parser")  {}
        
        void prepare_imp(std::string filePath)
        {
            base::prepare_imp();
            std::cout << "parsing file at path " << filePath << std::endl;
            std::cout << "using vector<char> iterators, continuously using one semantic actions objects" << std::endl;
            json_ = loadFileFromResourceFolder(filePath.c_str());
        }
        
        duration bench_imp()
        {
            timer timer;

            sa_.clear();
            auto t0 = timer.now();
            std::vector<char>::const_iterator first = json_.begin();
            std::vector<char>::const_iterator last = json_.end();
            result() = json::parse(first, last, sa_);
            return timer.now() - t0;
        }
        
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            if (result()) {
                base::report_imp(min, max, tot, n);
                std::cout << sa_ << std::endl;
            } else {
                std::cout << "json test parser: error" << std::endl;
            }
        }
        
    private:
        SemanticActions sa_;
        std::vector<char> json_;
    };
    
    struct bench_parser : bench_parser_base<bench_parser>
    {
        typedef bench_parser_base<bench_parser> base;
        typedef typename bench_parser_base::timer timer;
        typedef typename bench_parser_base::duration duration;
        
        typedef json::value_generator<json::unicode::UTF_8_encoding_tag> SemanticActions;
        
        bench_parser() : base("Bench Parser")  {}
        
        void prepare_imp(std::string filePath)
        {
            base::prepare_imp();
            std::cout << "parsing file at path " << filePath << std::endl;
            std::cout << "using vector<char> iterators, continuously using one semantic actions objects" << std::endl;
            json_ = loadFileFromResourceFolder(filePath.c_str());
        }
        
        duration bench_imp()
        {
            timer timer;
            
            sa_.clear();
            auto t0 = timer.now();
            std::vector<char>::const_iterator first = json_.begin();
            std::vector<char>::const_iterator last = json_.end();
            result() = json::parse(first, last, sa_);
            return timer.now() - t0;
        }
        
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            if (result()) {
                base::report_imp(min, max, tot, n);
            } else {
                std::cout << "json parser: error" << std::endl;
            }
        }
        
    private:
        SemanticActions sa_;
        std::vector<char> json_;
    };
    
    
    struct bench_parser_arena_allocator : bench_parser_base<bench_parser_arena_allocator>
    {
        typedef bench_parser_base<bench_parser_arena_allocator> base;
        typedef typename bench_parser_base::timer timer;
        typedef typename bench_parser_base::duration duration;
                
        typedef json::utility::arena_allocator<void, json::utility::SysArena> Allocator;
        typedef json::value_generator<json::unicode::UTF_8_encoding_tag, Allocator> SemanticActions;
        
        bench_parser_arena_allocator()
        :   base("Bench Parser with Arena Allocator"),
            arena_(4*1024),
            a_(arena_),
            sa_(a_)
        {}
        
        void prepare_imp(std::string filePath)
        {
            base::prepare_imp();
            std::cout << "parsing file at path " << filePath << std::endl;
            std::cout << "using vector<char> iterators, "
                "\ncontinuously using one semantic actions objects (clearing it for each run)"
                "\ncontinuously using one arena (clearing it for each run)" 
            << std::endl;
            json_ = loadFileFromResourceFolder(filePath.c_str());
        }
        
        duration bench_imp()
        {
            timer timer;
            
            sa_.clear();
            arena_.clear();
            auto t0 = timer.now();
            std::vector<char>::const_iterator first = json_.begin();
            std::vector<char>::const_iterator last = json_.end();
            result() = json::parse(first, last, sa_);
            return timer.now() - t0;
        }
        
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            if (result()) {
                base::report_imp(min, max, tot, n);
                std::cout << "Arena: total memory allocated: " << arena_.totalSize() << " bytes" << std::endl;
                std::cout << "Arena: total memory used: " << arena_.bytesUsed() << " bytes"<< std::endl;
                //    std::cout << "Arena: min block size: " << arena_.minBlock() << " bytes"<< std::endl;
                std::cout << "Arena: number of blocks allocated: " << arena_.numberAllocatedBlocks() << std::endl;
                
            } else {
                std::cout << "json parser: error" << std::endl;
            }
        }
        
    private:
        std::vector<char> json_;
        json::utility::SysArena arena_;
        Allocator a_;
        SemanticActions sa_;
    };
    

    
    
    
    struct bench_parser_stream_iter : bench_parser_base<bench_parser_stream_iter>
    {
        typedef bench_parser_base<bench_parser_stream_iter> base;
        typedef typename bench_parser_base::timer timer;
        typedef typename bench_parser_base::duration duration;
        
        typedef json::value_generator<json::unicode::UTF_8_encoding_tag> SemanticActions;
        
        bench_parser_stream_iter() : bench_parser_base("Bench Parser With Stream")  {}
        
        void prepare_imp(std::string filePath)
        {
            base::prepare_imp();
            std::cout << "parsing file at path " << filePath << std::endl;
            std::cout << "using istream iterators, continuously using one semantic actions objects" << std::endl;
            buffer_ = static_cast<char*>(malloc(4096));
            ifs_.rdbuf()->pubsetbuf(buffer_, 4096);
            ifs_.open(std::string("Resources/").append(filePath), std::ios::binary);
            if (ifs_.fail())
                throw std::runtime_error("could not open file");
        }
        
        duration bench_imp()
        {
            typedef std::istream_iterator<char> iterator;
            
            timer timer;
            
            sa_.clear();
            ifs_.clear();
            ifs_.seekg(0);
            auto t0 = timer.now();
            iterator first(ifs_);
            iterator last;  // EOS
            result() = json::parse(first, last, sa_);
            return timer.now() - t0;
        }
        
        void teardown_imp() {
            ifs_.close();
            free(buffer_);
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            if (result()) {
                base::report_imp(min, max, tot, n);
            } else {
                std::cout << "json parser: error" << std::endl;
            }
        }
        
    private:
        SemanticActions sa_;
        std::ifstream ifs_;
        char* buffer_;
    };
    
    
    template <typename StreamIter>
    struct bench_validating_parser_stream_iter : bench_parser_base<bench_validating_parser_stream_iter<StreamIter>>
    {
        typedef bench_parser_base<bench_validating_parser_stream_iter<StreamIter>> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;
        
        typedef StreamIter iterator;
        
        bench_validating_parser_stream_iter() : base("Bench Validating Parser with Stream Iterator")  {}
        
        void prepare_imp(std::string filePath)
        {
            base::prepare_imp();
            std::cout << "parsing file at path " << filePath << std::endl;
            std::cout << "using istream iterators" << std::endl;
            buffer_ = static_cast<char*>(malloc(4096));
            ifs_.rdbuf()->pubsetbuf(buffer_, 4096);
            ifs_.open(std::string("Resources/").append(filePath), std::ios::binary);
            if (ifs_.fail())
                throw std::runtime_error("could not open file");
        }
        
        duration bench_imp()
        {
            timer timer;
            
            ifs_.clear();
            ifs_.seekg(0);
            auto t0 = timer.now();
            iterator first(ifs_);
            iterator last;  // EOS
            base::result() = json::parse(first, last);
            return timer.now() - t0;
        }
        
        void teardown_imp() {
            ifs_.close();
            free(buffer_);
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            if (base::result()) {
                base::report_imp(min, max, tot, n);
            } else {
                std::cout << "json parser: error" << std::endl;
            }
        }
        
    private:
        std::ifstream ifs_;
        char* buffer_;
    };
    
    
    template <typename StreamBufIter>
    struct bench_validating_parser_streambuf_iter : bench_parser_base<bench_validating_parser_streambuf_iter<StreamBufIter>>
    {
        typedef bench_parser_base<bench_validating_parser_streambuf_iter<StreamBufIter>> base;
        typedef typename base::timer timer;
        typedef typename base::duration duration;

        typedef StreamBufIter iterator;
        
        bench_validating_parser_streambuf_iter() : base("Bench Validating Parser with Streambuf Iterator")  {}
        
        void prepare_imp(std::string filePath)
        {
            base::prepare_imp();
            std::cout << "parsing file at path " << filePath << std::endl;
            std::cout << "using istreambuf iterators" << std::endl;
            buffer_ = static_cast<char*>(malloc(4096));
            ifs_.rdbuf()->pubsetbuf(buffer_, 4096);
            ifs_.open(std::string("Resources/").append(filePath), std::ios::binary);
            if (ifs_.fail())
                throw std::runtime_error("could not open file");
        }
        
        duration bench_imp()
        {
            
            timer timer;
            
            ifs_.clear();
            ifs_.seekg(0);
            
            auto t0 = timer.now();
            iterator first(ifs_);
            iterator last;  // EOS
            base::result() = json::parse(first, last);
            return timer.now() - t0;
        }
        
        void teardown_imp() {
            ifs_.close();
            free(buffer_);
        }
        
        void report_imp(duration min, duration max, duration tot, std::size_t n)
        {
            if (base::result()) {
                base::report_imp(min, max, tot, n);
            } else {
                std::cout << "json parser: error" << std::endl;
            }
        }
        
    private:
        std::ifstream ifs_;
        char* buffer_;
    };
    
    
    
    
    
}  // namespace test







static void bench_parser_stream_iter()
{
    test::bench_parser_stream_iter b;
    b.run(TEST_JSON);
}

static void bench_validating_parser_stream_iter()
{
    test::bench_validating_parser_stream_iter<std::istream_iterator<char>> b;
    b.run(TEST_JSON);
}

static void bench_validating_parser_streambuf_iter()
{
    test::bench_validating_parser_streambuf_iter<std::istreambuf_iterator<char>> b;
    b.run(TEST_JSON);
}


static void bench_parser_arena_allocator()
{
    test::bench_parser_arena_allocator b;
    b.run(TEST_JSON);
}

static void bench_validating_parser()
{
    test::bench_validating_parser b;
    b.run(TEST_JSON);
}

static void bench_test_parser()
{
    test::bench_test_parser b;
    b.run(TEST_JSON);
}

static void bench_parser()
{
    test::bench_parser b;
    b.run(TEST_JSON);
}








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
        bench_validating_parser();
        bench_validating_parser_stream_iter();
        bench_validating_parser_streambuf_iter();
        bench_parser();
        bench_parser_arena_allocator();
        bench_parser_stream_iter();
        bench_test_parser();
    }
    catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return -1;
    }
    return 0;
}



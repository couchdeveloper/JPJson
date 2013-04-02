//
//  main.cpp
//  Sample1
//
//  Created by Andreas Grosam on 21.03.13.
//  Copyright (c) 2013 Andreas Grosam. All rights reserved.
//

#include "json/parser/parse.hpp"
#include "json/parser/value_generator.hpp"
#include "json/value/value.hpp"
#include "json/generator/write_value.hpp"

#include <iostream>
#include <fstream>
#include <iterator>


int main(int argc, const char * argv[])
{
    // read a JSON text file in UTF-8:
    std::string filePath = "Resources/Test-UTF8.json";
    std::filebuf fb;
    if (fb.open(filePath.c_str(), std::ios_base::in|std::ios_base::binary) == nullptr) {
        std::cout << "ERROR: could not open file at path: " << filePath << std::endl;
        return -1;
    }
    
    //
    // Parse the JSON text and get a representation:
    //
    
    // First, define a Semantic Actions type from the built-in class template
    // `json::value_generator`, which builds a representation as a json::value:
    //
    // If we want a representation from a a JSON text, we need a "Semantic Actions"
    // object which builds such a JSON representation. Here, we use a Semantic
    // Actions class which builds a representation as a json::value. Internally,
    // JSON Object will be represented by a std::map, JSON Array will be
    // represented by a std::vector, JSON String will be represented as UTF-8
    // encoded std::string, etc. The concrete implementation details can can be
    // set via template parameters. Here we use the default template arguments,
    // which is generally fine for most use cases.
    // In addition to the JSON representation, this Semantic Actions object collects
    // a few other information about the parsed JSON.
    // Note that this all that depends on the concrete implementation of the
    // Semantic Actions class - and any can be defined and used in conjuction with
    // a parser as far as it implementis the required interface `semantic_actions_base`.
    typedef json::value_generator<> SemanticActions;

    // Define some handy types for the various JSON types:
    typedef typename SemanticActions::Value Value;
    typedef typename Value::object_type Object;
    typedef typename Value::array_type Array;
    typedef typename Value::string_type String;
    typedef typename Value::integral_number_type IntNumber;
    typedef typename Value::float_number_type FloatNumber;
    typedef typename Value::boolean_type Boolean;
    typedef typename Value::null_type Null;
    
    SemanticActions sa;
    
    
    // Start the parser with our smentic actions object:
    //
    // Pass the start and end pointer to the JSON text, as well as the semantic
    // actions object to the parse() function. The parse function returns true
    // if the parser succeeded.
    // Use istreambuf_iterators for the parser. This is faster than using
    // stream_iterators - but not as fast when iterating over a fixed size buffer
    // using const char* pointers.
    // Note: The parse function can accept any iterator types.
    std::istreambuf_iterator<char> first(&fb);
    std::istreambuf_iterator<char> last;  // eof
    bool result = json::parse(first, last, sa);

    if (result)
    {
        // The semantic actions object's result() function returns the root object
        // of our JSON. Since we don't need the JSON representation anymore within
        // our semantic actions object, we use std::move in order to "move construct"
        // our copy (which costs zero time).
        Value json0 = std::move(sa.result());
        
        // print info about our result:
        std::cout << sa << std::endl;
        
        // We expect an Array:
        if (json0.is_array())
        {
            // Pretty print the json root into a std::string.
            // We use function write_value() in order to write the JSON representation
            // (as a json::value) into an Output Iterator. write_value() accepts any
            // Output Iterator types.
            // This produces the JSON text representy by the json::value instance
            // which should be semantically equal to the original JSON which we read
            // from the file. The strings itself may differ, though.
            std::string jsonString;
            json::write_value(json0, std::back_inserter(jsonString), json::writer_base::pretty_print);
            std::cout << std::endl << jsonString << "\n\n" << std::endl;
            
            std::cout << "[1][\"source\"] = " << json0[1]["source"].as<String>() << std::endl;
            
            // validate this string again:
            auto first = jsonString.begin();
            auto last = jsonString.end();
            
            // Since we want to reuse the Semantic Actions object, we need to clear
            // all previously collected data. Reusing a Semantic Actions object
            // may yield better performance and possibly lower memory utilization.
            // The details lay in the implemention of the Semantic Actions class.
            // For example, a certain Semantic Actions implemention may cache
            // key strings, or reuse previously allocated buffers.
            sa.clear();
            
            if (not json::parse(first, last, sa) and first != last) {
                std::cout << "parser error: invalid JSON" << std::endl;
            }
            // print info about our result:
            std::cout << sa << std::endl;
                        
            Value json1 = std::move(sa.result());
            if (json0 != json1) {
                std::cout << "ERROR: json's did not compare equal!" << std::endl;
            }
            
            // "normal" print the json root into a std::string:
            jsonString.clear();
            json::write_value(json0, std::back_inserter(jsonString));
            std::cout << std::endl << jsonString << "\n\n" << std::endl;
            
            // validate this string again:
            first = jsonString.begin();
            last = jsonString.end();
            sa.clear();
            if (not json::parse(first, last, sa) and first != last) {
                std::cout << "parser error: invalid JSON" << std::endl;
            }
            // print info about our result:
            std::cout << sa << std::endl;
            Value json2 = std::move(sa.result());
            if (json0 != json2) {
                std::cout << "ERROR: json's did not compare equal!" << std::endl;
            }
            
            
            // print the json root into a std::string using Escaped Unicode encoding:
            jsonString.clear();
            json::write_value(json0, std::back_inserter(jsonString), 0, json::unicode::escaped_unicode_encoding);
            std::cout << std::endl << jsonString << "\n\n" << std::endl;
            
            // validate this string again:
            first = jsonString.begin();
            last = jsonString.end();
            sa.clear();
            if (not json::parse(first, last, sa) and first != last) {
                std::cout << "parser error: invalid JSON" << std::endl;
            }
            // print info about our result:
            std::cout << sa << std::endl;
            Value json3 = std::move(sa.result());
            if (json0 != json3) {
                std::cout << "ERROR: json's did not compare equal!" << std::endl;
            }
            
            
        }
        else {
            std::cout << "ERROR: Expected Array as JSON root" << std::endl;
        }
    }
    else {
        std::cout << "parser error" << std::endl;
    }
    
    fb.close();
    return 0;
}


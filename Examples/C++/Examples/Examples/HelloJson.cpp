//
//  HelloJson.cpp
//  Examples
//
//
//  Created by Andreas Grosam on 6/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <string.h>

// Notes:
//
// Setting up a target:
//
// Ensure that a header search path is defined which points to the location
// of the json_parser folder.
// 
// Ensure, that the target has a header search path defined which points
// to the boost library, e.g.: "/opt/local/include" 
//
// The json library is header only. That means, there is no need to add any
// C++ modules to the project or link to a library, just include the required
// headers and you are done.
// 
//
// Useful preprocessor defines:
// define NDBEUG in your Release build.


// The simple example requires to include one header file:
#include "json/json.hpp"


// All of the json names are declared in namespace json. For convenience,
// we define a using namespace declaration:



using namespace json;

int main (int argc, const char * argv[])
{
    // This sample is mainly for testing whether the programm can compile
    // successfully and shows the most basic usgae.

    // We want to parse a json text, so for simplicity, we define a valid json 
    // text using a string literal. The json text is a json array containing one
    // json string:
    const char* jsonText = "[\"Hello Json!\"]";
    
    // Note that a json text may be encoded in any of the UTF encoding forms,
    // UTF-8, UTF-16 and UTF-32. Defining a json text this way, that is using a
    // string literal whose char type is "char" the parser implicitly assumes
    // that the encoding form equals UTF-8 (which is the only reasonable
    // conclusion, if you think about it). You must be carefully though when
    // defining a string literal within the source code which shall contain a 
    // UTF-8 encoded text. As long as you only use ASCII characters, there's no 
    // problem. If you include characters not in the ASCII set, you should enter 
    // your string using escape sequences. The C and C++ standard may tell you 
    // more about this.
    
    // Our parse() function requires a "semantic actions" instance which
    // gives us the possibility to retrieve the parsers result - that is, the 
    // json container. It is also a great way to fine tune certain aspects of 
    // the parser's behavior and how the json container is created and implemnted.
    // 
    // For now, it is enough to use the default semantic actions class, which
    // is defined in namespace json::semantic_actions. The SemanticActions class
    // is actually a template class, where the iternal details for parsing can be
    // specifiefd. For now, the default template parameters suffice:
    semantic_actions::SemanticActions<> sa;
    
    // The parse() function requires two iterators, one that specifies the
    // start of the json text and the other iterator specifies the end of the
    // text. The iterators can be anything that fullfills the "Input Iterator"
    // concept, that means you can virtually use any iterator, e.g.: pointers 
    // to char buffers, as well as stream iterators and others.    
    const char* iter = jsonText;
    const char* eos = iter + strlen(jsonText);
    
    // Here we call the parse function which takes three parameters as discussed
    // above: the two iteraters, and the semantic actions instance. Note,
    // that the start of the text is a reference to an iterator and it will
    // be modified, that is incremented as the parser processess the text.
    // You may check this iterator when something went wrong during parsing
    // so you get the position where the parser stopped processing the input.
    bool result = parse(iter, eos, sa);
    
    if (result) {
        std::cout << sa.result() << std::endl;
        // This should print out '["Hello Json!"]'
    }
    else {
        std::cout << "Error while parsing: " << sa.error().second << std::endl;
    }
    
    
    
    
    
#if 0    
    // A valid json object contains either a json object or an json
    // array. We use an array here for testing if our project is set up
    // correctly.
    
    // Create a json string, and initialize it:
    String s = "Hello Json";
    
    // Create an empty json array:
    Array a;
    
    // Now add the string to the array:
    a.push_back(s);
    
    // Our json container is a "Value" instance. This is actually a
    // "discriminated union". That is it can be anything which is a
    // valid json type. In order to be a valid json container, however,
    // a json value must be either an array or an object:    
    Value json = a;
    
    // That's a valid json container!  :)
    
    
    // now print this json container to std console:
    std::cout << json << std::endl;
#endif    
    
    return 0;
}


//
//  Sample.cpp
//  Examples
//
//  Created by Andreas Grosam on 6/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


#include <iostream>
#include <string>
#include "json/json.hpp"


using json::string;
using json::array;
using json::semantic_actions::SemanticActions;
using json::parse;

typedef string<char> String;


int main (int argc, const char * argv[])
{
    // A valid json object contains either a json object or an json array.
    // We setup a valid json text, parse it and examine how to access the
    // json container, or: "What is the parsers result at all?"
    
    // Create a json text:
    std::string text = "{\"name\": \"Andreas\", \"lastName\": \"Grosam\"}";
    
    
    SemanticActions<> sa;
    
    // When we define the semantics actions class, which is a template class,
    // we automatically generate various types, which are declared in the
    // semantic actions class. In particular this are the various "json-types", 
    // that is the types: 
    //
    // Value, String, Boolean, Array, Object and Null.
    //
    // 
    // For our convenience and for illustration we define a couple 
    // of typedefs:
    
    typedef SemanticActions<>               semantic_actions_t;
    
    // The various json types:
    typedef semantic_actions_t::Value       Value;
    typedef semantic_actions_t::Array       Array;
    typedef semantic_actions_t::Object      Object;
    typedef semantic_actions_t::Boolean     Boolean;
    typedef semantic_actions_t::String      String;
    typedef semantic_actions_t::Number      Number;
    typedef semantic_actions_t::Null        Null;
    
    //
    //  *                                                                 *
    //  *  When you use a semantics actions class, you should always get  *
    //  *  the various json types as it is shown above!                   *
    //  *                                                                 *
    //    
    
    
    // The type Value is a discriminated union and is exactly the result
    // type of the semantic actions instance, which is available after the
    // parser has successfully finsihed parsing the json text.
    // 
    
    // The concrete type of the json container which will be created by the
    // semantic actions class and which can be accesses by the result() member
    // function can be determined as SemanticActions<>::result_type. This type 
    // is just an alias for SemanticActions<>::Value.
    
    
    
    // As usual, we need the begin and end iterators of the json text,
    // and the semantics actions instance and start the parser with the
    // parser function:
    std::string::iterator iter = text.begin();
    std::string::iterator eos = text.end();    
    bool result = parse(iter, eos, sa);
    
    if (result) 
    {
        // Parsing was successful.
        // Here, we use a reference to the json container which is
        // a member of the semantic actions instance. 
        Value& jsonValue = sa.result();
        
        // A valid json text results either in an array or an object.
        // Check what we got:
        
        if (jsonValue.is_type<Array>()) {
            std::cout << "we got an json array!" << std::endl;
        }
        else if (jsonValue.is_type<Object>()) {
            std::cout << "we got an json object!" << std::endl;
        }
        else {
            std::cout << "Ohps, we got something wierd!" << std::endl;
        }
        
        // We expected a json object here, which is easy to detect, since
        // we created the json text as such  ;)        
        Object& o = jsonValue.as<Object>();
        
        // If we got to here, we actually got an json object. Otherwise, an
        // excpetion would have been thrown.
        
        // We possibly know that there is a key "name" in the object,
        // but we check that first:
        if (o.has_key("name"))
        {            
            // OK, retrieve the associated value for this key:
            Value v = o["name"];
            std::cout << "The value of key 'name' equals: " << v << std::endl;
            // Note: 'v' will be printed like a json container. This is due to
            // the fact that v is of type Value. If we get the actual type from
            // the discriminated union, as follows ...
            String s = v.as<String>();
            // the the json string prints as:            
            std::cout << "The string prints as: " << s << std::endl;
            
            // Be carefully, when using template member function Value::as<>, 
            // since it may throw an exception if the actual type isn't what 
            // you expect:
            
            try {
                Array& a = v.as<Array>();
                std::cout << a << std::endl;
            } 
            catch (std::exception& ex)
            {
                std::cout << "Ohps, got error: " << ex.what() << std::endl;
            }
            
        }
        else {
            std::cout << "Error: the json object does not contain the key 'name'" << std::endl;
        }
        
        
        // OK, print the json container know, to see what's in it:
        std::cout << sa.result() << std::endl;

    }
    else {
        std::cout << "error while parsing" << std::endl;
    }
    
    
    return 0;
}


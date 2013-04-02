//
//  unicode_filter.hpp
//  
//
//  Created by Andreas Grosam on 1/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef JSON_UNICODE_UNICODE_FILTER_HPP
#define JSON_UNICODE_UNICODE_FILTER_HPP

#include "json/config.hpp"
#include "unicode_utilities.hpp"



#pragma mark - Conversion Filter
namespace json { namespace unicode { namespace filter {
    
    using json::unicode::code_point_t;
    using json::unicode::kReplacementCharacter;
    using json::unicode::isNonCharacter;
    using json::unicode::isSurrogate;
    
    
    struct filter_tag {};
    
    struct None : filter_tag
    {        
        bool operator() (code_point_t) const { return false; }
        bool replace() const { return false; }
        code_point_t replacement(code_point_t cp) const { return cp; }
    };
    
    struct Noncharacter : filter_tag
    {        
        Noncharacter(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            return isNonCharacter(cp); 
        }
        
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
    private:
        code_point_t replacement_;
    };
    
    struct Surrogate : filter_tag
    {        
        Surrogate(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            return isSurrogate(cp); 
        }
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
        
    private:
        code_point_t replacement_;
    };
    
    struct SurrogateOrNoncharacter : filter_tag
    {        
        SurrogateOrNoncharacter(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            return isSurrogate(cp) or isNonCharacter(cp); 
        }
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
        
    private:
        code_point_t replacement_;
    };
    
    
    struct NoncharacterOrNULL : filter_tag
    {        
        NoncharacterOrNULL(code_point_t repl = kReplacementCharacter) 
        : replacement_(repl)
        {
        }
        
        bool operator() (code_point_t cp) const {
            bool result = cp == 0 or isNonCharacter(cp); 
            return result;
        }
        bool replace() const { return replacement_ != 0; }
        
        code_point_t replacement(code_point_t) const { 
            return replacement_; 
        }
        
        void replacement_character(code_point_t replacement) {
            replacement_ = replacement;
        }
        
        code_point_t replacement_character() const {
            return replacement_;
        }
        
    private:
        code_point_t replacement_;
    };
    
    
    
}}}


#endif   // JSON_UNICODE_UNICODE_FILTER_HPP

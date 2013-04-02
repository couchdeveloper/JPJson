//
//  json_string.hpp
//  
//
//  Created by Andreas Grosam on 22.02.13.
//
//

#ifndef JSON_UTILITY_JSON_NUMBER_HPP
#define JSON_UTILITY_JSON_NUMBER_HPP


namespace json { namespace utility {
    
    template <typename Iterator>
    bool is_valid_json_number(Iterator& first, Iterator last)
    {
        if (first == last)
            return false;
        
        if (*first == '-')
            ++first;
        
        if (first == last or !std::isdigit(*first)) {
            return false;
        }
        
        // one digit ...
        ++first;
        while (first != last and std::isdigit(*first++))
        {
        }
        if (first == last) {
            return true;
        }
        
        if (*first == '.')
        {
            ++first;
            if (first == last or !std::isdigit(*first)) {
                return false;
            }
            // one digit after decimal point
            ++first;
            while (first != last and std::isdigit(*first++))
            {
            }
            if (first == last) {
                return true;
            }
            
        }
        if (*first == 'e' or *first == 'E') {
            first++;
            if (first == last)
                return false;
            if (*first == '-' or *first == '+')
                ++first;
            if (first == last or !std::isdigit(*first)) {
                return false;
            }
            ++first;
            while (first != last and std::isdigit(*first++))
            {
            }
        }
        return true;
    }

    
        
    static bool is_valid_json_number(const char* s)
    {
        if (*s == '-') ++s;
        
        if (!std::isdigit(*s)) {
            return false;
        }
        ++s;
        while (std::isdigit(*s)) {
            ++s;
        }
        if (*s == '.')
        {
            ++s;
            if (!std::isdigit(*s)) {
                return false;
            }
            ++s;
            while (std::isdigit(*s)) {
                ++s;
            }
        }
        if (*s == 'e' or *s == 'E') {
            s++;
            if (*s == '-' or *s == '+') ++s;
            if (!std::isdigit(*s)) {
                return false;
            }
            ++s;
            while (std::isdigit(*s)) {
                ++s;
            }
        }
        if (*s == 0)
            return true;
        else
            return false;
    }



}}




#endif

//
//  main.cpp
//  Unicode
//
//  Created by Andreas Grosam on 26.03.13.
//  Copyright (c) 2013 Andreas Grosam. All rights reserved.
//

#include <iostream>
#include "json/unicode/unicode_converter.hpp"
#include "json/unicode/unicode_conversion.hpp"
#include <cstring>


using namespace json::unicode;


//int convert(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding,
//        OutIteratorT& dest, ToEncodingT toEncoding,
//        mb_state<FromEncodingT>& state,
//        ConvertOption convertOption = None,
//        typename std::enable_if<
//        !std::is_same<
//        typename std::iterator_traits<InIteratorT>::iterator_category,
//        std::random_access_iterator_tag
//        >::value
//        >::type* dummy = 0)


//inline int
//convert(InIteratorT& first, InIteratorT last, FromEncodingT fromEncoding,
//        OutIteratorT& dest, ToEncodingT toEncoding,
//        ConvertOption convertOption = None,
//        typename std::enable_if<
//        !std::is_same<
//        typename std::iterator_traits<InIteratorT>::iterator_category,
//        std::random_access_iterator_tag
//        >::value
//        >::type* dummy = 0)


int main(int argc, const char * argv[])
{

    const char* s = "This is UTF-8";
    
    const char* first = s;
    const char* last = first + strlen(s);
    
    uint16_t buffer[256];
    uint16_t* dest = buffer;
    
    int result = convert(first, last, UTF_8_encoding, dest, UTF_16_encoding);
    
    return result;
}


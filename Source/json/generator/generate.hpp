//
//  generate.hpp
//  
//
//  Created by Andreas Grosam on 5/23/11.
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

#ifndef JSON_GENERATE_HPP
#define JSON_GENERATE_HPP


#include "json/config.hpp"
#include <assert.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include "json/unicode/unicode_utilities.hpp"
#include "json/unicode/unicode_conversions.hpp"
#include <algorithm>


namespace json { namespace generator_internal {
   
    //
    // Copies a BOM for the specified encoding into a range beginning with dest.
    //
    // Returns an iterator to the end of the destination range (which points to 
    // the element following the BOM).
    //
    // OutputIterator shall be appropriate for the encoding.

    template <typename OutputIterator, typename EncodingT>
    inline OutputIterator
    copyBOM(OutputIterator dest, EncodingT encoding)
    {
        BOOST_STATIC_ASSERT( (boost::is_base_and_derived<json::unicode::utf_encoding_tag, EncodingT>::value) );
        
        typedef typename EncodingT::bom_type bom_type;
        bom_type bom = EncodingT::bom();
        return std::copy(bom.begin(), bom.end(), dest);        
    }
    
    
    // Escape the Unicode sequence starting with first. 
    // Only charaters which are required to be escaped according the JSON
    // specification RFC 4627 will be escaped through prefixing it with a
    // reverse solidus.
    // These are namely the following characters:
    // Control characters (U+0000 through U+001F), quation marks ('"') and 
    // reverse solidus ('\'). The solidus character ('/') will be optionally
    // escaped if parameter escapeSolidus equals true. 
    // The input range [first .. last) shall contain a wellformed Unicode 
    // sequence in the specified Unicode encoding.
    // 
    // Returns: 
    //  The number of inserted code units in the output encoding.
    //  If an error occured, the out parameter error contains a
    //  value indicating the error.
    //  Always check the error on return.
    //
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT
    >
    inline std::size_t
    escape_convert_unsafe(
                          InIteratorT&     first, 
                          InIteratorT      last, 
                          InEncodingT      inEncoding,
                          OutIteratorT&    dest,
                          OutEncodingT     outEncoding,
                          bool             escapeSolidus,
                          int&             error)
    {
        typedef typename boost::iterator_value<InIteratorT>::type in_code_unit;
        typedef typename boost::make_unsigned<in_code_unit>::type uin_code_unit;        
        
        std::size_t count = 0;
        while (first != last) {
            int result;
            uin_code_unit ch = static_cast<uin_code_unit>(*first);
            in_code_unit escapedSequence[8];
            
            bool escaped = true;
            in_code_unit* startEscaped = static_cast<in_code_unit*>(escapedSequence);
            in_code_unit* endEscaped = startEscaped;
            switch (ch) {
                case static_cast<uin_code_unit>('"'):   *endEscaped++ = '\\'; *endEscaped++ = '"'; break;
                case static_cast<uin_code_unit>('\\'):  *endEscaped++ = '\\'; *endEscaped++ = '\\'; break;
                case static_cast<uin_code_unit>('/'):   
                    if (escapeSolidus) {
                        *endEscaped++ = '\\'; *endEscaped++ = '/';
                    } else {
                        escaped = false;
                    }
                    break;
                case static_cast<uin_code_unit>('\b'):  *endEscaped++ = '\\'; *endEscaped++ = 'b'; break;
                case static_cast<uin_code_unit>('\f'):  *endEscaped++ = '\\'; *endEscaped++ = 'f'; break;
                case static_cast<uin_code_unit>('\n'):  *endEscaped++ = '\\'; *endEscaped++ = 'n'; break;
                case static_cast<uin_code_unit>('\r'):  *endEscaped++ = '\\'; *endEscaped++ = 'r'; break;
                case static_cast<uin_code_unit>('\t'):  *endEscaped++ = '\\'; *endEscaped++ = 't'; break;                    
                default:
                    if (ch < 0x20u) {
                        // escape a control character
                        *endEscaped++ = '\n';
                        *endEscaped++ = 'u';
                        *endEscaped++ = '0';
                        *endEscaped++ = '0';
                        *endEscaped++ = ((ch >> 4)+'0');
                        *endEscaped++ = ((ch & 0x0Fu)+'0');
                    }
                    else {
                        // unescaped
                        escaped = false;
                    }
            } // switch
            
            if (escaped) {
                result = (int)json::unicode::convert_unsafe(startEscaped, endEscaped, inEncoding, dest, outEncoding, error);
                if (result) {
                    ++first;
                }
            }
            else {
                result = convert_one_unsafe(first, last, inEncoding, dest, outEncoding);
            }
            if (result > 0)
                count += result;
            else {
                error = result;
                return 0;
            }
        } // while
        
        error = 0;
        return count;
    }    
    
    
    

    
    
}} // namespace json::generator_internal



#endif

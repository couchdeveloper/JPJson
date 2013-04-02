//
//  base64.hpp
//  base64
//
//  Created by Andreas Grosam on 11/14/11.
//  Created by Andreas Grosam on 5/16/11.
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

#ifndef BASE64_BASE64_HPP
#define BASE64_BASE64_HPP


#include <boost/iterator/iterator_traits.hpp>
#include <stdexcept>
#include <stdint.h>
#include <sstream>
#include <iostream>
#include <cassert>

//  Generate lookup table:
// 
//    int main (int argc, const char * argv[])
//    {
//        const std::string base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
//        
//        std::vector<char> lookup;
//        char min = *std::min_element(base64chars.begin(), base64chars.end());
//        char max = *std::max_element(base64chars.begin(), base64chars.end());
//        for (char i = min; i <= max; ++i) 
//        {        
//            std::size_t pos = base64chars.find_first_of(i);
//            if (i != '=')
//                lookup.push_back(pos != std::string::npos ? base64chars.find_first_of(i) : 0xFF);
//            else 
//                lookup.push_back(0xFE);
//        }
//        
//        std::cout << "offest: " << (int)(min) << std::endl;
//        std::ostream_iterator<int> out(std::cout, ",");
//        std::copy(lookup.begin(), lookup.end(), out);
//        std::cout << std::endl;
//        
//        return 0;
//    }



namespace json { namespace utility {

    namespace base64_detail {
    
        template <typename T> 
        int to_int(const T& v) {
            return static_cast<int>(static_cast<uint8_t>(v));
        }
    
    }

        
    // Encode a sequence of bytes starting at 'first' into its base64 representation
    // using the usual alphabet and write the result into output iterator 'result'.
    // Note: this encoding does not embed new lines.
    template <class InputIterator, class OutputIterator>
    OutputIterator encodeBase64(InputIterator first, InputIterator last, OutputIterator result)
    {
        static_assert( (sizeof(typename boost::iterator_value<InputIterator>::type) == sizeof(char)), "" );
        
        using base64_detail::to_int;
        
        const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        while (first != last)
        {
            // Read the next three bytes (if available) into one 24-bit number:
            uint32_t n = to_int(*first) << 16;
            ++first;
            if (first != last) {
                n |= (to_int(*first) << 8);
                ++first;
                if (first != last) { 
                    n |= to_int(*first);   
                    ++first;
                    // encode it into 4 bytes:
                    *result++ = base64chars[static_cast<uint8_t>(n >> 18) & 0x3Fu];
                    *result++ = base64chars[static_cast<uint8_t>(n >> 12) & 0x3Fu];
                    *result++ = base64chars[static_cast<uint8_t>(n >> 6) & 0x3Fu];
                    *result++ = base64chars[static_cast<uint8_t>(n) & 0x3Fu];
                    //continue;                    
                }
                else {
                    // padding = 1
                    *result++ = base64chars[static_cast<uint8_t>(n >> 18) & 0x3Fu];
                    *result++ = base64chars[static_cast<uint8_t>(n >> 12) & 0x3Fu];
                    *result++ = base64chars[static_cast<uint8_t>(n >> 6) & 0x3Fu];
                    *result++ = '=';
                }
            }
            else {
                //padding = 2
                *result++ = base64chars[static_cast<uint8_t>(n >> 18) & 0x3Fu];
                *result++ = base64chars[static_cast<uint8_t>(n >> 12) & 0x3Fu];
                *result++ = '=';
                *result++ = '=';
            }
        }
        
        assert(first == last);        
        return result;
    }

        
    namespace base64_detail {
        
        
        // Returns
        // 
        // Errors:
        //  -1: invalid character
        //  -2: padding character
        inline int lookup(uint8_t c) 
        {
            //const char padChar = '=';
            // Generated lookup table for "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=".
            // offset: 43
            // The pad character is included and counts as -2.            
            const int8_t lookup_table[] = {
                62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,
                1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,
                -1,-1,-1,-1,-1,26,27, 28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,
                43,44,45,46,47,48,49,50,51
            };
            
            // check range:
            c -= 43u;
            if ( c < static_cast<uint8_t>(sizeof(lookup_table)) ) {
                return lookup_table[c];
            }
            else { 
                return -1;
            }
        }
        
    }

    
    // Decode a base64 encoded sequence starting at 'first' into its binary
    // repesentation and write it into output iterator 'result'.
    // The base62 sequence shall not contain new lines.
    template <class InputIterator, class OutputIterator>
    OutputIterator decodeBase64(InputIterator first, InputIterator last, OutputIterator result)
    {
        static_assert(sizeof(typename boost::iterator_value<InputIterator>::type) == sizeof(char), "");
        
        using base64_detail::to_int;
        using base64_detail::lookup;
        
        // The length of the input shall be a modulo of 4
        
        // Errors:
        //  invalid character
        //  spurious bytes after last padding character
        //  expected padding character
        //  unexpected EOF
        // 
        
        enum error_t {
            ErrorNone = 0,
            ErrorInvalidCharacter = -1,
            ErrorSpuriousBytes = -2,
            ErrorExpectedPaddingCharacter = -3,
            ErrorUnexpectedEOF = -4
        };
        
        int error = ErrorNone;
        while (first != last) 
        {            
            int32_t n = lookup(static_cast<uint8_t>(*first));                                
            if (n >= 0 and ++first != last) {
                int32_t c = lookup(static_cast<uint8_t>(*first));
                if (c >= 0 and ++first != last) {
                    n <<= 6;
                    n |= c;
                    c = lookup(static_cast<uint8_t>(*first));
                    if (++first != last) { 
                        if (c >= 0) {
                            n <<= 6;
                            n |= c;
                            c = lookup(static_cast<uint8_t>(*first));
                            if (c >= 0) {
                                n <<= 6;
                                n |= c;
                                // write 3 bytes:
                                *result++ = (n>>16) & 0x000000FF;
                                *result++ = (n>>8) & 0x000000FF;
                                *result++ = (n) & 0x000000FF;
                                ++first;
                                continue;
                            }
                            else {
                                if (c == -2) {
                                    // first and last padding character
                                    if (++first != last) {
                                        error = ErrorSpuriousBytes; // error: spurious bytes after last padding character
                                        break;
                                    }
                                    *result++ = (n>>10) & 0x000000FF;
                                    *result++ = (n>>2) & 0x000000FF;
                                    assert(first == last);
                                    return result;
                                }
                                else {
                                    error = ErrorInvalidCharacter; // error: invalid character
                                    break;
                                }
                            }
                        }
                        else {
                            if (c == -2) {
                                // 1. padding character, expecting second one
                                c = lookup(static_cast<uint8_t>(*first));
                                if (c != -2) {
                                    error = ErrorExpectedPaddingCharacter; // error: expected padding character
                                    break;
                                }
                                if (++first != last) {
                                    error = ErrorSpuriousBytes; // error: spurious bytes after last padding character
                                    break;
                                }
                                *result++ = (n>>4) & 0x000000FF;
                                assert(first == last);
                                return result;
                            }
                            else {
                                error = ErrorInvalidCharacter;// error: invalid character
                                break;
                            }
                        }
                    } else {
                        error = ErrorUnexpectedEOF; // error: unexpected EOF
                        break;
                    }
                }
                else {
                    // error: unexpected EOF or invalid byte or invalid padding
                    if (c < 0) {
                        error = ErrorInvalidCharacter;
                    }
                    else {
                        error = ErrorUnexpectedEOF;
                    }
                    break;                        
                }
            } else {
                // error: unexpected EOF or invalid byte or invalid padding
                if (n < 0) {
                    error = ErrorInvalidCharacter;
                }
                else {
                    error = ErrorUnexpectedEOF;
                }
                break;                        
            }
        }  // while
        
        if (error != ErrorNone) {
            // We reach here only in case of an error
            // Gather some error info:
            bool eof_reached = first == last;
            int32_t ch = eof_reached ? EOF : static_cast<uint8_t>(*first);
            std::stringstream ss(std::stringstream::out);
            switch (error) {
                case ErrorInvalidCharacter:
                    ss << "Invalid character:" << std::hex << "0x" << ch;
                    break;
                case ErrorSpuriousBytes:
                    ss << "Spurious bytes: ";
                    std::copy(first, last, std::ostream_iterator<uint8_t>(ss));
                    break;
                case ErrorExpectedPaddingCharacter:
                    ss << "Expected padding character, got " << std::hex << ch;
                    break;
                case ErrorUnexpectedEOF:
                    ss << "Unexpected EOF";
                    break;
                default:;
            }
            
            std::string msg = ss.str();
            std::cerr << "Error: " << msg << std::endl;
            throw std::runtime_error(msg);
        }
        assert(first == last);
        return result;        
    }
    
    
    
}}  // namespace json::utility
    
    

#endif

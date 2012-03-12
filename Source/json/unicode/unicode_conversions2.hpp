//
//  unicode_conversions2.hpp
//  
//
//  Created by Andreas Grosam on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _unicode_conversions2_hpp
#define _unicode_conversions2_hpp

#error This file is deprecated!


#include "json/config.hpp"
#include "unicode_utilities.hpp"
#include "codepoint_cvt.hpp"
#include "utf_cvt.hpp"
#include "unicode_filter.hpp"

#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>




#pragma mark - UTF to Unicode Code Point Conversions
namespace json { namespace unicode {
    // 
    //  UTF to Unicode Code Point Conversions
    //
    //  Synopsis:
    //
    // A.1
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, Encoding encoding,
    //                       code_point_t& code_point)
    //
    // A.2
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point, Filter filter)
    //
    // A.3
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, code_point_t& code_point)
    //
    // A.4
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, code_point_t& code_point,
    //              Filter filter)
    //
    // B.1
    //  int 
    //  convert_to_codepoint_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point)
    //
    // B.2
    //  int 
    //  convert_to_codepoint_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point, Filter filter)
    //
    // B.3
    //  int 
    //  convert_to_codepoint_unsafe(Iterator& first, Iterator last, code_point_t& code_point)
    //
    // B.4
    //  int 
    //  convert_to_codepoint_unsafe(Iterator& first, Iterator last, code_point_t& code_point,
    //                     Filter filter)
    //
    //
    //
    // Converts a sequence of code units constituting one Unicode charater 
    // encoded in encoding to an Unicode code point.
    //
    // The allowed encodings forms of the input sequence are UTF-32, UTF-16
    // and UTF-8, with their respective endianness variants.
    //
    // If the encoding is not specified, an appropriate encoding type from the
    // iterator's value_type will be generated. This encoding type is always in
    // host endianness.
    //
    // The functions takes two iterators, the first iterator points to the start
    // of a minimal well-formed code unit subsequence which maps to a single
    // Unicode scalar value.
    // The second iterator shall point past the end of the sub sequence of code 
    // units constituting the first unicode character, or it may point farther 
    // away.
    //
    // The *_safe versions will detect ill-formed Unicode code sequences. The
    // Unicode conformance clause (C10) requires not to interpret any ill-formed
    // code unit subsequences in a string as characters. Thus, the safe functions
    // will stop advancing the iterator at the point where the ill-formness
    // has been detected. The iterator 'first' may then point to the start of
    // a minimal well-formed subsequence.
    // 
    //
    // Optionally, a Filter operator can be specified. If the filter evaluates
    // true for a certain code point, it will be checked if it shall be replaced.
    // If this is true, the code point will be replaced by the substitution. If
    // this is false, an error E_PREDICATE_FAILED will be returned, and the
    // parameter code_point contains the original code point.
    // Otherwise, the code point will be accepted as is.
    // 
    // Both variants (safe and unsafe versions) expect that the input sequence
    // is not empty (assert(first != last). The safe version will check for an
    // unexpected end of input sequence, though. 
    //
    // The unsafe version ignores parameter last, and will advance parameter 
    // first as far as required. The unsafe version assumes that the input
    // sequence is a well-formed Unicode sequence. If the sequence is in fact
    // mal-formed, ends prematurely, or if the the input sequence is empty, the 
    // behavior is undefined. Calling an unsafe version with ill-formed input
    // is strongly discouraged, since it may cause the application to crash,
    // returns bogus results, or otherwise corrupt the state of the application.
    //
    // When the function returns successfully, zero will be returned, and 
    // iterator 'first' has been advanved by the number of code units consti-
    // tuting a code point. Parameter code_point has been assigned the result of
    // the conversion.
    //
    // When the functions fails, parameter first will be advanced until the
    // input sequence can be detected as ill-formed, and a negative integer will
    // be returned, indicating an error. Input parameter code_point is left un-
    // changed.
    //
    // The safe version returns zero on success. Otherwise an error code.
    // 
    // The unsafe version assums well-formed input, and returns zero. Otherwise
    // (if the input is not well-formed) the behavior is undefined.
    //
    // The algorithm can be used with any iterator type.
    // Optimizations for certain iterator types are possible.
    // The EncodingT sepcifies the Iterator's encoding.
    //
    //
    //  Examples:
    //  
    //  1)  Convert a sequence of UTF-8 code units to Unicode code points and 
    //      store them in an vector.
    //   
    //  std::vector<unit8_t>  input = ...;
    //  std::vector<uint32_t> output;
    //  std::vector<unit8_t>::iterator first = input.begin();
    //  std::vector<unit8_t>::iterator last = input.last();
    //
    //  while (first != last) {
    //      code_point_t cp;
    //      if (convert_one(first, last, cp) > 0)
    //          output.push_back(cp);
    //      else {
    //          throw std::runtime_error("conversion failed");
    //      }
    //  }
    //  
    
    // A.1a
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point)
    template <typename IteratorT, typename EncodingT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint(IteratorT& first, IteratorT last, 
                         EncodingT encoding, 
                         code_point_t& code_point,
                         typename boost::enable_if<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT>
                         >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter<IteratorT, EncodingT> Converter;        
        return Converter().to_codepoint(first, last, code_point);        
    }
    
    
    // A.1b
    //  int 
    //  convert_to_codepoint(Iterator& first, Encoding encoding,
    //              code_point_t& code_point)
    template <typename IteratorT, typename EncodingT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint(IteratorT& first, 
                         EncodingT encoding, 
                         code_point_t& code_point,
                         typename boost::enable_if<
                            boost::is_base_and_derived<utf_encoding_tag, EncodingT>
                         >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter<IteratorT, EncodingT> Converter;        
        return Converter().to_codepoint(first, code_point);        
    }
    
    
    // A.2
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint(IteratorT& first, IteratorT last, EncodingT encoding, 
                         code_point_t& code_point, FilterT filter,
                         typename boost::enable_if<
                         boost::mpl::and_<
                         boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                         boost::is_base_and_derived<filter::filter_tag, FilterT>,
                         boost::mpl::not_<boost::is_same<filter::None, FilterT> > 
                         >                
                         >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter<IteratorT, EncodingT> Converter;        
        int result = Converter().to_codepoint(first, last, code_point);
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }
    
    // A.2a
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, Encoding encoding,
    //              code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint(IteratorT& first, IteratorT last, EncodingT encoding, 
                                    code_point_t& code_point, FilterT filter,
                                    typename boost::enable_if<
                                    boost::mpl::and_<
                                    boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                                    boost::is_same<filter::None, FilterT>
                                    >                
                                    >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter<IteratorT, EncodingT> Converter;        
        return Converter().to_codepoint(first, last, code_point);        
    }
    
    
    // A.3
    //  int 
    //  convert_to_codepoint(Iterator& first, Iterator last, code_point_t& code_point)
    template <typename IteratorT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint(IteratorT& first, IteratorT last, code_point_t& code_point) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter<IteratorT, Encoding> Converter;        
        return Converter().to_codepoint(first, last, code_point);        
    }
    
    
    // A.4
    //  int 
    //  convert_one(Iterator& first, Iterator last, code_point_t& code_point,
    //              Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint(IteratorT& first, IteratorT last, code_point_t& code_point, 
                                    FilterT filter,
                                    typename boost::enable_if<
                                    boost::mpl::and_<
                                    boost::is_base_and_derived<filter::filter_tag, FilterT>,
                                    boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                                    >
                                    >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter<IteratorT, Encoding> Converter;        
        int result = Converter().to_codepoint(first, last, code_point); 
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }
    
    // A.4a
    //  int 
    //  convert_one(Iterator& first, Iterator last, code_point_t& code_point,
    //              Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint(IteratorT& first, IteratorT last, code_point_t& code_point, 
                                    FilterT filter,
                                    typename boost::enable_if<
                                    boost::is_same<filter::None, FilterT>
                                    >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter<IteratorT, Encoding> Converter;        
        return Converter().to_codepoint(first, last, code_point); 
    }
    
    
    // B.1
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point)
    template <typename IteratorT, typename EncodingT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint_unsafe(IteratorT& first, IteratorT last, EncodingT encoding, 
                                code_point_t& code_point,
                                typename boost::enable_if<
                                boost::is_base_and_derived<utf_encoding_tag, EncodingT>
                                >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter_unsafe<IteratorT, EncodingT> Converter;        
        return Converter().to_codepoint(first, code_point);        
    }
    
    
    // B.2
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint_unsafe(IteratorT& first, IteratorT last, EncodingT encoding, 
                                code_point_t& code_point, FilterT filter,
                                typename boost::enable_if<
                                boost::mpl::and_<
                                boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                                boost::is_base_and_derived<filter::filter_tag, FilterT>,
                                boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                                >                
                                >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter_unsafe<IteratorT, EncodingT> Converter;        
        int result = Converter().to_codepoint(first, code_point);        
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }
    
    // B.2a
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, Encoding encoding,
    //                     code_point_t& code_point, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint_unsafe(IteratorT& first, IteratorT last, EncodingT encoding, 
                                code_point_t& code_point, FilterT filter,
                                typename boost::enable_if<
                                boost::mpl::and_<
                                boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                                boost::is_same<filter::None, FilterT>
                                >                
                                >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter_unsafe<IteratorT, EncodingT> Converter;        
        return Converter().to_codepoint(first, code_point);        
    }
    
    
    
    // B.3
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point)
    template <typename IteratorT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint_unsafe(IteratorT& first, IteratorT last, code_point_t& code_point) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter_unsafe<IteratorT, Encoding> Converter;        
        return Converter().to_codepoint(first, code_point);        
    }
    
    
    // B.4
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point,
    //                     Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint_unsafe(IteratorT& first, IteratorT last, 
                                           code_point_t& code_point, FilterT filter,
                                           typename boost::enable_if<
                                           boost::mpl::and_<
                                           boost::is_base_and_derived<filter::filter_tag, FilterT>,
                                           boost::mpl::not_<boost::is_same<filter::None, FilterT> > 
                                           >
                                           >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter_unsafe<IteratorT, Encoding> Converter;        
        int result = Converter().to_codepoint(first, code_point); 
        if (result > 0) {
            if (filter(code_point)) {
                if (filter.replace()) {
                    code_point = filter.replacement(code_point);
                }
                else {
                    return E_PREDICATE_FAILED;
                }
            }
        }
        return result;
    }
    
    // B.4a
    //  int 
    //  convert_one_unsafe(Iterator& first, Iterator last, code_point_t& code_point,
    //                     Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_to_codepoint_unsafe(IteratorT& first, IteratorT last, 
                                           code_point_t& code_point, FilterT filter,
                                           typename boost::enable_if<
                                           boost::is_same<filter::None, FilterT>
                                           >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter_unsafe<IteratorT, Encoding> Converter;        
        return Converter().to_codepoint(first, code_point); 
    }
    
    
}}        



#pragma mark - Unicode Code Point To UTF
namespace json { namespace unicode {
    
    //  Unicode Code Point to UTF Conversions
    //
    //  Synopsis:
    //
    // A.1
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Encoding encoding)
    // 
    // A.2
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    //
    // A.3
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest)
    //
    // A.4
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Filter filter)
    //
    //
    // B.1
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding)
    //
    // B.2
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    //
    // B.3
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest)
    //
    // B.4
    //  int 
    //  convert_one_unsafe(code_point_t code_point, Iterator& dest, Filter filter)
    //
    //
    // Converts an Unicode code unit to a corresponding sequence of Unicode code 
    // units using Encoding and stores the result into the sequence starting
    // at iterator dest.
    // 
    // Returns the number of generated code units in encoding or a negative
    // integer indicating an error.
    //
    //
    //
    // The allowed encodings forms of the output sequence are UTF-32, UTF-16
    // and UTF-8, with their respective endianness variants.
    //
    // The functions take a parameter code_point which shall be converted, and
    // an iterator specifying the sequence of code units where the result should
    // be copied. 
    //
    // The encoding can be explicitly specified. If it is omitted, an appropria-
    // te encoding type from the iterator's value_type will be generated. This 
    // encoding type is always in host endianness.
    //
    // Optionally, a Filter operator can be specified. If the filter evaluates
    // true for a certain code point, it will be checked if it shall be replaced.
    // If this is true, the code point will be replaced by the replacement. If
    // this is false, an error E_PREDICATE_FAILED will be returned.
    // Otherwise, the code point will be accepted as is.
    // 
    // 
    // The safe version will detect an invalid Unicode code point, and will 
    // return an error in this case. The unsafe versions will not detect invalid
    // code points and in this case the result of the function is undefined.
    //
    // Both versions (safe and unsafe) do not check for a range error of the
    // output sequence. The caller is responsible that the output sequence can
    // hold the result, or otherwise will throw an exception.
    //
    // When the function returns successfully, the number of generated code 
    // units will be returned, and iterator 'dest' has been advanved by the 
    // number of code units respectively.
    //
    // When the functions fails, dest is unchanged and a negative integer will
    // be returned indicating the error.
    //
    // The unsafe version assumes a valid Unicode scalar value, otherwise (if 
    // the input is not valid) the behavior is undefined.
    //
    // The algorithm can be used with any iterator type.
    // The EncodingT sepcifies the Iterator's encoding.
    //
    //
    //  Examples:
    //  
    //  1)  Convert an Unicode code point to UTF-8 and store the result in a 
    //      vector.
    //   
    //  std::vector<unit8_t>  output;
    //  code_point_t cp = ...;
    //  int result = convert_one(cp, std::back_inserter(output));
    //  
    
    // A.1
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Encoding encoding)
    template <typename IteratorT, typename EncodingT> 
    inline int
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint(code_point_t code_point, IteratorT& dest, EncodingT encoding,
                           typename boost::enable_if<
                           boost::is_base_and_derived<utf_encoding_tag, EncodingT> 
                           >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter<IteratorT, EncodingT> Converter;        
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // A.2
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                           FilterT filter,
                           typename boost::enable_if<
                           boost::mpl::and_<
                           boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                           boost::is_base_and_derived<filter::filter_tag, FilterT>,
                           boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                           >                
                           >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter<IteratorT, EncodingT> Converter;        
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter::from_codepoint(code_point, dest);        
    }
    
    // A.2a
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                           FilterT filter,
                           typename boost::enable_if<
                           boost::mpl::and_<
                           boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                           boost::is_same<filter::None, FilterT>
                           >                
                           >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter<IteratorT, EncodingT> Converter;        
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    
    
    // A.3
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest)
    template <typename IteratorT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint(code_point_t code_point, IteratorT& dest) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter<IteratorT, Encoding> Converter;        
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // A.4
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint(code_point_t code_point, IteratorT& dest, FilterT filter,
                           typename boost::enable_if<
                           boost::mpl::and_<
                           boost::is_base_and_derived<filter::filter_tag, FilterT>,
                           boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                           >
                           >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter<IteratorT, Encoding> Converter;        
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // A.4a
    //  int 
    //  convert_from_codepoint(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint(code_point_t code_point, IteratorT& dest, FilterT filter,
                           typename boost::enable_if<
                           boost::is_same<filter::None, FilterT>
                           >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter<IteratorT, Encoding> Converter;        
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // B.1
    template <typename IteratorT, typename EncodingT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint_unsafe(code_point_t code_point, IteratorT& dest, EncodingT encoding,
                                  typename boost::enable_if<
                                  boost::is_base_and_derived<utf_encoding_tag, EncodingT> 
                                  >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter_unsafe<IteratorT, EncodingT> Converter;        
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // B.2
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint_unsafe(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                                  FilterT filter,
                                  typename boost::enable_if<
                                  boost::mpl::and_<
                                  boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                                  boost::is_base_and_derived<filter::filter_tag, FilterT>,
                                  boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                                  >
                                  >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter_unsafe<IteratorT, EncodingT> Converter; 
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // B.2a
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest, Encoding encoding, Filter filter)
    template <typename IteratorT, typename EncodingT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint_unsafe(code_point_t code_point, IteratorT& dest, EncodingT encoding, 
                                  FilterT filter,
                                  typename boost::enable_if<
                                  boost::mpl::and_<
                                  boost::is_base_and_derived<utf_encoding_tag, EncodingT>,
                                  boost::is_same<filter::None, FilterT>
                                  >
                                  >::type* dummy = 0) 
    {
        typedef internal::codepoint_converter_unsafe<IteratorT, EncodingT> Converter; 
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // B.3
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest)
    template <typename IteratorT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint_unsafe(code_point_t code_point, IteratorT& dest) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter_unsafe<IteratorT, Encoding> Converter;        
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // B.4
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint_unsafe(code_point_t code_point, IteratorT& dest, FilterT filter,
                                  typename boost::enable_if< 
                                  boost::mpl::and_<
                                  boost::is_base_and_derived<filter::filter_tag, FilterT>,
                                  boost::mpl::not_<boost::is_same<filter::None, FilterT> >
                                  >
                                  >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter_unsafe<IteratorT, Encoding> Converter;   
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    // B.4a
    //  int 
    //  convert_from_codepoint_unsafe(code_point_t code_point, Iterator& dest, Filter filter)
    template <typename IteratorT, typename FilterT> 
    inline int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif            
    convert_from_codepoint_unsafe(code_point_t code_point, IteratorT& dest, FilterT filter,
                                  typename boost::enable_if<
                                  boost::is_same<filter::None, FilterT>
                                  >::type* dummy = 0) 
    {
        typedef typename iterator_encoding<IteratorT>::type Encoding;
        typedef internal::codepoint_converter_unsafe<IteratorT, Encoding> Converter;   
        if (filter(code_point)) {
            if (filter.replace()) {
                code_point = filter.replacement(code_point);
            }
            else {
                return E_PREDICATE_FAILED;
            }
        }
        return Converter::from_codepoint(code_point, dest);        
    }
    
    
    
    
}}  // namespace json::unicode




#pragma mark - Unicode Code Point Sequence to UTF Sequence
namespace json { namespace unicode {
  
    
    
    // Convert the Unicode code points contained in the sequence [first, last) 
    // to a sequence of code units encoded in outEncoding and copy the result 
    // to the sequence starting at dest.
    //
    // The output sequence shall be large enough to hold the generated code 
    // points.
    //
    // On exit, if the function was successful, parameter first will be 
    // advanced until last, and dest will be advanced as much as needed to 
    // store the generated code points. Otherwise, if the the function failed, 
    // both iterators will be advanced to the point where the error occured.
    //
    // If the function fails, parameter error will be set accordingly to an 
    // negative number indicating the error - otherwise, it will be set to 0.
    //
    // Returns the number of generated code units. If the function fails, it 
    // returns zero, however the value "0" may also be a valid number of 
    // generated code units IFF the input string is empty. The result state of 
    // the conversion is always indicated by paramerer error, though.
    //
    // Errors:
    //  E_INVALID_CODE_POINT
    //      
    //
    template <
    typename InIteratorT, 
    typename OutIteratorT, typename OutEncodingT
    >
    inline std::size_t
    convert_from_codepoints(
                       InIteratorT&     first, 
                       InIteratorT      last, 
                       OutIteratorT&    dest,
                       OutEncodingT     outEncoding,
                       int&             error)
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(code_point_t));
        
        std::size_t count = 0;
        while (first != last) {
            int result = convert_from_codepoint(static_cast<code_point_t>(*first),
                                                dest, outEncoding);
            if (result > 0) {
                count += result;
                ++first;
            }
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    
    template <
    typename InIteratorT,
    typename OutIteratorT, typename OutEncodingT,
    typename FilterT
    >
    inline std::size_t
    convert_from_codepoints(
                       InIteratorT&     first, 
                       InIteratorT      last, 
                       OutIteratorT&    dest,
                       OutEncodingT     outEncoding,
                       FilterT          filter,
                       int&             error)
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(code_point_t));
        
        std::size_t count = 0;
        while (first != last) {
            int result = convert_from_codepoint(static_cast<code_point_t>(*first),
                                                dest, outEncoding, filter);
            if (result > 0) {
                count += result;
                ++first;
            }
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    
    
    template <
    typename InIteratorT, 
    typename OutIteratorT, typename OutEncodingT
    >
    inline std::size_t
    convert_from_codepoints_unsafe(
                              InIteratorT&     first, 
                              InIteratorT      last, 
                              OutIteratorT&    dest,
                              OutEncodingT     outEncoding,
                              int&             error)
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(code_point_t));
        
        
        std::size_t count = 0;
        while (first != last) {
            int result = convert_from_codepoint_unsafe(static_cast<code_point_t>(*first),
                                                       dest, outEncoding);
            if (result > 0) {
                count += result;
                ++first;
            }
            else {
                error = result;
                return 0;
                break;
            }
        }
        error = 0;
        return count;
    }    
    

    
}}


#endif

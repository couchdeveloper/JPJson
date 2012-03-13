//
//  utf_cvt.hpp
//  
//
//  Created by Andreas Grosam on 1/31/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef _utf_cvt_hpp
#define _utf_cvt_hpp

#error This file is deprecated!


#include "json/config.hpp"
#include "unicode_utilities.hpp"
#include "unicode_errors.hpp"
#include "unicode_filter.hpp"
#include "codepoint_cvt.hpp"

#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>




// If not defined, it uses specializations for UTF-8 encoding forms (should be left 
// undefined). If defined, a generic implementation will be used. Ideally, 
// performacne should be almost as fast - but in practice is suffers a lot. The
// performance penalty can be alleviated somewhat by enabling LTO.
//#define NO_USE_SPECIALIZATION_UTF8

// Trade off minor optimization vs code size 
//#define NO_USE_MINOR_SPEED_OPTIMZATION


// For test only  
// If defined, code for UTF-8 quad bytes is not compiled in.
// clang's optimized code suffers greatly if code becomes more complex. This switch
// illustrates this. If it is defined, code quality is comparable to GCC 4.6.2.
// Otherwise (the required case), it becomes quite bad.
#define NO_QUAD


// For testing compiler's optimization abilities. (should be left undefined)
//#define FORCE_INLINE_OUTER
//#define FORCE_INLINE_INNER


// If defined is disables the capabilities of the converter to handle partial
// multi-byte sequences and keeping state information. This reduces code
// complexity.
#define NO_PARTIAL_MULTIBYTE


//#define JP_EXPECT(cond)  cond
#define JP_EXPECT(cond)  __builtin_expect(static_cast<long>(cond), 1)


//
//  Generic Converters
//
namespace json { namespace unicode { 
    
#pragma mark - Class internal::converter UTF to UTF
    
    
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        class Enable = void
    >
    struct converter 
    {        
    private:
        typedef internal::codepoint_converter<InIteratorT, InEncodingT> from_cvt_t;
        typedef internal::codepoint_converter_unsafe<OutIteratorT, OutEncodingT> to_cvt_t;
        
        from_cvt_t  from_cvt_;
        
    public:
        
        converter() {}
        
        void clear_state() { from_cvt_.clear_state(); }
        
        // Converts and thereby validates an Unicode sequence in 'InEncodingT' 
        // encoding form to 'OutEncodingT' encoding form and copies it into 
        // output iterator 'dest'.
        //
        // The class instance keeps state information for partial multi-byte 
        // sequences. That is, member function convert() can be called multiple
        // times which is required for input that will be received in multiple 
        // chunks, where the partial sequences can be devided at an arbitrary 
        // code unit.
        //
        // Returns zero if the input was well-formed and complete, otherwise if 
        // the sequence was so far well-formed but contains a partial multi-byte 
        // sequence at the end of the sequence, it returns E_UNEXPECTED_ENDOFINPUT. 
        // Otherwise it returns a negative integer indicating the kind of error.
        // 
        // InOut parameter 'first' will be advanced as long as the sequence can 
        // be unambiguously interpreted as a sequence of particular Unicode code 
        // points and 'first' does not equal 'last', or where the converter 
        // recognizes that code units collected so far constitute an ill-formed 
        // subsequence. 
        //
        // In case of an error, a client would write the replacement character 
        // U+FFFD into dest. In order to continue with the conversion, the client
        // must advance 'first' by one IF and only if it points to an *illegal 
        // start byte* that is, the return value was E_INVALID_START_BYTE, and 
        // then continue with the conversion.
        int 
#if defined (FORCE_INLINE_OUTER)        
        __attribute__((always_inline))  // TODO: This shall not be forced to be inlined!
#endif        
        convert(InIteratorT& first, InIteratorT& last, OutIteratorT& dest)
        {
            int result = 0;
            while (JP_EXPECT(first != last)) {
                code_point_t cp;
                result = from_cvt_.to_codepoint(first, last, cp);
                if (JP_EXPECT(result > 0)) {
                    result = to_cvt_t::from_codepoint(cp, dest);
                }
                else {
                    break;
                }
            }
            return result;
        }
        
        
    };
    
    
    
    // Converts a well-formed Unicode sequence in 'InEncodingT' encoding form 
    // to 'OutEncodingT' encoding form and copies it into output iterator 'dest'.
    //
    // The class instance keeps state information for partial multi-byte 
    // sequences. That is, member function convert() can be called multiple
    // times if required for partial input.
    //
    // Returns zero on success or E_UNEXPECTED_ENDOFINPUT. Since the function 
    // assumes a well-formed Unicode sequence the result is undefined if the 
    // input is ill-formed.
    // 
    template <
        typename InIteratorT, typename InEncodingT, 
        typename OutIteratorT, typename OutEncodingT, 
        class Enable = void
    >
    struct converter_unsafe 
    {   
    private:
        typedef internal::codepoint_converter_unsafe<InIteratorT, InEncodingT> from_cvt_t;
        typedef internal::codepoint_converter_unsafe<OutIteratorT, OutEncodingT> to_cvt_t;
        from_cvt_t  from_cvt_;

    public:
        converter_unsafe() {}
        
        void clear_state() { from_cvt_.clear_state(); }
        
        int
#if defined (FORCE_INLINE_OUTER)        
        __attribute__((always_inline))  // TODO: This shall not be forced to be inlined!
#endif        
        convert(InIteratorT& first, InIteratorT last, OutIteratorT& dest)
        {
            int result = 0;
            while (JP_EXPECT(first != last)) {
                code_point_t cp;
                result = from_cvt_.to_codepoint(first, last, cp);
                if (JP_EXPECT(result > 0)) {
                    result = to_cvt_t::from_codepoint(cp, dest);
                }
                else {
                    break;
                }
            }
            return result;
        }
        
        
    };    
}}



//
//   Specialization for UTF-8 to UTF-8
//
#if !defined (NO_USE_SPECIALIZATION_UTF8)    
namespace json { namespace unicode {
    
    
    namespace internal {
        
        //  This code is for testing only. 
        //  The function convert() is "hand written" such that the
        //  compiler's generated code should be comparable with the code generated
        //  from sources which use the "generic basic converter" classes which use
        //  an Unicode code point as pivot.
        //
        
        // The boolean template parameter 'NoCheckLast' and 'NoCheckTrails'
        // determine to amount of checks performed in member function convert().
        template <bool NoCheckLast, bool NoCheckTrails>
        class converter_utf8_utf8
        {
        public:            
            typedef uint8_t buffer_type[4];
            
        private:            
            enum State {
                State_0 = 0,
                State_1, 
                State_21,
                State_22,
                State_31,
                State_32
            };
            
            buffer_type buffer_;
            int state_;
            
        public:
            converter_utf8_utf8() : state_(0) {}
            
            void clear_state() { state_ = 0; }
            const buffer_type& buffer() const { return buffer_; }
            
            
            // Validates an Unicode sequence in UTF-8 encoding form and copies it to
            // the output iterator 'dest' (which may be a no-op output iterator).
            // It is supposed the compiler can apply
            // optimizations which effectively removes unneccesary validation code.
            // (possibly LTO can achieve the same effect with normal function parameters).
            
            
            // Validate a sequence of UTF-8 characters and copy it to output 
            // iterator 'dest'.
            //
            // Returns zero if the input sequence was well-formed and complete.
            // Otherwise, it returns E_UNEXPECTED_ENDOFINPUT if the input sequence
            // was well-formed but contains a partial sequence of a multi-byte 
            // character at the end of the sequence. Otherwise it returns a 
            // negative value indicating an ill-formed input sequence.
            //
            // The function is capable to validate and copy a partial multi-byte 
            // sequence. The state information is kept in the instance. In any
            // case, the start of a possibly ill-formed Unicode sequence shall
            // be performed with an initial state. The state is initialized 
            // (cleared) after construction and after calling member function 
            // clear_state().
            //
            // If the state is clear, parameter 'first' should point to a start 
            // byte or a single byte, otherwise the sequence is treated ill-formed.
            // If the function has successfully validated a well-formed and 
            // complete input sequence, the internal state is again clear. 
            // If the input was so far valid but ended at a partial multi-byte 
            // sequence, the function returns E_UNEXPECTED_ENDOFINPUT and sets 
            // its state accordingly. The member function convert() can
            // subsequently be called with the next sequence continuing at the 
            // end of the former sequence at any code unit boundary.
            // 
            // In any case, the return value of the function indicates the result 
            // of the conversion. When the validation fails due to an ill-formed 
            // sequence, the converter shall not be reused unless the state has
            // been cleared via calling member function clear_state(), otherwise
            // the result is undefined.
            // 
            template <typename InIteratorT, typename OutIteratorT>
            int
#if defined (FORCE_INLINE_INNER)        
            __attribute__((always_inline))   
            // Note:
            // This function contains a while loop. It is supposed the CPU cycles
            // for setting up the while loop is negligible compared to the time
            // spent in the while loop. Thus, the function is not a good candidate
            // to be inlined.
            // 
#endif                    
            convert(InIteratorT& first, InIteratorT& last, OutIteratorT& dest)
            {
                BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                                    == sizeof(typename UTF_8_encoding_tag::code_unit_type));
                
                assert(state_ >= 0);
                
                int result;
#if !defined (NO_PARTIAL_MULTIBYTE)                
                switch (state_) {
                    case State_1: goto state_1;
                    case State_21: goto state_21;
                    case State_22: goto state_22;
#if !defined (NO_QUAD)
                    case State_31: goto state_31;
                    case State_32: goto state_32;
#endif                        
                }
#endif                
                while (JP_EXPECT(first != last)) {
                    buffer_[0]= static_cast<uint8_t>(*first);
                    if (utf8_is_single(buffer_[0])) {
                        assert(state_ == State_0);
                        // single byte
                        ++first;
                        *dest++ = buffer_[0];
                        continue;
                    }
                    if (buffer_[0] < 0xE0u) {
                        if (buffer_[0] >= 0xC2u) {
                            // two bytes
                            ++first;
                            if (JP_EXPECT(NoCheckLast or first != last)) {
                            state_1:
                                buffer_[1] = static_cast<uint8_t>(*first);            
                                if (JP_EXPECT(NoCheckTrails or utf8_is_trail(buffer_[1]))) {
                                    ++first;
                                    state_ = State_0;
                                    *dest++ = buffer_[0];
                                    *dest++ = buffer_[1];
                                    continue;
                                }  else { state_ = result = E_TRAIL_EXPECTED; } 
                            } else { state_ = State_1; result = E_UNEXPECTED_ENDOFINPUT; }
                        } else { state_ = result = E_INVALID_START_BYTE; }
                    } 
                    else {
                        // three and four bytes
                        ++first;
                        if (JP_EXPECT(NoCheckLast or first != last)) {
                        state_21:
                            buffer_[1] = *first;            
                            if (buffer_[0] < 0xF0u) { // three bytes
                                if (JP_EXPECT(NoCheckTrails
                                              or ((buffer_[0] == 0xE0u) and ((buffer_[1] - 0xA0u) <= (0xBFu-0xA0u)))
                                              or ((buffer_[0] == 0xEDu) and ((buffer_[1] - 0x80u) <= (0x9Fu-0x80u)))
                                              or utf8_is_trail(buffer_[1])))
                                {
                                    ++first;
                                    if (JP_EXPECT(NoCheckLast or first != last)) {
                                    state_22:
                                        buffer_[2] = *first;
                                        if (JP_EXPECT(NoCheckTrails or utf8_is_trail(buffer_[2]))) {
                                            ++first;
                                            state_ = State_0;
                                            *dest++ = buffer_[0];
                                            *dest++ = buffer_[1];
                                            *dest++ = buffer_[2];    
                                            continue;
                                        } else { state_ = result = E_TRAIL_EXPECTED;}
                                    } else { state_ = State_22; result = E_UNEXPECTED_ENDOFINPUT;} 
                                } else { state_ = result = E_TRAIL_EXPECTED; /* or E_UNCONVERTABLE_OFFSET */ }
                            }
#if !defined (NO_QUAD)   // four bytes    
                            else { 
                                if (buffer_[0] <= 0xF5u) { // four bytes
                                    if (JP_EXPECT(NoCheckTrails
                                                  or ((buffer_[0] == 0xF0u) and ((buffer_[1] - 0x90u) <= (0xBFu-0x90u)))
                                                  or ((buffer_[0] == 0xF4u) and ((buffer_[1] - 0x80u) <= (0x8Fu-0x80u)))
                                                  or (utf8_is_trail(buffer_[1]))))
                                    {
                                        ++first;
                                        if (JP_EXPECT(NoCheckLast or first != last)) {
                                        state_31:
                                            buffer_[2] = *first;
                                            if (JP_EXPECT(NoCheckTrails or utf8_is_trail(buffer_[2]))) {
                                                ++first;
                                                if (JP_EXPECT(NoCheckLast or first != last)) {
                                                state_32:
                                                    buffer_[3] = *first;
                                                    if (JP_EXPECT(NoCheckTrails or utf8_is_trail(buffer_[3]))) {
                                                        ++first;
                                                        state_ = State_0;
                                                        *dest++ = buffer_[0];
                                                        *dest++ = buffer_[1];
                                                        *dest++ = buffer_[2];            
                                                        *dest++ = buffer_[3];
                                                        continue;
                                                    } else { state_ = result = E_TRAIL_EXPECTED; }
                                                } else { state_ = State_32; result = E_UNEXPECTED_ENDOFINPUT;} 
                                            } else { state_ = result = E_TRAIL_EXPECTED; }
                                        } else { state_ = State_31; result = E_UNEXPECTED_ENDOFINPUT; } 
                                    } else { state_ = result = E_TRAIL_EXPECTED; /* E_UNCONVERTABLE_OFFSET */ }
                                } else { state_ = result = E_INVALID_START_BYTE; }
                            }
#endif // four bytes                    
                        } else { state_ = State_21; result = E_UNEXPECTED_ENDOFINPUT; } 
                    }
                    return result; // if we reach here it indicates and error during the conversion.
                    
                } // while
                
                assert(state_ == State_0);
                return 0;  // if we reach here, we had success or the sequence was empty
            }
            
            
        };  // class converter_utf8_utf8
        
        
    } // namespace internal
    
    
    
    
    //
    //  Specialization for UTF-8 encoding form and for input iterators which are
    //  not random access iterators.
    //
    template <typename InIteratorT, typename OutIteratorT>
    struct converter<InIteratorT, UTF_8_encoding_tag, OutIteratorT, UTF_8_encoding_tag, 
        typename boost::disable_if<
            boost::is_same<
                typename std::iterator_traits<InIteratorT>::iterator_category, 
                std::random_access_iterator_tag
            >    
        >::type
    >
    {
    private:
        typedef typename internal::converter_utf8_utf8<false, false> cvt_type;
        cvt_type cvt_;
    public:
        int 
#if defined (FORCE_INLINE_OUTER)        
        __attribute__((always_inline))
#endif        
        convert(InIteratorT& first, InIteratorT& last, OutIteratorT& dest) {
            return cvt_.convert(first, last, dest);
        }
    };
    
    
    //
    //  Specialization for UTF-8 encoding form and random access input iterators
    //
    template <typename InIteratorT, typename OutIteratorT>
    struct converter<InIteratorT, UTF_8_encoding_tag, OutIteratorT, UTF_8_encoding_tag, 
        typename boost::enable_if<
            boost::is_same<
                typename std::iterator_traits<InIteratorT>::iterator_category, 
                std::random_access_iterator_tag
            >    
        >::type
    >
    {        
    public:
        int 
#if defined (FORCE_INLINE_OUTER)        
        __attribute__((always_inline))
#endif        
        convert(InIteratorT& first, InIteratorT& last, OutIteratorT& dest) 
        {
#if !defined (NO_USE_MINOR_SPEED_OPTIMZATION)  // speed vs code size
            // If we have random access input iterators, we can apply some
            // minor optimizations (don't check for first != last):
            const size_t treshhold_lengt = 24; // should be reasonable size which justifies the additional costs for selecting the optimization.
            int result;
            internal::converter_utf8_utf8<true, false> cvt;
            size_t length = std::distance(first, last);
            if (length > treshhold_lengt) {
                InIteratorT last1 = last - 3;
                while (last1 != first and utf8_is_trail(*last1))
                    --last1;
                result = cvt.convert(first, last1, dest);
                if (result < 0)
                    return result;
            }
#endif          
            internal::converter_utf8_utf8<false, false> cvt2;
            return cvt2.convert(first, last, dest);
        }
    };
    
    
    
    
    //
    //  Specialization for unsafe UTF-8 to UTF-8 conversion. This is merely a 
    //  somewhat safer copy since it checks for valid start bytes and valid 
    //  number of trail bytes.
    //  An even more lenient conversion from UTF-8 to UTF-8 would just copy
    //  the sequence.
    //
    template <typename InIteratorT, typename OutIteratorT>
    struct converter_unsafe<InIteratorT, UTF_8_encoding_tag, OutIteratorT, UTF_8_encoding_tag, void> 
    {        
    public:
        int 
#if defined (FORCE_INLINE_OUTER)        
        __attribute__((always_inline))
#endif        
        convert(InIteratorT& first, InIteratorT& last, OutIteratorT& dest) {
            internal::converter_utf8_utf8<true, true> cvt;
            return cvt.convert(first, last, dest);
        }
    };
        

}}
#endif    // USE_SPECIALIZATION_UTF8



//
//  Convert One
//
#if 0
namespace json { namespace unicode { 
    
#pragma mark - Function convert_one  UTF to UTF    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT
    >
    int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif                    
    convert_one(InIteratorT& first, InIteratorT last, InEncodingT, 
                OutIteratorT dest, OutEncodingT)
    {
        typedef internal::converter<InIteratorT, InEncodingT, OutIteratorT, OutEncodingT> cvt;
        return cvt::copy_one(first, last, InEncodingT(), dest, OutEncodingT());
    }
    
    
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT
    >
    int 
#if !defined(DEBUG)        
    __attribute__((always_inline))
#endif                    
    convert_one_unsafe(InIteratorT& first, InIteratorT last, InEncodingT, 
                       OutIteratorT dest, OutEncodingT)
    {
        typedef internal::converter<InIteratorT, InEncodingT, OutIteratorT, OutEncodingT> cvt;
        return cvt::copy_one_unsafe(first, last, InEncodingT(), dest, OutEncodingT());
    }
    
    
}}
#endif




//  ----------------------------------------------------------------------------
//
//                                 Specializations
//
//  ----------------------------------------------------------------------------

// Template Specializations for Input Encoding equals Output Encoding
// and Filter is None.
// All "safe" versions check for wellformed Unicode and and check input
// iterator.
//
#if 0
namespace json { namespace unicode { namespace internal { 
    
    
#if 0 // enable/disable Specialization for UTF-8 to UTF-8
#pragma mark - Class internal::converter UTF-8 to UTF-8   Specialization
    //
    // UTF-8  to  UTF-8
    //
    template <
        typename InIteratorT, typename OutIteratorT, typename EncodingT
    >
    class converter<
        InIteratorT, EncodingT, 
        OutIteratorT, EncodingT, 
        typename boost::enable_if<
            boost::is_same<UTF_8_encoding_tag, EncodingT>
        >::type
    > 
    {
        typedef typename UTF_8_encoding_tag::encode_buffer_type encode_buffer_type;
        typedef typename UTF_8_encoding_tag::code_unit_type code_unit;
                
    public:

        // Convert the first character contained in the possibly mal-formed UTF-8 
        // sequence starting at first to output . Parameter first 
        // should point to the start of a UTF-8 multi byte sequence.
        // On exit first has been advanced by the number of bytes constituting the
        // character, or in case of an error, to the point where the error occured.
        // Returns the number of generated code points [1, 0], or a negative number
        // indicating an error.
        //
        // Does not fail if the resulting code point is an Unicode noncharacter.
        // 
        // Errors:
        //      E_TRAIL_EXPECTED:           trail byte expected
        //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
        //      E_INVALID_START_BYTE:       invalid start byte
        //      E_INVALID_CODE_POINT:       detected surrogate or code point is invalid Unicode 
        //     
        // Implementation notes:
        // The algorithm must work with either signed or usigned UTF-8 code units.
        //
        // According the "Constraints on Conversion process" in the Unicode
        // Standard v6, this algorithm conforms to the concept of "maximal sub-
        // part".
        static int 
#if !defined(DEBUG)        
        __attribute__((always_inline))
#endif            
        copy_one(InIteratorT& first, InIteratorT last, EncodingT, 
                 OutIteratorT& dest, EncodingT)
        {            
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));
            
            assert(first != last);
            
            const int num_trails = utf8_num_trails(static_cast<utf8_code_unit>(*first));
            assert(num_trails <= 3 and num_trails >= 0);            
            if (num_trails == 0) {
                *dest++ = *first++;
                return 1;
            } 
#if 0            
            if (num_trails == 1) {
                ++first;
                if (first == last) {
                    return E_UNEXPECTED_ENDOFINPUT;
                }            
                uint8_t ch1 = *first;
                if (not utf8_is_trail(ch1)) {
                    return E_TRAIL_EXPECTED;
                }            
                *dest++ = ch1;
                *dest++ = *first++;
                return 1;
            } 
#endif            
            
#if 1
            typedef codepoint_converter<InIteratorT, EncodingT> from_cvt_t;
            typedef codepoint_converter_unsafe<OutIteratorT, EncodingT> to_cvt_t;
            
            code_point_t cp;
            from_cvt_t from_cvt;
            int result = from_cvt.to_codepoint(first, last, cp);
            if (result > 0) {
                return to_cvt_t::from_codepoint(cp, dest);
            }
            return result;
#else            
            uint8_t ch[4];
            ch[0] = static_cast<uint8_t>(*first++);
            if (num_trails == 1) {
                if (first == last) {
                    return E_UNEXPECTED_ENDOFINPUT;
                }            
                ch[1] = *first;
                if (not utf8_is_trail(ch[1])) {
                    return E_TRAIL_EXPECTED;
                }            
                ++first;
                *dest++ = ch[0];
                *dest++ = ch[1];
                return 1;
            } 
            if (num_trails >= 2)
            {
                if (first == last) {
                    return E_UNEXPECTED_ENDOFINPUT;
                }  
                ch[1]= *first;
                switch (ch[0]) {
                    default:
                        if (not utf8_is_trail(ch[1])) return E_TRAIL_EXPECTED;
                        break;
                    case 0xE0u:  // A0..BF      2 trail bytes
                        if ((ch[1] - 0xA0u) > (0xBFu-0xA0u)) return E_TRAIL_EXPECTED; // or E_UNCONVERTABLE_OFFSET;
                        break;
                    case 0xEDu: // 80..9F       2 trail bytes
                        if ((ch[1] - 0x80u) > (0x9Fu-0x80u)) return E_TRAIL_EXPECTED; // or E_UNCONVERTABLE_OFFSET;
                        break;
                    case 0xF0u: // 90..BF       3 trail bytes
                        if ((ch[1] - 0x90u) > (0xBFu-0x90u)) return E_TRAIL_EXPECTED; // or E_UNCONVERTABLE_OFFSET;
                        break;
                    case 0xF4:  // 80..8F       3 trail bytes
                        if ((ch[1] - 0x80u) > (0x8Fu-0x80u)) return E_TRAIL_EXPECTED; // or E_UNCONVERTABLE_OFFSET;
                        break;
                }
                // get ch[2], and ch[3] if required
                for (int i = 2; i <= num_trails; ++i) {
                    ++first;
                    if (first == last) {
                        return E_UNEXPECTED_ENDOFINPUT;
                    }
                    ch[i] = *first;
                    if (not utf8_is_trail(ch[i])) {
                        return E_TRAIL_EXPECTED;
                    }
                }
                ++first;
                uint8_t* p = ch;
                switch (num_trails) {
                    case 3: *dest++ = *p++;
                    case 2: *dest++ = *p++;
                    case 1: *dest++ = *p++; 
                            *dest++ = *p;
                }
                return num_trails + 1;
            } 
            else {
                return E_INVALID_START_BYTE;
            }
#endif            
        }
        
        
        static int
#if !defined(DEBUG)        
        __attribute__((always_inline))
#endif            
        copy_one_unsafe(InIteratorT& first, InIteratorT last, EncodingT, 
                        OutIteratorT& dest, EncodingT)
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));
            
            assert(first != last);            
            
            int32_t num_trails = utf8_num_trails(static_cast<utf8_code_unit>(*first));
            switch (num_trails) {
                case 3: *dest++ = *first++;
                case 2: *dest++ = *first++;
                case 1: *dest++ = *first++;
                case 0: *dest++ = *first++;
            }
            return num_trails + 1;
        }        
        
    };
#endif    
        

    
    
#pragma mark - Class internal::converter UTF-16 to UTF-16   Specialization
#if 0 // not yet implemented  

    template <
        typename InIteratorT, typename OutIteratorT, typename EncodingT
    >
    class converter<
        InIteratorT, EncodingT, OutIteratorT, EncodingT, 
        typename boost::enable_if<
            boost::is_base_of<UTF_16_encoding_tag, EncodingT>
    >::type
    > 
    {
        typedef typename UTF_8_encoding_tag::encode_buffer_type encode_buffer_type;
        typedef typename UTF_8_encoding_tag::code_unit_type code_unit;
    
        static int
#if !defined(DEBUG)        
        __attribute__((always_inline))
#endif            
        copy_one_unsafe(InIteratorT& first, InIteratorT last, EncodingT, 
                 OutIteratorT& dest, EncodingT, 
                 filter::None)
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                                == sizeof(typename EncodingT::code_unit_type));
            
            // Upgrade Encoding to include endianness if required:
            typedef typename boost::mpl::if_<
            boost::is_same<UTF_16_encoding_tag, EncodingT>,
            typename to_host_endianness<EncodingT>::type,
            EncodingT 
            >::type                                         from_encoding_t;        
            typedef typename from_encoding_t::endian_tag    from_endian_t;
            typedef typename host_endianness::type          to_endian_t;
            
            assert(first != last);
            
            utf16_code_unit ch0 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            if (utf16_is_single(ch0)) {
                ++first;
                *dest++ = ch0;
                return 1;
            }
            else if (utf16_is_lead(ch0)) {
                ++first;
                if (first != last) {
                    utf16_code_unit ch1 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
                    if (utf16_is_trail(ch1)) {
                        ++first;
                        *dest++ = ch0;
                        *dest++ = ch1;
                        return 2;
                    }
                    else {
                        return E_TRAIL_EXPECTED;
                    }
                }
                else {
                    return E_UNEXPECTED_ENDOFINPUT;
                }
            }
            else {
                return E_INVALID_START_BYTE;
            }            
        }
        
    };
#endif    
    
    
}}}
#endif


// ============================================================================



//
// Class Template Specializations
// class converter
//



//
// Class Template Specializations
// class converter
//
//
// Template Specializations for Input Encoding equals Output Encoding
// and Filter is None.
// These "unsafe" versions merely copy.
//
#if 0
namespace json { namespace unicode { namespace internal { 
    
    
    template <
    typename InIteratorT, typename OutIteratorT, typename EncodingT>
    struct converter_unsafe<InIteratorT, OutIteratorT, EncodingT, filter::None,
    typename boost::enable_if<
    boost::is_same<UTF_8_encoding_tag, EncodingT>
    >::type> 
    {
        
        static int 
#if !defined(DEBUG)        
        __attribute__((always_inline))
#endif            
        copy_one(InIteratorT& first, InIteratorT last, EncodingT, 
                 OutIteratorT& dest, EncodingT, 
                 filter::None)
        {
            int result = 0;
            while (first != last) {
                *dest++ = *first++;
                ++result;
            }
            return result;
        }
    };
    
    
}}}
#endif





// Individual Specializations

///=============================================================================
#if 0
namespace json { namespace unicode { namespace internal {    
    
    
#pragma mark - UTF-8 to UTF-8  
    
    using namespace json::unicode;
    
    // UTF-8 to UTF-8
    //
    // Convert the first character contained in the well-formed UTF-8 sequence
    // [first, last) to an UTF-8 sequence starting at dest.
    // This function merely applies filter.
    // 
    // (see generic documentation above)
    // 
    // Returns the number of generated code units [1, 2].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid number of trail bytes.
    //  - does not check if first is a valid start byte for UTF-8
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED:     
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_same<UTF_8_encoding_tag, InEncodingT>,
    boost::is_same<UTF_8_encoding_tag, OutEncodingT>
    >
    >::type>
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        typedef InEncodingT                         from_encoding_t;
        typedef OutEncodingT                        to_encoding_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t());
            }            
            return result;
        }   
    };
    
    // Specialization for filter type equals filter::None
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_same<UTF_8_encoding_tag, InEncodingT>,
    boost::is_same<UTF_8_encoding_tag, OutEncodingT>
    >
    >::type>
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef json::unicode::filter::None         NoFilter; 
        typedef InEncodingT                         from_encoding_t;
        typedef OutEncodingT                        to_encoding_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            // assuming start byte
            const int count = utf8_num_trails(static_cast<uint8_t>(*first)) + 1;
            int i = count;
            while (i--) {
                *dest++ = *first++;
            }
            return count;
        }   
    };
    
    
    // UTF-8 to UTF-8
    //
    // Convert the first character contained in the possibly mal-formed UTF-8 
    // sequence [first, last) to an UTF-8 sequence starting at dest.
    // (see generic documentation above)
    //
    // This function merely verifies wellformedness and applies filter.
    //
    // Returns the number of generated code units [1, 2], or a negative number
    // indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_NONCHARACTER:             input character is an Unicode noncharacter
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_same<UTF_8_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_8_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        typedef InEncodingT                             from_encoding_t;
        typedef OutEncodingT                            to_encoding_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter) const
        {
            // TODO: possibly not the fastest algorithm. Would need to use a buffer
            // where the input is stored after the first conversion, so that we 
            // can just copy the content to dest.
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t());
            }
            return result;
        }    
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest) const
        {
            // TODO: possibly not the fastest algorithm. Would need to use a buffer
            // where the input is stored after the first conversion, so that we 
            // can just copy the content to dest.
            typename UTF_8_encoding_tag::encode_buffer_type buffer;
            int result = json::unicode::convert_to_codepoint(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t());
            }
            return result;
        }    
        
    };    
    
    
#pragma mark - UTF-8 to UTF-16  
    
    using namespace json::unicode;
    
    // UTF-8 to UTF-16
    //
    // Convert the first character contained in the well-formed UTF-8 sequence
    // [first, last) to an UTF-16 Unicode code unit or to a UTF-16
    // surrogate pair and copy the result to the sequence starting at dest.
    // 
    // (see generic documentation above)
    // 
    // Returns the number of generated code units [1, 2].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid number of trail bytes.
    //  - does not check if first is a valid start byte for UTF-8
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED:     
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_same<UTF_8_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
    >
    >::type>
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endiannes if required:
        typedef typename boost::mpl::if_<
        boost::is_same<UTF_16_encoding_tag, OutEncodingT>,
        typename to_host_endianness<OutEncodingT>::type,
        OutEncodingT 
        >::type                                         to_encoding_t;        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
    private:        
        int
        slow_path(
                  InIteratorT&     first, 
                  InIteratorT      last, 
                  OutIteratorT&    dest,
                  FilterT          filter = FilterT()) const
        {     
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                if (cp < 0x10000u) {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(cp));
                    return 1;
                } else {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(cp));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(cp));
                    return 2;
                }
            }
            return result;
        }        
        
    public:        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {     
#if 1                
            if (utf8_is_single(*first)) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename OutEncodingT::code_unit_type>(static_cast<uint8_t>(*first++)));
                return 1;
            } else {
                return slow_path(first, last, dest, filter);
            }
#else                
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                if (cp < 0x10000u) {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(cp));
                    return 1;
                } else {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(cp));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(cp));
                    return 2;
                }
            }
            return result;
#endif                
        }   
    };
    
    // UTF-8 to UTF-16
    //
    // Convert the first character contained in the possibly mal-formed UTF-8 
    // sequence [first, last) to an UTF-16 Unicode code unit or to a UTF-16
    // surrogate pair and copy the result to the sequence starting at dest.
    // If the character is an Unicode noncharacter, an error will be returned,
    // and no code unit will be generated.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code units [1, 2], or a negative number
    // indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_NONCHARACTER:             input character is an Unicode noncharacter
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_same<UTF_8_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endiannes if required:
        typedef typename boost::mpl::if_<
        boost::is_same<UTF_16_encoding_tag, OutEncodingT>,
        typename to_host_endianness<OutEncodingT>::type,
        OutEncodingT 
        >::type                                         to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        
        
    private: 
        static
        int slow_path(
                      InIteratorT&     first, 
                      InIteratorT      last, 
                      OutIteratorT&    dest,
                      FilterT          filter = FilterT())
        {
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint(first, last, from_encoding_t(), cp, filter);
            if (__builtin_expect(result > 0, 1)) {
                if (__builtin_expect(cp < 0x10000u, 1)) {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(cp));
                    return 1;
                } else {
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(cp));
                    *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(cp));
                    return 2;
                }
            }
            return result;
        }
        
        int write(code_point_t      codepoint,
                  OutIteratorT&     dest) const
        {
            if (__builtin_expect(codepoint < 0x10000u, 1)) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(codepoint));
                return 1;
            } else {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_lead(codepoint));
                *dest++ = byte_swap<from_endian_t, to_endian_t>(utf16_get_trail(codepoint));
                return 2;
            }
        }
        
        
    public:        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                                == sizeof(typename InEncodingT::code_unit_type));
            
#if 1            
            // TODO: filter is not applied to ASCII
            if (utf8_is_single(*first)) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(static_cast<typename to_encoding_t::code_unit_type>(static_cast<uint8_t>(*first++)));
                return 1;
            } else {
                code_point_t cp;
                int result = json::unicode::convert_to_codepoint(first, last, cp, filter);
                if (result) {
                    return write(cp, dest);
                }
                else {
                    return result;
                }
            }
#else
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint(first, last, cp, filter);
            if (result) {
                return write(cp, dest);
            }
            else {
                return result;
            }            
#endif            
        }    
    };    
    
    
    
#pragma mark - UTF-8 to UTF-32
    
    
    // UTF-8 to UTF-32
    // 
    // Convert the first character contained in the well-formed UTF-8 sequence
    // [first, last) to an Unicode code pointand copy the result to the 
    // sequence starting at dest. The input sequence must not be empty.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code units [0, 1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid number of trail bytes.
    //  - does not check if first is a valid start byte for UTF-8
    //  - does not check for noncharacters
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe< 
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_same<UTF_8_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type
    >
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
        boost::is_same<UTF_32_encoding_tag, OutEncodingT>,
        typename to_host_endianness<OutEncodingT>::type,
        OutEncodingT 
        >::type                                         to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp;        
            int result = json::unicode::convert_to_codepoint_unsafe(
                                                                    first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(cp);
            }
            return result;
        }    
    };
    
    // UTF-8 to UTF-32
    //
    // Convert the first character contained in the possibly mal-formed UTF-8 
    // sequence [first, last) to an Unicode code point and copy the result 
    // to the sequence starting at dest.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code points [0, 1], or a negative number
    // indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_PREDICATE_FAILED:         filter policy returned error
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_same<UTF_8_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        typedef InEncodingT                             from_encoding_t;
        // Upgrade Target Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
        boost::is_same<UTF_32_encoding_tag, OutEncodingT>,
        typename to_host_endianness<OutEncodingT>::type,
        OutEncodingT 
        >::type                                         to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint(
                                                             first, last, from_encoding_t(), 
                                                             cp, filter);
            if (result > 0) {
                *dest = byte_swap<from_endian_t, to_endian_t>(cp);
                ++dest;
            }
            return result;            
        }  
    };
    
    
    
    
    
#pragma mark - UTF-16 to UTF-8
    
    // Convert the first character contained in the well-formed UTF-16 sequence
    // [first, last) to a UTF-8 sequence and copy the result to the sequence 
    // starting at dest.
    //
    // Parameter inFirst must point to the start of a UTF-16 sequence.
    // The output sequence must be large enough to hold the generated code 
    // point.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code units [0 .. 4].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe <
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
    boost::is_same<UTF_8_encoding_tag, OutEncodingT>
    >
    >::type    
    >
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
        boost::is_same<UTF_16_encoding_tag, InEncodingT>,
        typename to_host_endianness<InEncodingT>::type,
        InEncodingT 
        >::type                                         from_encoding_t;        
        typedef OutEncodingT                            to_encoding_t;
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            
#if 1
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            ++first;  // consume single or high surrogate
            //int n = utf8_encoded_length(static_cast<code_point_t>(ch));  // Returns zero if this is a surrogate or not a valid Unicode code point
            if (utf16_is_single(ch)) {
                return json::unicode::convert_from_codepoint_unsafe(static_cast<code_point_t>(ch), dest, to_encoding_t(), filter);
            }
            else {
                // assuming: utf16_is_lead(ch)
                utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first)); 
                ++first;  // consume low surrogate
                code_point_t cp = utf16_surrogate_pair_to_code_point(ch, ch2);
                return json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t(), filter);
            }
#else            
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            int n = utf8_encoded_length(ch);  // Returns zero if this is a surrogate or not a valid Unicode code point
            switch (n) {
                case 1: 
                    *dest++ = (uint8_t)(ch);
                    ++first;
                    break;
                case 2:
                    *dest++ = (uint8_t)((ch >> 6) | 0xC0u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                case 3: 
                    *dest++ = (uint8_t)((ch >> 12) | 0xE0u);
                    *dest++ = (uint8_t)(((ch >> 6) & 0x3Fu) | 0x80u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                default: 
                    // assuming: utf16_is_lead(ch)
                    utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*++first)); 
                    code_point_t code_point = utf16_surrogate_pair_to_code_point(ch, ch2);
                    ++first;
                    n = json::unicode::convert_from_codepoint_unsafe(code_point, dest, UTF_8_encoding_tag());
            }
            
            return n;
#endif            
        }    
    };    
    
    // Convert the first character contained in the possibly mal-formed UTF-16 
    // sequence [first, last) to an UTF-8 sequence and copy the result to the
    // sequence starting at dest.
    //
    // Parameter inFirst should point to the start of a UTF-16 sequence.
    // The output sequence should be large enough to hold the generated code 
    // point.
    //
    // On exit first has been advanced by the number of code units constituting 
    // the character, or in case of an error, it points to the code unit where
    // the error occured.
    //
    // Returns the number of generated code units [0 .. 4], or a negative 
    // number indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one <
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
    boost::is_same<UTF_8_encoding_tag, OutEncodingT>
    >
    >::type    
    >
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename boost::mpl::if_<
        boost::is_same<UTF_16_encoding_tag, InEncodingT>,
        typename to_host_endianness<InEncodingT>::type,
        InEncodingT 
        >::type                                         from_encoding_t;   
        typedef OutEncodingT                            to_encoding_t;
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()
                    ) const
        {
            assert(first != last);
            
#if 1
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            ++first;  // consume single or high surrogate
            //int n = utf8_encoded_length(static_cast<code_point_t>(ch));  // Returns zero if this is a surrogate or not a valid Unicode code point
            if (utf16_is_single(ch)) {
                return json::unicode::convert_from_codepoint(static_cast<code_point_t>(ch), dest, to_encoding_t(), filter);
            }
            else if (utf16_is_lead(ch))
            {
                if (first == last) {
                    return E_UNEXPECTED_ENDOFINPUT;
                }
                utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
                if (!utf16_is_trail(ch2)) {
                    return E_TRAIL_EXPECTED;
                }
                ++first;  // consume low surrogate
                code_point_t cp = utf16_surrogate_pair_to_code_point(ch, ch2);
                return json::unicode::convert_from_codepoint(cp, dest, to_encoding_t(), filter);
            }
            else {
                return E_INVALID_START_BYTE;
            }
#else            
            utf16_code_unit ch = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
            int n = utf8_encoded_length(ch);  // Returns zero if this is a surrogate or not a valid Unicode code point
            switch (n) {
                case 1: 
                    *dest++ = (uint8_t)(ch);
                    ++first;
                    break;
                case 2:
                    *dest++ = (uint8_t)((ch >> 6) | 0xC0u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                case 3: 
                    *dest++ = (uint8_t)((ch >> 12) | 0xE0u);
                    *dest++ = (uint8_t)(((ch >> 6) & 0x3Fu) | 0x80u);
                    *dest++ = (uint8_t)((ch & 0x3Fu) | 0x80u);
                    ++first;
                    break;
                default: 
                    if (utf16_is_lead(ch)) {
                        ++first;
                        if (first != last) {
                            utf16_code_unit ch2 = byte_swap<from_endian_t, to_endian_t>(static_cast<utf16_code_unit>(*first));
                            if (utf16_is_trail(ch2)) {
                                code_point_t code_point = utf16_surrogate_pair_to_code_point(ch, ch2);
                                ++first;
                                n = json::unicode::convert_from_codepoint(code_point, dest, UTF_8_encoding_tag());
                            } else {
                                return E_TRAIL_EXPECTED;
                            } 
                        }
                        else {
                            return E_UNEXPECTED_ENDOFINPUT;
                        }
                    }
                    else {
                        return E_INVALID_START_BYTE;
                    }
            }
            
            
            // TODO implement filters
            
            return n;
#endif            
        }    
        
    };
    
    
    
    
#pragma mark - UTF-16 to UTF-16
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;                
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp;            
            int result = json::unicode::convert_to_codepoint_unsafe(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t());
            }
            return result;
        }    
    };
    
    // Specialization for filter type filter::None
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,    
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        typedef typename from_encoding_t::endian_tag            from_endian_t;
        typedef typename to_encoding_t::endian_tag              to_endian_t;
        typedef typename host_endianness::type                  host_endian_t;
        
        typedef json::unicode::filter::None                     NoFilter;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            utf16_code_unit cu = byte_swap<from_endian_t, host_endian_t>(static_cast<utf16_code_unit>(*first));
            int result = utf16_is_single(cu) ? 1 : 2;
            switch (result) {
                case 2: 
                    *dest++ = byte_swap<host_endian_t, to_endian_t>(cu);
                    ++first;
                    cu = byte_swap<from_endian_t, host_endian_t>(static_cast<utf16_code_unit>(*first));
                case 1:
                    *dest++ = byte_swap<host_endian_t, to_endian_t>(cu);
                    ++first;
            }
            return result;
        }    
    };
    
    
    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,    
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        inline int    
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const 
        {
            code_point_t cp;            
            int result = json::unicode::convert_to_codepoint(first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t());
            }
            return result;
        }    
    };    
    
#pragma mark - UTF-16 to UTF-32
    
    // Convert the first character contained in the well-formed UTF-16 sequence
    // [first, last) to a UTF-32 code point and copy the result to the 
    // sequence starting at dest.
    //
    // Parameter inFirst must point to the start of a UTF-16 sequence.
    // The output sequence must be large enough to hold the generated code 
    // point.
    //
    // On exit, first has been advanced by the number of code units constituting 
    // the character.
    //
    // Returns the number of generated code point [0,1].
    // 
    //
    // The result and behavior is undefined if the input sequence is not a 
    // well-formed Unicode.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //
    // Errors:
    //   E_PREDICATE_FAILED
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp;            
            int result = json::unicode::convert_to_codepoint_unsafe(
                                                                    first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                *dest++ = byte_swap<from_endian_t, to_endian_t>(cp);
            }
            return result;
        }    
    };    
    
    // Convert the first character contained in the possibly mal-formed UTF-16 
    // sequence [first, last) to a UTF-32 code point and copy the result 
    // to the sequence starting at dest.
    //
    // Parameter inFirst should point to the start of a UTF-16 sequence.
    // The output sequence should be large enough to hold the generated code 
    // point.
    //
    // On exit, parameter first has been advanced by the number of code units 
    // constituting the character, or in case of an error, it points to the code 
    // unit where the error occured.
    //
    // Returns the number of generated code units [0, 1], or a negative 
    // number indicating an error.
    //
    // Errors:
    //      E_TRAIL_EXPECTED:           trail byte expected
    //      E_UNEXPECTED_ENDOFINPUT:    unexpected and of input
    //      E_INVALID_START_BYTE        invalid start byte
    //      E_PREDICATE_FAILED
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_16_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename host_endianness::type          from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const 
        {
            code_point_t cp;
            int result = json::unicode::convert_to_codepoint(
                                                             first, last, from_encoding_t(), cp, filter);
            if (result > 0) {
                *dest = byte_swap<from_endian_t, to_endian_t>(cp);
                ++dest;
            }
            return result;
        }    
    };    
    
    
#pragma mark - UTF-32 to UTF-8
    
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-8 and copy the result to the sequence starting at dest.
    //
    // (see generic documentation)
    //
    // Returns the number of generated code point [0 .. 4].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    // 
    //  Errors:
    //    E_PREDICATE_FAILED
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_same<UTF_8_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        // Output iterators do not strictly require a value_type!
        //BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<OutIteratorT>::type)
        //                    == sizeof(typename OutEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef OutEncodingT                                    to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-8 and copy the result to the sequence starting at outFirst.
    //
    // (see generic documentation above)
    //
    // Returns the number of generated code point [0 .. 4], or a negative number
    // which indicates an error.
    //
    //  Errors:
    //  E_NONCHARACTER:         The given Unicode code point is a noncharacter.
    //  E_INVALID_CODE_POINT:   The Unicode code point is a surrogate or not
    //                          not a valid Unicode code point at all.
    //  E_PREDICATE_FAILED
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_same<UTF_8_encoding_tag, OutEncodingT>
    >
    >::type
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef OutEncodingT                                    to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_from_codepoint(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    
    
#pragma mark - UTF-32 to UTF-16
    
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-16 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code point [0 .. 4].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t());
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    
    // Convert the first Unicode code point from the sequence [first,last) 
    // to UTF-16 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation)
    //
    // Returns the number of generated code point [0 .. 4], or a negative number
    // which indicates an error.
    //
    //  Errors:
    //  E_PREDICATE_FAILED: 
    //  all errors from convert_one(code_point, iterator, encoding)
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_16_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source and Target Encoding to include endianness if required:        
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename host_endianness::type          to_endian_t;
        
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, to_endian_t>(*first));
            int result = json::unicode::convert_from_codepoint(cp, dest, to_encoding_t());
            if (result > 0) {
                ++first;                
            }
            return result;
        }    
    };
    
    
#pragma mark - UTF-32 to UTF-32
    
#pragma mark convert_one_unsafe
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code point [1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-16
    //  - does not check for noncharacters
    //
    // Errors:
    //  E_PREDICATE_FAILED
    //
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        typedef typename host_endianness::type          host_endian_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, host_endian_t>(*first));
            int result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code point [1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Errors:
    //  E_PREDICATE_FAILED
    //
#pragma mark convert_one    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT, 
    typename FilterT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, FilterT,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag    from_endian_t;
        typedef typename to_encoding_t::endian_tag      to_endian_t;
        typedef typename host_endianness::type          host_endian_t;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    FilterT          filter = FilterT()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, host_endian_t>(*first));
            int result = json::unicode::convert_from_codepoint_unsafe(cp, dest, to_encoding_t(), filter);
            if (result > 0) {
                ++first;
            }
            return result;
        }    
    };
    
    // Specialization for filter type equals filter::None
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code points [1].
    //
    // The result and behavior is undefined if the input sequence is not a 
    // valid Unicode code point.
    // 
    // Unsafe version:
    //  - does not check for iterating past last
    //  - does not check for valid surrogate pair
    //  - does not check if first is a valid start byte for UTF-32
    //
#pragma mark convert_one_unsafe<filter::None>    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one_unsafe<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag            from_endian_t;
        typedef typename to_encoding_t::endian_tag              to_endian_t;
        typedef typename host_endianness::type                  host_endian_t;
        
        typedef json::unicode::filter::None                     NoFilter;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            *dest++ = static_cast<utf32_code_unit>(byte_swap<from_endian_t, to_endian_t>(*first++));
            return 1;
        }    
    };
    
    // Specialization for filter type equals filter::None
    // Convert the first Unicode code point from the sequence [first, last) 
    // to UTF-32 and copy the result to the sequence starting at dest.
    // If the encoding's endianness does not equal the host endianness a swap 
    // will automatically applied to the resulting code points.
    //
    // (see generic documentation above.)
    // 
    // Returns the number of generated code points [1].
    //
    // Errors:
    //  E_INVALID_CODE_POINT
#pragma mark convert_one<filter::None>    
    template <
    typename InIteratorT, typename InEncodingT, 
    typename OutIteratorT, typename OutEncodingT
    >
    struct convert_one<
    InIteratorT, InEncodingT, OutIteratorT, OutEncodingT, json::unicode::filter::None,
    typename boost::enable_if<
    boost::mpl::and_<
    boost::is_base_of<UTF_32_encoding_tag, InEncodingT>,
    boost::is_base_of<UTF_32_encoding_tag, OutEncodingT>
    >
    >::type    
    > 
    {
        BOOST_STATIC_ASSERT(sizeof(typename boost::iterator_value<InIteratorT>::type)
                            == sizeof(typename InEncodingT::code_unit_type));
        
        // Upgrade Source Encoding to include endianness if required:
        typedef typename add_endianness<InEncodingT>::type      from_encoding_t;        
        typedef typename add_endianness<OutEncodingT>::type     to_encoding_t;        
        
        typedef typename from_encoding_t::endian_tag            from_endian_t;
        typedef typename to_encoding_t::endian_tag              to_endian_t;
        typedef typename host_endianness::type                  host_endian_t;
        
        typedef json::unicode::filter::None                     NoFilter;
        
        inline int
        operator() (
                    InIteratorT&     first, 
                    InIteratorT      last, 
                    OutIteratorT&    dest,
                    NoFilter         filter = NoFilter()) const
        {
            code_point_t cp = static_cast<code_point_t>(byte_swap<from_endian_t, host_endian_t>(*first));
            if (isUnicodeScalarValue(cp)) {
                *dest++ = static_cast<code_point_t>(byte_swap<host_endian_t, to_endian_t>(cp));
                ++first;
                return 1;                
            }
            else {
                return E_INVALID_CODE_POINT;
            }
        }    
    };
    
    
}}}
#endif // #if 0



#endif

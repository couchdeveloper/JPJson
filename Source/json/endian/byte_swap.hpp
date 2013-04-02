//
//  byte_swap.hpp
//
//  Created by Andreas Grosam on 5/18/11.
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

#ifndef JSON_BYTE_SWAP_HPP
#define JSON_BYTE_SWAP_HPP


#include "json/config.hpp"
#include "endian.hpp"
#include <stdexcept>
#include <cstdlib>
#include <type_traits>

namespace json 
{
    namespace internal 
    {
        template <typename Base, typename Derived>
        using IsBaseAndDerived = typename std::integral_constant<bool,
            std::is_base_of<Base, Derived>::value
            and !std::is_same<Base, Derived>::value
        >::type;
        

        template<typename T, size_t sz>
        struct swap_bytes {
            inline T operator()(T val) const {
                throw std::out_of_range("data size");
            }
        };
        
        template<typename T>
        struct swap_bytes<T, 1> {
            constexpr inline T operator()(T val) const {
                return val;
            }
        };
        
        template<typename T>
        struct swap_bytes<T, 2> {
            constexpr inline T operator()(T val) const {
#if defined (__APPLE__) && !defined (JSON_SWAP_NO_BUILTIN)
                return __DARWIN_OSSwapInt16(val);
#elif defined (__llvm__)  && !defined (JSON_SWAP_NO_BUILTIN)
                return __builtin_bswap16(val);                
#elif defined (__GNUC__) && (__GNUC_MINOR__ >= 3 ) 
                return __builtin_bswap16(val);                
#elif defined (_MSC_VER)  && !defined (JSON_SWAP_NO_BUILTIN)
                return _byteswap_ushort(val);                
#else                
                return ((((val) >> 8) & 0xff) | (((val) & 0xff) << 8));
#endif                
            }
        };
        
        template<typename T>
        struct swap_bytes<T, 4> {
            constexpr inline T operator()(T val) const {
#if defined (__APPLE__)  && !defined (JSON_SWAP_NO_BUILTIN)
                return __DARWIN_OSSwapInt32(val);
#elif defined (__llvm__)  && !defined (JSON_SWAP_NO_BUILTIN)
                return __builtin_bswap32(val);                
#elif defined (__GNUC__) && (__GNUC_MINOR__ >= 3 ) && !defined (JSON_SWAP_NO_BUILTIN)
                return __builtin_bswap32(val);                
#elif defined (_MSC_VER)  && !defined (JSON_SWAP_NO_BUILTIN)
                return _byteswap_ulong(val);                
#else                
                
                return ((((val) & 0xff000000) >> 24) |
                        (((val) & 0x00ff0000) >>  8) |
                        (((val) & 0x0000ff00) <<  8) |
                        (((val) & 0x000000ff) << 24));
#endif            
            }
        };
        
        template<>
        struct swap_bytes<float, 4> {
            inline float operator()(float val) const{
                uint32_t mem = swap_bytes<uint32_t, sizeof(uint32_t)>()(*(uint32_t*)&val);
                return *(float*)&mem;
            }
        };
        
        template<typename T>
        struct swap_bytes<T, 8> {
            constexpr inline T operator()(T val) const {
#if defined (__APPLE__)  && !defined (JSON_SWAP_NO_BUILTIN)
                return __DARWIN_OSSwapInt64(val);
#elif defined (__llvm__)  && !defined (JSON_SWAP_NO_BUILTIN)
                return __builtin_bswap64(val);                
#elif defined (__GNUC__) && (__GNUC_MINOR__ >= 3 )  && !defined (JSON_SWAP_NO_BUILTIN)
                return __builtin_bswap64(val);                
#elif defined (_MSC_VER)  && !defined (JSON_SWAP_NO_BUILTIN)
                return _byteswap_uint64(val);                
#else                
                
                return ((((val) & 0xff00000000000000ull) >> 56) |
                        (((val) & 0x00ff000000000000ull) >> 40) |
                        (((val) & 0x0000ff0000000000ull) >> 24) |
                        (((val) & 0x000000ff00000000ull) >> 8 ) |
                        (((val) & 0x00000000ff000000ull) << 8 ) |
                        (((val) & 0x0000000000ff0000ull) << 24) |
                        (((val) & 0x000000000000ff00ull) << 40) |
                        (((val) & 0x00000000000000ffull) << 56));
#endif            
            }
        };
        
        template<>
        struct swap_bytes<double, 8> {
            inline double operator()(double val) const {
                uint64_t mem = swap_bytes<uint64_t, sizeof(uint64_t)>()(*(uint64_t*)&val);
                return *(double*)&mem;
            }
        };
        
        template<typename FromEndianT, typename ToEndianT, class T>
        struct do_byte_swap {
            constexpr inline T operator()(T value) const {
                return swap_bytes<T, sizeof(T)>()(value);
            }
        };
        
        
        // Specialisation when attempting to swap to the same endianess
        template<typename FromAndToEndianT, class T> 
        struct do_byte_swap<FromAndToEndianT, FromAndToEndianT, T> { 
            constexpr inline T operator()(T value) const {
                return value; 
            } 
        };
        
    } // namespace internal

    
    // Restrict candidates whoses template parameter types are 
    // derived from endian_tag and which are not equal (this results
    // in a byte swap):
    template<typename FromEndianT, typename ToEndianT, class T>
    constexpr inline
    typename std::enable_if<
            internal::IsBaseAndDerived<internal::endian_tag, FromEndianT>::value
            and internal::IsBaseAndDerived<internal::endian_tag, ToEndianT>::value
            and !std::is_same<FromEndianT, ToEndianT>::value
          , T
        >::type
    byte_swap(T value)
    {
        // ensure the data is only 1, 2, 4 or 8 bytes
        static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "");
        // ensure we're only swapping arithmetic types
        static_assert(std::is_arithmetic<T>::value, "");
        
        return internal::do_byte_swap<FromEndianT, ToEndianT, T>()(value);
    }
    
    // Restrict candidates whoses template parameter types are 
    // derived from endian_tag and which are equal (this results
    // in a no-op):
    template<typename FromEndianT, typename ToEndianT, class T>
    constexpr inline
    typename std::enable_if<
        internal::IsBaseAndDerived<internal::endian_tag, FromEndianT>::value
        and internal::IsBaseAndDerived<internal::endian_tag, ToEndianT>::value
        and std::is_same<FromEndianT, ToEndianT>::value
      , T
    >::type
    byte_swap(T value)
    {
        return value;
    }
    
    
    // Catch Illegal template parameters which are NOT derived from endian_tag.
    template<typename FromEndianT, typename ToEndianT, class T>
    constexpr inline
    typename std::enable_if<
        !internal::IsBaseAndDerived<internal::endian_tag, FromEndianT>::value
        or !internal::IsBaseAndDerived<internal::endian_tag, ToEndianT>::value
        , T
    >::type
    byte_swap(T value);
//    {
//        static_assert(sizeof(T) == 0, "");
//        return T();
//    }
    

    template <typename T>
    constexpr T byte_swap(const T& value) {
        return internal::swap_bytes<T, sizeof(T)>()(value);
    }
    
} // namespace json





#endif // JSON_BYTE_SWAP_HPP

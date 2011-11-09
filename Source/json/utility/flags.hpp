//
//  flags.hpp
//  
//
//  Created by Andreas Grosam on 8/30/11.
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

#ifndef JSON_UTILITY_FLAGS_HPP
#define JSON_UTILITY_FLAGS_HPP


#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/assert.hpp>

//
//  Typesafe Bitwise Enum
//
//  Obsolete with C++11
//
//  Example:
//
//  // Define a struct with an embedded enum type, subclassed from
//  // template class json::utility::flags:
//
//    struct a_flags {
//        enum E {
//            None  = 1UL << 0,
//            A     = 1UL << 1,
//            B     = 1UL << 2,
//            C     = 1UL << 3
//        };
//        typedef E enum_type;
//    };
//    
//    typedef json::utility::flags<a_flags> AFlags; 
// 
//  // Define global operators for enums:
//    UTILITY_DEFINE_FLAG_OPERATORS(AFlags);  // shall be defined in global namespace
//
//
//
//
//    Flags flags = Flags::A | Flags::B;
//    OtherFlags oflags = OtherFlags::A;

//    if (flags) {/* one or more flags set */}
//    if (flags & Flags::A) { /* flag A is set */ }
//
//    if ((oflags & OtherFlags::A) and not(flags & Flags::C)) {}
//    if (oflags & OtherFlags::A) {}
//    if (OtherFlags::A & oflags) {}
//    if (OtherFlags::A && oflags) {} // should not comile?
//    if (OtherFlags::A == oflags) {}
//    if (oflags == 0) {} // implicit conversion from 0 to false
//    if (oflags != 0) {}
//    if (oflags == false) {} 
//    if (oflags != false) {}
//    
//    if (oflags1 & (OtherFlags::A | OtherFlags::B)) {}
//    Flags flags3 = Flags::A | Flags::B;
//    if (oflags and flags3) {}
//    if (oflags or flags3) {}
//    if (oflags and not flags3) {}

    
    
    
namespace json { namespace utility { 
    
    template <typename T>
    struct flags_enable : boost::mpl::false_ {
    };
    
    
    
    template <typename T>
    class flags : public T
    {                
        typedef void (flags::*bool_type)() const;
        void this_type_does_not_support_rel_ops() const {}
        
    public:
        
        typedef typename T::enum_type       enum_type;        

        BOOST_STATIC_ASSERT( boost::is_enum<enum_type>::value );
        
        
        flags(const flags& other) : flags_(other.flags_) {}
        
        flags(enum_type flag) : flags_(flag) {}
        
        flags& operator=(const flags& other) {
            flags_ = other.flags_;
            return *this;
        }
        
        operator bool_type() const {
            return (flags_ != 0) ? &flags::this_type_does_not_support_rel_ops : 0;
        }
        
    private:
        
        explicit flags(int flags) : flags_(flags) {}
        
        // friends    
    private:
        
        // Relational operators
        friend inline
        bool operator==(const flags& lhv, const flags& rhv) {
            return lhv.flags_ == rhv.flags_; // bit-wise comparison
        }
        
        friend inline
        bool operator!= (const flags& lhv, const flags& rhv) {
            return !(lhv == rhv);  // bit-wise comparison
        }
        
        
        // Bit operations
        friend inline flags
        operator&(flags lhv, flags rhv)
        { return flags(static_cast<int>(lhv.flags_) & static_cast<int>(rhv.flags_)); }
        
        friend inline flags
        operator|(flags lhv, flags rhv)
        { return flags(static_cast<int>(lhv.flags_) | static_cast<int>(rhv.flags_)); }
        
        friend inline flags
        operator^(flags lhv, flags rhv)
        { return flags(static_cast<int>(lhv.flags_) ^ static_cast<int>(rhv.flags_)); }
        
        friend inline flags&
        operator|=(flags& lhv, flags rhv)
        { return lhv = lhv | rhv; }
        
        friend inline flags&
        operator&=(flags& lhv, flags rhv)
        { return lhv = lhv & rhv; }
        
        friend inline flags&
        operator^=(flags& lhv, flags rhv)
        { return lhv = lhv ^ rhv; }
        
        friend inline flags
        operator~(flags lhv)
        { return flags(~static_cast<int>(lhv.flags_)); }
        
        
        // std stream operator
        friend inline 
        std::ostream& operator<<(std::ostream& os, const flags& v) {
            os << v.flags_;
            return os;
        }
        
    private:
        int flags_;
    };
    
}}   // namesapce json::utility





namespace {
    //
    // Those operators must be defined in global or anonymous namespace
    //
    
    using json::utility::flags_enable;
    using json::utility::flags;
    
    template <class EnumT>
    inline 
    typename boost::enable_if<
        flags_enable<EnumT>, 
        flags<typename flags_enable<EnumT>::type>
    >::type    
    operator&(EnumT lhv, EnumT rhv)
    { return flags<typename flags_enable<EnumT>::type>(lhv) & flags<typename flags_enable<EnumT>::type>(rhv); }
    
    template <class EnumT> 
    inline 
    typename boost::enable_if<
        flags_enable<EnumT>, 
        flags<typename flags_enable<EnumT>::type>
    >::type        
    operator|(EnumT lhv, EnumT rhv)
    { return flags<typename flags_enable<EnumT>::type>(lhv) | flags<typename flags_enable<EnumT>::type>(rhv); }
    
    template <class EnumT> 
    inline
    typename boost::enable_if<
        flags_enable<EnumT>, 
        flags<typename flags_enable<EnumT>::type>
    >::type        
    operator^(EnumT lhv, EnumT rhv)
    { return flags<typename flags_enable<EnumT>::type>(lhv) ^ flags<typename flags_enable<EnumT>::type>(rhv); }
    
    template <class EnumT> 
    inline 
    typename boost::enable_if<
        flags_enable<EnumT>, 
        flags<typename flags_enable<EnumT>::type>&
    >::type        
    operator|=(EnumT& lhv, EnumT rhv)
    { return lhv = lhv | rhv; }
    
    template <class EnumT> 
    inline 
    typename boost::enable_if<
        flags_enable<EnumT>, 
        flags<typename flags_enable<EnumT>::type>&
    >::type        
    operator&=(EnumT& lhv, EnumT rhv)
    { return lhv = lhv & rhv; }
    
    template <class EnumT> 
    inline 
    typename boost::enable_if<
        flags_enable<EnumT>, 
        flags<typename flags_enable<EnumT>::type>&
    >::type        
    operator^=(EnumT& lhv, EnumT rhv)
    { return lhv = lhv ^ rhv; }
    
    template <class EnumT> 
    inline 
    typename boost::enable_if<
        flags_enable<EnumT>, 
        flags<typename flags_enable<EnumT>::type>
    >::type        
    operator~(EnumT v)
    { return ~flags<typename flags_enable<EnumT>::type>(v); }
    
}

#define UTILITY_DEFINE_FLAG_OPERATORS(T)                        \
namespace json { namespace utility {                            \
    template <>                                                 \
    struct flags_enable<T::enum_type> : boost::mpl::true_ {     \
        typedef T type;                                         \
    };                                                          \
}}






#endif // JSON_UTILITY_FLAGS_HPP

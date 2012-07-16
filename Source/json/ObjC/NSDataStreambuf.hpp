//
//  NSDataStreambuf.hpp
//  
//
//  Created by Andreas Grosam on 5/23/12.
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

#ifndef JSON_OBJC_NSDATA_STREAMBUF_HPP
#define JSON_OBJC_NSDATA_STREAMBUF_HPP


#include "json/config.hpp"
#include <streambuf>
#include <utility>
#include <algorithm>
#include <stdint.h>
#include <boost/config.hpp>

#import <Foundation/Foundation.h>


namespace {

    void throwMissalignedBuffer() {
        throw std::runtime_error("NSDataStreambuf: missaligned buffer");
    }
    
}

namespace json { namespace objc {

    using namespace std;
    
    template <class CharT, class Traits = char_traits<CharT> >
    class  NSDataStreambuf
    : public basic_streambuf<CharT, Traits>
    {
    public:
        typedef CharT                         char_type;
        typedef Traits                        traits_type;
        typedef typename traits_type::int_type int_type;
        typedef typename traits_type::pos_type pos_type;
        typedef typename traits_type::off_type off_type;
                
    private:
        
        void* _buffer;
        size_t _buffer_size;
        mutable char_type* _hm;
        ios_base::openmode _mode;
        
    public:
        //
        // Constructors:
        //
        // Initializes the internal openmode field as set by parameter `which` 
        // and initializes the base class by calling its parent's constructor 
        // streambuf::streambuf        
        //
        // @param data  NSData object whose content is used to intialize the 
        //              internal byte sequence.
        //
        // @param which Specifies the openmode for the stream buffer. If (which 
        //              & ios_base::in) is true, input operations are allowed, 
        //              and if (which & ios_base::out) is true, output operations 
        //              are allowed.
        //
        
        explicit NSDataStreambuf(ios_base::openmode __wch = ios_base::in | ios_base::out);
        explicit NSDataStreambuf(NSData* __data,
                                        ios_base::openmode __wch = ios_base::in | ios_base::out);
    #ifndef BOOST_NO_RVALUE_REFERENCES
        NSDataStreambuf(NSDataStreambuf&& __rhs);
    #endif
        
        // Assign and swap:
    #ifndef BOOST_NO_RVALUE_REFERENCES
        NSDataStreambuf& operator=(NSDataStreambuf&& __rhs);
    #endif
        
        // Destructor
        ~NSDataStreambuf() {
            free(_buffer);
        }

#if 0
        void swap(NSDataStreambuf& __rhs);
#endif        
        
        //
        //  Get and set:
        //
        // The first version sets a copy of parameter __data as the new internal 
        // character sequence and initializes the input and output sequences according 
        // to the mode used when the object was created.
        //
        // The second version returns a NSData object with a copy of the content 
        // in the internal character sequence.

        void data(NSData* __data);
        NSData* data() const;
        
        
        
    protected:
        // 27.8.1.4 Overridden virtual functions:
        virtual int_type underflow();
        virtual int_type pbackfail(int_type __c = traits_type::eof());
        virtual int_type overflow (int_type __c = traits_type::eof());
        virtual pos_type seekoff(off_type __off, ios_base::seekdir __way,
                                 ios_base::openmode __wch = ios_base::in | ios_base::out);
        virtual pos_type seekpos(pos_type __sp,
                                 ios_base::openmode __wch = ios_base::in | ios_base::out);
    };

    template <class CharT, class Traits>
    inline 
    NSDataStreambuf<CharT, Traits>::NSDataStreambuf(ios_base::openmode __wch)
    : _hm(0), _mode(__wch), _buffer(NULL), _buffer_size(0)
    {
    }

    template <class CharT, class Traits>
    inline 
    NSDataStreambuf<CharT, Traits>::NSDataStreambuf(NSData* __data, ios_base::openmode __wch)
    :   _hm(0), _mode(__wch), _buffer(NULL), _buffer_size(0)
    {
        data(__data);
    }

    #ifndef BOOST_NO_RVALUE_REFERENCES
#if 0    

    template <class CharT, class Traits>
    NSDataStreambuf<CharT, Traits>::NSDataStreambuf(NSDataStreambuf&& __rhs);

    template <class CharT, class Traits>
    NSDataStreambuf<CharT, Traits>&
    NSDataStreambuf<CharT, Traits>::operator=(NSDataStreambuf&& __rhs);
#endif
    #endif  // BOOST_NO_RVALUE_REFERENCES
    
#if 0    

    template <class CharT, class Traits>
    void
    NSDataStreambuf<CharT, Traits>::swap(NSDataStreambuf& __rhs)
    {
        ptrdiff_t __rninp = __rhs.gptr()  - __rhs.eback();
        ptrdiff_t __reinp = __rhs.egptr() - __rhs.eback();
        ptrdiff_t __rnout = __rhs.pptr()  - __rhs.pbase();
        ptrdiff_t __reout = __rhs.epptr() - __rhs.pbase();
        ptrdiff_t __rhm   = __rhs._hm   - __rhs.pbase();
        ptrdiff_t __lninp = this->gptr()  - this->eback();
        ptrdiff_t __leinp = this->egptr() - this->eback();
        ptrdiff_t __lnout = this->pptr()  - this->pbase();
        ptrdiff_t __leout = this->epptr() - this->pbase();
        ptrdiff_t __lhm   = this->_hm   - this->pbase();
        _VSTD::swap(_mode, __rhs._mode);
        __str_.swap(__rhs.__str_);
        char_type* __p = static_cast<char_type*>(_buffer);
        this->setg(__p, __p + __rninp, __p + __reinp);
        this->setp(__p, __p + __reout);
        this->pbump(__rnout);
        _hm = __p + __rhm;
        __p = const_cast<char_type*>(static_cast<char_type*>(__rhs._buffer));
        __rhs.setg(__p, __p + __lninp, __p + __leinp);
        __rhs.setp(__p, __p + __leout);
        __rhs.pbump(__lnout);
        __rhs._hm = __p + __lhm;
        locale __tl = __rhs.getloc();
        __rhs.pubimbue(this->getloc());
        this->pubimbue(__tl);
    }

    template <class CharT, class Traits>
    inline 
    void
    swap(NSDataStreambuf<CharT, Traits>& __x,
         NSDataStreambuf<CharT, Traits>& __y)
    {
        __x.swap(__y);
    }
    
#endif    

    template <class CharT, class Traits>
    NSData*
    NSDataStreambuf<CharT, Traits>::data() const
    {
        if (_mode & ios_base::out)
        {
            if (_hm < this->pptr())
                _hm = this->pptr();
            return [NSData dataWithBytes:this->pbase() length:sizeof(char_type)*(_hm-this->pbase())];
        }
        else if (_mode & ios_base::in) {
            return [NSData dataWithBytes:this->eback() length:sizeof(char_type)*(this->egptr()-this->eback())];
        }
        return [NSData data];
    }

    template <class CharT, class Traits>
    void
    NSDataStreambuf<CharT, Traits>::data(NSData* __data)
    {
        size_t __byte_size = [__data length];
        
        switch (sizeof(CharT)) {
            case 1:break;
            case 2: {
                if ((__byte_size & 0x01u) !=0 )
                    throwMissalignedBuffer();
            }
                break;
            case 4: {
                if ((__byte_size & 0x03u) !=0)
                    throwMissalignedBuffer();
            }
                break;
        }
        free(_buffer);
        _buffer = NULL;
        _buffer_size = 0;
        _hm = 0;

        size_t __sz = __byte_size/(sizeof(char_type));
        if (_mode & ios_base::out)
        {
            if (__data) {
                _buffer_size = std::max(size_t(1), size_t(__byte_size > 10));
                _buffer_size <<= 1;
                _buffer = malloc(_buffer_size);
                memcpy(_buffer, [__data bytes], __byte_size);
            }
            char_type* __bytes = static_cast<char_type*>(_buffer);
            _hm = __bytes + __sz;
            this->setp(__bytes, __bytes + _buffer_size/sizeof(char_type));
            if (_mode & (ios_base::app | ios_base::ate))
                this->pbump(static_cast<int>(__sz));
        }
        if (_mode & ios_base::in)
        {
            if (_buffer == NULL && __byte_size > 0) {
                _buffer_size = __byte_size;
                _buffer = malloc(_buffer_size);
                memcpy(_buffer, [__data bytes], __byte_size);
            }
            char_type* __bytes = static_cast<char_type*>(_buffer);
            _hm = __bytes + __sz;
            this->setg(__bytes, __bytes, _hm);
        }        
    }
    
    

    template <class CharT, class Traits>
    typename NSDataStreambuf<CharT, Traits>::int_type
    NSDataStreambuf<CharT, Traits>::underflow()
    {
        if (_hm < this->pptr())
            _hm = this->pptr();
        if (_mode & ios_base::in)
        {
            if (this->egptr() < _hm)
                this->setg(this->eback(), this->gptr(), _hm);
            if (this->gptr() < this->egptr())
                return traits_type::to_int_type(*this->gptr());
        }
        return traits_type::eof();
    }

    template <class CharT, class Traits>
    typename NSDataStreambuf<CharT, Traits>::int_type
    NSDataStreambuf<CharT, Traits>::pbackfail(int_type __c)
    {
        if (_hm < this->pptr())
            _hm = this->pptr();
        if (this->eback() < this->gptr())
        {
            if (traits_type::eq_int_type(__c, traits_type::eof()))
            {
                this->setg(this->eback(), this->gptr()-1, _hm);
                return traits_type::not_eof(__c);
            }
            if ((_mode & ios_base::out) ||
                traits_type::eq(traits_type::to_char_type(__c), this->gptr()[-1]))
            {
                this->setg(this->eback(), this->gptr()-1, _hm);
                *this->gptr() = traits_type::to_char_type(__c);
                return __c;
            }
        }
        return traits_type::eof();
    }

    template <class CharT, class Traits>
    typename NSDataStreambuf<CharT, Traits>::int_type
    NSDataStreambuf<CharT, Traits>::overflow(int_type __c)
    {
        if (!traits_type::eq_int_type(__c, traits_type::eof()))
        {
            ptrdiff_t __ninp = this->gptr()  - this->eback();
            if (this->pptr() == this->epptr())
            {
                if (!(_mode & ios_base::out)) {
                    return traits_type::eof();
                }
                ptrdiff_t __nout = this->pptr() - this->pbase();
                ptrdiff_t __hm = _hm - this->pbase();
                
                if (_buffer_size == 0) {
                    _buffer_size = 32*1024;
                } else {
                    _buffer_size <<=1;
                }
                _buffer = realloc(_buffer, _buffer_size);
                char_type* __p = static_cast<char_type*>(_buffer);
                this->setp(__p, __p + _buffer_size/(sizeof(char_type)));
                this->pbump(static_cast<int>(__nout));
                _hm = this->pbase() + __hm;
            }
            _hm = std::max(this->pptr() + 1, _hm);
            if (_mode & ios_base::in)
            {
                char_type* __p = static_cast<char_type*>(_buffer);
                this->setg(__p, __p + __ninp, _hm);
            }
            return this->sputc(__c);
        }
        return traits_type::not_eof(__c);
    }

    template <class CharT, class Traits>
    typename NSDataStreambuf<CharT, Traits>::pos_type
    NSDataStreambuf<CharT, Traits>::seekoff(off_type __off,
                                                          ios_base::seekdir __way,
                                                          ios_base::openmode __wch)
    {
        if (_hm < this->pptr())
            _hm = this->pptr();
        if ((__wch & (ios_base::in | ios_base::out)) == 0)
            return pos_type(-1);
        if ((__wch & (ios_base::in | ios_base::out)) == (ios_base::in | ios_base::out)
            && __way == ios_base::cur)
            return pos_type(-1);
        off_type __noff;
        switch (__way)
        {
            case ios_base::beg:
                __noff = 0;
                break;
            case ios_base::cur:
                if (__wch & ios_base::in)
                    __noff = this->gptr() - this->eback();
                else
                    __noff = this->pptr() - this->pbase();
                break;
            case ios_base::end:
                __noff = _hm - static_cast<char_type*>(_buffer);
                break;
            default:
                return pos_type(-1);
        }
        __noff += __off;
        if (__noff < 0 || _hm - static_cast<char_type*>(_buffer) < __noff)
            return pos_type(-1);
        if (__noff != 0)
        {
            if ((__wch & ios_base::in) && this->gptr() == 0)
                return pos_type(-1);
            if ((__wch & ios_base::out) && this->pptr() == 0)
                return pos_type(-1);
        }
        if (__wch & ios_base::in)
            this->setg(this->eback(), this->eback() + __noff, _hm);
        if (__wch & ios_base::out)
        {
            this->setp(this->pbase(), this->epptr());
            this->pbump(static_cast<int>(__noff));
        }
        return pos_type(__noff);
    }

    template <class CharT, class Traits>
    inline 
    typename NSDataStreambuf<CharT, Traits>::pos_type
    NSDataStreambuf<CharT, Traits>::seekpos(pos_type __sp,
                                                          ios_base::openmode __wch)
    {
        return seekoff(__sp, ios_base::beg, __wch);
    }


}}   // namespace json::objc





#endif  // JSON_OBJC_NSDATA_STREAMBUF_HPP

//
//  basic_NSStreamStreambuf.hpp
//  
//
//  Created by Andreas Grosam on 05.10.12.
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

#ifndef JSON_OBJC_NSSTREAM_STREAMBUF_HPP
#define JSON_OBJC_NSSTREAM_STREAMBUF_HPP


#include "json/config.hpp"
#include <streambuf>
#include <utility>
#include <algorithm>
#include <stdint.h>
#include <boost/config.hpp>
#include <fstream>

#import <Foundation/Foundation.h>


#define NSSTREAMSTREAMBUF_NO_EXCEPTIONS

namespace {
    
    void throwMissalignedBuffer() {
        throw std::runtime_error("basic_NSStreamStreambuf: missaligned buffer");
    }
    
}

/**
 
 basic_NSStreamStreambuf applies the functionality of a streambuf class to read 
 from a Cocoa NSInputStream object or write into an NSOuputStream object.

 By calling member open, a Cocoa NSInputStream or NSOutputStream object is 
 associated to the streambuf as its associated byte sequence. Depending on the 
 mode used in this operation, the access to the controlled input sequence or the 
 controller ouput sequence may be restricted.
 
 The state of the streambuf object - i.e. whether its NSStream object is open
 or not - may be tested by calling member function is_open.
 
 The class overrides some virtual members inherited from streambuf to provide a 
 specific functionality for files
 
 Note: valid ios_base::openmodes: 
    
 
 
 */

namespace json { namespace objc {
    
    using namespace std;
    
    template <class CharT, class Traits = char_traits<CharT> >
    class  basic_NSStreamStreambuf
    : public basic_streambuf<CharT, Traits>
    {
    public:
        typedef CharT                               char_type;
        typedef Traits                              traits_type;
        typedef typename traits_type::int_type      int_type;
        typedef typename traits_type::pos_type      pos_type;
        typedef typename traits_type::off_type      off_type;
        typedef typename traits_type::state_type    state_type;
        
    public:
        // Constructors and Destructors
        explicit basic_NSStreamStreambuf();
#ifndef BOOST_NO_RVALUE_REFERENCES
        basic_NSStreamStreambuf(basic_NSStreamStreambuf&& rhs);
#endif
        virtual ~basic_NSStreamStreambuf();
        
        // Assign and swap:
#ifndef BOOST_NO_RVALUE_REFERENCES
        basic_NSStreamStreambuf& operator=(basic_NSStreamStreambuf&& rhs);
#endif
        void swap(basic_NSStreamStreambuf& rhs);
        
        // 27.9.1.4 Members:
        bool is_open() const;
        basic_NSStreamStreambuf* open(NSInputStream* nsstream);
        basic_NSStreamStreambuf* open(NSOutputStream* nsstream);
        basic_NSStreamStreambuf* close();
        
//        // Opens the external stream.
//        // Returns this on success, otherwise NULL or failure.
//        // If the external stream is already open or parameter nsistream equals
//        // nil, this function immediately fails.
//        // Otherwise, it opens the stream.
//        streambuf_type*
//        open(NSInputStream* nsistream);
//
//        // Closes the external NSInputStream.
//        // If no external stream is open, this function immediately fails.
//        // If a "put buffer area" exists, overflow(eof) is called to flush
//        // all the charatcers. Then, the NSInputStream is closed.
//        streambuf_type*
//        close();

        
    protected:
        // 27.8.1.4 Overridden virtual functions:
        
        // Buffer management and positioning:
        virtual basic_streambuf<char_type, traits_type>* setbuf(char_type* s, streamsize n);
        virtual int sync();
        virtual pos_type seekoff(off_type off, ios_base::seekdir way,
                                 ios_base::openmode which = ios_base::in | ios_base::out);
        virtual pos_type seekpos(pos_type __sp,
                                 ios_base::openmode which = ios_base::in | ios_base::out);
        
        // Input functions (get):
        virtual streamsize showmanyc();
        virtual streamsize xsgetn(char_type* s, streamsize n);
        virtual int_type underflow();
        virtual int_type pbackfail(int_type c = traits_type::eof());
        
        // Output functions (put):
        virtual streamsize xsputn (const char* s, streamsize n);
        virtual int_type overflow (int_type __c = traits_type::eof());
        
        // Locale
        virtual void imbue(const locale& __loc);

    private:
        
        NSStream*       _ns_stream;         // external NSInputStream or NSOutputStream
        
        // Buffer managment for external representation (encoding) of the characters:
        uint8_t*        _ebuf;              // buffer holding characters in external representation
        const uint8_t*  _ebuf_next;         // pointer to next byte in buffer for external representation
        const uint8_t*  _ebuf_end;          // pointer to past the end of buffer for external representation
        uint8_t         _ebuf_min[8];       // auto minimal buffer for external representation
        size_t          _ebuf_size;

        
        // Buffer managment for internal representation of the characters iff a
        // conversion is required (in that case, _always_noconv equals false):
        const codecvt<char_type, char, state_type>* _cv;  // converter between different character encodings
        char_type*      _ibuf;              // buffer for internal representation
        size_t          _ibuf_size;         // size of the buffer for internal representation
        state_type      _cv_state;          // conversion state type used in various conversions processes
        
        ios_base::openmode __om_;
        ios_base::openmode __cm_;

        
        bool _owns_ebuf;
        bool _owns_ibuf;
        bool _always_noconv;
        
        bool __read_mode();
        void __write_mode();
    };
    
    
    typedef basic_NSStreamStreambuf<char> NSStreamStreambuf;
    

    
    
    
    
    
    
    // =========================================================================
    #pragma mark - Implementation
    // =========================================================================
    
    template <class CharT, class Traits>
    basic_NSStreamStreambuf<CharT, Traits>::basic_NSStreamStreambuf()
    :   _ebuf(0),
        _ebuf_next(0),
        _ebuf_end(0),
        _ebuf_size(0),
        _ibuf(0),
        _ibuf_size(0),
        _ns_stream(nil),
        _cv(&use_facet<codecvt<char_type, char, state_type> >(this->getloc())),
        _cv_state(),
        __om_(0),
        __cm_(0),
        _owns_ebuf(false),
        _owns_ibuf(false),
        _always_noconv(_cv->always_noconv())
    {
        setbuf(0, 4096);
    }
    
#ifndef BOOST_NO_RVALUE_REFERENCES
    
    template <class CharT, class Traits>
    basic_NSStreamStreambuf<CharT, Traits>::basic_NSStreamStreambuf(basic_NSStreamStreambuf&& __rhs)
    : basic_streambuf<CharT, Traits>(__rhs)
    {
        if (__rhs._ebuf == __rhs._ebuf_min)
        {
            _ebuf = _ebuf_min;
            _ebuf_next = _ebuf + (__rhs._ebuf_next - __rhs._ebuf);
            _ebuf_end = _ebuf + (__rhs._ebuf_end - __rhs._ebuf);
        }
        else
        {
            _ebuf = __rhs._ebuf;
            _ebuf_next = __rhs._ebuf_next;
            _ebuf_end = __rhs._ebuf_end;
        }
        _ebuf_size = __rhs._ebuf_size;
        _ibuf = __rhs._ibuf;
        _ibuf_size = __rhs._ibuf_size;
        _ns_stream = __rhs._ns_stream;
        _cv = __rhs._cv;
        _cv_state = __rhs._cv_state;
        __om_ = __rhs.__om_;
        __cm_ = __rhs.__cm_;
        _owns_ebuf = __rhs._owns_ebuf;
        _owns_ibuf = __rhs._owns_ibuf;
        _always_noconv = __rhs._always_noconv;
        if (__rhs.pbase())
        {
            if (__rhs.pbase() == __rhs._ibuf)
                this->setp(_ibuf, _ibuf + (__rhs. epptr() - __rhs.pbase()));
            else
                this->setp((char_type*)_ebuf,
                           (char_type*)_ebuf + (__rhs. epptr() - __rhs.pbase()));
            this->pbump(__rhs. pptr() - __rhs.pbase());
        }
        else if (__rhs.eback())
        {
            if (__rhs.eback() == __rhs._ibuf)
                this->setg(_ibuf, _ibuf + (__rhs.gptr() - __rhs.eback()),
                           _ibuf + (__rhs.egptr() - __rhs.eback()));
            else
                this->setg((char_type*)_ebuf,
                           (char_type*)_ebuf + (__rhs.gptr() - __rhs.eback()),
                           (char_type*)_ebuf + (__rhs.egptr() - __rhs.eback()));
        }
        __rhs._ebuf = 0;
        __rhs._ebuf_next = 0;
        __rhs._ebuf_end = 0;
        __rhs._ebuf_size = 0;
        __rhs._ibuf = 0;
        __rhs._ibuf_size = 0;
        __rhs._ns_stream = nil;
        __rhs._cv_state = state_type();
        __rhs.__om_ = 0;
        __rhs.__cm_ = 0;
        __rhs._owns_ebuf = false;
        __rhs._owns_ibuf = false;
        __rhs.setg(0, 0, 0);
        __rhs.setp(0, 0);
    }
    
    template <class CharT, class Traits>
    inline _LIBCPP_INLINE_VISIBILITY
    basic_NSStreamStreambuf<CharT, Traits>&
    basic_NSStreamStreambuf<CharT, Traits>::operator=(basic_NSStreamStreambuf&& __rhs)
    {
        close();
        swap(__rhs);
    }
    
#endif  // BOOST_NO_RVALUE_REFERENCES
    
    template <class CharT, class Traits>
    basic_NSStreamStreambuf<CharT, Traits>::~basic_NSStreamStreambuf()
    {
#ifndef NSSTREAMSTREAMBUF_NO_EXCEPTIONS
        try
        {
#endif  // NSSTREAMSTREAMBUF_NO_EXCEPTIONS
            close();
#ifndef NSSTREAMSTREAMBUF_NO_EXCEPTIONS
        }
        catch (...)
        {
        }
#endif  // NSSTREAMSTREAMBUF_NO_EXCEPTIONS
        if (_owns_ebuf)
            delete [] _ebuf;
        if (_owns_ibuf)
            delete [] _ibuf;
    }
    
    template <class CharT, class Traits>
    void
    basic_NSStreamStreambuf<CharT, Traits>::swap(basic_NSStreamStreambuf& __rhs)
    {
        basic_streambuf<char_type, traits_type>::swap(__rhs);
        if (_ebuf != _ebuf_min && __rhs._ebuf != __rhs._ebuf_min)
        {
            _VSTD::swap(_ebuf, __rhs._ebuf);
            _VSTD::swap(_ebuf_next, __rhs._ebuf_next);
            _VSTD::swap(_ebuf_end, __rhs._ebuf_end);
        }
        else
        {
            ptrdiff_t __ln = _ebuf_next - _ebuf;
            ptrdiff_t __le = _ebuf_end - _ebuf;
            ptrdiff_t __rn = __rhs._ebuf_next - __rhs._ebuf;
            ptrdiff_t __re = __rhs._ebuf_end - __rhs._ebuf;
            if (_ebuf == _ebuf_min && __rhs._ebuf != __rhs._ebuf_min)
            {
                _ebuf = __rhs._ebuf;
                __rhs._ebuf = __rhs._ebuf_min;
            }
            else if (_ebuf != _ebuf_min && __rhs._ebuf == __rhs._ebuf_min)
            {
                __rhs._ebuf = _ebuf;
                _ebuf = _ebuf_min;
            }
            _ebuf_next = _ebuf + __rn;
            _ebuf_end = _ebuf + __re;
            __rhs._ebuf_next = __rhs._ebuf + __ln;
            __rhs._ebuf_end = __rhs._ebuf + __le;
        }
        _VSTD::swap(_ebuf_size, __rhs._ebuf_size);
        _VSTD::swap(_ibuf, __rhs._ibuf);
        _VSTD::swap(_ibuf_size, __rhs._ibuf_size);
        _VSTD::swap(__file_, __rhs.__file_);
        _VSTD::swap(_cv, __rhs._cv);
        _VSTD::swap(_cv_state, __rhs._cv_state);
        _VSTD::swap(__om_, __rhs.__om_);
        _VSTD::swap(__cm_, __rhs.__cm_);
        _VSTD::swap(_owns_ebuf, __rhs._owns_ebuf);
        _VSTD::swap(_owns_ibuf, __rhs._owns_ibuf);
        _VSTD::swap(_always_noconv, __rhs._always_noconv);
        if (this->eback() == (char_type*)__rhs._ebuf_min)
        {
            ptrdiff_t __n = this->gptr() - this->eback();
            ptrdiff_t __e = this->egptr() - this->eback();
            this->setg((char_type*)_ebuf_min,
                       (char_type*)_ebuf_min + __n,
                       (char_type*)_ebuf_min + __e);
        }
        else if (this->pbase() == (char_type*)__rhs._ebuf_min)
        {
            ptrdiff_t __n = this->pptr() - this->pbase();
            ptrdiff_t __e = this->epptr() - this->pbase();
            this->setp((char_type*)_ebuf_min,
                       (char_type*)_ebuf_min + __e);
            this->pbump(__n);
        }
        if (__rhs.eback() == (char_type*)_ebuf_min)
        {
            ptrdiff_t __n = __rhs.gptr() - __rhs.eback();
            ptrdiff_t __e = __rhs.egptr() - __rhs.eback();
            __rhs.setg((char_type*)__rhs._ebuf_min,
                       (char_type*)__rhs._ebuf_min + __n,
                       (char_type*)__rhs._ebuf_min + __e);
        }
        else if (__rhs.pbase() == (char_type*)_ebuf_min)
        {
            ptrdiff_t __n = __rhs.pptr() - __rhs.pbase();
            ptrdiff_t __e = __rhs.epptr() - __rhs.pbase();
            __rhs.setp((char_type*)__rhs._ebuf_min,
                       (char_type*)__rhs._ebuf_min + __e);
            __rhs.pbump(__n);
        }
    }
    
    template <class CharT, class Traits>
    inline _LIBCPP_INLINE_VISIBILITY
    void
    swap(basic_NSStreamStreambuf<CharT, Traits>& __x, basic_NSStreamStreambuf<CharT, Traits>& __y)
    {
        __x.swap(__y);
    }
    
    template <class CharT, class Traits>
    inline _LIBCPP_INLINE_VISIBILITY
    bool
    basic_NSStreamStreambuf<CharT, Traits>::is_open() const
    {
        return __file_ != 0;
    }
    
    template <class CharT, class Traits>
    basic_NSStreamStreambuf<CharT, Traits>*
    basic_NSStreamStreambuf<CharT, Traits>::open(const char* __s, ios_base::openmode __mode)
    {
        basic_NSStreamStreambuf<CharT, Traits>* __rt = 0;
        if (__file_ == 0)
        {
            __rt = this;
            const char* __mdstr;
            switch (__mode & ~ios_base::ate)
            {
                case ios_base::out:
                case ios_base::out | ios_base::trunc:
                    __mdstr = "w";
                    break;
                case ios_base::out | ios_base::app:
                case ios_base::app:
                    __mdstr = "a";
                    break;
                case ios_base::in:
                    __mdstr = "r";
                    break;
                case ios_base::in | ios_base::out:
                    __mdstr = "r+";
                    break;
                case ios_base::in | ios_base::out | ios_base::trunc:
                    __mdstr = "w+";
                    break;
                case ios_base::in | ios_base::out | ios_base::app:
                case ios_base::in | ios_base::app:
                    __mdstr = "a+";
                    break;
                case ios_base::out | ios_base::binary:
                case ios_base::out | ios_base::trunc | ios_base::binary:
                    __mdstr = "wb";
                    break;
                case ios_base::out | ios_base::app | ios_base::binary:
                case ios_base::app | ios_base::binary:
                    __mdstr = "ab";
                    break;
                case ios_base::in | ios_base::binary:
                    __mdstr = "rb";
                    break;
                case ios_base::in | ios_base::out | ios_base::binary:
                    __mdstr = "r+b";
                    break;
                case ios_base::in | ios_base::out | ios_base::trunc | ios_base::binary:
                    __mdstr = "w+b";
                    break;
                case ios_base::in | ios_base::out | ios_base::app | ios_base::binary:
                case ios_base::in | ios_base::app | ios_base::binary:
                    __mdstr = "a+b";
                    break;
                default:
                    __rt = 0;
                    break;
            }
            if (__rt)
            {
                __file_ = fopen(__s, __mdstr);
                if (__file_)
                {
                    __om_ = __mode;
                    if (__mode & ios_base::ate)
                    {
                        if (fseek(__file_, 0, SEEK_END))
                        {
                            fclose(__file_);
                            __file_ = 0;
                            __rt = 0;
                        }
                    }
                }
                else
                    __rt = 0;
            }
        }
        return __rt;
    }
    
    template <class CharT, class Traits>
    inline _LIBCPP_INLINE_VISIBILITY
    basic_NSStreamStreambuf<CharT, Traits>*
    basic_NSStreamStreambuf<CharT, Traits>::open(const string& __s, ios_base::openmode __mode)
    {
        return open(__s.c_str(), __mode);
    }
    
    template <class CharT, class Traits>
    basic_NSStreamStreambuf<CharT, Traits>*
    basic_NSStreamStreambuf<CharT, Traits>::close()
    {
        basic_NSStreamStreambuf<CharT, Traits>* __rt = 0;
        if (__file_)
        {
            __rt = this;
            unique_ptr<FILE, int(*)(FILE*)> __h(__file_, fclose);
            if (sync())
                __rt = 0;
            if (fclose(__h.release()) == 0)
                __file_ = 0;
            else
                __rt = 0;
        }
        return __rt;
    }
    
    template <class CharT, class Traits>
    typename basic_NSStreamStreambuf<CharT, Traits>::int_type
    basic_NSStreamStreambuf<CharT, Traits>::underflow()
    {
        if (__file_ == 0)
            return traits_type::eof();
        bool __initial = __read_mode();
        char_type __1buf;
        if (this->gptr() == 0)
            this->setg(&__1buf, &__1buf+1, &__1buf+1);
        const size_t __unget_sz = __initial ? 0 : min<size_t>((this->egptr() - this->eback()) / 2, 4);
        int_type __c = traits_type::eof();
        if (this->gptr() == this->egptr())
        {
            memmove(this->eback(), this->egptr() - __unget_sz, __unget_sz * sizeof(char_type));
            if (_always_noconv)
            {
                size_t __nmemb = static_cast<size_t>(this->egptr() - this->eback() - __unget_sz);
                __nmemb = fread(this->eback() + __unget_sz, 1, __nmemb, __file_);
                if (__nmemb != 0)
                {
                    this->setg(this->eback(),
                               this->eback() + __unget_sz,
                               this->eback() + __unget_sz + __nmemb);
                    __c = traits_type::to_int_type(*this->gptr());
                }
            }
            else
            {
                memmove(_ebuf, _ebuf_next, _ebuf_end - _ebuf_next);
                _ebuf_next = _ebuf + (_ebuf_end - _ebuf_next);
                _ebuf_end = _ebuf + (_ebuf == _ebuf_min ? sizeof(_ebuf_min) : _ebuf_size);
                size_t __nmemb = _VSTD::min(static_cast<size_t>(this->egptr() - this->eback() - __unget_sz),
                                            static_cast<size_t>(_ebuf_end - _ebuf_next));
                codecvt_base::result __r;
                state_type __svs = _cv_state;
                size_t __nr = fread((void*)_ebuf_next, 1, __nmemb, __file_);
                if (__nr != 0)
                {
                    _ebuf_end = _ebuf_next + __nr;
                    char_type*  __inext;
                    __r = _cv->in(_cv_state, _ebuf, _ebuf_end, _ebuf_next,
                                    this->eback() + __unget_sz,
                                    this->egptr(), __inext);
                    if (__r == codecvt_base::noconv)
                    {
                        this->setg((char_type*)_ebuf, (char_type*)_ebuf, (char_type*)_ebuf_end);
                        __c = traits_type::to_int_type(*this->gptr());
                    }
                    else if (__inext != this->eback() + __unget_sz)
                    {
                        this->setg(this->eback(), this->eback() + __unget_sz, __inext);
                        __c = traits_type::to_int_type(*this->gptr());
                    }
                }
            }
        }
        else
            __c = traits_type::to_int_type(*this->gptr());
        if (this->eback() == &__1buf)
            this->setg(0, 0, 0);
        return __c;
    }
    
    template <class CharT, class Traits>
    typename basic_NSStreamStreambuf<CharT, Traits>::int_type
    basic_NSStreamStreambuf<CharT, Traits>::pbackfail(int_type __c)
    {
        if (__file_ && this->eback() < this->gptr())
        {
            if (traits_type::eq_int_type(__c, traits_type::eof()))
            {
                this->gbump(-1);
                return traits_type::not_eof(__c);
            }
            if ((__om_ & ios_base::out) ||
                traits_type::eq(traits_type::to_char_type(__c), this->gptr()[-1]))
            {
                this->gbump(-1);
                *this->gptr() = traits_type::to_char_type(__c);
                return __c;
            }
        }
        return traits_type::eof();
    }
    
    template <class CharT, class Traits>
    typename basic_NSStreamStreambuf<CharT, Traits>::int_type
    basic_NSStreamStreambuf<CharT, Traits>::overflow(int_type __c)
    {
        if (__file_ == 0)
            return traits_type::eof();
        __write_mode();
        char_type __1buf;
        char_type* __pb_save = this->pbase();
        char_type* __epb_save = this->epptr();
        if (!traits_type::eq_int_type(__c, traits_type::eof()))
        {
            if (this->pptr() == 0)
                this->setp(&__1buf, &__1buf+1);
            *this->pptr() = traits_type::to_char_type(__c);
            this->pbump(1);
        }
        if (this->pptr() != this->pbase())
        {
            if (_always_noconv)
            {
                size_t __nmemb = static_cast<size_t>(this->pptr() - this->pbase());
                if (fwrite(this->pbase(), sizeof(char_type), __nmemb, __file_) != __nmemb)
                    return traits_type::eof();
            }
            else
            {
                char* __extbe = _ebuf;
                codecvt_base::result __r;
                do
                {
                    const char_type* __e;
                    __r = _cv->out(_cv_state, this->pbase(), this->pptr(), __e,
                                     _ebuf, _ebuf + _ebuf_size, __extbe);
                    if (__e == this->pbase())
                        return traits_type::eof();
                    if (__r == codecvt_base::noconv)
                    {
                        size_t __nmemb = static_cast<size_t>(this->pptr() - this->pbase());
                        if (fwrite(this->pbase(), 1, __nmemb, __file_) != __nmemb)
                            return traits_type::eof();
                    }
                    else if (__r == codecvt_base::ok || __r == codecvt_base::partial)
                    {
                        size_t __nmemb = static_cast<size_t>(__extbe - _ebuf);
                        if (fwrite(_ebuf, 1, __nmemb, __file_) != __nmemb)
                            return traits_type::eof();
                        if (__r == codecvt_base::partial)
                        {
                            this->setp((char_type*)__e, this->pptr());
                            this->pbump(this->epptr() - this->pbase());
                        }
                    }
                    else
                        return traits_type::eof();
                } while (__r == codecvt_base::partial);
            }
            this->setp(__pb_save, __epb_save);
        }
        return traits_type::not_eof(__c);
    }
    
    template <class CharT, class Traits>
    basic_streambuf<CharT, Traits>*
    basic_NSStreamStreambuf<CharT, Traits>::setbuf(char_type* __s, streamsize __n)
    {
        this->setg(0, 0, 0);
        this->setp(0, 0);
        if (_owns_ebuf)
            delete [] _ebuf;
        if (_owns_ibuf)
            delete [] _ibuf;
        _ebuf_size = __n;
        if (_ebuf_size > sizeof(_ebuf_min))
        {
            if (_always_noconv && __s)
            {
                _ebuf = (char*)__s;
                _owns_ebuf = false;
            }
            else
            {
                _ebuf = new char[_ebuf_size];
                _owns_ebuf = true;
            }
        }
        else
        {
            _ebuf = _ebuf_min;
            _ebuf_size = sizeof(_ebuf_min);
            _owns_ebuf = false;
        }
        if (!_always_noconv)
        {
            _ibuf_size = max<streamsize>(__n, sizeof(_ebuf_min));
            if (__s && _ibuf_size >= sizeof(_ebuf_min))
            {
                _ibuf = __s;
                _owns_ibuf = false;
            }
            else
            {
                _ibuf = new char_type[_ibuf_size];
                _owns_ibuf = true;
            }
        }
        else
        {
            _ibuf_size = 0;
            _ibuf = 0;
            _owns_ibuf = false;
        }
        return this;
    }
    
    template <class CharT, class Traits>
    typename basic_NSStreamStreambuf<CharT, Traits>::pos_type
    basic_NSStreamStreambuf<CharT, Traits>::seekoff(off_type __off, ios_base::seekdir __way,
                                            ios_base::openmode)
    {
        int __width = _cv->encoding();
        if (__file_ == 0 || (__width <= 0 && __off != 0) || sync())
            return pos_type(off_type(-1));
        // __width > 0 || __off == 0
        int __whence;
        switch (__way)
        {
            case ios_base::beg:
                __whence = SEEK_SET;
                break;
            case ios_base::cur:
                __whence = SEEK_CUR;
                break;
            case ios_base::end:
                __whence = SEEK_END;
                break;
            default:
                return pos_type(off_type(-1));
        }
        if (fseeko(__file_, __width > 0 ? __width * __off : 0, __whence))
            return pos_type(off_type(-1));
        pos_type __r = ftello(__file_);
        __r.state(_cv_state);
        return __r;
    }
    
    template <class CharT, class Traits>
    typename basic_NSStreamStreambuf<CharT, Traits>::pos_type
    basic_NSStreamStreambuf<CharT, Traits>::seekpos(pos_type __sp, ios_base::openmode)
    {
        if (__file_ == 0 || sync())
            return pos_type(off_type(-1));
        if (fseeko(__file_, __sp, SEEK_SET))
            return pos_type(off_type(-1));
        return __sp;
    }
    
    template <class CharT, class Traits>
    int
    basic_NSStreamStreambuf<CharT, Traits>::sync()
    {
        if (__file_ == 0)
            return 0;
        if (__cm_ & ios_base::out)
        {
            if (this->pptr() != this->pbase())
                if (overflow() == traits_type::eof())
                    return -1;
            codecvt_base::result __r;
            do
            {
                char* __extbe;
                __r = _cv->unshift(_cv_state, _ebuf, _ebuf + _ebuf_size, __extbe);
                size_t __nmemb = static_cast<size_t>(__extbe - _ebuf);
                if (fwrite(_ebuf, 1, __nmemb, __file_) != __nmemb)
                    return -1;
            } while (__r == codecvt_base::partial);
            if (__r == codecvt_base::error)
                return -1;
            if (fflush(__file_))
                return -1;
        }
        else if (__cm_ & ios_base::in)
        {
            off_type __c;
            if (_always_noconv)
                __c = this->egptr() - this->gptr();
            else
            {
                int __width = _cv->encoding();
                __c = _ebuf_end - _ebuf_next;
                if (__width > 0)
                    __c += __width * (this->egptr() - this->gptr());
                else
                {
                    if (this->gptr() != this->egptr())
                    {
                        reverse(this->gptr(), this->egptr());
                        codecvt_base::result __r;
                        const char_type* __e = this->gptr();
                        char* __extbe;
                        do
                        {
                            __r = _cv->out(_cv_state, __e, this->egptr(), __e,
                                             _ebuf, _ebuf + _ebuf_size, __extbe);
                            switch (__r)
                            {
                                case codecvt_base::noconv:
                                    __c += this->egptr() - this->gptr();
                                    break;
                                case codecvt_base::ok:
                                case codecvt_base::partial:
                                    __c += __extbe - _ebuf;
                                    break;
                                default:
                                    return -1;
                            }
                        } while (__r == codecvt_base::partial);
                    }
                }
            }
            if (fseeko(__file_, -__c, SEEK_CUR))
                return -1;
            this->setg(0, 0, 0);
            __cm_ = 0;
        }
        return 0;
    }
    
    template <class CharT, class Traits>
    void
    basic_NSStreamStreambuf<CharT, Traits>::imbue(const locale& __loc)
    {
        sync();
        _cv = &use_facet<codecvt<char_type, char, state_type> >(__loc);
        bool __old_anc = _always_noconv;
        _always_noconv = _cv->always_noconv();
        if (__old_anc != _always_noconv)
        {
            this->setg(0, 0, 0);
            this->setp(0, 0);
            // invariant, char_type is char, else we couldn't get here
            if (_always_noconv)  // need to dump _ibuf
            {
                if (_owns_ebuf)
                    delete [] _ebuf;
                _owns_ebuf = _owns_ibuf;
                _ebuf_size = _ibuf_size;
                _ebuf = (char*)_ibuf;
                _ibuf_size = 0;
                _ibuf = 0;
                _owns_ibuf = false;
            }
            else  // need to obtain an _ibuf.
            {     // If _ebuf is user-supplied, use it, else new _ibuf
                if (!_owns_ebuf && _ebuf != _ebuf_min)
                {
                    _ibuf_size = _ebuf_size;
                    _ibuf = (char_type*)_ebuf;
                    _owns_ibuf = false;
                    _ebuf = new char[_ebuf_size];
                    _owns_ebuf = true;
                }
                else
                {
                    _ibuf_size = _ebuf_size;
                    _ibuf = new char_type[_ibuf_size];
                    _owns_ibuf = true;
                }
            }
        }
    }
    
    template <class CharT, class Traits>
    bool
    basic_NSStreamStreambuf<CharT, Traits>::__read_mode()
    {
        if (!(__cm_ & ios_base::in))
        {
            this->setp(0, 0);
            if (_always_noconv)
                this->setg((char_type*)_ebuf,
                           (char_type*)_ebuf + _ebuf_size,
                           (char_type*)_ebuf + _ebuf_size);
            else
                this->setg(_ibuf, _ibuf + _ibuf_size, _ibuf + _ibuf_size);
            __cm_ = ios_base::in;
            return true;
        }
        return false;
    }
    
    template <class CharT, class Traits>
    void
    basic_NSStreamStreambuf<CharT, Traits>::__write_mode()
    {
        if (!(__cm_ & ios_base::out))
        {
            this->setg(0, 0, 0);
            if (_ebuf_size > sizeof(_ebuf_min))
            {
                if (_always_noconv)
                    this->setp((char_type*)_ebuf,
                               (char_type*)_ebuf + (_ebuf_size - 1));
                else
                    this->setp(_ibuf, _ibuf + (_ibuf_size - 1));
            }
            else
                this->setp(0, 0);
            __cm_ = ios_base::out;
        }
    }

    
}}   // namespace json::objc





#endif  // JSON_OBJC_NSSTREAM_STREAMBUF_HPP

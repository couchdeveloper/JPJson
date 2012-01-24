//
//  istreambuf_iterator.hpp
//  
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

#ifndef JSON_UTILITY_ISTREAMBUF_ITERATOR_HPP
#define JSON_UTILITY_ISTREAMBUF_ITERATOR_HPP


#include "json/config.hpp"
#include <iterator>


namespace json { namespace utility { 
    
    
        
        // This is a faster version of a std::istreambuf_iterator which is
        // found in gcc's c++stdlib. The one in LLVM's c++lib is fast as well.
        
        template <class charT=char, class traits=std::char_traits<charT> >
        class istreambuf_iterator :
        public std::iterator<std::input_iterator_tag, charT,
        typename traits::off_type, charT*, charT&>
        {
        public:
            typedef charT                               char_type;
            typedef traits                              traits_type;
            typedef typename traits::int_type           int_type;
            typedef std::basic_streambuf<charT,traits>  streambuf_type;
            typedef std::basic_istream<charT,traits>    istream_type;
            
            class proxy {
                charT keep_; streambuf_type* sbuf_;
            public:
                proxy (charT c, streambuf_type* sbuf) : keep_(c), sbuf_(sbuf) { }
                charT operator*() {return keep_;}
            };
            
            istreambuf_iterator() throw() : sbuf_(0) { }
            istreambuf_iterator(istream_type& s) throw(): sbuf_(s.rdbuf()) { }
            istreambuf_iterator(streambuf_type* s) throw(): sbuf_(s) { }
            istreambuf_iterator(const proxy& p) throw(): sbuf_(p.sbuf_) { }
            
            charT operator*() const { return traits_type::to_char_type (sbuf_->sgetc()); }
            
            istreambuf_iterator& operator++()
            {
                if (traits_type::eq_int_type(sbuf_->snextc(), traits_type::eof()))
                    sbuf_ = 0;
                return *this;
            }
            proxy operator++(int)
            {
                char_type c = sbuf_->sgetc();
                ++(*this);
                return proxy(c, sbuf_);
            }
                        
            
            bool equal(istreambuf_iterator const& other) const 
            {
                const int_type k_eof = traits_type::eof();
                const bool eof_this = !sbuf_ or traits_type::eq_int_type(sbuf_->sgetc(), k_eof);                
                const bool eof_other = !other.sbuf_  or traits_type::eq_int_type(other.sbuf_->sgetc(), k_eof);                
                return eof_this == eof_other;
            }
            
            friend inline 
            bool        
            operator== (const istreambuf_iterator& lhv, const istreambuf_iterator& rhv) {
                return lhv.equal(rhv);
            }
            friend inline 
            bool        
            operator!= (const istreambuf_iterator& lhv, const istreambuf_iterator& rhv) {
                return not lhv.equal(rhv);
            }
            
        protected:
            /*
            bool eof() const { 
                return sbuf_ != 0; 
            }
             */
            
        private:
            streambuf_type* sbuf_;
        };
    
    

}}


#endif

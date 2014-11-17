//
//  syncqueue_streambuf.hpp
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

#ifndef JSON_UTILITY_SYNCQUEUE_STREAMBUF_HPP
#define JSON_UTILITY_SYNCQUEUE_STREAMBUF_HPP


#include "json/config.hpp"
#include "synchronous_queue.hpp"
#include <streambuf>
#include <ios>
#include <utility>
#include <stdint.h>


namespace json { namespace utility {
    
    //
    // basic_syncqueue_streambuf  
    //
    // basic_syncqueue_streambuf implements the abstract std::basic_streambuf
    // for use in conjunction with a synchrounous queue, whose value_type
    // models the buffer type concept. The buffer's value_type shall be a
    // char type.

    template <typename BufferT, typename CharT, typename TraitsT = std::char_traits<CharT> >
    class basic_syncqueue_streambuf : public std::basic_streambuf<CharT, TraitsT>
    {
        static_assert(sizeof(typename BufferT::value_type) == 1, "" );
        typedef std::basic_streambuf<CharT, TraitsT>    base_type;
    public:
        typedef TraitsT                                 traits_type;
        typedef typename base_type::int_type            int_type;
        typedef std::size_t                             streampos;
        typedef typename base_type::off_type            off_type;
        typedef CharT                                   char_type;
        
        typedef synchronous_queue<BufferT>              queue_type;
        typedef typename queue_type::value_type         queue_buffer_type;
        typedef typename queue_buffer_type::value_type  qb_char_type;
        
        
        
    private:
        typedef std::pair<typename queue_type::result_type, queue_buffer_type> acquire_result_t;

		// noncopyable
		basic_syncqueue_streambuf(const basic_syncqueue_streambuf&);
		void operator=(const basic_syncqueue_streambuf&);
        
    public:
        explicit basic_syncqueue_streambuf(queue_type& queue, double timeout = -1)
        : base_type(), queue_ptr_(&queue), base_pos_(0), timeout_(timeout), timeout_occured_(false)
        {
        }
        
        // Sets a sync queue and a buffer. It is assumed, the buffer has been
        // retrieved by a previous call to the get() member function of the
        // queue.
        explicit basic_syncqueue_streambuf(queue_type& queue, queue_buffer_type& buffer, double timeout = -1)
        : base_type(), queue_ptr_(&queue), base_pos_(0), buffer_(buffer), timeout_(timeout), timeout_occured_(false)
        {
            set_queue_buffer();
        }
        
        ~basic_syncqueue_streambuf() {
            if (this->eback()) {
                //assert(queue_ptr_->empty() == false);
                //queue_ptr_->commit();
            }
        }
        
        queue_buffer_type buffer() const {
            return buffer_;
        }
        
        bool timeout_occured() const { return timeout_occured_; }
        
        std::ios::streampos 
        base_pos() const { return base_pos_; }        
        
    protected:
        int_type overflow(int_type c = traits_type::eof()) {
            return EOF;
        }
        
        
        void set_queue_buffer() {
            // buffer_.data() returns a pointer to char. The bytes may
            // be missaligned for a CharT and the buffer may even contain a 
            // partial CharT at the and of the buffer range. In this case, we 
            // do NOT handle this case gracefully, but throw an exception. 
            // This of course requires a precondition that the synchronous 
            // buffer always delivers bytes alligned appropriately for CharT.
            // 
            // (Note: an optimizing compiler should remove all unneccesary
            // switch cases)
            void* start = const_cast<void*>(reinterpret_cast<void const*>(buffer_.data()));
            void* end = const_cast<void*>(reinterpret_cast<void const*>(buffer_.data() + buffer_.size()));
            switch (sizeof(CharT)) {
                case 1:break;
                case 2: {
                    if ((reinterpret_cast<uintptr_t>(start) & 0x01u) !=0 or (reinterpret_cast<uintptr_t>(end) & 0x01u) != 0)
                        throwMissalignedBuffer();
                }
                    break;
                case 4: {
                    if ((reinterpret_cast<uintptr_t>(start) & 0x03u) !=0 or (reinterpret_cast<uintptr_t>(end) & 0x03u) != 0)
                        throwMissalignedBuffer();
                }
                    break;
                default:
                    if ((reinterpret_cast<uintptr_t>(start) % sizeof(CharT)) !=0 or (reinterpret_cast<uintptr_t>(end) % sizeof(CharT)) != 0)
                        throwMissalignedBuffer();
            }
            
            this->setg(reinterpret_cast<CharT*>(start), 
                       reinterpret_cast<CharT*>(start), 
                       reinterpret_cast<CharT*>(end));
        }
        
                
        int_type underflow() {
            if (this->eback()) {
                base_pos_ += buffer_.size();
                //queue_ptr_->commit();
                this->setg(0,0,0);
            }
            //acquire_result_t r = queue_ptr_->acquire(timeout_);
            acquire_result_t r = queue_ptr_->get(timeout_);
            if (r.first != queue_type::OK) {
                timeout_occured_ = true;
                return EOF;  // timeout
            }
            else {
                buffer_ = std::move(r.second);
            }
            if (buffer_.data() == NULL or buffer_.size() == 0) {
                //queue_ptr_->commit();
                return EOF;
            }
            set_queue_buffer();
            
            return traits_type::to_int_type(*(this->eback()));
        }
        
        std::ios_base::streampos 
        seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in)
        {
            off_type curpos = base_pos_ + (this->gptr() - this->eback()); 
            off_type newpos = off;

            if (way == std::ios::cur) {
                newpos += curpos;
            } else if (way == std::ios::end) {
                newpos += buffer_.size();
            }
            if (newpos == curpos) {
                return curpos;
            }
            
            off_type endpos = base_pos_ + (this->egptr() - this->eback());

            // we can change position only within the range of the current
            // buffer, otherwise we return an invalid streampos (-1):
            // If we point beyond the current buffer:
            if (newpos > endpos) {
                return streampos(-1);
            }
            // if we point before the current buffer:
            else if (newpos < base_pos_) {
                return streampos(-1);
            }
            
            this->setg(this->eback(), this->eback() + (newpos - base_pos_), this->egptr());
            return newpos;
        }
        
        std::ios_base::streampos 
        seekpos(streampos sp, std::ios_base::openmode which = std::ios_base::in)
        {
            return this->seekoff(sp, std::ios::beg);
        }
        
    private:        
        void throwMissalignedBuffer() const {
            throw std::runtime_error("basic_syncqueue_streambuf: missaligned buffer");
        }
        
    private:
        queue_type*         queue_ptr_;
        queue_buffer_type   buffer_;
        streampos           base_pos_;
        double              timeout_;
        bool                timeout_occured_;
    };


        
    
}}

#endif

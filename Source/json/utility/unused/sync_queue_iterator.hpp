//
//  sync_queue_iterator.hpp
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

#error Unused

#ifndef JSON_SYNC_QUEUE_ITERATOR_HPP
#define JSON_SYNC_QUEUE_ITERATOR_HPP


#include <boost/config.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <iostream>
#include <assert.h>


namespace json { 
    
    //
    //    SyncQueueT::buffer_type requirements
    //
    //    struct buffer : copyable, defaultconstructable {
    //
    //        typedef uint8_t         value_type;
    //        typedef uint8_t*        pointer;
    //        typedef uint8_t const*  const_pointer;
    //        
    //        // Returns the size of the buffer in bytes
    //        std::size_t     size() const;
    //        
    //        // Returns a pointer to the byte array
    //        const_pointer   data() const;
    //    };
    
    template<typename T, typename SyncQueueT, int S = sizeof(T) >
    class sync_queue_iterator: public boost::iterator_facade<
        sync_queue_iterator<T, SyncQueueT, S>,          // Derived
        T const,                                        // Value
        boost::single_pass_traversal_tag                // CategoryOrTraversal
    >
    {
        typedef SyncQueueT                              queue_type;
        typedef typename SyncQueueT::value_type         buffer_type;     
        typedef typename buffer_type::const_pointer     const_data_pointer;
        typedef typename buffer_type::pointer           data_pointer;
        
    public:
        
        // The default constructed instance represents EOF.
        sync_queue_iterator()
        : p_(0), sync_queue_(0)
        {
            // If p_ equals zero, it indicates this is eof.
        }
        
        sync_queue_iterator(SyncQueueT& queue) 
        : p_(0), sync_queue_(&queue), data_start_(0), data_end_(0)
        {
            // The member p_ is solely compared to check for EOF. When
            // p_ equals NULL we indicate EOF. When constructing a buffers 
            // instance we need to consume at least one buffer in order to 
            // disambiguate a buffers instance which did not yet consume at 
            // least one buffer from one which received EOF.
            // 
            use_next();
        }
        
    private:
        
        // Returns the number of bytes available in the buffer.
        // Returns zero on eof or if a timeout occured.
        size_t recv() 
        {
#if defined (DEBUG)            
            assert(data_start_ == data_end_); // we shall consume all available bytes first!
#endif            
            
            std::pair<typename queue_type::result_type, buffer_type> result = sync_queue_->acquire(300);  // TODO: timeout 300 secs
            
            if (result.first != queue_type::OK) {
                // timeout! 
                sync_queue_ = 0;
                p_ = 0;
                data_start_ = data_end_ = 0;
                std::cerr << "ERROR: sync_queue_iterator: timeout expired while trying to consume buffer\n";
                return 0;
            } 
            
            data_start_ = result.second.data();
            data_end_ = data_start_ ?  data_start_ + result.second.size() : NULL;
            if (data_start_ == data_end_) {
                // eof
                sync_queue_->commit();
                p_ = 0;
                sync_queue_ = 0;
                data_start_ = data_end_ = 0;  //make clear, that we no not need a commit
                return 0;
            } else {
                return result.second.size(); 
            }
        }
        
    public:
        
        // If use next returns false, eof has been reached, or a timeout
        // has expired.
        bool use_next() 
        {
            if (sync_queue_ == 0)  // means eof reached, or iterator equals the EOF-iterator
                return false;  

            // When we reach here, we make the following assertions:
            // 1) This is not the EOF iterator.
            // 2) If this is the first call, p_ equals 0 - and eof is undefined.
            //    Otherwise, eof has not been reached and p_ points to 
            //    buffer_end_.
            // 3) If this is the first call, only sync_queue_ and p_ has been
            //    initialized - other member variables are not yet initialized.
            // 5) We never reach here, if eof was reached.
            // 6) data_start_ is aligned to unit boundaries. Therefore, it 
            //    points to the start of a unit, or past the end of the last 
            //    unit. That is, data_start_ will be incremented  only in steps
            //    of sizeof(T).
            // 7) temp buffer is either full or empty
            // 8) data_start_ may equal data_end_
            
            if (p_ == 0 and recv() == 0) {
                // eof or timeout
                return false;
            }
            
            while (1) 
            {
                if (data_start_ != data_end_) {
                    // There are bytes available in data.
                    // If enough bytes are available to construct one unit,
                    // directly use the data buffer, otherwise copy the fist
                    // partial bytes to the tmp_buffer_:
                    size_t num_units = (data_end_ - data_start_) / sizeof(T);
                    if (num_units > 0) {
                        // One or more units available in data.
                        p_ = reinterpret_cast<T const*>(data_start_);
                        buffer_end_ = p_ + num_units;
                        data_start_ += num_units*sizeof(T);
                        return true;
                    } 
                    else 
                    {
                        // Not enough bytes available to construct one unit.
                        // Copy the remaining bytes to the temp buffer, and 
                        // continue read new buffers until we have enough:
                        size_t countBytes = data_end_ - data_start_;
                        memcpy(tmp_buffer_, data_start_, countBytes);
                        sync_queue_->commit(); // everything consumed, commit the buffer
                        data_start_ = data_end_ = 0;
                        while (countBytes < sizeof(T)) {
                            size_t bytesAvail;
                            if ((bytesAvail = recv()) == 0) {
                                // timeout or eof;
                                return false;
                            }
                            int bytesConsumed = (int)std::min(bytesAvail, sizeof(T) - countBytes);
                            memcpy(tmp_buffer_ + countBytes, data_start_, bytesConsumed);
                            data_start_ += bytesConsumed;
                            countBytes += bytesConsumed;
                            if (data_start_ == data_end_) {
                                sync_queue_->commit();
                                data_start_ = data_end_ = 0;
                            }
                        }
                        // buffer is filled
                        p_ = reinterpret_cast<T const*>(tmp_buffer_);
                        buffer_end_ = p_ + 1;
                        return true;
                    }
                }
                else {
                    // Last byte has been consumed.
                    // Commit required?
                    if (data_start_) {
                        sync_queue_->commit();
                        data_start_ = data_end_ = 0;
                    }
                                        
                    // Need to read new data buffer which will start at a 
                    // unit boundary.
                    if (recv() == 0) {
                        return false;
                    }
                }
            }
        }
        
        void increment() { 
            assert(p_ != 0);
            if (++p_ == buffer_end_) {
                use_next();
            }
        }
        
        bool equal(sync_queue_iterator const& other) const {
            return p_ == other.p_;
        }
        
        T const& dereference() const { 
            assert(p_ != 0);
            return *p_; 
        }
        
        
    private:
        friend class boost::iterator_core_access;
        const_data_pointer  data_start_;
        const_data_pointer  data_end_;
        T const*            buffer_end_;
        T const*            p_;
        queue_type*         sync_queue_;
        uint8_t             tmp_buffer_[sizeof(T)];
    };
    
    
}  // namespace json



#endif

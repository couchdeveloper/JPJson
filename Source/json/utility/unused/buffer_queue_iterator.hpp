#ifndef JSON_BUFFER_QUEUE_ITERATOR_HPP
#define JSON_BUFFER_QUEUE_ITERATOR_HPP

//
//  buffer_queue_iterator.hpp
//
//  Created by Andreas Grosam on 6/24/11.
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

#include <boost/config.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_traits.hpp>
    
    
    

namespace json { 
    
    template<typename T, typename BuffersT>
    class buffer_queue_iterator:
        public boost::iterator_facade<
            buffer_queue_iterator<T, BuffersT>,   // Derived
            T const,                              // Value
            boost::single_pass_traversal_tag      // CategoryOrTraversal
        >
    {
        typedef BuffersT                        buffers_type;
        typedef typename BuffersT::buffer_type  buffer_type;        
        
    public:
        
        // The default constructed instance represents EOF.
        buffer_queue_iterator()
        : buffersPtr_(0), bufferPtr_(0), dataPtr_(0), dataBackPtr_(0)
        {
            // If buffersPtr_ equals zero, it indicates this is eof.
        }
        
        buffer_queue_iterator(BuffersT& buffers) 
        : buffersPtr_(&buffers), bufferPtr_(0), dataPtr_(0), dataBackPtr_(0)
        {
            // The member dataPtr_ is solely compared to check for EOF. When
            // dataPtr_ equals NULL we indicate EOF. When constructing a buffers 
            // instance we need to consume at least one buffer in order to 
            // disambiguate a buffers instance which did not yet consume at least
            // one buffer from one which received EOF.
            // 
            use_next();
        }
        
    public:
        
        void use_next() {
            if (buffersPtr_ == 0)
                return;
            if (bufferPtr_)
                buffersPtr_->commit(bufferPtr_);
            
            bufferPtr_ = 0;
            buffer_type* result = buffersPtr_->aquire(300);  // TODO: timeout 300 secs
            if (result) {
                bufferPtr_ = result;
                dataPtr_ = bufferPtr_->data();
                // If buffer is a NULL buffer, and size equals zero, we treat this as EOF.
                // If EOF was received, we set bufferPtr_ to NULL indicating that we reached EOF.
                if (dataPtr_ and bufferPtr_->size() > 0 )
                    dataBackPtr_ = dataPtr_ + bufferPtr_->size() - 1;
                else {
                    // size of buffer is zero, or data pointer equals NULL: got EOF
                    buffersPtr_->commit(bufferPtr_);
                    bufferPtr_ = 0;
                    buffersPtr_ = 0;  // prevent from blocking when incrementing an iterator which equals EOF
                    dataBackPtr_ = dataPtr_ = 0;
                }
                // note dataBackPtr_ is not past the end, actually it points to the last valid element.
            }
            else {
                printf("ERROR: buffer_queue_iterator: timeout expired while trying to consume buffer\n");
                dataPtr_ = dataBackPtr_ = 0;
                bufferPtr_ = 0;
                buffersPtr_ = 0;
            }            
        }
        
        void increment() { 
            if (dataPtr_ != dataBackPtr_) {
                ++dataPtr_;
            }
            else { 
                use_next();
            }
        }
        
        bool equal(buffer_queue_iterator const& other) const {
            return dataPtr_ == other.dataPtr_;
        }
        
        T const& dereference() const { return *dataPtr_; }
        
        
    private:
        friend class boost::iterator_core_access;
        
        T const*        dataPtr_;
        T const*        dataBackPtr_;
        buffers_type*   buffersPtr_;
        buffer_type*    bufferPtr_;
    };

    
}  // namespace json


#endif // JSON_BUFFER_QUEUE_ITERATOR_HPP


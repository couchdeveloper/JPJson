//
//  SemanticActionsMD.hpp
//
//  Created by Andreas Grosam on 7/2/11.
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

#ifndef JSON_OBJC_SEMANTIC_ACTIONS_MD_HPP
#define JSON_OBJC_SEMANTIC_ACTIONS_MD_HPP 

#warning deprecated file



#include "SemanticActions.hpp"
#include "json/ProducerConsumerQueue.hpp"
#include "json/ObjC/mutex.hpp"
#include "json/ObjC/semaphore.hpp"



namespace json { namespace objc { namespace internal {

    template <typename MutexT, typename SemaphoreT>
    class id_queue  : public json::utility::ProducerConsumerQueue<id, MutexT, SemaphoreT>
    {
        typedef json::utility::ProducerConsumerQueue<id, MutexT, SemaphoreT> base;
        typedef typename base::list_iterator list_iterator;
    public:        
        // Creates a queue instance with at max capacity elements
        // capacity shall be greater or equal 1.
        id_queue(size_t capacity = 10) 
        : base(10) 
        {
        }
        
        ~id_queue() {
            // if there are values in the process_list, we need to release them:
            this->mutex_.lock();            
            list_iterator iter = this->process_list_.begin();
            list_iterator last = this->process_list_.end();
            while (iter != last) {
                [*iter++ release];
            }
            this->mutex_.unlock();
        }
        
        void produce(id value) {
            [value retain];
            base::produce(value);
        }
        
        id aquire(float timeout_sec = 0.0) {
            id* result = base::aquire(timeout_sec);
            return *result;
        }
        
        void commit(id value) {
            [value release];
            base::commit(&value);
        }
        
        
    };

}}}  // namespace json::objc::internal



namespace json { namespace objc { 
    
    //
    //  class SemanticActionsMD
    //
    //  Extends the SemanticActions class with the capability to parse
    //  more than one json document. The parser inserts the json container
    //  from successfully parsed documents into an multithread safe queue
    //  within the semantic actions instance.
    //  The json containers can be retrieved concurrently while the parser
    //  is still parsing documents.    
    //
    //  
    // Template parameter EncodingT shall be derived from json::utf_encoding_tag.
    // EncodingT shall match the StringBufferEncoding of the parser. Usullay, 
    // this is a UTF-8 encoding.
    //
    // The class SemanticActionsMD is a sub class of class SemanticActions and 
    // inherits all its public features.
    // 
    // The following options are available:
    // 
    //  - CheckForDuplicateKey:     Checks whether a key for a JSON object  
    //                              already exists. If it exists, an "duplicate 
    //                              key error" will be logged to error console 
    //                              and error state will be set.
    //                              Default: false
    //
    //  - KeepStringCacheOnClear:   Doesn't clear the string cache if the semantic
    //                              actions is cleared via function clear().
    //                              Default: false
    //
    //  - CacheDataStrings:         Caches data strings in addition to key strings.
    //                              Default: false
    //
    // 
    //  - ParseMultipleDocuments:   If true, the parser will continue to read
    //                              the input and parses as many documents until
    //                              the end of the input is reached.
    //                              Otherwise, the parser parses only one document
    //                              and then stops. Any remaining characters left
    //                              in the input, except whitespaces, issues an
    //                              error.
    //                              If the parser encounters an error in the input, 
    //                              the parser will stop regardless of how many
    //                              documents may still exists in the input.
    //                              Default: true


    template <typename EncodingT>
    class SemanticActionsMD : public SemanticActions<EncodingT>
    {
    protected:
        typedef SemanticActions<EncodingT> base;
        typedef internal::id_queue<gcd::mutex, gcd::semaphore> queue_t;
        
        queue_t     queue_;
        bool        opt_multiple_documents_;
        
    public:    
        SemanticActionsMD(size_t capacity = 100) 
        :   queue_(capacity), opt_multiple_documents_(true)
        {
        }
        
        ~SemanticActionsMD() 
        {
        }
        
        void parse_end_imp()                            
        {
            base::parse_end_imp();
            // If the wrapper has a delegate set, we assume the delegate already
            // handled the result of the parser. Otherwise, we take the result 
            // and add it to our queue:
            if (wrapper_.deleagte == nil and opt_multiple_documents_ and this->result_) {
                // Note, result_ is retained by this.
                queue_.produce(this->result_);  // now, result_ is retained by queue_
                [this->result_ release];
                this->result_ = nil;
            }
        }
        
        
        void clear_imp() {
            base::clear_imp();
            queue_.clear(); // empties the avail_queue only.
        }
        
        
        id aquire(float timeout = 60) {
            id result = queue_.aquire(timeout);
            if (result == nil) {
                // timeout
            }
            return result;
        }
        
        size_t avail() const {
            return queue_.avail();
        }
        
        void commit(id value) {
            queue_.commit(value);
        }
        
        id result() {
            if (opt_multiple_documents_) {
                id res = aquire(0);  // do not block
                if (res) {
                    [[res retain] autorelease];
                    commit(res);
                }
                return res;
            } 
            else
            {
                return base::result();
            }
        }
        
        bool parseMultipleDocuments() const { return  opt_multiple_documents_; }
        
        bool& parseMultipleDocuments() { return opt_multiple_documents_; }
        
    };

}}  // namespace json::semantic_actions
    
#endif // JSON_OBJC_SEMANTIC_ACTIONS_MD_HPP


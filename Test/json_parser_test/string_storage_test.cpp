//
//  string_storage_test.cpp
//  Test
//
//  Created by Andreas Grosam on 4/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>


#include "json/parser/string_storage.hpp"
#include "json/unicode/unicode_traits.hpp"
#include "gtest/gtest.h"



namespace test  {

    using json::parser_internal::string_storage;

    template <typename EncodingT> class StringStorage;
    
    template <typename EncodingT>
    struct StringStorageDelegateBase
    {
        virtual ~StringStorageDelegateBase() {}
        virtual void sync(StringStorage<EncodingT>& stringStorage) = 0;        
    };
    
    
    template <typename EncodingT>
    class StringStorage : public string_storage<EncodingT>
    {
        typedef string_storage<EncodingT> base;
        
        
    public:
        
        typedef typename base::code_unit_type           code_unit_type;
        typedef typename base::buffer_type              buffer_type;
        typedef typename base::const_buffer_type        const_buffer_type;
        typedef typename base::storage_pointer          storage_pointer;

        
        
        typedef StringStorageDelegateBase<EncodingT>    delegate_t;
        
        
        StringStorage(delegate_t* delegate = 0) 
        : base(), delegate_(delegate)
        {}
        
        StringStorage(std::size_t initial_storage_capacity, delegate_t* delegate = 0) 
        : base(initial_storage_capacity), delegate_(delegate)
        {}
        
        
        void flush() {
            if (delegate_) {
                delegate_->sync(*this);
            }
            this->reset();
        }
        
    private:
        virtual void sync()
        {
            if (delegate_) {
                delegate_->sync(*this);
            }
            this->reset();
        }
        
    private:
        delegate_t* delegate_;        
    };

    
    template <typename EncodingT>
    class DefaultStringStorageDelegate : public StringStorageDelegateBase<EncodingT>
    {
    public:
        DefaultStringStorageDelegate()  : size_(0) {}
        
        size_t size() const { return size_; }
                
    private:    
        virtual void sync(StringStorage<EncodingT>& stringStorage) 
        {
            size_ += stringStorage.size();
        }

    private:
        size_t size_;        
    };

}

namespace {
    
    using namespace json;
    using json::unicode::UTF_8_encoding_tag;
    
    using json::parser_internal::string_storage;
    
    
    class StringStorageTest : public ::testing::Test {
    protected:
        // You can remove any or all of the following functions if its body
        // is empty.
        
        StringStorageTest() {
            // You can do set-up work for each test here.
        }
        
        virtual ~StringStorageTest() {
            // You can do clean-up work that doesn't throw exceptions here.
        }
        
        // If the constructor and destructor are not enough for setting up
        // and cleaning up each test, you can define the following methods:
        
        virtual void SetUp() {
            // Code here will be called immediately after the constructor (right
            // before each test).
        }
        
        virtual void TearDown() {
            // Code here will be called immediately after each test (right
            // before the destructor).
        }
        
        // Objects declared here can be used by all tests in the test case for Foo.
    };
    
    
    /**
     Synopsis 
     
     template <typename EncodingT>
     class string_storage : boost::noncopyable
     {
     public:        
         typedef type       encoding_type;
         typedef type       code_unit_type;
         typedef type       buffer_type;        
         typedef type       const_buffer_type;        
         typedef type       storage_pointer;
         
         
     public:
         string_storage();

         
         string_storage(std::size_t initial_storage_capacity);
         
         // Extend the buffer in order to hold additional 'size' code units. The
         // buffer might possibly grow in order to hold at least 'size' code 
         // units, and it might possibly sync the current buffer.
         void extend(size_t size); 
         
         
         // Append the content of the string buffer 'buffer'.
         //  A string buffer always starts at the 
         // beginning of a character and ends with a complete character. When
         // synching we must take care of these boundaries, that is we cannot
         // arbitrarily cut a possibly multi byte sequence.
         void append(const_buffer_type const& buffer);
         
         // Append the code unit 'code_unit' to the current string buffer on top of the stack
         void append(code_unit_type code_unit);
         
         // Return the current buffer
         const_buffer_type buffer() const;
         
         // Return the current buffer on top of the stack
         buffer_type buffer();
         
         // Resets the current buffer, that is its size becomes zero.
         // The original content - that is, content past the new end pointer will
         // not be invalidated, unless it will be overridden or the storage needs
         // to be reallocated.
         void reset();
         
         // Returns the size of the buffer
         size_t size() const;
         
         // Returns a referene to the current end pointer of the buffer
         storage_pointer& dest();
         
         storage_pointer data():
         const storage_pointer data() const;
         
         
         // Reserve a storage size
         void reserve(size_t size);
         
         size_t avail() const;
         
         size_t capacity() const;
         
     
     protected:

        virtual void sync() = 0; 

     
     }
     */
    
    TEST_F(StringStorageTest, DefaultCtor) 
    {
        typedef test::StringStorage<UTF_8_encoding_tag> string_storage_t;
        typedef string_storage_t::const_buffer_type  const_buffer_t;
        
        string_storage_t stringStorage;
        
        EXPECT_EQ(0, stringStorage.capacity());
        EXPECT_EQ(0, stringStorage.avail());
        EXPECT_EQ(NULL, stringStorage.data());
        EXPECT_EQ(0, stringStorage.size());
        const_buffer_t buffer = stringStorage.buffer();
        
        
        EXPECT_EQ(0, buffer.first);
        EXPECT_EQ(0, buffer.second);
    }
    
    TEST_F(StringStorageTest, Ctor1) 
    {
        typedef test::StringStorage<UTF_8_encoding_tag> string_storage_t;
        typedef string_storage_t::const_buffer_type  const_buffer_t;
        
        const size_t Capacity = 1024;
        string_storage_t stringStorage(Capacity);
        
        EXPECT_EQ(Capacity, stringStorage.capacity());
        EXPECT_EQ(Capacity, stringStorage.avail());
        EXPECT_TRUE(stringStorage.data() != NULL);
        EXPECT_EQ(0, stringStorage.size());
        const_buffer_t buffer = stringStorage.buffer();
        
        
        EXPECT_TRUE(buffer.first != 0);
        EXPECT_EQ(0, buffer.second);
    }
    

    
    
    TEST_F(StringStorageTest, AppendBuffer) 
    {
        typedef test::StringStorage<UTF_8_encoding_tag> string_storage_t;
        typedef string_storage_t::const_buffer_type  const_buffer_t;
        
        test::DefaultStringStorageDelegate<UTF_8_encoding_tag> stringStorageDelegate;
        
        const size_t Capacity = 64;
        string_storage_t stringStorage(Capacity, &stringStorageDelegate);
        
        const size_t Length = 7;
        const_buffer_t buffer("abcdefg", Length);
        
        stringStorage.append(buffer);
        
        EXPECT_EQ(Capacity, stringStorage.capacity());
        EXPECT_EQ(Capacity - Length, stringStorage.avail());
        EXPECT_TRUE(stringStorage.data() != NULL);
        EXPECT_EQ(Length, stringStorage.size());
        
        const_buffer_t buf = stringStorage.buffer();
        EXPECT_TRUE(buf.first != 0);
        EXPECT_EQ(Length, buf.second);
        
        
        stringStorage.reset();
        EXPECT_EQ(Capacity, stringStorage.capacity());
        EXPECT_EQ(Capacity, stringStorage.avail());
        EXPECT_TRUE(stringStorage.data() != NULL);
        EXPECT_EQ(0, stringStorage.size());
    }
    
    
    
    
    TEST_F(StringStorageTest, Append1) 
    {
        typedef test::StringStorage<UTF_8_encoding_tag> string_storage_t;
        typedef string_storage_t::const_buffer_type  const_buffer_t;
        typedef string_storage_t::code_unit_type  char_t;
        
        test::DefaultStringStorageDelegate<UTF_8_encoding_tag> stringStorageDelegate;
        
        const size_t Capacity = 64;
        string_storage_t stringStorage(Capacity, &stringStorageDelegate);
        
        const char_t A = static_cast<char_t>('a');
        const size_t Length = 12;
        
        for (int i = 0; i < Length; i++) 
        {
            stringStorage.append(A);
        }
        
        
        EXPECT_EQ(Capacity, stringStorage.capacity());
        EXPECT_EQ(Capacity - Length, stringStorage.avail());
        EXPECT_TRUE(stringStorage.data() != NULL);
        EXPECT_EQ(Length, stringStorage.size());
        
        const_buffer_t buf = stringStorage.buffer();
        EXPECT_TRUE(buf.first != 0);
        EXPECT_EQ(Length, buf.second);
        
        
        stringStorage.reset();
        EXPECT_EQ(Capacity, stringStorage.capacity());
        EXPECT_EQ(Capacity, stringStorage.avail());
        EXPECT_TRUE(stringStorage.data() != NULL);
        EXPECT_EQ(0, stringStorage.size());
    }
    
    
    
    TEST_F(StringStorageTest, Append2) 
    {
        typedef test::StringStorage<UTF_8_encoding_tag> string_storage_t;
        typedef string_storage_t::const_buffer_type  const_buffer_t;
        typedef string_storage_t::code_unit_type  char_t;
        
        test::DefaultStringStorageDelegate<UTF_8_encoding_tag> stringStorageDelegate;
        
        const size_t Capacity = 64;
        string_storage_t stringStorage(Capacity, &stringStorageDelegate);
        
        const char_t A = static_cast<char_t>('a');
        const size_t Length = 12345;
        
        for (int i = 0; i < Length; i++) 
        {
            stringStorage.append(A);
        }
        
        stringStorage.flush();
        EXPECT_EQ(Length, stringStorageDelegate.size());
        
        
        EXPECT_TRUE(stringStorage.capacity() >= Length);
        EXPECT_EQ(stringStorage.capacity(), stringStorage.avail());
        EXPECT_TRUE(stringStorage.data() != NULL);
        EXPECT_EQ(0, stringStorage.size());
        
    }

    
    
}
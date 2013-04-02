//
//  SafeBool.hpp
//
//  Created by Andreas Grosam on 4/29/11.
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

// Test helper class
//
// Class SafeBool can be used in a boolean expression:
// SafeBool b(true);
// if (b)
//    ...
// Type conversions are not allowed, though. (see compile time assertions below)

#include <type_traits>


class SafeBool
{
private:
    const bool v_;
    struct s { int dummy; };
    typedef int s::* bool_convert;
public:
    explicit SafeBool(bool v) : v_(v) {}
    operator bool_convert () const { return v_ ? &s::dummy : 0; }
};
static_assert( (std::is_convertible<SafeBool,bool>::value), "SafeBool shall be convertible to bool" );
static_assert( (not std::is_convertible<SafeBool,int>::value), "SafeBool shall NOT be convertible to int");
static_assert( (not std::is_convertible<SafeBool,void*>::value), "SafeBool shall NOT be convertible to void*");


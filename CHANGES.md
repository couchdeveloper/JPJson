## This file describes API changes, new features and bug fixes on a high level point of view.

# JPJsonParser

### Version 0.1 beta (2012-03-13)

* Initial Version



### Version 0.2 beta (2012-05-02)

#### Changes

* Caution: this version changed the "SAX-style" Objective-C API.

* Improved runtime on arm processors by about 15% to 20% (on Intel no change).

* Improved stack space usage (accomplished through modified internal string buffer).

* Implemented "large string" feature which sends partial strings to the semantic actions object. 
  This makes it possible to parse very large JSON strings by maintaining a fixed memory foot print.

* Adjusted and fixed AppleDoc documentation which is now more accurate.



### Version 0.3 beta

#### Changes

* Caution: this version changed the "SAX-style" Objective-C API.

* Renamed class JPJsonSemanticActions to JPRepresentationGenerator.

* Extended JPJsonWriter with protocols and helper methods so that custom classes can be easily serialized.

* Added an example (Sample 6) to show how to extend JPJsonWriter to serialize NSDate and custom classes.


#### Issues

* Documentation generator Appledoc generates documentation for private classes and categories.



### Version 0.4 beta

#### Changes

* .gitignore: changed 'user.' to '$user.' as a file name prefix in order to ingore user specific files. This affects the xcconfig files which users of the JPJson package requires to rename their user specific xcconfig files, e.g from "user.boost.xcconfig" to "$user.boost.xcconfig".

* Xcode Project Schemes will be shared now.

* JPJsonWriter: improved performance and memory foot print when serializing NSString objects whose internal encoding is not Unicode.

* Updated Project settings for iOS applications and iOS static library to match the Apple recommended way to link against a static library (which resolves an issue when archiving a project, but does not fix the issue when evaluating the dependecies - which has still bugs in Xcode 4.3.3).

* The recommended version for boost in now 1.50



#### Bug fixes

* JPJsonWriter: fixed a memory leak in JPJsonWriter (now uses NSDataStreambuf class internally)

* JPJsonWriter: fixed a bug in converting NSNumber objects whose underlying type equals BOOL and whose address is not equal `kCFBooleanTrue` or not equal `kCFBooleanFalse`.



### Version 0.5 beta

#### Bug fixes

* Fixed a minor bug in Sample6, introduced due to an API change.


### Version 0.6 beta

* Improved installation document and fixed typos.


### Version 0.6.1 beta

* Added NSOutputStream API to JPJSONWriter.




### Version 0.7 beta

* Quite a lot has been changed within the details of the implementation. Now, the implementation utilizes and requires clang's standard C++ library and C++11 features, with the effect of some nice performance improvements and a simplification of the source code.

* The minumim iOS versions is now 5.0. (4.2 is no longer supported, due to the change towards most recent C++ compiler and standard C++ library features)

* Updated all projects to most recent Xcode version (as of writing: v4.6.1).

* The performance of the Objective-C parser has been improved slightly:
  On ARM it is now quite noticable faster than the fastest competitor: JSONKit, 
  and almost about twice as fast as NSJSONSerialization for many work loads.


* The C++ API has been finished, including a pretty nice version of a JSON representation, class `json::value`, which utilizes standard containers and a discriminated union class: `json::variant`.
  
  When speed matters: the performance of the C++ parser generating a JSON representation  with standard containers utilizing an arena allocator is about twice as fast as the Objective-C parser which creates a representation of Foundation objects.

* The C++ API now requires a conforming C++11 compiler and a C++11 standard library. The _full_ set of features of the C++ JSON representation, class `json::value`, will require a C++11 library which fully implemented the "scoped allocator model" for containers. At the time of writing, gcc 4.8's standard library does not fulfil this requirement.

* Note: currently, the C++ API is only tested with clang on Mac OS X/iOS.


* The gtest framwork required a few changes in order to compile Unit tests in C++11 utilizing a C++11 standard library. Thus, gtest should now be cloned from GitHub which already contains these modifications and compiles successfully (please consult the INSTALL document where and how to clone gtest from GitHub).


#### Bug fixes

* Fixed a memory leak bug in when creating a NSDecimalNumber.


  
### Version 0.7 beta

* API Changes

    Parser options (JPJsonParserOptions) have been changed. Please read the documentation.


* Unicode Noncharacher and Unicode 'NULL' Behavior and Options

    JPJsonParser now can be configured in detail how to handle Unicode noncharacters and Unicode 'NULL' characters in JSON Strings.
    Please consult the documentation for a detailed description.


* The Documentation now is more complete


  
#### Bug fixes

* Objective-C Representation Generator:
    Fixed an edge-case where JSON Number conversions selected a NSNumberDecimal where a double would have been appropriate.
    




### Version 0.8.0 beta

 * API Changes
 
   - Changed values and namings for parser options `JPJsonParserOptions`

 * Updated Documentation
 
 * Removed a couple of boost dependencies.
 
 


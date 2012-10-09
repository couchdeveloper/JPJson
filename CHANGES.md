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

* JPJsonWriter: fixed a bug in converting NSNumber objects whose underlaying type equals BOOL and whose address is not equal `kCFBooleanTrue` or not equal `kCFBooleanFalse`.



### Version 0.5 beta

#### Bug fixes

* Fixed a minor bug in Sample6, introduced due to an API change.


### Version 0.6 beta

* Improved installation document and fixed typos.




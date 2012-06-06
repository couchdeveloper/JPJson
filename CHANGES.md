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

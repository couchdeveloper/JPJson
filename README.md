# JPJson Package


### JPJson is an Objective-C framework for parsing and generating JSON documents under the terms of a developer friendly Apache License, Version 2.0.

Status: Beta *)


The implementation details are implemented in C++. There is also a fully functional C++ API with a STL compatible JSON representation, which offers superior performance. The C++ API status is Alpha, though.
 




### Basic Features

*   The implementation strictly conforms to the specification defined in RFC 4627, the "JavaScript Object Notation (JSON)" and to the specifications and guidelines issued in the Unicode Standard Version 6.0.

*   The "JSON Parser" can read JSON text in any Unicode encoding scheme, that is UTF-8, UTF-16, UTF-16LE, UTF-16BE, UTF-32, UTF-32LE and UTF-32BE.

*   The parser reads a JSON Text from NSStrings or NSData objects and generates a *JSON representation* which is a hierarchy of native Foundation Objects. 

*   The parser can read partially input. That is, one JSON Text can be partitioned into several chunks, for instance when receiving more than one NSData objects from an NSURLConnection.

*   The "JSON Writer" transforms a JSON representation into a JSON text in any of the Unicode encoding schemes.

*   The library offers a few very easy to use high level APIs.

*   The implementation has been rigorously fine-tuned for performance and low memory foot-print. 

*   The Objective-C API integrates very nicely with NSURLConnection and offers the opportunity to simultaneously download, parse and process one or even more than one JSON document per connection. The design allows to leverage multiple CPUs and also allows to control memory usage and thus making it easy to keep a low memory profile which is key on restricted devices.

*   On a more advanced level, the API provides for great flexibility. In addition the convenience API, it offers a SAX-style API and provides ways to fully customize many aspects of the parser and generator.

    For instance, it would be very easy to define a "validating" JSON parser, which just parses a JSON text and prints out detailed diagnostics. Parsing is very fast, and when just validating a JSON text the parser becomes roughly four times faster than when additionally generating the representation.



  
  
### Possible Improvements

- there is a minor issue related to formatting numbers into JSON Numbers. Well, the current behavior still conforms to RFC 4627, but it could be made better.

- Performance of the "Writer" has not been checked. There is room for improvements, too.
  
  
  
***)** Due to its Beta status, not all minor features are implemented in this version. Well, the feature might actually be there and may be well tested, but a higher level function may not actually use it, or the higher level API simply does not make it public.
  
The following features are not yet implemented in version 0.1, but planned in the next beta version:

### Not Yet Implemented 

*   writing into streams when generating JSON text 
*   when generating JSON text from a repesentation only UTF-8 can be specified.


### Known Bugs

("Known bugs" are subject to be fixed in the next version.)

- a few minor bugs exist in the Documentation of the Objective-API







### Notes For Developers

The library is designed for flexibility. It is devided into several independent sub-modules and utility classes, each comming with its own unit tests and benchmark tests.

The implementation has been rigorously fine-tuned for performance and low memory foot-print. The performance is comparable to the currently fastest available Objective-C implementation (JSONKit). However, the JPJsonParser can be used to simultaneously download and parse JSON text of arbitrary size, and thus is cleary superior in those scenarios, escpecically on hardware with more than one CPU.


Included in the package are sample projects showing the basic and advanced usage, unit tests, benchmark tests, library and framework projects, and more complete iOS projects showing how to download and parse a very large JSON text without exceeding restricted memory. Additionaly, there is an AppleDoc build phase for generating the Objective-C API help documents.



### Design

On a higher level, there is the "JSON Parser", the "JSON Writer" and the "Semantic Actions", which will be explained briefly. The C++ API also contains a nice and fast implementation for the JSON representation with an STL like interface.

These high level modules use common support moduls and utility classes. For instance there is a separate Unicode conversion library, a byte-swapping module, synchronization primitives, logger modul, string-hasher, base64 decoder/encoder, an experimental json-path module and more. 



The parser is actually devided into two parts: the "core parser", whose purpose is to solely parse JSON text and sending "parser events" with associated JSON data and further info when encountering the many different parts of a JSON text.

The second part is the "Semantic Actions". The Semantic Actions is a class whose purpose is to act on the parser events. This class can be customized, aka subclassed, which allows to do virtually anything when parsing a JSON document. 


In an advanced application, the developer pairs the JSON Parser with a "Semantic Actions object", either a "biult-in" one or one that has been customized for special needs in order achieve the desired behavior without the need to reimplement many parts of the library.

One of the "builtin" Semantic Actions object creates a representation of a JSON text which is a hierarchy of Foundation objects and is the default in the "convenience interface".
  

Installation
============

## Mac OS X/iOS - Xcode - Objective-C-API


The recommended way to integrate JPJson for use in your product is to build a JPJson library directly with the corresponding Xcode project from the JPJson package and then link your product against it. This keeps a few essential compiler switches private and the libraries can be build independently from the build settings of other products.

The package includes (among others) Xcode projects for building a static library, a dynamic library and a Mac OS X framework, all exposing the identical Objective-C API. You will choose to link your product against *one* of the libraries above. Note that iOS only supports static libraries. On Mac OS X you can link against any kind of library.

When building with Xcode the preferred method is to arrange your project in a *Workspace* and include the desired JPJson project into it.


Before you can start, you need to setup a few things on your developer machine and also in Xcode, which needs to be done only once, though:

1.  Install Dependencies
2.  Setup User Specific Settings in *Source Trees* in Xcode preferences
3.  Setup User Specific Settings in JPJson projects

Once you have finished this, you can setup your Xcode projects that link against a JPJson library.


Parts of the JPJson package, the libraries itself, but also Unit tests and bench tests depend on a number of third party packages. 

If you are only interested in compiling the JPJson *libraries* which you need for including them in your projects, you strictly only need [boost](http://www.boost.org/), a set of free peer-reviewed portable C++ source libraries. Installing boost for building JPJson libraries just involves downloading the compressed archive, decompressing it and putting the folder somewhere on your *developer machine's disk*, which is described in detail in the next section.


------------
### Installing Boost

In order to create applications linking against the JPJson library or framework you need to build JPJson. JPJson depends on [boost](http://www.boost.org/). Thus, you need to install boost on *your* developer machines, unless you haven't done it so far. For JPJson it suffices to just download the most recent boost archive and put the decompressed folder anywhere on your local disk and you are done with installing boost. The version of boost shall be the newest one, which is at the time of writing v1.53.0.

Other ways to install boost is to use one of the package managers available for Mac OS X, for instance [MacPorts](http://www.macports.org/) or [Homebrew](http://mxcl.github.com/homebrew/).


------------
### Installing AppleDoc

Note: AppleDoc is only needed on *your* developer machine if you want to modify or extend the documentation of JPJson library as a contributor.


[AppleDoc](https://github.com/tomaz/appledoc) is an Objective-C code Apple style documentation set generator. Most of the API documentation of JPJson is provided with AppleDoc, so it is recommended to install AppleDoc which creates the nicely formatted documentation for JPJson.

The recommended way to install AppleDoc is to clone the GitHub project and compile the tool using Xcode. Upgrading to newer versions is then also greatly simplified.

In order to clone the ApleDoc git project, create an appropriate destination folder on your developer machine, say into `~/Develop/AppleDoc`, open a console window with Terminal and `cd` into the newly created folder. Then, type the following into the console:

`git clone git://github.com/tomaz/appledoc.git`

This creates a folder `appledoc` on your local developer machine within your current working directory.

All further details which are required to properly install AppleDoc can be found in the README file within the package. Follow the instructions to install it.


------------
### Installing ICU

Note: this library is only needed on *your* developer machine if you want to run Unit tests and perform bench marks. The JPJson libraries itself do not depend on it.


The easiest way to install [ICU](http://site.icu-project.org/) is to use a package manager, like [MacPorts](http://www.macports.org/) or [Homebrew](http://mxcl.github.com/homebrew/).

Otherwise, you can download the ICU package and install it manually, which is described in the README file in the package.


------------
### Installing gtest "The Google C++ Testing Framework"

Note: this library is only needed on *your* developer machine *if* you want to run Unit tests. The JPJson libraries itself do not depend on it. 

The gtest framework will be used in various Unit test applications in the JPJson package. gtest provides a Mac OS X framework and additionally a few source files which can be optionally used when creating Unit test applications. 

> Hint: If you develop in C++, you may find this Unit testing framework very useful, since it provides support for testing C++ templates, which is usually lacking in other Unit test frameworks.


Since version 0.7, JPJson's Unit Tests build with the C++11 standard library. A few changes have been required in order to compile gtest successfully with a most recent C++11 library. The applied modifications and an updated Xcode project are now available on GitHub: [gtest](https://github.com/couchdeveloper/gtest).

In order to clone the gtest project, create an appropriate destination folder on your developer machine, say into `~/Develop/GTest`, open a console window with Terminal and `cd` into the newly created folder. Then, type the following into the console:

`git clone git://github.com/couchdeveloper/gtest.git`


The gtest framework will be installed in the `/Library/Frameworks` folder on the developer's machine. This makes it quite easy to setup Unit test projects using the gtest framework. 

The gtest packackage can be build in various ways, one of which is using Xcode which is described below briefly:

What we want to achieve is to first *build* the gtest framework and once it has been build successfully, *install* it in the appropriate location for third party frameworks. Apple suggest to install third party frameworks into the `/Library/Frameworks` directory. Installing it into this location requires *privileged access* to this folder.

Build the gtest Framework with Xcode in order to check that it builds successfully:

*  Set the active Scheme to `gtest-framework`. 
*  Ensure that the active  **Build Configuration** is set to **Release** in the corresponding Scheme.
*  Choose command **Product -> Build**.

If this was successful, you may clear the build folder and proceed with installing gtest using `xcodebuild` tool:

If you are a "sudoer" on your system (which is not the default for a normal User Account), you may simply use the **Terminal App**, `cd` into the folder where gtest's Xcode project is located and perform the installation build typing the following commands into the console (note that `$` indicates the prompt and does not belong to the command):

```
$ cd ~/Develop/gtest/gtest-1.6.0/xcode
$ sudo xcodebuild -target gtest-framework -configuration Release \
  install INSTALL_PATH=/Library/Frameworks DSTROOT=/
```

You will be prompted for your password (enter your user password) and then xcodebuild starts building and installing the framework into `/Library/Frameworks` folder. You are done installing gtest.


Note: If you are not a sudoer, you may build the Release Configuration with Xcode, locate the destination folder, and then copy the folder `gtest.framework` manually to the `/Library/Frameworks` folder using the Finder - which will prompt you for an admin password.

> Note: when setting up a project that links against a framework located at `/Library/Frameworks` Xcode 4.5 may not find the headers when compiling, causing Xcode to issue an error. This is due to a bug on Xcode's compiler which does not search the required system header search path at `/Library/Frameworks`. The workaround is to explicitly set a framework search path in the target's build settings.



------------
### Setting up Xcode

In order to build the products, Xcode requires user defined settings. The JPJson projects need to locate a few *source folders* of external libraries namely for

- **boost**, and

- **gtest**, (which is required only if you want to run Unit tests)


User defined settings are accomplished with the combination of "Source Trees" and an user specific Xcode config file. A "Source Tree" defines a path to header search paths or library search path, and the Xcode config file defines the corresponding build settings which are then available in the project settings, target settings and also in Xcode config files.

Note: User defined settings, including the user specific Xcode config files are not tracked by the source code management. They exist only locally and may be different for each "user" (developer), as it is intended.


Before you can build the JPJson libraries you need to add *Source Trees* settings in Xcode preferences and you need to modify or create user Xcode config files which are located in folder "<JPJson ROOT>/Xcode.config" (see below in "User Specific Settings in the JPJson Package"). This needs to be done only *once*.




The most reliable way to refer to external source code when building under Xcode is to use Xcode's "Source Trees" concept :

Open Xcode Preferences and navigate to the "Source Trees" panel which is under the "Locations" tab. You need to create at least the entry for `JPJson.BOOST_ROOT`. The entry for `JPJson.GTEST_ROOT` is only required if you want to run the Unit tests (recommended).


    Setting Name            Display Name            Path
    ------------------------------------------------------------------------
    JPJson.BOOST_ROOT       BOOST_ROOT              /local/path/to/boost
    JPJson.GTEST_ROOT       GTEST_ROOT              /local/path/to/gtest


The path to boost can be set as follows: suppose, after downloading and decompressing the archive `boost_1_49_0.zip` a folder has been created whose name is `boost_1_49_0`. Given this example, you need to specify the absolute path to that folder including its name, for example: `/Users/Me/Develop/boost/boost_1_49_0`. 
Of course, this may be different on your system.
Note: Don't add a trailing '/'.

Likewise, suppose you have downloaded and extracted the gtest package `gtest-1.6.0.zip`, which created a folder `gtest-1.6.0`. Specify the path including the folder name, for example:
`/Users/Me/Develop/gtest/gtest-1.6.0`. 
Note: Don't add a trailing '/'.


As mentioned, these "JPJson.BOOST_ROOT" and "JPJson.GTEST_ROOT" are now available as build settings in Xcode projects, targets and Xcode config files.

Note also that you can define different names for the Setting name - just ensure that the corresponding Xcode config file (see below in "User Specific Settings in the JPJson Package") reads the correct variable and initializes a corresponding build setting correctly.



------------
### User Specific Settings in the JPJson Package.

The JPJson package requires a few user specific Xcode config files. The "user" is actually *you*, the developer building the JPJson library. There are three template files as default config files and these are probably already fine, but there is the need to rename the default Xcode config files in order to prevent them to be tracked by the VCS.

Locate the Xcode config files prefixed with `default` with the JPJson package and change the prefix from `default` to `$user`:

*  `default.boost.xcconfig` becomes `$user.boost.xcconfig`
*  `default.gtest.xcconfig` becomes `$user.gtest.xcconfig`
*  `default.ICU.xcconfig` becomes `$user.ICU.xcconfig`


You may now modify your local copy of the Xcode config file in order to setup user specific settings appropriately tailored for your development environment. The Xcode config file may define build settings which derive from Source Tree settings. Ensure you use the correct names in order to set them up correctly.

Please refere to the comments in the Xcode config files, in order to get more info.


Note again, that user specific Xcode config files will not be tracked by the source code management.





------------
### Setting up an Xcode Project


This section describes how to setup an Xcode project with a binary that links against the JPJson library. There are a few settings required for the target which links against the JPJson library. Let us refer to this target as the "client".


First, create a Xcode workspace and create a new project or reference any existing Xcode project where you want to incorporate JPJson. 

 - For an iOS project add the "`JPJson iOS Libraries`" Xcode project to the workspace, either as a sibling to your main project or as a subproject.

 - For Mac OS X projects add "`JPJson Mac OS X Libraries`" project to the workspace, either as a sibling to your main project or as a subproject.

Note: Both projects are located in the corresponding sub folders of the `Libraries` folder within the JPJson package.


Once you have added the library project to your workspace, link the client target to the JPJson library by selecting the client project in the Navigation area, then select the client target in the Editor area and then use the "Link Binary With Libraries" section in "Build Phases" tab of the target editor and add the JPJson library by clicking on the (+) Button and selecting it from within the "Workspace" folder.

For iOS targets you can only choose the static library "`libjson-iOS.a`", for Mac OS X you can select either a framework, a dynamic library or a static library. In any case you shall select only *one* JPJson library.

When building your product the JPJson library will be build with its own settings as specified in their corresponding Xcode project with its intended build settings.

The library products are placed into the folder in the respective `$(BUILT_PRODUCTS_DIR)` folder of the current built configuration and the public headers are copied to a subfolder "`include/JPJson`" within the same directory.

Xcode automatically adds a header search path for projects linking against libraries whose public headers are located at `"$(BUILT_PRODUCTS_DIR)/include`". These default settings are now appropriate to locate the JPJson headers by a "qualified" import directive, e.g.:
 
     #import "JPJson/JPJsonParser.h"

That is, you are not required to add a header search path for the client target.


Yet, there are still a few settings required:

The JPJson Lib requires to be linked itself with the standard C++ library. This requires a corresponding build setting in the _main_ product that produces the executable binary. Since version 0.7 JPJson uses clang's standard C++ library instead of gcc's. We need to adding a the option `-lc++` to the **Other Linker Flags** build setting of the product producing the executable.

Note: Linking against gcc's standard C++ library would require to add a flag `-lstdc++`.

Furthermore, in order to ensure the linker puts all possibly required code from the static archive containing Objective-C code into the final executable we need to add the option `-ObjC ` to the **Other Linker Flags**, too.

 - In the target build settings of the client, add the following two options to **Other Linker Flags**:  
    `-ObjC -lc++`  
    e.g.:  
    `OTHER_LDFLAGS = -ObjC -lc++`


 - Optionally, but recommended, add a few macros to the target build setting **Preprocessor Macros** of your client:
   e.g.:
    *For Debug builds:*  
   `GCC_PREPROCESSOR_DEFINITIONS = DEBUG=1 __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0`  

   *For Release builds:*  
   `GCC_PREPROCESSOR_DEFINITIONS = NDEBUG NS_BLOCK_ASSERTIONS __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0`  


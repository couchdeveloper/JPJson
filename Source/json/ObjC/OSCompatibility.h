//
//  OSCompatibility.h
//  
//
//  Created by Andreas Grosam on 24.08.13.
//
//

#include <ConditionalMacros.h>

#ifndef JSON_OBJC__OSCOMPATIBILITY_H
#define JSON_OBJC__OSCOMPATIBILITY_H

#if TARGET_OS_IPHONE
// Compiling for iOS
    #if __IPHONE_OS_VERSION_MIN_REQUIRED >= 60000
        // >= iOS 6.0
        #define JP_DISPATCH_RELEASE(__object) do {} while(0)
        #define JP_DISPATCH_RETAIN(__object) do {} while(0)
        #define JP_DISPATCH_BRIDGE_VOID_CAST(__object) (__bridge void*)__object
    #else
        // <= iOS 5.x
        #define JP_DISPATCH_RELEASE(__object) do {dispatch_release(__object);} while(0)
        #define JP_DISPATCH_RETAIN(__object) do { dispatch_retain(__object); } while(0)
        #define JP_DISPATCH_BRIDGE_VOID_CAST(__object) __object
    #endif
#elif TARGET_OS_MAC
    // Compiling for Mac OS X
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1080
        // >= Mac OS X 10.8
        #define JP_DISPATCH_RELEASE(__object) do {} while(0)
        #define JP_DISPATCH_RETAIN(__object) do {} while(0)
        #define JP_DISPATCH_BRIDGE_VOID_CAST(__object) (__bridge void*)__object
    #else
        // <= Mac OS X 10.7.x
        #define JP_DISPATCH_RELEASE(__object) do {dispatch_release(__object);} while(0)
        #define JP_DISPATCH_RETAIN(__object) do { dispatch_retain(__object); } while(0)
        #define JP_DISPATCH_BRIDGE_VOID_CAST(__object) __object
    #endif
#endif




#endif

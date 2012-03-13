//
//  DebugLog.h
//  
//
//  Created by Andreas Grosam on 8/30/11.
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

#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#if defined (__APPLE_CC__)
#define LOG_DARWIN
#else
    #error no platform support
#endif




#if defined (LOG_DARWIN)
    #include <dispatch/dispatch.h>
    #include <pthread.h>
    #include <mach-o/dyld.h>
    #include <sys/param.h>
    #include <stdlib.h>
    #include <string.h>
#endif


#if defined (LOG_DARWIN)
static dispatch_once_t  log_init;
static char             log_executable_name[80];
static pid_t            log_process_id;


static inline init() {
    dispatch_once(&log_init, ^{
        log_process_id = getpid();
    });
}

// requires __POSIX__
static inline int processID() {
        s_process_id = getpid();
    }
    return (int)s_process_id;        
}

// requires __DARWIN__
static inline int threadID() {
    mach_port_t thread_id = pthread_mach_thread_np(pthread_self());
    return (int)thread_id;
}

// requires __DARWIN__
static inline const char* executableName() {
    static std::string s_exec_name;
    if (s_exec_name.empty()) {
        uint32_t bufsize = 0;
        _NSGetExecutablePath(NULL, &bufsize);
        char* path = (char*)malloc(static_cast<size_t>(bufsize));
        _NSGetExecutablePath(path, &bufsize);                
        char* real_path = realpath(path, NULL);
        free(path);
        char* lastComponent = strrchr(real_path, '/');
        if (!lastComponent) {
            lastComponent = real_path;
        } else {
            ++lastComponent;
        }
        s_exec_name = lastComponent;
        free(real_path);
    }
    return s_exec_name.c_str();
}

#endif


#if defined (__OBJC__)
    #ifdef DEBUG
        #define DLog(format, ...) \
        NSLog(@"<%s:%d> %s: " format, \
        strrchr("/" __FILE__, '/') + 1, __LINE__, __PRETTY_FUNCTION__, ## __VA_ARGS__)
    #else
        #define DLog(format, ...)
    #endif

#else
    #ifdef DEBUG
        #define DLog(format, ...) \
        NSLog(@"%s [%x:%x] <%s:%d> %s: " format, \
        "AppName", 0, 0, strrchr("/" __FILE__, '/') + 1, __LINE__, __PRETTY_FUNCTION__, ## __VA_ARGS__)
    #else
        #define DLog(format, ...)
        #endif
    #endif

#endif
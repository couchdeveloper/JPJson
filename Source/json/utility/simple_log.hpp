//
//  simple_log.hpp
//  
//
//  Created by Andreas Grosam on 7/29/11.
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

#ifndef JSON_UTILITY_SIMPLE_LOG_HPP
#define JSON_UTILITY_SIMPLE_LOG_HPP


#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <string>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/greater_equal.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>

#if defined (__APPLE_CC__)
#include <TargetConditionals.h>
#include <pthread.h>
#include <mach-o/dyld.h>
#include <sys/param.h>
#include <stdlib.h>
#include <cstring>
#else
#error no platform support
#endif

#if TARGET_OS_IPHONE or TARGET_IPHONE_SIMULATOR
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace json { namespace utility {
    namespace internal {
        
        
        // requires __POSIX__
        inline int processID() {
            static pid_t s_process_id = 0;
            if (s_process_id == 0) {
                s_process_id = getpid();
            }
            return (int)s_process_id;        
        }
        
        // requires __DARWIN__
        inline int threadID() {
            static mach_port_t s_thread_id = 0;
            if (s_thread_id == 0) {
                s_thread_id = pthread_mach_thread_np(pthread_self());
            }
            return s_thread_id;
        }
        
        // requires __DARWIN__
        inline const char* executableName() {
            static std::string s_exec_name;
#if TARGET_OS_IPHONE or TARGET_IPHONE_SIMULATOR
        // TODO: implement for iOS
            if (s_exec_name.empty()) {
                char buffer[256];
                *buffer = 0;
                CFBundleRef mainBundle = CFBundleGetMainBundle();
                if (mainBundle) {
                    CFStringRef name = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(mainBundle, kCFBundleExecutableKey);
                    if (name) {
                        if (!CFStringGetCString(name, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                            *buffer = 0;
                        }
                    }
                }
                s_exec_name = *buffer ? buffer : "<Appname>";
            }
#else                        
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
#endif            
            return s_exec_name.c_str();
        }
        
                
#if 0        
        // __DARWIN__ || __UNIX__ || __LINUX__ || __BSD__
        inline char * getExecPath (char * path, size_t dest_len, char * argv0)
        {
            char * baseName = NULL;
            char * systemPath = NULL;
            char * candidateDir = NULL;
            
            /* the easiest case: we are in linux */
            if (readlink ("/proc/self/exe", path, dest_len) != -1)
            {
                dirname (path);
                strcat  (path, "/");
                return path;
            }
            
            /* Ups... not in linux, no  guarantee */
            
            /* check if we have something like execve("foobar", NULL, NULL) */
            if (argv0 == NULL)
            {
                /* we surrender and give current path instead */
                if (getcwd (path, dest_len) == NULL) return NULL;
                strcat  (path, "/");
                return path;
            }
            
            
            /* argv[0] */
            /* if dest_len < PATH_MAX may cause buffer overflow */
            if ((realpath (argv0, path)) && (!access (path, F_OK)))
            {
                dirname (path);
                strcat  (path, "/");
                return path;
            }
            
            /* Current path */
            baseName = basename (argv0);
            if (getcwd (path, dest_len - strlen (baseName) - 1) == NULL)
                return NULL;
            
            strcat (path, "/");
            strcat (path, baseName);
            if (access (path, F_OK) == 0)
            {
                dirname (path);
                strcat  (path, "/");
                return path;
            }
            
            /* Try the PATH. */
            systemPath = getenv ("PATH");
            if (systemPath != NULL)
            {
                dest_len--;
                systemPath = strdup (systemPath);
                for (candidateDir = strtok (systemPath, ":"); candidateDir != NULL; candidateDir = strtok (NULL, ":"))
                {
                    strncpy (path, candidateDir, dest_len);
                    strncat (path, "/", dest_len);
                    strncat (path, baseName, dest_len);
                    
                    if (access(path, F_OK) == 0)
                    {
                        free (systemPath);
                        dirname (path);
                        strcat  (path, "/");
                        return path;
                    }
                }
                free(systemPath);
                dest_len++;
            }
            
            /* again someone has use execve: we dont knowe the executable name; we surrender and give instead current path */
            if (getcwd (path, dest_len - 1) == NULL) return NULL;
            strcat  (path, "/");
            return path;
        }
#endif        
        

        struct logger_imp
        {
            static void vflog(FILE* fstream, const char* format, va_list args) {
                char time_str[80];
                time_t t;
                std::time(&t);
                struct tm* timeInfo;
                timeInfo = localtime(&t);
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %X", timeInfo);
                fprintf(fstream, "%s %s [%x:%x]: ", 
                        time_str, executableName(), processID(), threadID());
                vfprintf(fstream, format, args);
                if (format[strlen(format)] != '\n')
                    fprintf(fstream, "\n");
            }
            static void flog(FILE* fstream, const char* s) {
                char time_str[80];
                time_t t;
                std::time(&t);
                struct tm* timeInfo;
                timeInfo = localtime(&t);
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %X", timeInfo);
                fprintf(fstream, "%s %s [%x:%x]: %s", 
                        time_str, executableName(), processID(), threadID(), s);
                if (s[strlen(s)] != '\n')
                    fprintf(fstream, "\n");
            }
        };
        
    } // namespace internal
    
    
    enum LOG_Level {
        LOG_LEVEL_TRACE   = 16,
        LOG_LEVEL_DEBUG   =  8,
        LOG_LEVEL_INFO    =  4,
        LOG_LEVEL_WARNING =  3,
        LOG_LEVEL_ERROR   =  2,
        LOG_LEVEL_FATAL   =  1,
        LOG_LEVEL_NONE    =  0
    };
    
    template <LOG_Level S>
    struct log_severity {
        static const LOG_Level value = S;
        
    };
    
    typedef log_severity<LOG_LEVEL_TRACE>    log_trace;
    typedef log_severity<LOG_LEVEL_DEBUG>    log_debug;
    typedef log_severity<LOG_LEVEL_INFO>     log_info;
    typedef log_severity<LOG_LEVEL_WARNING>  log_warning;
    typedef log_severity<LOG_LEVEL_ERROR>    log_error;
    typedef log_severity<LOG_LEVEL_FATAL>    log_fatal;
    typedef log_severity<LOG_LEVEL_NONE>     log_none;
    
    static log_trace   LOG_TRACE;
    static log_debug   LOG_DEBUG;
    static log_info    LOG_INFO;
    static log_warning LOG_WARNING;
    static log_error   LOG_ERROR;
    static log_fatal   LOG_FATAL;
    static log_none    LOG_NONE;
    
    
    
    template <typename MaxSeverity = log_warning >
    class logger {
    public:    
        
        
        logger() 
        : max_severity_(MaxSeverity::value)
        {
        }
        
        
        template <typename LogSeverity>
        void log_level(LogSeverity) {
            max_severity_ = LogSeverity::value;
        }
        
        void log_level(LOG_Level level) {
            max_severity_ = level;
        }
        
        LOG_Level log_level() const {
            //return std::min(max_severity_, MaxSeverity::value);
            LOG_Level v = MaxSeverity::value;
            return std::min(max_severity_, v);
        }
        
        
        template <typename LogSeverity>
        typename boost::enable_if<boost::mpl::bool_<(LogSeverity::value <= MaxSeverity::value)>, void>::type
        log(LogSeverity severity, const char* format, ...) const
        {
            if (LogSeverity::value > max_severity_) 
                return;
            va_list args;                     
            va_start(args, format);
            internal::logger_imp::vflog(stdout, format, args);
            va_end(args);
        }
        
        template <typename LogSeverity>
        typename boost::enable_if<boost::mpl::bool_<(LogSeverity::value > MaxSeverity::value )>, void>::type
        log(LogSeverity severity, const char* format, ...) const
        {
        }
        
    private:        
        LOG_Level       max_severity_;
    };
    
    
    
    
}}   // namespace json::utility



#endif  // JSON_UTILITY_SIMPLE_LOG_HPP

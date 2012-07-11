//
//  DLog.h
//  JSONViewer
//
//  Created by Andreas Grosam on 5/30/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef DLOG_H
#define DLOG_H

#define DEBUG_LOGLEVEL_DEBUG    4
#define DEBUG_LOGLEVEL_INFO     3
#define DEBUG_LOGLEVEL_WARN     2
#define DEBUG_LOGLEVEL_ERROR    1
#define DEBUG_LOGLEVEL_NONE     0


#if !defined (DEBUG_LOG)
#define DEBUG_LOG DEBUG_LOGLEVEL_ERROR
#endif

#if defined (DEBUG_LOG) && (DEBUG_LOG >= DEBUG_LOGLEVEL_DEBUG)
#define DLogDebug(...) NSLog(@"%s %@", __PRETTY_FUNCTION__, [NSString stringWithFormat:__VA_ARGS__])
#else
#define DLogDebug(...) do { } while (0)
#endif

#if defined (DEBUG_LOG) && (DEBUG_LOG >= DEBUG_LOGLEVEL_INFO)
#define DLogInfo(...) NSLog(@"INFO: %s %@", __PRETTY_FUNCTION__, [NSString stringWithFormat:__VA_ARGS__])
#else
#define DLogInfo(...) do { } while (0)
#endif

#if defined (DEBUG_LOG) && (DEBUG_LOG >= DEBUG_LOGLEVEL_WARN)
#define DLogWarn(...) NSLog(@"WARNING: %s %@", __PRETTY_FUNCTION__, [NSString stringWithFormat:__VA_ARGS__])
#else
#define DLogWarn(...) do { } while (0)
#endif

#if defined (DEBUG_LOG) && (DEBUG_LOG >= DEBUG_LOGLEVEL_ERROR)
#define DLogError(...) NSLog(@"ERROR: %s %@", __PRETTY_FUNCTION__, [NSString stringWithFormat:__VA_ARGS__])
#else
#define DLogError(...) do { } while (0)
#endif



#endif

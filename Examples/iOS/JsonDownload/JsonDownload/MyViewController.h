//
//  MyViewController.h
//  JsonDownload
//
//  Created by Andreas Grosam on 7/6/11.
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

#import <UIKit/UIKit.h>
#import "JPJson/JPAsyncJsonParser.h"
#include <dispatch/dispatch.h>


@class JPNSDataBuffers;
@interface MyViewController : UIViewController
{
    IBOutlet UILabel*   messageLabel_;     
    IBOutlet UIButton*  startDownloadButton_;
    IBOutlet UIActivityIndicatorView* activityIndicatorView_;

    JPNSDataBuffers*    bufferQueue_;
    NSURLConnection*    connection_;    
    BOOL                runLoopDone_;
    dispatch_queue_t    parser_handler_queue_;  // queue used to schedule parser handlers
    
    // stats:
    size_t              totalBytesDownloaded_;  // bytes donwloaded
    size_t              con_number_buffers_;    // number of buffer received by the connection
    size_t              numberJsonDocuments_;  // number of JSON documents parsed per download
    uint64_t            t_start_;
    
}


@property (nonatomic, retain) JPNSDataBuffers* bufferQueue;

- (IBAction) startButtonTapped:(id)sender;


@end

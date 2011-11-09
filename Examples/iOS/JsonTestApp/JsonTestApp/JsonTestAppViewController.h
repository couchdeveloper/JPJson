//
//  JsonTestAppViewController.h
//  JsonTestApp
//
//  Created by Andreas Grosam on 6/30/11.
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

@class JPNSDataBuffers;
@interface JsonTestAppViewController : UIViewController <JPAsyncJsonParserDelegate> 
{
    JPNSDataBuffers*    bufferQueue_;
    JPAsyncJsonParser*  parser_;
    IBOutlet UILabel*   messageLabel_;     
    IBOutlet UIActivityIndicatorView* producerActivityIndicatorView_;
    IBOutlet UIActivityIndicatorView* parserActivityIndicatorView_;
    
    NSMutableArray*     documents_;
    NSUInteger          countDocuments_;
    CFAbsoluteTime      startTime_;
    BOOL                canceledDownload_;
}


@property (nonatomic, retain) JPNSDataBuffers* bufferQueue;

- (IBAction) startButtonTapped:(id)sender;
- (IBAction) cancelButtonTapped:(id)sender;



@end

//
//  JPSemanticActions.mm
//  json_parser
//
//  Created by Andreas Grosam on 7/1/11.
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

#import "JPSemanticActions.h"
#import "JPSemanticActionsBase_private.h"
#include "SemanticActions.hpp"
#include <dispatch/dispatch.h>
#include <assert.h>
#include <sstream>


//
//  Defines the internal semantic actions class 'SemanticActions'
// 
typedef json::objc::SemanticActions<JP_CFStringEncoding>    SemanticActions;





@interface JPSemanticActions()
// As defined in the header file JPSemanticActionsBase_private.h,
// the property imp shall be implemented:
@property (readonly, nonatomic) SemanticActionsBase* imp;
@end



#pragma mark - JPSemanticActions
@implementation JPSemanticActions
{
@private
    SemanticActions*                    sa_;  // the implementation 
    
    // We provide the property "inputEncoding" and "hasBOM" for clients, as 
    // they will probably detect this.
    BOOL                                hasBOM_;
    // TODO: timeout
}


@synthesize hasBOM = hasBOM_;


//
// Desginated Initializer
//
- (id) initWithHandlerDispatchQueue:(dispatch_queue_t)handlerQueue
{
    self = [super initWithHandlerDispatchQueue:handlerQueue];
    if (self) {
        sa_ = new SemanticActions(self);
    }    
    return self;
}

- (void) dealloc 
{
    delete sa_;
    [super dealloc];
}

#pragma mark - 

//@property (readonly, nonatomic) SemanticActionsBase* imp;
- (SemanticActionsBase*) imp {
    assert(sa_);
    return sa_;
}




#pragma mark -

- (void) cancel {
    sa_->cancel();
}

- (void) clear {
    sa_->clear();
}

// property result
- (id) result
{
    id value = sa_->result();
    return value;
}

- (NSString*) inputEncoding {
    assert(self.imp);
    NSString* encoding = [NSString stringWithUTF8String: sa_->inputEncoding().c_str()];
    return encoding;
}

- (void) setHasBOM:(BOOL) set {
    hasBOM_ = set;
}




- (void) configureWithOptions:(JPJsonParserOptions)options
{
    [super configureWithOptions:options];
    
    self.checkForDuplicateKey = JPJsonParserCheckForDuplicateKey & options ? YES : NO ;
    self.keepStringCacheOnClear = JPJsonParserKeepStringCacheOnClear & options ? YES : NO;
    self.cacheDataStrings = JPJsonParserCacheDataStrings & options ? YES : NO;
    self.createMutableContainers = JPJsonParserCreateMutableContainers & options ? YES : NO;
    
    if (JPJsonParserNumberGeneratorOptions & options) {
        if (JPJsonParserNumberGeneratorGenerateAuto & options)
            self.numberGeneratorOption = JPSemanticActionsNumberGeneratorGenerateAuto;
        else if (JPJsonParserNumberGeneratorGenerateStrings & options)
            self.numberGeneratorOption = JPSemanticActionsNumberGeneratorGenerateNSString;
        else if (JPJsonParserGeneratorGenerateDecimals & options)
            self.numberGeneratorOption = JPSemanticActionsNumberGeneratorGenerateNSDecimalNumber;
    }    
}


#pragma mark -


// @property (nonatomic, assign) BOOL checkForDuplicateKey
- (BOOL) checkForDuplicateKey  {
    return (BOOL)sa_->checkDuplicateKey();
}
- (void) setCheckForDuplicateKey:(BOOL)value {
    sa_->checkDuplicateKey(static_cast<bool>(value));
}

// @property (nonatomic, assign) BOOL keepStringCacheOnClear;
- (BOOL) keepStringCacheOnClear  {
    return (BOOL)sa_->keepStringCacheOnClear();
}
- (void) setKeepStringCacheOnClear:(BOOL)value {
    sa_->keepStringCacheOnClear(static_cast<bool>(value));
}

// @property (nonatomic, assign) BOOL cacheDataStrings
- (BOOL) cacheDataStrings  {
    return (BOOL)sa_->cacheDataStrings();
}
- (void) setCacheDataStrings:(BOOL)value {
    sa_->cacheDataStrings(static_cast<bool>(value));
}

// @property (nonatomic, assign) BOOL createMutableContainer;
- (BOOL) createMutableContainers  {
    return (BOOL)sa_->createMutableContainers();
}
- (void) setCreateMutableContainers:(BOOL)value {
    sa_->createMutableContainers(static_cast<bool>(value));
}




- (void) setNumberGeneratorOption:(JPSemanticActionsNumberGeneratorOption)opt
{
    switch (opt) {
        case JPSemanticActionsNumberGeneratorGenerateAuto:
            sa_->numberGeneratorOption(json::objc::sa_options::NumberGeneratorGenerateAuto);
            break;
        case JPSemanticActionsNumberGeneratorGenerateNSString:
            sa_->numberGeneratorOption(json::objc::sa_options::NumberGeneratorGenerateString);
            break;
        case JPSemanticActionsNumberGeneratorGenerateNSDecimalNumber:
            sa_->numberGeneratorOption(json::objc::sa_options::NumberGeneratorGenerateDecimalNumber);
            break;
    }
}

- (JPSemanticActionsNumberGeneratorOption) numberGeneratorOption
{
    switch (sa_->numberGeneratorOption()) {
        case json::objc::sa_options::NumberGeneratorGenerateAuto:
            return JPSemanticActionsNumberGeneratorGenerateAuto;
        case json::objc::sa_options::NumberGeneratorGenerateString:
            return JPSemanticActionsNumberGeneratorGenerateNSString;
        case json::objc::sa_options::NumberGeneratorGenerateDecimalNumber:
            return JPSemanticActionsNumberGeneratorGenerateNSDecimalNumber;
    }
    assert("bad JPSemanticActionsNumberGeneratorOption" == 0);
    return json::objc::sa_options::NumberGeneratorGenerateAuto;
}






@end



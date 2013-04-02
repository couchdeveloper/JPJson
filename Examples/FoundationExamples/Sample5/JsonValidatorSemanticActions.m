//
//  JsonValidatorSemanticActions.m
//  FoundationExamples
//
//  Created by Andreas Grosam on 10/24/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "JsonValidatorSemanticActions.h"




@interface JsonValidatorSemanticActions ()

@property (nonatomic, retain) NSMutableArray* stack;

@end




@implementation JsonValidatorSemanticActions {
    NSMutableArray* stack_;    
}


@synthesize stack = stack_;


- (NSMutableArray*) stack {
    if (stack_ == nil) {
        stack_ = [[NSMutableArray alloc] initWithCapacity:8];
    }
    return stack_;
}

- (void)dealloc {
}

#pragma mark -

- (void) parserFoundJsonObjectBegin {
    NSMutableSet* jsonObject = [[NSMutableSet alloc] initWithCapacity:10];
    [self.stack addObject:jsonObject];
}

- (bool) parserFoundJsonObjectEnd {
    [self.stack removeLastObject];
    return YES;
}

- (void) parserFoundJsonKey:(const void*)bytes length:(size_t)length encoding:(NSStringEncoding)encoding
{
    NSString* key = [[NSString alloc] initWithBytes:bytes length:length encoding:encoding];
    BOOL keyExists = [[self.stack lastObject] member:key] != nil;
    if (keyExists) {
        [self setErrorCode:-1 
               description:[NSString stringWithFormat:@"key '%@' already exists in JSON object", key]];
    } else {
        [[self.stack lastObject] addObject:key];
    }
}



@end

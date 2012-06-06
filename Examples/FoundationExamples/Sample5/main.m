//
//  main.m
//  Sample5
//
//  Created by Andreas Grosam on 10/24/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JPJson/JPJsonParser.h"
#import "JsonValidatorSemanticActions.h"

//
//  Objectives:
//
//  Create a fast validating JSON parser, which also checks for 
//  duplicate key errors.
//


static NSData* readFile(NSString* fileName)
{
    NSError* error;
    NSData* data = [[NSData alloc] initWithContentsOfFile:[@"Resources" stringByAppendingPathComponent:fileName]
                                                  options:NSDataReadingUncached 
                                                    error:&error];    
    if (data == nil) {
        NSLog(@"ERROR: could not load file. %@", error);
        NSFileManager* fileManager = [[NSFileManager alloc] init];
        NSLog(@"current working directory: %@", [fileManager currentDirectoryPath]);
        abort();
    }
    
    return data;
}



int main (int argc, const char * argv[])
{
    @autoreleasepool {
        
        // What we want to do here is to create a parser that just validates
        // a JSON text which also includes a check for a duplicate key error.
        // We accomplish this by subclassing JPStreamSemanticActions, the new
        // class "JsonValidatorSemanticActions".
        JsonValidatorSemanticActions* sa = [[JsonValidatorSemanticActions alloc] initWithHandlerDispatchQueue:NULL];
        
        // Once we created the semantic actions object we may now set various
        // options available for this semantic actions object and which may also
        // affect the parser:
        sa.parseMultipleDocuments = NO;  // this is the default anyway, but just as an example
        sa.logLevel = JPSemanticActionsLogLevelDebug; // log verbose
        
        
        // To test our JSON validator, we create an invalid JSON document:
        // It contains a duplicate key:
        NSString* jsonText = @"{\"key1\" : 1, \"key1\" : 2 }";
        NSData* data = [NSData dataWithBytes:[jsonText UTF8String] 
                                      length:[jsonText lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
                        
                                
        // Now pass the semantic actions object to the parse method:
        BOOL success = [JPJsonParser parseData:data semanticActions:sa];
        
        // As you can see, the return value is BOOL - as opposed to the convenient 
        // method, where it is id, which would return a JSON representation as
        // a hierarchy of Foundation objects.
        
        // Check if the parser has successfully parsed the JSON document provided
        // in data:
        if (!success) {
            // if we failed, the semantic actions object knows why:
            NSError* error = sa.error;
            NSLog(@"Parsing JSON document failed with error: %@", error);
            //return -1;
        }
        
        
        // This kind of semantic actions object did not create a result, but we
        // may check:
        id result = sa.result;
        assert(result == nil);
        
        // If we are curious, we may log the description of the semantic actions
        // object to the console, which reveals some interesting information:
        NSLog(@"\n%@", sa.description);
        
        
        // This validating parser has one caveat though:
        // It won't detect duplicate key errors. 
    }
    return 0;
}


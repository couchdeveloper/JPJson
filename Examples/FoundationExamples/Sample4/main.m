//
//  main.m
//  Sample4
//
//  Created by Andreas Grosam on 10/24/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPStreamSemanticActions.h"

//
//  Objectives:
//
//  Create a fast validating JSON parser.
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
        
        // Read the JSON document from file. The JSON document can be encoded in
        // any Unicode encoding scheme. There may also be a BOM to indicate the
        // encoding. The parser will detect the encoding automatically. So,
        // don't worry about it.
        NSData* data = readFile(@"Test-UTF8-esc.json");
        
        
        // What we want to do here is to create a parser that just validates
        // a JSON text - and this should be fast (a validating parser might be
        // more than 4 times as fast than a parser which also needs to create
        // the JSON repesentation as Foundation objects).
        //
        // We can accomplish this using a specialized semantic actions class.
        // In fact, the base class JPStreamSemanticActions will almost do this.
        // JPStreamSemanticActions provides a "stream API" which is just a
        // set of empty methods which will be invoked by the inernal json
        // parser on various events. Usually, JPStreamSemanticActions should be
        // subclassed to do something useful. The methods provided in the base
        // class JPStreamSemanticActions do (almost) nothing, and this is exactly
        // what we want. We are soley interested in the parser saying: "success".
        
        // Since we do not use handler blocks here in this sample, we need to use the 
        // following init method to explicitly specify NOT to use a disatch queue. 
        // (We will see later how handler blocks work.)
        JPStreamSemanticActions* sa = [[JPStreamSemanticActions alloc] initWithHandlerDispatchQueue:NULL];
        
        // Once we created the semantic actions object we may now set various
        // options available for this semantic actions object and which may also
        // affect the parser:
        sa.parseMultipleDocuments = NO;  // this is the default anyway, but just as an example
        sa.logLevel = JPSemanticActionsLogLevelDebug; // log verbose
        
        
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
            return -1;
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


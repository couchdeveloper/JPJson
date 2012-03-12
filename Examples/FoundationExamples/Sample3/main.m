//
//  main.m
//  Sample3
//
//  Created by Andreas Grosam on 10/24/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPSemanticActions.h"
#import "JPJson/JPJsonWriter.h"
#import "JPJson/NSData+JPJsonDetectEncoding.h"

//
//  Objectives:
//
//  Use the Default Semantic Actions class ”JPSemanticActions”
//  to define the behavior of the parser.
//


static NSData* readFile(NSString* fileName)
{
    NSError* error;
    NSData* data = [[NSData alloc] initWithContentsOfFile:fileName 
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
        
        
        // Create a semantic actions object. Semantic Actions allow to define
        // methods which will be invoked when the parser successfully parsed a
        // certain rule, that is when the parser detects JSON elements.
        // Semantic Actions are a powerful tool to customize your needs, since 
        // Semantic Actions can do anything you want and there is the possibility
        // to define your own semantic actions classes.
        //
        // Here we want to create a semantic actions object which creates a JSON
        // representation which is built upon Foundation objects. This is the 
        // "default" semantic actions object which is the same semantic actions 
        // class which is used in the convenient class method +parseData:options:error:.
        // The class is JPSemanticActions, and part of the library.
        //
        // Since we do not use handler blocks here in this sample, we need to use the 
        // following init method to explicitly specify NOT to use a disatch queue. 
        // (We will see later how handler blocks work.)
        JPSemanticActions* sa = [[JPSemanticActions alloc] initWithHandlerDispatchQueue:NULL];
        
        // Once we created the semantic actions object we may now set various
        // options available for this semantic actions object and which may also
        // affect the parser:
        sa.parseMultipleDocuments = NO;  // this is the default anyway, but just as an example
        sa.logLevel = JPSemanticActionsLogLevelDebug; // log verbose
        sa.createMutableContainers = YES; // create mutable NSArray and mutable NSDictionaries.
        // There are a couple more options which set various aspects of how the
        // json parser is configured, and how the semantic actions object can be
        // tailored. For more details, see the description of class JPSemanticActions 
        // and class JPSemanticActionsBase.
        
        
        // Now pass the semantic actions object to the parse method:
        BOOL success = [JPJsonParser parseData:data semanticActions:sa];
        
        // As you can see, the return value is BOOL - as opposed to the convenient 
        // method, where it is id, the JSON representation.
        
        // Check if the parser was successfully parsed the JSON document provided
        // in data:
        if (!success) {
            // if we failed, the semantic actions object knows why:
            NSError* error = sa.error;
            NSLog(@"Parsing JSON document failed with error: %@", error);
            return -1;
        }
        
        
        // In order to get the result of the whole operation, ask the semantic actions
        // object:
        id result = sa.result;
        
        // If we are curious, we may log the description of the semantic actions
        // object to the console, which reveals some interesting information:
        NSLog(@"\n%@", sa.description);
        
        // 'result' is solely retained by the semantic actions object. In order to 
        // maintain a reference to the result, we need to keep the semantic
        // actions object alife as long as we need the result, or alternatively,
        // retain/autorelease it:
        result = [[result retain] autorelease];
        [sa release], sa = nil;
        
        // If parsing was successful, result shall not be nil:
        assert(result);
        

        // We convert the JSON representation provided in variable 'result' to a 
        // JSON text again:
        NSError* error;
        NSData* jsonData = [JPJsonWriter dataWithObject:result
                                               encoding:JPUnicodeEncoding_UTF8
                                                options:(JPJsonWriterOptions)(JPJsonWriterEscapeUnicodeCharacters)
                                                  error:&error];
        if (!jsonData) {
            NSLog(@"Extracting JSON into string failed with error: %@", error);
            return -1;
        }
        else {
            NSError* error;
            NSString* filePath = @"Out-UTF8.json";
            BOOL result = [jsonData writeToFile:filePath options:0 error:&error];
            if (!result) {
                NSLog(@"Writing JSON document to file '%@' failed with error: %@", filePath, error);
            }
        }
        
        // We convert the JSON representation provided in variable 'result' to a 
        // JSON text again, now using pritty printing:
        NSData* jsonDataPretty = [JPJsonWriter dataWithObject:result
                                               encoding:JPUnicodeEncoding_UTF8
                                                options:(JPJsonWriterOptions)(JPJsonWriterPrettyPrint|JPJsonWriterEscapeUnicodeCharacters)
                                                  error:&error];
        if (!jsonDataPretty) {
            NSLog(@"Extracting JSON into string failed with error: %@", error);
            return -1;
        }
        else {
            NSError* error;
            NSString* filePath = @"Out-UTF8-pretty.json";
            BOOL result = [jsonDataPretty writeToFile:filePath options:0 error:&error];
            if (!result) {
                NSLog(@"Writing JSON document to file '%@' failed with error: %@", filePath, error);
            }
        }
    }
    return 0;
}


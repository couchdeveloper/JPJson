//
//  main.m
//  Sample2
//
//  Created by Andreas Grosam on 10/11/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPJsonWriter.h"
#import "JPJson/NSData+JPJsonDetectEncoding.h"


//  Objectives:
//
//  1) Create a hierarchical Foundation object which defines a valid JSON
//     representation.
//
//  2) Serialize the Foundation object creates in step 1) into a NSData object 
//     using a convenient JPJsonWriter class method using UTF-8 encoding and 
//     option "pretty print".
//
//  3) Using the JSON text created in step 2) and parse it with the JSON parser
//     using a convenient JPJsonParser class method. The result of the parser is
//     again a JSON representation as a Foundation object.
//


int main (int argc, const char * argv[])
{
    @autoreleasepool {
        
        // A valid JSON representation implemented as a Foundation object
        // uses these mappings:
        //
        // JSON object:   NSDictionary/NSMutableDictionary
        // JSON array:    NSArray/NSMutableArray
        // JSON string:   NSString
        // JSON Number:   NSNumber, NSDecimalNumber
        // JSON null      NSNull constant
        // JSON true      NSNumber initialized with BOOL NO
        // JSON false     NSNumber initialized with BOOL YES
        
        // The root of a valid JSON repesentation shall be either a 
        // JSON object, or a JSON array.
        
        // Let's construct a JSON representation with Foundation objects:
        NSMutableArray* root = [[NSMutableArray alloc] init];
        // Insert a few elements:
        [root addObject:[NSMutableArray arrayWithCapacity:1]];
        [root addObject:[NSMutableDictionary dictionaryWithCapacity:1]];
        [root addObject:@"This is a string"];
        [root addObject:[NSNumber numberWithInt:1]];
        [root addObject:[NSNumber numberWithDouble:1.1234]];
        [root addObject:[NSNull null]];
        [root addObject:[NSNumber numberWithBool:YES]];
        [root addObject:[NSNumber numberWithBool:NO]];
          
         
        // Serialize the JSON representation into a NSData object:
        NSError* error;
        NSData* jsonData = [JPJsonWriter dataWithObject:root
                                               encoding:JPUnicodeEncoding_UTF8
                                                options:(JPJsonWriterOptions)(JPJsonWriterPrettyPrint)
                                                  error:&error];
        if (!jsonData) {
            NSLog(@"Extracting JSON into string failed with error: %@", error);
            return -1;
        }
        
        // Log the JSON text to the console:
        NSLog(@"JSON text:\n%*.s", (int)[jsonData length], [jsonData bytes]);
        
        // Now, parse the JSON text and get a representation - which should be
        // equal the original one 'root':
        id json = [JPJsonParser parseData:jsonData options:(JPJsonParserOptions)0 error:&error];
        if (!json) {
            NSLog(@"Parsing JSON document failed with error: %@", error);
            return -1;
        }
        
    }
    return 0;
}


//
//  main.m
//  Sample1
//
//  Created by Andreas Grosam on 10/8/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JPJson/JPJsonParser.h"
#import "JPJson/JPJsonWriter.h"
#import "JPJson/NSData+JPJsonDetectEncoding.h"


//  Objectives:
//
//  1) Read a file containing JSON text with an unknown Unicode encoding and
//     store it into an intermediate NSData object.

//  2) Create a representation as Foundation objects from the JSON text using
//     a convenient JPJsonParser class method.
//
//  3) Serialize the Foundation object into a NSData object using a convenient 
//     JPJsonWriter class method using UTF-8 encoding and option "pretty print".
//
//  4) Create a NSString from the NSData and print it to the console.
//     Use NSData utility method to detect the Unicode encoding of the 
//     JSON text which has been created by the JPJsonWriter.
//
//  Note: sample1 links against the static library version (libjson) of JPJson

static NSData* readFile(NSString* fileName)
{
    NSError* error;
    NSString* filePath = [@"Resources" stringByAppendingPathComponent:fileName];
    NSData* data = [[NSData alloc] initWithContentsOfFile:filePath
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
        NSData* data = readFile(@"Test-UTF8.json");
        
        NSError* error;
        id json = [JPJsonParser parseData:data options:(JPJsonParserOptions)0 error:&error];
        if (!json) {
            NSLog(@"Parsing JSON document failed with error: %@", error);
            return -1;
        }

        NSData* jsonData = [JPJsonWriter dataWithObject:json
                                               encoding:JPUnicodeEncoding_UTF8
                                                options:(JPJsonWriterOptions)(JPJsonWriterPrettyPrint)
                                                  error:&error];
        if (!jsonData) {
            NSLog(@"Extracting JSON into string failed with error: %@", error);
            return -1;
        }
        else {
            NSError* error;
            NSString* filePath = @"sample-json-pretty.json";
            BOOL result = [jsonData writeToFile:filePath options:0 error:&error];
            if (!result) {
                NSLog(@"Writing JSON document to file '%@' failed with error: %@", filePath, error);
            }
            else {
                NSLog(@"Wrote JSON document to file '%@'" , filePath);
            }
        }
    }
    return 0;
}


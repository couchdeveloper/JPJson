//
//  main.m
//  Sample6
//
//  Created by Andreas Grosam on 5/22/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#if !__has_feature(objc_arc) 
#error This Objective-C file shall be compiled with ARC enabled.
#endif


#import <Foundation/Foundation.h>
#import "JPJson/JPJsonWriter.h"
#import "JPJson/JPJsonWriterExtensions.h"


//  Objectives:
//
//  1) Create a custom class which can be serialized by JPJsonWriter


// Helper category for creating a RFC3339DateFormatter
@implementation NSDateFormatter (RFC3339DateFormatter) 
+(NSDateFormatter*) rfc3339DateFormatter {
    NSDateFormatter* rfc3339DateFormatter = [[NSDateFormatter alloc] init];
    NSLocale* enUSPOSIXLocale = [[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
    [rfc3339DateFormatter setLocale:enUSPOSIXLocale];
    [rfc3339DateFormatter setDateFormat:@"yyyy'-'MM'-'dd'T'HH':'mm':'ss'Z'"];
    [rfc3339DateFormatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
    return rfc3339DateFormatter;
}
@end



//
// Serializing a NSDate
//
// An NSDate object cannot be serialized directly with a JPJsonWriter. Thus, the
// occurence of an NSDate object within a hierarchy of objects which should
// be serialzed to JSON will not work by default.
//
// A NSDate object may be serialized as a formatted date string. So, in order to
// make this work - we just ned to define a Category for NSDate and implement the
// JPJsonSerializableProtocol. 
//
// Serializing a NSDate involves to convert the date into a string with an
// appropriate format. The format purposfully meets RFC 3339. Then we just serialize
// the string using the JPJsonSerializableProtocol. The JPJsonSerializableProtocol 
// is already implemented for a NSString by the JPJson library.
//
@interface NSDate (JPJsonWriterExtension) <JPJsonSerializableProtocol>
@end
@implementation NSDate (JPJsonWriterExtension) 
- (NSInteger) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>) buffer 
                        encoding:(JPUnicodeEncoding)encoding 
                         options:(JPJsonWriterOptions)options 
                           level:(NSUInteger)level 
{
    NSDateFormatter* rfc3339DateFormatter = [NSDateFormatter rfc3339DateFormatter];
    id<JPJsonSerializableProtocol> dateString = (id<JPJsonSerializableProtocol>)[rfc3339DateFormatter stringFromDate:self];    
    return [dateString JPJson_serializeTo:buffer encoding:encoding options:options level:level];    
}
@end



//
// A custom class `Person` having four properties, one of them is an array.
//
@interface Person : NSObject
- (id) initWithFirstName:(NSString*)firstName lastName:(NSString*)lastName dayOfBirth:(NSDate*)dob;
@property (nonatomic, retain) NSString* firstName;
@property (nonatomic, retain) NSString* lastName;
@property (nonatomic, retain) NSDate* dayOfBirth;
@property (nonatomic, readonly, retain) NSArray* comments;
- (void) addComment:(NSString*)comment;
@end


@implementation Person  {
    NSString*   _firstName;
    NSString*   _lastName;
    NSDate*     _dayOfBirth;
    NSMutableArray* _comments;
}
- (id) initWithFirstName:(NSString*)firstName lastName:(NSString*)lastName dayOfBirth:(NSDate*)dob
{
    self = [super init];
    if (self) {
        _firstName = firstName;
        _lastName = lastName;
        _dayOfBirth = dob;
    }
    return self;
}
@synthesize firstName = _firstName;
@synthesize lastName = _lastName;
@synthesize dayOfBirth = _dayOfBirth;
@synthesize comments = _comments;

- (void) addComment:(NSString*) comment
{
    if (comment == nil) 
        return;
    
    if (_comments == nil) {
        _comments = [[NSMutableArray alloc] initWithCapacity:12];
    }
    [_comments addObject:comment];
}

@end



//
//  Serializing a Custom Class
//
// The following class `Person` shall be serialized directly using the `JPJsonWriter`.
// The class `Person` shall be mapped to a JSON Object - that is a list of key-value
// pairs, where the key are comprised by the properties.
// For this purpose the implementation utilizes the JPJsonWriter class method
// `serializeObjectAsJSONObject:buffer:encoding:options:level:`, which in  turn
// requires that the class Person implements the `NSFastEnumeration` protocol,
// the JPJsonSerializableProtocol, and as well responds to message `count` and 
// message `objectForKey:`
//
@interface Person (JPJsonWriterExtension) <JPJsonSerializableProtocol, NSFastEnumeration>
@end
@implementation Person (JPJsonWriterExtension) 

- (id) objectForKey:(id)aKey {
    return [self valueForKey:aKey];
}

- (NSUInteger) count {
    return 4;  // return the number of properies
}

- (NSUInteger) countByEnumeratingWithState:(NSFastEnumerationState *)state 
                                   objects:(id __unsafe_unretained [])buffer 
                                     count:(NSUInteger)len
{
    static id __unsafe_unretained properties[] = {@"firstName", @"lastName", @"dayOfBirth", @"comments"};
    
    if (state->state >= 4) {
        return 0;
    }    
    state->itemsPtr = properties;
    state->state = 4;
    state->mutationsPtr = &state->extra[0];
    return 4;
}

- (NSInteger) JPJson_serializeTo:(id<JPJsonStreambufferProtocol>) buffer 
                        encoding:(JPUnicodeEncoding)encoding 
                         options:(JPJsonWriterOptions)options 
                           level:(NSUInteger)level 
{
    return [JPJsonWriter serializeObjectAsJSONObject:self buffer:buffer encoding:encoding options:options level:level];    
}

@end









int main (int argc, const char * argv[])
{
    @autoreleasepool {

        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setLocale:[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"]];
        [dateFormatter setDateFormat:@"yyyy'-'MM'-'dd"];
        
        // Create an array of Persons:
        NSArray* json = [[NSArray alloc] initWithObjects:
                         [[Person alloc] initWithFirstName:@"John" 
                                                  lastName:@"Appleseed" 
                                                dayOfBirth:[dateFormatter dateFromString:@"1962-04-21"]],
                          [[Person alloc] initWithFirstName:@"Pete" 
                                                   lastName:@"Pearson" 
                                                 dayOfBirth:[dateFormatter dateFromString:@"1912-10-12"]],
                          [[Person alloc] initWithFirstName:@"Tab" 
                                                   lastName:@"Benoit" 
                                                 dayOfBirth:[dateFormatter dateFromString:@"1970-03-09"]],
                          nil];
        
        Person* p0 = [json objectAtIndex:0];
        [p0 addComment:@"This is a comment"];
        [p0 addComment:@"This is another comment"];
        
        NSError* error;
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
            NSString* filePath = @"persons-json-pretty.json";
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


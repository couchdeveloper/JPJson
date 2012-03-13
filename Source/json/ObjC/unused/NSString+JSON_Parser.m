//
//  NSString+JSON_Parser.m
//  json_parser
//
//  Created by Andreas Grosam on 6/17/11.
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

#warning deprecated file


#import "NSString+JSON_Parser.h"
#import "json_parse.h"

#error Not yet implemented


@implementation NSString (NSString_JSON_Parser)


- (id)objectFromJSONString 
{
    id result = json_parse(self, NULL);
    return result;
}




- (id)objectFromJSONString:(NSError **)error
{
    id result = json_parse(self, error);
    return result;
}




@end

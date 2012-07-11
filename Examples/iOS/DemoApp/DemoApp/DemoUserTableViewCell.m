//
//  DemoUserTableViewCell.m
//  DemoApp
//
//  Created by Andreas Grosam on 26.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "DemoUserTableViewCell.h"

@interface DemoUserTableViewCell ()


@end

@implementation DemoUserTableViewCell

@synthesize nameLabel;
@synthesize emailLabel;
@synthesize statusLabel;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
    }
    return self;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end

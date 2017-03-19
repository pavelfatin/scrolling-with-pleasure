#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

int main(int argc, const char *argv[]) {
    NSApplication *application = [NSApplication sharedApplication];

    AppDelegate *applicationDelegate = [[AppDelegate alloc] init];
    [application setDelegate:applicationDelegate];
    [application run];

    return 0;
}

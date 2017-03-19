#import "AppDelegate.h"
#import "MyView.h"

@implementation AppDelegate

@synthesize window;

MyView * view;

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

- (void)dealloc {
    [super dealloc];
}

-(id)init {
    if(self = [super init]) {
        NSRect contentSize = NSMakeRect(200.0, 400.0, 640.0, 480.0);
        NSUInteger windowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
        window = [[NSWindow alloc] initWithContentRect:contentSize styleMask:windowStyleMask backing:NSBackingStoreBuffered defer:YES];
        window.backgroundColor = [NSColor whiteColor];
        window.title = @"Scrolling logger";

[window makeKeyAndOrderFront:NSApp];

        view = [[MyView alloc] initWithFrame:CGRectMake(0, 0, 640, 480)];
        NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[view frame] options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways owner:view userInfo:nil];
        [view addTrackingArea:area];
    }
    return self;
 }

-(void)applicationWillFinishLaunching:(NSNotification *)notification {
    [window setContentView:view];
}

-(void)applicationDidFinishLaunching:(NSNotification *)notification {
    [window makeKeyAndOrderFront:self];
}

@end

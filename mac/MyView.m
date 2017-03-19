#include <mach/mach_time.h>
#include <inttypes.h>
#import "MyView.h"

@implementation MyView

bool momentumScrolling = false;

-(void)scrollWheel:(NSEvent *)theEvent {
  NSEventPhase momentumPhase = [theEvent momentumPhase];

  if (momentumPhase == NSEventPhaseBegan) {
    momentumScrolling = true;
  }

  if (momentumPhase == NSEventPhaseEnded) {
    momentumScrolling = false;
  }

  if ([theEvent hasPreciseScrollingDeltas]) {
    int dY = [theEvent scrollingDeltaY];

    if (dY != 0.0) {
      uint64_t time = mach_absolute_time();
      printf("%" PRIu64 "\t%d\t%d\n", time, momentumScrolling ? 1 : 0, dY);
    }
  }
}

@end

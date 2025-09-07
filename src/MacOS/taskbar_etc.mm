// clang++ -std=c++17 -ObjC++ -framework Foundation -framework AppKit -framework CoreGraphics MonitorInfo.mm -o MonitorInfo

// MonitorInfo.mm
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>

#include <vector>
#include <string>

struct MonitorInfo {
  NSRect      frame;
  NSRect      visibleFrame;
  NSRect      menuBarFrame;
  NSRect      dockFrame;
  std::string dockPosition;
  CGFloat     dockSize;
};

static std::vector<MonitorInfo> allMonitorsWithDockInfo() {
  // bootstrap AppKit in a CLI tool
  NSApplicationLoad();

  // 1) enumerate screens and infer gaps
  std::vector<MonitorInfo> out;
  for (NSScreen *scr in [NSScreen screens]) {
    MonitorInfo mi;
    mi.frame        = scr.frame;
    mi.visibleFrame = scr.visibleFrame;
    mi.dockFrame    = NSZeroRect;
    mi.menuBarFrame = NSZeroRect;
    mi.dockPosition = "none";
    mi.dockSize     = 0;

    // compute edge‐gaps
    CGFloat leftGap   = NSMinX(mi.visibleFrame) - NSMinX(mi.frame);
    CGFloat bottomGap = NSMinY(mi.visibleFrame) - NSMinY(mi.frame);
    CGFloat rightGap  = NSMaxX(mi.frame)        - NSMaxX(mi.visibleFrame);
    CGFloat topGap    = NSMaxY(mi.frame)        - NSMaxY(mi.visibleFrame);

    // infer Dock position + thickness
    if      (bottomGap > 0) { mi.dockPosition = "bottom"; mi.dockSize = bottomGap; }
    else if (leftGap   > 0) { mi.dockPosition = "left";   mi.dockSize = leftGap;   }
    else if (rightGap  > 0) { mi.dockPosition = "right";  mi.dockSize = rightGap;  }

    // compute menu‐bar rect (full‐width strip at top)
    if (topGap > 0) {
      mi.menuBarFrame = NSMakeRect(
        mi.frame.origin.x,
        NSMaxY(mi.frame) - topGap,
        mi.frame.size.width,
        topGap
      );
    }

    out.push_back(mi);
  }

  // 2) query on‐screen windows and pick the one at Dock‐level
  CFArrayRef infoList = CGWindowListCopyWindowInfo(
    kCGWindowListOptionOnScreenOnly,
    kCGNullWindowID
  );
  if (infoList) {
    NSArray *wins = CFBridgingRelease(infoList);
    // the official Dock‐level
    int dockLevel = CGWindowLevelForKey(kCGDockWindowLevelKey);
    for (NSDictionary *w in wins) {
      if (![w[(id)kCGWindowOwnerName] isEqualToString:@"Dock"])
        continue;
      NSNumber *layer = w[(id)kCGWindowLayer];
      if (!layer || [layer intValue] != dockLevel)
        continue;

      NSDictionary *bounds = w[(id)kCGWindowBounds];
      if (!bounds) continue;
      CGRect r = CGRectMake(
        [bounds[@"X"]     doubleValue],
        [bounds[@"Y"]     doubleValue],
        [bounds[@"Width"] doubleValue],
        [bounds[@"Height"] doubleValue]
      );
      NSRect dockRect = NSRectFromCGRect(r);

      // assign to whichever monitor it overlaps
      for (auto &mi : out) {
        if (NSIntersectsRect(mi.frame, dockRect)) {
          mi.dockFrame = dockRect;
        }
      }
      // once we’ve found the Dock window, no need to keep searching
      break;
    }
  }

  // 3) fallback: if for some reason we still have no dockFrame, compute it geometrically
  for (auto &mi : out) {
    if (mi.dockFrame.size.width == 0 && mi.dockFrame.size.height == 0 &&
        mi.dockPosition != "none")
    {
      if (mi.dockPosition == "bottom") {
        mi.dockFrame = NSMakeRect(
          mi.frame.origin.x,
          mi.frame.origin.y,
          mi.frame.size.width,
          mi.dockSize
        );
      }
      else if (mi.dockPosition == "left") {
        mi.dockFrame = NSMakeRect(
          mi.frame.origin.x,
          mi.frame.origin.y,
          mi.dockSize,
          mi.frame.size.height
        );
      }
      else if (mi.dockPosition == "right") {
        mi.dockFrame = NSMakeRect(
          NSMaxX(mi.frame) - mi.dockSize,
          mi.frame.origin.y,
          mi.dockSize,
          mi.frame.size.height
        );
      }
    }
  }

  return out;
}

int main(int argc, const char * argv[]) {
  @autoreleasepool {
    auto monitors = allMonitorsWithDockInfo();
    for (size_t i = 0; i < monitors.size(); ++i) {
      const auto &m = monitors[i];
      NSLog(@"Monitor %zu:", i+1);
      NSLog(@"  Full frame     = %@",   NSStringFromRect(m.frame));
      NSLog(@"  Visible frame  = %@",   NSStringFromRect(m.visibleFrame));
      NSLog(@"  Menu bar rect  = %@",   NSStringFromRect(m.menuBarFrame));
      NSLog(@"  Dock position  = %s",   m.dockPosition.c_str());
      if (m.dockPosition != "none") {
        NSLog(@"  Dock inferred size = %.0f", m.dockSize);
        NSLog(@"  Dock rect            = %@",   NSStringFromRect(m.dockFrame));
      }
    }
  }
  return 0;
}

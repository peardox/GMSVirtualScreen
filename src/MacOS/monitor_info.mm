/* Build command
clang++ -std=c++17 monitor_info.mm -o monitor_app -framework CoreGraphics -framework Foundation -framework AppKit
*/
/* Sample output
Detected macOS Monitors:
----------------------------------------
Display Name:     PHL 275V8
Display ID:       3
Primary Display:  True
Rect:             {0, 0, 2560, 1440}
Pixel Resolution: 2560 x 1440 pixels
Physical Size:    602.1 mm x 338.7 mm
Refresh Rate:     75.0
Approx. Diagonal: 27.2 inches
----------------------------------------
Display Name:     LG TV SSCR2
Display ID:       2
Primary Display:  False
Rect:             {0.0, -1692.0, 3008.0, 1692.0}
Pixel Resolution: 3008 x 1692 pixels
Physical Size:    1591.7 mm x 895.3 mm
Refresh Rate:     30.0
Approx. Diagonal: 71.9 inches
*/

#import <iostream>
#import <vector>
#import <string>
#import <iomanip>
#import <cmath>

// Import the AppKit framework for NSScreen
#import <AppKit/AppKit.h>

// Structure to hold monitor information
struct MonitorInfo {
    CGDirectDisplayID displayID;
    std::string displayName;
    CGRect monitorRect;
    Boolean isPrimary;
    double refreshRate;
    size_t widthPixels;
    size_t heightPixels;
    double physicalWidthMM;
    double physicalHeightMM;
};

// Helper function to get the display name using the modern AppKit framework
std::string getDisplayName(CGDirectDisplayID displayID) {
    // The @autoreleasepool is necessary for managing memory in Objective-C code.
    @autoreleasepool {
        // Iterate through all available screens
        for (NSScreen *screen in [NSScreen screens]) {
            // Get the device description dictionary for the screen
            NSDictionary *deviceDescription = [screen deviceDescription];
            // Get the screen number, which is the CGDirectDisplayID
            NSNumber *screenNumber = [deviceDescription objectForKey:@"NSScreenNumber"];

            if ([screenNumber unsignedIntValue] == displayID) {
                // If the IDs match, get the localized name
                NSString *name = [screen localizedName];
                if (name) {
                    // Convert the Objective-C NSString to a C++ std::string
                    return std::string([name UTF8String]);
                }
            }
        }
    }
    return "Unknown"; // Fallback if no match is found
}

int main() {
    std::vector<MonitorInfo> monitors;
    CGDirectDisplayID displayIDs[16];
    uint32_t numDisplays;

    CGError err = CGGetActiveDisplayList(16, displayIDs, &numDisplays);
    if (err != kCGErrorSuccess) {
        std::cerr << "Error: Could not get active display list. Error code: " << err << "\n";
        return 1;
    }

    if (numDisplays == 0) {
        std::cout << "No monitors found.\n";
        return 1;
    }

    std::cout << "Detected macOS Monitors:\n";
    for (uint32_t i = 0; i < numDisplays; ++i) {
        CGDirectDisplayID displayID = displayIDs[i];
        MonitorInfo currentMonitor;
        currentMonitor.displayID = displayID;
        currentMonitor.displayName = getDisplayName(displayID);
        currentMonitor.isPrimary = CGDisplayIsMain(displayID);
        currentMonitor.monitorRect = CGDisplayBounds(displayID);

        CGDisplayModeRef dmr = CGDisplayCopyDisplayMode(displayID);
        if (dmr) {
            currentMonitor.refreshRate = CGDisplayModeGetRefreshRate(dmr);
            currentMonitor.widthPixels = CGDisplayModeGetWidth(dmr);
            currentMonitor.heightPixels = CGDisplayModeGetHeight(dmr);
            CGDisplayModeRelease(dmr);
        }

        CGSize physicalSize = CGDisplayScreenSize(displayID);
        currentMonitor.physicalWidthMM = physicalSize.width;
        currentMonitor.physicalHeightMM = physicalSize.height;
        monitors.push_back(currentMonitor);
    }

    for (const auto &mon : monitors) {
        std::cout << "----------------------------------------\n";
        std::cout << "Display Name:     " << mon.displayName << "\n";
        std::cout << "Display ID:       " << mon.displayID << "\n";
        std::cout << "Primary Display:  " << (mon.isPrimary ? "True\n" : "False\n");
        std::cout << "Rect:             {" << mon.monitorRect.origin.x << ", " << mon.monitorRect.origin.y << ", " << mon.monitorRect.size.width << ", " << mon.monitorRect.size.height << "}\n";
        std::cout << "Pixel Resolution: " << mon.widthPixels << " x " << mon.heightPixels << " pixels\n";
        std::cout << "Physical Size:    " << std::fixed << std::setprecision(1) << mon.physicalWidthMM << " mm x " << mon.physicalHeightMM << " mm\n";
        std::cout << "Refresh Rate:     " << mon.refreshRate << "\n";
        if (mon.physicalWidthMM > 0 && mon.physicalHeightMM > 0) {
            double widthInches = mon.physicalWidthMM / 25.4;
            double heightInches = mon.physicalHeightMM / 25.4;
            double diagonalInches = sqrt(pow(widthInches, 2) + pow(heightInches, 2));
            std::cout << "Approx. Diagonal: " << std::fixed << std::setprecision(1) << diagonalInches << " inches\n";
        } else {
            std::cout << "Physical size not available for diagonal calculation.\n";
        }
    }
    std::cout << "----------------------------------------\n";
    return 0;
}

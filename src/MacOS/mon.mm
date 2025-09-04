#include <iostream>
#include <vector>
#include <string>
#include <iomanip> // For std::fixed and std::setprecision
#include <cmath>   // For sqrt and pow

// Include Core Graphics framework. This is a C-style header.
#include <ApplicationServices/ApplicationServices.h>

// Structure to hold monitor information
struct MonitorInfo {
    CGDirectDisplayID displayID; // Unique ID for the display
    CGRect monitorRect;          // Rect for display
    Boolean isPrimary;           // Is this the primary display?
    Boolean isAsleep;		 // Display is asleep
    double refreshRate;          // Refresh Rate
    size_t widthPixels;          // Width in pixels
    size_t heightPixels;         // Height in pixels
    double physicalWidthMM;      // Physical width in millimeters
    double physicalHeightMM;     // Physical height in millimeters
};

int main() {
    std::vector<MonitorInfo> monitors;

    // Get a list of all active displays
    CGDirectDisplayID displayIDs[16]; // Allocate space for up to 16 displays
    uint32_t numDisplays;

    // CGGetActiveDisplayList returns the number of active displays
    // and fills the displayIDs array.
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

        // --- Get pixel dimensions (resolution) ---
        // CGDisplayPixelsWide and CGDisplayPixelsHigh return the native pixel resolution.
       currentMonitor.isPrimary = CGDisplayIsMain(displayID);
       currentMonitor.monitorRect = CGDisplayBounds(displayID);
       currentMonitor.isAsleep = CGDisplayIsAsleep(displayID);
        // --- Display Mode Values ---
        CGDisplayModeRef dmr = CGDisplayCopyDisplayMode(displayID);
        currentMonitor.refreshRate = CGDisplayModeGetRefreshRate(dmr);
        currentMonitor.widthPixels = CGDisplayModeGetWidth(dmr);
        currentMonitor.heightPixels = CGDisplayModeGetHeight(dmr);

        // --- Get physical dimensions in millimeters ---
        // CGDisplayScreenSize returns a CGSize containing width and height in millimeters.
        CGSize physicalSize = CGDisplayScreenSize(displayID);
        
        currentMonitor.physicalWidthMM = physicalSize.width;
        currentMonitor.physicalHeightMM = physicalSize.height;

        monitors.push_back(currentMonitor);
    }

    for (const auto& mon : monitors) {
        std::cout << "----------------------------------------\n";
        std::cout << "Display ID:       " << mon.displayID << "\n";
	std::cout << "Primary Display:  " << (mon.isPrimary ? "True\n" : "False\n");
        std::cout << "Sleeping:         " << (mon.isAsleep ? "True\n" : "False\n");

        std::cout << "Rect:             {" << mon.monitorRect.origin.x << ", "
                                << mon.monitorRect.origin.y << ", "
                                << mon.monitorRect.size.width << ", "
                                << mon.monitorRect.size.height << "}\n";

        std::cout << "Pixel Resolution: " << mon.widthPixels << " x "
                  << mon.heightPixels << " pixels\n";
        std::cout << "Physical Size:    " << std::fixed << std::setprecision(1)
                  << mon.physicalWidthMM << " mm x " << mon.physicalHeightMM
                  << " mm\n";
        std::cout << "Refresh Rate:     " << mon.refreshRate << "\n";

        // Calculate and display approximate diagonal size in inches
        if (mon.physicalWidthMM > 0 && mon.physicalHeightMM > 0) {
            // Convert mm to inches (1 inch = 25.4 mm)
            double widthInches = mon.physicalWidthMM / 25.4;
            double heightInches = mon.physicalHeightMM / 25.4;

            // Calculate diagonal using Pythagorean theorem
            double diagonalInches = sqrt(pow(widthInches, 2) + pow(heightInches, 2));
            std::cout << "Approx. Diagonal: " << std::fixed
                      << std::setprecision(1) << diagonalInches << " inches\n";
        } else {
            std::cout << "Physical size not available for diagonal calculation.\n";
        }
    }
    std::cout << "----------------------------------------\n";

    return 0;
}

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

#pragma comment(lib, "user32.lib")

// EDID structure for parsing display name
struct EDIDData {
    unsigned char header[8];
    unsigned char manufacturerID[2];
    unsigned char productID[2];
    unsigned char serialNumber[4];
    unsigned char weekOfManufacture;
    unsigned char yearOfManufacture;
    unsigned char edidVersion;
    unsigned char edidRevision;
    unsigned char videoInputDefinition;
    unsigned char maxHorizontalSize;
    unsigned char maxVerticalSize;
    unsigned char displayGamma;
    unsigned char powerManagement;
    unsigned char chromaticity[10];
    unsigned char timingBitmap[3];
    unsigned char reservedTiming[16];
    unsigned char descriptorBlocks[72];
    unsigned char extensionFlag;
    unsigned char checksum;
};

std::string parseManufacturerID(const unsigned char* data) {
    unsigned short id = (data[0] << 8) | data[1];
    char manufacturer[4] = {0};
    
    manufacturer[0] = 'A' + ((id >> 10) & 0x1F) - 1;
    manufacturer[1] = 'A' + ((id >> 5) & 0x1F) - 1;
    manufacturer[2] = 'A' + (id & 0x1F) - 1;
    
    return std::string(manufacturer);
}

std::string parseProductName(const unsigned char* edidData) {
    // Look for product name in descriptor blocks
    for (int i = 0; i < 4; i++) {
        unsigned char* descriptor = 
            (unsigned char*)edidData + 54 + (i * 18);
        
        // Check if this is a display product name descriptor (type 0xFC)
        if (descriptor[0] == 0x00 && descriptor[1] == 0x00 && 
            descriptor[2] == 0x00 && descriptor[3] == 0xFC) {
            
            std::string name;
            for (int j = 5; j < 18; j++) {
                if (descriptor[j] == 0x0A || descriptor[j] == 0x00) 
                    break;
                name += static_cast<char>(descriptor[j]);
            }
            return name;
        }
    }
    return "Unknown Display";
}

std::string getDisplayName(LUID adapterId, UINT32 targetId) {
    DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
    targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
    targetName.header.size = sizeof(targetName);
    targetName.header.adapterId = adapterId;
    targetName.header.id = targetId;
    
    LONG result = DisplayConfigGetDeviceInfo(&targetName.header);
    if (result == ERROR_SUCCESS) {
        // Convert wide string to regular string
        int len = WideCharToMultiByte(CP_UTF8, 0, 
            targetName.monitorFriendlyDeviceName, -1, 
            nullptr, 0, nullptr, nullptr);
        std::string name(len, 0);
        WideCharToMultiByte(CP_UTF8, 0, 
            targetName.monitorFriendlyDeviceName, -1, 
            &name[0], len, nullptr, nullptr);
        name.pop_back(); // Remove null terminator
        return name;
    }
    
    return "Unknown Display";
}

std::string getEDIDInfo(LUID adapterId, UINT32 targetId) {
    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO colorInfo = {};
    colorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
    colorInfo.header.size = sizeof(colorInfo);
    colorInfo.header.adapterId = adapterId;
    colorInfo.header.id = targetId;
    
    // Try to get EDID data
    DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS nameFlags = {};
    nameFlags.friendlyNameFromEdid = 1;
    
    return getDisplayName(adapterId, targetId);
}

int main() {
    std::cout << "Enumerating attached displays using EDID data:\n";
    std::cout << std::string(50, '=') << "\n\n";
    
    UINT32 pathCount = 0;
    UINT32 modeCount = 0;
    
    // Get buffer sizes
    LONG result = GetDisplayConfigBufferSizes(
        QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount);
    
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to get display config buffer sizes. "
                  << "Error: " << result << std::endl;
        return 1;
    }
    
    // Allocate buffers
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);
    
    // Query display configuration
    result = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,
        &pathCount, paths.data(), &modeCount, modes.data(), nullptr);
    
    if (result != ERROR_SUCCESS) {
        std::cerr << "Failed to query display config. "
                  << "Error: " << result << std::endl;
        return 1;
    }
    
    int displayCount = 1;
    
    for (UINT32 i = 0; i < pathCount; i++) {
        const auto& path = paths[i];
        
        // Get display name using EDID
        std::string displayName = getDisplayName(
            path.targetInfo.adapterId, path.targetInfo.id);
        
        // Get additional EDID information
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetDeviceName = {};
        targetDeviceName.header.type = 
            DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetDeviceName.header.size = sizeof(targetDeviceName);
        targetDeviceName.header.adapterId = path.targetInfo.adapterId;
        targetDeviceName.header.id = path.targetInfo.id;
        
        result = DisplayConfigGetDeviceInfo(&targetDeviceName.header);
        
        std::cout << "Display " << displayCount << ":\n";
        std::cout << "  Name: " << displayName << "\n";
        
        if (result == ERROR_SUCCESS) {
            // Convert connector instance to string
            std::string connectorInstance;
            int len = WideCharToMultiByte(CP_UTF8, 0, 
                targetDeviceName.monitorDevicePath, -1, 
                nullptr, 0, nullptr, nullptr);
            connectorInstance.resize(len);
            WideCharToMultiByte(CP_UTF8, 0, 
                targetDeviceName.monitorDevicePath, -1, 
                &connectorInstance[0], len, nullptr, nullptr);
            connectorInstance.pop_back(); // Remove null terminator
            
            std::cout << "  Device Path: " << connectorInstance << "\n";
            
            // Display output technology
            const char* outputTech = "Unknown";
            switch (targetDeviceName.outputTechnology) {
                case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15:
                    outputTech = "VGA"; break;
                case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI:
                    outputTech = "DVI"; break;
                case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI:
                    outputTech = "HDMI"; break;
                case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL:
                    outputTech = "DisplayPort"; break;
                case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED:
                    outputTech = "eDP"; break;
                case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL:
                    outputTech = "UDI External"; break;
                case DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED:
                    outputTech = "UDI Embedded"; break;
                default:
                    outputTech = "Other"; break;
            }
            std::cout << "  Connection Type: " << outputTech << "\n";
        }
        
        std::cout << "  Adapter ID: " << std::hex 
                  << path.targetInfo.adapterId.HighPart << ":"
                  << path.targetInfo.adapterId.LowPart << std::dec << "\n";
        std::cout << "  Target ID: " << path.targetInfo.id << "\n";
        std::cout << "\n";
        
        displayCount++;
    }
    
    if (pathCount == 0) {
        std::cout << "No active displays found.\n";
    }
    
    return 0;
}
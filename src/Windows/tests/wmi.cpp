#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <tchar.h>
#include <Wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")

// Helper to convert TCHAR to std::string
std::string TCHAR_to_string(const TCHAR *tstr) {
  if (!tstr)
    return "";
#ifdef UNICODE
  std::wstring ws(tstr);
  int bufferSize = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0,
                                      nullptr, nullptr);
  if (bufferSize == 0) {
    return "";
  }
  std::string result(bufferSize - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &result[0], bufferSize,
                      nullptr, nullptr);
  return result;
#else
  return std::string(tstr);
#endif
}

// <<< ADDED BACK: This function was mistakenly removed >>>
// Helper to convert TCHAR to std::wstring
std::wstring TCHAR_to_wstring(const TCHAR *tstr) {
  if (!tstr)
    return L"";
#ifdef UNICODE
  return std::wstring(tstr);
#else
  std::wstring wstr;
  int bufferSize = MultiByteToWideChar(CP_UTF8, 0, tstr, -1, nullptr, 0);
  if (bufferSize == 0) {
    return L"";
  }
  wstr.resize(bufferSize - 1);
  MultiByteToWideChar(CP_UTF8, 0, tstr, -1, &wstr[0], bufferSize);
  return wstr;
#endif
}

// The definitive method using WMI
std::string GetMonitorNameWithWMI(const TCHAR *targetDeviceID) {
  std::string monitorName;
  HRESULT hres;

  hres = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres)) {
    return "";
  }

  hres = CoInitializeSecurity(
      NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
      RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

  if (FAILED(hres)) {
    CoUninitialize();
    return "";
  }

  IWbemLocator *pLoc = NULL;
  hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *)&pLoc);

  if (FAILED(hres)) {
    CoUninitialize();
    return "";
  }

  IWbemServices *pSvc = NULL;
  hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), NULL, NULL, 0, NULL, 0, 0,
                             &pSvc);

  if (FAILED(hres)) {
    pLoc->Release();
    CoUninitialize();
    return "";
  }

  hres = CoSetProxyBlanket(
      pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
      RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

  if (FAILED(hres)) {
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return "";
  }

  IEnumWbemClassObject *pEnumerator = NULL;
  hres = pSvc->ExecQuery(
      bstr_t("WQL"), bstr_t("SELECT * FROM WmiMonitorID"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
      &pEnumerator);

  if (FAILED(hres)) {
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return "";
  }

  IWbemClassObject *pclsObj = NULL;
  ULONG uReturn = 0;
  while (pEnumerator) {
    hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
    if (0 == uReturn) {
      break;
    }

    VARIANT vtInstanceName, vtUserFriendlyName;
    pclsObj->Get(L"InstanceName", 0, &vtInstanceName, 0, 0);
    pclsObj->Get(L"UserFriendlyName", 0, &vtUserFriendlyName, 0, 0);

    std::wstring wmiInstanceName = vtInstanceName.bstrVal;
    std::wstring targetDeviceID_w(TCHAR_to_wstring(targetDeviceID));

    if (wmiInstanceName.find(targetDeviceID_w.substr(8)) != std::wstring::npos) {
      if (vtUserFriendlyName.vt == (VT_ARRAY | VT_UI1)) {
        SAFEARRAY *psa = vtUserFriendlyName.parray;
        USHORT *pData = NULL;
        SafeArrayAccessData(psa, (void **)&pData);
        long lower, upper;
        SafeArrayGetLBound(psa, 1, &lower);
        SafeArrayGetUBound(psa, 1, &upper);
        for (long i = lower; i <= upper; ++i) {
          if (pData[i] == 0)
            break;
          monitorName += static_cast<char>(pData[i]);
        }
        SafeArrayUnaccessData(psa);
      }
    }

    VariantClear(&vtInstanceName);
    VariantClear(&vtUserFriendlyName);
    pclsObj->Release();

    if (!monitorName.empty()) {
      break;
    }
  }

  pSvc->Release();
  pLoc->Release();
  pEnumerator->Release();
  CoUninitialize();

  return monitorName;
}

struct MonitorInfo {
  bool isPrimary;
  std::string deviceName, deviceDescription;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor,
                              LPRECT lprcMonitor, LPARAM dwData) {
  auto *monitors = reinterpret_cast<std::vector<MonitorInfo> *>(dwData);
  MONITORINFOEX monitorInfo;
  monitorInfo.cbSize = sizeof(MONITORINFOEX);

  if (GetMonitorInfo(hMonitor, &monitorInfo)) {
    MonitorInfo currentMonitor{};
    currentMonitor.isPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY);
    currentMonitor.deviceName = TCHAR_to_string(monitorInfo.szDevice);

    std::string modelName;
    DISPLAY_DEVICE monitorDevice{};
    monitorDevice.cb = sizeof(DISPLAY_DEVICE);
    for (DWORD devNum = 0;
         EnumDisplayDevices(monitorInfo.szDevice, devNum, &monitorDevice, 0);
         ++devNum) {
      if (monitorDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
        modelName = GetMonitorNameWithWMI(monitorDevice.DeviceID);
        break;
      }
    }

    if (!modelName.empty()) {
      currentMonitor.deviceDescription = modelName;
    } else {
      currentMonitor.deviceDescription =
          TCHAR_to_string(monitorDevice.DeviceString);
    }
    monitors->push_back(currentMonitor);
  }
  return TRUE;
}

int main() {
  std::vector<MonitorInfo> monitors;
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc,
                      reinterpret_cast<LPARAM>(&monitors));
  if (monitors.empty()) {
    std::cout << "No monitors found.\n";
    return 1;
  }
  std::cout << "Detected Windows Monitors (Per Screen):\n";
  for (const auto &mon : monitors) {
    std::cout << "----------------------------------------\n";
    std::cout << "Is Primary Monitor:           "
              << (mon.isPrimary ? "Yes" : "No") << "\n";
    std::cout << "Device Name:                  " << mon.deviceName << "\n";
    std::cout << "Device Description:           " << mon.deviceDescription
              << "\n";
  }
  std::cout << "----------------------------------------\n";
  return 0;
}
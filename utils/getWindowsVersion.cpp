
#include <Windows.h>
#include "../config/phconfig.h"
#include "common.h"
#include "getWindowsVersion.h"
#include "Handles.h"

static char* TAG = "getWindowsVersion.cpp:\t";

extern RtlGetVersion fnRtlGetVersion;

ULONG getWindowsVersion() {
    ULONG WindowsVersion = 0;

    HMODULE hNtdll;
    LONG ntStatus;
    ULONG    majorVersion = 0;
    ULONG    minorVersion = 0;
    ULONG    buildVersion = 0;
    RTL_OSVERSIONINFOW VersionInformation = { 0 };

    VersionInformation.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    ntStatus = fnRtlGetVersion(&VersionInformation);

    if (ntStatus != 0) {
        LOGE << TAG << "fnRtlGetVersion Error.\n";
        return 1;
    }

    majorVersion = VersionInformation.dwMajorVersion;
    minorVersion = VersionInformation.dwMinorVersion;
    buildVersion = VersionInformation.dwBuildNumber;

    if (majorVersion == 6 && minorVersion < 1 || majorVersion < 6)
    {
        WindowsVersion = WINDOWS_ANCIENT;
    }
    // Windows 7, Windows Server 2008 R2
    else if (majorVersion == 6 && minorVersion == 1)
    {
        WindowsVersion = WINDOWS_7;
    }
    // Windows 8, Windows Server 2012
    else if (majorVersion == 6 && minorVersion == 2)
    {
        WindowsVersion = WINDOWS_8;
    }
    // Windows 8.1, Windows Server 2012 R2
    else if (majorVersion == 6 && minorVersion == 3)
    {
        WindowsVersion = WINDOWS_8_1;
    }
    // Windows 10, Windows Server 2016
    else if (majorVersion == 10 && minorVersion == 0)
    {
        if (buildVersion > 22631)
        {
            WindowsVersion = WINDOWS_NEW;
        }
        else if (buildVersion >= 22631)
        {
            WindowsVersion = WINDOWS_11_23H2;
        }
        else if (buildVersion >= 22621)
        {
            WindowsVersion = WINDOWS_11_22H2;
        }
        else if (buildVersion >= 22000)
        {
            WindowsVersion = WINDOWS_11;
        }
        else if (buildVersion >= 19045)
        {
            WindowsVersion = WINDOWS_10_22H2;
        }
        else if (buildVersion >= 19044)
        {
            WindowsVersion = WINDOWS_10_21H2;
        }
        else if (buildVersion >= 19043)
        {
            WindowsVersion = WINDOWS_10_21H1;
        }
        else if (buildVersion >= 19042)
        {
            WindowsVersion = WINDOWS_10_20H2;
        }
        else if (buildVersion >= 19041)
        {
            WindowsVersion = WINDOWS_10_20H1;
        }
        else if (buildVersion >= 18363)
        {
            WindowsVersion = WINDOWS_10_19H2;
        }
        else if (buildVersion >= 18362)
        {
            WindowsVersion = WINDOWS_10_19H1;
        }
        else if (buildVersion >= 17763)
        {
            WindowsVersion = WINDOWS_10_RS5;
        }
        else if (buildVersion >= 17134)
        {
            WindowsVersion = WINDOWS_10_RS4;
        }
        else if (buildVersion >= 16299)
        {
            WindowsVersion = WINDOWS_10_RS3;
        }
        else if (buildVersion >= 15063)
        {
            WindowsVersion = WINDOWS_10_RS2;
        }
        else if (buildVersion >= 14393)
        {
            WindowsVersion = WINDOWS_10_RS1;
        }
        else if (buildVersion >= 10586)
        {
            WindowsVersion = WINDOWS_10_TH2;
        }
        else if (buildVersion >= 10240)
        {
            WindowsVersion = WINDOWS_10;
        }
        else
        {
            WindowsVersion = WINDOWS_10;
        }
    }
    else
    {
        WindowsVersion = WINDOWS_NEW;
    }

    return WindowsVersion;
}
#include <stdio.h>
#include <Windows.h>
#include "./config/phconfig.h"
#include "./utils/getWindowsVersion.h"
#include "./src/gpuMonitor.h"
#include "./utils/Handles.h"
#include "./utils/common.h"

#pragma comment(lib, "cfgmgr32.lib")

static char* TAG = "main.cpp:\t";

int main()
{
    if (!loadDll())
    {
        LOGE << TAG << "loadDll Error.\n";
        return 1;
    }

    if (!loadFunc())
    {
        LOGE << TAG << "loadFunc Error.\n";
        return 1;
    }

    //ULONG WindowsVersion = 0;
    //WindowsVersion = getWindowsVersion();
    //printf("Vesion: %ul.\n", WindowsVersion);

    GpuMonitor monitor;
    if (!monitor.start())
    {
        LOGE << TAG << "monitor.start failed.\n";
    }

    LOGI << TAG << "process exit sucessfully.\n";
    return 0;
}
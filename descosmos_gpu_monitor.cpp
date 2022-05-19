#include <stdio.h>
#include <stdlib.h>
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

    while (true)
    {
        LOGI << "\n\n";
        std::vector<FLOAT_ULONG64> dataList = monitor.collect();
        Sleep(2000);
        LOGI << TAG << "GPU_UTILIZATION: " << dataList[GPU_UTILIZATION].float_ << "\n";
        LOGI << TAG << "GPU_DEDICATED_USAGE: " << dataList[GPU_UTILIZATION].ulong64_ << "\n";
        LOGI << TAG << "GPU_DEDICATED_LIMIT: " << dataList[GPU_UTILIZATION].ulong64_ << "\n";
        LOGI << TAG << "GPU_SHARED_USAGE: " << dataList[GPU_UTILIZATION].ulong64_ << "\n";
        LOGI << TAG << "GPU_SHARED_LIMIT: " << dataList[GPU_UTILIZATION].ulong64_ << "\n";
    }
    


    LOGI << TAG << "process exit sucessfully.\n";
    return 0;
}
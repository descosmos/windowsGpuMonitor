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

    GpuMonitor monitor(18416);
    if (!monitor.start())
    {
        LOGE << TAG << "monitor.start failed.\n";
        return -1;
    }

    LOGI << TAG << "start collect.\n";
    while (true)
    {
        LOGI << "\n\n";
        std::vector<FLOAT_ULONG64> dataList = monitor.collect();
        Sleep(2000);
        LOGI << TAG << "GPU_TARGET_PROCESS_UTILIZATION: " << dataList[GPU_TARGET_PROCESS_UTILIZATION].float_ << "\n";
        LOGI << TAG << "GPU_TARGET_PROCESS_DEDICATED_USAGE: " << dataList[GPU_TARGET_PROCESS_DEDICATED_USAGE].ulong64_ / 1024 / 1024<< "MB \n";
        LOGI << TAG << "GPU_DEDICATED_LIMIT: " << dataList[GPU_DEDICATED_LIMIT].ulong64_ / 1024 / 1024<< "MB \n";
        LOGI << TAG << "GPU_TARGET_PROCESS_SHARED_USAGE: " << dataList[GPU_TARGET_PROCESS_SHARED_USAGE].ulong64_ / 1024 / 1024 << "MB \n";
        LOGI << TAG << "GPU_SHARED_LIMIT: " << dataList[GPU_SHARED_LIMIT].ulong64_ / 1024 / 1024 << "MB \n";
    }
    


    LOGI << TAG << "process exit sucessfully.\n";
    return 0;
}
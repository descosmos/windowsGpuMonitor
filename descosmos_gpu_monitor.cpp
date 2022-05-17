#include <stdio.h>
#include <Windows.h>
#include "./config/phconfig.h"
#include "./utils/getWindowsVersion.h"
#include "./src/gpuMonitor.h"

int main()
{
    ULONG WindowsVersion = 0;
    WindowsVersion = getWindowsVersion();
    printf("Vesion: %ul.\n", WindowsVersion);

    GpuMonitor monitor;

    return 0;
}
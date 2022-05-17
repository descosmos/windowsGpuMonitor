#ifndef _GPU_MONITOR_H
#define _GPU_MONITOR_H

#include <Windows.h>
#include <vector>
#include <string>
#include <stdint.h>
#include "../d3dkmt/d3dkmthk.h"
#include "../config/dltmgr.h"

#ifndef GPU_DATA_LIST_SIZE
#define GPU_DATA_LIST_SIZE 5
#endif // !GPU_DATA_LIST_SIZE


typedef struct _D3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1
{
    ULONG CommitLimit;
    ULONG BytesCommitted;
    ULONG BytesResident;
    D3DKMT_QUERYSTATISTICS_MEMORY Memory;
    ULONG Aperture; // boolean
    ULONGLONG TotalBytesEvictedByPriority[D3DKMT_MaxAllocationPriorityClass];
    ULONG64 SystemMemoryEndAddress;
    struct
    {
        ULONG64 PreservedDuringStandby : 1;
        ULONG64 PreservedDuringHibernate : 1;
        ULONG64 PartiallyPreservedDuringHibernate : 1;
        ULONG64 Reserved : 61;
    } PowerFlags;
    ULONG64 Reserved[7];
} D3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1, * PD3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1;

enum GpuMonitorDataIndex
{
    GPU_UTILIZATION = 0,
    GPU_DEDICATED_USAGE = 1,
    GPU_DEDICATED_LIMIT = 2,
    GPU_SHARED_USAGE = 3,
    GPU_SHARED_LIMIT = 4
};

union FLOAT_ULONG64
{
    FLOAT float_;
    ULONG64 ulong64_;
};


class GpuMonitor {
public:
    GpuMonitor();
    ~GpuMonitor();

    bool start();
    bool collect();
    bool stop();

private:
    BOOLEAN EtGpuEnabled_ = FALSE;
    BOOLEAN EtGpuSupported_ = FALSE;
    BOOLEAN EtD3DEnabled_ = FALSE;

    ULONG EtGpuTotalNodeCount_ = 0;
    ULONG EtGpuTotalSegmentCount_ = 0;

    PH_UINT64_DELTA EtClockTotalRunningTimeDelta_ = { 0, 0 };
    LARGE_INTEGER EtClockTotalRunningTimeFrequency_ = { 0 };

    std::vector<FLOAT_ULONG64> dataList_;
};


#endif // !_GPU_MONITOR_H

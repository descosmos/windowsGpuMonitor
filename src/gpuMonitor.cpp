#include <Windows.h>
#include <initguid.h>
/*
	Attention: <Windows.h> must be front of <cfgmgr32.h> and <ntddvdeo.h>
*/
#include <cfgmgr32.h>
#include <ntddvdeo.h>
#include <d3d11.h>
#include "gpuMonitor.h"
#include "../config/phconfig.h"
#include "../config/phnt.h"
#include "../config/phnt_windows.h"
#include "../utils/common.h"
#include "../utils/getWindowsVersion.h"
#include "../utils/Handles.h"


static const char* TAG = "gpuMonitor.cpp:\t";

extern RtlCreateHeap fnRtlCreateHeap;
extern RtlDestroyHeap fnRtlDestroyHeap;
extern RtlAllocateHeap fnRtlAllocateHeap;
extern RtlFreeHeap fnRtlFreeHeap;
extern RtlSizeHeap fnRtlSizeHeap;
extern RtlZeroHeap fnRtlZeroHeap;
extern RtlSetHeapInformation fnRtlSetHeapInformation;
extern RtlQueryHeapInformation fnRtlQueryHeapInformation;


GpuMonitor::GpuMonitor()
{
}

GpuMonitor::~GpuMonitor()
{
}

bool GpuMonitor::start()
{
	PhInitializeWindowsVersion();
	PhHeapInitialization(0, 0);

	EtGpuSupported_ = windowsVersion_ >= WINDOWS_10_RS4;
	EtD3DEnabled_ = EtGpuSupported_;		//	TODO: EtD3DEnabled_ 的值是由配置文件和 EtGpuSupported_共同决定, 此处不使用配置文件因此 EtD3DEnabled_ 没有存在必要, 待删除

	if (initializeD3DStatistics()) {
		EtGpuEnabled_ = true;
	}

	return true;
}

std::vector<FLOAT_ULONG64> GpuMonitor::collect()
{
	return dataList_;
}

bool GpuMonitor::stop()
{
	return false;
}

bool GpuMonitor::initializeD3DStatistics()
{
	std::vector<void*> deviceAdapterList;
	PWSTR deviceInterfaceList;
	ULONG deviceInterfaceListLength = 0;
	PWSTR deviceInterface;
	D3DKMT_OPENADAPTERFROMDEVICENAME openAdapterFromDeviceName;
	D3DKMT_QUERYSTATISTICS queryStatistics;
	D3DKMT_ADAPTER_PERFDATACAPS perfCaps;

	if (CM_Get_Device_Interface_List_Size(
		&deviceInterfaceListLength,
		(PGUID)&GUID_DISPLAY_DEVICE_ARRIVAL,
		NULL,
		CM_GET_DEVICE_INTERFACE_LIST_PRESENT
	) != CR_SUCCESS)
	{
		return FALSE;
	}


	deviceAdapterList.reserve(deviceInterfaceListLength);

	return true;
}

bool GpuMonitor::PhHeapInitialization(SIZE_T HeapReserveSize, SIZE_T HeapCommitSize)
{
	//PhHeapHandle_ = RtlCreateHeap(
	//	HEAP_GROWABLE | HEAP_CLASS_1,
	//	NULL,
	//	HeapReserveSize ? HeapReserveSize : 2 * 1024 * 1024, // 2 MB
	//	HeapCommitSize ? HeapCommitSize : 1024 * 1024, // 1 MB
	//	NULL,
	//	NULL
	//);

//#if (PHNT_VERSION >= PHNT_VISTA)
//	RtlSetHeapInformation(
//		PhHeapHandle_,
//		HeapCompatibilityInformation,
//		&(ULONG){ HEAP_COMPATIBILITY_LFH },
//		sizeof(ULONG)
//	);
//#endif

	if (!PhHeapHandle_)
	{
		return false;
	}
		
	return true;
}

bool GpuMonitor::PhInitializeWindowsVersion()
{
	windowsVersion_ = getWindowsVersion();
	if (windowsVersion_ == 1)
	{
		return false;
	}
	return true;
}

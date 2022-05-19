

#include <Windows.h>
#include <initguid.h>
/*
	Attention: <Windows.h> must be front of <cfgmgr32.h> and <ntddvdeo.h>
*/
#include <cfgmgr32.h>
#include <ntddvdeo.h>
#include <d3d11.h>
#include <winternl.h>
#include <winnt.h>
#include "gpuMonitor.h"
#include "../config/phconfig.h"
#include "../config/phnt.h"
#include "../config/phnt_windows.h"
#include "../config/refp.h"

#include "../utils/common.h"
#include "../utils/getWindowsVersion.h"
#include "../utils/Handles.h"
#include "../utils/baseup.h"

static const char* TAG = "gpuMonitor.cpp:\t";

extern RtlCreateHeap fnRtlCreateHeap;
extern RtlDestroyHeap fnRtlDestroyHeap;
extern RtlAllocateHeap fnRtlAllocateHeap;
extern RtlFreeHeap fnRtlFreeHeap;
extern RtlSizeHeap fnRtlSizeHeap;
extern RtlZeroHeap fnRtlZeroHeap;
extern RtlSetHeapInformation fnRtlSetHeapInformation;
extern RtlQueryHeapInformation fnRtlQueryHeapInformation;

//extern D3DKMTOpenAdapterFromDeviceName fnD3DKMTOpenAdapterFromDeviceName;
//extern D3DKMTQueryAdapterInfo fnD3DKMTQueryAdapterInfo;

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
	EtD3DEnabled_ = EtGpuSupported_;		//	TODO: 

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
		LOGE << TAG << "CM_Get_Device_Interface_List_Size failed.\n";
		return FALSE;
	}

	deviceInterfaceList = (PWSTR)fnRtlAllocateHeap(PhHeapHandle_, HEAP_GENERATE_EXCEPTIONS, deviceInterfaceListLength * sizeof(WCHAR));
	memset(deviceInterfaceList, 0, deviceInterfaceListLength * sizeof(WCHAR));

	//	TODO: use CM_Get_Device_Interface_List instead of CM_Get_Device_Interface_ListW
	if (CM_Get_Device_Interface_ListW(
		(PGUID)&GUID_DISPLAY_DEVICE_ARRIVAL,
		NULL,
		deviceInterfaceList,
		deviceInterfaceListLength,
		CM_GET_DEVICE_INTERFACE_LIST_PRESENT
	) != CR_SUCCESS)
	{
		fnRtlFreeHeap(PhHeapHandle_, 0, deviceInterfaceList);
		LOGE << TAG << "CM_Get_Device_Interface_List failed.\n";
		return FALSE;
	}

	deviceAdapterList.reserve(0);	//	TODO: explain why 0

	for (deviceInterface = deviceInterfaceList; *deviceInterface; deviceInterface += PhCountStringZ(deviceInterface) + 1)
	{
		deviceAdapterList.push_back(deviceInterface);
	}

#ifdef DEBUG
	LOGI << TAG << "deviceAdapterList.size: " << deviceAdapterList.size() << "\n";
#endif // DEBUG

	for (ULONG i = 0; i < deviceAdapterList.size(); i++)
	{
		memset(&openAdapterFromDeviceName, 0, sizeof(D3DKMT_OPENADAPTERFROMDEVICENAME));
		openAdapterFromDeviceName.pDeviceName = (PCWSTR)deviceAdapterList[i];

		if (!NT_SUCCESS(D3DKMTOpenAdapterFromDeviceName(&openAdapterFromDeviceName)))
		{
			LOGE << TAG << "fnD3DKMTOpenAdapterFromDeviceName: " << openAdapterFromDeviceName.pDeviceName << "failed\n";
			continue;
		}

		if (EtGpuSupported_ && deviceAdapterList.size() > 1) // Note: Changed to RS4 due to reports of BSODs on LTSB versions of RS3
		{
			if (EtpIsGpuSoftwareDevice(openAdapterFromDeviceName.hAdapter))
			{
				EtCloseAdapterHandle(openAdapterFromDeviceName.hAdapter);
				continue;
			}
		}

		if (EtGpuSupported_)
		{
			D3DKMT_SEGMENTSIZEINFO segmentInfo;

			memset(&segmentInfo, 0, sizeof(D3DKMT_SEGMENTSIZEINFO));

			if (NT_SUCCESS(EtQueryAdapterInformation(
				openAdapterFromDeviceName.hAdapter,
				KMTQAITYPE_GETSEGMENTSIZE,
				&segmentInfo,
				sizeof(D3DKMT_SEGMENTSIZEINFO)
			)))
			{
#ifdef DEBUG
				LOGI << TAG << "EtGpuDedicatedLimit_: " << EtGpuDedicatedLimit_ << "\n";
				LOGI << TAG << "EtGpuSharedLimit_: " << EtGpuDedicatedLimit_ << "\n";
#endif // DEBUG
				EtGpuDedicatedLimit_ += segmentInfo.DedicatedVideoMemorySize;
				EtGpuSharedLimit_ += segmentInfo.SharedSystemMemorySize;
			}
		}

		memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
		queryStatistics.Type = D3DKMT_QUERYSTATISTICS_ADAPTER;
		queryStatistics.AdapterLuid = openAdapterFromDeviceName.AdapterLuid;

#ifdef DEBUG
		LOGI << TAG << "D3DKMT_QUERYSTATISTICS_ADAPTER queryStatistics.QueryResult.AdapterInformation.NodeCount: " << queryStatistics.QueryResult.AdapterInformation.NodeCount << "\n";
		LOGI << TAG << "D3DKMT_QUERYSTATISTICS_ADAPTER queryStatistics.QueryResult.AdapterInformation.NbSegments: " << queryStatistics.QueryResult.AdapterInformation.NbSegments << "\n";
#endif // DEBUG


		if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)))
		{
			EtGpuTotalNodeCount_ += queryStatistics.QueryResult.AdapterInformation.NodeCount;
			EtGpuTotalSegmentCount_ += queryStatistics.QueryResult.AdapterInformation.NbSegments;

			for (ULONG ii = 0; ii < queryStatistics.QueryResult.AdapterInformation.NbSegments; ii++)
			{
				memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
				queryStatistics.Type = D3DKMT_QUERYSTATISTICS_SEGMENT;
				queryStatistics.AdapterLuid = openAdapterFromDeviceName.AdapterLuid;
				queryStatistics.QuerySegment.SegmentId = ii;

				if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)))
				{
					ULONG64 commitLimit;
					ULONG aperture;

					if (windowsVersion_ >= WINDOWS_8)
					{
						commitLimit = queryStatistics.QueryResult.SegmentInformation.CommitLimit;
						aperture = queryStatistics.QueryResult.SegmentInformation.Aperture;
					}
					else
					{
						PD3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1 segmentInfo;

						segmentInfo = (PD3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1)&queryStatistics.QueryResult;
						commitLimit = segmentInfo->CommitLimit;
						aperture = segmentInfo->Aperture;
					}

					if (!EtGpuSupported_ || !EtD3DEnabled_) // Note: Changed to RS4 due to reports of BSODs on LTSB versions of RS3
					{
						if (aperture)
							EtGpuSharedLimit_ += commitLimit;
						else
							EtGpuDedicatedLimit_ += commitLimit;
					}
				}
			}
		}

		EtCloseAdapterHandle(openAdapterFromDeviceName.hAdapter);
	}

	{
		//	TODO: dereference and free
	}

	fnRtlFreeHeap(PhHeapHandle_, 0, deviceInterfaceList);

	if (EtGpuTotalNodeCount_ == 0)
	{
		LOGE << TAG << "EtGpuTotalNodeCount_ == 0 \n";
		return false;
	}

#ifdef DEBUG
	LOGI << TAG << "EtGpuTotalNodeCount_: " << EtGpuTotalNodeCount_ << "\n";
#endif // DEBUG

	return true;
}

bool GpuMonitor::PhHeapInitialization(SIZE_T HeapReserveSize, SIZE_T HeapCommitSize)
{
	PhHeapHandle_ = fnRtlCreateHeap(
		HEAP_GROWABLE | HEAP_CLASS_1,
		NULL,
		HeapReserveSize ? HeapReserveSize : 2 * 1024 * 1024, // 2 MB
		HeapCommitSize ? HeapCommitSize : 1024 * 1024, // 1 MB
		NULL,
		NULL
	);



#if (PHNT_VERSION >= PHNT_VISTA)
	ULONG HEAP_COMPATIBILITY_LFH_TMP = HEAP_COMPATIBILITY_LFH;
	fnRtlSetHeapInformation(
		PhHeapHandle_,
		HeapCompatibilityInformation,
		&HEAP_COMPATIBILITY_LFH_TMP,
		sizeof(ULONG)
	);
#endif

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

bool GpuMonitor::EtpIsGpuSoftwareDevice(D3DKMT_HANDLE AdapterHandle)
{
	D3DKMT_ADAPTERTYPE adapterType;

	memset(&adapterType, 0, sizeof(D3DKMT_ADAPTERTYPE));

	if (NT_SUCCESS(EtQueryAdapterInformation(
		AdapterHandle,
		KMTQAITYPE_ADAPTERTYPE,
		&adapterType,
		sizeof(D3DKMT_ADAPTERTYPE)
	)))
	{
		if (adapterType.SoftwareDevice) // adapterType.HybridIntegrated
		{
			return TRUE;
		}
	}

	return false;
}

NTSTATUS GpuMonitor::EtQueryAdapterInformation(D3DKMT_HANDLE AdapterHandle, KMTQUERYADAPTERINFOTYPE InformationClass, PVOID Information, UINT32 InformationLength)
{
	D3DKMT_QUERYADAPTERINFO queryAdapterInfo;

	memset(&queryAdapterInfo, 0, sizeof(D3DKMT_QUERYADAPTERINFO));
	queryAdapterInfo.hAdapter = AdapterHandle;
	queryAdapterInfo.Type = InformationClass;
	queryAdapterInfo.pPrivateDriverData = Information;
	queryAdapterInfo.PrivateDriverDataSize = InformationLength;

	return D3DKMTQueryAdapterInfo(&queryAdapterInfo);
}

BOOLEAN GpuMonitor::EtCloseAdapterHandle(D3DKMT_HANDLE AdapterHandle)
{
	return BOOLEAN();
}

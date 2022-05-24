

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
#include "../config/dltmgr.h"
#include "../config/phsup.h"

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

extern RtlSetBits fnRtlSetBits;
extern RtlClearBits fnRtlClearBits;
extern RtlInitializeBitMap fnRtlInitializeBitMap;

//extern D3DKMTOpenAdapterFromDeviceName fnD3DKMTOpenAdapterFromDeviceName;
//extern D3DKMTQueryAdapterInfo fnD3DKMTQueryAdapterInfo;

GpuMonitor::GpuMonitor(DWORD targetProcessId): targetProcessId_(targetProcessId)
{
	dataList_.resize(8);
}

GpuMonitor::~GpuMonitor()
{
}

bool GpuMonitor::start()
{
	PhInitializeWindowsVersion();

	if (!escalationRightOfCurrentProcess())
	{
		LOGE << TAG << "escalationRightOfCurrentProcess failed.\n";
		return false;
	}

	if (!openTargetProcessHandle(targetProcessId_))
	{
		LOGE << TAG << "openTargetProcessHandle failed.\n";
		return false;
	}

	PhHeapInitialization(0, 0);

	EtGpuSupported_ = windowsVersion_ >= WINDOWS_10_RS4;
	EtD3DEnabled_ = EtGpuSupported_;		//	TODO: !!PhGetIntegerSetting(SETTING_NAME_ENABLE_GPUPERFCOUNTERS)

	if (initializeD3DStatistics()) {
		EtGpuEnabled_ = true;
	}

	EtGpuNodesTotalRunningTimeDelta_.resize(EtGpuTotalNodeCount_);

	return true;
}

std::vector<FLOAT_ULONG64> GpuMonitor::collect()
{
	static ULONG runCount = 0; // MUST keep in sync with runCount in process provider
	DOUBLE elapsedTime = 0; // total GPU node elapsed time in micro-seconds
	FLOAT tempGpuUsage = 0;
	ULONG i;
	//PET_PROCESS_BLOCK maxNodeBlock = NULL;

	EtpUpdateSystemSegmentInformation();
	EtpUpdateSystemNodeInformation();

	LOGI << TAG << "EtClockTotalRunningTimeDelta_.Delta: " << EtClockTotalRunningTimeDelta_.Delta << "\n";
	LOGI << TAG << "EtClockTotalRunningTimeFrequency_.QuadPart: " << EtClockTotalRunningTimeFrequency_.QuadPart << "\n";

	elapsedTime = (DOUBLE)(EtClockTotalRunningTimeDelta_.Delta * 10000000) / EtClockTotalRunningTimeFrequency_.QuadPart;
	LOGI << TAG << "elapsedTime: " << elapsedTime << "\n";

	if (elapsedTime != 0)
	{
		for (i = 0; i < EtGpuTotalNodeCount_; i++)
		{
			FLOAT usage = (FLOAT)(EtGpuNodesTotalRunningTimeDelta_[i].Delta / elapsedTime);
			LOGI << TAG << "EtGpuTotalNodeCount_[" << i << "] usage: "<< usage <<".\n";

			if (usage > 1)
				usage = 1;

			if (usage > tempGpuUsage)
				tempGpuUsage = usage;
		}
	}

	EtGpuNodeUsage_ = tempGpuUsage;

	EtpUpdateProcessSegmentInformation();
	EtpUpdateProcessNodeInformation();
	if (elapsedTime != 0)
	{
		targetProcessGpuUtilization_ = (FLOAT)(GpuRunningTimeDelta_.Delta / elapsedTime);

		// HACK
		if (targetProcessGpuUtilization_ > EtGpuNodeUsage_)
		{
			targetProcessGpuUtilization_ = EtGpuNodeUsage_;
		}
			
		//for (i = 0; i < EtGpuTotalNodeCount; i++)
		//{
		//    FLOAT usage = (FLOAT)(block->GpuTotalRunningTimeDelta[i].Delta / elapsedTime);
		//
		//    if (usage > block->GpuNodeUtilization)
		//    {
		//        block->GpuNodeUtilization = usage;
		//    }
		//}


		if (targetProcessGpuUtilization_ > 1)
		{
			targetProcessGpuUtilization_ = 1;
		}
			

		//if (runCount != 0)
		//{
		//	targetProcessGpuUtilization_ = GpuNodeUtilization_;
		//	targetProcessGpuDedicatedUsage_ = (ULONG)(block->GpuDedicatedUsage / PAGE_SIZE);
		//	targetProcessGpuSharedUsage_ = (ULONG)(block->GpuSharedUsage / PAGE_SIZE);
		//	targetProcessCommitUsage_ = (ULONG)(block->GpuCommitUsage / PAGE_SIZE);
		//}
	}

	//	TODO: fixme
	dataList_[GPU_TARGET_PROCESS_UTILIZATION].float_ = targetProcessGpuUtilization_;
	dataList_[GPU_TARGET_PROCESS_DEDICATED_USAGE].ulong64_ = targetProcessGpuDedicatedUsage_;
	dataList_[GPU_DEDICATED_LIMIT].ulong64_ = EtGpuDedicatedLimit_;
	dataList_[GPU_TARGET_PROCESS_SHARED_USAGE].ulong64_ = targetProcessGpuSharedUsage_;
	dataList_[GPU_SHARED_LIMIT].ulong64_ = EtGpuSharedLimit_;
	dataList_[GPU_SYSTEM_DEDICATED_USAGE].ulong64_ = EtGpuDedicatedUsage_;
	dataList_[GPU_SYSTEM_SHARED_USAGE].ulong64_ = EtGpuSharedUsage_;
	dataList_[GPU_SYSTEM_UTILIZATION].float_ = EtGpuNodeUsage_;

	runCount++;

	LOGI << TAG << "collect successfully.\n";
	return dataList_;
}

bool GpuMonitor::stop()
{
	if (targetProcessHandle_)
	{
		return CloseHandle(targetProcessHandle_);
	}
	return true;
}

bool GpuMonitor::initializeD3DStatistics()
{
	std::vector<void*> deviceAdapterList;
	PWSTR deviceInterfaceList;
	ULONG deviceInterfaceListLength = 0;
	PWSTR deviceInterface;
	D3DKMT_OPENADAPTERFROMDEVICENAME openAdapterFromDeviceName;
	D3DKMT_QUERYSTATISTICS queryStatistics;

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

	deviceAdapterList.resize(0);	//	TODO: explain why 0

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
			PETP_GPU_ADAPTER gpuAdapter = new ETP_GPU_ADAPTER();
			gpuAdapter->AdapterLuid = openAdapterFromDeviceName.AdapterLuid;
			gpuAdapter->NodeCount = queryStatistics.QueryResult.AdapterInformation.NodeCount;
			gpuAdapter->SegmentCount = queryStatistics.QueryResult.AdapterInformation.NbSegments;
			gpuAdapter->FirstNodeIndex = EtGpuNextNodeIndex_;
			fnRtlInitializeBitMap(&gpuAdapter->ApertureBitMap, gpuAdapter->ApertureBitMapBuffer, queryStatistics.QueryResult.AdapterInformation.NbSegments);
			gpuAdapterList_.push_back(gpuAdapter);

			EtGpuTotalNodeCount_ += queryStatistics.QueryResult.AdapterInformation.NodeCount;
			EtGpuTotalSegmentCount_ += queryStatistics.QueryResult.AdapterInformation.NbSegments;
			EtGpuNextNodeIndex_ += gpuAdapter->NodeCount;

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

					if (aperture)
					{
						fnRtlSetBits(&gpuAdapter->ApertureBitMap, ii, 1);
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

bool GpuMonitor::escalationRightOfCurrentProcess()
{
	HANDLE token_handle;
	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS, &token_handle))
	{
		LOGE << TAG << "openProcessToken error.\n";
		return false;
	}

	LUID luid;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		LOGE << TAG << "lookupPrivilegevalue error.\n";
		return false;
	}

	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(token_handle, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		LOGE << TAG << "adjust error.\n";
		return false;
	}

	LOGI << TAG << "escalate rights for current process successfully.\n";

	return true;
}

bool GpuMonitor::openTargetProcessHandle(DWORD pid)
{
//#ifdef DEBUG
//	targetProcessHandle = GetCurrentProcess();
//	return true;
//#endif

	targetProcessHandle_ = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, targetProcessId_);
	if (!targetProcessHandle_)
	{
		LOGE << TAG << "OpenProcess failed.\n";
		LOGE << TAG << "Error Number: " << GetLastError() << "\n";
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

void GpuMonitor::EtpUpdateSystemSegmentInformation()
{
	ULONG i;
	ULONG j;
	PETP_GPU_ADAPTER gpuAdapter;
	D3DKMT_QUERYSTATISTICS queryStatistics;
	ULONG64 dedicatedUsage;
	ULONG64 sharedUsage;

	dedicatedUsage = 0;
	sharedUsage = 0;

	for (i = 0; i < gpuAdapterList_.size(); i++)
	{
		gpuAdapter = gpuAdapterList_[i];

		for (j = 0; j < gpuAdapter->SegmentCount; j++)
		{
			//LOGI << TAG << "EtpUpdateSystemSegmentInformation gpuAdapter->SegmentCount.\n";
			memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
			queryStatistics.Type = D3DKMT_QUERYSTATISTICS_SEGMENT;
			queryStatistics.AdapterLuid = gpuAdapter->AdapterLuid;
			queryStatistics.QuerySegment.SegmentId = j;

			if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)))
			{
				//LOGI << TAG << "EtpUpdateSystemSegmentInformation D3DKMT_QUERYSTATISTICS_SEGMENT D3DKMTQueryStatistics.\n";
				ULONG64 bytesCommitted;
				ULONG aperture;

				if (windowsVersion_ >= WINDOWS_8)
				{
					bytesCommitted = queryStatistics.QueryResult.SegmentInformation.BytesResident;
					aperture = queryStatistics.QueryResult.SegmentInformation.Aperture;
				}
				else
				{
					PD3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1 segmentInfo;

					segmentInfo = (PD3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1)&queryStatistics.QueryResult;
					bytesCommitted = segmentInfo->BytesResident;
					aperture = segmentInfo->Aperture;
				}

				if (aperture) // RtlCheckBit(&gpuAdapter->ApertureBitMap, j)
					sharedUsage += bytesCommitted;
				else
					dedicatedUsage += bytesCommitted;
			}
		}
	}

	EtGpuDedicatedUsage_ = dedicatedUsage;
	EtGpuSharedUsage_ = sharedUsage;
	//LOGI << TAG << "EtpUpdateSystemSegmentInformation exit.\n";
}

void GpuMonitor::EtpUpdateSystemNodeInformation()
{
	PETP_GPU_ADAPTER gpuAdapter;
	D3DKMT_QUERYSTATISTICS queryStatistics;
	LARGE_INTEGER performanceCounter;
	memset(&performanceCounter, 0, sizeof(LARGE_INTEGER));

	for (ULONG i = 0; i < gpuAdapterList_.size(); i++)
	{
		gpuAdapter = gpuAdapterList_[i];

		for (ULONG j = 0; j < gpuAdapter->NodeCount; j++)
		{
			memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
			queryStatistics.Type = D3DKMT_QUERYSTATISTICS_NODE;
			queryStatistics.AdapterLuid = gpuAdapter->AdapterLuid;
			queryStatistics.QueryNode.NodeId = j;

			if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)))
			{
				ULONG64 runningTime;
				//ULONG64 systemRunningTime;

				runningTime = queryStatistics.QueryResult.NodeInformation.GlobalInformation.RunningTime.QuadPart;
				//systemRunningTime = queryStatistics.QueryResult.NodeInformation.SystemInformation.RunningTime.QuadPart;

				//PhUpdateDelta(&EtGpuNodesTotalRunningTimeDelta_[gpuAdapter->FirstNodeIndex + j], runningTime);
				LOGI << TAG << "runningTime: " << runningTime << " EtGpuNodesTotalRunningTimeDelta_[" << gpuAdapter->FirstNodeIndex + j << "].Value: " << EtGpuNodesTotalRunningTimeDelta_[j].Value << "\n";
				EtGpuNodesTotalRunningTimeDelta_[gpuAdapter->FirstNodeIndex + j].Delta = runningTime - EtGpuNodesTotalRunningTimeDelta_[gpuAdapter->FirstNodeIndex + j].Value;
				EtGpuNodesTotalRunningTimeDelta_[gpuAdapter->FirstNodeIndex + j].Value = runningTime;
			}
			else
			{
				LOGE << TAG << "D3DKMT_QUERYSTATISTICS_NODE D3DKMTQueryStatistics failed.\n";
			}
		}
	}

	PhQueryPerformanceCounter(&performanceCounter, &EtClockTotalRunningTimeFrequency_);
	PhUpdateDelta(&EtClockTotalRunningTimeDelta_, performanceCounter.QuadPart);
}

void GpuMonitor::PhQueryPerformanceCounter(_Out_ PLARGE_INTEGER PerformanceCounter, _Out_opt_ PLARGE_INTEGER PerformanceFrequency)
{
	QueryPerformanceFrequency(&EtClockTotalRunningTimeFrequency_);
	QueryPerformanceCounter(PerformanceCounter);
}

void GpuMonitor::EtpUpdateProcessSegmentInformation()
{
	ULONG i;
	ULONG j;
	PETP_GPU_ADAPTER gpuAdapter;
	D3DKMT_QUERYSTATISTICS queryStatistics;
	ULONG64 dedicatedUsage;
	ULONG64 sharedUsage;
	ULONG64 commitUsage;

	if (!targetProcessHandle_)
	{
		LOGE << TAG << "targetProcessHandle_ is NULL.\n";
		return;
	}

	dedicatedUsage = 0;
	sharedUsage = 0;
	commitUsage = 0;

	for (i = 0; i < gpuAdapterList_.size(); i++)
	{
		gpuAdapter = gpuAdapterList_[i];

		for (j = 0; j < gpuAdapter->SegmentCount; j++)
		{
			memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
			queryStatistics.Type = D3DKMT_QUERYSTATISTICS_PROCESS_SEGMENT;
			queryStatistics.AdapterLuid = gpuAdapter->AdapterLuid;
			queryStatistics.hProcess = targetProcessHandle_;
			queryStatistics.QueryProcessSegment.SegmentId = j;

			if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)))
			{
				ULONG64 bytesCommitted;

				if (windowsVersion_ >= WINDOWS_8)
					bytesCommitted = queryStatistics.QueryResult.ProcessSegmentInformation.BytesCommitted;
				else
					bytesCommitted = (ULONG)queryStatistics.QueryResult.ProcessSegmentInformation.BytesCommitted;

				if (RtlCheckBit(&gpuAdapter->ApertureBitMap, j))
					sharedUsage += bytesCommitted;
				else
					dedicatedUsage += bytesCommitted;
			}
			else {
				LOGE << TAG << "D3DKMT_QUERYSTATISTICS_PROCESS_SEGMENT D3DKMTQueryStatistics failed.\n";
			}
		}

		memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
		queryStatistics.Type = D3DKMT_QUERYSTATISTICS_PROCESS;
		queryStatistics.AdapterLuid = gpuAdapter->AdapterLuid;
		queryStatistics.hProcess = targetProcessHandle_;

		if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)))
		{
			commitUsage += queryStatistics.QueryResult.ProcessInformation.SystemMemory.BytesAllocated;
		}
		else
		{
			LOGE << TAG << "D3DKMT_QUERYSTATISTICS_PROCESS D3DKMTQueryStatistics failed.\n";
		}
	}

	targetProcessGpuDedicatedUsage_ = dedicatedUsage;
	//LOGI << TAG << "targetProcessGpuDedicatedUsage_: " << targetProcessGpuDedicatedUsage_ << "\n";
	targetProcessGpuSharedUsage_ = sharedUsage;
	//LOGI << TAG << "targetProcessGpuSharedUsage_: " << targetProcessGpuSharedUsage_ << "\n";
	targetProcessCommitUsage_ = commitUsage;
	//LOGI << TAG << "targetProcessGpuSharedUsage_: " << targetProcessGpuSharedUsage_ << "\n";
}

void GpuMonitor::EtpUpdateProcessNodeInformation()
{
	ULONG i;
	ULONG j;
	PETP_GPU_ADAPTER gpuAdapter;
	D3DKMT_QUERYSTATISTICS queryStatistics;
	ULONG64 totalRunningTime;

	if (!targetProcessHandle_)
	{
		LOGE << TAG << "targetProcessHandle_ is NULL.\n";
		return;
	}

	totalRunningTime = 0;

	for (i = 0; i < gpuAdapterList_.size(); i++)
	{
		gpuAdapter = gpuAdapterList_[i];

		for (j = 0; j < gpuAdapter->NodeCount; j++)
		{
			memset(&queryStatistics, 0, sizeof(D3DKMT_QUERYSTATISTICS));
			queryStatistics.Type = D3DKMT_QUERYSTATISTICS_PROCESS_NODE;
			queryStatistics.AdapterLuid = gpuAdapter->AdapterLuid;
			queryStatistics.hProcess = targetProcessHandle_;
			queryStatistics.QueryProcessNode.NodeId = j;

			if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)))
			{
				//ULONG64 runningTime;
				//runningTime = queryStatistics.QueryResult.ProcessNodeInformation.RunningTime.QuadPart;
				//PhUpdateDelta(&Block->GpuTotalRunningTimeDelta[j], runningTime);

				totalRunningTime += queryStatistics.QueryResult.ProcessNodeInformation.RunningTime.QuadPart;
				//totalContextSwitches += queryStatistics.QueryResult.ProcessNodeInformation.ContextSwitch;
			}
			else 
			{
				LOGE << TAG << "D3DKMT_QUERYSTATISTICS_PROCESS_NODE D3DKMTQueryStatistics failed.\n";
			}
		}
	}

	PhUpdateDelta(&GpuRunningTimeDelta_, totalRunningTime);
}


#include "Handles.h"
#include "common.h"
#include "../config/ntifs.h"

static const char* TAG = "lHandles.cpp:\t";

static const char* ntdllPath = "ntdll.dll";
static const char* gdi32Path = "gdi32.dll";

static HMODULE hNtdll;
static HMODULE gdi32dll;

RtlGetVersion fnRtlGetVersion;
RtlCreateHeap fnRtlCreateHeap;
RtlDestroyHeap fnRtlDestroyHeap;
RtlAllocateHeap fnRtlAllocateHeap;
RtlFreeHeap fnRtlFreeHeap;
RtlSizeHeap fnRtlSizeHeap;
RtlZeroHeap fnRtlZeroHeap;
RtlSetHeapInformation fnRtlSetHeapInformation;
RtlQueryHeapInformation fnRtlQueryHeapInformation;

RtlSetBits fnRtlSetBits;
RtlClearBits fnRtlClearBits;
RtlInitializeBitMap fnRtlInitializeBitMap;

//D3DKMTOpenAdapterFromDeviceName fnD3DKMTOpenAdapterFromDeviceName;
//D3DKMTQueryAdapterInfo fnD3DKMTQueryAdapterInfo;

bool loadDll() 
{   
    hNtdll = GetModuleHandle(ntdllPath);
    if (hNtdll == nullptr) {
        LOGI << TAG << "Load ntdll.dll Error.\n";
        return false;
    }

	gdi32dll = GetModuleHandle(gdi32Path);
	if (gdi32dll == nullptr) {
		LOGI << TAG << "Load gdi32.dll Error.\n";
		return false;
	}

	return true;
}

bool loadFunc()
{
	/*
		ntdll.dll
	*/
	fnRtlGetVersion = (RtlGetVersion)getFuncFromNtdll(hNtdll, "RtlGetVersion");
	if (fnRtlGetVersion == NULL) {
		LOGE << TAG << "Get fnRtlGetVersion Error.\n";
		return false;
	}

	fnRtlCreateHeap = (RtlCreateHeap)getFuncFromNtdll(hNtdll, "RtlCreateHeap");
	if (fnRtlCreateHeap == NULL) {
		LOGE << TAG << "Get fnRtlCreateHeap Error.\n";
		return false;
	}

	fnRtlDestroyHeap = (RtlDestroyHeap)getFuncFromNtdll(hNtdll, "RtlDestroyHeap");
	if (fnRtlDestroyHeap == NULL) {
		LOGE << TAG << "Get fnRtlDestroyHeap Error.\n";
		return false;
	}

	fnRtlAllocateHeap = (RtlAllocateHeap)getFuncFromNtdll(hNtdll, "RtlAllocateHeap");
	if (fnRtlAllocateHeap == NULL) {
		LOGE << TAG << "Get fnRtlAllocateHeap Error.\n";
		return false;
	}

	fnRtlFreeHeap = (RtlFreeHeap)getFuncFromNtdll(hNtdll, "RtlFreeHeap");
	if (fnRtlFreeHeap == NULL) {
		LOGE << TAG << "Get fnRtlFreeHeap Error.\n";
		return false;
	}

	fnRtlSizeHeap = (RtlSizeHeap)getFuncFromNtdll(hNtdll, "RtlSizeHeap");
	if (fnRtlSizeHeap == NULL) {
		LOGE << TAG << "Get fnRtlSizeHeap Error.\n";
		return false;
	}

	fnRtlZeroHeap = (RtlZeroHeap)getFuncFromNtdll(hNtdll, "RtlZeroHeap");
	if (fnRtlZeroHeap == NULL) {
		LOGE << TAG << "Get fnRtlZeroHeap Error.\n";
		return false;
	}

	fnRtlSetHeapInformation = (RtlSetHeapInformation)getFuncFromNtdll(hNtdll, "RtlSetHeapInformation");
	if (fnRtlSetHeapInformation == NULL) {
		LOGE << TAG << "Get fnRtlSetHeapInformation Error.\n";
		return false;
	}

	fnRtlQueryHeapInformation = (RtlQueryHeapInformation)getFuncFromNtdll(hNtdll, "RtlQueryHeapInformation");
	if (fnRtlQueryHeapInformation == NULL) {
		LOGE << TAG << "Get fnRtlQueryHeapInformation Error.\n";
		return false;
	}

	fnRtlSetBits = (RtlSetBits)getFuncFromNtdll(hNtdll, "RtlSetBits");
	if (fnRtlSetBits == NULL) {
		LOGE << TAG << "Get fnRtlSetBits Error.\n";
		return false;
	}

	fnRtlClearBits = (RtlClearBits)getFuncFromNtdll(hNtdll, "RtlClearBits");
	if (fnRtlClearBits == NULL) {
		LOGE << TAG << "Get fnRtlClearBits Error.\n";
		return false;
	}

	fnRtlInitializeBitMap = (RtlInitializeBitMap)getFuncFromNtdll(hNtdll, "RtlInitializeBitMap");
	if (fnRtlInitializeBitMap == NULL) {
		LOGE << TAG << "Get fnRtlInitializeBitMap Error.\n";
		return false;
	}

	/*
		gdi32.dll
	*/
	//fnD3DKMTOpenAdapterFromDeviceName = (D3DKMTOpenAdapterFromDeviceName)getFuncFromNtdll(gdi32dll, "D3DKMTOpenAdapterFromDeviceName");
	//if (fnD3DKMTOpenAdapterFromDeviceName == NULL) {
	//	LOGE << TAG << "Get fnD3DKMTOpenAdapterFromDeviceName Error.\n";
	//	return false;
	//}

	//fnD3DKMTQueryAdapterInfo = (D3DKMTQueryAdapterInfo)getFuncFromNtdll(gdi32dll, "D3DKMTQueryAdapterInfo");
	//if (fnD3DKMTQueryAdapterInfo == NULL) {
	//	LOGE << TAG << "Get fnD3DKMTQueryAdapterInfo Error.\n";
	//	return false;
	//}

    return true;
}

void* getFuncFromNtdll(HMODULE dllHandle, const char* funName)
{
    void* func = nullptr;
    func = GetProcAddress(dllHandle, funName);
    if (func == nullptr) {
        LOGE << TAG << "getFuncFromNtdll Error.\n";
        return nullptr;
    }

    return func;
}

void unLoadNtdll()
{
    //  TODO:
}
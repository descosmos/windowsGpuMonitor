#include "Handles.h"
#include "common.h"
#include "../config/ntifs.h"

static const char* TAG = "lHandles.cpp:\t";

/**
* Ntdll.dll
*/
static HMODULE hNtdll;

RtlGetVersion fnRtlGetVersion;
RtlCreateHeap fnRtlCreateHeap;
RtlDestroyHeap fnRtlDestroyHeap;
RtlAllocateHeap fnRtlAllocateHeap;
RtlFreeHeap fnRtlFreeHeap;
RtlSizeHeap fnRtlSizeHeap;
RtlZeroHeap fnRtlZeroHeap;
RtlSetHeapInformation fnRtlSetHeapInformation;
RtlQueryHeapInformation fnRtlQueryHeapInformation;


bool loadDll() 
{   
    hNtdll = GetModuleHandle("ntdll.dll");
    if (hNtdll == nullptr) {
        LOGI << TAG << "GetModuleHandle Error.\n";
        return false;
    }
	return true;
}

bool loadFunc()
{
	fnRtlGetVersion = (RtlGetVersion)getFuncFromNtdll("RtlGetVersion");
	if (fnRtlGetVersion == NULL) {
		LOGI << TAG << "Get fnRtlGetVersion Error.\n";
		return false;
	}

	fnRtlCreateHeap = (RtlCreateHeap)getFuncFromNtdll("RtlCreateHeap");
	if (fnRtlCreateHeap == NULL) {
		LOGI << TAG << "Get fnRtlCreateHeap Error.\n";
		return false;
	}

	fnRtlDestroyHeap = (RtlDestroyHeap)getFuncFromNtdll("RtlDestroyHeap");
	if (fnRtlDestroyHeap == NULL) {
		LOGI << TAG << "Get fnRtlDestroyHeap Error.\n";
		return false;
	}

	fnRtlAllocateHeap = (RtlAllocateHeap)getFuncFromNtdll("RtlAllocateHeap");
	if (fnRtlAllocateHeap == NULL) {
		LOGI << TAG << "Get fnRtlAllocateHeap Error.\n";
		return false;
	}

	fnRtlFreeHeap = (RtlFreeHeap)getFuncFromNtdll("RtlFreeHeap");
	if (fnRtlFreeHeap == NULL) {
		LOGI << TAG << "Get fnRtlFreeHeap Error.\n";
		return false;
	}

	fnRtlSizeHeap = (RtlSizeHeap)getFuncFromNtdll("RtlSizeHeap");
	if (fnRtlSizeHeap == NULL) {
		LOGI << TAG << "Get fnRtlSizeHeap Error.\n";
		return false;
	}

	fnRtlZeroHeap = (RtlZeroHeap)getFuncFromNtdll("RtlZeroHeap");
	if (fnRtlZeroHeap == NULL) {
		LOGI << TAG << "Get fnRtlZeroHeap Error.\n";
		return false;
	}

	fnRtlSetHeapInformation = (RtlSetHeapInformation)getFuncFromNtdll("RtlSetHeapInformation");
	if (fnRtlSetHeapInformation == NULL) {
		LOGI << TAG << "Get fnRtlSetHeapInformation Error.\n";
		return false;
	}

	fnRtlQueryHeapInformation = (RtlQueryHeapInformation)getFuncFromNtdll("RtlQueryHeapInformation");
	if (fnRtlQueryHeapInformation == NULL) {
		LOGI << TAG << "Get fnRtlQueryHeapInformation Error.\n";
		return false;
	}

    return true;
}

void* getFuncFromNtdll(const char* funName)
{
    void* func = nullptr;
    func = GetProcAddress(hNtdll, "RtlGetVersion");
    if (func == nullptr) {
        LOGI << TAG << "GetProcAddress Error.\n";
        printf("GetProcAddress Error.\n");
        return nullptr;
    }

    return func;
}

void unLoadNtdll()
{
    //  TODO:
}
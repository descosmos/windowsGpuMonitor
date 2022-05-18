#ifndef _RTL_HEAP_H
#define _RTL_HEAP_H

#include <Windows.h>
#include <../config/ntifs.h>

typedef LONG(WINAPI* RtlGetVersion) (RTL_OSVERSIONINFOW*);
typedef VOID(WINAPI* RtlCreateHeap) (ULONG Flags, PVOID HeapBase, SIZE_T ReserveSize, SIZE_T CommitSize, PVOID Lock, _RTL_HEAP_PARAMETERS Parameters);
typedef VOID(WINAPI* RtlDestroyHeap) (PVOID HeapBase);
typedef VOID(WINAPI* RtlAllocateHeap) (PVOID HeapBase, ULONG Flags, SIZE_T Size);
typedef BOOLEAN(WINAPI* RtlFreeHeap) (PVOID HeapBase, ULONG Flags, PVOID BaseAddress);
typedef SIZE_T(WINAPI* RtlSizeHeap) (PVOID HeapBase, ULONG Flags, PVOID BaseAddress);
typedef NTSTATUS(WINAPI* RtlZeroHeap) (PVOID HeapBase, ULONG Flags);
typedef NTSTATUS(WINAPI* RtlSetHeapInformation) (PVOID HeapBase, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength);
typedef NTSTATUS(WINAPI* RtlQueryHeapInformation) (PVOID HeapBase, HEAP_INFORMATION_CLASS HeapInformationClass, PVOID HeapInformation, SIZE_T HeapInformationLength, PSIZE_T ReturnLength);


bool loadDll();
bool loadFunc();
void* getFuncFromNtdll(const char* funName);
void unLoadNtdll();


#endif // !_RTL_HEAP_H

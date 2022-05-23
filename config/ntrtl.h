#ifndef _NTRTL_H
#define _NTRTL_H

#include <Windows.h>

typedef NTSTATUS(NTAPI* PRTL_HEAP_COMMIT_ROUTINE)(
    _In_ PVOID Base,
    _Inout_ PVOID* CommitAddress,
    _Inout_ PSIZE_T CommitSize
    );

//typedef struct _RTL_HEAP_PARAMETERS
//{
//    ULONG Length;
//    SIZE_T SegmentReserve;
//    SIZE_T SegmentCommit;
//    SIZE_T DeCommitFreeBlockThreshold;
//    SIZE_T DeCommitTotalFreeThreshold;
//    SIZE_T MaximumAllocationSize;
//    SIZE_T VirtualMemoryThreshold;
//    SIZE_T InitialCommit;
//    SIZE_T InitialReserve;
//    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
//    SIZE_T Reserved[2];
//} RTL_HEAP_PARAMETERS, * PRTL_HEAP_PARAMETERS;

// Bitmaps

typedef struct _RTL_BITMAP
{
    ULONG SizeOfBitMap;
    PULONG Buffer;
} RTL_BITMAP, * PRTL_BITMAP;

FORCEINLINE
BOOLEAN
RtlCheckBit(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_range_(< , BitMapHeader->SizeOfBitMap) ULONG BitPosition
)
{
#ifdef _WIN64
    return BitTest64((LONG64 const*)BitMapHeader->Buffer, (LONG64)BitPosition);
#else
    return (((PLONG)BitMapHeader->Buffer)[BitPosition / 32] >> (BitPosition % 32)) & 0x1;
#endif
}

#endif // !_NTRTL_H
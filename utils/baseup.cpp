#include <Windows.h>
#include <math.h>
#include <objbase.h>
#include <emmintrin.h>

#include "baseup.h"

#define PH_VECTOR_LEVEL_NONE 0
#define PH_VECTOR_LEVEL_SSE2 1
#define PH_VECTOR_LEVEL_AVX 2

#define PH_NATIVE_STRING_CONVERSION 1

// Misc.

static BOOLEAN PhpVectorLevel = PH_VECTOR_LEVEL_NONE;

/**
 * Determines the length of the specified string, in characters.
 *
 * \param String The string.
 */
SIZE_T PhCountStringZ(
    _In_ PWSTR String
)
{
#ifndef _ARM64_
    if (PhpVectorLevel >= PH_VECTOR_LEVEL_SSE2)
    {
        PWSTR p;
        ULONG unaligned;
        __m128i b;
        __m128i z;
        ULONG mask;
        ULONG index;

        p = (PWSTR)((ULONG_PTR)String & ~0xe); // String should be 2 byte aligned
        unaligned = PtrToUlong(String) & 0xf;
        z = _mm_setzero_si128();

        if (unaligned != 0)
        {
            b = _mm_load_si128((__m128i*)p);
            b = _mm_cmpeq_epi16(b, z);
            mask = _mm_movemask_epi8(b) >> unaligned;

            if (_BitScanForward(&index, mask))
                return index / sizeof(WCHAR);

            p += 16 / sizeof(WCHAR);
        }

        while (TRUE)
        {
            b = _mm_load_si128((__m128i*)p);
            b = _mm_cmpeq_epi16(b, z);
            mask = _mm_movemask_epi8(b);

            if (_BitScanForward(&index, mask))
                return (SIZE_T)(p - String) + index / sizeof(WCHAR);

            p += 16 / sizeof(WCHAR);
        }
    }
    else
#endif
    {
        return wcslen(String);
    }
}
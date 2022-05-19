#ifndef _REFP_H
#define _REFP_H
#include <Windows.h>
#include "phnt_ntdef.h"

/**
 * The object header contains object manager information including the reference count of an object
 * and its type.
 */
typedef struct _PH_OBJECT_HEADER
{
    union
    {
        struct
        {
            USHORT TypeIndex;
            UCHAR Flags;
            UCHAR Reserved1;
#ifdef _WIN64
            ULONG Reserved2;
#endif
            union
            {
                LONG RefCount;
                struct
                {
                    LONG SavedTypeIndex : 16;
                    LONG SavedFlags : 8;
                    LONG Reserved : 7;
                    LONG DeferDelete : 1; // MUST be the high bit, so that RefCount < 0 when deferring delete
                };
            };
#ifdef _WIN64
            ULONG Reserved3;
#endif
        };
        SLIST_ENTRY DeferDeleteListEntry;
    };

#ifdef DEBUG
    PVOID StackBackTrace[16];
    LIST_ENTRY ObjectListEntry;
#endif

    /**
     * The body of the object. For use by the \ref PhObjectToObjectHeader and
     * \ref PhObjectHeaderToObject macros.
     */
    QUAD_PTR Body;
} PH_OBJECT_HEADER, * PPH_OBJECT_HEADER;

/**
 * Gets a pointer to the object header for an object.
 *
 * \param Object A pointer to an object.
 *
 * \return A pointer to the object header of the object.
 */
#define PhObjectToObjectHeader(Object) ((PPH_OBJECT_HEADER)CONTAINING_RECORD((PCHAR)(Object), PH_OBJECT_HEADER, Body))

#endif // !_REFP_H

#ifndef __CU2CCE_CUDA_RUNTIME_TYPES_H__
#define __CU2CCE_CUDA_RUNTIME_TYPES_H__

/*
 * CUDA Runtime data-shape compatibility types.
 *
 * These are source-level containers only. They deliberately use cu2cce-owned
 * names so they cannot collide with compiler-provided CUDA/CANN types. The
 * adaptor maps the public CUDA spellings to these containers after the CANN
 * headers have been included.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wcceExtent {
    size_t width;
    size_t height;
    size_t depth;
} wcceExtent;

typedef struct wccePitchedPtr {
    void *ptr;
    size_t pitch;
    size_t xsize;
    size_t ysize;
} wccePitchedPtr;

typedef struct wccePos {
    size_t x;
    size_t y;
    size_t z;
} wccePos;

typedef enum wcceMemoryType {
    wcceMemoryTypeUnregistered = 0,
    wcceMemoryTypeHost = 1,
    wcceMemoryTypeDevice = 2,
    wcceMemoryTypeManaged = 3,
} wcceMemoryType;

typedef struct wccePointerAttributes {
    wcceMemoryType type;
    int device;
    void *devicePointer;
    void *hostPointer;
} wccePointerAttributes;

typedef enum wcceChannelFormatKind {
    wcceChannelFormatKindSigned = 0,
    wcceChannelFormatKindUnsigned = 1,
    wcceChannelFormatKindFloat = 2,
    wcceChannelFormatKindNone = 3,
} wcceChannelFormatKind;

typedef struct wcceChannelFormatDesc {
    int x;
    int y;
    int z;
    int w;
    wcceChannelFormatKind f;
} wcceChannelFormatDesc;

typedef void *wcceArray_t;
typedef const void *wcceArray_const_t;
typedef void *wcceMipmappedArray_t;
typedef uint64_t wcceTextureObject_t;
typedef uint64_t wcceSurfaceObject_t;

typedef struct wcceMemcpy3DParms {
    wcceArray_t srcArray;
    wccePos srcPos;
    wccePitchedPtr srcPtr;
    wcceArray_t dstArray;
    wccePos dstPos;
    wccePitchedPtr dstPtr;
    wcceExtent extent;
    int kind;
} wcceMemcpy3DParms;

/* Common cudaFuncAttributes fields. Values are only meaningful if a future
 * backend-specific implementation fills them; the current wrapper reports
 * API_NOT_SUPPORT for the attribute query. */
typedef struct wcceFuncAttributes {
    size_t sharedSizeBytes;
    int constSizeBytes;
    int localSizeBytes;
    int maxThreadsPerBlock;
    int numRegs;
    int ptxVersion;
    int binaryVersion;
    int cacheModeCA;
    int maxDynamicSharedSizeBytes;
    int preferredShmemCarveout;
} wcceFuncAttributes;

/* Opaque CUDA memory-pool shapes retained for source compatibility. The
 * current Ascend implementation does not expose CUDA mempool semantics. */
typedef void *wcceMemPool_t;
typedef struct wcceMemPoolProps {
    unsigned int allocType;
    unsigned int handleTypes;
    int locationType;
    int locationId;
} wcceMemPoolProps;

#ifdef __cplusplus
}
#endif

#endif /* __CU2CCE_CUDA_RUNTIME_TYPES_H__ */

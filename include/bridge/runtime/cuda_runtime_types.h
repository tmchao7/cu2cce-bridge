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

typedef enum wcceResourceType {
    wcceResourceTypeArray = 0,
    wcceResourceTypeMipmappedArray = 1,
    wcceResourceTypeLinear = 2,
    wcceResourceTypePitch2D = 3,
} wcceResourceType;

typedef enum wcceTextureAddressMode {
    wcceAddressModeWrap = 0,
    wcceAddressModeClamp = 1,
    wcceAddressModeMirror = 2,
    wcceAddressModeBorder = 3,
} wcceTextureAddressMode;

typedef enum wcceTextureFilterMode {
    wcceFilterModePoint = 0,
    wcceFilterModeLinear = 1,
} wcceTextureFilterMode;

typedef enum wcceTextureReadMode {
    wcceReadModeElementType = 0,
    wcceReadModeNormalizedFloat = 1,
} wcceTextureReadMode;

typedef enum wcceResourceViewFormat {
    wcceResourceViewFormatNone = 0,
    wcceResourceViewFormatUnsignedChar1 = 1,
    wcceResourceViewFormatUnsignedChar2 = 2,
    wcceResourceViewFormatUnsignedChar4 = 3,
    wcceResourceViewFormatSignedChar1 = 4,
    wcceResourceViewFormatSignedChar2 = 5,
    wcceResourceViewFormatSignedChar4 = 6,
    wcceResourceViewFormatUnsignedShort1 = 7,
    wcceResourceViewFormatUnsignedShort2 = 8,
    wcceResourceViewFormatUnsignedShort4 = 9,
    wcceResourceViewFormatSignedShort1 = 10,
    wcceResourceViewFormatSignedShort2 = 11,
    wcceResourceViewFormatSignedShort4 = 12,
    wcceResourceViewFormatUnsignedInt1 = 13,
    wcceResourceViewFormatUnsignedInt2 = 14,
    wcceResourceViewFormatUnsignedInt4 = 15,
    wcceResourceViewFormatSignedInt1 = 16,
    wcceResourceViewFormatSignedInt2 = 17,
    wcceResourceViewFormatSignedInt4 = 18,
    wcceResourceViewFormatHalf1 = 19,
    wcceResourceViewFormatHalf2 = 20,
    wcceResourceViewFormatHalf4 = 21,
    wcceResourceViewFormatFloat1 = 22,
    wcceResourceViewFormatFloat2 = 23,
    wcceResourceViewFormatFloat4 = 24,
} wcceResourceViewFormat;

typedef struct wcceResourceDesc {
    wcceResourceType resType;
    union {
        struct { wcceArray_t array; } array;
        struct { wcceMipmappedArray_t mipmap; } mipmap;
        struct {
            void *devPtr;
            wcceChannelFormatDesc desc;
            size_t sizeInBytes;
        } linear;
        struct {
            void *devPtr;
            wcceChannelFormatDesc desc;
            size_t width;
            size_t height;
            size_t pitchInBytes;
        } pitch2D;
    } res;
    unsigned int flags;
} wcceResourceDesc;

typedef struct wcceTextureDesc {
    wcceTextureAddressMode addressMode[3];
    wcceTextureFilterMode filterMode;
    wcceTextureReadMode readMode;
    int sRGB;
    int normalizedCoords;
    unsigned int maxAnisotropy;
    wcceTextureFilterMode mipmapFilterMode;
    float mipmapLevelBias;
    float minMipmapLevelClamp;
    float maxMipmapLevelClamp;
    float borderColor[4];
} wcceTextureDesc;

typedef struct wcceResourceViewDesc {
    wcceResourceViewFormat format;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int firstMipmapLevel;
    unsigned int lastMipmapLevel;
    unsigned int firstLayer;
    unsigned int lastLayer;
} wcceResourceViewDesc;

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

typedef struct wcceMemcpy3DPeerParms {
    int srcDevice;
    wccePitchedPtr srcPtr;
    wccePos srcPos;
    int dstDevice;
    wccePitchedPtr dstPtr;
    wccePos dstPos;
    wcceExtent extent;
} wcceMemcpy3DPeerParms;

/* Common cudaFuncAttributes fields. Values are only meaningful if a future
 * backend-specific implementation fills them; the current wrapper reports
 * API_NOT_SUPPORT for the attribute query. */
typedef enum wcceFuncCache {
    wcceFuncCachePreferNone = 0,
    wcceFuncCachePreferShared = 1,
    wcceFuncCachePreferL1 = 2,
    wcceFuncCachePreferEqual = 3,
} wcceFuncCache;

typedef enum wcceSharedMemConfig {
    wcceSharedMemBankSizeDefault = 0,
    wcceSharedMemBankSizeFourByte = 1,
    wcceSharedMemBankSizeEightByte = 2,
} wcceSharedMemConfig;

typedef enum wcceLimit {
    wcceLimitStackSize = 0,
    wcceLimitPrintfFifoSize = 1,
    wcceLimitMallocHeapSize = 2,
    wcceLimitDevRuntimeSyncDepth = 3,
    wcceLimitDevRuntimePendingLaunchCount = 4,
} wcceLimit;

#if defined(__has_include)
#  if __has_include(<acl/acl_base.h>) && __has_include(<acl/acl_rt.h>)
#    include <acl/acl_base.h>
#    include <acl/acl_rt.h>
typedef void (*wcceStreamCallback_t)(aclrtStream stream, aclError status, void *userData);
#  else
typedef void (*wcceStreamCallback_t)(void *stream, int status, void *userData);
#  endif
#else
typedef void (*wcceStreamCallback_t)(void *stream, int status, void *userData);
#endif

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

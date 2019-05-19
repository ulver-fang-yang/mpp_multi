

#ifndef MPP_MULTI_H
#define MPP_MULTI_H


#include "rk_mpi.h"
#include "mpp_mem.h"
#include "mpp_env.h"
#include "mpp_time.h"
#include "mpp_common.h"


#define fy_log(format, ...) printf("[log]%s:%d %s()| "format"\r\n",__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define fy_err(format, ...) printf("[error]%s:%d %s()| "format"\r\n",__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define fy_debug(format, ...) printf("[debug]%s:%d %s()| "format"\r\n",__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

#define MPP_ABORT                       (0x10000000)
#define MPP_DBG_TIMING                  (0x00000001)
#define MPP_STRINGS(x)      MPP_TO_STRING(x)
#define MPP_TO_STRING(x)    #x


#define fy_abort() do {                \
    if (MPP_DBG_TIMING & MPP_ABORT) {        \
        abort();                        \
    }                                   \
} while (0)

#define fy_assert(cond) do {                                           \
    if (!(cond)) {                                                      \
        fy_err("Assertion %s failed at %s:%d\n",                       \
               MPP_STRINGS(cond), __FUNCTION__, __LINE__);              \
        fy_abort();                                                    \
    }                                                                   \
} while (0)

#define show_options(opt) \
    do { \
        _show_options(sizeof(opt)/sizeof(OptionInfo), opt); \
    } while (0)

#define MAX_FILE_NAME_LENGTH 256


typedef struct OptionInfo_t {
    const char*     name;
    const char*     argname;
    const char*     help;
} OptionInfo;

typedef struct mpp_multi_cmd_s
{
    char            file_input[MAX_FILE_NAME_LENGTH];
    char            file_output[MAX_FILE_NAME_LENGTH];
    RK_U8           codec_type;
    MppCodingType   coding_type;
    MppFrameFormat  format;
    RK_U32          width;
    RK_U32          height;
    RK_S32          frame_num;
    RK_S8           payload_num;
    size_t          pkt_size;

    // report information
    size_t          max_usage;

    //  base flow context
    MppCtx          ctx;
    MppApi          *mpi;

    //  input / output
    MppPacket       packet;
} mpp_multi_cmd_t;


#endif  //  MPP_MULTI_H



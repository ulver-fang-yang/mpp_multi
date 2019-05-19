

#ifndef MPP_MULTI_ENC_H
#define MPP_MULTI_ENC_H


#include "mpp_multi.h"


typedef struct {
    // global flow control flag
    uint32_t frm_eos;
    uint32_t pkt_eos;
    uint32_t frame_count;
//    RK_U64 stream_size;

    // src and dst
    FILE *output_fp;

    // base flow context
    MppCtx ctx;
    MppApi *mpi;
    MppEncPrepCfg prep_cfg;
    MppEncRcCfg rc_cfg;
    MppEncCodecCfg codec_cfg;

    // input / output
    MppBuffer frm_buf;
    MppEncSeiMode sei_mode;

    // paramter for resource malloc
    uint32_t width;
    uint32_t height;
    uint32_t hor_stride;
    uint32_t ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;
    uint32_t num_frames;

    // resources
    size_t frame_size;
    /* NOTE: packet buffer may overflow */
    size_t packet_size;

    // rate control runtime parameter
    int gop;
    int fps;
    int bps;
} MpiEncData;


void mpp_multi_enc(mpp_multi_cmd_t *cmd);


#endif  //  MPP_MULTI_ENC_H



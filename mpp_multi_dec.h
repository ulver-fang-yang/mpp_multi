
#ifndef MPP_MULTI_DEC_H
#define MPP_MULTI_DEC_H


#include "mpp_multi.h"


typedef struct {
    MppCtx          ctx;
    MppApi          *mpi;
    MppBufferGroup  frame_group;

    //  end of stream flag when set quit the loop
    RK_U32          eos;


    //  input and output */
    MppBufferGroup  frm_grp;
    MppBufferGroup  pkt_grp;
    MppPacket       packet;
    size_t          packet_size;
    MppFrame        frame;

    //  buffer for stream data reading
    uint8_t         *input_buf;
    FILE            *output_fp;
    RK_S32          frame_count;
    RK_S32          frame_num;
    size_t          max_usage;
} MpiDecLoopData;


void mpp_multi_dec(mpp_multi_cmd_t *cmd);


#endif  //  MPP_MULTI_DEC_H


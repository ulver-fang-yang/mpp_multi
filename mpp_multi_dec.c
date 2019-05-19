

#include "mpp_multi_dec.h"

//#define NEED_POLL_BLOCK

void dump_mpp_frame_to_file(MppFrame frame, FILE *fp)
{
    RK_U32 width    = 0;
    RK_U32 height   = 0;
    RK_U32 h_stride = 0;
    RK_U32 v_stride = 0;
    MppFrameFormat fmt  = MPP_FMT_YUV420SP;
    MppBuffer buffer    = NULL;
    RK_U8 *base = NULL;

    if (NULL == fp || NULL == frame)
        return ;

    width    = mpp_frame_get_width(frame);
    height   = mpp_frame_get_height(frame);
    h_stride = mpp_frame_get_hor_stride(frame);
    v_stride = mpp_frame_get_ver_stride(frame);
    fmt      = mpp_frame_get_fmt(frame);
    buffer   = mpp_frame_get_buffer(frame);

    if (NULL == buffer)
        return ;

    base = (RK_U8 *)mpp_buffer_get_ptr(buffer);

    switch (fmt) {
    case MPP_FMT_YUV422SP : {
        /* YUV422SP -> YUV422P for better display */
        RK_U32 i, j;
        RK_U8 *base_y = base;
        RK_U8 *base_c = base + h_stride * v_stride;
        RK_U8 *tmp = mpp_malloc(RK_U8, h_stride * height * 2);
        RK_U8 *tmp_u = tmp;
        RK_U8 *tmp_v = tmp + width * height / 2;

        for (i = 0; i < height; i++, base_y += h_stride)
            fwrite(base_y, 1, width, fp);

        for (i = 0; i < height; i++, base_c += h_stride) {
            for (j = 0; j < width / 2; j++) {
                tmp_u[j] = base_c[2 * j + 0];
                tmp_v[j] = base_c[2 * j + 1];
            }
            tmp_u += width / 2;
            tmp_v += width / 2;
        }

        fwrite(tmp, 1, width * height, fp);
        mpp_free(tmp);
    } break;
    case MPP_FMT_YUV420SP : {
        RK_U32 i;
        RK_U8 *base_y = base;
        RK_U8 *base_c = base + h_stride * v_stride;

        for (i = 0; i < height; i++, base_y += h_stride) {
            fwrite(base_y, 1, width, fp);
        }
        for (i = 0; i < height / 2; i++, base_c += h_stride) {
            fwrite(base_c, 1, width, fp);
        }
    } break;
    case MPP_FMT_YUV420P : {
        RK_U32 i;
        RK_U8 *base_y = base;
        RK_U8 *base_c = base + h_stride * v_stride;

        for (i = 0; i < height; i++, base_y += h_stride) {
            fwrite(base_y, 1, width, fp);
        }
        for (i = 0; i < height / 2; i++, base_c += h_stride / 2) {
            fwrite(base_c, 1, width / 2, fp);
        }
        for (i = 0; i < height / 2; i++, base_c += h_stride / 2) {
            fwrite(base_c, 1, width / 2, fp);
        }
    } break;
    case MPP_FMT_YUV444SP : {
        /* YUV444SP -> YUV444P for better display */
        RK_U32 i, j;
        RK_U8 *base_y = base;
        RK_U8 *base_c = base + h_stride * v_stride;
        RK_U8 *tmp = mpp_malloc(RK_U8, h_stride * height * 2);
        RK_U8 *tmp_u = tmp;
        RK_U8 *tmp_v = tmp + width * height;

        for (i = 0; i < height; i++, base_y += h_stride)
            fwrite(base_y, 1, width, fp);

        for (i = 0; i < height; i++, base_c += h_stride * 2) {
            for (j = 0; j < width; j++) {
                tmp_u[j] = base_c[2 * j + 0];
                tmp_v[j] = base_c[2 * j + 1];
            }
            tmp_u += width;
            tmp_v += width;
        }

        fwrite(tmp, 1, width * height * 2, fp);
        mpp_free(tmp);
    } break;
    default : {
        fy_err("not supported format %d\n", fmt);
    } break;
    }
}

int decode_simple(MpiDecLoopData *data, int eos)
{
    RK_U32 pkt_done = 0;
    RK_U32 err_info = 0;
    MPP_RET ret = MPP_OK;
    MppCtx ctx  = data->ctx;
    MppApi *mpi = data->mpi;
    MppPacket packet = data->packet;
#ifdef NEED_POLL_BLOCK_SET_FRAME
    MppBufferGroup  frm_grp;
    MppBuffer grp_buf;
#endif
    MppFrame  frame  = NULL;

    //  write data to packet
    mpp_packet_write(packet, 0, data->input_buf, data->packet_size);
    //  reset pos and set valid length
    mpp_packet_set_pos(packet, data->input_buf);
    mpp_packet_set_length(packet, data->packet_size);
    if (eos == 1)
        // setup eos flag
        mpp_packet_set_eos(packet);

#ifdef NEED_POLL_BLOCK_SET_FRAME
    ret = mpp_buffer_group_get_internal(&data->frm_grp, MPP_BUFFER_TYPE_ION);
    if (ret)
    {
        fy_err("get mpp buffer group rga_grp_ failed ret %d\n", ret);
        return -1;
//        goto RET;
    }

    ret = mpp_buffer_get(data->frm_grp, &grp_buf, 1920 * 1080 * 4);
    if (ret)
    {
        fy_err("get mpp buffer rga_grp_ failed ret:%d\n", ret);
        return -1;
//        goto RET;
    }

    mpp_frame_set_buffer(frame, grp_buf);
    fy_log("mpp_frame_set_buffer\n");
//#endif

    RK_S32 usedslots, freeslots;
    
    ret = mpi->control(data->ctx, MPP_DEC_GET_STREAM_COUNT, &usedslots);
    if (ret != MPP_OK)
    {
        fy_err("Failed to get decoder used slots (code = %d).\n", ret);
        return -1;
//        goto RET;
    }

    freeslots = 4 - usedslots;
    if (freeslots <= 0)
    {
        ret = MPP_ERR_UNKNOW;
        return -1;
//        goto RET;
    }
#endif

    do {
        RK_S32 times = 5;
        //  send the packet first if packet is not done
        if (!pkt_done) {
            ret = mpi->decode_put_packet(data->ctx, packet);
            if (MPP_OK == ret)
                pkt_done = 1;
        }
        fy_log("decode_put_frame, packet: %x, ret: %d\n", (int)packet, ret);

        //  then get all available frame and release
        do {
            RK_S32 get_frm = 0;
            RK_U32 frm_eos = 0;

        try_again:
            ret = mpi->decode_get_frame(data->ctx, &frame);
            fy_log("decode_get_frame, frame: %x, ret: %d\n", (int)frame, ret);
            if (MPP_ERR_TIMEOUT == ret) {
                if (times > 0) {
                    times--;
                    msleep(2);
                    goto try_again;
                }
                fy_err("decode_get_frame failed too much time\n");
            }
            if (MPP_OK != ret) {
                fy_err("decode_get_frame failed ret %d\n", ret);
                break;
            }

            if (frame) {
                if (mpp_frame_get_info_change(frame)) {
                    RK_U32 width = mpp_frame_get_width(frame);
                    RK_U32 height = mpp_frame_get_height(frame);
                    RK_U32 hor_stride = mpp_frame_get_hor_stride(frame);
                    RK_U32 ver_stride = mpp_frame_get_ver_stride(frame);
                    RK_U32 buf_size = mpp_frame_get_buf_size(frame);

                    fy_log("decode_get_frame get info changed found\n");
                    fy_log("decoder require buffer w:h [%d:%d] stride [%d:%d] buf_size %d",
                            width, height, hor_stride, ver_stride, buf_size);

                    if (NULL == data->frm_grp) {
                        //  If buffer group is not set create one and limit it
                        ret = mpp_buffer_group_get_internal(&data->frm_grp, MPP_BUFFER_TYPE_ION);
                        if (ret) {
                            fy_err("get mpp buffer group failed ret %d\n", ret);
                            break;
                        }

                        //  Set buffer to mpp decoder
                        ret = mpi->control(ctx, MPP_DEC_SET_EXT_BUF_GROUP, data->frm_grp);
                        if (ret) {
                            fy_err("set buffer group failed ret %d\n", ret);
                            break;
                        }
                    } else {
                        //  If old buffer group exist clear it
                        ret = mpp_buffer_group_clear(data->frm_grp);
                        if (ret) {
                            fy_err("clear buffer group failed ret %d\n", ret);
                            break;
                        }
                    }

                    //  Use limit config to limit buffer count to 24 with buf_size
                    ret = mpp_buffer_group_limit_config(data->frm_grp, buf_size, 24);
                    if (ret) {
                        fy_err("limit buffer group failed ret %d\n", ret);
                        break;
                    }

                    //  All buffer group config done. Set info change ready to let decoder continue decoding
                    ret = mpi->control(ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
                    if (ret) {
                        fy_err("info change ready failed ret %d\n", ret);
                        break;
                    }
                } else {
                    err_info = mpp_frame_get_errinfo(frame) | mpp_frame_get_discard(frame);
                    if (err_info) {
                        fy_log("decoder_get_frame get err info:%d discard:%d.\n",
                                mpp_frame_get_errinfo(frame), mpp_frame_get_discard(frame));
                    }
                    data->frame_count++;
                    fy_log("decode_get_frame get frame %d\n", data->frame_count);
                    if (data->output_fp && !err_info)
                        dump_mpp_frame_to_file(frame, data->output_fp);
                }
                frm_eos = mpp_frame_get_eos(frame);
                mpp_frame_deinit(&frame);
                frame = NULL;
                get_frm = 1;
            }
            else
                break;

            //  try get runtime frame memory usage
            if (data->frm_grp) {
                size_t usage = mpp_buffer_group_usage(data->frm_grp);
                if (usage > data->max_usage)
                    data->max_usage = usage;
            }

            //  if last packet is send but last frame is not found continue
            if (pkt_done && !frm_eos) {
                msleep(10);
                continue;
            }

            if (frm_eos) {
                fy_log("found last frame\n");
                break;
            }

            if (data->frame_num > 0 && data->frame_count >= data->frame_num) {
                data->eos = 1;
                break;
            }

            if (get_frm)
                continue;
            break;
        } while (1);

//        if (data->frame_num > 0 && data->frame_count >= data->frame_num) {
        if (data->frame_count >= data->frame_num) {
            data->eos = 1;
            fy_log("reach max frame number %d\n", data->frame_count);
            break;
        }

        if (pkt_done)
        {
            data->eos = 1;
            break;
        }

        msleep(3);
    } while (1);

    return ret;
}

void mpp_decode(MpiDecLoopData *data, int eos)
{
/*
    data->ctx            = cmd->ctx;
    data->mpi            = cmd->mpi;
    data->eos            = 0;
    data->packet         = cmd->packet;
    data->frame          = NULL;
    data->frame_count    = 0;
    data->frame_num      = cmd->frame_num;*/

    while (!data->eos)
        decode_simple(data, eos);
}

void mpp_multi_dec_run(mpp_multi_cmd_t *cmd, int pindex)
{
    MPP_RET ret = MPP_OK;
    MpiDecLoopData data;

    MpiCmd mpi_cmd      = MPP_CMD_BASE;
    MppParam param      = NULL;
    RK_U32 need_split   = 1;
#ifdef NEED_POLL_BLOCK
    RK_S64 paramS64;
    RK_S32 paramS32;
#endif

    memset(&data, 0, sizeof(data));

    FILE *in_fp = fopen(cmd->file_input, "r");
    if (NULL == in_fp)
    {
        fy_err("open input file fail\n");
        goto MULTI_DEC_ERR_OUT;
    }

    char file_output[MAX_FILE_NAME_LENGTH];
    sprintf(file_output, "%s_0%d", cmd->file_output, pindex);
    data.output_fp = fopen(file_output, "a+");
    if (NULL == data.output_fp)
    {
        fy_err("open output file fail\n");
        goto MULTI_DEC_ERR_OUT;
    }

    fseek(in_fp, 0, SEEK_END);
    uint32_t file_size = ftell(in_fp);
    fseek(in_fp, 0, SEEK_SET);

    uint32_t pos = 0;
    uint8_t *buf = (uint8_t *)malloc(cmd->pkt_size);
    if (NULL == buf)
    {
        fy_err("alloc memory fail\n");
        goto MULTI_DEC_ERR_OUT;
    }
    
    ret = mpp_packet_init(&cmd->packet, buf, cmd->pkt_size);
    if (ret)
    {
        fy_err("mpp_packet_init failed\n");
        goto MULTI_DEC_ERR_OUT;
    }
#ifdef DBG_LOG
    fy_log("cmd->packet: %x\n", (int)cmd->packet);
#endif

#ifdef DBG_LOG
    fy_log("mpp_multi_dec decoder test start w %d h %d type %d\n", cmd->width, cmd->height, cmd->coding_type);
#endif

    ret = mpp_create(&cmd->ctx, &cmd->mpi);

    if (MPP_OK != ret)
    {
        fy_err("mpp_create failed\n");
        goto MULTI_DEC_ERR_OUT;
    }

    //  NOTE: decoder split mode need to be set before init
    mpi_cmd = MPP_DEC_SET_PARSER_SPLIT_MODE;
    param = &need_split;
    ret = cmd->mpi->control(cmd->ctx, mpi_cmd, param);
    if (MPP_OK != ret)
    {
        fy_err("mpi->control failed\n");
        goto MULTI_DEC_ERR_OUT;
    }

    ret = mpp_init(cmd->ctx, MPP_CTX_DEC, cmd->coding_type);
    if (MPP_OK != ret)
    {
        fy_err("mpp_init failed\n");
        goto MULTI_DEC_ERR_OUT;
    }

#ifdef NEED_POLL_BLOCK
    //  make decode calls blocking with a timeout
    paramS32 = MPP_POLL_BLOCK;
    ret = cmd->mpi->control(cmd->ctx, MPP_SET_OUTPUT_BLOCK, &paramS32);
    if (ret != MPP_OK)
    {
        fy_err("Failed to set blocking mode on MPI (code = %d).\n", ret);
        goto MULTI_DEC_ERR_OUT;
    }

    paramS64 = 100;
    ret = cmd->mpi->control(cmd->ctx, MPP_SET_OUTPUT_BLOCK_TIMEOUT, &paramS64);
    if (ret != MPP_OK)
    {
        fy_err("Failed to set block timeout on MPI (code = %d).\n", ret);
        goto MULTI_DEC_ERR_OUT;
    }
#ifdef DBG_LOG
    fy_log("paramS64: %llu\n", paramS64);
#endif
#endif
    data.ctx            = cmd->ctx;
    data.mpi            = cmd->mpi;
    data.packet         = cmd->packet;
    data.frame_num      = cmd->frame_num;
    data.input_buf      = buf;

    while (pos <= file_size)
    {
        data.eos            = 0;
        data.frame          = NULL;
        data.frame_count    = 0;
        if (pos + cmd->pkt_size < file_size)
        {
            fread(buf, cmd->pkt_size, 1, in_fp);
            data.packet_size = cmd->pkt_size;
            mpp_decode(&data, 0);
            pos += cmd->pkt_size;
        }
        else
        {
            fread(buf, file_size - pos, 1, in_fp);
            data.packet_size = file_size - pos;
            mpp_decode(&data, 1);
            pos = file_size;
        }
    }

    fclose(in_fp);
    fclose(data.output_fp);

    exit(0);

MULTI_DEC_ERR_OUT:
    if (cmd->packet)
    {
        mpp_packet_deinit(&cmd->packet);
        cmd->packet = NULL;
    }

    if (cmd->ctx)
    {
        mpp_destroy(cmd->ctx);
        cmd->ctx = NULL;
    }

    if (in_fp)
        fclose(in_fp);

    if (data.output_fp)
        fclose(data.output_fp);

    exit(-1);
}

void mpp_multi_dec(mpp_multi_cmd_t *cmd)
{
//    MPP_RET ret = MPP_OK;

    cmd->pkt_size = SZ_64K;

#ifdef DBG_LOG
    fy_log("mpp_multi_dec start\n");
#endif

    pid_t dec_pid;
    int pindex = 0;

    while (pindex < cmd->payload_num)
    {
        dec_pid = fork();

        if (dec_pid < 0)
            fy_err("fork fail\n");
        else if (dec_pid == 0)
        {
            fy_log("enter child process: %d\n", pindex);
            mpp_multi_dec_run(cmd, pindex);
        }
        pindex++;
    }
}


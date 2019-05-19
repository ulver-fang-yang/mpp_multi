

#include "mpp_multi_enc.h"

#define NEED_WRITE_LEN

#ifdef NEED_WRITE_LEN
#include <arpa/inet.h>
#endif


MPP_RET read_yuv_image(RK_U8 *buf, FILE *fp, RK_U32 width, RK_U32 height,
                       RK_U32 hor_stride, RK_U32 ver_stride, MppFrameFormat fmt)
{
    MPP_RET ret = MPP_OK;
    RK_U32 read_size;
    RK_U32 row = 0;
    RK_U8 *buf_y = buf;
    RK_U8 *buf_u = buf_y + hor_stride * ver_stride; // NOTE: diff from gen_yuv_image
    RK_U8 *buf_v = buf_u + hor_stride * ver_stride / 4; // NOTE: diff from gen_yuv_image

    switch (fmt) {
    case MPP_FMT_YUV420SP : {
        for (row = 0; row < height; row++) {
            read_size = fread(buf_y + row * hor_stride, 1, width, fp);
            if (read_size != width) {
                fy_err("read ori yuv file luma failed");
                ret  = MPP_NOK;
                goto err;
            }
        }

        for (row = 0; row < height / 2; row++) {
            read_size = fread(buf_u + row * hor_stride, 1, width, fp);
            if (read_size != width) {
                fy_err("read ori yuv file cb failed");
                ret  = MPP_NOK;
                goto err;
            }
        }
    } break;
    case MPP_FMT_YUV420P : {
        for (row = 0; row < height; row++) {
            read_size = fread(buf_y + row * hor_stride, 1, width, fp);
            if (read_size != width) {
                fy_err("read ori yuv file luma failed");
                ret  = MPP_NOK;
                goto err;
            }
        }

        for (row = 0; row < height / 2; row++) {
            read_size = fread(buf_u + row * hor_stride / 2, 1, width / 2, fp);
            if (read_size != width / 2) {
                fy_err("read ori yuv file cb failed");
                ret  = MPP_NOK;
                goto err;
            }
        }

        for (row = 0; row < height / 2; row++) {
            read_size = fread(buf_v + row * hor_stride / 2, 1, width / 2, fp);
            if (read_size != width / 2) {
                fy_err("read ori yuv file cr failed");
                ret  = MPP_NOK;
                goto err;
            }
        }
    } break;
    case MPP_FMT_ARGB8888 : {
        for (row = 0; row < height; row++) {
            read_size = fread(buf_y + row * hor_stride * 4, 1, width * 4, fp);
        }
    } break;
    case MPP_FMT_YUV422_YUYV :
    case MPP_FMT_YUV422_UYVY : {
        for (row = 0; row < height; row++) {
            read_size = fread(buf_y + row * hor_stride, 1, width * 2, fp);
        }
    } break;
    default : {
        fy_err("read image do not support fmt %d\n", fmt);
        ret = MPP_ERR_VALUE;
    } break;
    }

err:

    return ret;
}

MPP_RET fill_yuv_image(RK_U8 *buf, RK_U32 width, RK_U32 height,
                       RK_U32 hor_stride, RK_U32 ver_stride, MppFrameFormat fmt,
                       RK_U32 frame_count)
{
    MPP_RET ret = MPP_OK;
    RK_U8 *buf_y = buf;
    RK_U8 *buf_c = buf + hor_stride * ver_stride;
    RK_U32 x, y;

    switch (fmt) {
    case MPP_FMT_YUV420SP : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width; x++) {
                p[x] = x + y + frame_count * 3;
            }
        }

        p = buf_c;
        for (y = 0; y < height / 2; y++, p += hor_stride) {
            for (x = 0; x < width / 2; x++) {
                p[x * 2 + 0] = 128 + y + frame_count * 2;
                p[x * 2 + 1] = 64  + x + frame_count * 5;
            }
        }
    } break;
    case MPP_FMT_YUV420P : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width; x++) {
                p[x] = x + y + frame_count * 3;
            }
        }

        p = buf_c;
        for (y = 0; y < height / 2; y++, p += hor_stride / 2) {
            for (x = 0; x < width / 2; x++) {
                p[x] = 128 + y + frame_count * 2;
            }
        }

        p = buf_c + hor_stride * ver_stride / 4;
        for (y = 0; y < height / 2; y++, p += hor_stride / 2) {
            for (x = 0; x < width / 2; x++) {
                p[x] = 64 + x + frame_count * 5;
            }
        }
    } break;
    case MPP_FMT_YUV422_UYVY : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width / 2; x++) {
                p[x * 4 + 1] = x * 2 + 0 + y + frame_count * 3;
                p[x * 4 + 3] = x * 2 + 1 + y + frame_count * 3;
                p[x * 4 + 0] = 128 + y + frame_count * 2;
                p[x * 4 + 2] = 64  + x + frame_count * 5;
            }
        }
    } break;
    default : {
        fy_err("filling function do not support type %d\n", fmt);
        ret = MPP_NOK;
    } break;
    }
    return ret;
}

MPP_RET ctx_init(mpp_multi_cmd_t *cmd, MpiEncData *p)
{
    MPP_RET ret = MPP_OK;

    if (!cmd)
    {
        fy_err("invalid input cmd %p\n", cmd);
        return MPP_ERR_NULL_PTR;
    }

    // get paramter from cmd
    p->width        = cmd->width;
    p->height       = cmd->height;
    p->hor_stride   = MPP_ALIGN(cmd->width, 16);
    p->ver_stride   = MPP_ALIGN(cmd->height, 16);
    p->fmt          = cmd->format;
    p->type         = cmd->coding_type;
    if (cmd->coding_type == MPP_VIDEO_CodingMJPEG)
        cmd->frame_num = 1;
    p->num_frames   = cmd->frame_num;

    // update resource parameter
    if (p->fmt <= MPP_FMT_YUV420SP_VU)
        p->frame_size = p->hor_stride * p->ver_stride * 3 / 2;
    else if (p->fmt <= MPP_FMT_YUV422_UYVY) {
        // NOTE: yuyv and uyvy need to double stride
        p->hor_stride *= 2;
        p->frame_size = p->hor_stride * p->ver_stride;
    } else
        p->frame_size = p->hor_stride * p->ver_stride * 4;
    p->packet_size  = p->width * p->height;

    return ret;
}

MPP_RET ctx_deinit(MpiEncData **data)
{
    MpiEncData *p = NULL;

    if (!data)
    {
        fy_err("invalid input data %p\n", data);
        return MPP_ERR_NULL_PTR;
    }

    p = *data;
    if (p)
    {
        MPP_FREE(p);
        *data = NULL;
    }

    return MPP_OK;
}

MPP_RET mpp_setup(MpiEncData *p)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    MppEncCodecCfg *codec_cfg;
    MppEncPrepCfg *prep_cfg;
    MppEncRcCfg *rc_cfg;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;
    codec_cfg = &p->codec_cfg;
    prep_cfg = &p->prep_cfg;
    rc_cfg = &p->rc_cfg;

    /* setup default parameter */
    p->fps = 30;
    p->gop = 60;
    p->bps = p->width * p->height / 8 * p->fps;

    prep_cfg->change        = MPP_ENC_PREP_CFG_CHANGE_INPUT |
                              MPP_ENC_PREP_CFG_CHANGE_ROTATION |
                              MPP_ENC_PREP_CFG_CHANGE_FORMAT;
    prep_cfg->width         = p->width;
    prep_cfg->height        = p->height;
    prep_cfg->hor_stride    = p->hor_stride;
    prep_cfg->ver_stride    = p->ver_stride;
    prep_cfg->format        = p->fmt;
    prep_cfg->rotation      = MPP_ENC_ROT_0;
    ret = mpi->control(ctx, MPP_ENC_SET_PREP_CFG, prep_cfg);
    if (ret) {
        fy_err("mpi control enc set prep cfg failed ret %d\n", ret);
        goto RET;
    }

    rc_cfg->change  = MPP_ENC_RC_CFG_CHANGE_ALL;
    rc_cfg->rc_mode = MPP_ENC_RC_MODE_CBR;
    rc_cfg->quality = MPP_ENC_RC_QUALITY_MEDIUM;

    if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_CBR) {
        rc_cfg->bps_target   = p->bps;
        rc_cfg->bps_max      = p->bps * 17 / 16;
        rc_cfg->bps_min      = p->bps * 15 / 16;
    } else if (rc_cfg->rc_mode ==  MPP_ENC_RC_MODE_VBR) {
        if (rc_cfg->quality == MPP_ENC_RC_QUALITY_CQP) {
            rc_cfg->bps_target   = -1;
            rc_cfg->bps_max      = -1;
            rc_cfg->bps_min      = -1;
        } else {
            rc_cfg->bps_target   = p->bps;
            rc_cfg->bps_max      = p->bps * 17 / 16;
            rc_cfg->bps_min      = p->bps * 1 / 16;
        }
    }

    rc_cfg->fps_in_flex      = 0;
    rc_cfg->fps_in_num       = p->fps;
    rc_cfg->fps_in_denorm    = 1;
    rc_cfg->fps_out_flex     = 0;
    rc_cfg->fps_out_num      = p->fps;
    rc_cfg->fps_out_denorm   = 1;

    rc_cfg->gop              = p->gop;
    rc_cfg->skip_cnt         = 0;

#ifdef DBG_LOG
    fy_log("mpi_enc_test bps %d fps %d gop %d\n",
            rc_cfg->bps_target, rc_cfg->fps_out_num, rc_cfg->gop);
#endif
    ret = mpi->control(ctx, MPP_ENC_SET_RC_CFG, rc_cfg);
    if (ret) {
        fy_err("mpi control enc set rc cfg failed ret %d\n", ret);
        goto RET;
    }

    codec_cfg->coding = p->type;
    switch (codec_cfg->coding) {
    case MPP_VIDEO_CodingAVC : {
        codec_cfg->h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
                                 MPP_ENC_H264_CFG_CHANGE_ENTROPY |
                                 MPP_ENC_H264_CFG_CHANGE_TRANS_8x8;
        /*
         * H.264 profile_idc parameter
         * 66  - Baseline profile
         * 77  - Main profile
         * 100 - High profile
         */
        codec_cfg->h264.profile  = 100;
        /*
         * H.264 level_idc parameter
         * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
         * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
         * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
         * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
         * 50 / 51 / 52         - 4K@30fps
         */
        codec_cfg->h264.level    = 40;
        codec_cfg->h264.entropy_coding_mode  = 1;
        codec_cfg->h264.cabac_init_idc  = 0;
        codec_cfg->h264.transform8x8_mode = 1;
    } break;
    case MPP_VIDEO_CodingMJPEG : {
        codec_cfg->jpeg.change  = MPP_ENC_JPEG_CFG_CHANGE_QP;
        codec_cfg->jpeg.quant   = 10;
    } break;
    case MPP_VIDEO_CodingVP8 : {
    } break;
    case MPP_VIDEO_CodingHEVC : {
        codec_cfg->h265.change = MPP_ENC_H265_CFG_INTRA_QP_CHANGE;
        codec_cfg->h265.intra_qp = 26;
    } break;
    default : {
        fy_err("support encoder coding type %d\n", codec_cfg->coding);
    } break;
    }
    ret = mpi->control(ctx, MPP_ENC_SET_CODEC_CFG, codec_cfg);
    if (ret) {
        fy_err("mpi control enc set codec cfg failed ret %d\n", ret);
        goto RET;
    }

    /* optional */
    p->sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->sei_mode);
    if (ret) {
        fy_err("mpi control enc set sei cfg failed ret %d\n", ret);
        goto RET;
    }

RET:
    return ret;
}

MPP_RET mpp_run(MpiEncData *p, FILE *in_fp)
{
    MPP_RET ret = MPP_OK;
    MppApi *mpi;
    MppCtx ctx;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;

//    uint32_t read_pos = 0;
    while (!p->pkt_eos)
    {
        MppFrame frame = NULL;
        MppPacket packet = NULL;
        void *buf = mpp_buffer_get_ptr(p->frm_buf);

        if (in_fp)
        {
            ret = read_yuv_image(buf, in_fp, p->width, p->height, p->hor_stride, p->ver_stride, p->fmt);
            if (ret == MPP_NOK  || feof(in_fp))
            {
                fy_log("found last frame. feof %d\n", feof(in_fp));
                p->frm_eos = 1;
            }
            else if (ret == MPP_ERR_VALUE)
                goto RET;
        }
        else
        {
            ret = fill_yuv_image(buf, p->width, p->height, p->hor_stride,
                                 p->ver_stride, p->fmt, p->frame_count);
            if (ret)
                goto RET;
        }

        ret = mpp_frame_init(&frame);
        if (ret)
        {
            fy_err("mpp_frame_init failed\n");
            goto RET;
        }
        fy_log("mpp_frame_init\n");

        mpp_frame_set_width(frame, p->width);
        mpp_frame_set_height(frame, p->height);
        mpp_frame_set_hor_stride(frame, p->hor_stride);
        mpp_frame_set_ver_stride(frame, p->ver_stride);
        mpp_frame_set_fmt(frame, p->fmt);
        mpp_frame_set_buffer(frame, p->frm_buf);
        mpp_frame_set_eos(frame, p->frm_eos);

        ret = mpi->encode_put_frame(ctx, frame);
        if (ret)
        {
            fy_err("mpp encode put frame failed\n");
            goto RET;
        }
        fy_log("encode_put_frame\n");

        ret = mpi->encode_get_packet(ctx, &packet);
        if (ret)
        {
            fy_err("mpp encode get packet failed\n");
            goto RET;
        }
        fy_log("encode_get_packet\n");

        if (packet)
        {
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);

            p->pkt_eos = mpp_packet_get_eos(packet);

            if (p->output_fp)
            {
#ifdef NEED_WRITE_LEN
                uint32_t ilen = htonl(len);
                fwrite(&ilen, 1, sizeof(ilen), p->output_fp);
#endif
                fwrite(ptr, 1, len, p->output_fp);
            }
            mpp_packet_deinit(&packet);

#ifdef DBG_LOG
            fy_log("encoded frame %d size %d\n", p->frame_count, len);
#endif
            p->frame_count++;

            if (p->pkt_eos)
            {
                fy_log("found last packet\n");
                fy_assert(p->frm_eos);
            }
        }

#ifdef DBG_LOG
        fy_log("encode frame: %d\n", p->frame_count);
        fy_log("max frame: %d\n", p->num_frames);
#endif
        if (p->num_frames && p->frame_count >= p->num_frames)
        {
            fy_log("encode max %d frames", p->frame_count);
            break;
        }
        if (p->frm_eos && p->pkt_eos)
            break;
    }
RET:
    p->frm_eos = 0;
    p->pkt_eos = 0;
    return ret;
}

void mpp_multi_enc_run(mpp_multi_cmd_t *cmd, int pindex)
{
    FILE *in_fp = NULL;
    MpiEncData *p = NULL;

    do
    {
        MPP_RET ret;
        p = malloc(sizeof(MpiEncData));
        if (NULL == p)
        {
            fy_err("mpp_multi_enc_run alloc memory fail\n");
            break;
        }

        in_fp = fopen(cmd->file_input, "r");
        if (NULL == in_fp)
        {
            fy_err("mpp_multi_enc_run open input file fail\n");
            break;
        }

        char output_filename[256];
        sprintf(output_filename, "%s_0%d", cmd->file_output, pindex);
        p->output_fp = fopen(output_filename, "a+");
        if (NULL == p->output_fp)
        {
            fy_err("mpp_multi_enc_run open output file fail\n");
            break;
        }

        ret = ctx_init(cmd, p);
        if (ret)
        {
            fy_err("test data init failed ret %d\n", ret);
            break;
        }

        ret = mpp_buffer_get(NULL, &(p->frm_buf), p->frame_size);
        if (ret)
        {
            fy_err("failed to get buffer for input frame ret %d, frame_size: %d\n", ret, p->frame_size);
            break;
        }

        ret = mpp_create(&p->ctx, &p->mpi);
        if (ret)
        {
            fy_err("mpp_create failed ret %d\n", ret);
            break;
        }

        ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
        if (ret)
        {
            fy_err("mpp_init failed ret %d\n", ret);
            break;
        }

        ret = mpp_setup(p);
        if (ret)
        {
            fy_err("test mpp setup failed ret %d\n", ret);
            break;
        }

        if (p->type == MPP_VIDEO_CodingAVC)
        {
            MppPacket packet = NULL;
            ret = p->mpi->control(p->ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
            if (ret)
            {
                fy_err("mpi control enc get extra info failed\n");
                break;
            }

            if (packet)
            {
                void *ptr   = mpp_packet_get_pos(packet);
                size_t len  = mpp_packet_get_length(packet);

#ifdef NEED_WRITE_LEN
                uint32_t ilen = htonl(len);
                fwrite(&ilen, 1, sizeof(ilen), p->output_fp);
#endif
                fwrite(ptr, 1, len, p->output_fp);
                fy_log("memcpy OK\n");

                packet = NULL;
            }
        }

        fy_log("mpi_set success\n");

        while (!feof(in_fp))
        {
            mpp_run(p, in_fp);
        }

        return;
    } while (0);

    if (p)
    {
        if (p->ctx)
        {
            mpp_destroy(p->ctx);
            p->ctx = NULL;
        }

        if (p->frm_buf)
        {
            mpp_buffer_put(p->frm_buf);
            p->frm_buf = NULL;
        }

        ctx_deinit(&p);
        free(p);
    }

    if (NULL != in_fp)
        fclose(in_fp);

    fy_err("mpi_set failed\n");
}


void mpp_multi_enc(mpp_multi_cmd_t *cmd)
{
#ifdef DBG_LOG
    fy_log("mpp_multi_enc start\n");
#endif

    pid_t enc_pid;
    int pindex = 0;

    while (pindex < cmd->payload_num)
    {
        enc_pid = fork();

        if (enc_pid < 0)
            fy_err("fork fail\n");
        else if (enc_pid == 0)
        {
            fy_log("enter child process: %d\n", pindex);
            mpp_multi_enc_run(cmd, pindex);
        }
        pindex++;
    }
}




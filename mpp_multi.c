

#include <stdio.h>
#include "mpp_multi.h"
#include "mpp_multi_dec.h"
#include "mpp_multi_enc.h"


static OptionInfo mpp_multi_cmd[] = {
    {"i",               "input_file",           "input bitstream file"},
    {"o",               "output_file",          "output bitstream file, "},
    {"w",               "width",                "the width of input bitstream"},
    {"h",               "height",               "the height of input bitstream"},
    {"t",               "codec_type",           "the codec type, dec: deoder, enc: encoder, default: decoder" },
    {"c",               "coding_type",          "input file coding type"},
    {"f",               "format",               "output file format type"},
    {"n",               "frame_number",         "max output frame number"},
    {"p",               "payload",              "encode/decode payload number"},
};


void _show_options(int count, OptionInfo *options)
{
    int i;
    for (i = 0; i < count; i++) {
        printf("-%s  %-16s\t%s\n",
                options[i].name, options[i].argname, options[i].help);
    }
}

static void mpp_multi_show_usage(char *app_name)
{
    fy_log("usage: %s [options]\n", app_name);
    show_options(mpp_multi_cmd);
    mpp_show_support_format();
}

static RK_S32 mpp_multi_parse_options(int argc, char **argv, mpp_multi_cmd_t *cmd)
{
    const char *opt;
    const char *next;
    RK_S32 optindex = 1;
    RK_S32 handleoptions = 1;
    RK_S32 err = MPP_NOK;

    if ((argc < 2) || (cmd == NULL)) {
        err = 1;
        return err;
    }

    /* parse options */
    while (optindex < argc) {
        opt  = (const char*)argv[optindex++];
        next = (const char*)argv[optindex];

        if (handleoptions && opt[0] == '-' && opt[1] != '\0') {
            if (opt[1] == '-') {
                if (opt[2] != '\0') {
                    opt++;
                } else {
                    handleoptions = 0;
                    continue;
                }
            }

            opt++;

            switch (*opt) {
            case 'i':
                if (next) {
                    strncpy(cmd->file_input, next, MAX_FILE_NAME_LENGTH);
                    cmd->file_input[strlen(next)] = '\0';
                } else {
                    fy_err("input file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'o':
                if (next) {
                    strncpy(cmd->file_output, next, MAX_FILE_NAME_LENGTH);
                    cmd->file_output[strlen(next)] = '\0';
                } else {
                    fy_log("output file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'w':
                if (next) {
                    cmd->width = atoi(next);
                } else {
                    fy_err("invalid input width\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'h':
                if ((*(opt + 1) != '\0') && !strncmp(opt, "help", 4)) {
                    mpp_multi_show_usage(argv[0]);
                    err = 1;
                    goto PARSE_OPINIONS_OUT;
                } else if (next) {
                    cmd->height = atoi(next);
                } else {
                    fy_log("input height is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 't':
                if (next) {
                    cmd->codec_type = (MppCodingType)atoi(next);
                }

                if (!next || ((cmd->codec_type != 0) && (cmd->codec_type != 1))) {
                    fy_err("invalid codec type, it must be 0(decode) or 1(encode)\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'c':
                if (next) {
                    cmd->coding_type = (MppCodingType)atoi(next);
//                    err = 0;
                    err = mpp_check_support_format(MPP_CTX_DEC, cmd->coding_type);
                }

                if (!next || err) {
                    fy_err("invalid coding type\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'f':
                if (next) {
                    cmd->format = (MppFrameFormat)atoi(next);
                }

                if (!next || err) {
                    fy_err("invalid input coding type\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'n':
                if (next) {
                    cmd->frame_num = atoi(next);
                    if (cmd->frame_num < 0)
                        fy_log("infinite loop decoding mode\n");
                } else {
                    fy_err("invalid frame number\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'p':
                if (next) {
                    cmd->payload_num = atoi(next);
                    if ((cmd->payload_num < 0) || (cmd->payload_num > 4))
                        fy_log("too much payload number\n");
                } else {
                    fy_err("invalid payload number\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            default:
                goto PARSE_OPINIONS_OUT;
                break;
            }

            optindex++;
        }
    }

    err = 0;

PARSE_OPINIONS_OUT:
    return err;
}

int main(int argc, char *argv[])
{
    fy_log("/*******  hh mpp enc/dec api multi test demo in *******/\n");
    if (argc == 1)
    {
        mpp_multi_show_usage(argv[0]);
        fy_log("hh mpp enc/dec api multi test demo complete directly\n");
        return 0;
    }

    mpp_multi_cmd_t  cmd_ctx;
    mpp_multi_cmd_t *cmd = &cmd_ctx;

    memset((void*)cmd, 0, sizeof(*cmd));
    cmd->format = MPP_FMT_BUTT;
//    cmd->pkt_size = MPI_DEC_STREAM_SIZE;

    // parse the cmd option
    mpp_multi_parse_options(argc, argv, cmd);

    if (cmd->codec_type == 0)
        mpp_multi_dec(cmd);
    else if (cmd->codec_type == 1)
        mpp_multi_enc(cmd);

    return 0;
}



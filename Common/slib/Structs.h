
//amp structs
#pragma pack (8)
#include <windows.h>
#include <mmsystem.h>

#ifndef __AMP_STRUCTS
#define __AMP_STRUCTS

#define         SYNCWORD        0xfff
#define         TRUE            1
#define         FALSE           0

#define         MAJOR           1
#define         MINOR           0
#define         PATCH           0

#define         DSCP_LAYER3     0x00000001
#define         DSCP_LAYER2     0x00000010
#define         DSCP_LAYER1     0x00000100

#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))

#define DB(type,cmd)
#define GETHDR_ERR 1
#define GETHDR_NS  2

#define BUFFER_SIZE     4096
#define BUFFER_AUX      2048
#define i_sq2   0.707106781188
#define IS_ILLEGAL 0xfeed
#define N_CUE 16
#define NC_O  4

typedef void (*_s_stdout)(char *);

struct AUDIO_HEADER {
        int ID;
        int layer;
        int protection_bit;
        int bitrate_index;
        int sampling_frequency;
        int padding_bit;
        int private_bit;
        int mode;
        int mode_extension;
        int copyright;
        int original;
        int emphasis;
};

struct SIDE_INFO {
        int main_data_begin;
        int scfsi[2][4];
        int part2_3_length[2][2];
        int big_values[2][2];
        int global_gain[2][2];
        int scalefac_compress[2][2];
        int window_switching_flag[2][2];
        int block_type[2][2];
        int mixed_block_flag[2][2];
        int table_select[2][2][3];
        int subblock_gain[2][2][3];
        int region0_count[2][2];
        int region1_count[2][2];
        int preflag[2][2];
        int scalefac_scale[2][2];
        int count1table_select[2][2];
};



#define S_DUPLICATED 0x8000
#define S_PLAYED     0x0001
#define S_LOOPED     0x0010


#endif
//variables and arrays
#include "proto.h"
#pragma warning (disable : 4305)

char* in_file;

int scalefac_l[2][2][22];
int scalefac_s[2][2][13][3];

int is[2][578];
float xr[2][32][18];

int *t_l,*t_s;
int nch;
int t_sampling_frequency[2][3] = {
{ 22050 , 24000 , 16000},
{ 44100 , 48000 , 32000}
};

short pcm_sample[64];

short t_bitrate[2][3][15] = {{
{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256},
{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160},
{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160}
},{
{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
}};

int t_b8_l[2][3][22]={{ /* table B.8b ISO/IEC 11172-3 */
{5,11,17,23,29,35,43,53,65,79,95,115,139,167,199,237,283,335,395,463,521,575},
{5,11,17,23,29,35,43,53,65,79,95,113,135,161,193,231,277,331,393,463,539,575},
{5,11,17,23,29,35,43,53,65,79,95,115,139,167,199,237,283,335,395,463,521,575}
},{
{3,7,11,15,19,23,29,35,43,51,61,73,89,109,133,161,195,237,287,341,417,575},
{3,7,11,15,19,23,29,35,41,49,59,71,87,105,127,155,189,229,275,329,383,575},
{3,7,11,15,19,23,29,35,43,53,65,81,101,125,155,193,239,295,363,447,549,575}
}};
int t_b8_s[2][3][13]={{ /* table B.8b ISO/IEC 11172-3 */
{3,7,11,17,23,31,41,55,73,99,131,173,191},
{3,7,11,17,25,35,47,61,79,103,135,179,191},
{3,7,11,17,25,35,47,61,79,103,133,173,191}
},{
{3,7,11,15,21,29,39,51,65,83,105,135,191},
{3,7,11,15,21,27,37,49,63,79,99,125,191},
{3,7,11,15,21,29,41,57,77,103,137,179,191}
}};
//transform

short stereo_samples[18][32][2];
short mono_samples[18][32];
float s[2][32][18];
float res[32][18];
float win[4][36];

const float t_sin[4][36]={{
   -0.032160,    0.103553,   -0.182543,    0.266729,   -0.353554,    0.440377,
   -0.524563,    0.603553,   -0.674947,    0.736575,   -0.786566,    0.823400,
   -0.845957,    0.853554,   -0.845957,    0.823399,   -0.786566,    0.736575,
   -0.674947,    0.603553,   -0.524564,    0.440378,   -0.353553,    0.266729,
   -0.182544,    0.103553,   -0.032160,   -0.029469,    0.079459,   -0.116293,
    0.138851,   -0.146446,    0.138851,   -0.116293,    0.079459,   -0.029469
},{
   -0.032160,    0.103553,   -0.182543,    0.266729,   -0.353554,    0.440377,
   -0.524563,    0.603553,   -0.674947,    0.736575,   -0.786566,    0.823400,
   -0.845957,    0.853554,   -0.845957,    0.823399,   -0.786566,    0.736575,
   -0.675590,    0.608761,   -0.537300,    0.461749,   -0.382683,    0.300706,
   -0.214588,    0.120590,   -0.034606,   -0.026554,    0.049950,   -0.028251,
    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,    0.000000
},{
   -0.103553,    0.353554,   -0.603553,    0.786566,   -0.853554,    0.786566,
   -0.603553,    0.353553,   -0.103553,   -0.079459,    0.146446,   -0.079459,
    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,
    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,
    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,
    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,    0.000000
},{
    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,    0.000000,
   -0.127432,    0.379410,   -0.608182,    0.792598,   -0.915976,    0.967944,
   -0.953717,    0.923880,   -0.887011,    0.843391,   -0.793353,    0.737277,
   -0.674947,    0.603553,   -0.524564,    0.440378,   -0.353553,    0.266729,
   -0.182544,    0.103553,   -0.032160,   -0.029469,    0.079459,   -0.116293,
    0.138851,   -0.146446,    0.138851,   -0.116293,    0.079459,   -0.029469
}};

const float t_2cos[4][18]={
{ -0.174311,  -0.517638,  -0.845237,  -1.147153,  -1.414214,  -1.638304, -1.812616,  -1.931852,  -1.992389,
   0.174311,   0.517638,   0.845237,   1.147153,   1.414214,   1.638304, 1.812616,   1.931852,   1.992389},
{ -0.174311,  -0.517638,  -0.845237,  -1.147153,  -1.414214,  -1.638304, -1.812616,  -1.931852,  -1.992389,
   0.174311,   0.517638,   0.845237,   1.147153,   1.414214,   1.638304, 1.812616,   1.931852,   1.992389},
{ -0.517638, -1.41421, -1.93185, 0.517638, 1.41421, 1.93185,0,0,0,0,0,0,0,0,0,0,0,0},
{ -0.174311,  -0.517638,  -0.845237,  -1.147153,  -1.414214,  -1.638304, -1.812616,  -1.931852,  -1.992389,
   0.174311,   0.517638,   0.845237,   1.147153,   1.414214,   1.638304, 1.812616,   1.931852,   1.992389}
};

float b1 = 1.997590912; float b2 = 1.990369453; float b3 = 1.978353019;
float b4 = 1.961570560; float b5 = 1.940062506; float b6 = 1.913880671;
float b7 = 1.883088130; float b8 = 1.847759065; float b9 = 1.807978586;
float b10= 1.763842529; float b11= 1.715457220; float b12= 1.662939225;
float b13= 1.606415063; float b14= 1.546020907; float b15= 1.481902251;
float b16= 1.414213562; float b17= 1.343117910; float b18= 1.268786568;
float b19= 1.191398609; float b20= 1.111140466; float b21= 1.028205488;
float b22= 0.942793474; float b23= 0.855110187; float b24= 0.765366865;
float b25= 0.673779707; float b26= 0.580569355; float b27= 0.485960360;
float b28= 0.390180644; float b29= 0.293460949; float b30= 0.196034281;
float b31= 0.098135349;

float t_dewindow[16][32] =       {
 0.000000000 ,-0.000442505 , 0.003250122 ,-0.007003784 , 0.031082153 ,-0.078628540 , 0.100311279 ,-0.572036743 ,
 1.144989014 , 0.572036743 , 0.100311279 , 0.078628540 , 0.031082153 , 0.007003784 , 0.003250122 , 0.000442505 ,

-0.000015259 ,-0.000473022 , 0.003326416 ,-0.007919312 , 0.030517578 ,-0.084182739 , 0.090927124 ,-0.600219727 ,
 1.144287109 , 0.543823242 , 0.108856201 , 0.073059082 , 0.031478882 , 0.006118774 , 0.003173828 , 0.000396729 ,

-0.000015259 ,-0.000534058 , 0.003387451 ,-0.008865356 , 0.029785156 ,-0.089706421 , 0.080688477 ,-0.628295898 ,
 1.142211914 , 0.515609741 , 0.116577148 , 0.067520142 , 0.031738281 , 0.005294800 , 0.003082275 , 0.000366211 ,

-0.000015259 ,-0.000579834 , 0.003433228 ,-0.009841919 , 0.028884888 ,-0.095169067 , 0.069595337 ,-0.656219482 ,
 1.138763428 , 0.487472534 , 0.123474121 , 0.061996460 , 0.031845093 , 0.004486084 , 0.002990723 , 0.000320435 ,

-0.000015259 ,-0.000625610 , 0.003463745 ,-0.010848999 , 0.027801514 ,-0.100540161 , 0.057617187 ,-0.683914185 ,
 1.133926392 , 0.459472656 , 0.129577637 , 0.056533813 , 0.031814575 , 0.003723145 , 0.002899170 , 0.000289917 ,

-0.000015259 ,-0.000686646 , 0.003479004 ,-0.011886597 , 0.026535034 ,-0.105819702 , 0.044784546 ,-0.711318970 ,
 1.127746582 , 0.431655884 , 0.134887695 , 0.051132202 , 0.031661987 , 0.003005981 , 0.002792358 , 0.000259399 ,

-0.000015259 ,-0.000747681 , 0.003479004 ,-0.012939453 , 0.025085449 ,-0.110946655 , 0.031082153 ,-0.738372803 ,
 1.120223999 , 0.404083252 , 0.139450073 , 0.045837402 , 0.031387329 , 0.002334595 , 0.002685547 , 0.000244141 ,

-0.000030518 ,-0.000808716 , 0.003463745 ,-0.014022827 , 0.023422241 ,-0.115921021 , 0.016510010 ,-0.765029907 ,
 1.111373901 , 0.376800537 , 0.143264771 , 0.040634155 , 0.031005859 , 0.001693726 , 0.002578735 , 0.000213623 ,

-0.000030518 ,-0.000885010 , 0.003417969 ,-0.015121460 , 0.021575928 ,-0.120697021 , 0.001068115 ,-0.791213989 ,
 1.101211548 , 0.349868774 , 0.146362305 , 0.035552979 , 0.030532837 , 0.001098633 , 0.002456665 , 0.000198364 ,

-0.000030518 ,-0.000961304 , 0.003372192 ,-0.016235352 , 0.019531250 ,-0.125259399 ,-0.015228271 ,-0.816864014 ,
 1.089782715 , 0.323318481 , 0.148773193 , 0.030609131 , 0.029937744 , 0.000549316 , 0.002349854 , 0.000167847 ,

-0.000030518 ,-0.001037598 , 0.003280640 ,-0.017349243 , 0.017257690 ,-0.129562378 ,-0.032379150 ,-0.841949463 ,
 1.077117920 , 0.297210693 , 0.150497437 , 0.025817871 , 0.029281616 , 0.000030518 , 0.002243042 , 0.000152588 ,

-0.000045776 ,-0.001113892 , 0.003173828 ,-0.018463135 , 0.014801025 ,-0.133590698 ,-0.050354004 ,-0.866363525 ,
 1.063217163 , 0.271591187 , 0.151596069 , 0.021179199 , 0.028533936 ,-0.000442505 , 0.002120972 , 0.000137329 ,

-0.000045776 ,-0.001205444 , 0.003051758 ,-0.019577026 , 0.012115479 ,-0.137298584 ,-0.069168091 ,-0.890090942 ,
 1.048156738 , 0.246505737 , 0.152069092 , 0.016708374 , 0.027725220 ,-0.000869751 , 0.002014160 , 0.000122070 ,

-0.000061035 ,-0.001296997 , 0.002883911 ,-0.020690918 , 0.009231567 ,-0.140670776 ,-0.088775635 ,-0.913055420 ,
 1.031936646 , 0.221984863 , 0.151962280 , 0.012420654 , 0.026840210 ,-0.001266479 , 0.001907349 , 0.000106812 ,

-0.000061035 ,-0.001388550 , 0.002700806 ,-0.021789551 , 0.006134033 ,-0.143676758 ,-0.109161377 ,-0.935195923 ,
 1.014617920 , 0.198059082 , 0.151306152 , 0.008316040 , 0.025909424 ,-0.001617432 , 0.001785278 , 0.000106812 ,

-0.000076294 ,-0.001480103 , 0.002487183 ,-0.022857666 , 0.002822876 ,-0.146255493 ,-0.130310059 ,-0.956481934 ,
 0.996246338 , 0.174789429 , 0.150115967 , 0.004394531 , 0.024932861 ,-0.001937866 , 0.001693726 , 0.000091553 ,

-0.000076294 ,-0.001586914 , 0.002227783 ,-0.023910522 ,-0.000686646 ,-0.148422241 ,-0.152206421 ,-0.976852417 ,
 0.976852417 , 0.152206421 , 0.148422241 , 0.000686646 , 0.023910522 ,-0.002227783 , 0.001586914 , 0.000076294 ,

-0.000091553 ,-0.001693726 , 0.001937866 ,-0.024932861 ,-0.004394531 ,-0.150115967 ,-0.174789429 ,-0.996246338 ,
 0.956481934 , 0.130310059 , 0.146255493 ,-0.002822876 , 0.022857666 ,-0.002487183 , 0.001480103 , 0.000076294 ,

-0.000106812 ,-0.001785278 , 0.001617432 ,-0.025909424 ,-0.008316040 ,-0.151306152 ,-0.198059082 ,-1.014617920 ,
 0.935195923 , 0.109161377 , 0.143676758 ,-0.006134033 , 0.021789551 ,-0.002700806 , 0.001388550 , 0.000061035 ,

-0.000106812 ,-0.001907349 , 0.001266479 ,-0.026840210 ,-0.012420654 ,-0.151962280 ,-0.221984863 ,-1.031936646 ,
 0.913055420 , 0.088775635 , 0.140670776 ,-0.009231567 , 0.020690918 ,-0.002883911 , 0.001296997 , 0.000061035 ,

-0.000122070 ,-0.002014160 , 0.000869751 ,-0.027725220 ,-0.016708374 ,-0.152069092 ,-0.246505737 ,-1.048156738 ,
 0.890090942 , 0.069168091 , 0.137298584 ,-0.012115479 , 0.019577026 ,-0.003051758 , 0.001205444 , 0.000045776 ,

-0.000137329 ,-0.002120972 , 0.000442505 ,-0.028533936 ,-0.021179199 ,-0.151596069 ,-0.271591187 ,-1.063217163 ,
 0.866363525 , 0.050354004 , 0.133590698 ,-0.014801025 , 0.018463135 ,-0.003173828 , 0.001113892 , 0.000045776 ,

-0.000152588 ,-0.002243042 ,-0.000030518 ,-0.029281616 ,-0.025817871 ,-0.150497437 ,-0.297210693 ,-1.077117920 ,
 0.841949463 , 0.032379150 , 0.129562378 ,-0.017257690 , 0.017349243 ,-0.003280640 , 0.001037598 , 0.000030518 ,

-0.000167847 ,-0.002349854 ,-0.000549316 ,-0.029937744 ,-0.030609131 ,-0.148773193 ,-0.323318481 ,-1.089782715 ,
 0.816864014 , 0.015228271 , 0.125259399 ,-0.019531250 , 0.016235352 ,-0.003372192 , 0.000961304 , 0.000030518 ,

-0.000198364 ,-0.002456665 ,-0.001098633 ,-0.030532837 ,-0.035552979 ,-0.146362305 ,-0.349868774 ,-1.101211548 ,
 0.791213989 ,-0.001068115 , 0.120697021 ,-0.021575928 , 0.015121460 ,-0.003417969 , 0.000885010 , 0.000030518 ,

-0.000213623 ,-0.002578735 ,-0.001693726 ,-0.031005859 ,-0.040634155 ,-0.143264771 ,-0.376800537 ,-1.111373901 ,
 0.765029907 ,-0.016510010 , 0.115921021 ,-0.023422241 , 0.014022827 ,-0.003463745 , 0.000808716 , 0.000030518 ,

-0.000244141 ,-0.002685547 ,-0.002334595 ,-0.031387329 ,-0.045837402 ,-0.139450073 ,-0.404083252 ,-1.120223999 ,
 0.738372803 ,-0.031082153 , 0.110946655 ,-0.025085449 , 0.012939453 ,-0.003479004 , 0.000747681 , 0.000015259 ,

-0.000259399 ,-0.002792358 ,-0.003005981 ,-0.031661987 ,-0.051132202 ,-0.134887695 ,-0.431655884 ,-1.127746582 ,
 0.711318970 ,-0.044784546 , 0.105819702 ,-0.026535034 , 0.011886597 ,-0.003479004 , 0.000686646 , 0.000015259 ,

-0.000289917 ,-0.002899170 ,-0.003723145 ,-0.031814575 ,-0.056533813 ,-0.129577637 ,-0.459472656 ,-1.133926392 ,
 0.683914185 ,-0.057617187 , 0.100540161 ,-0.027801514 , 0.010848999 ,-0.003463745 , 0.000625610 , 0.000015259 ,

-0.000320435 ,-0.002990723 ,-0.004486084 ,-0.031845093 ,-0.061996460 ,-0.123474121 ,-0.487472534 ,-1.138763428 ,
 0.656219482 ,-0.069595337 , 0.095169067 ,-0.028884888 , 0.009841919 ,-0.003433228 , 0.000579834 , 0.000015259 ,

-0.000366211 ,-0.003082275 ,-0.005294800 ,-0.031738281 ,-0.067520142 ,-0.116577148 ,-0.515609741 ,-1.142211914 ,
 0.628295898 ,-0.080688477 , 0.089706421 ,-0.029785156 , 0.008865356 ,-0.003387451 , 0.000534058 , 0.000015259 ,

-0.000396729 ,-0.003173828 ,-0.006118774 ,-0.031478882 ,-0.073059082 ,-0.108856201 ,-0.543823242 ,-1.144287109 ,
 0.600219727 ,-0.090927124 , 0.084182739 ,-0.030517578 , 0.007919312 ,-0.003326416 , 0.000473022 , 0.000015259
};

char t_slen1[16]={0,0,0,0,3,1,1,1,2,2,2,3,3,3,4,4};
char t_slen2[16]={0,1,2,3,0,1,2,3,1,2,3,1,2,3,2,3};
int t_linbits[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,6,8,10,13,4,5,6,7,8,9,11,13};

int non_zero[2];
int is_max[21];
int intensity_scale;

__inline int _qsign(int x,int *q);
int decode_scalefactors(struct SIDE_INFO *info,struct AUDIO_HEADER *header,int gr,int ch);
int decode_huffman_data(struct SIDE_INFO *info,int gr,int ch,int ssize);

char spooky_table[2][3][3][4]={
{
{ {6,5,5,5},   {9,9,9,9},   {6,9,9,9} },
{ {6,5,7,3},   {9,9,12,6},  {6,9,12,6}},
{ {11,10,0,0}, {18,18,0,0}, {15,18,0,0}}
},
{
{ {7,7,7,0},   {12,12,12,0}, {6,15,12,0}},
{ {6,6,6,3},   {12,9,9,6},   {6,12,9,6}},
{ {8,8,5,0},   {15,12,9,0},  {6,18,9,0}}
}};

unsigned char buffer[BUFFER_SIZE+BUFFER_AUX];
int append;
int data;
unsigned char _buffer[32];
int _bptr;
////
#include "vary_ext.inc"
#include "vary_huf.inc"
//#endif
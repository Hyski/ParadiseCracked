#ifndef __AMP_PROTO
#define __AMP_PROTO
#include "structs.h"
#define _DLL_BUILD_

extern  int DS_BUFFERLEN;
extern  int DS_PROCPOWER;

extern unsigned int viewbits(int n);
extern void sackbits(int n);

extern int  layer3_frame(struct AUDIO_HEADER *header,int cnt);
extern char *in_file;
extern int bytes_read;

extern int scalefac_l[2][2][22];
extern int scalefac_s[2][2][13][3];
extern int t_b8_l[2][3][22];
extern int t_b8_s[2][3][13];
extern short t_bitrate[2][3][15];

extern int is[2][578];
extern float xr[2][32][18];

extern int *t_l,*t_s;
extern int nch;
extern int t_sampling_frequency[2][3];

extern short pcm_sample[64];

extern void imdct_init();
extern void imdct(int win_type,int sb,int ch);
extern void poly(int ch,int i);
extern void premultiply();

extern short stereo_samples[18][32][2];
extern short mono_samples[18][32];
extern float res[32][18];
extern float s[2][32][18];

extern char t_slen1[16];
extern char t_slen2[16];
extern int t_linbits[32];

extern int non_zero[2];
extern int is_max[21];
extern int intensity_scale;

extern __inline int _qsign(int x,int *q);
extern int decode_scalefactors(struct SIDE_INFO *info,struct AUDIO_HEADER *header,int gr,int ch);
extern int decode_huffman_data(struct SIDE_INFO *info,int gr,int ch,int ssize);

extern char spooky_table[2][3][3][4];

extern unsigned char buffer[];
extern int append;
extern int data;

extern void getinfo(struct AUDIO_HEADER *header,struct SIDE_INFO *info);
extern int gethdr(struct AUDIO_HEADER *header);
extern void getcrc();
extern void fillbfr(int advance);
extern unsigned int getbits(int n);

extern unsigned char _buffer[32];
extern int _bptr;
extern __inline unsigned int _getbits(int n);
extern __inline void _fillbfr(int size);

extern void requantize_mono(int gr,int ch,struct SIDE_INFO *info,struct AUDIO_HEADER *header);
extern void requantize_ms(int gr,struct SIDE_INFO *info,struct AUDIO_HEADER *header);
extern void alias_reduction(int ch);

extern __inline float fras_l(int sfb,int global_gain,int scalefac_scale,int scalefac,int preflag);
extern __inline float fras_s(int global_gain,int subblock_gain,int scalefac_scale,int scalefac);
extern __inline float fras2(int is,float a);
extern int find_isbound(int isbound[3],int gr,struct SIDE_INFO *info,struct AUDIO_HEADER *header);
extern __inline void stereo_s(int l,float a[2],int pos,int ms_flag,int is_pos,struct AUDIO_HEADER *header);
extern __inline void stereo_l(int l,float a[2],int ms_flag,int is_pos,struct AUDIO_HEADER *header);

extern int no_of_imdcts[2];
extern int t_pretab[22];
extern float t_is[7];
extern float t_is2[2][32];
extern float Cs[8];
extern float Ca[8];
extern float tab[4];
extern float tabi[4];
extern short t_reorder[2][3][576];

extern int huffman_decode(int tbl,int *x,int *y);

extern unsigned int h0[1];
extern unsigned int h1[4];
extern unsigned int h2[9];
extern unsigned int h3[9];
extern unsigned int h5[16];
extern unsigned int h6[16];
extern unsigned int h7[36];
extern unsigned int h8[36];
extern unsigned int h9[36];
extern unsigned int h10[64];
extern unsigned int h11[64];
extern unsigned int h12[64];
extern unsigned int h13[256];
extern unsigned int h15[256];
extern unsigned int h16[256];
extern unsigned int h24[256];
extern unsigned int hA[16];
extern unsigned int hB[16];
extern unsigned char h_cue[34][N_CUE];
extern unsigned int *tables[34];
extern float t_43[8192];

extern float win[4][36];

extern const float t_sin[4][36];
extern const float t_2cos[4][18];
extern float b1;
extern float b2;
extern float b3;
extern float b4;
extern float b5;
extern float b6;
extern float b7;
extern float b8;
extern float b9;
extern float b10;
extern float b11;
extern float b12;
extern float b13;
extern float b14;
extern float b15;
extern float b16;
extern float b17;
extern float b18;
extern float b19;
extern float b20;
extern float b21;
extern float b22;
extern float b23;
extern float b24;
extern float b25;
extern float b26;
extern float b27;
extern float b28;
extern float b29;
extern float b30;
extern float b31;
extern float t_dewindow[16][32];
#endif
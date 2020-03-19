#pragma warning (disable : 4244)

#include <math.h>
#include <sys/types.h>

#include "proto.h"

#define PI12      0.261799387f
#define PI36      0.087266462f
#define COSPI3    0.500000000f
#define COSPI6    0.866025403f
#define DCTODD1   0.984807753f
#define DCTODD2  -0.342020143f
#define DCTODD3  -0.642787609f
#define DCTEVEN1  0.939692620f
#define DCTEVEN2 -0.173648177f
#define DCTEVEN3 -0.766044443f

void imdct_init()
{
  int i;

  for(i=0;i<36;i++) /* 0 */
    win[0][i] = (float) sin(PI36 *(i+0.5));
  for(i=0;i<18;i++) /* 1 */
    win[1][i] = (float) sin(PI36 *(i+0.5));
  for(i=18;i<24;i++)
    win[1][i] = 1.0f;
  for(i=24;i<30;i++)
    win[1][i] = (float) sin(PI12 *(i+0.5-18));
  for(i=30;i<36;i++)
    win[1][i] = 0.0f;
  for(i=0;i<6;i++) /* 3 */
    win[3][i] = 0.0f;
  for(i=6;i<12;i++)
    win[3][i] = (float) sin(PI12 * (i+ 0.5 - 6.0));
  for(i=12;i<18;i++)
    win[3][i] = 1.0f;
  for(i=18;i<36;i++)
    win[3][i] = (float) sin(PI36 * (i + 0.5));
}


void imdct(int win_type,int sb,int ch)
{
/*------------------------------------------------------------------*/
/*                                                                  */
/*    Function: Calculation of the inverse MDCT                     */
/*    In the case of short blocks the 3 output vectors are already  */
/*    overlapped and added in this modul.                           */
/*                                                                  */
/*    New layer3                                                    */
/*                                                                  */
/*------------------------------------------------------------------*/

       float    tmp[18], save, sum;
       float  pp1, pp2;
       float   *win_bt;
       int     i, p, ss;
                 float *in = xr[ch][sb];
                 float out[36];


   if(win_type == 2){
                for(p=0;p<36;p+=9) {
                        out[p]   = out[p+1] = out[p+2] = out[p+3] =
                        out[p+4] = out[p+5] = out[p+6] = out[p+7] =
                        out[p+8] = 0.0f;
                }

        for(ss=0;ss<18;ss+=6) {

        /*
         *  12 point IMDCT
         */

                /* Begin 12 point IDCT */

                /* Input aliasing for 12 pt IDCT */
                in[5+ss]+=in[4+ss];in[4+ss]+=in[3+ss];in[3+ss]+=in[2+ss];
                in[2+ss]+=in[1+ss];in[1+ss]+=in[0+ss];

                /* Input aliasing on odd indices (for 6 point IDCT) */
                in[5+ss] += in[3+ss];  in[3+ss]  += in[1+ss];

                /* 3 point IDCT on even indices */

                pp2 = in[4+ss] * 0.500000000f;
                pp1 = in[2+ss] * 0.866025403f;
                sum = in[0+ss] + pp2;
                tmp[1]= in[0+ss] - in[4+ss];
                tmp[0]= sum + pp1;
                tmp[2]= sum - pp1;

                /* End 3 point IDCT on even indices */

                /* 3 point IDCT on odd indices (for 6 point IDCT) */

                pp2 = in[5+ss] * 0.500000000f;
                pp1 = in[3+ss] * 0.866025403f;
                sum = in[1+ss] + pp2;
                tmp[4] = in[1+ss] - in[5+ss];
                tmp[5] = sum + pp1;
                tmp[3] = sum - pp1;

                /* End 3 point IDCT on odd indices */

                /* Twiddle factors on odd indices (for 6 point IDCT) */

                tmp[3] *= 1.931851653f;
                tmp[4] *= 0.707106781f;
                tmp[5] *= 0.517638090f;

                /* Output butterflies on 2 3 point IDCT's (for 6 point IDCT) */

                save = tmp[0];
                tmp[0] += tmp[5];
                tmp[5] = save - tmp[5];
                save = tmp[1];
                tmp[1] += tmp[4];
                tmp[4] = save - tmp[4];
                save = tmp[2];
                tmp[2] += tmp[3];
                tmp[3] = save - tmp[3];

                /* End 6 point IDCT */

                /* Twiddle factors on indices (for 12 point IDCT) */

                tmp[0]  *=  0.504314480f;
                tmp[1]  *=  0.541196100f;
                tmp[2]  *=  0.630236207f;
                tmp[3]  *=  0.821339815f;
                tmp[4]  *=  1.306562965f;
                tmp[5]  *=  3.830648788f;

                /* End 12 point IDCT */

                /* Shift to 12 point modified IDCT, multiply by window type 2 */
                tmp[8]  = -tmp[0] * 0.793353340f;
                tmp[9]  = -tmp[0] * 0.608761429f;
                tmp[7]  = -tmp[1] * 0.923879532f;
                tmp[10] = -tmp[1] * 0.382683432f;
                tmp[6]  = -tmp[2] * 0.991444861f;
                tmp[11] = -tmp[2] * 0.130526192f;

                tmp[0]  =  tmp[3];
                tmp[1]  =  tmp[4] * 0.382683432f;
                tmp[2]  =  tmp[5] * 0.608761429f;

                tmp[3]  = -tmp[5] * 0.793353340f;
                tmp[4]  = -tmp[4] * 0.923879532f;
                tmp[5]  = -tmp[0] * 0.991444861f;

                tmp[0] *= 0.130526192f;

                out[ss + 6]  += tmp[0];
                out[ss + 7]  += tmp[1];
                out[ss + 8]  += tmp[2];
                out[ss + 9]  += tmp[3];
                out[ss + 10] += tmp[4];
                out[ss + 11] += tmp[5];
                out[ss + 12] += tmp[6];
                out[ss + 13] += tmp[7];
                out[ss + 14] += tmp[8];
                out[ss + 15] += tmp[9];
                out[ss + 16] += tmp[10];
                out[ss + 17] += tmp[11];

        }
        for (i=0;i<18;i++) res[sb][i]=out[i] + s[ch][sb][i];
        for (;i<36;i++) s[ch][sb][i-18]=out[i];

    } else {

        /* 36 point IDCT */

        /* input aliasing for 36 point IDCT */
        in[17]+=in[16]; in[16]+=in[15]; in[15]+=in[14]; in[14]+=in[13];
        in[13]+=in[12]; in[12]+=in[11]; in[11]+=in[10]; in[10]+=in[9];
        in[9] +=in[8];  in[8] +=in[7];  in[7] +=in[6];  in[6] +=in[5];
        in[5] +=in[4];  in[4] +=in[3];  in[3] +=in[2];  in[2] +=in[1];
        in[1] +=in[0];

        /* 18 point IDCT for odd indices */

        /* input aliasing for 18 point IDCT */
        in[17]+=in[15]; in[15]+=in[13]; in[13]+=in[11]; in[11]+=in[9];
        in[9] +=in[7];  in[7] +=in[5];  in[5] +=in[3];  in[3] +=in[1];

                        /* 9 point IDCT on even indices */

                        /* original: */

                        /*   for(i=0; i<9; i++) {
                        sum = 0.0;

                        for(j=0;j<18;j+=2)
                        sum += in[j] * cos(PI36 * (2*i + 1) * j);

                        tmp[i] = sum;
                        } */

/* 9 Point Inverse Discrete Cosine Transform
//
// This piece of code is Copyright 1997 Mikko Tommila and is freely usable
// by anybody. The algorithm itself is of course in the public domain.
//
// Again derived heuristically from the 9-point WFTA.
//
// The algorithm is optimized (?) for speed, not for small rounding errors or
// good readability.
//
// 36 additions, 11 multiplications
//
// Again this is very likely sub-optimal.
//
// The code is optimized to use a minimum number of temporary variables,
// so it should compile quite well even on 8-register Intel x86 processors.
// This makes the code quite obfuscated and very difficult to understand.
//
// References:
// [1] S. Winograd: "On Computing the Discrete Fourier Transform",
//     Mathematics of Computation, Volume 32, Number 141, January 1978,
//     Pages 175-199

// Some modifications for maplay by Jeff Tsay
*/
        {
        float t0, t1, t2, t3, t4, t5, t6, t7;

                t1 = COSPI3 * in[12];
                t2 = COSPI3 * (in[8] + in[16] - in[4]);

                t3 = in[0] + t1;
                t4 = in[0] - t1 - t1;
                t5 = t4 - t2;

                t0 = DCTEVEN1 * (in[4] + in[8]);
                t1 = DCTEVEN2 * (in[8] - in[16]);

                tmp[4] = t4 + t2 + t2;
                t2 = DCTEVEN3 * (in[4] + in[16]);

                t6 = t3 - t0 - t2;
                t0 += t3 + t1;
                t3 += t2 - t1;

                t2 = DCTODD1 * (in[2]  + in[10]);
                t4 = DCTODD2 * (in[10] - in[14]);
                t7 = COSPI6 * in[6];

                t1 = t2 + t4 + t7;
                tmp[0] = t0 + t1;
                tmp[8] = t0 - t1;
                t1 = DCTODD3 * (in[2] + in[14]);
                t2 += t1 - t7;

                tmp[3] = t3 + t2;
                t0 = COSPI6 * (in[10] + in[14] - in[2]);
                tmp[5] = t3 - t2;

                t4 -= t1 + t7;

                tmp[1] = t5 - t0;
                tmp[7] = t5 + t0;
                tmp[2] = t6 + t4;
                tmp[6] = t6 - t4;
        }

        /* End 9 point IDCT on even indices */

                        /* original: */
                        /*   for(i=0; i<9; i++) {
                        sum = 0.0;

                        for(j=0;j<18;j+=2)
                        sum += in[j+1] * cos(PI36 * (2*i + 1) * j);

                        tmp[17-i] = sum;
                        } */

        /* This includes multiplication by the twiddle factors */
        /* at the end -- Jeff. */
        {
        float t0, t1, t2, t3, t4, t5, t6, t7;

                t1 = COSPI3 * in[13];
                t2 = COSPI3 * (in[9] + in[17] - in[5]);

                t3 = in[1] + t1;
                t4 = in[1] - t1 - t1;
                t5 = t4 - t2;

                t0 = DCTEVEN1 * (in[5] + in[9]);
                t1 = DCTEVEN2 * (in[9] - in[17]);

                tmp[13] = (t4 + t2 + t2)*0.707106781f;
                t2 = DCTEVEN3 * (in[5] + in[17]);

                t6 = t3 - t0 - t2;
                t0 += t3 + t1;
                t3 += t2 - t1;

                t2 = DCTODD1 * (in[3]  + in[11]);
                t4 = DCTODD2 * (in[11] - in[15]);
                t7 = COSPI6  * in[7];

                t1 = t2 + t4 + t7;
                tmp[17] = (t0 + t1) * 0.501909918f;
                tmp[9]  = (t0 - t1) * 5.736856623f;
                t1 = DCTODD3 * (in[3] + in[15]);
                t2 += t1 - t7;

                tmp[14] = (t3 + t2) * 0.610387294f;
                t0 = COSPI6 * (in[11] + in[15] - in[3]);
                tmp[12] = (t3 - t2) * 0.871723397f;

                t4 -= t1 + t7;

                tmp[16] = (t5 - t0) * 0.517638090f;
                tmp[10] = (t5 + t0) * 1.931851653f;
                tmp[15] = (t6 + t4) * 0.551688959f;
                tmp[11] = (t6 - t4) * 1.183100792f;
        }

        /* End 9 point IDCT on odd indices */

        /* Butterflies on 9 point IDCT's */
        for (i=0;i<9;i++) {
                save = tmp[i];
                tmp[i] += tmp[17-i];
                tmp[17-i] = save - tmp[17-i];
        }
        /* end 18 point IDCT */

        /* twiddle factors for 36 point IDCT */

        tmp[0] *=  -0.500476342f;
        tmp[1] *=  -0.504314480f;
        tmp[2] *=  -0.512139757f;
        tmp[3] *=  -0.524264562f;
        tmp[4] *=  -0.541196100f;
        tmp[5] *=  -0.563690973f;
        tmp[6] *=  -0.592844523f;
        tmp[7] *=  -0.630236207f;
        tmp[8] *=  -0.678170852f;
        tmp[9] *=  -0.740093616f;
        tmp[10]*=  -0.821339815f;
        tmp[11]*=  -0.930579498f;
        tmp[12]*=  -1.082840285f;
        tmp[13]*=  -1.306562965f;
        tmp[14]*=  -1.662754762f;
        tmp[15]*=  -2.310113158f;
        tmp[16]*=  -3.830648788f;
        tmp[17]*= -11.46279281f;

        /* end 36 point IDCT */

        /* shift to modified IDCT */
        win_bt = win[win_type];

        res[sb][0] =-tmp[9]  * win_bt[0] + s[ch][sb][0];
        res[sb][1] =-tmp[10] * win_bt[1] + s[ch][sb][1];
        res[sb][2] =-tmp[11] * win_bt[2] + s[ch][sb][2];
        res[sb][3] =-tmp[12] * win_bt[3] + s[ch][sb][3];
        res[sb][4] =-tmp[13] * win_bt[4] + s[ch][sb][4];
        res[sb][5] =-tmp[14] * win_bt[5] + s[ch][sb][5];
        res[sb][6] =-tmp[15] * win_bt[6] + s[ch][sb][6];
        res[sb][7] =-tmp[16] * win_bt[7] + s[ch][sb][7];
        res[sb][8] =-tmp[17] * win_bt[8] + s[ch][sb][8];

        res[sb][9] = tmp[17] * win_bt[9] + s[ch][sb][9];
        res[sb][10]= tmp[16] * win_bt[10] + s[ch][sb][10];
        res[sb][11]= tmp[15] * win_bt[11] + s[ch][sb][11];
        res[sb][12]= tmp[14] * win_bt[12] + s[ch][sb][12];
        res[sb][13]= tmp[13] * win_bt[13] + s[ch][sb][13];
        res[sb][14]= tmp[12] * win_bt[14] + s[ch][sb][14];
        res[sb][15]= tmp[11] * win_bt[15] + s[ch][sb][15];
        res[sb][16]= tmp[10] * win_bt[16] + s[ch][sb][16];
        res[sb][17]= tmp[9]  * win_bt[17] + s[ch][sb][17];


        s[ch][sb][0]= tmp[8]  * win_bt[18];
        s[ch][sb][1]= tmp[7]  * win_bt[19];
        s[ch][sb][2]= tmp[6]  * win_bt[20];
        s[ch][sb][3]= tmp[5]  * win_bt[21];
        s[ch][sb][4]= tmp[4]  * win_bt[22];
        s[ch][sb][5]= tmp[3]  * win_bt[23];
        s[ch][sb][6]= tmp[2]  * win_bt[24];
        s[ch][sb][7]= tmp[1]  * win_bt[25];
        s[ch][sb][8]= tmp[0]  * win_bt[26];

        s[ch][sb][9]= tmp[0]  * win_bt[27];
        s[ch][sb][10]= tmp[1]  * win_bt[28];
        s[ch][sb][11]= tmp[2]  * win_bt[29];
        s[ch][sb][12]= tmp[3]  * win_bt[30];
        s[ch][sb][13]= tmp[4]  * win_bt[31];
        s[ch][sb][14]= tmp[5]  * win_bt[32];
        s[ch][sb][15]= tmp[6]  * win_bt[33];
        s[ch][sb][16]= tmp[7]  * win_bt[34];
        s[ch][sb][17]= tmp[8]  * win_bt[35];
    }

    if (sb&1) for (i=1;i<18;i+=2) res[sb][i]=-res[sb][i];
}

/* fast DCT according to Lee[84]
 * reordering according to Konstantinides[94]
 */
void poly(const int ch,int f)
{
static float u[2][2][32][16]; /* no v[][], it's redundant */
static int u_start[2]={0,0}; /* first element of u[][] */
static int u_div[2]={0,0}; /* which part of u[][] is currently used */
float c0,c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,c13,c14,c15;
float c16,c17,c18,c19,c20,c21,c22,c23,c24,c25,c26,c27,c28,c29,c30,c31;
float d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15;
float d16,d17,d18,d19,d20,d21,d22,d23,d24,d25,d26,d27,d28,d29,d30,d31;
int start = u_start[ch];
int div = u_div[ch];

        float (*u_p)[16];

        /* step 1: initial reordering and 1st (16 wide) butterflies
        */

        d0 =res[ 0][f]; d16=(d0  - res[31][f]) *  b1; d0 += res[31][f];
        d1 =res[ 1][f]; d17=(d1  - res[30][f]) *  b3; d1 += res[30][f];
        d3 =res[ 2][f]; d19=(d3  - res[29][f]) *  b5; d3 += res[29][f];
        d2 =res[ 3][f]; d18=(d2  - res[28][f]) *  b7; d2 += res[28][f];
        d6 =res[ 4][f]; d22=(d6  - res[27][f]) *  b9; d6 += res[27][f];
        d7 =res[ 5][f]; d23=(d7  - res[26][f]) * b11; d7 += res[26][f];
        d5 =res[ 6][f]; d21=(d5  - res[25][f]) * b13; d5 += res[25][f];
        d4 =res[ 7][f]; d20=(d4  - res[24][f]) * b15; d4 += res[24][f];
        d12=res[ 8][f]; d28=(d12 - res[23][f]) * b17; d12+= res[23][f];
        d13=res[ 9][f]; d29=(d13 - res[22][f]) * b19; d13+= res[22][f];
        d15=res[10][f]; d31=(d15 - res[21][f]) * b21; d15+= res[21][f];
        d14=res[11][f]; d30=(d14 - res[20][f]) * b23; d14+= res[20][f];
        d10=res[12][f]; d26=(d10 - res[19][f]) * b25; d10+= res[19][f];
        d11=res[13][f]; d27=(d11 - res[18][f]) * b27; d11+= res[18][f];
        d9 =res[14][f]; d25=(d9  - res[17][f]) * b29; d9 += res[17][f];
        d8 =res[15][f]; d24=(d8  - res[16][f]) * b31; d8 += res[16][f];

        /* step 2: 8-wide butterflies
        */
        c0 = d0 + d8 ; c8 = ( d0 - d8 ) *  b2;
        c1 = d1 + d9 ; c9 = ( d1 - d9 ) *  b6;
        c2 = d2 + d10; c10= ( d2 - d10) * b14;
        c3 = d3 + d11; c11= ( d3 - d11) * b10;
        c4 = d4 + d12; c12= ( d4 - d12) * b30;
        c5 = d5 + d13; c13= ( d5 - d13) * b26;
        c6 = d6 + d14; c14= ( d6 - d14) * b18;
        c7 = d7 + d15; c15= ( d7 - d15) * b22;

        c16=d16 + d24; c24= (d16 - d24) *  b2;
        c17=d17 + d25; c25= (d17 - d25) *  b6;
        c18=d18 + d26; c26= (d18 - d26) * b14;
        c19=d19 + d27; c27= (d19 - d27) * b10;
        c20=d20 + d28; c28= (d20 - d28) * b30;
        c21=d21 + d29; c29= (d21 - d29) * b26;
        c22=d22 + d30; c30= (d22 - d30) * b18;
        c23=d23 + d31; c31= (d23 - d31) * b22;

        /* step 3: 4-wide butterflies
        */
        d0 = c0 + c4 ; d4 = ( c0 - c4 ) *  b4;
        d1 = c1 + c5 ; d5 = ( c1 - c5 ) * b12;
        d2 = c2 + c6 ; d6 = ( c2 - c6 ) * b28;
        d3 = c3 + c7 ; d7 = ( c3 - c7 ) * b20;

        d8 = c8 + c12; d12= ( c8 - c12) *  b4;
        d9 = c9 + c13; d13= ( c9 - c13) * b12;
        d10= c10+ c14; d14= (c10 - c14) * b28;
        d11= c11+ c15; d15= (c11 - c15) * b20;

        d16= c16+ c20; d20= (c16 - c20) *  b4;
        d17= c17+ c21; d21= (c17 - c21) * b12;
        d18= c18+ c22; d22= (c18 - c22) * b28;
        d19= c19+ c23; d23= (c19 - c23) * b20;

        d24= c24+ c28; d28= (c24 - c28) *  b4;
        d25= c25+ c29; d29= (c25 - c29) * b12;
        d26= c26+ c30; d30= (c26 - c30) * b28;
        d27= c27+ c31; d31= (c27 - c31) * b20;

        /* step 4: 2-wide butterflies
        */
/**/    c0 = d0 + d2 ; c2 = ( d0 - d2 ) *  b8;
        c1 = d1 + d3 ; c3 = ( d1 - d3 ) * b24;
/**/    c4 = d4 + d6 ; c6 = ( d4 - d6 ) *  b8;
        c5 = d5 + d7 ; c7 = ( d5 - d7 ) * b24;
/**/    c8 = d8 + d10; c10= ( d8 - d10) *  b8;
        c9 = d9 + d11; c11= ( d9 - d11) * b24;
/**/    c12= d12+ d14; c14= (d12 - d14) *  b8;
        c13= d13+ d15; c15= (d13 - d15) * b24;
/**/    c16= d16+ d18; c18= (d16 - d18) *  b8;
        c17= d17+ d19; c19= (d17 - d19) * b24;
/**/    c20= d20+ d22; c22= (d20 - d22) *  b8;
        c21= d21+ d23; c23= (d21 - d23) * b24;
/**/    c24= d24+ d26; c26= (d24 - d26) *  b8;
        c25= d25+ d27; c27= (d25 - d27) * b24;
/**/    c28= d28+ d30; c30= (d28 - d30) *  b8;
        c29= d29+ d31; c31= (d29 - d31) * b24;

        /* step 5: 1-wide butterflies
        */
        d0 = c0 + c1 ; d1 = ( c0 - c1 ) * b16;
        d2 = c2 + c3 ; d3 = ( c2 - c3 ) * b16;
        d4 = c4 + c5 ; d5 = ( c4 - c5 ) * b16;
        d6 = c6 + c7 ; d7 = ( c6 - c7 ) * b16;
        d8 = c8 + c9 ; d9 = ( c8 - c9 ) * b16;
        d10= c10+ c11; d11= (c10 - c11) * b16;
        d12= c12+ c13; d13= (c12 - c13) * b16;
        d14= c14+ c15; d15= (c14 - c15) * b16;
        d16= c16+ c17; d17= (c16 - c17) * b16;
        d18= c18+ c19; d19= (c18 - c19) * b16;
        d20= c20+ c21; d21= (c20 - c21) * b16;
        d22= c22+ c23; d23= (c22 - c23) * b16;
        d24= c24+ c25; d25= (c24 - c25) * b16;
        d26= c26+ c27; d27= (c26 - c27) * b16;
        d28= c28+ c29; d29= (c28 - c29) * b16;
        d30= c30+ c31; d31= (c30 - c31) * b16;

        /* step 6: final resolving & reordering
         * the other 32 are stored for use with the next granule
         */

        u_p = (float (*)[16])&u[ch][div][0][start];

/*16*/                 u_p[0][0] =+d1 ;
        u_p[31][0] = -(u_p[1][0] =+d16 +d17 +d18 +d22 -d30);
        u_p[30][0] = -(u_p[2][0] =+d8 +d9 +d10 -d14);
        u_p[29][0] = -(u_p[3][0] =-d16 -d17 -d18 -d22 +d24 +d25 +d26);
/*20*/  u_p[28][0] = -(u_p[4][0] =+d4 +d5 -d6);
        u_p[27][0] = -(u_p[5][0] =+d16 +d17 +d18 +d20 +d21 -d24 -d25 -d26);
        u_p[26][0] = -(u_p[6][0] =-d8 -d9 -d10 +d12 +d13);
        u_p[25][0] = -(u_p[7][0] =-d16 -d17 -d18 -d20 -d21 +d28 +d29);
/*24*/  u_p[24][0] = -(u_p[8][0] =-d2 +d3);
        u_p[23][0] = -(u_p[9][0] =+d16 +d17 +d19 +d20 +d21 -d28 -d29);
        u_p[22][0] = -(u_p[10][0] =+d8 +d9 +d11 -d12 -d13);
        u_p[21][0] = -(u_p[11][0] =-d16 -d17 -d19 -d20 -d21 +d24 +d25 +d27);
/*28*/  u_p[20][0] = -(u_p[12][0] =-d4 -d5 +d7);
        u_p[19][0] = -(u_p[13][0] =+d16 +d17 +d19 +d23 -d24 -d25 -d27);
        u_p[18][0] = -(u_p[14][0] =-d8 -d9 -d11 +d15);
        u_p[17][0] = -(u_p[15][0]   =-d16 -d17 -d19 -d23 +d31);
        u_p[16][0] = 0.0f;

        /* the other 32 are stored for use with the next granule
         */

        u_p = (float (*)[16])&u[ch][!div][0][start];

/*0*/   u_p[16][0] = -2*d0;
        u_p[15][0] = u_p[17][0] = -(+d16 );
        u_p[14][0] = u_p[18][0] = -(+d8 );
        u_p[13][0] = u_p[19][0] = -(-d16 +d24 );
/*4*/   u_p[12][0] = u_p[20][0] = -(+d4 );
        u_p[11][0] = u_p[21][0] = -(+d16 +d20 -d24 );
        u_p[10][0] = u_p[22][0] = -(-d8 +d12 );
        u_p[9][0] = u_p[23][0] = -(-d16 -d20 +d28 );
/*8*/   u_p[8][0] = u_p[24][0] = -(+d2 );
        u_p[7][0] = u_p[25][0] = -(+d16 +d18 +d20 -d28 );
        u_p[6][0] = u_p[26][0] = -(+d8 +d10 -d12 );
        u_p[5][0] = u_p[27][0] = -(-d16 -d18 -d20 +d24 +d26 );
/*12*/  u_p[4][0] = u_p[28][0] = -(-d4 +d6 );
        u_p[3][0] = u_p[29][0] = -(+d16 +d18 +d22 -d24 -d26 );
        u_p[2][0] = u_p[30][0] = -(-d8 -d10 +d14 );
        u_p[1][0] = u_p[31][0] = -(-d16 -d18 -d22 +d30 );
        u_p[0][0] = -d1;


        /* we're doing dewindowing and calculating final samples now
         */

        {
        int j;
        float out;
        int outi;
        float *dewindow = (float*) t_dewindow;
        float *u_ptr;

                u_p = u[ch][div];

                if (nch == 2) {
                        switch (start) {
#if !defined(MEDIUM_STEREO_CACHE) && !defined(SMALL_STEREO_CACHE)
                        case 0:
                                u_ptr = (float *)u_p;

                                for (j=0;j<32;j++) {
                                        out  = *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 1:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 2:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 3:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 4:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 5:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 6:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 7:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

#endif
#if !defined(SMALL_STEREO_CACHE)
                        case 8:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 9:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 10:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 11:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 12:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 13:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 14:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 15:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;

                                        outi = out;
                                        stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;
#endif
#if defined(MEDIUM_STEREO_CACHE) || defined(SMALL_STEREO_CACHE)
                        default:
                                {
                                int i=start;

                                        for (j=0;j<32;j++) {
                                                u_ptr = u_p[j];

                                                out  = u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;

                                                outi = out;
                                                stereo_samples[f][j][ch] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                        }
                                }
                                break;
#endif
                        }

                        u_start[ch]=(u_start[ch]-1)&0xf;
                        u_div[ch]=u_div[ch] ? 0 : 1;
                } else {
                        switch (start) {
#if !defined(MEDIUM_MONO_CACHE) && !defined(SMALL_MONO_CACHE)
                        case 0:
                                u_ptr = (float *)u_p;

                                for (j=0;j<32;j++) {
                                        out  = *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;
                                        out += *u_ptr++ * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 1:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 2:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 3:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 4:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 5:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 6:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 7:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

#endif
#if !defined(SMALL_MONO_CACHE)
                        case 8:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 9:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 10:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 11:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 12:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 13:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 14:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[14] * *dewindow++;
                                        out += u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;

                        case 15:
                                for (j=0;j<32;j++) {
                                        u_ptr = u_p[j];

                                        out  = u_ptr[15] * *dewindow++;
                                        out += u_ptr[0] * *dewindow++;
                                        out += u_ptr[1] * *dewindow++;
                                        out += u_ptr[2] * *dewindow++;
                                        out += u_ptr[3] * *dewindow++;
                                        out += u_ptr[4] * *dewindow++;
                                        out += u_ptr[5] * *dewindow++;
                                        out += u_ptr[6] * *dewindow++;
                                        out += u_ptr[7] * *dewindow++;
                                        out += u_ptr[8] * *dewindow++;
                                        out += u_ptr[9] * *dewindow++;
                                        out += u_ptr[10] * *dewindow++;
                                        out += u_ptr[11] * *dewindow++;
                                        out += u_ptr[12] * *dewindow++;
                                        out += u_ptr[13] * *dewindow++;
                                        out += u_ptr[14] * *dewindow++;

                                        outi = out;
                                        mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                }
                                break;
#endif
#if defined(MEDIUM_MONO_CACHE) || defined(SMALL_MONO_CACHE)
                        default:
                                {
                                int i=start;

                                        for (j=0;j<32;j++) {
                                                u_ptr = u_p[j];

                                                out  = u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;
                                                out += u_ptr[i++ & 0xf] * *dewindow++;

                                                outi = out;
                                                mono_samples[f][j] = outi > 32767 ? 32767 : outi < -32768 ? -32768 : outi;
                                        }
                                }
                                break;
#endif
                        }

                        u_start[0]=(u_start[0]-1)&0xf;
                        u_div[0]=u_div[0] ? 0 : 1;
                }
        }
}

void premultiply()
{
  int i,t;

  for (i = 0; i < 16; ++i)
    for (t = 0; t < 32; ++t)
      t_dewindow[i][t] *= 16383.5f;
}


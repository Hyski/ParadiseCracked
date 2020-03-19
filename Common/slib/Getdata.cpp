#include "proto.h"

int decode_scalefactors(struct SIDE_INFO *info,struct AUDIO_HEADER *header,int gr,int ch)
{
int sfb,window;
int slen1,slen2;
int i1,i2,i=0;
int j,k;
        if (header->ID==1) {

        /* this is MPEG-1 scalefactors format, quite different than
         * the MPEG-2 format.
         */
                slen1=t_slen1[info->scalefac_compress[gr][ch]];
                slen2=t_slen2[info->scalefac_compress[gr][ch]];
                i1=3*slen1;
                i2=3*slen2;

                if (info->window_switching_flag[gr][ch] && info->block_type[gr][ch]==2) {
                        if (info->mixed_block_flag[gr][ch]) {
                                for (sfb=0;sfb<8;sfb++) {
                                        scalefac_l[gr][ch][sfb]=getbits(slen1);
                                        i+=slen1;
                                }
                                for (sfb=3;sfb<6;sfb++) {
                                        for (window=0;window<3;window++)
                                                scalefac_s[gr][ch][sfb][window]=getbits(slen1);
                                        i+=i1;
                                }
                                for (;sfb<12;sfb++) {
                                        for (window=0;window<3;window++)
                                                scalefac_s[gr][ch][sfb][window]=getbits(slen2);
                                        i+=i2;
                                }
                        } else { /* !mixed_block_flag */
                                for (sfb=0;sfb<6;sfb++) {
                                        for (window=0;window<3;window++)
                                                scalefac_s[gr][ch][sfb][window]=getbits(slen1);
                                        i+=i1;
                                }
                                for (;sfb<12;sfb++) {
                                        for (window=0;window<3;window++)
                                                scalefac_s[gr][ch][sfb][window]=getbits(slen2);
                                        i+=i2;
                                }
                        }
                        for (window=0;window<3;window++)
                                scalefac_s[gr][ch][12][window]=0;
                } else { /* block_type!=2 */
                        if ( !info->scfsi[ch][0] || !gr )
                                for (sfb=0;sfb<6;sfb++) {
                                        scalefac_l[gr][ch][sfb]=getbits(slen1);
                                        i+=slen1;
                                }
                        else for (sfb=0;sfb<6;sfb++) {
                                scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];
                        }
                        if ( !info->scfsi[ch][1] || !gr )
                                for (sfb=6;sfb<11;sfb++) {
                                        scalefac_l[gr][ch][sfb]=getbits(slen1);
                                        i+=slen1;
                                }
                        else for (sfb=6;sfb<11;sfb++) {
                                scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];
                        }
                        if ( !info->scfsi[ch][2] || !gr )
                                for (sfb=11;sfb<16;sfb++) {
                                        scalefac_l[gr][ch][sfb]=getbits(slen2);
                                        i+=slen2;
                                }
                        else for (sfb=11;sfb<16;sfb++) {
                                scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];
                        }
                        if ( !info->scfsi[ch][3] || !gr )
                                for (sfb=16;sfb<21;sfb++) {
                                        scalefac_l[gr][ch][sfb]=getbits(slen2);
                                        i+=slen2;
                                }
                        else for (sfb=16;sfb<21;sfb++) {
                                scalefac_l[1][ch][sfb]=scalefac_l[0][ch][sfb];
                        }
                        scalefac_l[gr][ch][21]=0;
                }
        } else { /* ID==0 */
                int index,index2,spooky_index;
                int slen[5],nr_of_sfb[5]; /* actually, there's four of each, not five, labelled 1 through 4, but
                                           * what's a word of storage compared to one's sanity. so [0] is irellevant.
                                           */

                /* ok, so we got 3 indexes.
                 * spooky_index - indicates whether we use the normal set of slen eqs and nr_of_sfb tables
                 *                or the one for the right channel of intensity stereo coded frame
                 * index        - corresponds to the value of scalefac_compress, as listed in the standard
                 * index2       - 0 for long blocks, 1 for short wo/ mixed_block_flag, 2 for short with it
                 */
                if ( (header->mode_extension==1 || header->mode_extension==3) && ch==1) { /* right ch... */
                        int int_scalefac_compress=info->scalefac_compress[0][ch]>>1;
                        intensity_scale=info->scalefac_compress[0][1]&1;
                        spooky_index=1;
                        if (int_scalefac_compress < 180) {
                                slen[1]=int_scalefac_compress/36;
                                slen[2]=(int_scalefac_compress%36)/6;
                                slen[3]=(int_scalefac_compress%36)%6;
                                slen[4]=0;
                                info->preflag[0][ch]=0;
                                index=0;
                        }
                        if ( 180 <= int_scalefac_compress && int_scalefac_compress < 244) {
                                slen[1]=((int_scalefac_compress-180)%64)>>4;
                                slen[2]=((int_scalefac_compress-180)%16)>>2;
                                slen[3]=(int_scalefac_compress-180)%4;
                                slen[4]=0;
                                info->preflag[0][ch]=0;
                                index=1;
                        }
                        if ( 244 <= int_scalefac_compress && int_scalefac_compress < 255) {
                                slen[1]=(int_scalefac_compress-244)/3;
                                slen[2]=(int_scalefac_compress-244)%3;
                                slen[3]=0;
                                slen[4]=0;
                                info->preflag[0][ch]=0;
                                index=2;
                        }
                } else { /* the usual */
                        spooky_index=0;
                        if (info->scalefac_compress[0][ch] < 400) {
                                slen[1]=(info->scalefac_compress[0][ch]>>4)/5;
                                slen[2]=(info->scalefac_compress[0][ch]>>4)%5;
                                slen[3]=(info->scalefac_compress[0][ch]%16)>>2;
                                slen[4]=info->scalefac_compress[0][ch]%4;
                                info->preflag[0][ch]=0;
                                index=0;
                        }
                        if (info->scalefac_compress[0][ch] >= 400 && info->scalefac_compress[0][ch] < 500) {
                                slen[1]=((info->scalefac_compress[0][ch]-400)>>2)/5;
                                slen[2]=((info->scalefac_compress[0][ch]-400)>>2)%5;
                                slen[3]=(info->scalefac_compress[0][ch]-400)%4;
                                slen[4]=0;
                                info->preflag[0][ch]=0;
                                index=1;
                        }
                        if (info->scalefac_compress[0][ch] >= 500 && info->scalefac_compress[0][ch] < 512) {
                                slen[1]=(info->scalefac_compress[0][ch]-500)/3;
                                slen[2]=(info->scalefac_compress[0][ch]-500)%3;
                                slen[3]=0;
                                slen[4]=0;
                                info->preflag[0][ch]=1;
                                index=2;
                        }
                }

                if (info->window_switching_flag[0][ch] && info->block_type[0][ch]==2)
                        if (info->mixed_block_flag[0][ch]) index2=2;
                        else index2=1;
                else index2=0;

                for (j=1;j<=4;j++) nr_of_sfb[j]=spooky_table[spooky_index][index][index2][j-1];

        /* now we'll do some actual scalefactor extraction, and a little more.
         * for each scalefactor band we'll set the value of is_max to indicate
         * illegal is_pos, since with MPEG2 it's not 'hardcoded' to 7.
         */
                if (!info->window_switching_flag[0][ch] || (info->window_switching_flag[0][ch] && info->block_type[0][ch]!=2)) {
                        sfb=0;
                        for (j=1;j<=4;j++)
                                for (k=0;k<nr_of_sfb[j];k++) {
                                        scalefac_l[0][ch][sfb]=getbits(slen[j]);
                                        i+=slen[j];
                                        if (ch) is_max[sfb]=(1<<slen[j])-1;
                                        sfb++;
                                }
                } else if (info->block_type[0][ch]==2)
                        if (!info->mixed_block_flag[0][ch]) {
                                sfb=0;
                                for (j=1;j<=4;j++)
                                        for (k=0;k<nr_of_sfb[j];k+=3) {
                                                /* we assume here that nr_of_sfb is divisible by 3. it is.
                                                 */
                                                scalefac_s[0][ch][sfb][0]=getbits(slen[j]);
                                                scalefac_s[0][ch][sfb][1]=getbits(slen[j]);
                                                scalefac_s[0][ch][sfb][2]=getbits(slen[j]);
                                                i+=3*slen[j];
                                                if (ch) is_max[sfb+6]=(1<<slen[j])-1;
                                                sfb++;
                                        }
                        } else {
                                /* what we do here is:
                                 * 1. assume that for every fs, the two lowest subbands are equal to the
                                 *    six lowest scalefactor bands for long blocks/MPEG2. they are.
                                 * 2. assume that for every fs, the two lowest subbands are equal to the
                                 *    three lowest scalefactor bands for short blocks. they are.
                                 */
                                sfb=0;
                                for (k=0;k<6;k++) {
                                        scalefac_l[0][ch][sfb]=getbits(slen[1]);
                                        i+=slen[j];
                                        if (ch) is_max[sfb]=(1<<slen[1])-1;
                                        sfb++;
                                }
                                nr_of_sfb[1]-=6;
                                sfb=3;
                                for (j=1;j<=4;j++)
                                        for (k=0;k<nr_of_sfb[j];k+=3) {
                                                scalefac_s[0][ch][sfb][0]=getbits(slen[j]);
                                                scalefac_s[0][ch][sfb][1]=getbits(slen[j]);
                                                scalefac_s[0][ch][sfb][2]=getbits(slen[j]);
                                                i+=3*slen[j];
                                                if (ch) is_max[sfb+6]=(1<<slen[j])-1;
                                                sfb++;
                                        }
                        }
        }
return i;
}

/* this is for huffman decoding, but inlined funcs have to go first
 */
static __inline int _qsign(int x,int *q)
{
int ret_value=0,i;
        for (i=3;i>=0;i--)
                if ((x>>i) & 1) {
                        if (getbits(1)) *q++=-1;
                                else *q++=1;
                        ret_value++;
                }
                else *q++=0;
        return ret_value;
}

int decode_huffman_data(struct SIDE_INFO *info,int gr,int ch,int ssize)
{
int l,i,cnt,x,y;
int q[4],r[3],linbits[3],tr[4]={0,0,0,0};
int big_value = info->big_values[gr][ch] << 1;

        for (l=0;l<3;l++) {
                tr[l]=info->table_select[gr][ch][l];
                linbits[l]=t_linbits[info->table_select[gr][ch][l]];
        }

        tr[3]=32+info->count1table_select[gr][ch];

        /* we have to be careful here because big_values are not necessarily
         * aligned with sfb boundaries
         */
        if (!info->window_switching_flag[gr][ch] && info->block_type[gr][ch]==0) {

        /* this code needed some cleanup
        */
                r[0]=t_l[info->region0_count[gr][ch]] + 1;
                if (r[0] > big_value)
                        r[0]=r[1]=big_value;
                else {
                        r[1]=t_l[ info->region0_count[gr][ch] + info->region1_count[gr][ch] + 1 ] + 1;
                        if (r[1] > big_value)
                                r[1]=big_value;
                }
                r[2]=big_value;

        } else {

                if (info->block_type[gr][ch]==2 && info->mixed_block_flag[gr][ch]==0)
                        r[0]=3*(t_s[2]+1);
                else
                        r[0]=t_l[7]+1;

                if (r[0] > big_value)
                        r[0]=big_value;

                r[1]=r[2]=big_value;
        }

        l=0; cnt=0;
        for (i=0;i<3;i++) {
                for (;l<r[i];l+=2) {
                        int j = linbits[i];

                        cnt+=huffman_decode(tr[i],&x,&y);

                        if (x==15 && j>0) {
                                x+=getbits(j);
                                cnt+=j;
                        }
                        if (x) {
                                if (getbits(1)) x=-x;
                                cnt++;
                        }
                        if (y==15 && j>0) {
                                y+=getbits(j);
                                cnt+=j;
                        }
                        if (y) {
                                if (getbits(1)) y=-y;
                                cnt++;
                        }

//                      if (SHOW_HUFFBITS) printf(" (%d,%d)\n",x,y);
                        is[ch][l]=x;
                        is[ch][l+1]=y;
                }
        }
        while ((cnt < info->part2_3_length[gr][ch]-ssize) && (l<576)) {
                cnt+=huffman_decode(tr[3],&x,&y);
                cnt+=_qsign(x,q);
                for (i=0;i<4;i++) is[ch][l+i]=q[i]; /* ziher je ziher, is[578]*/
                l+=4;
        }
        /*  set position to start of the next gr/ch
         */
        if (cnt != info->part2_3_length[gr][ch] - ssize ) {
                data-=cnt-(info->part2_3_length[gr][ch] - ssize);
                data&= 8*BUFFER_SIZE - 1;
        }
        if (l<576) non_zero[ch]=l;
        else non_zero[ch]=576;
        /* zero out everything else
        */
        for (;l<576;l++) is[ch][l]=0;
        return 1;
}

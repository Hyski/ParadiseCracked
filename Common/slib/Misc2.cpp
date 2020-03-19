#include "proto.h"

extern const char* log_name;

#pragma warning (disable : 4244)
static __inline float fras_l(int sfb,int global_gain,int scalefac_scale,int scalefac,int preflag)
{
int a,scale;
        if (scalefac_scale) scale=2;
        else scale=1;
        a=global_gain - 210 - (scalefac << scale);
        if (preflag) a-=(t_pretab[sfb] << scale);

        if (a < -127) return 0;

        if (a>=0) return tab[a&3]*(1 << (a>>2));
        else return tabi[(-a)&3]/(1 << ((-a) >> 2));
}

static __inline float fras_s(int global_gain,int subblock_gain,int scalefac_scale,int scalefac)
{
int a;
        a=global_gain - 210 - (subblock_gain << 3);
        if (scalefac_scale) a-= (scalefac << 2);
        else a-= (scalefac << 1);

        if (a < -127) return 0;

        if (a>=0) return tab[a&3]*(1 << (a>>2));
        else return tabi[(-a)&3]/(1 << ((-a) >> 2));
}

static __inline float fras2(int is,float a)
{
        if (is==0) return 0;
        if (is > 0) return t_43[is]*a;
        else return -t_43[-is]*a;
}


void requantize_mono(int gr,int ch,struct SIDE_INFO *info,struct AUDIO_HEADER *header)
{
int l,i,sfb;
float a;
int global_gain=info->global_gain[gr][ch];
int scalefac_scale=info->scalefac_scale[gr][ch];
int sfreq=header->sampling_frequency;


        no_of_imdcts[0]=no_of_imdcts[1]=32;

        if (info->window_switching_flag[gr][ch] && info->block_type[gr][ch]==2)
                if (info->mixed_block_flag[gr][ch]) {
                        int window,window_len,preflag=0; /* pretab is all zero in this low frequency area */
                        int scalefac=scalefac_l[gr][ch][0];

                        l=0;sfb=0;
                        a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
                        while (l<36) {
                                xr[ch][0][l]=fras2(is[ch][l],a);
                                if (l==t_l[sfb]) {
                                        scalefac=scalefac_l[gr][ch][++sfb];
                                        a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
                                }
                                l++;
                        }
                        sfb=3;
                        window_len=t_s[sfb]-t_s[sfb-1];
                        while (l<non_zero[ch]) {
                                for (window=0;window<3;window++) {
                                        int scalefac=scalefac_s[gr][ch][sfb][window];
                                        int subblock_gain=info->subblock_gain[gr][ch][window];
                                        a=fras_s(global_gain,subblock_gain,scalefac_scale,scalefac);
                                        for (i=0;i<window_len;i++) {
                                                xr[ch][0][t_reorder[header->ID][sfreq][l]]=fras2(is[ch][l],a);
                                                l++;
                                        }
                                }
                                sfb++;
                                window_len=t_s[sfb]-t_s[sfb-1];
                        }
                        while (l<576) xr[ch][0][t_reorder[header->ID][sfreq][l++]]=0;
                } else {
                        int window,window_len;

                        sfb=0; l=0;
                        window_len=t_s[0]+1;
                        while (l<non_zero[ch]) {
                                for (window=0;window<3;window++) {
                                        int scalefac=scalefac_s[gr][ch][sfb][window];
                                        int subblock_gain=info->subblock_gain[gr][ch][window];
                                        float a=fras_s(global_gain,subblock_gain,scalefac_scale,scalefac);
                                        for (i=0;i<window_len;i++) {
                                                xr[ch][0][t_reorder[header->ID][sfreq][l]]=fras2(is[ch][l],a);
                                                l++;
                                        }
                                }
                                sfb++;
                                window_len=t_s[sfb]-t_s[sfb-1];
                        }
                        while (l<576) xr[ch][0][t_reorder[header->ID][sfreq][l++]]=0;
                }
        else {
        /* long blocks */
                int preflag=info->preflag[gr][ch];
                int scalefac=scalefac_l[gr][ch][0];

                sfb=0; l=0;
                a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
                while (l<non_zero[ch]) {
                        xr[ch][0][l]=fras2(is[ch][l],a);
                        if (l==t_l[sfb]) {
                                scalefac=scalefac_l[gr][ch][++sfb];
                                a=fras_l(sfb,global_gain,scalefac_scale,scalefac,preflag);
                        }
                        l++;
                }
                while (l<576) xr[ch][0][l++]=0;
        }
}
static int find_isbound(int isbound[3],int gr,struct SIDE_INFO *info,struct AUDIO_HEADER *header)
{
int sfb,window,window_len,ms_flag,tmp,i;

        isbound[0]=isbound[1]=isbound[2]=-1;
        no_of_imdcts[0]=no_of_imdcts[1]=32;

   if (header->mode_extension==1 || header->mode_extension==3) {
        if (info->window_switching_flag[gr][0] && info->block_type[gr][0]==2) {

                tmp=non_zero[1];
                sfb=0; while ((3*t_s[sfb]+2) < tmp  && sfb < 12) sfb++;
                while ((isbound[0]<0 || isbound[1]<0 || isbound[2]<0) && !(info->mixed_block_flag[gr][0] && sfb<3) && sfb) {
                        for (window=0;window<3;window++) {
                                if (sfb==0) {
                                        window_len=t_s[0]+1;
                                        tmp=(window+1)*window_len - 1;
                                } else {
                                        window_len=t_s[sfb]-t_s[sfb-1];
                                        tmp=(3*t_s[sfb-1]+2) + (window+1)*window_len;
                                }
                                if (isbound[window] < 0)
                                        for (i=0;i<window_len;i++)
                                                if (is[1][tmp--] != 0) {
                                                        isbound[window]=t_s[sfb]+1;
                                                        break;
                                                }
                        }
                        sfb--;
                }

                if (sfb==2 && info->mixed_block_flag[gr][0])
                        if (isbound[0]<0 && isbound[1]<0 && isbound[2]<0) {
                                tmp=35;
                                //Commented by DisPell//while (is[1][tmp] == 0) tmp--;
                                while (tmp>=0 && is[1][tmp]==0) tmp--;//Added by DisPell//
                                sfb=0; while (t_l[sfb] < tmp  && sfb < 21) sfb++;
                                isbound[0]=isbound[1]=isbound[2]=t_l[sfb]+1;
                        } else for (window=0;window<3;window++)
                                if (isbound[window]<0) isbound[window]=36;
                if (header->ID==1) isbound[0]=isbound[1]=isbound[2]=MAX(isbound[0],MAX(isbound[1],isbound[2]));

                tmp=non_zero[0];
                sfb=0; while ((3*t_s[sfb]+2) < tmp && sfb < 12) sfb++;
                no_of_imdcts[0]=no_of_imdcts[1]=(t_s[sfb]-1)/6+1; /* 18?????? */

        } else {

                tmp=non_zero[1];
                //Commented by DisPell//while (is[1][tmp] == 0) tmp--;
                while(tmp>=0 && is[1][tmp]==0) tmp--;//Added by DisPell//
                sfb=0; while (t_l[sfb] < tmp && sfb < 21) sfb++;
                isbound[0]=isbound[1]=isbound[2]=t_l[sfb]+1;
                no_of_imdcts[0]=no_of_imdcts[1]=(non_zero[0]-1)/18+1; /* left channel should have more elements here */
        }
        if (header->mode_extension==1) ms_flag=0;
        else ms_flag=1;
   } else {

        ms_flag=1;

        if (!info->window_switching_flag[gr][0] || (info->window_switching_flag[gr][0] && info->block_type[gr][0]!=2))
                isbound[0]=isbound[1]=isbound[2]=(MAX(non_zero[0],non_zero[1]));
        else isbound[0]=isbound[1]=isbound[2]=576;

        if (info->window_switching_flag[gr][0] && info->block_type[gr][0]==2) {

                        tmp=(MAX(non_zero[0],non_zero[1]))/3;
                        sfb=0; while (t_s[sfb]<tmp && sfb<12) sfb++;
                        no_of_imdcts[0]=no_of_imdcts[1]=(t_s[sfb]-1)/6+1;
        }
        else no_of_imdcts[0]=no_of_imdcts[1]=(isbound[0]-1)/18+1;

   }

   return ms_flag;
}

static __inline void stereo_s(int l,float a[2],int pos,int ms_flag,int is_pos,struct AUDIO_HEADER *header)
{
float ftmp,Mi,Si;

        if (l>=576) return;

        if ((is_pos != IS_ILLEGAL) && (header->ID==1)) {
                ftmp=fras2(is[0][l],a[0]);
                xr[0][0][pos]=(1-t_is[is_pos])*ftmp;
                xr[1][0][pos]=t_is[is_pos]*ftmp;
                return;
        }

        if ((is_pos != IS_ILLEGAL) && (header->ID==0)) {
                ftmp=fras2(is[0][l],a[0]);
                if (is_pos&1) {
                        xr[0][0][pos]= t_is2[intensity_scale][(is_pos+1)>>1] * ftmp;
                        xr[1][0][pos]= ftmp;
                } else {
                        xr[0][0][pos]= ftmp;
                        xr[1][0][pos]= t_is2[intensity_scale][is_pos>>1] * ftmp;
                }
                return;
        }

        if (ms_flag) {
                Mi=fras2(is[0][l],a[0]);
                Si=fras2(is[1][l],a[1]);
                xr[0][0][pos]=(Mi+Si)*i_sq2;
                xr[1][0][pos]=(Mi-Si)*i_sq2;
        } else {
                xr[0][0][pos]=fras2(is[0][l],a[0]);
                xr[1][0][pos]=fras2(is[1][l],a[1]);
        }
}

static __inline void stereo_l(int l,float a[2],int ms_flag,int is_pos,struct AUDIO_HEADER *header)
{
float ftmp,Mi,Si;
        if (l>=576) return;
        if ((is_pos != IS_ILLEGAL) && (header->ID==1)) {
                ftmp=fras2(is[0][l],a[0]);
                xr[0][0][l]=(1-t_is[is_pos])*ftmp;
                xr[1][0][l]=t_is[is_pos]*ftmp;
                return;
        }

        if ((is_pos != IS_ILLEGAL) && (header->ID==0)) {
                ftmp=fras2(is[0][l],a[0]);
                if (is_pos&1) {
                        xr[0][0][l]= t_is2[intensity_scale][(is_pos+1)>>1] * ftmp;
                        xr[1][0][l]= ftmp;
                } else {
                        xr[0][0][l]= ftmp;
                        xr[1][0][l]= t_is2[intensity_scale][is_pos>>1] * ftmp;
                }
                return;
        }

        if (ms_flag) {
                Mi=fras2(is[0][l],a[0]);
                Si=fras2(is[1][l],a[1]);
                xr[0][0][l]=(Mi+Si)*i_sq2;
                xr[1][0][l]=(Mi-Si)*i_sq2;
        } else {
                xr[0][0][l]=fras2(is[0][l],a[0]);
                xr[1][0][l]=fras2(is[1][l],a[1]);
        }

}


void requantize_ms(int gr,struct SIDE_INFO *info,struct AUDIO_HEADER *header)
{
int l,sfb,ms_flag,is_pos,i,ch;
int *global_gain,subblock_gain[2],*scalefac_scale,scalefac[2],isbound[3];
int sfreq=header->sampling_frequency;
int id = header->ID;
float a[2];

global_gain=info->global_gain[gr];
scalefac_scale=info->scalefac_scale[gr];

        if (info->window_switching_flag[gr][0] && info->block_type[gr][0]==2)
                if (info->mixed_block_flag[gr][0]) {
                        int window,window_len;
                        int preflag[2]={0,0};

                        ms_flag=find_isbound(isbound,gr,info,header);

                        sfb=0; l=0;
                        for (ch=0;ch<2;ch++) {
                                scalefac[ch]=scalefac_l[gr][ch][0];
                                a[ch]=fras_l(0,global_gain[ch],scalefac_scale[ch],scalefac[ch],preflag[ch]);
                        }


                        while (l<36) {
                                int is_pos;
                                if (l<isbound[0]) is_pos=IS_ILLEGAL;
                                else {
                                        is_pos=scalefac[1];
                                        if (id==1) /* MPEG1 */
                                                if (is_pos==7) is_pos=IS_ILLEGAL;
                                        else /* MPEG2 */
                                                if (is_pos==is_max[sfb]) is_pos=IS_ILLEGAL;
                                }

                                stereo_l(l,a,ms_flag,is_pos,header);

                                if (l==t_l[sfb]) {
                                        sfb++;
                                        for (ch=0;ch<2;ch++) {
                                                scalefac[ch]=scalefac_l[gr][ch][sfb];
                                                a[ch]=fras_l(sfb,global_gain[ch],scalefac_scale[ch],scalefac[ch],preflag[ch]);
                                        }
                                }

                                l++;
                        }
                        sfb=3;
                        window_len=t_s[sfb]-t_s[sfb-1];

                        while (l<(MAX(non_zero[0],non_zero[1]))) {
                                for (window=0;window<3;window++) {
                                        subblock_gain[0]=info->subblock_gain[gr][0][window];
                                        subblock_gain[1]=info->subblock_gain[gr][1][window];
                                        scalefac[0]=scalefac_s[gr][0][sfb][window];
                                        scalefac[1]=scalefac_s[gr][1][sfb][window];

                                        if (t_s[sfb] < isbound[window]) {
                                                is_pos=IS_ILLEGAL;
                                                a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
                                                a[1]=fras_s(global_gain[1],subblock_gain[1],scalefac_scale[1],scalefac[1]);
                                        } else {
                                                is_pos=scalefac[1];
                                                if (id==1) /* MPEG1 */
                                                        if (is_pos==7) is_pos=IS_ILLEGAL;
                                                else /* MPEG2 */
                                                        if (is_pos==is_max[sfb+6]) is_pos=IS_ILLEGAL;

                                                a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
                                        }

                                        for (i=0;i<window_len;i++) {
                                                stereo_s(l,a,t_reorder[id][sfreq][l],ms_flag,is_pos,header);
                                                l++;
                                        }
                                }
                                sfb++;
                                window_len=t_s[sfb]-t_s[sfb-1];
                        }
                        while (l<576) {
                                int reorder = t_reorder[id][sfreq][l++];

                                xr[0][0][reorder]=xr[1][0][reorder]=0;
                        }
                } else {
                        int window,window_len;

                        ms_flag=find_isbound(isbound,gr,info,header);
                        sfb=0; l=0;
                        window_len=t_s[0]+1;

                        while (l<(MAX(non_zero[0],non_zero[1]))) {
                                for (window=0;window<3;window++) {
                                        subblock_gain[0]=info->subblock_gain[gr][0][window];
                                        subblock_gain[1]=info->subblock_gain[gr][1][window];
                                        scalefac[0]=scalefac_s[gr][0][sfb][window];
                                        scalefac[1]=scalefac_s[gr][1][sfb][window];

                                        if (t_s[sfb] < isbound[window]) {
                                                is_pos=IS_ILLEGAL;
                                                a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
                                                a[1]=fras_s(global_gain[1],subblock_gain[1],scalefac_scale[1],scalefac[1]);
                                        } else {
                                                is_pos=scalefac[1];
                                                if (id==1) /* MPEG1 */
                                                        if (is_pos==7) is_pos=IS_ILLEGAL;
                                                else /* MPEG2 */
                                                        if (is_pos==is_max[sfb+6]) is_pos=IS_ILLEGAL;

                                                a[0]=fras_s(global_gain[0],subblock_gain[0],scalefac_scale[0],scalefac[0]);
                                        }

                                        for (i=0;i<window_len;i++) {
                                                stereo_s(l,a,t_reorder[id][sfreq][l],ms_flag,is_pos,header);
                                                l++;
                                        }
                                }
                                window_len=-t_s[sfb]+t_s[++sfb];
                        }
                        while (l<576) {
                                int reorder = t_reorder[id][sfreq][l++];

                                xr[0][0][reorder]=xr[1][0][reorder]=0;
                        }
                }
        else {
                int *preflag=info->preflag[gr];

                ms_flag=find_isbound(isbound,gr,info,header);

                sfb=0; l=0;
                scalefac[0]=scalefac_l[gr][0][sfb];
                a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
                scalefac[1]=scalefac_l[gr][1][sfb];
                a[1]=fras_l(sfb,global_gain[1],scalefac_scale[1],scalefac[1],preflag[1]);
                while (l< isbound[0]) {
                        int is_pos=IS_ILLEGAL;
                        stereo_l(l,a,ms_flag,is_pos,header);
                        if (l==t_l[sfb]) {
                                sfb++;
                                scalefac[0]=scalefac_l[gr][0][sfb];
                                a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
                                scalefac[1]=scalefac_l[gr][1][sfb];
                                a[1]=fras_l(sfb,global_gain[1],scalefac_scale[1],scalefac[1],preflag[1]);
                        }
                        l++;
                }
                while (l<(MAX(non_zero[0],non_zero[1]))) {
                        int is_pos=scalefac[1];
                        if (id==1) /* MPEG1 */
                                if (is_pos==7) is_pos=IS_ILLEGAL;
                        else /* MPEG2 */
                                if (is_pos==is_max[sfb]) is_pos=IS_ILLEGAL;

                        stereo_l(l,a,ms_flag,is_pos,header);
                        if (l==t_l[sfb]) {
                                sfb++;
                                scalefac[0]=scalefac_l[gr][0][sfb];
                                scalefac[1]=scalefac_l[gr][1][sfb];
                                a[0]=fras_l(sfb,global_gain[0],scalefac_scale[0],scalefac[0],preflag[0]);
                        }
                        l++;
                }
                while (l<576) {
                        xr[0][0][l]=0;
                        xr[1][0][l]=0;
                        l++;
                }
        }
}
void alias_reduction(int ch)
{
int sb,i;

        for (sb=1;sb<32;sb++) {
                float *x = xr[ch][sb];

                for (i=0;i<8;i++) {
                        float a = x[i];
                        float b = x[-1-i];
                        x[-1-i] = b * Cs[i] - a * Ca[i];
                        x[i]    = a * Cs[i] + b * Ca[i];
                }
        }
}

#include "proto.h"

int layer3_frame(struct AUDIO_HEADER *header,int cnt)
{
static struct SIDE_INFO info;

int gr,ch,sb,i,tmp;
int mean_frame_size,bitrate,fs,hsize,ssize;

        if (header->ID)
                if (header->mode==3) {
                        nch=1;
                        hsize=21;
                } else {
                        nch=2;
                        hsize=36;
                }
        else
                if (header->mode==3) {
                        nch=1;
                        hsize=13;
                } else {
                        nch=2;
                        hsize=21;
                }

/* crc increases hsize by 2
*/
        if (header->protection_bit==0) hsize+=2;


/* read layer3 specific side_info
*/
        getinfo(header,&info);


/* MPEG2 only has one granule
*/
        bitrate=t_bitrate[header->ID][3-header->layer][header->bitrate_index];
        fs=t_sampling_frequency[header->ID][header->sampling_frequency];
        if (header->ID) mean_frame_size=144000*bitrate/fs;
        else mean_frame_size=72000*bitrate/fs;


/* check if mdb is too big for the first few frames. this means that
 * a part of the stream could be missing. We must still fill the buffer
 */
        if (info.main_data_begin > append)
                if (cnt*mean_frame_size < 960)
        {
                        //printf(" frame %d discarded, incomplete main_data\n",cnt);
                        fillbfr(mean_frame_size + header->padding_bit - hsize);
                        return 0;
                }


/* now update the data 'pointer' (counting in bits) according to
 * the main_data_begin information
 */
        data = 8 * ((append - info.main_data_begin) & (BUFFER_SIZE-1));


/* read into the buffer all bytes up to the start of next header
*/
        fillbfr(mean_frame_size + header->padding_bit - hsize);


/* these two should go away
*/
        t_l=&t_b8_l[header->ID][header->sampling_frequency][0];
        t_s=&t_b8_s[header->ID][header->sampling_frequency][0];

/* decode the scalefactors and huffman data
 * this part needs to be enhanced for error robustness
 */
        for (gr=0;gr < ((header->ID) ? 2 : 1);gr++) {
                for (ch=0;ch<nch;ch++)
        {

                        ssize=decode_scalefactors(&info,header,gr,ch);
                        decode_huffman_data(&info,gr,ch,ssize);
                }

        /* requantization, stereo processing, reordering(shortbl)
        */
                if (header->mode!=1 || (header->mode==1 && header->mode_extension==0))
                    for (ch=0;ch<nch;ch++) requantize_mono(gr,ch,&info,header);
                else 
                    requantize_ms(gr,&info,header);

        /* antialiasing butterflies
        */
                for (ch=0;ch<nch;ch++)
        {
                        if(!(info.window_switching_flag[gr][ch] && info.block_type[gr][ch]==2))
                                alias_reduction(ch);
                }

        /* just which window?
        */
                for (ch=0;ch<nch;ch++)
                {
                int win_type; /* same as in the standard, long=0, start=1 ,.... */

                        if (info.window_switching_flag[gr][ch] && info.block_type[gr][ch]==2 && info.mixed_block_flag[gr][ch])
                                win_type=0;
                        else if (!info.window_switching_flag[gr][ch]) win_type=0;
                        else win_type=info.block_type[gr][ch];

                /* imdct ...
                */
                        for (sb=0;sb<2;sb++)
                                imdct(win_type,sb,ch);

                        if (info.window_switching_flag[gr][ch] && info.block_type[gr][ch]==2 && info.mixed_block_flag[gr][ch])
                                win_type=2;

                /* no_of_imdcts tells us how many subbands from the top are all zero
                 * it is set by the requantize functions in misc2.c
                 */
                        for (sb=2;sb<no_of_imdcts[ch];sb++)
                                imdct(win_type,sb,ch);

                        /* clear s[][][] first so we don't totally blow the cache */
                        tmp = sb;
                        for (;sb<32;sb++)
                                for (i=0;i<18;i++)
                                {
                                        res[sb][i]=s[ch][sb][i];
                                        s[ch][sb][i]=0.0f;
                                }

                // polyphase filterbank
                                for (i=0;i<18;i++)
                                        poly(ch,i);
                }

                extern void sndPrintOut(void);
				sndPrintOut();
        }

        return 0;

}

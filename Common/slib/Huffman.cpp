#include "proto.h"

int huffman_decode(int tbl,int *x,int *y)
{
unsigned int chunk;
register unsigned int *h_tab;
register unsigned int lag;
register unsigned int half_lag;
int len;

        h_tab=tables[tbl];
        chunk=viewbits(19);

        h_tab += h_cue[tbl][chunk >> (19-NC_O)];
        len=(*h_tab>>8)&0x1f;

        /* check for an immediate hit, so we can decode those short codes very fast
        */
        if ((*h_tab>>(32-len)) != (chunk>>(19-len))) {
                if (chunk >> (19-NC_O) < N_CUE-1)
                  lag=(h_cue[tbl][(chunk >> (19-NC_O))+1] -
                       h_cue[tbl][chunk >> (19-NC_O)]);
                else {
                        /* we strongly depend on h_cue[N_CUE-1] to point to
                         * the last entry in the huffman table, so we should
                         * not get here anyway. if it didn't, we'd have to
                         * have another table with huffman tables lengths, and
                         * it would be a mess. just in case, scream&shout.
                         */
//                        printf(" h_cue clobbered. this is a bug. blip.\n");
                        return (-1);
                }
                chunk <<= 32-19;
                chunk |= 0x1ff;

                half_lag = lag >> 1;

                h_tab += half_lag;
                lag -= half_lag;

                while (lag > 1) {
                        half_lag = lag >> 1;

                        if (*h_tab < chunk)
                                h_tab += half_lag;
                        else
                                h_tab -= half_lag;

                        lag -= half_lag;
                }

                len=(*h_tab>>8)&0x1f;
                if ((*h_tab>>(32-len)) != (chunk>>(32-len))) {
                        if (*h_tab > chunk)
                                h_tab--;
                        else
                                h_tab++;

                        len=(*h_tab>>8)&0x1f;
                }
        }
        sackbits(len);
        *x=(*h_tab>>4)&0xf;
        *y=*h_tab&0xf;
        return len;
}

/**********
 * Copyright (c) 2017, Xilinx, Inc.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * **********/
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "hls_stream.h"
#include <ap_int.h>
#include "xil_decompress_engine.h"

#define MAX_OFFSET 65536 
#define HISTORY_SIZE MAX_OFFSET

#define BIT 8
#define READ_STATE 0
#define MATCH_STATE 1
#define LOW_OFFSET_STATE 4 
#define LOW_OFFSET 8 // This should be bigger than Pipeline Depth to handle inter dependency false case

typedef ap_uint<PARALLEL_BIT> uintV_t;
typedef ap_uint<GMEM_DWIDTH> uintMemWidth_t;
typedef ap_uint<32> compressd_dt;
typedef ap_uint<16> offset_dt;

#define GET_DIFF_IF_BIG(x,y)   (x>y)?(x-y):0

#if (D_COMPUTE_UNIT == 1)
namespace  dec_cu1 
#elif (D_COMPUTE_UNIT == 2)
namespace  dec_cu2 
#endif
{
void lz4_core(
        hls::stream<uintMemWidth_t> &inStreamMemWidth, 
        hls::stream<uint32_t> &outStreamMemWidthSize, 
        const uint32_t _input_size,
        const uint32_t _output_size
        )
{
    uint32_t input_size = _input_size;
    uint32_t output_size = _output_size;
    uint32_t input_size1  = input_size;
    uint32_t output_size1 = output_size;
    hls::stream<uintV_t>        instreamV("instreamV"); 
    hls::stream<uint32_t>       litlenStream("litlenStream"); 
    hls::stream<uintV_t>        litStream("litStream"); 
    hls::stream<offset_dt>      offsetStream("offsetStream"); 
    hls::stream<uint32_t>       matchlenStream("matchlenStream"); 
    hls::stream<uintV_t>        decompressed_stream("decompressed_stream");
    #pragma HLS STREAM variable=instreamV               depth=32
    #pragma HLS STREAM variable=litlenStream            depth=32
    #pragma HLS STREAM variable=litStream               depth=32
    #pragma HLS STREAM variable=offsetStream            depth=32
    #pragma HLS STREAM variable=matchlenStream          depth=32
    #pragma HLS STREAM variable=decompressed_stream     depth=32
    #pragma HLS RESOURCE variable=instreamV             core=FIFO_SRL
    #pragma HLS RESOURCE variable=litlenStream          core=FIFO_SRL
    #pragma HLS RESOURCE variable=litStream             core=FIFO_SRL
    #pragma HLS RESOURCE variable=offsetStream          core=FIFO_SRL
    #pragma HLS RESOURCE variable=matchlenStream        core=FIFO_SRL
    #pragma HLS RESOURCE variable=decompressed_stream   core=FIFO_SRL

    #pragma HLS dataflow 
    stream_downsizer<uint32_t, GMEM_DWIDTH, PARALLEL_BIT>(inStreamMemWidth,instreamV,input_size); 
    lz4_decompressr(instreamV, litlenStream, litStream, offsetStream, matchlenStream, input_size1); 
    lz_decompress<HISTORY_SIZE,READ_STATE,MATCH_STATE,LOW_OFFSET_STATE,LOW_OFFSET>(litlenStream, litStream, offsetStream, matchlenStream, decompressed_stream, output_size); 
    stream_upsizer_decompress<uint32_t, PARALLEL_BIT, GMEM_DWIDTH>(decompressed_stream, outStreamMemWidthSize, output_size1);
}

void lz4_dec(
                const uintMemWidth_t *in,      
                const uint32_t  input_idx[PARALLEL_BLOCK],
                const uint32_t  input_size[PARALLEL_BLOCK],
                const uint32_t  output_size[PARALLEL_BLOCK],
                const uint32_t  input_size1[PARALLEL_BLOCK],
                const uint32_t  output_size1[PARALLEL_BLOCK],
                uint32_t        *uncompress_outSize
                )
{
    hls::stream<uintMemWidth_t> inStreamMemWidth_0("inStreamMemWidth_0"); 
    hls::stream<uintMemWidth_t> outStreamMemWidth_0("outStreamMemWidth_0"); 
    hls::stream<uint32_t> outStreamMemWidthSize_0("outStreamMemWidthSize_0"); 
    #pragma HLS STREAM variable=inStreamMemWidth_0 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_0 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_0 depth=c_gmem_burst_size
    #pragma HLS RESOURCE variable=inStreamMemWidth_0  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_0 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_0 core=FIFO_SRL
#if PARALLEL_BLOCK > 1
    hls::stream<uintMemWidth_t> inStreamMemWidth_1("inStreamMemWidth_1");
    hls::stream<uintMemWidth_t> outStreamMemWidth_1("outStreamMemWidth_1"); 
    hls::stream<uint32_t> outStreamMemWidthSize_1("outStreamMemWidthSize_1"); 
    #pragma HLS STREAM variable=inStreamMemWidth_1 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_1 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_1 depth=c_gmem_burst_size
	#pragma HLS RESOURCE variable=inStreamMemWidth_1  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_1 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_1 core=FIFO_SRL
#endif 
#if PARALLEL_BLOCK > 2
    hls::stream<uintMemWidth_t> inStreamMemWidth_2("inStreamMemWidth_2"); 
    hls::stream<uintMemWidth_t> inStreamMemWidth_3("inStreamMemWidth_3"); 
    hls::stream<uintMemWidth_t> outStreamMemWidth_2("outStreamMemWidth_2"); 
    hls::stream<uintMemWidth_t> outStreamMemWidth_3("outStreamMemWidth_3"); 
    hls::stream<uint32_t> outStreamMemWidthSize_2("outStreamMemWidthSize_2"); 
    hls::stream<uint32_t> outStreamMemWidthSize_3("outStreamMemWidthSize_3"); 
    #pragma HLS STREAM variable=inStreamMemWidth_2 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=inStreamMemWidth_3 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_2 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_3 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_2 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_3 depth=c_gmem_burst_size
	#pragma HLS RESOURCE variable=inStreamMemWidth_2  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_2 core=FIFO_SRL
    #pragma HLS RESOURCE variable=inStreamMemWidth_3  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_3 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_2 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_3 core=FIFO_SRL
#endif
#if PARALLEL_BLOCK > 4
    hls::stream<uintMemWidth_t> inStreamMemWidth_4("inStreamMemWidth_4"); 
    hls::stream<uintMemWidth_t> inStreamMemWidth_5("inStreamMemWidth_5"); 
    hls::stream<uintMemWidth_t> inStreamMemWidth_6("inStreamMemWidth_6"); 
    hls::stream<uintMemWidth_t> inStreamMemWidth_7("inStreamMemWidth_7"); 
    hls::stream<uintMemWidth_t> outStreamMemWidth_4("outStreamMemWidth_4"); 
    hls::stream<uintMemWidth_t> outStreamMemWidth_5("outStreamMemWidth_5"); 
    hls::stream<uintMemWidth_t> outStreamMemWidth_6("outStreamMemWidth_6"); 
    hls::stream<uintMemWidth_t> outStreamMemWidth_7("outStreamMemWidth_7"); 
    hls::stream<uint32_t> outStreamMemWidthSize_4("outStreamMemWidthSize_4"); 
    hls::stream<uint32_t> outStreamMemWidthSize_5("outStreamMemWidthSize_5"); 
    hls::stream<uint32_t> outStreamMemWidthSize_6("outStreamMemWidthSize_6"); 
    hls::stream<uint32_t> outStreamMemWidthSize_7("outStreamMemWidthSize_7"); 
    #pragma HLS STREAM variable=inStreamMemWidth_4 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=inStreamMemWidth_5 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=inStreamMemWidth_6 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=inStreamMemWidth_7 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_4 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_5 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_6 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidth_7 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_4 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_5 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_6 depth=c_gmem_burst_size
    #pragma HLS STREAM variable=outStreamMemWidthSize_7 depth=c_gmem_burst_size

    #pragma HLS RESOURCE variable=inStreamMemWidth_4  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_4 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_4 core=FIFO_SRL
	
    #pragma HLS RESOURCE variable=inStreamMemWidth_5  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_5 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_5 core=FIFO_SRL
	
    #pragma HLS RESOURCE variable=inStreamMemWidth_6  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_6 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_6 core=FIFO_SRL
	
    #pragma HLS RESOURCE variable=inStreamMemWidth_7  core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidth_7 core=FIFO_SRL
    #pragma HLS RESOURCE variable=outStreamMemWidthSize_7 core=FIFO_SRL
#endif
    #pragma HLS dataflow    
    // Transfer data from global memory to kernel
    mm2s<GMEM_DWIDTH, GMEM_BURST_SIZE>(in,
                                       input_idx,
                                       inStreamMemWidth_0,
#if PARALLEL_BLOCK > 1
                                       inStreamMemWidth_1,
#endif
#if PARALLEL_BLOCK > 2
                                       inStreamMemWidth_2,
                                       inStreamMemWidth_3,
#endif
#if PARALLEL_BLOCK > 4
                                       inStreamMemWidth_4,
                                       inStreamMemWidth_5,
                                       inStreamMemWidth_6,
                                       inStreamMemWidth_7,
#endif
                                       input_size
                                      );
   lz4_core(inStreamMemWidth_0, outStreamMemWidthSize_0, input_size1[0],output_size1[0]); 
#if PARALLEL_BLOCK > 1
   lz4_core(inStreamMemWidth_1, outStreamMemWidthSize_1, input_size1[1],output_size1[1]); 
#endif
#if PARALLEL_BLOCK > 2
   lz4_core(inStreamMemWidth_2, outStreamMemWidthSize_2, input_size1[2],output_size1[2]); 
   lz4_core(inStreamMemWidth_3, outStreamMemWidthSize_3, input_size1[3],output_size1[3]); 
#endif
#if PARALLEL_BLOCK > 4
   lz4_core(inStreamMemWidth_4, outStreamMemWidthSize_4, input_size1[4],output_size1[4]); 
   lz4_core(inStreamMemWidth_5, outStreamMemWidthSize_5, input_size1[5],output_size1[5]); 
   lz4_core(inStreamMemWidth_6, outStreamMemWidthSize_6, input_size1[6],output_size1[6]); 
   lz4_core(inStreamMemWidth_7, outStreamMemWidthSize_7, input_size1[7],output_size1[7]); 
#endif
    // Transfer data from kernel to global memory
   s2mm_decompress<uint32_t, GMEM_BURST_SIZE, GMEM_DWIDTH>(
                                                 outStreamMemWidthSize_0,
#if PARALLEL_BLOCK > 1
                                                 outStreamMemWidthSize_1,
#endif
#if PARALLEL_BLOCK > 2
                                                 outStreamMemWidthSize_2,
                                                 outStreamMemWidthSize_3,
#endif
#if PARALLEL_BLOCK > 4
                                                 outStreamMemWidthSize_4,
                                                 outStreamMemWidthSize_5,
                                                 outStreamMemWidthSize_6,
                                                 outStreamMemWidthSize_7,
#endif
                                                 uncompress_outSize
                                                );

}

}//end of namepsace

extern "C" {
#if (D_COMPUTE_UNIT == 1)
void xil_lz4_dec_cu1
#elif (D_COMPUTE_UNIT == 2)
void xil_lz4_dec_cu2
#endif
(
                const uintMemWidth_t *in,      
                uint32_t        *unCompressSize, 
                uint32_t        *in_block_size,
                uint32_t        *in_compress_size,
                uint32_t        block_size_in_kb,                     
                uint32_t        no_blocks
                )
{
    #pragma HLS INTERFACE m_axi port=in offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=unCompressSize offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=in_block_size offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=in_compress_size offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=in bundle=control
    #pragma HLS INTERFACE s_axilite port=unCompressSize bundle=control
    #pragma HLS INTERFACE s_axilite port=in_block_size bundle=control
    #pragma HLS INTERFACE s_axilite port=in_compress_size bundle=control
    #pragma HLS INTERFACE s_axilite port=block_size_in_kb bundle=control
    #pragma HLS INTERFACE s_axilite port=no_blocks bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    #pragma HLS data_pack variable=in
    uint32_t max_block_size = block_size_in_kb * 1024;
    uint32_t compress_size[PARALLEL_BLOCK];
    uint32_t uncompress_size[PARALLEL_BLOCK];
    uint32_t compress_size1[PARALLEL_BLOCK];
    uint32_t block_size[PARALLEL_BLOCK];
    uint32_t block_size1[PARALLEL_BLOCK];
    uint32_t input_idx[PARALLEL_BLOCK];
    #pragma HLS ARRAY_PARTITION variable=input_idx dim=0 complete
    #pragma HLS ARRAY_PARTITION variable=compress_size dim=0 complete
    #pragma HLS ARRAY_PARTITION variable=uncompress_size dim=0 complete
    #pragma HLS ARRAY_PARTITION variable=compress_size1 dim=0 complete
    #pragma HLS ARRAY_PARTITION variable=block_size dim=0 complete
    #pragma HLS ARRAY_PARTITION variable=block_size1 dim=0 complete

    //printf ("In decode compute unit %d no_blocks %d\n", D_COMPUTE_UNIT, no_blocks);   
    uint32_t dec_size = 0;
    *unCompressSize = 0;
 
    for (int i = 0; i < no_blocks; i+=PARALLEL_BLOCK) {
        
        int nblocks = PARALLEL_BLOCK;
        if((i + PARALLEL_BLOCK) > no_blocks) {
            nblocks = no_blocks - i;
        }

        for (int j = 0; j < PARALLEL_BLOCK; j++) {
            if(j < nblocks) {
                uint32_t iSize = in_compress_size[i + j];
                uint32_t oSize = in_block_size[i + j];
                //printf("iSize %d oSize %d \n", iSize, oSize);
                compress_size[j] = iSize;
                block_size[j]  = oSize;
                compress_size1[j] = iSize;
                block_size1[j]  = oSize;
                input_idx[j]   = (i + j) * max_block_size;
            } else  {
                compress_size[j] = 0;
                block_size[j]  = 0;
                compress_size1[j] = 0;
                block_size1[j]  = 0;
                input_idx[j]   = 0;
            }
            uncompress_size[j] = 0;
        }
#if (D_COMPUTE_UNIT == 1)
        dec_cu1::lz4_dec(in, input_idx, compress_size, block_size, compress_size1, block_size1, unCompressSize);
#elif (D_COMPUTE_UNIT == 2)
        dec_cu2::lz4_dec(in, input_idx, compress_size, block_size, compress_size1, block_size1, unCompressSize);
#endif
    }
}
}

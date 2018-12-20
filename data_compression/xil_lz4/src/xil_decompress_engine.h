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
#include "stream_utils.h"
#include "lz_compress.h"
#include <ap_int.h>
#include "xil_lz4_config.h"

typedef ap_uint<32> compressd_dt;

#define BIT 8

//LZ4 Decompress states
#define READ_TOKEN      0
#define READ_LIT_LEN    1
#define READ_LITERAL    2
#define READ_OFFSET     3
#define READ_MATCH_LEN  4

inline void lz4_decompressr(
        hls::stream<ap_uint<PARALLEL_BIT> > &inStream,          
        hls::stream<uint32_t > &litlenStream,          
        hls::stream<ap_uint<PARALLEL_BIT> > &litStream,          
        hls::stream<ap_uint<16> >&offsetStream,          
        hls::stream<uint32_t >&matchlenStream,          
        uint32_t input_size
    )
{
    if (input_size == 0) return;

    uint8_t next_state = READ_TOKEN;
    uint8_t token_match_len = 0;
    uint8_t token_lit_len = 0;
    uint8_t input_index = 0;
    int8_t output_index = 0;
    uint32_t lit_len = 0;
    uint32_t match_len = 0;
    uint32_t out_written = 0;
    bool outFlag;

    ap_uint<16> offset;
    ap_uint<PARALLEL_BIT> outStreamValue;
    ap_uint<PARALLEL_BIT> inValue;

    ap_uint<3*PARALLEL_BYTE*8> input_window;   
    ap_uint<8> output_window[2*PARALLEL_BYTE];
    #pragma HLS ARRAY_PARTITION variable=output_window complete

    // Pre-read two data from the stream (two based on the READ_TOKEN)
    for (int i = 0; i < 2; i++){
    #pragma HLS PIPELINE II=1
        inValue = inStream.read();
        for (int j = 0; j < PARALLEL_BYTE; j++){
        #pragma HLS LOOP UNROLL
            input_window.range((((i*PARALLEL_BYTE)+j)+1)*BIT - 1, ((i*PARALLEL_BYTE)+j)*BIT) = inValue.range((j+1)*BIT - 1, j*BIT);
        }
    }

    // Initialize the loop terminate variable to input_window buffer size as
    // that much data is already read from stream
    uint32_t terminate = 2 * PARALLEL_BYTE;

lz4_decompressr: for (; (terminate < input_size); ){
    #pragma HLS PIPELINE II=1
        //  then shift the input buffer data and read PARALLEL_BIT data 
        //  from inStream and put the data in input buffer
        //printf("input_index=%d, output_index=%d\n",input_index,output_index);
        if (input_index >= PARALLEL_BYTE){
            uint8_t shift, copy;
            for(shift = 0; shift < PARALLEL_BYTE; shift++){
            #pragma HLS LOOP UNROLL
                input_window.range((shift+1)*BIT - 1, shift*BIT) = input_window.range(((PARALLEL_BYTE+shift)+1)*BIT - 1, (PARALLEL_BYTE+shift)*BIT);               
            }

            ap_uint<PARALLEL_BIT> input = inStream.read();
            terminate += PARALLEL_BYTE;

            for (copy = 0; copy < PARALLEL_BYTE; copy++){
            #pragma HLS LOOP UNROLL
                input_window.range(((copy+shift)+1)*BIT - 1, (copy+shift)*BIT) = input.range((copy+1)*BIT - 1, copy*BIT);
            }
            input_index = input_index - PARALLEL_BYTE;
        }
       
        //READ TOKEN stage
        if (next_state == READ_TOKEN){
            ap_uint<BIT> token_value = input_window.range((input_index+1)*BIT -1, input_index*BIT);
            token_lit_len     = token_value.range(7,4);
            token_match_len   = token_value.range(3,0);
            bool c0  = (token_lit_len == 0xF);
            input_index +=1;
            lit_len = token_lit_len;

            if(c0){
                next_state = READ_LIT_LEN;
            }else if (lit_len){
                next_state = READ_LITERAL;
                litlenStream << lit_len;
            }else{
                next_state = READ_OFFSET;
                litlenStream << lit_len;
            }
            //printf("TokenLitlen: %d\n",lit_len);
        }else if (next_state == READ_LIT_LEN){
            ap_uint<BIT> token_value = input_window.range((input_index+1)*BIT -1, input_index*BIT);
            input_index += 1;
            lit_len += token_value;

            if (token_value == 0xFF){
                next_state = READ_LIT_LEN;
            }
            else{
                next_state = READ_LITERAL;
                litlenStream << lit_len;
            }
            //printf("Litlen: %d, next_state: %d\n",lit_len,next_state);
        }else if (next_state  == READ_LITERAL){
                for (uint8_t read = 0; read < PARALLEL_BYTE; read++){
                #pragma HLS LOOP UNROLL
                    output_window[output_index+read] = input_window.range(((read+input_index)+1)*BIT -1, (read+input_index)*BIT);
                }
                if(lit_len>=PARALLEL_BYTE){
                    output_index += PARALLEL_BYTE;
                    input_index +=PARALLEL_BYTE;
                    lit_len -= PARALLEL_BYTE;
                }else{
                    output_index += lit_len;
                    input_index +=lit_len;
                    lit_len = 0;
                }
                if(lit_len == 0){
                    next_state = READ_OFFSET;
                }else{
                    next_state =READ_LITERAL;
                }
                //printf("READ_LITERAL \t lit_len:%d \n",lit_len);
        }else if (next_state == READ_OFFSET){
                offset.range(7,0) = input_window.range(((input_index)+1)*BIT -1, (input_index)*BIT);
                offset.range(15,8) = input_window.range(((input_index+1)+1)*BIT -1, (input_index+1)*BIT);
                bool c0  = (token_match_len == 0xF);
                input_index += 2;
                match_len = token_match_len+4; //+4 because of LZ4 standard
                offsetStream << offset; 

                if(c0){
                    next_state = READ_MATCH_LEN;
                }
                else{
                    next_state = READ_TOKEN;
                    matchlenStream << match_len; 
                }
                assert(offset>=40); //limited the min offset to 16 currently
                //printf("TokenMatchLen: %d\n",match_len);
        }else if (next_state == READ_MATCH_LEN){
            ap_uint<BIT> token_value = input_window.range((input_index+1)*BIT -1, input_index*BIT);
            input_index += 1;
            match_len += token_value;

            if (token_value == 0xFF){
                next_state = READ_MATCH_LEN;
            }
            else{
                next_state = READ_TOKEN;
                matchlenStream << match_len;
            }
            //printf("MatchLen: %d next_state: %d\n",match_len,next_state);
        }else{
            assert(0);
        }

        // If output written to buffer is more than or equals PARALLEL_BYTE
        //  then wite PARALLEL_BIT data to outStream and shift the 
        //  output buffer data
        if (output_index >= PARALLEL_BYTE){
            for (uint8_t k = 0; k < PARALLEL_BYTE; k++){
            #pragma HLS LOOP UNROLL
                outStreamValue.range((k+1)*BIT - 1, k*BIT) = output_window[k];
            }
            for(uint8_t shift = 0; shift < PARALLEL_BYTE; shift++){
            #pragma HLS LOOP UNROLL
                output_window[shift] = output_window[PARALLEL_BYTE+shift];
            }
            output_index = output_index - PARALLEL_BYTE;
            outFlag=true;
        }else if (output_index && (next_state == READ_OFFSET)){
            for (uint8_t k = 0; k < PARALLEL_BYTE; k++){
            #pragma HLS LOOP UNROLL
                outStreamValue.range((k+1)*BIT - 1, k*BIT) = output_window[k];
            }
            output_index = 0;
            outFlag=true;
        }else{
            outFlag=false;
        }
        if(outFlag){
            litStream << outStreamValue;
            out_written += PARALLEL_BYTE;
        }
    }

    for (uint8_t read = 0; read < PARALLEL_BYTE; read++){
    #pragma HLS LOOP UNROLL
        output_window[output_index+read] = input_window.range(((read+input_index)+1)*BIT -1, (read+input_index)*BIT);
    }
    output_index += lit_len;
    input_index +=lit_len;
    

    // Write out if there is remaining left over data in input buffer
    // to outStream
    if (lit_len){
        for (uint8_t k = 0; k < PARALLEL_BYTE; k++){
        #pragma HLS LOOP UNROLL
            outStreamValue.range((k+1)*BIT - 1, k*BIT) = output_window[k];
        }
        litStream << outStreamValue;
    }
    //printf("\nInIdx: %d \t outIdx: %d \t Input_size: %d \t read_from_stream: %d  \t written_to_stream: %d \t output_count: %d\n",input_index, output_index,input_size,terminate, out_written, output_count);                       
}

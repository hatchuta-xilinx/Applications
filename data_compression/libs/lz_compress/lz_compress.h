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
#pragma once
#include "hls_stream.h"
#include <ap_int.h>

typedef ap_uint<32> compressd_dt;
#define PARALLEL_BYTE 8
#define PARALLEL_BIT (PARALLEL_BYTE*8)

#define BUFF_DEPTH (64/PARALLEL_BYTE)/2
#define BUFFER_SIZE (BUFF_DEPTH*1024)
//LZ Decompress states
#define READ_LIT_LEN 0
#define WRITE_LITERAL 1
#define READ_OFFSET 2
#define READ_MATCH 3

template <int MATCH_LEN, int MATCH_LEVEL , int LZ_DICT_SIZE, int BIT, int MIN_OFFSET, int MIN_MATCH, int LZ_MAX_OFFSET_LIMIT>
void lz_compress(
        hls::stream<ap_uint<BIT> > &inStream,          
        hls::stream<compressd_dt> &outStream,          
        uint32_t input_size,
        uint32_t left_bytes
        )
{
    const int DICT_ELE_WIDTH = (MATCH_LEN*BIT + 24);
    typedef ap_uint< MATCH_LEVEL * DICT_ELE_WIDTH> uintDictV_t;
    typedef ap_uint<DICT_ELE_WIDTH> uintDict_t;

    if(input_size == 0) return;      
    //Dictionary
    uintDictV_t dict[LZ_DICT_SIZE];
    #pragma HLS RESOURCE variable=dict core=XPM_MEMORY uram

    uintDictV_t resetValue=0;
    for (int i = 0 ; i < MATCH_LEVEL; i++){
        #pragma HLS UNROLL 
        resetValue.range((i+1)*DICT_ELE_WIDTH -1, i*DICT_ELE_WIDTH + MATCH_LEN*BIT) = -1;
    }
    //Initialization of Dictionary
    dict_flush:for (int i  =0 ; i < LZ_DICT_SIZE; i++){
        #pragma HLS PIPELINE II=1
        #pragma HLS UNROLL FACTOR=2
        dict[i] = resetValue;
   }

    uint8_t present_window[MATCH_LEN];
    #pragma HLS ARRAY_PARTITION variable=present_window complete 
    for (uint8_t i = 1 ; i < MATCH_LEN; i++){
        present_window[i] = inStream.read();
    }
    lz_compress:for (uint32_t i = MATCH_LEN-1; i < input_size -left_bytes; i++)
              {
    #pragma HLS PIPELINE II=1
    #pragma HLS dependence variable=dict inter false
        uint32_t currIdx = i - MATCH_LEN +1;
        //shift present window and load next value
        for(int m = 0 ; m < MATCH_LEN -1; m++){
            #pragma HLS UNROLL
            present_window[m] = present_window[m+1];
        }
        present_window[MATCH_LEN-1] = inStream.read();

        //Calculate Hash Value
        uint32_t hash = (present_window[0] << 4) ^
                        (present_window[1] << 3) ^
                        (present_window[2] << 3) ^
                        (present_window[3]);

        //Dictionary Lookup
        uintDictV_t dictReadValue = dict[hash];
        uintDictV_t dictWriteValue = dictReadValue << DICT_ELE_WIDTH;
        for(int m = 0 ; m < MATCH_LEN ; m++){
            #pragma HLS UNROLL
            dictWriteValue.range((m+1)*BIT-1,m*BIT) = present_window[m];
        }
        dictWriteValue.range(DICT_ELE_WIDTH -1, MATCH_LEN*BIT) = currIdx;
        //Dictionary Update
        dict[hash] = dictWriteValue;

        //Match search and Filtering
        // Comp dict pick
        uint8_t match_length=0;
        uint32_t match_offset=0;
        for (int l = 0 ; l < MATCH_LEVEL ; l++){
            uint8_t len = 0;
            bool done = 0;
            uintDict_t compareWith = dictReadValue.range((l+1)*DICT_ELE_WIDTH-1, l*DICT_ELE_WIDTH);
            uint32_t compareIdx = compareWith.range(DICT_ELE_WIDTH-1, MATCH_LEN*BIT);
            for (int m = 0; m < MATCH_LEN; m++){
                if (present_window[m] == compareWith.range((m+1)*BIT-1,m*BIT) && !done){
                    len++;
                }else{
                    done = 1;
                }
            }
            if ((len >= MIN_MATCH)&& (currIdx > compareIdx) && ((currIdx -compareIdx) < LZ_MAX_OFFSET_LIMIT) && ((currIdx - compareIdx - 1) >= MIN_OFFSET)){
                len = len;
            }else{
                len = 0;
            }
            if (len > match_length){
                match_length = len;
                match_offset = currIdx -compareIdx - 1;
            }
        }
        compressd_dt outValue = 0;
        outValue.range(7,0)     = present_window[0];
        outValue.range(15,8)    = match_length;
        outValue.range(31,16)   = match_offset;
        outStream << outValue;
    }
    lz_compress_leftover:for (int m = 1 ; m < MATCH_LEN ; m++){
        #pragma HLS PIPELINE
        compressd_dt outValue = 0;
        outValue.range(7,0)     = present_window[m];
        outStream << outValue;
    }
    lz_left_bytes:for (int l = 0 ; l < left_bytes ; l++){
        #pragma HLS PIPELINE
        compressd_dt outValue = 0;
        outValue.range(7,0)     = inStream.read();
        outStream << outValue;
    }
}

template<int MATCH_LEN, int OFFSET_WINDOW>
void lz_bestMatchFilter(
        hls::stream<compressd_dt> &inStream,
        hls::stream<compressd_dt> &outStream,
        uint32_t input_size , uint32_t left_bytes
        )        
{
     
    const int c_max_match_length = MATCH_LEN; 
    if(input_size == 0) return;

    compressd_dt compare_window[MATCH_LEN];
    #pragma HLS array_partition variable=compare_window

    //Initializing shift registers
    for(uint32_t i = 0 ; i < c_max_match_length; i++){
        #pragma HLS UNROLL
        compare_window[i] = inStream.read();
    }

    lz_bestMatchFilter:for (uint32_t i = c_max_match_length; i < input_size ; i++){
        #pragma HLS PIPELINE II=1
        //shift register logic
        compressd_dt outValue = compare_window[0];
        for(uint32_t j = 0 ; j < c_max_match_length-1;j++){
            #pragma HLS UNROLL
            compare_window[j] = compare_window[j+1];
        }
        compare_window[c_max_match_length-1] = inStream.read();
        
        uint8_t match_length = outValue.range(15,8);
        bool best_match = 1;
        //Find Best match 
        for(uint32_t j = 0; j < c_max_match_length ; j++){
            compressd_dt compareValue = compare_window[j];
            uint8_t compareLen = compareValue.range(15,8);
            if ( match_length+j < compareLen){
                best_match = 0;
            }
        }
        if(best_match == 0){
            outValue.range(15,8) = 0;
            outValue.range(31,16) = 0;
        }
        outStream << outValue;
    }

    lz_bestMatchFilter_left_over:    
    for (uint32_t i = 0 ; i < c_max_match_length; i++){
        outStream << compare_window[i];
    }
}


template<int MAX_MATCH_LEN, int OFFSET_WINDOW>
void lz_booster(
        hls::stream<compressd_dt> &inStream,       
        hls::stream<compressd_dt> &outStream,       
        uint32_t input_size, uint32_t left_bytes
)
{

    if(input_size == 0) return;
    uint8_t local_mem[OFFSET_WINDOW];
    uint32_t match_loc = 0;
    uint32_t match_len =0;
    compressd_dt outValue;
    compressd_dt outStreamValue;
    bool matchFlag=false;
    bool outFlag = false;
    lz_booster:for (uint32_t i = 0 ; i < (input_size-left_bytes); i++){
        #pragma HLS PIPELINE II=1 
        #pragma HLS dependence variable=local_mem inter false
        compressd_dt inValue = inStream.read();
        uint8_t tCh      = inValue.range(7,0);
        uint8_t tLen     = inValue.range(15,8);
        uint16_t tOffset = inValue.range(31,16);
        uint8_t match_ch = local_mem[match_loc%OFFSET_WINDOW];
        local_mem[i%OFFSET_WINDOW] = tCh;
        outFlag = false;
        if (    matchFlag 
                && (match_len< MAX_MATCH_LEN) 
                && (tCh == match_ch) 
           ){
                match_len++;
                match_loc++;
                outValue.range(15,8) = match_len;
        }else{
            match_len = 1;
            match_loc = i - tOffset;
            if (i) outFlag = true;
            outStreamValue = outValue;
            outValue = inValue;
            if(tLen){
                matchFlag = true;
            }else{
                matchFlag =false;
            }
        }
        if(outFlag) outStream << outStreamValue;

    }
    outStream << outValue;
    lz_booster_left_bytes:
    for (uint32_t i = 0 ; i < left_bytes ; i++){
        outStream << inStream.read();
    }
}
static void lz_filter(
        hls::stream<compressd_dt> &inStream,       
        hls::stream<compressd_dt> &outStream,       
        uint32_t input_size, uint32_t left_bytes
)
{
    if(input_size == 0) return;
    uint32_t skip_len =0;
    lz_filter:for (uint32_t i = 0 ; i < input_size-left_bytes; i++){
        #pragma HLS PIPELINE II=1 
        compressd_dt inValue = inStream.read();
        uint8_t   tLen     = inValue.range(15,8);
        if (skip_len){
            skip_len--;
        }else{
            outStream << inValue;
            if(tLen)skip_len = tLen-1;
        }
    }
    lz_filter_left_bytes:
    for (uint32_t i = 0 ; i < left_bytes ; i++){
        outStream << inStream.read();
    }
}

template<int HISTORY_SIZE, int READ_STATE, int MATCH_STATE, int LOW_OFFSET_STATE, int LOW_OFFSET>
void lz_decompress(
        hls::stream<uint32_t> &litlenStream,          
        hls::stream<ap_uint<PARALLEL_BIT> >&litStream,          
        hls::stream<ap_uint<16> >&offsetStream,          
        hls::stream<uint32_t> &matchlenStream,          
        hls::stream<ap_uint<PARALLEL_BIT> > &outStream,          
        uint32_t original_size
    )
{
    if(original_size == 0) return;

    uint32_t offset_buff_size = LOW_OFFSET/PARALLEL_BYTE;
    ap_uint<PARALLEL_BIT> local_buf_even[BUFFER_SIZE];
    #pragma HLS RESOURCE variable=local_buf_even core=XPM_MEMORY uram
    #pragma HLS dependence variable=local_buf_even inter false
    ap_uint<PARALLEL_BIT> local_buf_odd[BUFFER_SIZE];
    #pragma HLS RESOURCE variable=local_buf_odd core=XPM_MEMORY uram
    #pragma HLS dependence variable=local_buf_odd inter false
    
    uint32_t total_cntr = 0; 
    uint32_t outCntr = 0;
    uint32_t lit_len = 0; 
    uint32_t output_cnt = 0;
    uint32_t match_loc = 0;
    uint32_t match_len=0;
    uint32_t byte_loc = 0;
    uint32_t even_write_buff_ind = 0;
    uint32_t odd_write_buff_ind = 0;
    uint32_t read_buff_ind = 0;
    uint32_t terminate = 0;
    uint32_t out_size = 0;
    uint32_t output_index = 0;
    uint32_t offset_incr = 0;

    uint8_t next_state = READ_LIT_LEN;
    int8_t incr_output_index = 0;
    bool even_cntr = true;
    bool outStreamFlag = false;
    
    ap_uint<16>offset = 0;
    ap_uint<PARALLEL_BIT> outStreamValue = 0;
    ap_uint<2*PARALLEL_BYTE*8> output_window;
    
    terminate = (original_size/PARALLEL_BYTE)*PARALLEL_BYTE;       

    lz_decompress:for( ;((out_size+PARALLEL_BYTE) < terminate) || next_state==WRITE_LITERAL; ) {
    #pragma HLS PIPELINE II=1

        if (outStreamFlag) out_size += PARALLEL_BYTE;
 
        if (next_state == READ_LIT_LEN){
            incr_output_index = 0;
            lit_len = litlenStream.read();
            if (lit_len){
                next_state = WRITE_LITERAL;
            }else{
                next_state = READ_OFFSET;
            }
            output_cnt += lit_len;
        }else if (next_state == WRITE_LITERAL){
            ap_uint<PARALLEL_BIT> input = litStream.read();
            for (int read = 0; read < PARALLEL_BYTE; read++){
            #pragma HLS LOOP UNROLL
                output_window.range(((read+output_index)+1)*8 -1, (read+output_index)*8) = input.range(((read)+1)*8 -1, (read)*8);
            }
            if(lit_len >= PARALLEL_BYTE){
                incr_output_index = PARALLEL_BYTE;
                lit_len -= PARALLEL_BYTE;
            }else{
                incr_output_index = lit_len;
                lit_len = 0;
            }
            if (lit_len == 0){
                next_state = READ_OFFSET;
            }else{
                next_state = WRITE_LITERAL;
            }          
        }else if (next_state == READ_OFFSET){
            incr_output_index = 0;
            offset = offsetStream.read();
            match_loc = output_cnt - offset;
            match_len = matchlenStream.read();
            output_cnt += match_len;
            next_state = READ_MATCH;
        }else if (next_state == READ_MATCH){
            read_buff_ind = match_loc/(2*PARALLEL_BYTE);
            byte_loc = match_loc%(2*PARALLEL_BYTE);
            
            // Read two data from the local buffer since the data to access can go to
            // consecutive local buffer index 
            ap_uint<2*PARALLEL_BIT> localValue;
            uint32_t even_idx = 0;
            uint32_t odd_idx = read_buff_ind % BUFFER_SIZE;
            if (byte_loc < PARALLEL_BYTE){
                even_idx  = read_buff_ind % BUFFER_SIZE;
            }else{
                even_idx  = (read_buff_ind+1) % BUFFER_SIZE;
            }
            ap_uint<PARALLEL_BIT> even_temp, odd_temp;
            even_temp.range(PARALLEL_BIT-1,0) = local_buf_even[even_idx];
            odd_temp.range(PARALLEL_BIT-1,0) = local_buf_odd[odd_idx];

            if (byte_loc < PARALLEL_BYTE){
                localValue.range(PARALLEL_BIT-1,0) = even_temp.range(PARALLEL_BIT-1, 0);
                localValue.range((2*PARALLEL_BIT)-1,PARALLEL_BIT) = odd_temp.range(PARALLEL_BIT-1, 0);
            }
            else{
                localValue.range(PARALLEL_BIT-1,0) = odd_temp.range(PARALLEL_BIT-1, 0);
                localValue.range((2*PARALLEL_BIT)-1,PARALLEL_BIT) = even_temp.range(PARALLEL_BIT-1, 0);
            }
            //printf("match_loc: %d \t",(read_buff_ind % HISTORY_SIZE));
            for (uint8_t copy = 0; copy < PARALLEL_BYTE; copy++){
            #pragma HLS LOOP UNROLL
                output_window.range(((copy+output_index)+1)*8 -1, (copy+output_index)*8) = localValue.range(((copy+(byte_loc%PARALLEL_BYTE))+1)*8 - 1, (copy+(byte_loc%PARALLEL_BYTE))*8);
            }
            if (match_len >= PARALLEL_BYTE){                    
                incr_output_index = PARALLEL_BYTE;                    
                match_loc += PARALLEL_BYTE;
                match_len -= PARALLEL_BYTE;
            }else{
                incr_output_index = match_len;
                match_loc += match_len;
                match_len = 0;
            }
            if(match_len ==0){
                next_state = READ_LIT_LEN;
            }else{
                next_state = READ_MATCH;
            }
        }
        if ((output_index+incr_output_index) >= PARALLEL_BYTE){
            for (uint8_t k = 0; k < PARALLEL_BYTE; k++){
            #pragma HLS LOOP UNROLL
                outStreamValue.range((k+1)*8 - 1, k*8) = output_window.range((k+1)*8 -1, k*8);
            }
            outStreamFlag=true;
            if (even_cntr){
                local_buf_even[even_write_buff_ind % BUFFER_SIZE] = outStreamValue;            
                even_write_buff_ind += 1;
                even_cntr=false;
            }else{
                local_buf_odd[odd_write_buff_ind % BUFFER_SIZE] = outStreamValue;            
                odd_write_buff_ind += 1;
                even_cntr=true;
            }
            
            for(uint8_t shift = 0; shift < PARALLEL_BYTE; shift++){
            #pragma HLS LOOP UNROLL
                output_window.range((shift+1)*8 -1, shift*8) = output_window.range(((shift+PARALLEL_BYTE)+1)*8 -1, (shift+PARALLEL_BYTE)*8);
            }
            output_index += incr_output_index - PARALLEL_BYTE;
        }else{            
            outStreamFlag = false;
            output_index += incr_output_index;
        }        

        if (outStreamFlag){
            outStream << outStreamValue;
            outCntr+=PARALLEL_BYTE;
        }
    }
    out_size += PARALLEL_BYTE;
    
    // Write out if there is remaining left over data in output buffer
    // to outStream
    if((original_size%PARALLEL_BYTE)!=0){
            for (uint8_t k = 0; k < PARALLEL_BYTE; k++){
            #pragma HLS LOOP UNROLL
                outStreamValue.range((k+1)*8 - 1, k*8) = output_window.range((k+1)*8 -1, k*8);
            }
            outStream << outStreamValue;
    }
    //printf("LoopTerminate:%d \t decompress_outSize:%d \n",out_size,total_cntr);
}

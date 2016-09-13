/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * mobitex_coding.h
 *
 *  Created on: 01.08.2013
 *      Author: Philipp Werner
 */

#ifndef INCLUDED_TNC_NX_MOBITEX_CODING_H
#define INCLUDED_TNC_NX_MOBITEX_CODING_H

#include <tnc_nx/api.h>
#include <stdint.h>
#include <cstdio>

#define FALSE 	0
#define TRUE	1
#define RUN		2

// FEC look-up table
const uint8_t FEC_LOOK_UP[16] = { 	0x0, 0x1, 0x2, 0x3,
									0x4, 0x1, 0x2, 0x1,
									0x8, 0x4, 0x8, 0x2,
									0xF, 0x4, 0x8, 0xF };
// FEC calculation matrixes
const uint8_t FEC_MATRIX_8b[4] = {		0b11101100,
										0b11010011,
										0b10111010,
										0b01110101 };
const uint16_t FEC_MATRIX_12b[4] = {	0b111011001000,
										0b110100110100,
										0b101110100010,
										0b011101010001 };

namespace gr {
  namespace tnc_nx {

    class TNC_NX_API mobitex_coding
    {
     private:

    	// FEC FUNCTIONS
    	uint8_t calc_fec			(uint8_t data);
    	uint8_t even_parity_8b		(uint8_t val);
    	uint8_t decalc_fec			(uint16_t data);
    	uint8_t even_parity_12b	(uint16_t val);
    	uint8_t check_fec			(uint8_t *byte, uint8_t fec);
    	
    	// CRC FUNCTIONS
    	uint16_t calc_cs_crc		(uint8_t *ptr, int bytecount);
    	uint16_t calc_db_crc		(uint8_t *ptr, int bytecount);
    	
     public:
		mobitex_coding();
		~mobitex_coding();
		
		struct head{
			// type of current message
			uint8_t 	msg_type;
			// number of datablocks to handle (0..32)
			uint8_t		blocks;
			// control-bytes
			uint8_t 	control[2];
			uint8_t 	control_fec[2];
		} org, cur;

		// callsign
		uint8_t 	callsign[8];
		// datablocks
		uint8_t 	mob_data[32][20];
		uint8_t 	mob_fec[32][20];
		// short datablocks (sdb)
		uint8_t 	mob_sdb[6];
		uint8_t		mob_sdb_fec[6];
		// datablocks errorcode
		uint8_t		errorcount;
		uint8_t 	errorcode[4];
		uint8_t		errorposition[32];

		// shift register for scrambler
		uint16_t 	scramble_shift_reg;
      
		// PUBLIC FUNCTIONS
		void clear_blocks();
		void clear_block(int blocknum);
		void clear_sdb();
		void clear_head(struct head *ptr);
		void clear_errors();
		void reset_scrambler();
		uint8_t scramble(uint8_t bit);
      
		// Decoding
		uint8_t decode_control(struct head *ptr);
		uint8_t decode_callsign();
		uint8_t decode_datablock(int blocknum);
		uint8_t decode_short_datablock();
		
		// Encoding
		void encode_control(struct head *ptr);
		void encode_callsign();
		void encode_datablock(int blocknum);
		void encode_short_datablock();

		// Error handling
		void get_errors();
		void save_head();

    };
  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_TNC_NX_MOBITEX_CODING_H */


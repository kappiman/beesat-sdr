/* -*- c++ -*- */
/* 
 * Copyright 2013 - 2016 Chair of Space Technology, Technische Universität Berlin
 * 
 * Authors: Philip Werner, Sascha Kapitola
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
 * frame_composer.cc
 *
 *  Created on: 16.10.2013
 *      Author: Philipp Werner
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tnc_nx/frame_composer.h>
#include <tnc_nx/nx_protocol.h>

namespace gr {
  namespace tnc_nx {

  /***************************
   * The private constructor *
   ***************************/
  frame_composer::frame_composer(mobitex_coding* mob_ptr)
  : mob(mob_ptr)
  {
  }

  /**************************
   * Our virtual destructor *
   **************************/
  frame_composer::~frame_composer()
  {
  }

  /*
   * Convert binary output-data to +/- 1 M-ary data stream for modulator input
   */
  int8_t frame_composer::con(uint8_t bit){
  	if(bit>0)
  		return 1;
  	else
  		return (-1);
  }

  /*
   * ASSEMBLE PARTS OF THE MESSAGE
   */
  // sync bytes
  int	frame_composer::write_sync(int8_t *buf){
  	for(int i=0; i<NUM_OF_SYNC_BYTES; ++i)
  		for(int j=0; j<8; ++j)
  			buf[i*8+j] = con(SYNC_4k8>>(7-j) & 0x01);
  	return (NUM_OF_SYNC_BYTES*8);
  }

  // header bytes (2 framesync bytes, 2 control and 1 control fec)
  int	frame_composer::write_header(int8_t *buf){
  	for(int i=0; i<16; ++i)
			buf[i] = con(FRAMESYNC>>(15-i) & 0x0001);			// FRAME SYNC
		for(int i=0; i<8; ++i)
			buf[i+16] = con(mob->cur.control[0]>>(7-i) & 0x01);		// CTRL 0
		for(int i=0; i<8; ++i)
			buf[i+24] = con(mob->cur.control[1]>>(7-i) & 0x01);		// CTRL 1
		for(int i=0; i<4; ++i)
			buf[i+32] = con(mob->cur.control_fec[0]>>(3-i) & 0x01);	// CTRL FEC 0
		for(int i=0; i<4; ++i)
			buf[i+36] = con(mob->cur.control_fec[1]>>(3-i) & 0x01);	// CTRL FEC 1
		return (5*8);
  }

  int frame_composer::write_callsign(int8_t *buf){
		for(int i=0; i<8; ++i)
			for(int j=0; j<8; ++j)
				buf[i*8+j] = con(mob->callsign[i]>>(7-j) & 0x01);	// CALLSIGN
		return (8*8);
  }

  int frame_composer::write_data(int8_t *buf){
		// DATABLOCKS / INTERLEAVING
		mob->reset_scrambler();

		// for each block
		for(int k=0; k<mob->cur.blocks; ++k){
			// for each bit per data byte
			for(int i=0; i<8; ++i)
				// for each data byte
				for(int j=0; j<20; ++j)
					buf[(k*30*8)+(i*20)+j] = con(mob->scramble(mob->mob_data[k][j] >> (7-i) & 0x01));

			// for each bit per FEC-nibble
			for(int i=0; i<4; ++i)
				// for each FEC-nibble
				for(int j=0; j<20; ++j)
					buf[(k*30*8)+(i*20)+j+160] = con(mob->scramble(mob->mob_fec[k][j] >> (3-i) & 0x01));
		}
		return (mob->cur.blocks*30*8);
  }

  int frame_composer::write_ec(int8_t *buf){
  	mob->reset_scrambler();

		// for each block
		for(int k=0; k<mob->cur.blocks; ++k){
			// for each bit per data byte
			for(int i=0; i<8; ++i)
				// for each data byte
				for(int j=0; j<20; ++j)
					buf[(k*30*8)+(i*20)+j] = con(mob->scramble(mob->mob_data[mob->errorposition[k]][j] >> (7-i) & 0x01));

			// for each bit per FEC-nibble
			for(int i=0; i<4; ++i)
				// for each FEC-nibble
				for(int j=0; j<20; ++j)
					buf[(k*30*8)+(i*20)+j+160] = con(mob->scramble(mob->mob_fec[mob->errorposition[k]][j] >> (3-i) & 0x01));
		}

  	return (mob->cur.blocks*30*8);
  }

  int frame_composer::write_sdb(int8_t *buf){
		// SHORT DATA BLOCK / SCRAMBLING / INTERLEAVING
		mob->reset_scrambler();
		// for each data/crc-byte per block
		for(int i=0; i<8; ++i)
			// for each bit per byte
			for(int j=0; j<6; ++j)
				buf[(i*6)+j] = con(mob->scramble(mob->mob_sdb[j] >> (7-i) & 0x01));

		// for each fec-nibble per block
		for(int i=0; i<4; ++i)
			// for each bit per byte
			for(int j=0; j<6; ++j)
				buf[(i*6)+j+48] = con(mob->scramble(mob->mob_sdb_fec[j] >> (3-i) & 0x01));
		return (6*12);
  }

  int frame_composer::write_hangbytes(int8_t *buf){
		for(int i=0; i<NUM_OF_HANG_BYTES; ++i)
			for(int j=0; j<8; ++j)
				buf[(i*8)+j] = con(HANGBYTE>>(7-j) & 0x01);			// HANGBYTES
		return (NUM_OF_HANG_BYTES*8);
  }

  int frame_composer::write_baud_sync(int8_t *buf_1, int8_t *buf_2){
		for(int i=0; i<8; ++i)
			buf_1[i] = con(HANGBYTE>>(7-i) & 0x01);					// BIT SYNC 4k8
		for(int i=0; i<8; ++i)
			buf_1[i+8] = con(0xCC>>(7-i) & 0x01);					// BIT SYNC 4k8
		for(int i=0; i<4; ++i){
			for(int j=0; j<8; ++j)
				buf_2[(i*8)+j+32] = con(SYNC_9k6>>(7-j) & 0x01);	// BIT SYNC 9k6
		}
		return (4*8);
  }

  int frame_composer::write_bc_header(int8_t *buf){
		// replace control byte with baudchange control-byte
		uint8_t temp_ctrl0 = mob->cur.control[0];
		mob->cur.control[0] = BAUDCHANGE_CTRL;
		mob->encode_control(&mob->cur);

		write_header(buf);

		// return to correct control bytes for message
		mob->cur.control[0] = temp_ctrl0;
		mob->encode_control(&mob->cur);

		return (5*8);
  }

  bool frame_composer::read_ctrl(uint8_t bit){
	static int bit_c  = 0;
	static int byte_c = 0;

	mob->cur.control[byte_c] = (mob->cur.control[byte_c]<<1) | (bit & 0x01);
	++bit_c;
	if(bit_c > 7){
		bit_c = 0;
		++byte_c;
		if(byte_c > 1){
			byte_c = 0;
			return true;
		}
	}
	return false;
  }
  bool frame_composer::read_ctrl_fec(uint8_t bit){
	static int bit_c  = 0;
	static int byte_c = 0;

	mob->cur.control_fec[byte_c] = (mob->cur.control_fec[byte_c] << 1) | (bit & 0x01);
	++bit_c;
	if(bit_c > 3){
		bit_c = 0;
		++byte_c;
		if(byte_c > 1){
			byte_c = 0;
			return true;
		}
	}

	return false;
  }

  bool frame_composer::read_callsign(uint8_t bit){
	static int bit_c  = 0;
	static int byte_c = 0;

	mob->callsign[byte_c] = (mob->callsign[byte_c] << 1) | (bit & 0x01);
	++bit_c;
	if(bit_c > 7){
		bit_c = 0;
		++byte_c;
		if(byte_c > 7){
			byte_c = 0;
			return true;
		}
	}

	return false;
  }

  bool frame_composer::read_data(uint8_t bit){
	static int bit_c   = 0;
	static int byte_c  = 0;
	static int block_c = 0;

	// de-scramble and -interleave data and crc bytes
	if(bit_c < 8){
		mob->mob_data[block_c][byte_c] |= mob->scramble(bit) << (7-bit_c);
		++byte_c;
		if(byte_c > 19){
			byte_c = 0;
			++bit_c;
		}

	// de-scramble and -interleave fec bytes and check datablock once complete
	}else{
		mob->mob_fec[block_c][byte_c] |= mob->scramble(bit) << (11-bit_c);
		++byte_c;
		if(byte_c > 19){
			byte_c = 0;
			++bit_c;
			if(bit_c > 11){
				bit_c = 0;
				mob->decode_datablock(block_c);
				++block_c;
				if(block_c == mob->cur.blocks){
					block_c = 0;
					return true;
				}
			}
		}
	}

	return false;
  }

  bool frame_composer::read_ec(uint8_t bit){
	static int bit_c   = 0;
	static int byte_c  = 0;
	static int block_c = 0;

	// de-scramble and -interleave data and crc bytes
	if(bit_c < 8){
		mob->mob_data[mob->errorposition[block_c]][byte_c] |= mob->scramble(bit) << (7-bit_c);
		++byte_c;
		if(byte_c > 19){
			byte_c = 0;
			++bit_c;
		}

	// de-scramble and -interleave fec bytes and check datablock once complete
	}else{
		mob->mob_fec[mob->errorposition[block_c]][byte_c] |= mob->scramble(bit) << (11-bit_c);
		++byte_c;
		if(byte_c > 19){
			byte_c = 0;
			++bit_c;
			if(bit_c > 11){
				bit_c = 0;
				mob->decode_datablock(mob->errorposition[block_c]);
				++block_c;
				if(block_c == mob->cur.blocks){
					block_c = 0;
					return true;
				}else{
					// clear next block to replace it with a new one
					mob->clear_block(mob->errorposition[block_c]);
				}
			}
		}
	}

	return false;
  }

  bool frame_composer::read_sdb(uint8_t bit){
	static int bit_c  = 0;
	static int byte_c = 0;

	// de-scramble and -interleave data and crc bytes
	if(bit_c < 8){
		mob->mob_sdb[byte_c] |= mob->scramble(bit) << (7-bit_c);
		++byte_c;
		if(byte_c > 5){
			byte_c = 0;
			++bit_c;
		}

	// de-scramble and -interleave fec bytes and check datablock once complete
	}else{
		mob->mob_sdb_fec[byte_c] |= mob->scramble(bit) << (11-bit_c);
		++byte_c;
		if(byte_c > 5){
			byte_c = 0;
			++bit_c;
			if(bit_c > 11){
				bit_c = 0;
				return true;
			}
		}
	}

	return false;
  }

  bool frame_composer::skip_fh(){
	static int bit_c  = 0;
	static int byte_c = 0;
	++bit_c;
	if(bit_c > 7){
		bit_c = 0;
		++byte_c;
		if(byte_c > 8){
			byte_c = 0;
			return true;
		}
	}
	return false;
  }

  } /* namespace tnc_nx */
} /* namespace gr */


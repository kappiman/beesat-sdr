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
 * mobitex_coding.cc
 *
 *  Created on: 01.08.2013
 *      Author: Philipp Werner
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tnc_nx/mobitex_coding.h>

#define GEN_POLY_CS 	0x1021
#define GEN_POLY_X25	0x8408

#define BIT_FLIP(a,n) {(a)[(n)/8] ^= (1 << ((n)%8));}

namespace gr {
  namespace tnc_nx {

    /***************************
     * The private constructor *
     ***************************/
    mobitex_coding::mobitex_coding()
    	: scramble_shift_reg(0x01FF)
    {
    	// INIT DATA BUFFERS TO ALL ZEROS
    	clear_blocks();
    	clear_head(&org);
    	clear_head(&cur);
    	clear_errors();
    	reset_scrambler();
    }

    /**************************
     * Our virtual destructor *
     **************************/
    mobitex_coding::~mobitex_coding()
    {
    }

	// set all data variables to zero
	void 
	mobitex_coding::clear_blocks() {
		for(int i=0; i<8; ++i){
			callsign[i] 	= 0;
		}
		clear_sdb();
		for(int i=0; i<32; ++i){
			clear_block(i);
		}
	}
	void mobitex_coding::clear_block(int blocknum){
		for(int j=0; j<20; ++j){
			mob_data[blocknum][j] = 0;
			mob_fec[blocknum][j]  = 0;
		}
	}

	void mobitex_coding::clear_sdb(){
		for(int i=0; i<6; ++i){
			mob_sdb[i]  	= 0;
			mob_sdb_fec[i] 	= 0;
		}
	}

	void mobitex_coding::clear_head(struct head *ptr){
		ptr->msg_type 	= 0;
		ptr->blocks 	= 0;
		for(int i=0; i<2; ++i){
			ptr->control[i] 	= 0;
			ptr->control_fec[i] = 0;
		}
	}

	void mobitex_coding::clear_errors(){
		for(int i=0; i<32; ++i)
			errorposition[i] = 0;
		for(int i=0; i<4; ++i)
			errorcode[i] = 0;
		errorcount = 0;
	}

	/*********************
	 * S C R A M B L E R *
	 *********************/
	void
	mobitex_coding::reset_scrambler(){
    	scramble_shift_reg = 0x01FF;
    }

	uint8_t
	mobitex_coding::scramble(uint8_t bit){
		//register-bit 9th stage is "1"
		if(scramble_shift_reg & 0x0001)
			bit ^= 1;

		//Check 5th and 9th Stage of
		if( ((scramble_shift_reg & 0x0011) == 0x0010) || ((scramble_shift_reg & 0x0011) == 0x0001))
			scramble_shift_reg |= 0x0200;

		scramble_shift_reg = (scramble_shift_reg >> 1) & 0x01FF;

		return bit;
    }
	
	/***************
	 * C O D I N G *
	 ***************/

	/*
	 * decode and correct (if possible) the mobitex defined header and databytes
	 * returns:
	 * 		TRUE when data could be corrected or was ok
	 * 		FALSE for uncorrectable errors
	 */
	uint8_t
	mobitex_coding::decode_control(struct head *ptr) {
		if(!(check_fec(&ptr->control[0], ptr->control_fec[0])))
			return FALSE;
		if(!(check_fec(&ptr->control[1], ptr->control_fec[1])))
			return FALSE;
		return TRUE;	
	}
	
	uint8_t
	mobitex_coding::decode_callsign() {
	        register int i, j = 0;
	  
		if(!(calc_cs_crc(callsign, 8)))
			return TRUE;
		/* try to flip one bit at a time to get CRC match */
		for (i = 0; i < 8*8; i++) {
		  BIT_FLIP(callsign, i);
		  if(!(calc_cs_crc(callsign, 8))) return TRUE;
		  BIT_FLIP(callsign, i);
		}

		/* try double bit flips */
		for (i = 0; i < 8*8; i++) {
		  BIT_FLIP(callsign, i);
		  for (j = i + 1; j < 8*8; j++) {
		    BIT_FLIP(callsign, j);
		    if(!(calc_cs_crc(callsign, 8))) return TRUE;
		    BIT_FLIP(callsign, j);
		  }
		  BIT_FLIP(callsign, i);
		}

		/* unable to correct single or double bit error */
		return FALSE;
	}
	
	uint8_t
	mobitex_coding::decode_datablock(int blocknum) {

		uint8_t block_ok = TRUE;

		/*
		// DEBUG: test FEC and CRC checks by changing any values
		switch(blocknum){
		case 2:
			mob_data[0][1] = 0x11;
			mob_data[0][2] = 0x22;
			mob_data[0][3] = 0x02;
			mob_data[0][12] = 0x8C;
			break;

		case 17:
			mob_fec[17][1] = 0x1;
			mob_fec[17][2] = 0x2;
			mob_fec[17][3] = 0x4;
			mob_fec[17][12] = 0x8;
			break;

		default:
			break;
		}
		*/

		//printf("Block %02X\n", blocknum);

		// check FEC and try to correct if neccessary for data and crc
		for(int i=0; i<20; ++i){
			if(!(check_fec(&mob_data[blocknum][i], mob_fec[blocknum][i]))){
				block_ok = FALSE;
				//printf("ERR in Byte: %02X\n", i);
			}
		}
		//printf("CRC - RX: %02X %02X - Calc: %04X\n", mob_data[blocknum][18], mob_data[blocknum][19], calc_db_crc(mob_data[blocknum], 18));

		if(calc_db_crc(mob_data[blocknum], 18) != (mob_data[blocknum][18]<<8 | mob_data[blocknum][19]))
			block_ok = FALSE;

		if(!(block_ok))
			errorcode[(int)(blocknum/8)] |= 1 << (blocknum % 8);

		return block_ok;
	}
	
	uint8_t
	mobitex_coding::decode_short_datablock() {
		uint8_t block_ok = TRUE;

		//printf("SDB\n");

		// check FEC and try to correct if neccessary for data and crc
		for(int i=0; i<6; ++i){
			if(!(check_fec(&mob_sdb[i], mob_sdb_fec[i]))){
				block_ok = FALSE;
				//printf("ERR in Byte: %02X\n", i);
			}
		}
		//printf("CRC: %X %X \n", mob_sdb[4], mob_sdb[5]);

		if(calc_db_crc(mob_sdb, 4) != (mob_sdb[4]<<8 | mob_sdb[5]))
			block_ok = FALSE;

		return block_ok;
	}
	
	void
	mobitex_coding::encode_control(struct head *ptr){
		ptr->control_fec[0] = calc_fec(ptr->control[0]);
		ptr->control_fec[1] = calc_fec(ptr->control[1]);
	}
	
	void
	mobitex_coding::encode_callsign(){
		uint16_t cs_crc = calc_cs_crc(callsign, 6);
		callsign[6] = (uint8_t)(cs_crc>>8) & 0xFF;
		callsign[7] = (uint8_t)(cs_crc) & 0x00FF;
	}

	void
	mobitex_coding::encode_datablock(int blocknum){
		uint16_t db_crc = calc_db_crc(mob_data[blocknum], 18);
		mob_data[blocknum][18] = (uint8_t)(db_crc>>8) & 0xFF;
		mob_data[blocknum][19] = (uint8_t)(db_crc) & 0x00FF;
		for(int i=0; i<20; ++i){
			mob_fec[blocknum][i] = calc_fec(mob_data[blocknum][i]);
		}
	}

	void
	mobitex_coding::encode_short_datablock(){
		uint16_t db_crc = calc_db_crc(mob_sdb, 4);
		mob_sdb[4] = (uint8_t)(db_crc>>8) & 0xFF;
		mob_sdb[5] = (uint8_t)(db_crc) & 0x00FF;
		for(int i=0; i<6; ++i){
			mob_sdb_fec[i] = calc_fec(mob_sdb[i]);
		}
	}

	// count all errors marked in errorcode and write the corresponding blocknumber to errorposition
	void
	mobitex_coding::get_errors(){
		int block_c 	= 0;
		errorcount	 	= 0;
		for(int i=0; i<4; ++i){
			for(int j=0; j<8; ++j){
				if((errorcode[i]>>j) & 0x01){
					errorposition[block_c++]  = j+(i*8);
					++errorcount;
				}
			}
		}
	}

	// copy current header information to "original"
	void mobitex_coding::save_head(){
		org.msg_type = cur.msg_type;
		org.blocks = cur.blocks;
		for(int i=0; i<2; ++i){
			org.control[i] = cur.control[i];
			org.control_fec[i] = cur.control_fec[i];
		}
	}

	
	/*******************
	 * FEC Calculation *
	 *******************/
	
	/*
	 * calculate FEC for given byte
	 * returns:
	 * 		FEC-Value (4 Bit)
	 */
	uint8_t
	mobitex_coding::calc_fec(uint8_t data)
	{
		uint8_t fec = 0;
		for(int i = 0; i < 4; ++i){
			fec = (fec<<1) | !(even_parity_8b(FEC_MATRIX_8b[i] & data));
		}
		return (fec & 0x0F);
	}
	
	/*
	 * check 8 bit value for even parity
	 * returns:
	 *  	1 for even parity
	 *  	0 for odd parity
	 */
	uint8_t
	mobitex_coding::even_parity_8b(uint8_t val)
	{
		val ^= val >> 4;
		if(val & 0x02)
			--val;
		if(val & 0x04)
			--val;
		if(!(val & 0x08))
			--val;
		val &= 0x01;
		return val;
	}
	
	/*
	 * de-calculate FEC for given byte
	 * returns:
	 * 		8 bit value with the bit to be swapped/corrected set
	 * 		0xFF if value can not be corrected
	 */
	uint8_t
	mobitex_coding::decalc_fec(uint16_t data)
	{
		char 	done 		= FALSE;
		char 	fec 		= 0;
		int 	err_cnt 	= 0;
		short 	return_val	= 0;
		
		while(done != TRUE){

			for(int i = 0; i < 4; ++i){
				fec = fec << 1;
				if(!(even_parity_12b(FEC_MATRIX_12b[i] & data))){
					// parity is odd -> error detected
					++err_cnt;
					// mark error in FEC-nibble
					fec |= 0x01;
					//printf("x ");
				} // else: parity is even -> no error
			}
			
			//printf("calculated FEC: %X,  err-cnt: %X\n", fec, err_cnt);

			if(!err_cnt){
				// no errors detected
				done = TRUE;
				return (uint8_t)(return_val & 0xFF);
			}else if(done == FALSE){
				// errors(?) found
				short temp = FEC_LOOK_UP[fec] << ((err_cnt - 1) * 4);
				data ^= temp;
				return_val = temp >> 4;
				done = RUN;
				// reset variables for next run
				err_cnt = 0;
				fec = 0;
			}else{
				done = TRUE;
				return_val = 0xFF;
				return 0xFF;
			}
		}
		return (uint8_t)(return_val & 0xFF);
	}
		
	/*
	 * check 12 bit value for even parity
	 * returns:
	 * 		1 for even parity
	 * 		0 for odd parity
	 */
	uint8_t
	mobitex_coding::even_parity_12b(uint16_t val)
	{
		val ^= val >> 6;
		if(val & 0x02)
			--val;
		if(val & 0x04)
			--val;
		if(val & 0x08)
			--val;
		if(val & 0x10)
			--val;
		if(!(val & 0x20))
			--val;
		val &= 0x01;
		//printf("par chk ret: %X\n", val);
		return val;
	}
	
	/*
	 * correct a given byte with the corresponding FEC, if possible
	 * returns:
	 * 		TRUE when byte could be corrected
	 * 		FALSE when error could not be corrected
	 */
	uint8_t
	mobitex_coding::check_fec(uint8_t *byte, uint8_t fec){
		uint8_t corrector = decalc_fec((*byte<<4) | (fec));
		//printf("CORRECTOR: %X", corrector);
		if(corrector == 0xFF){
			// byte can not be corrected
			return FALSE;
		}else{
			// correct the byte at given bit
			//printf(" from %X", *byte);
			*byte ^= corrector;
			//printf(" - to: %X\n", *byte);
			return TRUE;
		}
	}
	
	/****************************
	 * CALLSIGN CRC Calculation *
	 ****************************/
	
	/*
	 * calculate CRC16 of given number of bytes
	 * returns:
	 * 		calculated CRC16 (short)
	 */
	uint16_t
	mobitex_coding::calc_cs_crc(uint8_t *ptr, int bytecount){
		uint16_t crc16 = 0;
		for (int i = 0; i < bytecount; ++i) {
			crc16 = crc16 ^ (ptr[i] << 8);
			for (int j = 0; j < 8; ++j) {
				if (crc16 & 0x8000){
					crc16 = (crc16 << 1) ^ GEN_POLY_CS;
				} else {
					crc16 = crc16 << 1;
				}
			}
		}
		return crc16;
	}

	/*****************************
	 * DATABLOCK CRC Calculation *
	 *****************************/

	/*
	 * calculate CRC16 of given number of bytes
	 * returns:
	 * 		calculated CRC16 (short)
	 */
	uint16_t
	mobitex_coding::calc_db_crc(uint8_t *ptr, int bytecount){
		uint16_t crc16 = 0xFFFF;

		for(int i=0; i<bytecount; ++i){

			// printf("%X ", ptr[i]);

			uint16_t mask = 1;

			for(int j=0; j<8; ++j){

				uint16_t feedback = 0;

				if((ptr[i] & mask) != 0)
					feedback = 1;

				feedback = (feedback ^ crc16) & 1;

				crc16 = (crc16>>1) & 0x7FFF;

				if(feedback != 0)
					crc16 ^= GEN_POLY_X25;

				mask = mask << 1;
			}
		}
		//printf("DB CRC: %X\n", (crc16 ^ 0xFFFF));
		return (crc16 ^ 0xFFFF);
	}



  } /* namespace tnc_nx */
} /* namespace gr */


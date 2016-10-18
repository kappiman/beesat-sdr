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
 * nx_protocol.h
 *
 *  Created on: 01.08.2013
 *      Author: Philipp Werner
 */

#ifndef NX_PROTOCOL_H_
#define NX_PROTOCOL_H_

#include <stdint.h>

namespace gr{
	namespace tnc_nx{

		// BLOCK INTERFACE MESSAGE TYPES
		#define BIT_MSG 0x01
		#define BIT_TRG 0x05
		#define BIT_DIG 0x06

		// G/S DEFINITIONS
		#define GS_ADDRESS 	0x00
		#define GS_SUB_ADR	0x00
	
		#define BROADCAST_ADDRESS	0x0F
		#define BROADCAST_SUB_ADR	0x03

		// TNC-NX COMMUNICATION-PROTOCOL MESSAGE TYPES
		#define T_ACK	0x0
		#define T_REG	0x1
		#define T_DIGI	0x2
		#define T_ECHO	0x3
		#define T_BAUD	0x4

		// COMMUNICATION-PROTOCOL STATES
		enum protocol_states{
			idle = 0,
			wait_msg = 1,
		};

		// CONTROL BYTES
		const uint8_t  BAUDCHANGE_CTRL		= 0x80;

		// CONST HEADER BITS
		const uint8_t  SYNC_4k8				= 0xCC;
		const uint8_t  SYNC_9k6				= 0xF0;
		const uint16_t FRAMESYNC			= 0x0EF0;
		const uint8_t  HANGBYTE				= 0xCC;

		// HEADER / FOOTER DEFINITION
		const uint8_t	NUM_OF_PAD_BYTES	= 40;//8;
		const uint8_t  	NUM_OF_SYNC_BYTES 	= 6;
		const uint8_t 	NUM_OF_HANG_BYTES 	= 2;//10;
		const uint8_t	NUM_OF_BAUD_SYNC	= 4;


		/*************
		 * FUNCTIONS *
		 *************/

		// GET STUFF
		uint8_t ack_bit(uint8_t *ctrl);
		uint8_t baud_bit(uint8_t *ctrl);
		uint8_t message_type(uint8_t *ctrl);
		uint8_t num_of_blocks(uint8_t *ctrl);
		uint8_t num_of_errors(uint8_t *ctrl);

		// CHECK STUFF
		uint8_t check_address(uint8_t adr_byte);

	} // namespace tnc_nx
} // namespace gr
#endif /* NX_PROTOCOL_H_ */


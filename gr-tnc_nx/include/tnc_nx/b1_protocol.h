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
 * 
 * Created on: 23.10.2013
 * 
 */

#ifndef B1_PROTOCOL_H_
#define B1_PROTOCOL_H_

#include <stdint.h>

namespace gr {
namespace tnc_nx {

#define CTRL0_BAUD	0x00
#define CTRL0_DIGI	0x96

#define T_ECHO	    0x03
#define T_UNDEF     0xFF

#define BLOCKNUM_TM		32
#define BLOCKNUM_ECHO	10
#define BLOCKNUM_TC		1

// CONST HEADER BITS
const uint8_t SYNC_4k8 = 0xCC;
const uint8_t SYNC_9k6 = 0xF0;
const uint16_t FRAMESYNC = 0x0EF0;
const uint8_t HANGBYTE = 0xCC;

// HEADER / FOOTER DEFINITION
const uint8_t NUM_OF_PAD_BYTES = 40; //8;
const uint8_t NUM_OF_SYNC_BYTES = 6;
const uint8_t NUM_OF_HANG_BYTES = 10;
const uint8_t NUM_OF_BAUD_SYNC = 4;

/************************
 * PROTOCOL EVAL HELPER *
 ************************/
// ??
uint8_t b1_message_type(uint8_t *ctrl) {
  if (ctrl[0] == CTRL0_DIGI)
		return T_ECHO;
	else
		return T_UNDEF;
}

}	// namespace tnc_nx
}	// namespace gr
#endif /* B1_PROTOCOL_H_ */

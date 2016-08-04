/*
 * b1_protocol.h
 *
 *  Created on: 23.10.2013
 *      Author: phil
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

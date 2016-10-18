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
 * 
 * Created on: 01.08.2013
 * 
 */

#include <tnc_nx/nx_protocol.h>

namespace gr{
	namespace tnc_nx {

		// GET STUFF
		uint8_t ack_bit(uint8_t *ctrl) 			{return ((ctrl[1] >> 1)	& 0x01);}
		uint8_t baud_bit(uint8_t *ctrl) 		{return ( ctrl[1]      	& 0x01);}
		uint8_t message_type(uint8_t *ctrl) 	{return ((ctrl[0] >> 5)	& 0x07);}
		uint8_t num_of_blocks(uint8_t *ctrl)	{return ((ctrl[0]		& 0x1F) + 1);}
		uint8_t num_of_errors(uint8_t *ctrl)	{return ( ctrl[0] 		& 0x1F);}

		// CHECK STUFF
		uint8_t check_address(uint8_t adr_byte){
	    	// check address
	    	if( ((adr_byte & 0xF0) == (GS_ADDRESS<<4)) || ((adr_byte & 0xF0) == (BROADCAST_ADDRESS <<4)) ){
	    		// check sub-address
	    		if( ((adr_byte & 0x0C) == (GS_SUB_ADR<<2)) || ((adr_byte & 0x0C) == (BROADCAST_SUB_ADR<<2)) ){
	    			return 1;	// TRUE
	    		}
	    	}
	    	return 0;			// FALSE
	    }
	} // namespace tnc_nx
} // namespace gr


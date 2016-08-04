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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tnc_nx/gscf_com.h>

namespace gr {
  namespace tnc_nx {

    gscf_com::gscf_com()
    : d_cmdid(0), d_noradno(0), d_off(0)
    {
    }

    gscf_com::~gscf_com()
    {
    }

    pmt::pmt_t
    gscf_com::send_msg_output(pmt::pmt_t msg,
    							int msg_type, uint16_t msg_len)
    {
    	// make output message vector and accessible pointer
    	pmt::pmt_t out_vector = pmt::make_u8vector((8 + msg_len), 0);
    	uint8_t* out = (uint8_t*) pmt::uniform_vector_writable_elements(out_vector, d_off);
    	// insert gscf-header
    	out[0] = (msg_len + 2) >> 8 & 0x00FF;
    	out[1] = (msg_len + 2) & 0x00FF;
    	out[2] = msg_type & 0xFF;
    	out[3] = d_noradno >> 16 & 0x0000FF;
    	out[4] = d_noradno >> 8 & 0x0000FF;
    	out[5] = d_noradno & 0x0000FF;
    	out[6] = d_cmdid >> 8 & 0x00FF;
    	out[7] = d_cmdid & 0x00FF;
    	// make pointer to message elements and write them to output message
    	const uint8_t* m = (const uint8_t *) pmt::uniform_vector_elements(msg, d_off);
    	// add message elements to output message vector
    	for (int i = 0; i < msg_len; ++i)
    		out[8 + i] = m[i];

    	return pmt::cons(pmt::PMT_NIL, pmt::make_blob(&out[0], (8 + msg_len)));
    }

    pmt::pmt_t
    gscf_com::send_msg_output_no_cmdid(pmt::pmt_t msg,
    							int msg_type, uint16_t msg_len)
    {
    	// make output message vector and accessible pointer
    	pmt::pmt_t out_vector = pmt::make_u8vector((6 + msg_len), 0);
    	uint8_t* out = (uint8_t*) pmt::uniform_vector_writable_elements(out_vector,	d_off);
    	// insert gscf-header
    	out[0] = msg_len >> 8 & 0x00FF;
    	out[1] = msg_len & 0x00FF;
    	out[2] = msg_type & 0xFF;
    	out[3] = d_noradno >> 16 & 0x0000FF;
    	out[4] = d_noradno >> 8 & 0x0000FF;
    	out[5] = d_noradno & 0x0000FF;
    	// make pointer to message elements and write them to output message
    	const uint8_t* m = (const uint8_t *) pmt::uniform_vector_elements(msg,	d_off);
    	for (int i = 0; i < msg_len; ++i)
    		out[6 + i] = m[i];

    	return pmt::cons(pmt::PMT_NIL, pmt::make_blob(&out[0], (6 + msg_len)));
    }

  } /* namespace tnc_nx */
} /* namespace gr */


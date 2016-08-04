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
 * nx_encoder_impl.h
 *
 *  Created on: 01.08.2013
 *      Author: Philipp Werner
 */

#ifndef INCLUDED_TNC_NX_NX_ENCODER_IMPL_H
#define INCLUDED_TNC_NX_NX_ENCODER_IMPL_H

#include <tnc_nx/nx_encoder.h>
#include <tnc_nx/mobitex_coding.h>
#include <tnc_nx/frame_composer.h>
#include <tnc_nx/nx_protocol.h>

#include <gnuradio/blocks/pdu.h>

namespace gr {
  namespace tnc_nx {

    class nx_encoder_impl : public nx_encoder
    {
     private:
    	size_t	d_off;

    	TNC_NX_API mobitex_coding tx;
    	TNC_NX_API frame_composer fr;

    	// compose and transmit a message
    	void transmit_msg();

     public:
		nx_encoder_impl();
		~nx_encoder_impl();

		// handle incoming messages
		void handle_msg(pmt::pmt_t msg);

		// debug output
		void debug_msg(pmt::pmt_t msg, const char *errormsg);
    };

  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_TNC_NX_NX_ENCODER_IMPL_H */


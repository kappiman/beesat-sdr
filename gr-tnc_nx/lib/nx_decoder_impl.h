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

#ifndef INCLUDED_TNC_NX_NX_DECODER_IMPL_H
#define INCLUDED_TNC_NX_NX_DECODER_IMPL_H

#include <tnc_nx/nx_decoder.h>
#include <tnc_nx/mobitex_coding.h>
#include <tnc_nx/frame_composer.h>
#include <tnc_nx/nx_protocol.h>

#include <gnuradio/blocks/pdu.h>

namespace gr {
  namespace tnc_nx {

    class nx_decoder_impl : public nx_decoder
    {
     private:
    	size_t	 d_off;
    	uint16_t header;

    	TNC_NX_API mobitex_coding rx;
    	TNC_NX_API frame_composer fr;

    	enum states { 	SEARCH,
    					RX_CTRL,
    					RX_CFEC,
    					RX_CS,
    					RX_SDB,
    					RX_DATA,
    					SKIP_FH
    	} state;

    	// reset RX path to search for frame head
    	void reset_rx();

    	// create message to TNC-Logic-Block
    	void output_message();
    	void output_trigger();

     public:
      nx_decoder_impl();
      ~nx_decoder_impl();

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_TNC_NX_NX_DECODER_IMPL_H */


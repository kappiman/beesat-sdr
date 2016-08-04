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

#ifndef INCLUDED_TNC_NX_TNC_B1_IMPL_H
#define INCLUDED_TNC_NX_TNC_B1_IMPL_H

#include <tnc_nx/tnc_b1.h>
#include <tnc_nx/gscf_com.h>
#include <tnc_nx/b1_protocol.h>
#include <tnc_nx/frame_composer.h>
#include <tnc_nx/mobitex_coding.h>

// BOOST
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

namespace gr {
  namespace tnc_nx {

    class tnc_b1_impl : public tnc_b1
    {
     private:
    	size_t d_off;
    	uint16_t header;

    	TNC_NX_API mobitex_coding rx;
    	TNC_NX_API mobitex_coding tx;
    	TNC_NX_API frame_composer frx;
    	TNC_NX_API frame_composer ftx;
    	TNC_NX_API gscf_com		  g;

	    // Buffer for incoming messages
	    struct {
	    	pmt::pmt_t msg;
	    	bool waiting;
	    } buf;

    	// TIMER functionality
    	boost::asio::io_service			ios;
    	boost::asio::deadline_timer*	timer;
    	boost::thread*					thr;
    	int								w_msg;
	    void run_timer() {ios.run();}
	    void start_timer(int ms) {
	    	timer->expires_from_now(boost::posix_time::milliseconds(ms));
	    	timer->async_wait(boost::bind(&gr::tnc_nx::tnc_b1_impl::handle_timeout, this, _1));
	    }
	    void idle_timer() {
	    	timer->expires_from_now(boost::posix_time::pos_infin);
	    	timer->async_wait(boost::bind(&gr::tnc_nx::tnc_b1_impl::handle_infinite_timeout, this, _1));
	    }
	    void handle_timeout(const boost::system::error_code& ec);
	    void handle_infinite_timeout(const boost::system::error_code& ec){
	    	if(ec != boost::asio::error::operation_aborted)
	    		std::cout << "Infinity timed out, EC: " << ec << std::endl;
	    }

    	// RECEIVER STATES
    	enum states { 	SEARCH,
    					RX_CTRL,
    					RX_CFEC,
    					RX_CS,
    					RX_SDB,
    					RX_DATA,
    					RX_EC,
    					RX_RLY_CS,
    					RX_RLS_SDB,
    					SKIP_FH
    	} state;

    	enum protocol_states{	idle,
    							wait_MSG
    	} p_state;

    	void set_idle();
    	void reset_rx();

    	// TCP/IP-Input message handler
    	void handle_msg_in(pmt::pmt_t msg);
    	void transmit_msg();
    	void output_module_id();
    	void output_msg();

     public:
      tnc_b1_impl();
      ~tnc_b1_impl();

      // Receive-Input handler
      int general_work(int noutput_items,
              gr_vector_int &ninput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);
    };

  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_TNC_NX_TNC_B1_IMPL_H */


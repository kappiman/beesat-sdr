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

#ifndef INCLUDED_TNC_NX_TNC_IMPL_H
#define INCLUDED_TNC_NX_TNC_IMPL_H

// TNC-NX Module
#include <tnc_nx/tnc.h>
#include <tnc_nx/nx_protocol.h>
#include <tnc_nx/gscf_com.h>

// C++
#include <iostream>
#include <cstdio>

// GNURADIO
#include <gnuradio/blocks/api.h>
#include <gnuradio/blocks/pdu.h>

// BOOST
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

namespace gr {
	namespace tnc_nx {

		class tnc_impl : public tnc
		{
		private:
			void debug_msg(pmt::pmt_t msg, const char * errormsg);
			// send message to transmit-path
			void send_msg_tx(uint64_t BIT, pmt::pmt_t msg);
		    
		    // constant message offset
		    size_t	d_off;

		    // TTS INTERFACE
		    TNC_NX_API	gscf_com g;

		    // TIMER FUNCTIONALITY
		    boost::asio::io_service ios;
		    boost::asio::deadline_timer* timer;
		    boost::thread* thr;
		    int w_msg;

		    void run_timer() {ios.run();}
		    void start_timer(int ms) {
		    	timer->expires_from_now(boost::posix_time::milliseconds(ms));
		    	timer->async_wait(boost::bind(&gr::tnc_nx::tnc_impl::handle_timeout, this, _1));
		    }
		    void idle_timer() {
		    	timer->expires_from_now(boost::posix_time::pos_infin);
		    	timer->async_wait(boost::bind(&gr::tnc_nx::tnc_impl::handle_infinite_timeout, this, _1));
		    }
		    void handle_timeout(const boost::system::error_code& ec);
		    void handle_infinite_timeout(const boost::system::error_code& ec){
		    	if(ec != boost::asio::error::operation_aborted)
		    		std::cout << "Infinity timed out, EC: " << ec << std::endl;
		    }

		    // BUFFER FOR TX-MESSAGE
		    struct {
		    	pmt::pmt_t msg;
		    	bool waiting;
		    } buf;

		    // CURRENT MSG INFO
		    uint8_t cur_ctrl[2];
		    uint8_t cur_blocks;

		    // TNC-NX PROTOCOL HANDLING
		    enum protocol_states p_state;
		    int repeat_cnt;

		    // set a protocol state
		    void set_idle();
		    void set_wait_msg();

		public:
			tnc_impl();
			~tnc_impl();

			// handle message from Network Interface
			void handle_msg_input(pmt::pmt_t msg);
			// handle message from receive-path
			void handle_msg_rx(pmt::pmt_t msg);
		};

	} // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_TNC_NX_TNC_IMPL_H */


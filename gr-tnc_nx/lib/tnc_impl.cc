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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define DEBUG_ON
#ifdef DEBUG_ON
  #warning "DEBUG STILL ON!"
#endif

#include <gnuradio/io_signature.h>
#include "tnc_impl.h"

#include <cstdio>

namespace gr {
namespace tnc_nx {

tnc::sptr tnc::make() {
  return gnuradio::get_initial_sptr(new tnc_impl());
}

/* The private constructor */
tnc_impl::tnc_impl()
  : 	gr::block("TNC NX",
        gr::io_signature::make(0, 0, 0),
        gr::io_signature::make(0, 0, 0)),
    d_off(0),                       	// constant vector offset
    ios(), thr(NULL), timer(NULL),		// initialize timer
    p_state(idle), repeat_cnt(0),			// initialize protocol
    w_msg(2000),                    	// set wait times
    g()
{
  /******************
   * INIT I/O-PORTS *
   ******************/
  message_port_register_out(pmt::mp("TX"));
  message_port_register_in(pmt::mp("Input"));
  set_msg_handler(pmt::mp("Input"), boost::bind(&tnc_impl::handle_msg_input, this, _1));
  message_port_register_out(pmt::mp("Output"));
  message_port_register_in(pmt::mp("RX"));
  set_msg_handler(pmt::mp("RX"), boost::bind(&tnc_impl::handle_msg_rx, this, _1));

  /*********************
   * INIT TIMER THREAD *
   *********************/
  timer = new boost::asio::deadline_timer(ios);
  timer->expires_from_now(boost::posix_time::pos_infin);
  timer->async_wait(boost::bind(&tnc_impl::handle_infinite_timeout, this, _1));
  thr = new boost::thread(&tnc_impl::run_timer, this);

  /***********************
   * INIT MESSAGE BUFFER *
   ***********************/
  buf.msg = pmt::make_u8vector(600,0);
  buf.waiting = false;

  cur_ctrl[0]=0;
  cur_ctrl[1]=0;
  cur_blocks=0;
}

/* Our virtual destructor */
tnc_impl::~tnc_impl() {
  ios.stop();
  thr->join();
}

/*******************
 * TIMEOUT HANDLER *
 *******************/
void tnc_impl::handle_timeout(const boost::system::error_code& ec) {
  if (ec == 0) {
    switch (p_state) {

      case wait_msg:
        std::cout << "wait_msg Timeout" << std::endl;
        message_port_pub(pmt::mp("Output"), g.out_ack(SAT_NOK));;
        set_idle();
        break;

      case idle:
      default:
        std::cout << "ERROR: idle Timeout" << std::endl;
        set_idle();
        break;
    }
  } else if (ec == boost::asio::error::operation_aborted) {
    std::cout << "Timer operation aborted" << std::endl;
  } else {
    std::cout << "Timer Error: " << ec << std::endl;
  }
}

/***********************
 * SET PROTOCOL STATES *
 ***********************/

void tnc_impl::set_idle() {
  p_state = idle;
  idle_timer();
  repeat_cnt = 0;
  // check for waiting tx-messages
  if(buf.waiting){
    // handle waiting message from the buffer
    handle_msg_input(pmt::cons(pmt::PMT_NIL, buf.msg));
    buf.waiting = false;
  }
}

void tnc_impl::set_wait_msg() {
  // calculate actual transmission time in milliseconds
  double tx_time 	= 0;

  if(baud_bit(cur_ctrl)){
    // HIGH BAUDRATE (9600 bps)
    tx_time =(NUM_OF_PAD_BYTES
        + NUM_OF_SYNC_BYTES
        + 5							// framehead 4k8
        + NUM_OF_BAUD_SYNC)*8 * 1/(4.8)
        +(5	+ 8						// framehead + callsign 9k6
        //+ num_of_blocks(cur_blocks)*30	// data bytes
        + cur_blocks*30	// data bytes
        + NUM_OF_HANG_BYTES)*8 * 1/(9.6);
    printf("9k6-TX-time: %f\n", tx_time);
  }else{
    // LOW BAUDRATE (4800 bps)
    tx_time =(NUM_OF_PAD_BYTES
        + NUM_OF_SYNC_BYTES
        + 5	+ 8						// framehead + callsign 4k8
        //+ num_of_blocks(cur_blocks)*30	// data bytes
        + cur_blocks*30	// data bytes
        + NUM_OF_HANG_BYTES)*8 * 1/(4.8);
    printf("4k8-TX-time: %f\n", tx_time);
  }
  // set wait_msg state
  p_state = wait_msg;
  // start timer waiting for answer message
  start_timer((int)(w_msg+tx_time));
}

/*********************************/
/***** TRANSCEIVER INTERFACE *****/
/*********************************/

// SEND MESSAGE TO TX-PATH / ENCODER
void tnc_impl::send_msg_tx(uint64_t BIT, pmt::pmt_t msg) {
  message_port_pub(pmt::mp("TX"), pmt::cons(pmt::from_uint64(BIT), msg));
}

// GET MESSAGE FROM RX-PATH / DECODER
void tnc_impl::handle_msg_rx(pmt::pmt_t msg) {
  // access received message
  pmt::pmt_t meta = pmt::car(msg);
  pmt::pmt_t vector = pmt::cdr(msg);
  const uint8_t *m = (const uint8_t *) pmt::uniform_vector_elements(vector, d_off);

  std::cout << "TNC receive: " << pmt::to_uint64(meta);

  // check message type
  switch (pmt::to_uint64(meta)) {
    case BIT_TRG:	// receive trigger, send to TTS
      std::cout << " - TRG!" << std::endl;
      message_port_pub(pmt::mp("Output"), g.out_trigger_rx());
      idle_timer();
      break;

    case BIT_MSG:	// REG message received
      std::cout << " - MSG!" << std::endl;
      message_port_pub(pmt::mp("Output"), g.out_msg(vector));
      set_idle();
      break;

    case BIT_DIG:	// DIGI message received
      std::cout << " - DIG!" << std::endl;
      message_port_pub(pmt::mp("Output"), g.out_digi(vector));
      set_idle();
      break;

    default:
      // do not handle unknown messages
      std::cout << " - unhandled! - RX CAR: " << pmt::to_uint64(pmt::car(msg)) << std::endl;
      break;
  }
}

/****************************/
/***** TCP/IP INTERFACE *****/
/****************************/

/*
 * RECEIVE FROM TCP/IP
 */
void tnc_impl::handle_msg_input(pmt::pmt_t msg) {
  // access received message
  //pmt::pmt_t meta 	= pmt::car(msg);
  pmt::pmt_t vector 	= pmt::cdr(msg);
  const uint8_t* m 	= (const uint8_t*) pmt::uniform_vector_elements(vector, d_off);

  // read gscf header
  uint16_t g_len 	= (m[0]<<8) + m[1];
  uint8_t  g_type =  m[2];

  // check header length
  if (g_len + 6 != pmt::length(vector)) {
    std::cout << "GSCF-Msg-Length not correct" << std::endl;
    message_port_pub(pmt::mp("Output"), g.out_ack(TNC_NOK));
    return;	// -> abort
  }

  switch (g_type) {
    case g_TC:
      // check for pending communication session
      if(p_state != idle){
        printf("buffering message...!\n");
        // buffer message
        buf.msg 	= vector;
        buf.waiting = true;
      }else{
        // save GSCF-header information
        g.set_noradno((uint32_t)((m[3]<<16) + (m[4]<<8) + m[5]));
        g.set_cmdid((uint16_t)((m[6]<<8)+m[7]));

        // save msg-info
        cur_ctrl[0]= m[8];
        cur_ctrl[1]= m[9];
        cur_blocks = num_of_blocks(cur_ctrl);

        // remove GSCF-header and CMD-ID from message
        pmt::pmt_t 	out_vector 	= pmt::make_u8vector(g_len - 2, 0);
        uint8_t* 	out 		= (uint8_t*)pmt::uniform_vector_writable_elements(out_vector, d_off);
        for (int i=0; i<(g_len-2); ++i)
          out[i] = m[i+8];

        // pass message to ENCODER-block
        send_msg_tx(BIT_MSG, out_vector);

        // check ack-bit and set next state
        set_wait_msg();

        // send TNC OK to TTS
        message_port_pub(pmt::mp("Output"), g.out_ack(TNC_OK));
      }
      break;

    case g_CMD:
      break;

    default:
      std::cout << "GSCF-Msg-Type not recognized." << std::endl;
      message_port_pub(pmt::mp("Output"), g.out_ack(TNC_NOK));
      break;
    }
}

/*
 * DEBUG MESSAGE FUNCTION
 */

void tnc_impl::debug_msg(pmt::pmt_t msg, const char * errormsg) {
#ifdef DEBUG_ON
  std::cout << "****************************************** " << std::endl;
  std::cout << "DEBUG: " << errormsg << std::endl;

  // get the message
  pmt::pmt_t meta = pmt::car(msg);
  pmt::pmt_t vector = pmt::cdr(msg);
  size_t len = pmt::length(vector);
  std::cout << "Length: " << len << std::endl;

  const uint8_t* d = (const uint8_t*) pmt::uniform_vector_elements(vector, d_off);
  for (size_t i = 0; i < len; i += 16) {
    printf("%04x: ", ((unsigned int) i));
    for (size_t j = i; j < std::min(i + 16, len); j++) {
      printf("%02x ", d[j]);
    }
    std::cout << std::endl;
  }
  std::cout << "****************************************** " << std::endl;
#endif
}

} /* namespace tnc_nx */
} /* namespace gr */


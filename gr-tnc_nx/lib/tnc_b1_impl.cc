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

#include <gnuradio/io_signature.h>
#include "tnc_b1_impl.h"

namespace gr {
  namespace tnc_nx {

    tnc_b1::sptr
    tnc_b1::make()
    {
      return gnuradio::get_initial_sptr
        (new tnc_b1_impl());
    }

    /*
     * The private constructor
     */
    tnc_b1_impl::tnc_b1_impl()
      : gr::block("TNC BEESAT-1",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(0, 0, 0)),
              d_off(0),
              header(0),
              state(SEARCH), p_state(idle),
              rx(), frx(&rx),
              tx(), ftx(&tx),
              g(),
              ios(), thr(NULL), timer(NULL), w_msg(2000)
    {
      /******************
       * INIT I/O-PORTS *
       ******************/
      message_port_register_in(pmt::mp("Input"));
      set_msg_handler(pmt::mp("Input"), boost::bind(&tnc_b1_impl::handle_msg_in, this, _1));
      message_port_register_out(pmt::mp("Output"));
      message_port_register_out(pmt::mp("TX4k8"));

      /**************
       * INIT TIMER *
       **************/
      timer = new boost::asio::deadline_timer(ios);
      timer->expires_from_now(boost::posix_time::milliseconds(500));
      timer->async_wait(boost::bind(&tnc_b1_impl::output_module_id, this));
      thr = new boost::thread(&tnc_b1_impl::run_timer, this);

      // Init Message Buffer
      buf.msg = pmt::make_u8vector(600,0);
      buf.waiting = false;

      // set history-buffer for frame head search
      set_history(105);
    }

    /*
     * Our virtual destructor.
     */
    tnc_b1_impl::~tnc_b1_impl() { }

    /*****************
     * TIMER HANDLER *
     *****************/
    void tnc_b1_impl::handle_timeout(const boost::system::error_code& ec) {
      if (ec == 0) {
        std::cout << "TM Timeout" << std::endl;
        message_port_pub(pmt::mp("Output"), g.out_ack(SAT_NOK));
        set_idle();
      } else if (ec == boost::asio::error::operation_aborted) {
        std::cout << "Timer operation aborted" << std::endl;
      } else {
        std::cout << "Timer Error: " << ec << std::endl;
      }
    }

    void tnc_b1_impl::set_idle(){
      reset_rx();
      idle_timer();
      p_state = idle;
      if(buf.waiting){
        // handle waiting message from the buffer
        handle_msg_in(pmt::cons(pmt::PMT_NIL, buf.msg));
        buf.waiting = false;
      }
    }

    void tnc_b1_impl::reset_rx(){
      header = 0;
      state = SEARCH;
      rx.clear_blocks();
      rx.clear_head(&rx.cur);
      rx.clear_head(&rx.org);
      rx.clear_errors();
    }

    void tnc_b1_impl::handle_msg_in(pmt::pmt_t msg){
      // access received message
      pmt::pmt_t vector = pmt::cdr(msg);
      const uint8_t *m = (const uint8_t *) pmt::uniform_vector_elements(vector, d_off);

      // read gscf header
      uint16_t g_len 	= (m[0]<<8) + m[1];
      uint8_t  g_type =  m[2];

      // check header length
      if (g_len + 6 != pmt::length(vector)) {
        std::cout << "GSCF-Msg-Length not correct" << std::endl;
        g.out_ack(TNC_NOK);
        return;	// -> abort
      }

      switch (g_type) {
        case g_TC:
          if(p_state != idle){
            printf("buffering message ...\n");
            buf.msg = vector;
            buf.waiting = true;
          } else {
            //handle message
            g.set_noradno((uint32_t)((m[3]<<16) + (m[4]<<8) + m[5]));
            g.set_cmdid((uint16_t)((m[6]<<8)+m[7]));

            m = &m[8];

            // write and code data to mobitex transmit buffer
            tx.cur.control[0] = *m++;
            tx.cur.control[1] = *m++;
            tx.cur.msg_type	= b1_message_type(tx.cur.control);

            switch(tx.cur.msg_type){
              case T_ECHO:
                tx.cur.blocks = 10;
                break;
              default:
                // if message type is not recognized answer with TNC NOK and abort
                message_port_pub(pmt::mp("Output"), g.out_ack(TNC_NOK));
                return;
            }

            tx.encode_control(&tx.cur);
            printf("TX ctrl: %02X %02X fec: %01X %01X\n", tx.cur.control[0], tx.cur.control[1], tx.cur.control_fec[0], tx.cur.control_fec[1]);

            for(int i=0; i<8; ++i)
              tx.callsign[i] = *m++;
            tx.encode_callsign();

            for(int i=0; i<tx.cur.blocks; ++i){
              for(int j=0; j<18; ++j)
                tx.mob_data[i][j] = *m++;
              tx.encode_datablock(i);
            }
            // create output-messages to Modulator
            transmit_msg();
            // acknowledge transmission to TTS
            message_port_pub(pmt::mp("Output"),g.out_ack(TNC_OK));
            start_timer(w_msg);
            p_state = wait_MSG;
          }
          break;

        case g_CMD:
          std::cout << "No Commands supported currently!" << std::endl;
          break;

        default:
          message_port_pub(pmt::mp("Output"), g.out_ack(TNC_NOK));
          break;
      }
    }

    void tnc_b1_impl::transmit_msg(){
      int count = NUM_OF_PAD_BYTES*8;
      int len_4k8 = ((NUM_OF_PAD_BYTES +
            NUM_OF_SYNC_BYTES +
            5 + 8 +
            (tx.cur.blocks*30) +
            NUM_OF_HANG_BYTES
            ) * 8);

      // **** PMT FOR 4k8 MODULATOR INPUT **** //
      pmt::pmt_t out_vector = pmt::make_s8vector(len_4k8, 0);
      int8_t *out_1 = (int8_t*) pmt::uniform_vector_writable_elements(out_vector, d_off);

      // **** ASSEMBLE MESSAGE **** //
      count += ftx.write_sync(&out_1[count]);
      count += ftx.write_header(&out_1[count]);
      count += ftx.write_callsign(&out_1[count]);
      count += ftx.write_data(&out_1[count]);
      count += ftx.write_hangbytes(&out_1[count]);

      // **** PUBLISH MESSAGES **** //
      message_port_pub(pmt::mp("TX4k8"), pmt::cons(pmt::PMT_NIL, pmt::make_blob(&out_1[0], len_4k8)));
    }

    // transmit module id
    void tnc_b1_impl::output_module_id(){
    	message_port_pub(pmt::mp("Output"), g.out_module_id());
    	idle_timer();
    }

    // assemble message for output to tts
    void tnc_b1_impl::output_msg(){
    	int msg_len = (rx.org.blocks*18)+16;
    	// create output buffer
    	pmt::pmt_t out_vector = pmt::make_u8vector(msg_len, 0);
    	// make accessible pointer
    	uint8_t* out = (uint8_t *)pmt::uniform_vector_writable_elements(out_vector, d_off);

    	// write callsign for output messages
      rx.callsign[0] = 'D';
      rx.callsign[1] = 'P';
      rx.callsign[2] = '0';
      rx.callsign[3] = 'B';
      rx.callsign[4] = 'E';
      rx.callsign[5] = 'E';

    	*out++ = rx.org.control[0];
    	*out++ = rx.org.control[1];
    	for(int i=0; i<8; ++i)
    		*out++ = rx.callsign[i];
    	for(int k=0; k<rx.org.blocks; ++k)
    		for(int i=0; i<18; ++i)
    			*out++ = rx.mob_data[k][i];
    	*out++ = 0xAA;	// TEMP-DUMMY
    	for(int i=0; i<4; ++i)
    		*out++ = rx.errorcode[i];
    	*out++ = 0xBB;	// SIG-QLTY-DUMMY

    	if(rx.org.msg_type == T_ECHO)
    		message_port_pub(pmt::mp("Output"), g.out_digi(out_vector));
    	else
    		message_port_pub(pmt::mp("Output"), g.out_msg(out_vector));
    }

    /****************************
     * HANDLER FOR SAMPLE INPUT *
     ****************************/
    int
    tnc_b1_impl::general_work(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
        const uint8_t *in = (const uint8_t *) input_items[0];

        // Main loop for Incoming bits
        for(int i=104; i<(noutput_items+104); ++i) {
          switch (state){
            case SEARCH:
              // SEARCH for FRAMESYNC
              header = (header<<1) | (in[i] & 0x01);
              if(header == FRAMESYNC){
                printf("\n****** SYNC! ****** \n");
                // set next state;
                state = RX_CTRL;
                rx.clear_head(&rx.cur);
                rx.reset_scrambler();
              }
              break;

            case RX_CTRL:
              if(frx.read_ctrl(in[i])){
                state = RX_CFEC;
              }
              break;

            case RX_CFEC:
              if(frx.read_ctrl_fec(in[i])){
                if(rx.decode_control(&rx.cur)){								// CTRL-FEC OK
                  rx.cur.msg_type = b1_message_type(rx.cur.control);			// GET MSG-TYPE

                  printf("RX-MSG-TYPE: %02X\n", rx.cur.msg_type);

                  switch(rx.cur.msg_type) {
                    case T_ECHO:
                      rx.cur.blocks = BLOCKNUM_ECHO;
                      state = RX_DATA;
                      message_port_pub(pmt::mp("Output"), g.out_trigger_rx());
                      break;

                    default:
                      printf("Unknown CONTROL BYTES: 0x %02X %02X \n", rx.cur.control[0], rx.cur.control[1]);
                      reset_rx();
                      i = i-(5*8)+8;
                      break;
                  }
                } else {
                  printf("BAD CTRL - ctrl: 0x%02X %02X fec: 0x%01X%01X\n", rx.cur.control[0], rx.cur.control[1], rx.cur.control_fec[0], rx.cur.control_fec[1]);
                  reset_rx();
                  i = i-(5*8)+5;
                }
              }
            break;

          case RX_DATA:
            if(frx.read_data(in[i])){
              // get number and position of errors
              rx.get_errors();

              // check, if message is completely waste
              if(rx.cur.msg_type == T_ECHO && rx.errorcount > 9) {
                printf("completely corrupted message\n");
                reset_rx();
              } else {
                // transmit message to TTS
                rx.save_head();
                output_msg();
                // go to idle
                set_idle();
              }
            }
            break;

          default:
            break;
          } // end of switch sate
        } // end of iterating over incoming bits
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return 0;
    }

  } /* namespace tnc_nx */
} /* namespace gr */


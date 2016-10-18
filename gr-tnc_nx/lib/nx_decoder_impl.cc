/* -*- c++ -*- */
/* 
 * Copyright 2013 - 2016 Chair of Space Technology, Technische Universität Berlin
 * 
 * Authors: Philip Werner, Sascha Kapitola, Daniel Estévez
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
#include "nx_decoder_impl.h"

namespace gr {
  namespace tnc_nx {

    nx_decoder::sptr
    nx_decoder::make() {
      return gnuradio::get_initial_sptr(new nx_decoder_impl());
    }

    /*
     * The private constructor
     */
    nx_decoder_impl::nx_decoder_impl()
    : gr::block("NX Decoder",
                 gr::io_signature::make(1, 1, sizeof (char)),
                 gr::io_signature::make(0, 0, 0)),
                 d_off(0), header(0), state(SEARCH),
                 rx(), fr(&rx) {
      /********************************
       * SET UP I/O PORTS AND HANDLER *
       ********************************/
      
      // OUTPUT
      message_port_register_out(pmt::mp("out"));

      // set history for receiver, to be able to return to the 2nd bit of a found frame-synch marker, if reception
      // was aborted at any point from the marker down to the callsign
      // we need to hold at least the following:
      /*
       * 2 frame synch bytes
       * 2 ctrl-bytey
       * 1 ctrl-fec byte
       * 8 bytes callsign + crc
       * 	-> 13 bytes in total
       *
       * since bits are received, history his 13*8=104 samples
       */
      set_history(105);
    }

    /*
     * Our virtual destructor.
     */
    nx_decoder_impl::~nx_decoder_impl() {
    }

    void nx_decoder_impl::reset_rx() {
      header = 0;
      state = SEARCH;
      rx.clear_blocks();
    }

    void nx_decoder_impl::output_message() {
      int msg_len = (rx.org.blocks * 18) + 16;
      // create output buffer
      pmt::pmt_t out_vector = pmt::make_u8vector(msg_len, 0);
      // make accessible pointer
      uint8_t* out = (uint8_t *) pmt::uniform_vector_writable_elements(out_vector, d_off);

      *out++ = rx.org.control[0];
      *out++ = rx.org.control[1];
      for (int i = 0; i < 8; ++i)
        *out++ = rx.callsign[i];
      for (int k = 0; k < rx.org.blocks; ++k)
        for (int i = 0; i < 18; ++i)
          *out++ = rx.mob_data[k][i];
      *out++ = 0xAA; // TEMP-DUMMY
      for (int i = 0; i < 4; ++i)
        *out++ = rx.errorcode[i];
      *out++ = 0xBB; // SIG-QLTY-DUMMY

      if (rx.org.msg_type == T_DIGI)
        message_port_pub(pmt::mp("out"), pmt::cons(pmt::from_uint64((uint64_t) BIT_DIG), out_vector));
      else
        message_port_pub(pmt::mp("out"), pmt::cons(pmt::from_uint64((uint64_t) BIT_MSG), out_vector));
    }

    void nx_decoder_impl::output_trigger() {
      message_port_pub(pmt::mp("out"), pmt::cons(pmt::from_uint64((uint64_t) BIT_TRG), pmt::make_u8vector(0, 0)));
    }

    int
    nx_decoder_impl::general_work(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
      const uint8_t *in = (const uint8_t *) input_items[0];

      for (int i = 104; i < (noutput_items + 104); ++i) {

        switch (state) {

          case SEARCH:
            // SEARCH for FRAMESYNC
            header = (header << 1) | (in[i] & 0x01);
            if (header == FRAMESYNC) {
              // set next state;
              state = RX_CTRL;
              rx.clear_head(&rx.cur);
            }
            break;

          case RX_CTRL:
            if (fr.read_ctrl(in[i])) {
              state = RX_CFEC;
            }
            break;

          case RX_CFEC:
            if (fr.read_ctrl_fec(in[i])) {
              if (rx.decode_control(&rx.cur)) { // CTRL-FEC OK
                rx.cur.msg_type = message_type(rx.cur.control); // GET MSG-TYPE

                if (check_address(rx.cur.control[1])) { // ADR OK
                  // TODO: eval msg-type
                  /*printf("\n****** SYNC! ****** \n");
		    printf("RX-MSG-TYPE: %02X\n", rx.cur.msg_type);*/
                  switch (rx.cur.msg_type) {
                    case T_REG:
                    case T_ECHO:
                      state = RX_CS;
                      break;
                    
                    default:
                      reset_rx();
                      i = i - (5 * 8) + 5;
                      break;
                  }

                } else {
                  //printf("BAD ADR - ctrl: 0x %02X %02X \n", rx.cur.control[0], rx.cur.control[1]);
                  reset_rx();
                  i = i - (5 * 8) + 5;
                }
              } else {
                //printf("BAD CTRL - ctrl: 0x %02X %02X \n", rx.cur.control[0], rx.cur.control[1]);
                reset_rx();
                i = i - (5 * 8) + 5;
              }
            }
            break;

          case RX_CS:
            if (fr.read_callsign(in[i])) {
              if (!rx.decode_callsign()) {
                /*printf("BAD CS\n");
                printf("CS    : %.*s\n", 6, rx.callsign);
                printf("CRC-16: %02X%02X\n", rx.callsign[6], rx.callsign[7]);*/
              }
              switch (rx.cur.msg_type) {
                case T_ACK:
                  state = RX_SDB;
                  break;
                default:
                  rx.cur.blocks = num_of_blocks(rx.cur.control);
                  state = RX_DATA;
                  rx.clear_errors();
                  break;
              }
              rx.reset_scrambler();
              output_trigger();

            }
            break;

          case RX_SDB:
            if (fr.read_sdb(in[i])) {
              reset_rx();
            }
            break;

          case RX_DATA:
            if (fr.read_data(in[i])) {
              // get number and position of errors
              rx.get_errors();
              // copy/save original message header
              rx.save_head();

              // ACK REQUESTED
              if (ack_bit(rx.cur.control)) {
                if (!rx.errorcount) // NO Errors were detected
                  output_message(); // pass received message

                // NO ACK REQUESTED
              } else
                output_message(); // pass received message

              reset_rx();
            }
            break;
          
          case SKIP_FH:
            if (fr.skip_fh())
              reset_rx();
            break;

          default:

            break;

        }
      }

      // Tell runtime system how many input items we consumed on each input stream.
      consume_each(noutput_items);
      // Tell runtime system how many output items were produced.
      return 0;
    }

  } /* namespace tnc_nx */
} /* namespace gr */


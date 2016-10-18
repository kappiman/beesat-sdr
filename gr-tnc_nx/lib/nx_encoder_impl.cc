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

/*
 * nx_encoder_impl.cc
 *
 *  Created on: 01.08.2013
 *      Author: Philipp Werner
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define DEBUG_ON
#ifdef DEBUG_ON
#warning "DEBUG STILL ON!"
#endif

#include <gnuradio/io_signature.h>
#include "nx_encoder_impl.h"

namespace gr {
  namespace tnc_nx {

    nx_encoder::sptr
    nx_encoder::make()
    {
      return gnuradio::get_initial_sptr
        (new nx_encoder_impl());
    }

    /*
     * The private constructor
     */
    nx_encoder_impl::nx_encoder_impl()
      : gr::block("NX Encoder",
              gr::io_signature::make(0,0,0),
              gr::io_signature::make(0,0,0)),
        d_off(0),
        tx(),
        fr(&tx)
    {
      /********************************
    	 * SET UP I/O PORTS AND HANDLER *
    	 ********************************/
      // INPUT
      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"), boost::bind(&nx_encoder_impl::handle_msg, this, _1));
      // OUTPUT (single / double baudrate)
      message_port_register_out(pmt::mp("single"));
      message_port_register_out(pmt::mp("double"));

      // set initial callsign for ACK/EC "TNC_NX"
      tx.callsign[0] = 'T';
      tx.callsign[1] = 'N';
      tx.callsign[2] = 'C';
      tx.callsign[3] = '_';
      tx.callsign[4] = 'N';
      tx.callsign[5] = 'X';
      tx.callsign[6] = 0xDA;
      tx.callsign[7] = 0x74;
    }

    /*
     * Our virtual destructor.
     */
    nx_encoder_impl::~nx_encoder_impl()
    {
    }

    /*
     * MESSAGE HANDLER
     * 	- assemble data to mobitex-blocks, encode and transmit them
     * 	- set/unset relays-mode if neccessary
     */
    void nx_encoder_impl::handle_msg(pmt::pmt_t msg)
    {
      //debug_msg(msg, "ENCODER DEBUG");
      pmt::pmt_t meta = pmt::car(msg);
      pmt::pmt_t vector = pmt::cdr(msg);

      // identify parts of the message
      const uint8_t* m 	= (const uint8_t*) 	pmt::uniform_vector_elements(vector, d_off);

      switch(pmt::to_uint64(meta))
      {
        case BIT_MSG:	// NEW MESSAGE FOR TRANSMISSION
          // write and code data to mobitex transmit buffer
          tx.cur.control[0] = *m++;
          tx.cur.control[1] = *m++;
          tx.cur.msg_type	= message_type(tx.cur.control);
          tx.cur.blocks = num_of_blocks(tx.cur.control);
          tx.encode_control(&tx.cur);

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
          // save header for error correction
          tx.save_head();
          break;
        default:
          std::cout << "Unhandled BIT in Encoder: " << pmt::to_uint64(meta) << std::endl;
          break;
      }
    }

    /********************
     * OUTPUT FUNCTIONS *
     ********************/
    void nx_encoder_impl::transmit_msg(){
      int count = NUM_OF_PAD_BYTES*8;

      // DOUBLE SPEED BAUDRATE (9600bps)
      if(baud_bit(tx.cur.control)){
        int len_4k8 = ((NUM_OF_PAD_BYTES +
                NUM_OF_SYNC_BYTES +
                5 + NUM_OF_BAUD_SYNC
                ) * 8 +
                (5 + 8 +
                (tx.cur.blocks*30) +
                NUM_OF_HANG_BYTES
                ) * (8/2));

        //printf("len 4k8: %u\n", len_4k8);

        // **** PMT FOR 4k8 MODULATOR INPUT **** //
        pmt::pmt_t out_vector = pmt::make_s8vector(len_4k8, 0);
        int8_t *out_1 = (int8_t*) pmt::uniform_vector_writable_elements(out_vector, d_off);

        // **** PMT FOR 9k6 MODULATOR INPUT **** //
        pmt::pmt_t out_vector_2 = pmt::make_s8vector(len_4k8*2, 0);
        int8_t *out_2 = (int8_t*) pmt::uniform_vector_writable_elements(out_vector_2, d_off);

          // **** ASSEMBLE MESSAGE 4k8 **** //
          count += fr.write_sync(&out_1[count]);
          count += fr.write_bc_header(&out_1[count]);
        count += fr.write_baud_sync(&out_1[count], &out_2[count*2]);

        // calculate current position in 9k8-message
        count = count*2;

        // **** ASSEMBLE MESSAGE 9k6 **** //
        count += fr.write_header(&out_2[count]);
        count += fr.write_callsign(&out_2[count]);
        count += fr.write_data(&out_2[count]);
        count += fr.write_hangbytes(&out_2[count]);

        //debug_msg(out_vector, "4k8\n");
        //debug_msg(out_vector_2, "9k6\n");
          // **** PUBLISH MESSAGES **** //
        message_port_pub(pmt::mp("single"), pmt::cons(pmt::PMT_NIL, pmt::make_blob(&out_1[0], len_4k8)));
        message_port_pub(pmt::mp("double"), pmt::cons(pmt::PMT_NIL, pmt::make_blob(&out_2[0], len_4k8*2)));
      }
      // SIMPLE SPEED BAUDRATE (4800bps)
      else {
        int len_4k8 = ((NUM_OF_PAD_BYTES +
                NUM_OF_SYNC_BYTES +
                5 + 8 +
                (tx.cur.blocks*30) +
                NUM_OF_HANG_BYTES
                ) * 8);

        //printf("len 4k8: %u\n", len_4k8);

        // **** PMT FOR 4k8 MODULATOR INPUT **** //
        pmt::pmt_t out_vector = pmt::make_s8vector(len_4k8, 0);
        int8_t *out_1 = (int8_t*) pmt::uniform_vector_writable_elements(out_vector, d_off);

        // **** ASSEMBLE MESSAGE 4k8 **** //
        count += fr.write_sync(&out_1[count]);
        count += fr.write_header(&out_1[count]);
        count += fr.write_callsign(&out_1[count]);
        count += fr.write_data(&out_1[count]);
        count += fr.write_hangbytes(&out_1[count]);

          // **** PMT FOR 9k6 MODULATOR INPUT **** //
        pmt::pmt_t out_vector_2 = pmt::make_s8vector(len_4k8*2, 0);
        int8_t *out_2 = (int8_t*) pmt::uniform_vector_writable_elements(out_vector_2, d_off);

        //debug_msg(out_vector, "4k8\n");
        //debug_msg(out_vector_2, "9k6\n");
          // **** PUBLISH MESSAGES **** //
        message_port_pub(pmt::mp("single"), pmt::cons(pmt::PMT_NIL, pmt::make_blob(&out_1[0], len_4k8)));
        message_port_pub(pmt::mp("double"), pmt::cons(pmt::PMT_NIL, pmt::make_blob(&out_2[0], len_4k8*2)));
      }
    }

    void nx_encoder_impl::debug_msg(pmt::pmt_t msg, const char * errormsg){
    #ifdef DEBUG_ON
      std::cout << "****************************************** " << std::endl;
      std::cout << "DEBUG: " << errormsg << std::endl;

      // get the message
      //pmt::pmt_t meta = pmt::car(msg);
      //pmt::pmt_t vector = pmt::cdr(msg);

      pmt::pmt_t vector = msg;
      size_t len = pmt::length(vector);
      std::cout << "Length: " << len << std::endl;

      const uint8_t* d = (const uint8_t*) pmt::uniform_vector_elements(vector, d_off);
      for(size_t i=0; i<len; i+=16){
        printf("%04x: ", ((unsigned int)i));
        for(size_t j=i; j<std::min(i+16,len); j++){
          printf("%02x ",d[j] );
        }
        std::cout << std::endl;
      }
      std::cout << "****************************************** " << std::endl;
    #endif
    }

  } /* namespace tnc_nx */
} /* namespace gr */


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


#ifndef INCLUDED_TNC_NX_GSCF_COM_H
#define INCLUDED_TNC_NX_GSCF_COM_H

#include <tnc_nx/api.h>
#include <stdint.h>
#include <gnuradio/blocks/pdu.h>

namespace gr {
  namespace tnc_nx {

	#define 	g_MOID	0x01
	#define		g_ERR	0x02
	#define		g_TM	0x04
	#define		g_TC	0x06
	#define		g_ACK 	0x08
	#define		g_CMD 	0x09
	#define		g_DIGI	0x0A
	#define		g_TRIG	0x0C

	// ACK CODES
	#define TNC_OK		0x03
	#define TNC_NOK		0x04
  #define SAT_OK		0x05
	#define SAT_NOK		0x06

    /*!
     * \brief <+description+>
     *
     */
    class TNC_NX_API gscf_com
    {
    public:
      gscf_com();
      ~gscf_com();

      pmt::pmt_t out_ack(uint8_t ack) {
    	  return send_msg_output(pmt::make_u8vector(1, ack), g_ACK, 1);
      }
      pmt::pmt_t out_error(pmt::pmt_t err_msg) {
    	  return send_msg_output_no_cmdid(err_msg, g_ERR, pmt::length(err_msg));
      }
      pmt::pmt_t out_trigger_rx() {
    	  return send_msg_output_no_cmdid(pmt::make_u8vector(0, 0), g_TRIG, 0);
      }
      pmt::pmt_t out_module_id() {
    	  return send_msg_output_no_cmdid(pmt::make_u8vector(1, 0x04), g_MOID, 1);
      }
      pmt::pmt_t out_msg(pmt::pmt_t vector) {
    	  return send_msg_output(vector, g_TM, (uint16_t)(pmt::length(vector)));
      }
      pmt::pmt_t out_digi(pmt::pmt_t vector) {
    	  return send_msg_output(vector, g_DIGI, (uint16_t)(pmt::length(vector)));
      }

      void 		set_cmdid(uint16_t cmdid)		{ d_cmdid = cmdid; }
      uint16_t 	get_cmdid()					{ return d_cmdid; }
      void 		set_noradno(uint16_t noradno)	{ d_noradno = noradno; }
      uint32_t	get_noradno()					{ return d_noradno; }

    private:
      uint16_t 		d_cmdid;
      uint32_t		d_noradno;
      size_t		d_off;

      pmt::pmt_t send_msg_output(pmt::pmt_t msg, int msg_type, uint16_t msg_len);
      pmt::pmt_t send_msg_output_no_cmdid(pmt::pmt_t msg, int msg_type, uint16_t msg_len);
    };

  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_TNC_NX_GSCF_COM_H */

